//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//////////////////////////////////////////////////////////////////////
#include "otpch.h"

#include "definitions.h"

#include "ban.h"
#include "iologindata.h"
#include "configmanager.h"
#include "tools.h"
#include "database.h"

extern ConfigManager g_config;
extern IOLoginData IOLoginData;
IOBan IOBan;

Ban::Ban()
{
	OTSYS_THREAD_LOCKVARINIT(banLock);
}

void Ban::init()
{
	maxLoginTries = (uint32_t)g_config.getNumber(ConfigManager::LOGIN_TRIES);
	retryTimeout = (uint32_t)g_config.getNumber(ConfigManager::RETRY_TIMEOUT) / 1000;
	loginTimeout = (uint32_t)g_config.getNumber(ConfigManager::LOGIN_TIMEOUT) / 1000;
}

bool Ban::isIpBanished(uint32_t clientip)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	if(clientip != 0)
	{
		for(IpBanList::iterator it = ipBanList.begin(); it != ipBanList.end(); ++it)
		{
			if((it->ip & it->mask) == (clientip & it->mask))
			{
				time_t currentTime = time(NULL);
				if(it->time == 0 || currentTime < it->time)
					return true;
			}
		}
	}
	return false;
}

bool Ban::isIpDisabled(uint32_t clientip)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	if(maxLoginTries == 0)
		return false;
	
	if(clientip != 0)
	{
		time_t currentTime = time(NULL);
		IpLoginMap::const_iterator it = ipLoginMap.find(clientip);
		if(it != ipLoginMap.end())
		{
			if((it->second.numberOfLogins >= maxLoginTries) && (currentTime < it->second.lastLoginTime + loginTimeout))
				return true;
		}
	}

	return false;
}

bool Ban::acceptConnection(uint32_t clientip)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);

	if(clientip == 0)
		return false;

	uint64_t currentTime = OTSYS_TIME();

	IpConnectMap::iterator it = ipConnectMap.find(clientip);
	if(it == ipConnectMap.end())
	{
		ConnectBlock cb;
		cb.lastConnection = currentTime;

		ipConnectMap[clientip] = cb;
		return true;
	}

	if(currentTime - it->second.lastConnection < 1000)
		return false;

	it->second.lastConnection = currentTime;
	return true;
}

void Ban::addLoginAttempt(uint32_t clientip, bool isSuccess)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	if(clientip != 0)
	{
		time_t currentTime = time(NULL);

		IpLoginMap::iterator it = ipLoginMap.find(clientip);
		if(it == ipLoginMap.end())
		{
			LoginBlock lb;
			lb.lastLoginTime = 0;
			lb.numberOfLogins = 0;

			ipLoginMap[clientip] = lb;
			it = ipLoginMap.find(clientip);
		}

		if(it->second.numberOfLogins >= maxLoginTries)
			it->second.numberOfLogins = 0;

		if(!isSuccess || (currentTime < it->second.lastLoginTime + retryTimeout))
			++it->second.numberOfLogins;
		else
			it->second.numberOfLogins = 0;

		it->second.lastLoginTime = currentTime;
	}
}

bool Ban::isPlayerNamelocked(const std::string& name)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	uint32_t playerId;
	std::string playerName = name;
	if(!IOLoginData.getGuidByName(playerId, playerName))
		return false;
	
	for(PlayerNamelockList::iterator it = playerNamelockList.begin(); it != playerNamelockList.end(); ++it)
	{
 		if(it->id == playerId)
			return true;
	}
	return false;
}

bool Ban::isAccountDeleted(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountDeletionList::iterator it = accountDeletionList.begin(); it != accountDeletionList.end(); ++it)
	{
 		if(it->id == account)
			return true;
	}
	return false;
}

bool Ban::isAccountBanished(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	time_t currentTime = time(NULL);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
 		if(it->id == account)
		{
			if(currentTime < it->time)
				return true;
		}
	}
	return false;
}

int32_t Ban::getReason(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
 		if(it->id == account)
			return it->reasonId;
	}
	for(AccountDeletionList::iterator it = accountDeletionList.begin(); it != accountDeletionList.end(); ++it)
	{
 		if(it->id == account)
			return it->reasonId;
	}
	return 0;
}

int32_t Ban::getAction(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
 		if(it->id == account)
			return it->actionId;
	}
	for(AccountDeletionList::iterator it = accountDeletionList.begin(); it != accountDeletionList.end(); ++it)
	{
 		if(it->id == account)
			return it->actionId;
	}
	return 0;
}

uint32_t Ban::getBanTime(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
 		if(it->id == account)
			return it->time;
	}
	for(AccountDeletionList::iterator it = accountDeletionList.begin(); it != accountDeletionList.end(); ++it)
	{
 		if(it->id == account)
			return it->time;
	}
	return 0;
}

std::string Ban::getComment(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
 		if(it->id == account)
			return it->comment;
	}
	for(AccountDeletionList::iterator it = accountDeletionList.begin(); it != accountDeletionList.end(); ++it)
	{
 		if(it->id == account)
			return it->comment;
	}
	for(AccountNotationList::iterator it = accountNotationList.begin(); it != accountNotationList.end(); ++it)
	{
 		if(it->id == account)
			return it->comment;
	}
	return "";
}

uint32_t Ban::getBannedBy(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
 		if(it->id == account)
			return it->bannedBy;
	}
	for(AccountDeletionList::iterator it = accountDeletionList.begin(); it != accountDeletionList.end(); ++it)
	{
 		if(it->id == account)
			return it->bannedBy;
	}
	return 0;
}

int32_t Ban::getNotationsCount(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	int32_t notations = 0;
	for(AccountNotationList::iterator it = accountNotationList.begin(); it != accountNotationList.end(); ++it)
	{
		if(it->id == account)
			notations++;
	}
	return notations;
}

void Ban::addIpBan(uint32_t ip, uint32_t mask, time_t time)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(IpBanList::iterator it = ipBanList.begin(); it != ipBanList.end(); ++it)
	{
		if(it->ip == ip && it->mask == mask)
		{
			it->time = time;
			return;
		}
	}
	IpBanStruct ipBanStruct(ip, mask, time);
	ipBanList.push_back(ipBanStruct);
}

void Ban::addPlayerNamelock(uint32_t playerId)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(PlayerNamelockList::iterator it = playerNamelockList.begin(); it != playerNamelockList.end(); ++it)
	{
		if(it->id == playerId)
			return;
	}
	PlayerNamelockStruct playerNamelockStruct(playerId);
	playerNamelockList.push_back(playerNamelockStruct);
}

void Ban::addAccountNotation(uint32_t account, time_t time, std::string comment, uint32_t bannedBy)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountNotationList::iterator it = accountNotationList.begin(); it != accountNotationList.end(); ++it)
	{
		if(it->id == account)
			return;
	}
	AccountNotationStruct accountNotationStruct(account, time, comment, bannedBy);
	accountNotationList.push_back(accountNotationStruct);
}

void Ban::addAccountDeletion(uint32_t account, time_t time, int32_t reasonId, int32_t actionId, std::string comment, uint32_t bannedBy)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountDeletionList::iterator it = accountDeletionList.begin(); it != accountDeletionList.end(); ++it)
	{
		if(it->id == account)
			return;
	}
	AccountDeletionStruct accountDeletionStruct(account, time, reasonId, actionId, comment, bannedBy);
	accountDeletionList.push_back(accountDeletionStruct);
}

void Ban::addAccountBan(uint32_t account, time_t time, int32_t reasonId, int32_t actionId, std::string comment, uint32_t bannedBy)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
		if(it->id == account)
		{
			it->time = time;
			return;
		}
	}
	AccountBanStruct accountBanStruct(account, time, reasonId, actionId, comment, bannedBy);
	accountBanList.push_back(accountBanStruct);
}

bool Ban::removeIpBan(uint32_t n)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(IpBanList::iterator it = ipBanList.begin(); it != ipBanList.end(); ++it)
	{
		--n;
		if(n == 0)
		{
			ipBanList.erase(it);
			return true;
		}
	}
	return false;
}

bool Ban::removePlayerNamelock(uint32_t guid)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(PlayerNamelockList::iterator it = playerNamelockList.begin(); it != playerNamelockList.end(); ++it)
	{
		if(it->id == guid)
		{
			playerNamelockList.erase(it);
			return true;
		}
	}
	return false;
}

bool Ban::removeAccountNotations(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountNotationList::iterator it = accountNotationList.begin(); it != accountNotationList.end(); ++it)
	{
		if(it->id == account)
			accountNotationList.erase(it);
	}
	return true;
}

bool Ban::removeAccountBan(uint32_t account)
{
	OTSYS_THREAD_LOCK_CLASS lockClass(banLock);
	for(AccountBanList::iterator it = accountBanList.begin(); it != accountBanList.end(); ++it)
	{
		if(it->id == account)
		{
			accountBanList.erase(it);
			return true;
		}
	}
	return false;
}

const IpBanList& Ban::getIpBans()
{
	return ipBanList;
}

const PlayerNamelockList& Ban::getPlayerNamelocks()
{
	return playerNamelockList;
}

const AccountBanList& Ban::getAccountBans()
{
	return accountBanList;
}

bool Ban::loadBans()
{
	return IOBan.loadBans(*this);
}

bool Ban::saveBans()
{
	return IOBan.saveBans(*this);
}

bool IOBan::loadBans(Ban& banclass)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;

	DBQuery query;
	DBResult result;
	query << "SELECT * FROM bans";
	if(!db->storeQuery(query, result))
		return true;

	time_t currentTime = time(NULL);
	for(uint32_t i=0; i < result.getNumRows(); ++i)
	{
		int32_t banType = result.getDataInt("type", i);
		switch(banType)
		{
			case BAN_IPADDRESS:
			{
				time_t time = result.getDataInt("time", i);
				int32_t ip = result.getDataInt("ip", i);
				int32_t mask = result.getDataInt("mask", i);
				if(time > (int32_t)currentTime)
					banclass.addIpBan(ip, mask, time);
				break;
			}
			
			case NAMELOCK_PLAYER:
			{
				uint32_t player = result.getDataInt("player", i);
				banclass.addPlayerNamelock(player);
				break;
			}
			
			case BAN_ACCOUNT:
			{
				time_t time = result.getDataInt("time", i);
				uint32_t account = result.getDataInt("account", i);
				int32_t reasonId = result.getDataInt("reason_id", i);
				int32_t actionId = result.getDataInt("action_id", i);
				std::string comment = result.getDataString("comment", i);
				uint32_t bannedBy = result.getDataInt("banned_by", i);
				if(time > currentTime)
					banclass.addAccountBan(account, time, reasonId, actionId, comment, bannedBy);
				break;
			}
			
			case NOTATION_ACCOUNT:
			{
				uint32_t account = result.getDataInt("account", i);
				time_t time = result.getDataInt("time", i);
				std::string comment = result.getDataString("comment", i);
				uint32_t bannedBy = result.getDataInt("banned_by", i);
				banclass.addAccountNotation(account, time, comment, bannedBy);
				break;
			}
			
			case DELETE_ACCOUNT:
			{
				time_t time = result.getDataInt("time", i);
				uint32_t account = result.getDataInt("account", i);
				int32_t reasonId = result.getDataInt("reason_id", i);
				int32_t actionId = result.getDataInt("action_id", i);
				std::string comment = result.getDataString("comment", i);
				uint32_t bannedBy = result.getDataInt("banned_by", i);
				banclass.addAccountDeletion(account, time, reasonId, actionId, comment, bannedBy);
				break;
			}
		}
	}
	return true;
}

bool IOBan::saveBans(const Ban& banclass)
{
	Database* db = Database::instance();
	DBQuery query;
	
	if(!db->connect())
		return false;

	DBTransaction trans(db);
	if(!trans.start())
		return false;

	query << "DELETE FROM bans;";
	if(!db->executeQuery(query))
		return false;

	time_t currentTime = time(NULL);
	//save ip bans
	bool executeQuery = false;
	char bans[100];
	DBSplitInsert query_insert(db);
	query_insert.setQuery("INSERT INTO `bans` (`type` , `ip` , `mask`, `time`) VALUES ");
	for(IpBanList::const_iterator it = banclass.ipBanList.begin(); it != banclass.ipBanList.end(); ++it)
	{
		if(it->time > currentTime)
		{
			executeQuery = true;
			sprintf(bans, "(1,%d,%d,%u)", it->ip, it->mask, it->time);
			if(!query_insert.addRow(bans))
				return false;
		}
	}
	if(executeQuery)
	{
		if(!query_insert.executeQuery())
			return false;
	}

	//save player namelocks
	executeQuery = false;
	query_insert.setQuery("INSERT INTO `bans` (`type` , `player`) VALUES ");
	for(PlayerNamelockList::const_iterator it = banclass.playerNamelockList.begin(); it != banclass.playerNamelockList.end(); ++it)
	{
		executeQuery = true;
		sprintf(bans, "(2,%u)", it->id);
		if(!query_insert.addRow(bans))
			return false;
	}
	if(executeQuery)
	{
		if(!query_insert.executeQuery())
			return false;
	}

	//save account bans
	executeQuery = false;
	query_insert.setQuery("INSERT INTO `bans` (`type` , `account` , `time` , `reason_id` , `action_id` , `comment` , `banned_by`) VALUES ");
	for(AccountBanList::const_iterator it = banclass.accountBanList.begin(); it != banclass.accountBanList.end(); ++it)
	{
		if(it->time > currentTime)
		{
			executeQuery = true;
			sprintf(bans, "(3,%u,%u,%d,%d,'%s',%u)", it->id, it->time, it->reasonId, it->actionId, Database::escapeString(it->comment).c_str(), it->bannedBy);
			if(!query_insert.addRow(bans))
				return false;
		}
	}
	if(executeQuery)
	{
		if(!query_insert.executeQuery())
			return false;
	}

	//save account notations
	executeQuery = false;
	query_insert.setQuery("INSERT INTO `bans` (`type` , `account` , `time` , `comment` , `banned_by`) VALUES ");
	for(AccountNotationList::const_iterator it = banclass.accountNotationList.begin(); it != banclass.accountNotationList.end(); ++it)
	{
		if(it->time > currentTime)
		{
			executeQuery = true;
			sprintf(bans, "(4,%u,%u,'%s',%u)", it->id, it->time, Database::escapeString(it->comment).c_str(), it->bannedBy);
			if(!query_insert.addRow(bans))
				return false;
		}
	}
	if(executeQuery)
	{
		if(!query_insert.executeQuery())
			return false;
	}

	//save account deletions
	executeQuery = false;
	query_insert.setQuery("INSERT INTO `bans` (`type` , `account` , `time` , `reason_id` , `action_id` , `comment` , `banned_by`) VALUES ");
	for(AccountDeletionList::const_iterator it = banclass.accountDeletionList.begin(); it != banclass.accountDeletionList.end(); ++it)
	{
		executeQuery = true;
		sprintf(bans, "(5,%u,%u,%d,%d,'%s',%u)", it->id, it->time, it->reasonId, it->actionId, Database::escapeString(it->comment).c_str(), it->bannedBy);
		if(!query_insert.addRow(bans))
			return false;
	}
	if(executeQuery)
	{
		if(!query_insert.executeQuery())
			return false;
	}
	return trans.success();
}

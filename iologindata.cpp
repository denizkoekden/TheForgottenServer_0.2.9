//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Base class for the LoginData loading/saving
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////
#include "otpch.h"

#include "iologindata.h"
#include <algorithm>
#include <functional>
#include "item.h"
#include "configmanager.h"
#include "tools.h"
#include "town.h"
#include "definitions.h"
#include "game.h"
#include "vocation.h"
#include "house.h"
#include <iostream>
#include <iomanip>

extern ConfigManager g_config;
extern IOGuild IOGuild;
extern Vocations g_vocations;
extern Game g_game;

#ifndef __GNUC__
#pragma warning( disable : 4005)
#pragma warning( disable : 4996)
#endif

Account IOLoginData::loadAccount(uint32_t accno)
{
	Account acc;

	Database* db = Database::instance();
	DBQuery query;
	DBResult result;

	query << "SELECT `id`, `password`, `type`, `premdays`, `lastday`, `key`, `warnings` FROM accounts WHERE id = " << accno;
	if(db->connect() && db->storeQuery(query, result))
	{
		acc.accnumber = result.getDataInt("id");
		acc.password = result.getDataString("password");
		acc.accountType = (AccountType_t)result.getDataInt("type");
		acc.premiumDays = result.getDataInt("premdays");
		acc.lastDay = result.getDataInt("lastday");
		acc.recoveryKey = result.getDataString("key");
		acc.warnings = result.getDataInt("warnings");
		
		query << "SELECT `name` FROM players WHERE account_id = " << accno;
		if(db->storeQuery(query, result))
		{
			for(uint32_t i = 0; i < result.getNumRows(); ++i)
			{
				std::string ss = result.getDataString("name", i);
				acc.charList.push_back(ss.c_str());
			}
			acc.charList.sort();
		}
	}
	return acc;
}

bool IOLoginData::saveAccount(Account acc)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;

	DBQuery query;
	query << "UPDATE `accounts` SET `premdays` = " << acc.premiumDays << ", `warnings` = " << acc.warnings << ", `lastday` = " << acc.lastDay << " WHERE `id` = " << acc.accnumber;
	#if defined __USE_MYSQL__ && defined __USE_SQLITE__
		if(g_config.getNumber(ConfigManager::SQLTYPE) == SQL_TYPE_MYSQL)
			query << " LIMIT 1";
	#elif defined __USE_MYSQL__
		query << " LIMIT 1";
	#endif
	return db->executeQuery(query);
}

bool IOLoginData::createAccount(const Player* player)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	query << "INSERT INTO `accounts` (`id`, `password`, `type`, `premdays`, `lastday`, `key`, `warnings`, `group_id`) VALUES (" << player->getNewAccountNumber() << ", '" << Database::escapeString(player->newPassword) << "', 1, 0, 0, 0, 0, 1)";
	return db->executeQuery(query);
}

bool IOLoginData::changePassword(const Player* player)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	query << "UPDATE `accounts` SET `password` = '" << Database::escapeString(player->newPassword) << "' WHERE `id` =" << player->getRealAccountNumber();
	#if defined __USE_MYSQL__ && defined __USE_SQLITE__
		if(g_config.getNumber(ConfigManager::SQLTYPE) == SQL_TYPE_MYSQL)
			query << " LIMIT 1";
	#elif defined __USE_MYSQL__
		query << " LIMIT 1";
	#endif
	return db->executeQuery(query);
}

bool IOLoginData::getPassword(uint32_t accno, const std::string& name, std::string& password)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT `password` FROM `accounts` WHERE `id` = " << accno;
	if(!db->storeQuery(query, result) || result.getNumRows() != 1)
		return false;
	
	std::string accountPassword = result.getDataString("password");
	query << "SELECT `name` FROM `players` WHERE `account_id` = " << accno;
	if(!db->storeQuery(query, result))
		return false;
	
	for(uint32_t i = 0; i < result.getNumRows(); ++i)
	{
		if(result.getDataString("name", i) == name)
		{
			password = accountPassword;
			return true;
		}
	}
	return false;
}

bool IOLoginData::accountExists(uint32_t accno)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT `id` FROM `accounts` WHERE `id` = " << accno;
	if(!db->storeQuery(query, result) || result.getNumRows() == 0)
		return false;

	return true;
}

bool IOLoginData::setRecoveryKey(const Player* player, std::string recoveryKey)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	query << "UPDATE `accounts` SET `key` = '" << recoveryKey << "' WHERE `id` =" << player->getRealAccountNumber();
	#if defined __USE_MYSQL__ && defined __USE_SQLITE__
		if(g_config.getNumber(ConfigManager::SQLTYPE) == SQL_TYPE_MYSQL)
			query << " LIMIT 1";
	#elif defined __USE_MYSQL__
		query << " LIMIT 1";
	#endif
	
	return db->executeQuery(query);
}

bool IOLoginData::validRecoveryKey(const Player* player)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;

	DBQuery query;
	DBResult result;

	query << "SELECT `id` FROM `accounts` WHERE `key` = '" << Database::escapeString(player->recoveryKeyAttempt) << "' AND id = " << atoi(player->accountNumberAttempt.c_str());
	return db->storeQuery(query, result);
}

bool IOLoginData::setNewPassword(const Player* player, std::string newPassword)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;

	if(g_config.getNumber(ConfigManager::PASSWORD_TYPE) == PASSWORD_TYPE_MD5)
		newPassword = transformToMD5(newPassword);

	DBQuery query;
	query << "UPDATE `accounts` SET `password` = '" << Database::escapeString(newPassword) << "' WHERE `id` =" << Database::escapeString(player->accountNumberAttempt);
	#if defined __USE_MYSQL__ && defined __USE_SQLITE__
		if(g_config.getNumber(ConfigManager::SQLTYPE) == SQL_TYPE_MYSQL)
			query << " LIMIT 1";
	#elif defined __USE_MYSQL__
		query << " LIMIT 1";
	#endif
	return db->executeQuery(query);
}

AccountType_t IOLoginData::getAccountType(std::string name)
{
	Database* db = Database::instance();
	if(!db->connect())
		return ACCOUNT_TYPE_NORMAL;
	
	DBQuery query;
	DBResult result;
	query << "SELECT `account_id` FROM `players` WHERE `name` = '" << Database::escapeString(name) << "'";
	if(!db->storeQuery(query, result))
		return ACCOUNT_TYPE_NORMAL;
	
	query << "SELECT `type` FROM `accounts` WHERE `id` =" << result.getDataInt("account_id");
	if(!db->storeQuery(query, result))
		return ACCOUNT_TYPE_NORMAL;
	
	return (AccountType_t)result.getDataInt("type");
}

bool IOLoginData::loadPlayer(Player* player, std::string name)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
 
	query << "SELECT `id`, `account_id`, `group_id`, `sex`, `vocation`, `experience`, `level`, `maglevel`, `health`, `healthmax`, `blessings`, `mana`, `manamax`, `manaspent`, `soul`, `lookbody`, `lookfeet`, `lookhead`, `looklegs`, `looktype`, `lookaddons`, `posx`, `posy`, `posz`, `cap`, `lastlogin`, `lastlogout`, `lastip`, `conditions`, `redskulltime`, `redskull`, `guildnick`, `rank_id`, `town_id` FROM players WHERE name='" << Database::escapeString(name) << "'";
	if(!db->storeQuery(query, result) || result.getNumRows() != 1)
		return false;

	uint32_t accno = result.getDataInt("account_id");
	if(accno < 1)
		return false;

	Account acc = loadAccount(accno);

	// Getting all player properties
	player->setGUID(result.getDataInt("id"));

	player->accountNumber = accno;
	player->setSex((PlayerSex_t)result.getDataInt("sex"));

	player->accountType = acc.accountType;
	player->premiumDays = acc.premiumDays;

	player->groupId = result.getDataInt("group_id");
	const PlayerGroup* group = getPlayerGroup(result.getDataInt("group_id"));
	if(group){
		player->groupName = group->m_name;
		toLowerCaseString(player->groupName);
		player->accessLevel = (group->m_access >= 1);
		player->setFlags(group->m_flags);
	}
	
	if(player->premiumDays > 0)
	{
		player->maxDepotLimit = 2000;
		player->maxVipLimit = 50;
	}
	else
	{
		player->maxDepotLimit = 1000;
		player->maxVipLimit = 20;
	}

	player->setDirection(SOUTH);
	player->experience = result.getDataLong("experience");
	player->level = result.getDataInt("level");
	player->soul = result.getDataInt("soul");
	player->capacity = result.getDataInt("cap");
	player->lastLoginSaved = result.getDataLong("lastlogin");
	player->setVocation(result.getDataInt("vocation"));
	player->updateBaseSpeed();
	
	player->mana = result.getDataInt("mana");
	player->manaMax = result.getDataInt("manamax");
	player->manaSpent = result.getDataInt("manaspent");
	player->magLevel = result.getDataInt("maglevel");

	player->health = result.getDataInt("health");
	if(player->health <= 0)
		player->health = 100;

	player->healthMax = result.getDataInt("healthmax");
	if(player->healthMax <= 0)
		player->healthMax = 100;

	player->blessings = result.getDataInt("blessings");

	if(player->accessLevel > 0)
	{
		if(acc.accountType > 4)
			player->defaultOutfit.lookType = 266;
		else
			player->defaultOutfit.lookType = 75;
	}
	else
		player->defaultOutfit.lookType = result.getDataInt("looktype");
	player->defaultOutfit.lookHead = result.getDataInt("lookhead");
	player->defaultOutfit.lookBody = result.getDataInt("lookbody");
	player->defaultOutfit.lookLegs = result.getDataInt("looklegs");
	player->defaultOutfit.lookFeet = result.getDataInt("lookfeet");
	player->defaultOutfit.lookAddons = result.getDataInt("lookaddons");
	player->currentOutfit = player->defaultOutfit;

	if(g_game.getWorldType() != WORLD_TYPE_PVP_ENFORCED)
	{
		int32_t redSkullSeconds = result.getDataInt("redskulltime") - time(NULL);
		if(redSkullSeconds > 0)
		{
			//ensure that we round up the number of ticks
			player->redSkullTicks = (redSkullSeconds + 2) * 1000;
			if(result.getDataInt("redskull") == 1)
				player->skull = SKULL_RED;
		}
	}

	unsigned long conditionsSize = 0;
	const char* conditions = result.getDataBlob("conditions", conditionsSize);
	PropStream propStream;
	propStream.init(conditions, conditionsSize);

	Condition* condition;
	while((condition = Condition::createCondition(propStream)))
	{
		if(condition->unserialize(propStream))
			player->storedConditionList.push_back(condition);
		else
			delete condition;
	}

	player->loginPosition.x = result.getDataInt("posx");
	player->loginPosition.y = result.getDataInt("posy");
	player->loginPosition.z = result.getDataInt("posz");
	
	player->lastLogout = result.getDataLong("lastlogout");
	
	player->town = result.getDataInt("town_id");
	Town* town = Towns::getInstance().getTown(player->town);
	if(town)
		player->masterPos = town->getTemplePosition();
	Position loginPos = player->loginPosition;
	if(loginPos.x == 0 && loginPos.y == 0 && loginPos.z == 0)
		player->loginPosition = player->masterPos;
		
	uint32_t rankid = result.getDataInt("rank_id");
	if(rankid)
	{
		player->guildNick = result.getDataString("guildnick");
		query << "SELECT guild_ranks.name as rank, guild_ranks.guild_id as guildid, guild_ranks.level as level, guilds.name as guildname FROM guild_ranks,guilds WHERE guild_ranks.id = " << rankid << " AND guild_ranks.guild_id = guilds.id";
		if(db->storeQuery(query, result))
		{
			player->guildName = result.getDataString("guildname");
			player->guildLevel = result.getDataInt("level");
			player->guildId = result.getDataInt("guildid");
			player->guildRank = result.getDataString("rank");
		}
	}
	else
	{
		query << "SELECT guild_id FROM guild_invites WHERE player_id = " << player->getGUID();
		if(db->storeQuery(query, result))
		{
			for(uint32_t i = 0; i < result.getNumRows(); ++i)
			{
				uint32_t guild_id = result.getDataInt("guild_id", i);
				player->invitedToGuildsList.push_back(guild_id);
			}
		}
		else
			query.reset();
	}
	
	//get password
	query << "SELECT password FROM accounts WHERE id = " << accno;
	if(!db->storeQuery(query, result) || result.getNumRows() != 1)
		return false;
		
	player->password = result.getDataString("password");

	// we need to find out our skills
	// so we query the skill table
	query << "SELECT `skillid`, `value`, `count` FROM player_skills WHERE player_id = " << player->getGUID();
	if(db->storeQuery(query, result))
	{
		//now iterate over the skills
		for(uint32_t i = 0; i < result.getNumRows(); ++i)
		{
			int32_t skillid = result.getDataInt("skillid", i);
			if(skillid >= SKILL_FIRST && skillid <= SKILL_LAST)
			{
				player->skills[skillid][SKILL_LEVEL] = result.getDataInt("value", i);
				player->skills[skillid][SKILL_TRIES] = result.getDataInt("count", i);
			}
		}
	}
	else
	{
		for(int32_t i = 0; i < 7; i++)
		{
			query << "INSERT INTO `player_skills` ( `player_id`, `skillid`, `value`, `count` ) VALUES (" << player->getGUID() << ", " << i << ", 10, 0)";
			if(!db->executeQuery(query))
				query.reset();
		}
	}

 	query << "SELECT `player_id`, `name` FROM player_spells WHERE player_id = " << player->getGUID();
	if(db->storeQuery(query, result))
	{
		for(uint32_t i = 0; i < result.getNumRows(); ++i)
		{
			std::string spellName = result.getDataString("name", i);
			player->learnedInstantSpellList.push_back(spellName);
		}
	}
	else
		query.reset();

	//load inventory items
	ItemMap itemMap;
	
	query << "SELECT `pid`, `sid`, `itemtype`, `count`, `attributes` FROM player_items WHERE player_id = " << player->getGUID() << " ORDER BY sid DESC";
	if(db->storeQuery(query, result) && (result.getNumRows() > 0))
	{
		loadItems(itemMap, result);
		
		ItemMap::reverse_iterator it;
		ItemMap::iterator it2;
		
		for(it = itemMap.rbegin(); it != itemMap.rend(); ++it)
		{
			Item* item = it->second.first;
			int32_t pid = it->second.second;
			if(pid >= 1 && pid <= 10)
				player->__internalAddThing(pid, item);
			else
			{
				it2 = itemMap.find(pid);
				if(it2 != itemMap.end())
				{
					if(Container* container = it2->second.first->getContainer())
						container->__internalAddThing(item);
				}
			}
		}
	}
	else
		query.reset();
		
	player->updateInventoryWeigth();
	player->updateItemsLight(true);
	player->setSkillsPercents();
	
	//load depot items
	itemMap.clear();
	
	query << "SELECT `pid`, `sid`, `itemtype`, `count`, `attributes` FROM player_depotitems WHERE player_id = " << player->getGUID() << " ORDER BY sid DESC";
	if(db->storeQuery(query, result) && (result.getNumRows() > 0))
	{
		loadItems(itemMap, result);
		ItemMap::reverse_iterator it;
		ItemMap::iterator it2;
		for(it = itemMap.rbegin(); it != itemMap.rend(); ++it)
		{
			Item* item = it->second.first;
			int32_t pid = it->second.second;
			if(pid >= 0 && pid < 100)
			{
				if(Container* c = item->getContainer())
				{
					if(Depot* depot = c->getDepot())
						player->addDepot(depot, pid);
					else
						std::cout << "Error loading depot " << pid << " for player " << player->getGUID() << std::endl;
				}
				else
					std::cout << "Error loading depot " << pid << " for player " << player->getGUID() << std::endl;
			}
			else
			{
				it2 = itemMap.find(pid);
				if(it2 != itemMap.end())
				{
					if(Container* container = it2->second.first->getContainer())
						container->__internalAddThing(item);
				}
			}
		}
	}
	else
		query.reset();
	
	//load storage map
	query << "SELECT `key`, `value` FROM player_storage WHERE player_id = " << player->getGUID();
	if(db->storeQuery(query,result))
	{
		for(uint32_t i = 0; i < result.getNumRows(); ++i)
		{
			uint32_t key = result.getDataInt("key", i);
			int32_t value = result.getDataLong("value", i);
			player->addStorageValue(key, value);
		}
	}
	else
		query.reset();

	//load vip
	query << "SELECT `vip_id` FROM player_viplist WHERE player_id = " << player->getGUID();
	if(db->storeQuery(query,result))
	{
		for(uint32_t i = 0; i < result.getNumRows(); ++i)
		{
			uint32_t vip_id = result.getDataInt("vip_id",i);
			std::string dummy_str;
			if(storeNameByGuid(*db, vip_id))
				player->addVIP(vip_id, dummy_str, false, true);
		}
	}
	else
		query.reset();

	return true;
}

bool IOLoginData::saveItems(const Player* player, const ItemBlockList& itemList, DBSplitInsert& query_insert)
{
	std::list<Container*> listContainer;
	typedef std::pair<Container*, int32_t> containerBlock;
	std::list<containerBlock> stack;

	int32_t parentId = 0;
	int32_t runningId = 100;

	Item* item;
	int32_t pid;

	for(ItemBlockList::const_iterator it = itemList.begin(); it != itemList.end(); ++it)
	{
		pid = it->first;
		item = it->second;
		++runningId;
		
		uint32_t attributesSize;

		PropWriteStream propWriteStream;
		item->serializeAttr(propWriteStream);
		const char* attributes = propWriteStream.getStream(attributesSize);

		char buffer[attributesSize + 50];
		sprintf(buffer, "(%d,%d,%d,%d,%d,'%s')", player->getGUID(), pid, runningId, item->getID(), (int32_t)item->getItemCountOrSubtype(), Database::escapeString(attributes, attributesSize).c_str());
		if(!query_insert.addRow(buffer))
			return false;
		
		if(Container* container = item->getContainer())
			stack.push_back(containerBlock(container, runningId));
	}

	while(stack.size() > 0)
	{
		const containerBlock& cb = stack.front();
		Container* container = cb.first;
		parentId = cb.second;
		stack.pop_front();
		for(uint32_t i = 0; i < container->size(); ++i)
		{
			++runningId;
			item = container->getItem(i);
			Container* container = item->getContainer();
			if(container)
				stack.push_back(containerBlock(container, runningId));
			
			uint32_t attributesSize;
			PropWriteStream propWriteStream;
			item->serializeAttr(propWriteStream);
			const char* attributes = propWriteStream.getStream(attributesSize);

			char buffer[attributesSize + 50];
			sprintf(buffer, "(%d,%d,%d,%d,%d,'%s')", player->getGUID(), parentId, runningId, item->getID(), (int32_t)item->getItemCountOrSubtype(), Database::escapeString(attributes, attributesSize).c_str());
			if(!query_insert.addRow(buffer))
				return false;
		}
	}
	if(!query_insert.executeQuery())
		return false;

	return true;
}

bool IOLoginData::savePlayer(Player* player, bool preSave)
{
	if(preSave)
		player->preSave();
	
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "SELECT save FROM players WHERE id=" << player->getGUID();
	if(!db->storeQuery(query,result) || result.getNumRows() != 1)
		return false;

	if(result.getDataInt("save") != 1)
		return true;

	DBTransaction trans(db);
	if(!trans.start())
		return false;

	//serialize conditions
	PropWriteStream propWriteStream;
	for(ConditionList::const_iterator it = player->conditions.begin(); it != player->conditions.end(); ++it)
	{
		if((*it)->isPersistent())
		{
			if(!(*it)->serialize(propWriteStream))
				return false;
			propWriteStream.ADD_UCHAR(CONDITIONATTR_END);
		}
	}

	uint32_t conditionsSize;
	const char* conditions = propWriteStream.getStream(conditionsSize);

	//First, an UPDATE query to write the player itself
	query << "UPDATE `players` SET ";
	query << "`level` = " << player->level << ", ";
	query << "`group_id` = " << player->groupId << ", ";
	query << "`vocation` = " << (int32_t)player->getVocationId() << ", ";
	query << "`health` = " << player->health << ", ";
	query << "`healthmax` = " << player->healthMax << ", ";
	query << "`experience` = " << player->experience << ", ";
	query << "`lookbody` = " << (int32_t)player->defaultOutfit.lookBody << ", ";
	query << "`lookfeet` = " << (int32_t)player->defaultOutfit.lookFeet << ", ";
	query << "`lookhead` = " << (int32_t)player->defaultOutfit.lookHead << ", ";
	query << "`looklegs` = " << (int32_t)player->defaultOutfit.lookLegs << ", ";
	query << "`looktype` = " << (int32_t)player->defaultOutfit.lookType << ", ";
	query << "`lookaddons` = " << (int32_t)player->defaultOutfit.lookAddons << ", ";
	query << "`maglevel` = " << player->magLevel << ", ";
	query << "`mana` = " << player->mana << ", ";
	query << "`manamax` = " << player->manaMax << ", ";
	query << "`manaspent` = " << player->manaSpent << ", ";
	query << "`soul` = " << player->soul << ", ";
	query << "`town_id` = " << player->town << ", ";
	query << "`posx` = " << player->getLoginPosition().x << ", ";
	query << "`posy` = " << player->getLoginPosition().y << ", ";
	query << "`posz` = " << player->getLoginPosition().z << ", ";
	query << "`cap` = " << player->getCapacity() << ", ";
	query << "`sex` = " << player->sex << ", ";
	if(player->lastLoginSaved != 0)
		query << "`lastlogin` = " << player->lastLoginSaved << ", ";
	if(player->lastIP != 0)
		query << "`lastip` = " << player->lastIP << ", ";
	query << "`conditions` = '" << Database::escapeString(conditions, conditionsSize) << "', ";
	if(g_game.getWorldType() != WORLD_TYPE_PVP_ENFORCED)
	{
		int32_t redSkullTime = 0;
		if(player->redSkullTicks > 0)
			redSkullTime = time(NULL) + player->redSkullTicks/1000;
		query << "`redskulltime` = " << redSkullTime << ", ";
		int32_t redSkull = 0;
		if(player->skull == SKULL_RED)
			redSkull = 1;
		query << "`redskull` = " << redSkull << ", ";
	}
	query << "`lastlogout` = " << player->getLastLogout() << ", ";
	query << "`blessings` = " << player->blessings << ", ";
	query << "`guildnick` = '" << Database::escapeString(player->guildNick) << "', ";
	query << "`rank_id` = " << IOGuild.getRankIdByGuildIdAndLevel(player->getGuildId(), player->getGuildLevel()) << " ";
	query << " WHERE `id` = " << player->getGUID();

	if(!db->executeQuery(query))
		return false;
	query.str("");

	// skills
	for(int32_t i = 0; i <= 6; i++)
	{
		query << "UPDATE player_skills SET value = " << player->skills[i][SKILL_LEVEL] << ", count = " << player->skills[i][SKILL_TRIES] << " WHERE player_id = " << player->getGUID() << " AND  skillid = " << i;
		if(!db->executeQuery(query))
			return false;
		query.str("");
	}

	// learned spells
	query << "DELETE FROM player_spells WHERE player_id=" << player->getGUID();

	if(!db->executeQuery(query))
		return false;

	char buffer[100];
	DBSplitInsert query_insert(db);
	query_insert.setQuery("INSERT INTO `player_spells` (`player_id`, `name` ) VALUES ");
	for(LearnedInstantSpellList::const_iterator it = player->learnedInstantSpellList.begin();
			it != player->learnedInstantSpellList.end(); ++it)
	{
		sprintf(buffer, "(%d,'%s')", player->getGUID(), Database::escapeString(*it).c_str());
		if(!query_insert.addRow(buffer))
			return false;
	}

	if(!query_insert.executeQuery()){
		return false;
	}

	//item saving
	query << "DELETE FROM player_items WHERE player_id=" << player->getGUID();
	if(!db->executeQuery(query))
		return false;
	
	query_insert.setQuery("INSERT INTO `player_items` (`player_id`, `pid`, `sid`, `itemtype`, `count`, `attributes` ) VALUES ");
	
	ItemBlockList itemList;
	Item* item;
	for(int32_t slotId = 1; slotId <= 10; ++slotId)
	{
		if((item = player->inventory[slotId]))
			itemList.push_back(itemBlock(slotId, item));
	}
			
	if(!saveItems(player, itemList, query_insert))
		return false;
		
	//save depot items
	query << "DELETE FROM player_depotitems WHERE player_id=" << player->getGUID();
	if(!db->executeQuery(query))
		return false;
	
	query_insert.setQuery("INSERT INTO `player_depotitems` (`player_id`, `pid`, `sid`, `itemtype`, `count`, `attributes` ) VALUES ");
	itemList.clear();
	for(DepotMap::iterator it = player->depots.begin(); it !=player->depots.end() ;++it)
		itemList.push_back(itemBlock(it->first, it->second));

	if(!saveItems(player, itemList, query_insert))
		return false;

	query.reset();
	query << "DELETE FROM player_storage WHERE player_id=" << player->getGUID();

	if(!db->executeQuery(query))
		return false;

	query_insert.setQuery("INSERT INTO `player_storage` (`player_id`, `key`, `value` ) VALUES ");
	player->genReservedStorageRange();
	for(StorageMap::const_iterator cit = player->getStorageIteratorBegin(); cit != player->getStorageIteratorEnd();cit++)
	{
		sprintf(buffer, "(%d,%d,%u)", player->getGUID(), cit->first, (uint32_t)cit->second);
		if(!query_insert.addRow(buffer))
			return false;
	}
	
	if(!query_insert.executeQuery())
		return false;

	//save guild invites
	query.reset();
	query << "DELETE FROM `guild_invites` WHERE player_id=" << player->getGUID();
	
	if(!db->executeQuery(query))
		return false;
	
	query_insert.setQuery("INSERT INTO `guild_invites` (`player_id`, `guild_id`) VALUES ");
	for(InvitedToGuildsList::const_iterator it = player->invitedToGuildsList.begin(); it != player->invitedToGuildsList.end(); ++it)
	{
		sprintf(buffer, "(%d,%d)", player->getGUID(), *it);
		if(!query_insert.addRow(buffer))
			return false;
	}
	if(!query_insert.executeQuery())
		return false;

	//save vip list
	query.reset();
	query << "DELETE FROM `player_viplist` WHERE `player_id` = " << player->getGUID() << ";";
	if(!db->executeQuery(query))
		return false;

	query_insert.setQuery("INSERT INTO `player_viplist` (`player_id`, `vip_id` ) VALUES ");
	for(VIPListSet::iterator it = player->VIPList.begin(); it != player->VIPList.end(); it++)
	{
		sprintf(buffer, "(%d,%d)", player->getGUID(), *it);
		if(!query_insert.addRow(buffer))
			return false;
	}
	if(!query_insert.executeQuery())
		return false;

	//End the transaction
	return trans.success();
}

bool IOLoginData::storeNameByGuid(Database &db, uint32_t guid)
{
	DBQuery query;
	DBResult result;

	NameCacheMap::iterator it = nameCacheMap.find(guid);
	if(it != nameCacheMap.end())
		return true;

	query << "SELECT `name` FROM players WHERE id = " << guid;

	if(!db.storeQuery(query, result) || result.getNumRows() != 1)
		return false;

	nameCacheMap[guid] = result.getDataString("name");
	return true;
}

bool IOLoginData::getNameByGuid(uint32_t guid, std::string& name)
{
	NameCacheMap::iterator it = nameCacheMap.find(guid);
	if(it != nameCacheMap.end()){
		name = it->second;
		return true;
	}

	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "SELECT `name` FROM players WHERE id = " << guid;
	if(!db->storeQuery(query, result) || result.getNumRows() != 1)
		return false;

	name = result.getDataString("name");
	nameCacheMap[guid] = name;

	return true;
}

bool IOLoginData::getGuidByName(uint32_t &guid, std::string& name)
{
	GuidCacheMap::iterator it = guidCacheMap.find(name);
	if(it != guidCacheMap.end())
	{
		name = it->first;
		guid = it->second;
		return true;
	}

	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT name,id FROM players WHERE name = '" << Database::escapeString(name) << "'";
	if(!db->storeQuery(query, result) || result.getNumRows() != 1)
		return false;

	name = result.getDataString("name");
	guid = result.getDataInt("id");

	guidCacheMap[name] = guid;
	return true;
}

bool IOLoginData::getGuidByNameEx(uint32_t &guid, bool &specialVip, std::string& name)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT `name`, `id`, `group_id`, `account_id` FROM `players` WHERE `name` = '" << Database::escapeString(name) << "'";
	if(!db->storeQuery(query, result) || result.getNumRows() != 1)
		return false;

	name = result.getDataString("name");
	guid = result.getDataInt("id");
	const PlayerGroup* accountGroup = getPlayerGroupByAccount(result.getDataInt("account_id"));
	const PlayerGroup* playerGroup = getPlayerGroup(result.getDataInt("group_id"));
	uint64_t flags = 0;
	if(playerGroup)
		flags |= playerGroup->m_flags;

	if(accountGroup)
		flags |= accountGroup->m_flags;

	specialVip = (0 != (flags & ((uint64_t)1 << PlayerFlag_SpecialVIP)));
	return true;
}

bool IOLoginData::playerExists(std::string name)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT name FROM players WHERE name='" << Database::escapeString(name) << "'";
	return db->storeQuery(query, result);
}

const PlayerGroup* IOLoginData::getPlayerGroup(uint32_t groupid)
{
	PlayerGroupMap::const_iterator it = playerGroupMap.find(groupid);
	if(it != playerGroupMap.end())
		return it->second;
	else
	{
		Database* db = Database::instance();
		if(!db->connect())
			return NULL;
		
		DBQuery query;
		DBResult result;
	
		query << "SELECT `name`, `flags`, `access` FROM `groups` WHERE id = " << groupid;
		if(!db->storeQuery(query, result) && result.getNumRows() != 1)
			return NULL;
		
		PlayerGroup* group = new PlayerGroup;
		group->m_name = result.getDataString("name");
		group->m_flags = result.getDataLong("flags");
		group->m_access = result.getDataInt("access");
		playerGroupMap[groupid] = group;
		return group;
	}
	return NULL;
}

const PlayerGroup* IOLoginData::getPlayerGroupByAccount(uint32_t accno)
{
	Database* db = Database::instance();
	DBQuery query;
	DBResult result;

	query << "SELECT `group_id` FROM `accounts` WHERE `id` = " << accno;
	if(db->connect() && db->storeQuery(query, result) && result.getNumRows() == 1)
		return getPlayerGroup(result.getDataInt("group_id"));

	return NULL;
}

void IOLoginData::loadItems(ItemMap& itemMap, DBResult& result)
{
	for(uint32_t i = 0; i < result.getNumRows(); ++i)
	{
		int32_t sid = result.getDataInt("sid", i);
		int32_t pid = result.getDataInt("pid", i);
		int32_t type = result.getDataInt("itemtype", i);
		int32_t count = result.getDataInt("count", i);
			
		unsigned long attrSize = 0;
		const char* attr = result.getDataBlob("attributes", attrSize, i);
			
		PropStream propStream;
		propStream.init(attr, attrSize);
			
		Item* item = Item::CreateItem(type, count);
		if(item)
		{
			if(!item->unserializeAttr(propStream))
				std::cout << "WARNING: Serialize error in IOLoginData::loadItems" << std::endl;
			std::pair<Item*, int> pair(item, pid);
			itemMap[sid] = pair;
		}
	}
}

int32_t IOLoginData::getLevel(uint32_t guid)
{
	Database* db = Database::instance();
	if(!db->connect())
		return 0;
	
	DBQuery query;
	DBResult result;

	query << "SELECT level FROM players WHERE id=" << guid;
	if(!db->storeQuery(query, result))
		return 0;
		
	return result.getDataInt("level");
}

bool IOLoginData::isPremium(uint32_t guid)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT account_id FROM players WHERE id=" << guid;
	if(!db->storeQuery(query, result))
		return false;
	
	query << "SELECT premdays FROM accounts WHERE id=" << result.getDataInt("account_id");
	if(!db->storeQuery(query, result))
		return false;

	return result.getDataInt("premdays") > 0;
}

bool IOLoginData::resetGuildInformation(uint32_t guid)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	query << "UPDATE players SET rank_id = 0, guildnick = '' WHERE id = " << guid;
	return db->executeQuery(query);
}

bool IOLoginData::changeName(std::string oldName, std::string newName)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;

	if(playerExists(newName))
		return false;
	
	DBQuery query;
	query << "UPDATE players SET name='" << Database::escapeString(newName) << "' WHERE name='" << Database::escapeString(oldName) << "'";
	return db->executeQuery(query);
}

uint32_t IOLoginData::getAccountNumberByName(std::string name)
{
	Database* db = Database::instance();
	if(!db->connect())
		return 0;

	DBQuery query;
	DBResult result;
	query << "SELECT account_id FROM players WHERE name='" << Database::escapeString(name) << "'";
	if(!db->storeQuery(query, result))
		return 0;
		
	return result.getDataLong("account_id");
}

bool IOLoginData::createCharacter(const Player* player)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	if(playerExists(player->newCharacterName))
		return false;
	
	Vocation* vocation = g_vocations.getVocation(player->getNewVocation());
	Vocation* rookVoc = g_vocations.getVocation(0);
	uint16_t healthMax = 150, manaMax = 0, capMax = 400, lookType = 136, hpGain = vocation->getHPGain(), manaGain = vocation->getManaGain(), capGain = vocation->getCapGain(), rookHpGain = rookVoc->getHPGain(), rookManaGain = rookVoc->getManaGain(), rookCapGain = rookVoc->getCapGain();
	if(player->getNewSex() == PLAYERSEX_MALE)
		lookType = 128;
	int64_t level = g_config.getNumber(ConfigManager::START_LEVEL);

	uint64_t exp = 0;
	if(level > 1)
		exp = player->getExpForLv(level);

	for(int32_t i = 1; i < level; i++)
	{
		if(i < 8)
		{
			healthMax += rookHpGain;
			manaMax += rookManaGain;
			capMax += rookCapGain;
		}
		else
		{
			healthMax += hpGain;
			manaMax += manaGain;
			capMax += capGain;
		}
	}
	query << "INSERT INTO `players` (`id`, `name`, `group_id`, `account_id`, `level`, `vocation`, `health`, `healthmax`, `experience`, `lookbody`, `lookfeet`, `lookhead`, `looklegs`, `looktype`, `lookaddons`, `maglevel`, `mana`, `manamax`, `manaspent`, `soul`, `town_id`, `posx`, `posy`, `posz`, `conditions`, `cap`, `sex`, `lastlogin`, `lastip`, `redskull`, `redskulltime`, `save`, `rank_id`, `guildnick`, `lastlogout`, `blessings`) VALUES (NULL, '" << Database::escapeString(player->newCharacterName) << "', 1, " << player->getRealAccountNumber() << ", " << level << ", " << player->getNewVocation() << ", " << healthMax << ", " << healthMax << ", " << exp << ", 68, 76, 78, 39, " << lookType << ", 0, " << g_config.getNumber(ConfigManager::START_MAGICLEVEL) << ", " << manaMax << ", " << manaMax << ", 0, 100, " << g_config.getNumber(ConfigManager::SPAWNTOWN_ID) << ", " << g_config.getNumber(ConfigManager::SPAWNPOS_X) << ", " << g_config.getNumber(ConfigManager::SPAWNPOS_Y) << ", " << g_config.getNumber(ConfigManager::SPAWNPOS_Z) << ", 0, " << capMax << ", " << player->getNewSex() << ", 0, 0, 0, 0, 1, 0, '', 0, 0)";
	return db->executeQuery(query);
}

int32_t IOLoginData::deleteCharacter(const Player* player)
{
	Database* db = Database::instance();
	if(!db->connect())
		return 0;
	
	DBQuery query;
	DBResult result;
	
	Player* _player = g_game.getPlayerByName(player->removeChar);
	if(!_player)
	{
		query << "SELECT id FROM players WHERE name='" << Database::escapeString(player->removeChar) << "' AND account_id=" << player->getRealAccountNumber();
		if(!db->storeQuery(query, result))
			return 0;

		uint32_t id = result.getDataInt("id");
		if(IOGuild.getGuildLevel(id) == 3)
			return 3;

		House* house = Houses::getInstance().getHouseByPlayerId(id);
		if(house)
			return 2;

		query << "DELETE FROM `players` WHERE `id` = " << id;
		if(!db->executeQuery(query))
			return 0;
	
		query << "DELETE FROM `player_items` WHERE `player_id` = " << id;
		db->executeQuery(query);
		query.reset();
		query << "DELETE FROM `player_depotitems` WHERE `player_id` = " << id;
		db->executeQuery(query);
		query.reset();
		query << "DELETE FROM `player_storage` WHERE `player_id` = " << id;
		db->executeQuery(query);
		query.reset();
		query << "DELETE FROM `player_skills` WHERE `player_id` = " << id;
		db->executeQuery(query);
		query.reset();
		query << "DELETE FROM `player_viplist` WHERE `player_id` = " << id;
		db->executeQuery(query);
		query.reset();
		query << "DELETE FROM `player_viplist` WHERE `vip_id` = " << id;
		db->executeQuery(query);
		query.reset();
		query << "DELETE FROM `guild_invites` WHERE `player_id` = " << id;
		db->executeQuery(query);

		for(AutoList<Player>::listiterator it = Player::listPlayer.list.begin(); it != Player::listPlayer.list.end(); ++it)
		{
			VIPListSet::iterator it_ = it->second->VIPList.find(id);
			if(it_ != it->second->VIPList.end())
				it->second->VIPList.erase(it_);
		}
		return 1;
	}
	else	
		return 4;
}

bool IOLoginData::addStorageValue(uint32_t guid, uint32_t storageKey, uint64_t storageValue)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT key FROM player_storage WHERE player_id=" << guid << " AND key=" << storageKey;
	if(db->storeQuery(query, result))
		query << "UPDATE player_storage SET value=" << storageValue << " WHERE player_id=" << guid << " AND key=" << storageKey;
	else
	{
		query.reset();
		query << "INSERT INTO player_storage ( `player_id`, `key`, `value` ) VALUES (" << guid << ", " << storageKey << ", " << storageValue << ");";
	}

	return db->executeQuery(query);
}

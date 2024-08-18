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

#ifndef __IOLOGINDATA_H
#define __IOLOGINDATA_H

#include <string>
#include "account.h"
#include "player.h"
#include "database.h"

class PlayerGroup
{
	public:
		PlayerGroup(){}
		~PlayerGroup(){}
		std::string m_name;
		uint64_t m_flags;
		uint32_t m_access;
};

typedef std::pair<int32_t, Item*> itemBlock;
typedef std::list<itemBlock> ItemBlockList;

class IOLoginData
{
	public:
		IOLoginData(){}
		~IOLoginData(){}

		Account loadAccount(uint32_t accno);
		bool saveAccount(Account acc);
		bool createAccount(const Player* player);
		bool changePassword(const Player* player);
		bool getPassword(uint32_t accno, const std::string& name, std::string& password);
		bool accountExists(uint32_t accno);
		bool setRecoveryKey(const Player* player, std::string recoveryKey);
		bool validRecoveryKey(const Player* player);
		bool setNewPassword(const Player* player, std::string newPassword);
		AccountType_t getAccountType(std::string name);

		bool loadPlayer(Player* player, std::string name);
		bool savePlayer(Player* player, bool preSave);
		bool getGuidByName(uint32_t& guid, std::string& name);
		bool getGuidByNameEx(uint32_t &guid, bool& specialVip, std::string& name);
		bool getNameByGuid(uint32_t guid, std::string& name);
		bool playerExists(std::string name);
		int32_t getLevel(uint32_t guid);
		bool isPremium(uint32_t guid);
		bool resetGuildInformation(uint32_t guid);
		bool changeName(std::string oldName, std::string newName);
		uint32_t getAccountNumberByName(std::string name);
		bool createCharacter(const Player* player);
		int32_t deleteCharacter(const Player* player);
		bool addStorageValue(uint32_t guid, uint32_t storageKey, uint64_t storageValue);
		const PlayerGroup* getPlayerGroup(uint32_t groupid);

	protected:
		bool storeNameByGuid(Database &mysql, uint32_t guid);

		const PlayerGroup* getPlayerGroupByAccount(uint32_t accno);
		struct StringCompareCase
		{
			bool operator()(const std::string& l, const std::string& r) const
			{
				return strcasecmp(l.c_str(), r.c_str()) < 0;
			}
		};

		typedef std::map<int,std::pair<Item*,int> > ItemMap;

		void loadItems(ItemMap& itemMap, DBResult& result);
		bool saveItems(const Player* player, const ItemBlockList& itemList, DBSplitInsert& query_insert);

		typedef std::map<uint32_t, std::string> NameCacheMap;
		typedef std::map<std::string, uint32_t, StringCompareCase> GuidCacheMap;
		typedef std::map<uint32_t, PlayerGroup*> PlayerGroupMap;

		PlayerGroupMap playerGroupMap;
		NameCacheMap nameCacheMap;
		GuidCacheMap guidCacheMap;
};

#endif

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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////
#include "otpch.h"

#include "definitions.h"
#include "configmanager.h"
#include <iostream>

ConfigManager::ConfigManager()
{
	m_isLoaded = false;
}

ConfigManager::~ConfigManager()
{
	//
}

bool ConfigManager::loadFile(const std::string& _filename)
{
	lua_State* L = lua_open();
	if(!L)
		return false;

	if(luaL_dofile(L, _filename.c_str()))
	{
		lua_close(L);
		return false;
	}

	// parse config
	if(!m_isLoaded) // info that must be loaded one time (unless we reset the modules involved)
	{
		m_confString[CONFIG_FILE] = _filename;
		m_confString[IP] = getGlobalString(L, "ip", "127.0.0.1");
		m_confInteger[PORT] = getGlobalNumber(L, "port", 7171);
		m_confInteger[SQL_PORT] = getGlobalNumber(L, "mysqlPort", 3306);
		m_confString[MAP_NAME] = getGlobalString(L, "mapName", "forgotten");
		m_confString[MAP_AUTHOR] = getGlobalString(L, "mapAuthor", "Unknown");
		m_confString[HOUSE_RENT_PERIOD] = getGlobalString(L, "houseRentPeriod", "monthly");
		m_confString[MYSQL_HOST] = getGlobalString(L, "mysqlHost", "localhost");
		m_confString[MYSQL_USER] = getGlobalString(L, "mysqlUser", "root");
		m_confString[MYSQL_PASS] = getGlobalString(L, "mysqlPass", "");
		m_confString[MYSQL_DB] = getGlobalString(L, "mysqlDatabase", "theforgottenserver");
		m_confString[SQLITE_DB] = getGlobalString(L, "sqliteDatabase");
		#if defined __USE_MYSQL__ && defined __USE_SQLITE__
		m_confString[SQL_TYPE] = getGlobalString(L, "sqlType");
		#endif
		m_confString[USE_MD5_PASS] = getGlobalString(L, "useMD5Passwords", "no");
		#if defined __USE_MYSQL__ && defined __USE_SQLITE__
		m_confInteger[SQLTYPE] = SQL_TYPE_NONE;
		#endif
		m_confInteger[PASSWORD_TYPE] = PASSWORD_TYPE_PLAIN;
	}
	
	m_confString[LOGIN_MSG] = getGlobalString(L, "loginMessage", "Welcome to the Forgotten Server!");
	m_confString[SERVER_NAME] = getGlobalString(L, "serverName");
	m_confString[OWNER_NAME] = getGlobalString(L, "ownerName");
	m_confString[OWNER_EMAIL] = getGlobalString(L, "ownerEmail");
	m_confString[URL] = getGlobalString(L, "url");
	m_confString[LOCATION] = getGlobalString(L, "location");
	m_confInteger[LOGIN_TRIES] = getGlobalNumber(L, "loginTries", 3);
	m_confInteger[RETRY_TIMEOUT] = getGlobalNumber(L, "retryTimeout", 30 * 1000);
	m_confInteger[LOGIN_TIMEOUT] = getGlobalNumber(L, "loginTimeout", 5 * 1000);
	m_confString[MOTD] = getGlobalString(L, "motd");
	m_confInteger[MAX_PLAYERS] = getGlobalNumber(L, "maxPlayers");
	m_confInteger[PZ_LOCKED] = getGlobalNumber(L, "pzLocked", 0);
	m_confInteger[DEFAULT_DESPAWNRANGE] = getGlobalNumber(L, "deSpawnRange", 2);
	m_confInteger[DEFAULT_DESPAWNRADIUS] = getGlobalNumber(L, "deSpawnRadius", 50);
	m_confInteger[ALLOW_CLONES] = getGlobalNumber(L, "allowClones", 0);
	m_confInteger[RATE_EXPERIENCE] = getGlobalNumber(L, "rateExp", 1);
	m_confInteger[RATE_SKILL] = getGlobalNumber(L, "rateSkill", 1);
	m_confInteger[RATE_LOOT] = getGlobalNumber(L, "rateLoot", 1);
	m_confInteger[RATE_MAGIC] = getGlobalNumber(L, "rateMagic", 1);
	m_confInteger[RATE_SPAWN] = getGlobalNumber(L, "rateSpawn", 1);
	m_confInteger[GUILD_LEADER_MIN_LVL] = getGlobalNumber(L, "guildLeaderMinLvl", 7);
	m_confInteger[SPAWNPOS_X] = getGlobalNumber(L, "newPlayerSpawnPosX", 100);
	m_confInteger[SPAWNPOS_Y] = getGlobalNumber(L, "newPlayerSpawnPosY", 100);
	m_confInteger[SPAWNPOS_Z] = getGlobalNumber(L, "newPlayerSpawnPosZ", 7);
	m_confInteger[SPAWNTOWN_ID] = getGlobalNumber(L, "newPlayerTownId", 1);
	m_confString[WORLD_TYPE] = getGlobalString(L, "worldType", "pvp");
	m_confInteger[SERVERSAVE_H] = getGlobalNumber(L, "serverSaveHour", 3);
	m_confString[ACCOUNT_MANAGER] = getGlobalString(L, "accountManager", "yes");
	m_confInteger[START_LEVEL] = getGlobalNumber(L, "newPlayerLevel", 1);
	m_confInteger[START_MAGICLEVEL] = getGlobalNumber(L, "newPlayerMagicLevel", 0);
	m_confString[START_CHOOSEVOC] = getGlobalString(L, "newPlayerChooseVoc", "no");
	m_confInteger[HOUSE_PRICE] = getGlobalNumber(L, "housePriceEachSQM", 1000);
	m_confInteger[KILLS_TO_RED] = getGlobalNumber(L, "killsToRedSkull", 3);
	m_confInteger[KILLS_TO_BAN] = getGlobalNumber(L, "killsToBan", 5);
	m_confInteger[HIGHSCORES_TOP] = getGlobalNumber(L, "highscoreDisplayPlayers", 10);
	m_confInteger[HIGHSCORES_UPDATETIME] = getGlobalNumber(L, "updateHighscoresAfterMinutes", 60);
	m_confString[ON_OR_OFF_CHARLIST] = getGlobalString(L, "displayOnOrOffAtCharlist", "no");
	m_confString[ALLOW_CHANGEOUTFIT] = getGlobalString(L, "allowChangeOutfit", "yes");
	m_confString[ONE_PLAYER_ON_ACCOUNT] = getGlobalString(L, "onePlayerOnlinePerAccount", "yes");
	m_confString[CANNOT_ATTACK_SAME_LOOKFEET] = getGlobalString(L, "noDamageToSameLookfeet", "no");
	m_confString[AIMBOT_HOTKEY_ENABLED] = getGlobalString(L, "hotkeyAimbotEnabled", "yes");
	m_confInteger[ACTIONS_DELAY_INTERVAL] = getGlobalNumber(L, "timeBetweenActions", 200);
	m_confInteger[EX_ACTIONS_DELAY_INTERVAL] = getGlobalNumber(L, "timeBetweenExActions", 1000);
	m_confInteger[MAX_MESSAGEBUFFER] = getGlobalNumber(L, "maxMessageBuffer", 4);
	m_confInteger[CRITICAL_HIT_CHANCE] = getGlobalNumber(L, "criticalHitChance", 5);
	m_confString[SHOW_GAMEMASTERS_ONLINE] = getGlobalString(L, "displayGamemastersWithOnlineCommand", "no");
	m_confInteger[KICK_AFTER_MINUTES] = getGlobalNumber(L, "kickIdlePlayerAfterMinutes", 15);
	m_confString[REMOVE_AMMO] = getGlobalString(L, "removeAmmoWhenUsingDistanceWeapon", "yes");
	m_confString[REMOVE_RUNE_CHARGES] = getGlobalString(L, "removeChargesFromRunes", "yes");
	m_confString[RANDOMIZE_TILES] = getGlobalString(L, "randomizeTiles", "yes");
	m_confString[DEFAULT_PRIORITY] = getGlobalString(L, "defaultPriority", "high");
	m_confString[EXPERIENCE_FROM_PLAYERS] = getGlobalString(L, "experienceByKillingPlayers", "no");
	m_confString[SHUTDOWN_AT_SERVERSAVE] = getGlobalString(L, "shutdownAtServerSave", "no");
	m_confString[CLEAN_MAP_AT_SERVERSAVE] = getGlobalString(L, "cleanMapAtServerSave", "yes");
	m_confString[FREE_PREMIUM] = getGlobalString(L, "freePremium", "no");
	m_confInteger[PROTECTION_LEVEL] = getGlobalNumber(L, "protectionLevel", 1);
	m_confString[ADMIN_LOGS_ENABLED] = getGlobalString(L, "adminLogsEnabled", "no");
	m_confInteger[DEATH_LOSE_PERCENT] = getGlobalNumber(L, "deathLosePercent", 10);
	m_confInteger[STATUSQUERY_TIMEOUT] = getGlobalNumber(L, "statusTimeout", 5 * 60 * 1000);
	m_confString[BROADCAST_BANISHMENTS] = getGlobalString(L, "broadcastBanishments", "yes");
	m_confString[GENERATE_ACCOUNT_NUMBER] = getGlobalString(L, "generateAccountNumber", "yes");
	m_isLoaded = true;

	lua_close(L);
	return true;
}

bool ConfigManager::reload()
{
	if(!m_isLoaded)
		return false;

	return loadFile(m_confString[CONFIG_FILE]);
}

const std::string& ConfigManager::getString(uint32_t _what)
{ 
	if(m_isLoaded && _what < LAST_STRING_CONFIG)
		return m_confString[_what];
	else
	{
		std::cout << "Warning: [ConfigManager::getString] " << _what << std::endl;
		return m_confString[DUMMY_STR];
	}
}

int ConfigManager::getNumber(uint32_t _what)
{
	if(m_isLoaded && _what < LAST_INTEGER_CONFIG)
		return m_confInteger[_what];
	else
	{
		std::cout << "Warning: [ConfigManager::getNumber] " << _what << std::endl;
		return 0;
	}
}
bool ConfigManager::setNumber(uint32_t _what, int _value)
{
	if(m_isLoaded && _what < LAST_INTEGER_CONFIG)
	{
		m_confInteger[_what] = _value;
		return true;
	}
	else
	{
		std::cout << "Warning: [ConfigManager::setNumber] " << _what << std::endl;
		return false;
	}
}

std::string ConfigManager::getGlobalString(lua_State* _L, const std::string& _identifier, const std::string& _default)
{
	lua_getglobal(_L, _identifier.c_str());

	if(!lua_isstring(_L, -1))
		return _default;

	int len = (int)lua_strlen(_L, -1);
	std::string ret(lua_tostring(_L, -1), len);
	lua_pop(_L,1);

	return ret;
}

int ConfigManager::getGlobalNumber(lua_State* _L, const std::string& _identifier, const int _default)
{
	lua_getglobal(_L, _identifier.c_str());

	if(!lua_isnumber(_L, -1))
		return _default;

	int val = (int)lua_tonumber(_L, -1);
	lua_pop(_L,1);

	return val;
}

std::string ConfigManager::getGlobalStringField (lua_State* _L, const std::string& _identifier, const int _key, const std::string& _default) {
	lua_getglobal(_L, _identifier.c_str());

	lua_pushnumber(_L, _key);
	lua_gettable(_L, -2);  /* get table[key] */
	if(!lua_isstring(_L, -1))
		return _default;
	std::string result = lua_tostring(_L, -1);
	lua_pop(_L, 2);  /* remove number and key*/
	return result;
}

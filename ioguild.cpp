//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// IOGuild Class - saving/loading guild changes for offline players
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

#include "ioguild.h"
#include "database.h"
#include "game.h"

extern Game g_game;

bool IOGuild::getGuildIdByName(uint32_t &guildId, const std::string& guildName)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT id FROM guilds WHERE name='" << Database::escapeString(guildName) << "'";
	if(!db->storeQuery(query, result) || result.getNumRows() != 1)
		return false;

	guildId = result.getDataInt("id");
	return true;
}

bool IOGuild::guildExists(uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;

	query << "SELECT id FROM guilds WHERE id='" << guildId << "'";
	return db->storeQuery(query, result);
}

uint32_t IOGuild::getRankIdByGuildIdAndLevel(uint32_t guildId, uint32_t guildLevel)
{
	Database* db = Database::instance();
	if(!db->connect())
		return 0;
	
	DBQuery query;
	DBResult result;
	query << "SELECT id FROM guild_ranks WHERE level=" << guildLevel << " AND guild_id=" << guildId;
	if(!db->storeQuery(query, result))
		return 0;

	return result.getDataInt("id");
}

std::string IOGuild::getRankName(int16_t guildLevel, uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "SELECT name FROM guild_ranks WHERE level=" << guildLevel << " AND guild_id=" << guildId;
	if(!db->storeQuery(query, result))
		return false;

	return result.getDataString("name");
}

bool IOGuild::rankNameExists(std::string rankName, uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	query << "SELECT name FROM guild_ranks WHERE guild_id=" << guildId << " AND name='" << Database::escapeString(rankName) << "'";
	return db->storeQuery(query, result);
}

bool IOGuild::changeRankName(std::string oldRankName, std::string newRankName, uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "SELECT name FROM guild_ranks WHERE name='" << Database::escapeString(newRankName) << "' AND guild_id=" << guildId;
	if(db->storeQuery(query, result))
		return false;

	query << "UPDATE guild_ranks SET name='" << Database::escapeString(newRankName) << "' WHERE name='" << Database::escapeString(oldRankName) << "' AND guild_id=" << guildId;
	if(!db->executeQuery(query))
		return false;

	query << "SELECT id FROM guild_ranks WHERE name='" << Database::escapeString(newRankName) << "' AND guild_id=" << guildId;
	if(!db->storeQuery(query, result))
		return false;

	for(AutoList<Player>::listiterator it = Player::listPlayer.list.begin(); it != Player::listPlayer.list.end(); ++it)
	{
		if((*it).second->getGuildId() == guildId && (*it).second->getGuildRank() == oldRankName)
			(*it).second->setGuildRank(newRankName);
	}
	return false;
}

bool IOGuild::createGuild(Player* player)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "INSERT INTO `guilds` ( `id` , `name` , `ownerid` , `creationdata` , `motd` ) VALUES (NULL , '" << Database::escapeString(player->getGuildName()) << "', '" << player->getGUID() << "', '" << time(NULL) << "', 'Your guild has successfully been created, to view all available commands use: <!commands>. If you would like to remove this message use <!cleanmotd>, if you would like to edit it, use <!setmotd newMotd>.');";
	if(!db->executeQuery(query))
		return false;
	
	query << "SELECT id FROM guilds WHERE ownerid=" << player->getGUID();
	if(!db->storeQuery(query, result))
		return false;

	player->setGuildId(result.getDataInt("id"));
	player->setGuildLevel(GUILDLEVEL_LEADER);

	query << "SELECT guild_id FROM guild_ranks WHERE guild_id = " << player->getGUID();
	if(!db->storeQuery(query, result))
	{
		query << "INSERT INTO `guild_ranks` ( `id` , `guild_id` , `name` , `level` ) VALUES (NULL , " << player->getGuildId() << ", 'the Leader', 3);";
		if(!db->executeQuery(query))
			return false;

		query << "INSERT INTO `guild_ranks` ( `id` , `guild_id` , `name` , `level` ) VALUES (NULL, " << player->getGuildId() << ", 'a Vice-Leader', 2)";
		if(!db->executeQuery(query))
			return false;

		query << "INSERT INTO `guild_ranks` ( `id` , `guild_id` , `name` , `level` ) VALUES (NULL, " << player->getGuildId() << ", 'a Member', 1)";
		if(!db->executeQuery(query))
			return false;
	}
	return true;
}

bool IOGuild::joinGuild(Player* player, uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "SELECT name FROM guild_ranks WHERE guild_id=" << guildId << " AND level=1";
	if(!db->storeQuery(query, result))
		return false;
	player->setGuildId(guildId);
	player->setGuildRank(result.getDataString("name"));
	query << "SELECT name as guildname FROM guilds WHERE id=" << guildId;
	if(!db->storeQuery(query, result))
		return false;
	player->setGuildName(result.getDataString("guildname"));
	query << "SELECT id FROM guild_ranks WHERE guild_id=" << guildId << " AND level=1";
	if(!db->storeQuery(query, result))
		return false;
	
	player->setGuildLevel(GUILDLEVEL_MEMBER);
	player->invitedToGuildsList.clear();
	return true;
}

bool IOGuild::disbandGuild(uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	query << "UPDATE players SET rank_id = '' AND guildnick = '' WHERE rank_id = " << getRankIdByGuildIdAndLevel(guildId, 3) << " OR rank_id = " << getRankIdByGuildIdAndLevel(guildId, 2) << " OR rank_id = " << getRankIdByGuildIdAndLevel(guildId, 1);
	db->executeQuery(query);
	for(AutoList<Player>::listiterator it = Player::listPlayer.list.begin(); it != Player::listPlayer.list.end(); ++it)
	{
		if((*it).second->getGuildId() == guildId)
			(*it).second->resetGuildInformation();
	}
	
	query << "DELETE FROM guilds WHERE id=" << guildId;
	if(!db->executeQuery(query))
		return false;
	
	query << "DELETE FROM guild_invites WHERE guild_id=" << guildId;
	db->executeQuery(query);
	query.reset();
	
	query << "DELETE FROM guild_ranks WHERE guild_id=" << guildId;
	return db->executeQuery(query);
}

bool IOGuild::hasGuild(uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	query << "SELECT rank_id FROM players WHERE id=" << guildId;
	if(db->storeQuery(query, result))
	{
		if(result.getDataInt("rank_id") != 0)
			return true;
	}
	return false;
}

bool IOGuild::isInvitedToGuild(uint32_t guid, uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "SELECT player_id, guild_id FROM guild_invites WHERE player_id=" << guid << " AND guild_id=" << guildId;
	return db->storeQuery(query, result);
}

bool IOGuild::invitePlayerToGuild(uint32_t guid, uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	query << "INSERT INTO `guild_invites` ( `player_id` , `guild_id` ) VALUES ('" << guid << "', '" << guildId << "');";
	return db->executeQuery(query);
}

bool IOGuild::revokeGuildInvite(uint32_t guid, uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	query << "DELETE FROM `guild_invites` WHERE player_id=" << guid << " AND guild_id=" << guildId;
	return db->executeQuery(query);
}

uint32_t IOGuild::getGuildId(uint32_t guid)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	
	query << "SELECT rank_id FROM players WHERE id=" << guid;
	if(!db->storeQuery(query, result))
		return 0;
	
	query << "SELECT guild_id FROM guild_ranks WHERE id=" << result.getDataInt("rank_id");
	if(!db->storeQuery(query, result))
		return 0;
	
	return result.getDataInt("guild_id");
}

int8_t IOGuild::getGuildLevel(uint32_t guid)
{
	Database* db = Database::instance();
	if(!db->connect())
		return 0;
	
	DBQuery query;
	DBResult result;
	query << "SELECT rank_id FROM players WHERE id=" << guid;
	if(!db->storeQuery(query, result))
		return 0;
	
	query << "SELECT level FROM guild_ranks WHERE id=" << result.getDataInt("rank_id");
	if(!db->storeQuery(query, result))
		return 0;
	
	return result.getDataInt("level");
}

bool IOGuild::setGuildLevel(uint32_t guid, GuildLevel_t level)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	DBResult result;
	query << "SELECT id FROM guild_ranks WHERE guild_id=" << getGuildId(guid) << " AND level=" << level;
	if(!db->storeQuery(query, result))
		return false;

	query << "UPDATE players SET rank_id=" << result.getDataInt("id") << " WHERE id=" << guid;
	return db->executeQuery(query);
}

bool IOGuild::updateOwnerId(uint32_t guildId, uint32_t guid)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;

	DBQuery query;
	query << "UPDATE guilds SET ownerid = " << guildId << " WHERE id = " << guildId;
	return db->executeQuery(query);
}

bool IOGuild::setGuildNick(uint32_t guid, std::string guildNick)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;

	DBQuery query;
	query << "UPDATE players SET guildnick='" << Database::escapeString(guildNick) << "' WHERE id=" << guid;
	return db->executeQuery(query);
}

bool IOGuild::setMotd(uint32_t guildId, std::string newMotd)
{
	Database* db = Database::instance();
	if(!db->connect())
		return false;
	
	DBQuery query;
	query << "UPDATE guilds SET motd='" << Database::escapeString(newMotd) << "' WHERE id=" << guildId;
	return db->executeQuery(query);
}

std::string IOGuild::getMotd(uint32_t guildId)
{
	Database* db = Database::instance();
	if(!db->connect())
		return "";
	
	DBQuery query;
	DBResult result;
	query << "SELECT motd FROM guilds WHERE id=" << guildId;
	if(!db->storeQuery(query, result))
		return "";
	
	return result.getDataString("motd");
}

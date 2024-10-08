//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Status
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

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "status.h"
#include "configmanager.h"
#include "game.h"
#include "connection.h"
#include "networkmessage.h"
#include "tools.h"
#include "resources.h"

#ifndef WIN32
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
#endif

extern ConfigManager g_config;
extern Game g_game;

enum RequestedInfo_t
{
	REQUEST_BASIC_SERVER_INFO = 1,
	REQUEST_OWNER_SERVER_INFO = 2,
	REQUEST_MISC_SERVER_INFO = 4,
	REQUEST_PLAYERS_INFO = 8,
	REQUEST_MAP_INFO = 16
};

std::map<uint32_t, int64_t> ProtocolStatus::ipConnectMap;

void ProtocolStatus::onRecvFirstMessage(NetworkMessage& msg)
{
	std::map<uint32_t, int64_t>::const_iterator it = ipConnectMap.find(getIP());
	if(it != ipConnectMap.end())
	{
		if(OTSYS_TIME() < it->second + g_config.getNumber(ConfigManager::STATUSQUERY_TIMEOUT))
		{
			getConnection()->closeConnection();
			return;
		}
	}

	ipConnectMap[getIP()] = OTSYS_TIME();

	switch(msg.GetByte())
	{
		//XML info protocol
		case 0xFF:
		{
			if(msg.GetRaw() == "info")
			{
				OutputMessage* output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
				Status* status = Status::instance();
				std::string str = status->getStatusString();
				output->AddBytes(str.c_str(), str.size());
				setRawMessages(true); // we dont want the size header, nor encryption
				OutputMessagePool::getInstance()->send(output);
			}
			break;
		}
		//Another ServerInfo protocol
		case 0x01:
		{
			uint32_t requestedInfo = 0;
			if(msg.GetByte() == 1)
				requestedInfo |= REQUEST_BASIC_SERVER_INFO;

			if(msg.GetByte() == 1)
				requestedInfo |= REQUEST_OWNER_SERVER_INFO;

			if(msg.GetByte() == 1)
				requestedInfo |= REQUEST_MISC_SERVER_INFO;

			if(msg.GetByte() == 1)
				requestedInfo |= REQUEST_PLAYERS_INFO;

			if(msg.GetByte() == 1)
				requestedInfo |= REQUEST_MAP_INFO;
			
			OutputMessage* output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
			Status* status = Status::instance();
			status->getInfo(requestedInfo, output);
			OutputMessagePool::getInstance()->send(output);
			break;
		}
		default:
			break;
	}
	getConnection()->closeConnection();
}

#ifdef __DEBUG_NET_DETAIL__
void ProtocolStatus::deleteProtocolTask()
{
	std::cout << "Deleting ProtocolStatus" << std::endl;
	Protocol::deleteProtocolTask();
}
#endif

Status::Status()
{
	m_playersOnline = 0;
	m_playersMax = 0;
	m_start = OTSYS_TIME();
}

void Status::addPlayer()
{
	m_playersOnline++;
}

void Status::removePlayer()
{
	m_playersOnline--;
}

std::string Status::getStatusString() const
{
	std::string xml;
	char buffer[50];

	xmlDocPtr doc;
	xmlNodePtr p, root;

	doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"tsqp", NULL);
	root = doc->children;

	xmlSetProp(root, (const xmlChar*)"version", (const xmlChar*)"1.0");
	
	p = xmlNewNode(NULL,(const xmlChar*)"serverinfo");
	sprintf(buffer, "%u", (uint32_t)getUptime());
	xmlSetProp(p, (const xmlChar*)"uptime", (const xmlChar*)buffer);
	xmlSetProp(p, (const xmlChar*)"ip", (const xmlChar*)g_config.getString(ConfigManager::IP).c_str());
	xmlSetProp(p, (const xmlChar*)"servername", (const xmlChar*)g_config.getString(ConfigManager::SERVER_NAME).c_str()); char send_[30]; sprintf(send_, "%c%c%c %c%c%c%c%c%c%c%c%c %c%c%c%c%c%c\n", 84, 104, 101, 70, 111, 114, 103, 111, 116, 116, 101, 110, 83, 101, 114, 118, 101, 114);
	sprintf(buffer, "%d", g_config.getNumber(ConfigManager::PORT));
	xmlSetProp(p, (const xmlChar*)"port", (const xmlChar*)buffer);
	xmlSetProp(p, (const xmlChar*)"location", (const xmlChar*)g_config.getString(ConfigManager::LOCATION).c_str());
	xmlSetProp(p, (const xmlChar*)"url", (const xmlChar*)g_config.getString(ConfigManager::URL).c_str());
	xmlSetProp(p, (const xmlChar*)"server", (const xmlChar*)send_);
	xmlSetProp(p, (const xmlChar*)"version", (const xmlChar*)STATUS_SERVER_VERSION);
	xmlSetProp(p, (const xmlChar*)"client", (const xmlChar*)"8.0");
	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"owner");
	xmlSetProp(p, (const xmlChar*)"name", (const xmlChar*)g_config.getString(ConfigManager::OWNER_NAME).c_str());
	xmlSetProp(p, (const xmlChar*)"email", (const xmlChar*)g_config.getString(ConfigManager::OWNER_EMAIL).c_str());
	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"players");
	sprintf(buffer, "%d", m_playersOnline);
	xmlSetProp(p, (const xmlChar*)"online", (const xmlChar*)buffer);
	sprintf(buffer, "%d", m_playersMax);
	xmlSetProp(p, (const xmlChar*)"max", (const xmlChar*)buffer);
	sprintf(buffer, "%d", g_game.getLastPlayersRecord());
	xmlSetProp(p, (const xmlChar*)"peak", (const xmlChar*)buffer);
	xmlAddChild(root, p);
	
	p = xmlNewNode(NULL,(const xmlChar*)"monsters");
	sprintf(buffer, "%d", g_game.getMonstersOnline());
	xmlSetProp(p, (const xmlChar*)"total", (const xmlChar*)buffer);
	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"map");
	xmlSetProp(p, (const xmlChar*)"name", (const xmlChar*)m_mapName.c_str());
	xmlSetProp(p, (const xmlChar*)"author", (const xmlChar*)m_mapAuthor.c_str());
	uint32_t mapWidth, mapHeight;
	g_game.getMapDimensions(mapWidth, mapHeight);
	sprintf(buffer, "%u", mapWidth);
	xmlSetProp(p, (const xmlChar*)"width", (const xmlChar*)buffer);
	sprintf(buffer, "%u", mapHeight);
	xmlSetProp(p, (const xmlChar*)"height", (const xmlChar*)buffer);
	xmlAddChild(root, p);

	xmlNewTextChild(root, NULL, (const xmlChar*)"motd", (const xmlChar*)g_config.getString(ConfigManager::MOTD).c_str());

	xmlChar* s = NULL;
	int32_t len = 0;
	xmlDocDumpMemory(doc, (xmlChar**)&s, &len);
	
	if(s)
		xml = std::string((char*)s, len);
	else
		xml = "";
	
	xmlFreeOTSERV(s);
	xmlFreeDoc(doc);
	return xml;
}

void Status::getInfo(uint32_t requestedInfo, OutputMessage* output) const
{
	if(requestedInfo & REQUEST_BASIC_SERVER_INFO)
	{
		output->AddByte(0x10);
		output->AddString(g_config.getString(ConfigManager::SERVER_NAME).c_str());
		output->AddString(g_config.getString(ConfigManager::IP).c_str());
		char buffer[7];
		sprintf(buffer, "%d", g_config.getNumber(ConfigManager::PORT));
		output->AddString(buffer);
  	}

	if(requestedInfo & REQUEST_OWNER_SERVER_INFO)
	{
		output->AddByte(0x11);
		output->AddString(g_config.getString(ConfigManager::OWNER_NAME).c_str());
		output->AddString(g_config.getString(ConfigManager::OWNER_EMAIL).c_str());
  	}

	if(requestedInfo & REQUEST_MISC_SERVER_INFO)
	{
		uint64_t running = getUptime();
		output->AddByte(0x12);
		output->AddString(g_config.getString(ConfigManager::MOTD).c_str());
		output->AddString(g_config.getString(ConfigManager::LOCATION).c_str());
		output->AddString(g_config.getString(ConfigManager::URL).c_str());
		output->AddU32((uint32_t)(running >> 32));
		output->AddU32((uint32_t)(running));
		output->AddString(STATUS_SERVER_VERSION);
  	}

	if(requestedInfo & REQUEST_PLAYERS_INFO)
	{
		output->AddByte(0x20);
		output->AddU32(m_playersOnline);
		output->AddU32(m_playersMax);
		output->AddU32(g_game.getLastPlayersRecord());
  	}

  	if(requestedInfo & REQUEST_MAP_INFO)
	{
		output->AddByte(0x30);
		output->AddString(m_mapName.c_str());
		output->AddString(m_mapAuthor.c_str());
		uint32_t mapWidth, mapHeight;
		g_game.getMapDimensions(mapWidth, mapHeight);
		output->AddU16(mapWidth);
		output->AddU16(mapHeight);
  	}
	return;
}

bool Status::hasSlot() const
{
	return m_playersOnline < m_playersMax;
}

uint64_t Status::getUptime() const
{
	return (OTSYS_TIME() - m_start) / 1000;
}

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

#include "creatureevent.h"
#include "tools.h"
#include "player.h"

CreatureEvents::CreatureEvents() :
m_scriptInterface("CreatureScript Interface")
{
	m_scriptInterface.initState();
	m_loginEvent = NULL;
	m_logoutEvent = NULL;
	m_thinkEvent = NULL;
	m_prepareDeathEvent = NULL;
}

CreatureEvents::~CreatureEvents()
{
	delete m_loginEvent;
	delete m_logoutEvent;
	delete m_thinkEvent;
	delete m_prepareDeathEvent;

	CreatureEventList::iterator it;
	for(it = m_creatureEvents.begin(); it != m_creatureEvents.end(); ++it)
		delete it->second;
}

void CreatureEvents::clear()
{
	//clear global events
	if(m_loginEvent)
	{
		m_loginEvent->clearEvent();
		delete m_loginEvent;
		m_loginEvent = NULL;
	}

	if(m_logoutEvent)
	{
		m_logoutEvent->clearEvent();
		delete m_logoutEvent;
		m_logoutEvent = NULL;
	}

	if(m_thinkEvent)
	{
		m_thinkEvent->clearEvent();
		delete m_thinkEvent;
		m_thinkEvent = NULL;
	}

	if(m_prepareDeathEvent)
	{
		m_prepareDeathEvent->clearEvent();
		delete m_prepareDeathEvent;
		m_prepareDeathEvent = NULL;
	}

	//clear creature events
	CreatureEventList::iterator it;
	for(it = m_creatureEvents.begin(); it != m_creatureEvents.end(); ++it)
		it->second->clearEvent();
	//clear lua state
	m_scriptInterface.reInitState();
}

LuaScriptInterface& CreatureEvents::getScriptInterface()
{
	return m_scriptInterface;
}

std::string CreatureEvents::getScriptBaseName()
{
	return "creaturescripts";
}

Event* CreatureEvents::getEvent(const std::string& nodeName)
{
	if(nodeName == "event")
		return new CreatureEvent(&m_scriptInterface);
	return NULL;
}

bool CreatureEvents::registerEvent(Event* event, xmlNodePtr p)
{
	CreatureEvent* creatureEvent = dynamic_cast<CreatureEvent*>(event);
	if(!creatureEvent)
		return false;

	switch(creatureEvent->getEventType())
	{
		case CREATURE_EVENT_NONE:
			std::cout << "Error: [CreatureEvents::registerEvent] Trying to register event without type!." << std::endl;
			return false;
			break;
		// global events are stored in
		// member variables and their name is ignored!
		case CREATURE_EVENT_LOGIN:
			delete m_loginEvent;
			m_loginEvent = creatureEvent;
			return true;
			break;
		case CREATURE_EVENT_LOGOUT:
			delete m_logoutEvent;
			m_logoutEvent = creatureEvent;
			return true;
			break;
		case CREATURE_EVENT_THINK:
			delete m_thinkEvent;
			m_thinkEvent = creatureEvent;
			return true;
			break;
		case CREATURE_EVENT_PREPAREDEATH:
			delete m_prepareDeathEvent;
			m_prepareDeathEvent = creatureEvent;
			return true;
			break;
		// other events are stored in a std::map
		default:
		{
			CreatureEvent* oldEvent = getEventByName(creatureEvent->getName(), false);
			if(oldEvent)
			{
				// if there was an event with the same that is not loaded
				// (happens when realoading), it is reused
				if(oldEvent->isLoaded() == false && oldEvent->getEventType() == creatureEvent->getEventType())
					oldEvent->copyEvent(creatureEvent);
				return false;
			}
			else
			{
				//if not, register it normally
				m_creatureEvents[creatureEvent->getName()] = creatureEvent;
				return true;
			}
			break;
		}
	}
}

CreatureEvent* CreatureEvents::getEventByName(const std::string& name, bool forceLoaded /*= true*/)
{
	CreatureEventList::iterator it = m_creatureEvents.find(name);
	if(it != m_creatureEvents.end())
	{
		if(!forceLoaded || it->second->isLoaded())
			return it->second;
	}
	return NULL;
}

uint32_t CreatureEvents::playerLogin(Player* player)
{
	// fire global event if is registered
	if(m_loginEvent)
		return m_loginEvent->executeOnLogin(player);
	return 0;
}

uint32_t CreatureEvents::playerLogout(Player* player)
{
	// fire global event if is registered
	if(m_logoutEvent)
		return m_logoutEvent->executeOnLogout(player);
	return 0;
}

uint32_t CreatureEvents::playerThink(Player* player, uint32_t interval)
{
	// fire global event if is registered
	if(m_thinkEvent)
		return m_thinkEvent->executeOnThink(player, interval);
	return 0;
}

uint32_t CreatureEvents::playerPrepareDeath(Player* player, Creature* killer)
{
	// fire global event if is registered
	if(m_prepareDeathEvent)
		return m_prepareDeathEvent->executeOnPrepareDeath(player, killer);
	return 0;
}

/////////////////////////////////////

CreatureEvent::CreatureEvent(LuaScriptInterface* _interface) :
Event(_interface)
{
	m_type = CREATURE_EVENT_NONE;
	m_isLoaded = false;
}

CreatureEvent::~CreatureEvent()
{
	//
}

bool CreatureEvent::configureEvent(xmlNodePtr p)
{
	std::string str;
	//Name that will be used in monster xml files and
	// lua function to register events to reference this event
	if(readXMLString(p, "name", str))
		m_eventName = str;
	else
	{
		std::cout << "Error: [CreatureEvent::configureEvent] No name for creature event." << std::endl;
		return false;
	}
	
	if(readXMLString(p, "type", str))
	{
		if(str == "login")
			m_type = CREATURE_EVENT_LOGIN;
		else if(str == "logout")
			m_type = CREATURE_EVENT_LOGOUT;
		else if(str == "think")
			m_type = CREATURE_EVENT_THINK;
		else if(str == "preparedeath")
			m_type = CREATURE_EVENT_PREPAREDEATH;
		else if(str == "death")
			m_type = CREATURE_EVENT_DEATH;
		else
		{
			std::cout << "Error: [CreatureEvent::configureEvent] No valid type for creature event." << str << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "Error: [CreatureEvent::configureEvent] No type for creature event."  << std::endl;
		return false;
	}
	m_isLoaded = true;
	return true;
}

std::string CreatureEvent::getScriptEventName()
{
	//Depending on the type script event name is different
	switch(m_type)
	{
		case CREATURE_EVENT_LOGIN:
			return "onLogin";
			break;
		case CREATURE_EVENT_LOGOUT:
			return "onLogout";
			break;
		case CREATURE_EVENT_THINK:
			return "onThink";
			break;
		case CREATURE_EVENT_PREPAREDEATH:
			return "onPrepareDeath";
			break;
		case CREATURE_EVENT_DEATH:
			return "onDeath";
			break;
		case CREATURE_EVENT_NONE:
		default:
			return "";
	}
}

void CreatureEvent::copyEvent(CreatureEvent* creatureEvent)
{
	m_scriptId = creatureEvent->m_scriptId;
	m_scriptInterface = creatureEvent->m_scriptInterface;
	m_scripted = creatureEvent->m_scripted;
	m_isLoaded = creatureEvent->m_isLoaded;
}

void CreatureEvent::clearEvent()
{
	m_scriptId = 0;
	m_scriptInterface = NULL;
	m_scripted = false;
	m_isLoaded = false;
}

uint32_t CreatureEvent::executeOnLogin(Player* player)
{
	//onLogin(cid)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		char desc[30];
		sprintf(desc, "%s", player->getName().c_str());
		env->setEventDesc(desc);
		#endif

		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());

		uint32_t cid = env->addThing(player);

		lua_State* L = m_scriptInterface->getLuaState();

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);

		int32_t result = m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();

		return (result == LUA_TRUE);
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnLogin" << std::endl;
		return 0;
	}
}

uint32_t CreatureEvent::executeOnLogout(Player* player)
{
	//onLogout(cid)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		char desc[30];
		sprintf(desc, "%s", player->getName().c_str());
		env->setEventDesc(desc);
		#endif

		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());

		uint32_t cid = env->addThing(player);

		lua_State* L = m_scriptInterface->getLuaState();

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);

		int32_t result = m_scriptInterface->callFunction(1);
		m_scriptInterface->releaseScriptEnv();

		return (result == LUA_TRUE);
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnLogout" << std::endl;
		return 0;
	}
}

uint32_t CreatureEvent::executeOnThink(Player* player, uint32_t interval)
{
	//onThink(cid, interval)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		char desc[30];
		sprintf(desc, "%s", player->getName().c_str());
		env->setEventDesc(desc);
		#endif

		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());

		uint32_t cid = env->addThing(player);

		lua_State* L = m_scriptInterface->getLuaState();

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, interval);

		int32_t result = m_scriptInterface->callFunction(2);
		m_scriptInterface->releaseScriptEnv();

		return (result == LUA_TRUE);
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnThink" << std::endl;
		return 0;
	}
}

uint32_t CreatureEvent::executeOnPrepareDeath(Player* player, Creature* killer)
{
	//onPrepareDeath(cid, killer)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		char desc[30];
		sprintf(desc, "%s", player->getName().c_str());
		env->setEventDesc(desc);
		#endif

		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());

		uint32_t cid = env->addThing(player);
		uint32_t killercid = env->addThing(killer);

		lua_State* L = m_scriptInterface->getLuaState();

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, killercid);

		int32_t result = m_scriptInterface->callFunction(2);
		m_scriptInterface->releaseScriptEnv();

		return (result == LUA_TRUE);
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnPrepareDeath" << std::endl;
		return 0;
	}
}

uint32_t CreatureEvent::executeOnDeath(Creature* creature, Item* corpse, Creature* killer)
{
	//onDeath(cid, corpse, killer)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();

		#ifdef __DEBUG_LUASCRIPTS__
		char desc[30];
		sprintf(desc, "%s", creature->getName().c_str());
		env->setEventDesc(desc);
		#endif

		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(creature->getPosition());

		uint32_t cid = env->addThing(creature);
		uint32_t corpseid = env->addThing(corpse);
		uint32_t killercid = env->addThing(killer);

		lua_State* L = m_scriptInterface->getLuaState();

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, corpseid);
		lua_pushnumber(L, killercid);

		int32_t result = m_scriptInterface->callFunction(3);
		m_scriptInterface->releaseScriptEnv();

		return (result == LUA_TRUE);
	}
	else
	{
		std::cout << "[Error] Call stack overflow. CreatureEvent::executeOnDeath" << std::endl;
		return 0;
	}
}

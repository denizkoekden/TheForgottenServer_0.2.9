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
#include "const.h"
#include "player.h"
#include "monster.h"
#include "npc.h"
#include "game.h"
#include "item.h"
#include "container.h"
#include "combat.h"
#include "depot.h"
#include "house.h"
#include "tasks.h"
#include "tools.h"
#include "spells.h"
#include "configmanager.h"
#include "iologindata.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h> 

#include "actions.h"

extern Game g_game;
extern Spells* g_spells;
extern Actions* g_actions;
extern ConfigManager g_config;
extern IOLoginData IOLoginData;

Actions::Actions() :
m_scriptInterface("Action Interface")
{
	m_scriptInterface.initState();
}

Actions::~Actions()
{
	clear();
}

inline void Actions::clearMap(ActionUseMap& map)
{
	ActionUseMap::iterator it = map.begin();
	while(it != map.end())
	{
		delete it->second;
		map.erase(it);
		it = map.begin();
	}
}

void Actions::clear()
{
	clearMap(useItemMap);
	clearMap(uniqueItemMap);
	clearMap(actionItemMap);
	
	m_scriptInterface.reInitState();
	m_loaded = false;
}

LuaScriptInterface& Actions::getScriptInterface()
{
	return m_scriptInterface;
}

std::string Actions::getScriptBaseName()
{
	return "actions";
}

Event* Actions::getEvent(const std::string& nodeName)
{
	if(nodeName == "action")
		return new Action(&m_scriptInterface);
	else
		return NULL;
}

bool Actions::registerEvent(Event* event, xmlNodePtr p)
{
	Action* action = dynamic_cast<Action*>(event);
	if(!action)
		return false;
	
	int32_t value, value2;
	if(readXMLInteger(p,"itemid",value))
		useItemMap[value] = action;
	else if(readXMLInteger(p,"fromid",value) && readXMLInteger(p,"toid",value2))
	{
		while(value <= value2)
		{
			useItemMap[value] = action;
			value++;
		}
	}
	else if(readXMLInteger(p,"uniqueid",value))
		uniqueItemMap[value] = action;
	else if(readXMLInteger(p,"fromuid",value) && readXMLInteger(p,"touid",value2))
	{
		while(value <= value2)
		{
			uniqueItemMap[value] = action;
			value++;
		}
	}
	else if(readXMLInteger(p,"actionid",value))
		actionItemMap[value] = action;
	else if(readXMLInteger(p,"fromaid",value) && readXMLInteger(p,"toaid",value2))
	{
		while(value <= value2)
		{
			actionItemMap[value] = action;
			value++;
		}
	}
	else
		return false;
	
	return true;
}

ReturnValue Actions::canUse(const Player* player, const Position& pos)
{
	const Position& playerPos = player->getPosition();
	if(pos.x != 0xFFFF)
	{
		if(playerPos.z > pos.z)
			return RET_FIRSTGOUPSTAIRS;
		else if(playerPos.z < pos.z)
			return RET_FIRSTGODOWNSTAIRS;
		else if(!Position::areInRange<1,1,0>(playerPos, pos))
			return RET_TOOFARAWAY;
	}
	return RET_NOERROR;
}

ReturnValue Actions::canUse(const Player* player, const Position& pos, const Item* item)
{
	Action* action = getAction(item);
	if(action)
		return action->canExecuteAction(player, pos);

	return RET_NOERROR;
}

ReturnValue Actions::canUseFar(const Creature* creature, const Position& toPos, bool checkLineOfSight)
{
	if(toPos.x == 0xFFFF)
		return RET_NOERROR;

	const Position& creaturePos = creature->getPosition();

	if(creaturePos.z > toPos.z)
		return RET_FIRSTGOUPSTAIRS;
	else if(creaturePos.z < toPos.z)
		return RET_FIRSTGODOWNSTAIRS;
	else if(!Position::areInRange<7,5,0>(toPos, creaturePos))
		return RET_TOOFARAWAY;
	
	if(checkLineOfSight && !g_game.getMap()->canThrowObjectTo(creaturePos, toPos))
		return RET_CANNOTTHROW;

	return RET_NOERROR;
}

Action* Actions::getAction(const Item* item)
{
	if(item->getUniqueId() != 0)
	{
		ActionUseMap::iterator it = uniqueItemMap.find(item->getUniqueId());
		if(it != uniqueItemMap.end())
			return it->second;
	}
	
	if(item->getActionId() != 0)
	{
		ActionUseMap::iterator it = actionItemMap.find(item->getActionId());
		if(it != actionItemMap.end())
			return it->second;
	}
	
	ActionUseMap::iterator it = useItemMap.find(item->getID());
	if(it != useItemMap.end())
		return it->second;

	//rune items
	Action* runeSpell = g_spells->getRuneSpell(item->getID());
	if(runeSpell)
		return runeSpell;
	
	return NULL;
}

ReturnValue Actions::internalUseItem(Player* player, const Position& pos,
	uint8_t index, Item* item, uint32_t creatureId)
{
	//check if it is a house door
	if(Door* door = item->getDoor())
	{
		if(!door->canUse(player))
			return RET_CANNOTUSETHISOBJECT;
	}

	//look for the item in action maps
	Action* action = getAction(item);
	
	//if found execute it
	if(action)
	{
		int32_t stack = item->getParent()->__getIndexOfThing(item);
		PositionEx posEx(pos, stack);
		if(action->isScripted())
		{
			if(action->executeUse(player, item, posEx, posEx, false, creatureId))
				return RET_NOERROR;
		}
		else
		{
			if(action->function)
			{
				if(action->function(player, item, posEx, posEx, false, creatureId))
					return RET_NOERROR;
			}
		}
	}
	
	if(item->isReadable())
	{
		if(item->canWriteText())
		{
			player->setWriteItem(item, item->getMaxWriteLength());
			player->sendTextWindow(item, item->getMaxWriteLength(), true);
		}
		else
		{
			player->setWriteItem(NULL);
			player->sendTextWindow(item, 0, false);
		}
		return RET_NOERROR;
	}
	
	//if it is a container try to open it
	if(Container* container = item->getContainer())
	{
		if(openContainer(player, container, index))
			return RET_NOERROR;
	}

	//we dont know what to do with this item
	return RET_CANNOTUSETHISOBJECT;
}

bool Actions::useItem(Player* player, const Position& pos, uint8_t index, Item* item, bool isHotkey)
{
	if(OTSYS_TIME() - player->getLastAction() < g_config.getNumber(ConfigManager::ACTIONS_DELAY_INTERVAL))
		return false;

	if(isHotkey)
	{
		int32_t subType = -1;
		if(item->hasSubType() && !item->hasCharges())
			subType = item->getSubType();

		uint32_t itemCount = player->__getItemTypeCount(item->getID(), subType, false);
		ReturnValue ret = internalUseItem(player, pos, index, item, 0);
		if(ret != RET_NOERROR)
		{
			player->sendCancelMessage(ret);
			return false;
		}
		showUseHotkeyMessage(player, item, itemCount);
	}
	else
	{
		ReturnValue ret = internalUseItem(player, pos, index, item, 0);
		if(ret != RET_NOERROR)
		{
			player->sendCancelMessage(ret);
			return false;
		}
	}
	
	player->setLastAction(OTSYS_TIME());
	return true;
}

bool Actions::useItemEx(Player* player, const Position& fromPos, const Position& toPos,
	uint8_t toStackPos, Item* item, bool isHotkey, uint32_t creatureId /* = 0*/)
{
	if(OTSYS_TIME() - player->getLastAction() < g_config.getNumber(ConfigManager::EX_ACTIONS_DELAY_INTERVAL))
		return false;

	Action* action = getAction(item);
	if(!action)
	{
		player->sendCancelMessage(RET_CANNOTUSETHISOBJECT);
		return false;
	}

	ReturnValue ret = action->canExecuteAction(player, toPos);
	if(ret != RET_NOERROR)
	{
		player->sendCancelMessage(ret);
		return false;
	}

	int32_t fromStackPos = item->getParent()->__getIndexOfThing(item);
	PositionEx fromPosEx(fromPos, fromStackPos);
	PositionEx toPosEx(toPos, toStackPos);

	if(isHotkey)
	{
		int32_t subType = -1;
		if(item->hasSubType() && !item->hasCharges())
			subType = item->getSubType();

		uint32_t itemCount = player->__getItemTypeCount(item->getID(), subType, false);
		if(!action->executeUse(player, item, fromPosEx, toPosEx, true, creatureId))
		{
			if(!action->hasOwnErrorHandler())
				player->sendCancelMessage(RET_CANNOTUSETHISOBJECT);
			return false;
		}

		showUseHotkeyMessage(player, item, itemCount);
	}
	else
	{
		if(!action->executeUse(player, item, fromPosEx, toPosEx, true, creatureId))
		{
			if(!action->hasOwnErrorHandler())
				player->sendCancelMessage(RET_CANNOTUSETHISOBJECT);
			return false;
		}
	}
	player->setLastAction(OTSYS_TIME());
	return true;
}

void Actions::showUseHotkeyMessage(Player* player, Item* item, uint32_t itemCount)
{
	char buffer[60];
	if(itemCount == 1)
		sprintf(buffer, "Using the last %s...", item->getName().c_str());
	else
		sprintf(buffer, "Using one of %d %s...", itemCount, item->getPluralName().c_str());
	player->sendTextMessage(MSG_INFO_DESCR, buffer);
}

bool Actions::openContainer(Player* player, Container* container, const uint8_t index)
{
	Container* openContainer = NULL;

	//depot container
	if(Depot* depot = container->getDepot())
	{
		Depot* myDepot = player->getDepot(depot->getDepotId(), true);
		myDepot->setParent(depot->getParent());
		openContainer = myDepot;
	}
	else
		openContainer = container;
	
	//open/close container
	int32_t oldcid = player->getContainerID(openContainer);
	if(oldcid != -1)
	{
		player->onCloseContainer(openContainer);
		player->closeContainer(oldcid);
	}
	else
	{
		player->addContainer(index, openContainer);
		player->onSendContainer(openContainer);
	}

	return true;
}

Action::Action(LuaScriptInterface* _interface) :
Event(_interface)
{
	allowFarUse = false;
	checkLineOfSight = true;
}

Action::~Action()
{
	//
}

bool Action::configureEvent(xmlNodePtr p)
{
	int32_t intValue;
	if(readXMLInteger(p, "allowfaruse", intValue))
	{
		if(intValue != 0)
			setAllowFarUse(true);
	}

	if(readXMLInteger(p, "blockwalls", intValue))
	{
		if(intValue == 0)
			setCheckLineOfSight(false);
	}
	return true;
}

bool Action::loadFunction(const std::string& functionName)
{
	if(strcasecmp(functionName.c_str(), "bed") == 0)
		function = bed;
	else if(strcasecmp(functionName.c_str(), "increaseItemId") == 0)
		function = increaseItemId;
	else if(strcasecmp(functionName.c_str(), "decreaseItemId") == 0)
		function = decreaseItemId;
	else if(strcasecmp(functionName.c_str(), "highscoreBook") == 0)
		function = highscoreBook;
	else
		return false;

	m_scripted = false;
	return true;
}

bool Action::highscoreBook(Player* player, Item* item, const PositionEx& posFrom, const PositionEx& posTo, bool extendedUse, uint32_t creatureId)
{
	if(player)
	{
		if(item)
		{
			if(item->getActionId() >= 150 && item->getActionId() <= 158)
			{
				std::string highscoreString = g_game.getHighscoreString(item->getActionId() - 150);
				item->setText(highscoreString);
				player->sendTextWindow(item, highscoreString.size(), false);
				return true;
			}
		}
	}
	return false;
}

bool Action::increaseItemId(Player* player, Item* item, const PositionEx& posFrom, const PositionEx& posTo, bool extendedUse, uint32_t creatureId)
{
	if(player)
	{
		if(item)
		{
			g_game.transformItem(item, item->getID() + 1);
			return true;
		}
	}
	return false;
}

bool Action::decreaseItemId(Player* player, Item* item, const PositionEx& posFrom, const PositionEx& posTo, bool extendedUse, uint32_t creatureId)
{
	if(player)
	{
		if(item)
		{
			g_game.transformItem(item, item->getID() - 1);
			return true;
		}
	}
	return false;
}

bool Action::bed(Player* player, Item* item, const PositionEx& posFrom, const PositionEx& posTo, bool extendedUse, uint32_t creatureId)
{
	if(player)
	{
		if(item)
		{
			Tile *tile = g_game.getMap()->getTile(posFrom);
			if(tile)
			{
				HouseTile* houseTile = dynamic_cast<HouseTile*>(tile);
				if(houseTile)
				{
					House* house = houseTile->getHouse();
					if(house)
					{
						if(item->isBed())
						{
							if(item->getSleeper())
							{
								if(player->getGUID() == house->getHouseOwner())
								{
									item->transformTakenBedIntoFree(posFrom);
									item->resetSleeper();
									IOLoginData.addStorageValue(item->getSleeper(), STORAGEVALUE_SLEEPING, time(NULL));
									g_game.addMagicEffect(posFrom, NM_ME_STUN);
									return true;
								}
							}
							else
							{
								item->transformFreeBedIntoTaken(posFrom);
								item->setSleeper(player->getGUID());
								char buffer[50];
								sprintf(buffer, "%s is sleeping there.", player->getName().c_str());
								item->setSpecialDescription(buffer);
								player->addStorageValue(STORAGEVALUE_SLEEPING, (uint64_t)1);
								player->addStorageValue(STORAGEVALUE_SLEEPINGPOS_X, (uint64_t)posFrom.x);
								player->addStorageValue(STORAGEVALUE_SLEEPINGPOS_Y, (uint64_t)posFrom.y);
								player->addStorageValue(STORAGEVALUE_SLEEPINGPOS_Z, (uint64_t)posFrom.z);
								g_game.internalTeleport(player, posFrom, true);
								g_game.addMagicEffect(posFrom, NM_ME_SLEEP);
								player->kickPlayer(false);
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

std::string Action::getScriptEventName()
{
	return "onUse";
}

ReturnValue Action::canExecuteAction(const Player* player, const Position& toPos)
{
	ReturnValue ret = RET_NOERROR;
	if(!getAllowFarUse())
	{
		ret = g_actions->canUse(player, toPos);
		if(ret != RET_NOERROR)
			return ret;
	}
	else
	{
		ret = g_actions->canUseFar(player, toPos, getCheckLineOfSight());
		if(ret != RET_NOERROR)
			return ret;
	}
	return RET_NOERROR;
}

bool Action::executeUse(Player* player, Item* item, const PositionEx& fromPos, const PositionEx& toPos, bool extendedUse, uint32_t creatureId)
{
	//onUse(cid, item, fromPosition, itemEx, toPosition)
	if(m_scriptInterface->reserveScriptEnv())
	{
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
	
		#ifdef __DEBUG_LUASCRIPTS__
		std::stringstream desc;
		desc << player->getName() << " - " << item->getID() << " " << fromPos << "|" << toPos;
		env->setEventDesc(desc.str());
		#endif
	
		env->setScriptId(m_scriptId, m_scriptInterface);
		env->setRealPos(player->getPosition());
	
		uint32_t cid = env->addThing(player);
		uint32_t itemid1 = env->addThing(item);
	
		lua_State* L = m_scriptInterface->getLuaState();
	
		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		LuaScriptInterface::pushThing(L, item, itemid1);
		LuaScriptInterface::pushPosition(L, fromPos, fromPos.stackpos);
		//std::cout << "posTo" <<  (Position)posTo << " stack" << (int32_t)posTo.stackpos <<std::endl;
		Thing* thing = g_game.internalGetThing(player, toPos, toPos.stackpos);
		if(thing && (!extendedUse || thing != item))
		{
			uint32_t thingId2 = env->addThing(thing);
			LuaScriptInterface::pushThing(L, thing, thingId2);
			LuaScriptInterface::pushPosition(L, toPos, toPos.stackpos);
		}
		else
		{
			LuaScriptInterface::pushThing(L, NULL, 0);
			Position posEx;
			LuaScriptInterface::pushPosition(L, posEx, 0);
		}
	
		int32_t result = m_scriptInterface->callFunction(5);
		m_scriptInterface->releaseScriptEnv();
		return (result == LUA_TRUE);
	}
	else
	{
		std::cout << "[Error] Call stack overflow. Action::executeUse" << std::endl;
		return false;
	}
}

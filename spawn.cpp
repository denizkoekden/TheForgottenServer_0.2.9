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

#include "spawn.h"
#include "game.h"
#include "player.h"
#include "npc.h"
#include "tools.h"
#include "configmanager.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h> 

extern ConfigManager g_config;
extern Monsters g_monsters;
extern Game g_game;

#define DEFAULTSPAWN_INTERVAL 60000

Spawns::Spawns()
{
	loaded = false;
	started = false;
	filename = "";
}

Spawns::~Spawns()
{
	clear();
}

bool Spawns::loadFromXml(const std::string& _filename)
{
	if(isLoaded()){
		return true;
	}
	
	filename = _filename;
	
	xmlDocPtr doc = xmlParseFile(filename.c_str());

	if(doc){
		xmlNodePtr root, spawnNode;
		root = xmlDocGetRootElement(doc);
		
		if(xmlStrcmp(root->name,(const xmlChar*)"spawns") != 0){
			xmlFreeDoc(doc);
			return false;
		}

		int32_t intValue;
		std::string strValue;

		spawnNode = root->children;
		while(spawnNode){
			if(xmlStrcmp(spawnNode->name, (const xmlChar*)"spawn") == 0){
				Position centerPos;
				int32_t radius = -1;

				if(readXMLInteger(spawnNode, "centerx", intValue)){
					centerPos.x = intValue;
				}
				else{
					xmlFreeDoc(doc);
					return false;
				}

				if(readXMLInteger(spawnNode, "centery", intValue)){
					centerPos.y = intValue;
				}
				else{
					xmlFreeDoc(doc);
					return false;
				}

				if(readXMLInteger(spawnNode, "centerz", intValue)){
					centerPos.z = intValue;
				}
				else{
					xmlFreeDoc(doc);
					return false;
				}

				if(readXMLInteger(spawnNode, "radius", intValue)){
					radius = intValue;
				}
				else{
					xmlFreeDoc(doc);
					return false;
				}

				Spawn* spawn = new Spawn(centerPos, radius);
				spawnList.push_back(spawn);

				xmlNodePtr tmpNode = spawnNode->children;
				while(tmpNode){
					if(xmlStrcmp(tmpNode->name, (const xmlChar*)"monster") == 0){

						std::string name = "";
						Position pos = centerPos;
						Direction dir = NORTH;
						uint32_t interval = 0;

						if(readXMLString(tmpNode, "name", strValue)){
							name = strValue;
						}
						else{
							tmpNode = tmpNode->next;
							continue;
						}

						if(readXMLInteger(tmpNode, "direction", intValue)){
							switch(intValue){
								case 0: dir = NORTH; break;
								case 1: dir = EAST; break;
								case 2: dir = SOUTH; break;
								case 3: dir = WEST; break;
							}
						}

						if(readXMLInteger(tmpNode, "x", intValue)){
							pos.x += intValue;
						}
						else{
							tmpNode = tmpNode->next;
							continue;
						}

						if(readXMLInteger(tmpNode, "y", intValue)){
							pos.y += intValue;
						}
						else{
							tmpNode = tmpNode->next;
							continue;
						}

						if(readXMLInteger(tmpNode, "spawntime", intValue) || readXMLInteger(tmpNode, "interval", intValue)){
							interval = intValue * 1000;
						}
						else{
							tmpNode = tmpNode->next;
							continue;
						}

						spawn->addMonster(name, pos, dir, interval);
					}
					else if(xmlStrcmp(tmpNode->name, (const xmlChar*)"npc") == 0){

						Direction direction = NORTH;
						std::string name = "";
						Position placePos = centerPos;

						if(readXMLString(tmpNode, "name", strValue)){
							name = strValue;
						}
						else{
							tmpNode = tmpNode->next;
							continue;
						}
						
						if(readXMLInteger(tmpNode, "direction", intValue)){
							switch(intValue){
								case 0: direction = NORTH; break;
								case 1: direction = EAST; break;
								case 2: direction = SOUTH; break;
								case 3: direction = WEST; break;
							}
						}

						if(readXMLInteger(tmpNode, "x", intValue)){
							placePos.x += intValue;
						}
						else{
							tmpNode = tmpNode->next;
							continue;
						}

						if(readXMLInteger(tmpNode, "y", intValue)){
							placePos.y += intValue;
						}
						else{
							tmpNode = tmpNode->next;
							continue;
						}
						
						Npc* npc = new Npc(name);
						if(!npc->isLoaded()){
							delete npc;

							tmpNode = tmpNode->next;
							continue;
						}
						
						npc->setDirection(direction);
						npc->setMasterPos(placePos, radius);

						// Place the npc
						if(!g_game.placeCreature(npc, placePos)){
							delete npc;

							tmpNode = tmpNode->next;
							continue;
						}
					}

					tmpNode = tmpNode->next;
				}
			}

			spawnNode = spawnNode->next;
		}

		xmlFreeDoc(doc);
		loaded = true;
		return true;
	}
	
	return false;
}

void Spawns::startup()
{
	if(!isLoaded() || isStarted() || spawnList.empty())
		return;

	for(SpawnList::iterator it = spawnList.begin(); it != spawnList.end(); ++it){
		(*it)->startup();
	}

	started = true;
}

void Spawns::clear()
{
	for(SpawnList::iterator it= spawnList.begin(); it != spawnList.end(); ++it){
		delete (*it);
	}

	spawnList.clear();

	loaded = false;
	started = false;
	filename = "";
}

bool Spawns::isInZone(const Position& centerPos, int32_t radius, const Position& pos)
{
	return ((pos.x >= centerPos.x - radius) && (pos.x <= centerPos.x + radius) &&
		(pos.y >= centerPos.y - radius) && (pos.y <= centerPos.y + radius));
}

void Spawns::respawn()
{
	for(SpawnList::iterator it= spawnList.begin(); it != spawnList.end(); ++it)
		(*it)->respawn();
}

void Spawn::respawn()
{
	Monster* monster;
	for(SpawnedMap::iterator it = spawnedMap.begin(); it != spawnedMap.end(); ++it)
	{
		monster = it->second;
		if(monster->isRemoved())
			monster->releaseThing2();
		g_game.removeCreature(monster, false);
	}
	spawnedMap.clear();

	uint32_t spawnCount;
	for(SpawnMap::iterator it = spawnMap.begin(); it != spawnMap.end(); ++it)
	{
		spawnCount = 0;
		spawnBlock_t& sb = it->second;
		spawnMonster(it->first, sb.mType, sb.pos, sb.direction);
		++spawnCount;
		if(spawnCount >= (uint32_t)g_config.getNumber(ConfigManager::RATE_SPAWN))
			break;
	}
}

void Spawn::startSpawnCheck()
{
	if(checkSpawnEvent == 0)
		checkSpawnEvent = Scheduler::getScheduler().addEvent(createSchedulerTask(getInterval(), boost::bind(&Spawn::checkSpawn, this)));
}

Spawn::Spawn(const Position& _pos, int32_t _radius)
{
	centerPos = _pos;
	radius = _radius;
	interval = DEFAULTSPAWN_INTERVAL;
	checkSpawnEvent = 0;
}

Spawn::~Spawn()
{
	Monster* monster;
	for(SpawnedMap::iterator it = spawnedMap.begin(); it != spawnedMap.end(); ++it){
		monster = it->second;

		if(monster->isRemoved()){
			monster->releaseThing2();
		}
		monster->setSpawn(NULL);
	}

	spawnedMap.clear();
	spawnMap.clear();

	stopEvent();
}

bool Spawn::findPlayer(const Position& pos)
{
	SpectatorVec list;
	SpectatorVec::iterator it;
	g_game.getSpectators(list, pos, true);

	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it) {
		if((tmpPlayer = (*it)->getPlayer()) && !tmpPlayer->hasFlag(PlayerFlag_IgnoredByMonsters))
			return true;
	}

	return false;
}

bool Spawn::isInSpawnZone(const Position& pos)
{
	return Spawns::getInstance()->isInZone(centerPos, radius, pos);
}

bool Spawn::spawnMonster(uint32_t spawnId, MonsterType* mType, const Position& pos, Direction dir)
{
	Monster* monster = Monster::createMonster(mType);
	if(!monster){
		return false;
	}

	if(g_game.getGameState() == GAME_STATE_STARTUP){
		//No need to send out events to the surrounding since there is no one out there to listen!
		if(!g_game.internalPlaceCreature(monster, pos, true)){
			delete monster;
			return false;
		}
	}
	else{
		if(!g_game.placeCreature(monster, pos, true)){
			delete monster;
			return false;
		}
	}

	monster->setDirection(dir);
	monster->setSpawn(this);
	monster->setMasterPos(pos, radius);
	monster->useThing2();

	spawnedMap.insert(spawned_pair(spawnId, monster));
	spawnMap[spawnId].lastSpawn = OTSYS_TIME();
	return true;
}

void Spawn::startup()
{
	for(SpawnMap::iterator it = spawnMap.begin(); it != spawnMap.end(); ++it)
	{
		uint32_t spawnId = it->first;
		spawnBlock_t& sb = it->second;

		spawnMonster(spawnId, sb.mType, sb.pos, sb.direction);
	}
}

void Spawn::checkSpawn()
{
#ifdef __DEBUG_SPAWN__
	std::cout << "[Notice] Spawn::checkSpawn " << this << std::endl;
#endif
	checkSpawnEvent = 0;

	Monster* monster;
	uint32_t spawnId;

	for(SpawnedMap::iterator it = spawnedMap.begin(); it != spawnedMap.end();){
		spawnId = it->first;
		monster = it->second;

		if(monster->isRemoved()) {
			if(spawnId != 0) {
				spawnMap[spawnId].lastSpawn = OTSYS_TIME();
			}

			monster->releaseThing2();
			spawnedMap.erase(it++);
		}
		else if(!isInSpawnZone(monster->getPosition()) && spawnId != 0) {
			spawnedMap.insert(spawned_pair(0, monster));
			spawnedMap.erase(it++);
		}
		else{
			++it;
		}
	}
	
	uint32_t spawnCount = 0;
	for(SpawnMap::iterator it = spawnMap.begin(); it != spawnMap.end(); ++it) {
		spawnId = it->first;
		spawnBlock_t& sb = it->second;

		if(spawnedMap.count(spawnId) == 0){
			if(OTSYS_TIME() >= sb.lastSpawn + sb.interval){

				if(findPlayer(sb.pos)){
					sb.lastSpawn = OTSYS_TIME();
					continue;
				}

				spawnMonster(spawnId, sb.mType, sb.pos, sb.direction);

				++spawnCount;
				if(spawnCount >= (uint32_t)g_config.getNumber(ConfigManager::RATE_SPAWN)){
					break;
				}
			}
		}
	}

	if(spawnedMap.size() < spawnMap.size()){
		checkSpawnEvent = Scheduler::getScheduler().addEvent(createSchedulerTask(getInterval(), boost::bind(&Spawn::checkSpawn, this)));
	}
#ifdef __DEBUG_SPAWN__
	else{
		std::cout << "[Notice] Spawn::checkSpawn stopped " << this << std::endl;
	}
#endif
}

bool Spawn::addMonster(const std::string& _name, const Position& _pos, Direction _dir, uint32_t _interval)
{
	MonsterType* mType = g_monsters.getMonsterType(_name);
	if(!mType){
		std::cout << "[Spawn::addMonster] Can not find " << _name << std::endl;
		return false;
	}
	
	if(_interval < interval){
		interval = _interval;
	}

	spawnBlock_t sb;
	sb.mType = mType;
	sb.pos = _pos;
	sb.direction = _dir;
	sb.interval = _interval;
	sb.lastSpawn = 0;

	uint32_t spawnId = (int32_t)spawnMap.size() + 1;
	spawnMap[spawnId] = sb;

	return true;
}

void Spawn::removeMonster(Monster* monster)
{
	for(SpawnedMap::iterator it = spawnedMap.begin(); it != spawnedMap.end(); ++it){
		if(it->second == monster){
			monster->releaseThing2();
			spawnedMap.erase(it);
			break;
		}
	}
}

void Spawn::stopEvent()
{
	if(checkSpawnEvent != 0){
		Scheduler::getScheduler().stopEvent(checkSpawnEvent);
		checkSpawnEvent = 0;
	}
}

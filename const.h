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

#ifndef __OTSERV_CONST_H__
#define __OTSERV_CONST_H__

#include "definitions.h"

#define NETWORKMESSAGE_MAXSIZE 15360

enum MagicEffectClasses
{
	NM_ME_DRAW_BLOOD	= 0x00,
	NM_ME_LOSE_ENERGY	= 0x01,
	NM_ME_POFF		= 0x02,
	NM_ME_BLOCKHIT		= 0x03,
	NM_ME_EXPLOSION_AREA	= 0x04,
	NM_ME_EXPLOSION_DAMAGE	= 0x05,
	NM_ME_FIRE_AREA		= 0x06,
	NM_ME_YELLOW_RINGS	= 0x07,
	NM_ME_POISON_RINGS	= 0x08,
	NM_ME_HIT_AREA		= 0x09,
	NM_ME_ENERGY_AREA      = 0x0A, //10
	NM_ME_ENERGY_DAMAGE    = 0x0B, //11
	NM_ME_MAGIC_ENERGY     = 0x0C, //12
	NM_ME_MAGIC_BLOOD      = 0x0D, //13
	NM_ME_MAGIC_POISON     = 0x0E, //14
	NM_ME_HITBY_FIRE       = 0x0F, //15
	NM_ME_POISON           = 0x10, //16
	NM_ME_MORT_AREA        = 0x11, //17
	NM_ME_SOUND_GREEN      = 0x12, //18
	NM_ME_SOUND_RED        = 0x13, //19
	NM_ME_POISON_AREA      = 0x14, //20
	NM_ME_SOUND_YELLOW     = 0x15, //21
	NM_ME_SOUND_PURPLE     = 0x16, //22
	NM_ME_SOUND_BLUE       = 0x17, //23
	NM_ME_SOUND_WHITE      = 0x18, //24
	NM_ME_BUBBLES	       = 0x19, //25
	NM_ME_CRAPS            = 0x1A, //26
	NM_ME_GIFT_WRAPS       = 0x1B, //27
	NM_ME_FIREWORK_YELLOW  = 0x1C, //28
	NM_ME_FIREWORK_RED     = 0x1D, //29
	NM_ME_FIREWORK_BLUE    = 0x1E, //30
	NM_ME_STUN	       = 0x1F, //31
	NM_ME_SLEEP	       = 0x20, //32
	NM_ME_WATERCREATURE    = 0x21, //33
	NM_ME_GROUNDSHAKER     = 0x22, //34

	//for internal use, dont send to client
	NM_ME_NONE             = 0xFF,
	NM_ME_UNK              = 0xFFFF
};

enum ShootType_t
{
	NM_SHOOT_SPEAR = 0x00,
	NM_SHOOT_BOLT = 0x01,
	NM_SHOOT_ARROW = 0x02,
	NM_SHOOT_FIRE = 0x03,
	NM_SHOOT_ENERGY = 0x04,
	NM_SHOOT_POISONARROW = 0x05,
	NM_SHOOT_BURSTARROW = 0x06,
	NM_SHOOT_THROWINGSTAR = 0x07,
	NM_SHOOT_THROWINGKNIFE = 0x08,
	NM_SHOOT_SMALLSTONE = 0x09,
	NM_SHOOT_SUDDENDEATH = 0x0A, //10
	NM_SHOOT_LARGEROCK = 0x0B, //11
	NM_SHOOT_SNOWBALL = 0x0C, //12
	NM_SHOOT_POWERBOLT = 0x0D, //13
	NM_SHOOT_POISONFIELD = 0x0E, //14
	NM_SHOOT_INFERNALBOLT = 0x0F, //15
	NM_SHOOT_HUNTINGSPEAR = 0x10, //16
	NM_SHOOT_ENCHANTEDSPEAR = 0x11, //17
	NM_SHOOT_REDSTAR = 0x12, //18
	NM_SHOOT_GREENSTAR = 0x13, //19
	NM_SHOOT_ROYALSPEAR = 0x14, //20
	NM_SHOOT_SNIPERARROW = 0x15, //21
	NM_SHOOT_ONYXARROW = 0x16, //22
	NM_SHOOT_PIERCINGBOLT = 0x17, //23
	NM_SHOOT_WHIRLWINDSWORD = 0x18, //24
	NM_SHOOT_WHIRLWINDAXE = 0x19, //25
	NM_SHOOT_WHIRLWINDCLUB = 0x1A, //26
	NM_SHOOT_ETHEREALSPEAR = 0x1B, //27
	
	//for internal use, dont send to client
	NM_SHOOT_WEAPONTYPE = 0xFE, //254
	NM_SHOOT_NONE = 0xFF, //255
	NM_SHOOT_UNK = 0xFFFF
};

enum SpeakClasses
{
	SPEAK_SAY		= 0x01,
	SPEAK_WHISPER		= 0x02,
	SPEAK_YELL		= 0x03,
	SPEAK_PRIVATE		= 0x04,
	SPEAK_CHANNEL_Y		= 0x05,
	SPEAK_CHANNEL_RV1	= 0x06,
	SPEAK_CHANNEL_RV2	= 0x07,
	SPEAK_CHANNEL_RV3	= 0x08,
	SPEAK_BROADCAST		= 0x09,
	SPEAK_CHANNEL_R1	= 0x0A, //red - #c text
	SPEAK_PRIVATE_RED	= 0x0B,	//@name@text
	SPEAK_CHANNEL_O		= 0x0C,
	SPEAK_UNKNOWN_1		= 0x0D,
	SPEAK_CHANNEL_R2	= 0x0E,	//red anonymous - #d text
	SPEAK_UNKNOWN_2		= 0x0F,
	SPEAK_MONSTER_SAY	= 0x10,
	SPEAK_MONSTER_YELL	= 0x11
};

enum MessageClasses
{
	MSG_CLASS_FIRST			= 0x11,
	MSG_STATUS_CONSOLE_ORANGE	= MSG_CLASS_FIRST, /*Orange message in the console*/
	MSG_STATUS_WARNING		= 0x12, /*Red message in game window and in the console*/
	MSG_EVENT_ADVANCE		= 0x13, /*White message in game window and in the console*/
	MSG_EVENT_DEFAULT		= 0x14, /*White message at the bottom of the game window and in the console*/
	MSG_STATUS_DEFAULT		= 0x15, /*White message at the bottom of the game window and in the console*/
	MSG_INFO_DESCR			= 0x16, /*Green message in game window and in the console*/
	MSG_STATUS_SMALL		= 0x17, /*White message at the bottom of the game window"*/
	MSG_STATUS_CONSOLE_BLUE		= 0x18, /*Blue message in the console*/
	MSG_STATUS_CONSOLE_RED		= 0x19, /*Red message in the console*/
	MSG_CLASS_LAST			= MSG_STATUS_CONSOLE_RED
};

enum FluidColors
{
	FLUID_EMPTY	= 0x00,
	FLUID_BLUE	= 0x01,
	FLUID_RED	= 0x02,
	FLUID_BROWN	= 0x03,
	FLUID_GREEN	= 0x04,
	FLUID_YELLOW	= 0x05,
	FLUID_WHITE	= 0x06,
	FLUID_PURPLE	= 0x07
};

enum FluidClasses
{
	FLUID_EMPTY_1	= 0x00,
	FLUID_BLUE_1	= 0x01,
	FLUID_PURPLE_1	= 0x02,
	FLUID_BROWN_1	= 0x03,
	FLUID_BROWN_2	= 0x04,
	FLUID_RED_1	= 0x05,
	FLUID_GREEN_1	= 0x06,
	FLUID_BROWN_3	= 0x07,
	FLUID_YELLOW_1	= 0x08,
	FLUID_WHITE_1	= 0x09,
	FLUID_PURPLE_2	= 0x0A,
	FLUID_RED_2	= 0x0B,
	FLUID_YELLOW_2	= 0x0C,
	FLUID_BROWN_4	= 0x0D,
	FLUID_YELLOW_3	= 0x0E,
	FLUID_WHITE_2	= 0x0F,
	FLUID_BLUE_2	= 0x10,
};

enum e_fluids
{
	FLUID_WATER	= FLUID_BLUE,
	FLUID_BLOOD	= FLUID_RED,
	FLUID_BEER	= FLUID_BROWN,
	FLUID_SLIME	= FLUID_GREEN,
	FLUID_LEMONADE	= FLUID_YELLOW,
	FLUID_MILK	= FLUID_WHITE,
	FLUID_MANAFLUID	= FLUID_PURPLE,
	FLUID_LIFEFLUID	= FLUID_RED,
	FLUID_OIL	= FLUID_BROWN,
	FLUID_WINE	= FLUID_PURPLE,
};

const uint32_t reverseFluidMap[] =
{
	FLUID_EMPTY_1,
	FLUID_WATER,
	FLUID_MANAFLUID,
	FLUID_BEER, 
	FLUID_EMPTY_1,
	FLUID_BLOOD,
	FLUID_SLIME,
	FLUID_EMPTY_1,
	FLUID_LEMONADE,
	FLUID_MILK
};

const uint32_t fluidMap[] =
{
	FLUID_EMPTY_1,
	FLUID_BLUE_1,
	FLUID_RED_1,
	FLUID_BROWN_1, 
	FLUID_GREEN_1,
	FLUID_YELLOW_1,
	FLUID_WHITE_1,
	FLUID_PURPLE_1
};

const uint16_t isFreeHorizontalBed[] =
{
	1756,
	1760,
	3838,
	5500
};

const uint16_t isTakenHorizontalBed[] =
{
	1768,
	1764,
	3842,
	5498
};

const uint16_t isFreeVerticalBed[] =
{
	1754,
	1758,
	3836,
	5502
};

const uint16_t isTakenVerticalBed[] =
{
	1762,
	1766,
	3840,
	5496
};

enum WalkToType_t
{
	ACTION_NONE,
	ACTION_USE,
	ACTION_USEEX,
	ACTION_MOVE,
	ACTION_TRADE,
	ACTION_RORATE
};

enum SquareColor_t
{
	SQ_COLOR_NONE = 256,
	SQ_COLOR_BLACK = 0,
};

enum TextColor_t
{
	TEXTCOLOR_BLUE        = 5,
	TEXTCOLOR_LIGHTBLUE   = 35,
	TEXTCOLOR_LIGHTGREEN  = 30,
	TEXTCOLOR_LIGHTGREY   = 129,
	TEXTCOLOR_RED         = 180,
	TEXTCOLOR_ORANGE      = 198,
	TEXTCOLOR_WHITE_EXP   = 215,
	TEXTCOLOR_NONE        = 255
};

enum Icons_t
{
	ICON_POISON = 1,
	ICON_BURN = 2, 
	ICON_ENERGY =  4, 
	ICON_DRUNK = 8, 
	ICON_MANASHIELD = 16, 
	ICON_PARALYZE = 32, 
	ICON_HASTE = 64, 
	ICON_SWORDS = 128,
	ICON_DROWNING = 256
};

enum WeaponType_t
{
	WEAPON_NONE = 0,
	WEAPON_SWORD = 1,
	WEAPON_CLUB = 2,
	WEAPON_AXE = 3,
	WEAPON_SHIELD = 4,
	WEAPON_DIST = 5,
	WEAPON_WAND = 6,
	WEAPON_AMMO = 7
};

enum Ammo_t
{
	AMMO_NONE = 0,
	AMMO_BOLT = 1,
	AMMO_ARROW = 2,
	AMMO_SPEAR = 3,
	AMMO_THROWINGSTAR = 4,
	AMMO_THROWINGKNIFE = 5,
	AMMO_STONE = 6,
	AMMO_SNOWBALL = 7
};

enum AmmoAction_t
{
	AMMOACTION_NONE,
	AMMOACTION_REMOVECOUNT,
	AMMOACTION_REMOVECHARGE,
	AMMOACTION_MOVE,
	AMMOACTION_MOVEBACK
};

enum WieldInfo_t
{
	WIELDINFO_LEVEL = 1,
	WIELDINFO_MAGLV = 2,
	WIELDINFO_VOCREQ = 4,
	WIELDINFO_PREMIUM = 8
};

enum Skulls_t
{
	SKULL_NONE = 0,
	SKULL_YELLOW = 1,
	SKULL_GREEN = 2,
	SKULL_WHITE = 3,
	SKULL_RED = 4
};

enum Shields_t
{
	SHIELD_NONE = 0,
	SHIELD_INVITED = 1,
	SHIELD_HALF = 2,
	SHIELD_MEMBER = 3,
	SHIELD_LEADER = 4
};

enum item_t
{
	ITEM_FIREFIELD_PVP	= 1492,
	ITEM_FIREFIELD_NOPVP	= 1500,

	ITEM_POISONFIELD_PVP	= 1496,
	ITEM_POISONFIELD_NOPVP	= 1503,

	ITEM_ENERGYFIELD_PVP	= 1495,
	ITEM_ENERGYFIELD_NOPVP	= 1504,

	ITEM_COINS_GOLD		= 2148,
	ITEM_COINS_PLATINUM	= 2152,
	ITEM_COINS_CRYSTAL	= 2160,
	
	ITEM_DEPOT		= 2594,
	ITEM_LOCKER1		= 2589,
	
	ITEM_MALE_CORPSE	= 3058,
	ITEM_FEMALE_CORPSE	= 3065,
	
	ITEM_MEAT		= 2666,
	ITEM_HAM		= 2671,
	ITEM_GRAPE		= 2681,
	ITEM_APPLE		= 2674,
	ITEM_BREAD		= 2689,
	ITEM_ROLL		= 2690,
	ITEM_CHEESE		= 2696,

	ITEM_FULLSPLASH		= 2016,
	ITEM_SMALLSPLASH	= 2019,
	
	ITEM_PARCEL		= 2595,
	ITEM_PARCEL_STAMPED	= 2596,
	ITEM_LETTER		= 2597,
	ITEM_LETTER_STAMPED	= 2598,
	ITEM_LABEL		= 2599,
	
	ITEM_AMULETOFLOSS	= 2173,
	
	ITEM_DOCUMENT_RO	= 1968, //read-only
};

enum PlayerBlessings
{
	PlayerBlessing_First = 0,
	PlayerBlessing_Second,
	PlayerBlessing_Third,
	PlayerBlessing_Fourth,
	PlayerBlessing_Fifth
};

enum PlayerFlags
{
	PlayerFlag_CannotUseCombat = 0,
	PlayerFlag_CannotAttackPlayer,
	PlayerFlag_CannotAttackMonster,
	PlayerFlag_CannotBeAttacked,
	PlayerFlag_CanConvinceAll,
	PlayerFlag_CanSummonAll,
	PlayerFlag_CanIllusionAll,
	PlayerFlag_CanSenseInvisibility,
	PlayerFlag_IgnoredByMonsters,
	PlayerFlag_NotGainInFight,
	PlayerFlag_HasInfiniteMana,
	PlayerFlag_HasInfiniteSoul,
	PlayerFlag_HasNoExhaustion,
	PlayerFlag_CannotUseSpells,
	PlayerFlag_CannotPickupItem,
	PlayerFlag_CanAlwaysLogin,
	PlayerFlag_CanBroadcast,
	PlayerFlag_CanEditHouses,
	PlayerFlag_CannotBeBanned,
	PlayerFlag_CannotBePushed,
	PlayerFlag_HasInfiniteCapacity,
	PlayerFlag_CanPushAllCreatures,
	PlayerFlag_CanTalkRedPrivate,
	PlayerFlag_CanTalkRedChannel,
	PlayerFlag_TalkOrangeHelpChannel,
	PlayerFlag_NotGainExperience,
	PlayerFlag_NotGainMana,
	PlayerFlag_NotGainHealth,
	PlayerFlag_NotGainSkill,
	PlayerFlag_SetMaxSpeed,
	PlayerFlag_SpecialVIP,
	PlayerFlag_NotGenerateLoot,
	PlayerFlag_CanTalkRedChannelAnonymous,
	PlayerFlag_IgnoreProtectionZone,
	PlayerFlag_IgnoreSpellCheck,
	PlayerFlag_IgnoreWeaponCheck,
	PlayerFlag_CannotBeMuted,
	//add new flags here
	PlayerFlag_LastFlag
};

/*
enum GamemasterActions
{
	GamemasterAction_Notation = 1,
	GamemasterAction_NameReport = 2,
	GamemasterAction_Banishment = 4,
	GamemasterAction_NameReportBanishment = 8,
	GamemasterAction_BanishmentFinalWarning = 16,
	GamemasterAction_NameReportBanishment_Final_Warning = 32,
	GamemasterAction_StatementReport = 64,
	GamemasterAction_IpAdressBanishment = 128
};
*/

//Reserved player storage key ranges
//[10000000 - 20000000]
#define PSTRG_RESERVED_RANGE_START  10000000
#define PSTRG_RESERVED_RANGE_SIZE   10000000
//[1000 - 1500]
#define PSTRG_OUTFITS_RANGE_START   (PSTRG_RESERVED_RANGE_START + 1000)
#define PSTRG_OUTFITS_RANGE_SIZE    500
	
#define IS_IN_KEYRANGE(key, range) (key >= PSTRG_##range##_START && ((key - PSTRG_##range##_START) < PSTRG_##range##_SIZE))

#endif

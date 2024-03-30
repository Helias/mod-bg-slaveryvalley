#include "BattlegroundSV.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "GameGraveyard.h"
#include "GameObject.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "Transport.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include <unordered_map>

#include "Config.h"
#include "ScriptMgr.h"

// adding Battleground to the core battlegrounds list
BattlegroundTypeId BATTLEGROUND_SV = BattlegroundTypeId(31); // value from BattlemasterList.dbc
BattlegroundQueueTypeId BATTLEGROUND_QUEUE_SV = BattlegroundQueueTypeId(11);

BattlegroundSV::BattlegroundSV()
{
	BgObjects.resize(MAX_OBJECT_BANNER_SPAWNS + BG_SV_MAX_BUFF_SPAWNS);
	BgCreatures.resize(BG_SV_MAX_SPIRIT_GUIDES_SPAWNS + BG_SV_MAX_CREATURE_SPAWNS);

	StartMessageIds[BG_STARTING_EVENT_FIRST] = LANG_BG_SV_START_TWO_MINUTES;
	StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_SV_START_ONE_MINUTE;
	StartMessageIds[BG_STARTING_EVENT_THIRD] = LANG_BG_SV_START_HALF_MINUTE;
	StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_SV_HAS_BEGUN;

	// Setup nodes
	for (uint8 i = NODE_TYPE_FIX; i < BG_SV_MAX_NODE_TYPES; ++i)
		nodePoint[i] = BG_SV_nodePointInitial[i];

	//	Set score
	for (uint8 i = 0; i < 2; ++i)
		TeamScore[i] = sConfigMgr->GetOption<uint32>("InitialPoints", 80);

	// Set timer
	ReloadTimer = BG_SV_RELOAD_TIME;
	BossSpawnTimer = BG_SV_BOSS_SPAWN_TIMER;
	BossWarningTimer = BG_SV_BOSS_WARNING;
}

BattlegroundSV::~BattlegroundSV() {}

// Set Node Buff on Resurrect
void BattlegroundSV::HandlePlayerResurrect(Player *player)
{
	if (nodePoint[NODE_TYPE_MINE].nodeState == (player->GetTeamId() == TEAM_ALLIANCE ? BG_SV_NODE_STATE_CONTROLLED_A : BG_SV_NODE_STATE_CONTROLLED_H))
		player->CastSpell(player, BG_SV_MINE_BUFF, true);

	if (nodePoint[NODE_TYPE_BOSS].nodeState == (player->GetTeamId() == TEAM_ALLIANCE ? BG_SV_NODE_STATE_CONTROLLED_A : BG_SV_NODE_STATE_CONTROLLED_H))
		player->CastSpell(player, BG_SV_BOSS_BUFF, true);
}

void BattlegroundSV::PostUpdateImpl(uint32 diff)
{
	if (GetStatus() != STATUS_IN_PROGRESS)
	{
		// roots target and update score for new players in preperation phase
		if (ReloadTimer <= diff)
		{
			for (BattlegroundPlayerMap::const_iterator i = GetPlayers().begin(); i != GetPlayers().end(); ++i)
				if (Player *player = ObjectAccessor::FindPlayer(i->first))
				{
					BG_SV_StartRoot(player);
				}
			UpdateWorldState(BG_SV_WS_ALLIANCE_SCORE, TeamScore[TEAM_ALLIANCE]);
			UpdateWorldState(BG_SV_WS_HORDE_SCORE, TeamScore[TEAM_HORDE]);
			ReloadTimer = BG_SV_RELOAD_TIME;
		}
		else
			ReloadTimer -= diff;
	}

	if (GetStatus() == STATUS_IN_PROGRESS)
	{
		for (uint8 i = NODE_TYPE_FIX; i < BG_SV_MAX_NODE_TYPES; ++i)
		{

			// the point is waiting for a change on its banner
			if (nodePoint[i].needChange)
			{
				// the point is waiting for a change on its banner
				if (nodePoint[i].needChange)
				{
					if (nodePoint[i].timer <= diff)
					{
						uint32 nextBanner = GetNextBanner(&nodePoint[i], nodePoint[i].faction, true);

						nodePoint[i].last_entry = nodePoint[i].gameobject_entry;
						nodePoint[i].gameobject_entry = nextBanner;
						// nodePoint[i].faction = the faction should be the same one...

						GameObject *banner = GetBGObject(nodePoint[i].gameobject_type);

						if (!banner) // this should never happen
							return;

						float cords[4] = {banner->GetPositionX(), banner->GetPositionY(), banner->GetPositionZ(), banner->GetOrientation()};

						DelObject(nodePoint[i].gameobject_type);
						AddObject(nodePoint[i].gameobject_type, nodePoint[i].gameobject_entry, cords[0], cords[1], cords[2], cords[3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

						GetBGObject(nodePoint[i].gameobject_type)->SetUInt32Value(GAMEOBJECT_FACTION, nodePoint[i].faction == TEAM_ALLIANCE ? BG_SV_Factions[1] : BG_SV_Factions[0]);

						BG_SV_UpdateNodeWorldState(&nodePoint[i]);
						BG_SV_HandleCapturedNodes(&nodePoint[i], false);

						SendMessage2ToAll(LANG_BG_SV_TAKEN, nodePoint[i].faction == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, NULL, (nodePoint[i].faction == TEAM_ALLIANCE ? LANG_BG_SV_ALLY : LANG_BG_SV_HORDE), nodePoint[i].string);
						PlaySoundToAll(nodePoint[i].faction == TEAM_ALLIANCE ? BG_SV_SOUND_NODE_CAPTURED_ALLIANCE : BG_SV_SOUND_NODE_CAPTURED_HORDE);

						nodePoint[i].needChange = false;
						nodePoint[i].timer = BG_SV_BANNER_STATE_CHANGE_TIME;
					}
					else
						nodePoint[i].timer -= diff;
				}
			}
		}

		// updates Score and Buff while bg is running
		if (ReloadTimer <= diff)
		{
			UpdateWorldStates();
			ReloadTimer = BG_SV_RELAOD_TIME_INPROGRESS;
		}
		else
			ReloadTimer -= diff;

		// spawn Bosses after 5 minutes | Warning after 4 minutes
		if (BossSpawnAllowed == true)
		{
			if (BossSpawnTimer <= diff)
			{
				SpawnBoss();
				SendMessage2ToAll(LANG_BG_SV_BOSS_INC_NOW, CHAT_MSG_BG_SYSTEM_NEUTRAL, NULL);
				PlaySoundToAll(BG_SV_SOUND_BOSS_INC);
				BossSpawnTimer = BG_SV_BOSS_SPAWN_TIMER;
				BossSpawnAllowed = false;
			}
			else
				BossSpawnTimer -= diff;

			if (BossWarningTimer <= diff)
			{
				SendMessage2ToAll(LANG_BG_SV_BOSS_INC_1MIN, CHAT_MSG_BG_SYSTEM_NEUTRAL, NULL);
				BossWarningTimer = BG_SV_BOSS_SPAWN_TIMER;
			}
			else
				BossWarningTimer -= diff;
		}
	}
}

// update score and shows world states by entering the battleground
void BattlegroundSV::StartingEventCloseDoors()
{
	// TeamScore[TEAM_ALLIANCE] -= 1;
	UpdateWorldState(BG_SV_WS_ALLIANCE_SCORE, TeamScore[TEAM_ALLIANCE]);
	// TeamScore[TEAM_HORDE] -= 1;
	UpdateWorldState(BG_SV_WS_HORDE_SCORE, TeamScore[TEAM_HORDE]);

	for (uint8 i = 0; i < 4; ++i)
	{
		BG_SV_UpdateNodeWorldState(&nodePoint[i]);
	}
}

// unroot the players and start Boss spawn timer
void BattlegroundSV::StartingEventOpenDoors()
{
	for (BattlegroundPlayerMap::const_iterator ir = GetPlayers().begin(); ir != GetPlayers().end(); ++ir)
		if (Player *player = ObjectAccessor::FindPlayer(ir->first))
		{
			BG_SV_RemoveRoot(player);
		}

	BossSpawnAllowed = true;
}

// done when a player enters the battleground while running
void BattlegroundSV::AddPlayer(Player *player)
{
    Battleground::AddPlayer(player);
    PlayerScores.emplace(player->GetGUID().GetCounter(), new BattlegroundSVScore(player->GetGUID()));

	UpdateWorldState(BG_SV_WS_HORDE_SCORE, TeamScore[TEAM_HORDE]);
	UpdateWorldState(BG_SV_WS_ALLIANCE_SCORE, TeamScore[TEAM_ALLIANCE]);

	for (uint8 i = 0; i < 4; ++i)
	{
		BG_SV_UpdateNodeWorldState(&nodePoint[i]);
	}

	if (nodePoint[NODE_TYPE_MINE].nodeState == (player->GetTeamId() == TEAM_ALLIANCE ? BG_SV_NODE_STATE_CONTROLLED_A : BG_SV_NODE_STATE_CONTROLLED_H))
		player->CastSpell(player, BG_SV_MINE_BUFF, true);

	if (nodePoint[NODE_TYPE_BOSS].nodeState == (player->GetTeamId() == TEAM_ALLIANCE ? BG_SV_NODE_STATE_CONTROLLED_A : BG_SV_NODE_STATE_CONTROLLED_H))
		player->CastSpell(player, BG_SV_BOSS_BUFF, true);
}

void BattlegroundSV::UpdateWorldStates()
{
	// updates score
	UpdateWorldState(BG_SV_WS_HORDE_SCORE, TeamScore[TEAM_HORDE]);
	UpdateWorldState(BG_SV_WS_ALLIANCE_SCORE, TeamScore[TEAM_ALLIANCE]);

	// updates worldstates on the map
	for (uint8 i = 0; i < 4; ++i)
	{
		BG_SV_UpdateNodeWorldState(&nodePoint[i]);
	}

	// add Node buffs and remove root
	for (BattlegroundPlayerMap::const_iterator ir = GetPlayers().begin(); ir != GetPlayers().end(); ++ir)
		if (Player *player = ObjectAccessor::FindPlayer(ir->first))
		{
			if (nodePoint[NODE_TYPE_MINE].nodeState == (player->GetTeamId() == TEAM_ALLIANCE ? BG_SV_NODE_STATE_CONTROLLED_A : BG_SV_NODE_STATE_CONTROLLED_H))
				player->CastSpell(player, BG_SV_MINE_BUFF, true);

			if (nodePoint[NODE_TYPE_BOSS].nodeState == (player->GetTeamId() == TEAM_ALLIANCE ? BG_SV_NODE_STATE_CONTROLLED_A : BG_SV_NODE_STATE_CONTROLLED_H))
				player->CastSpell(player, BG_SV_BOSS_BUFF, true);

			if (GetStatus() == STATUS_IN_PROGRESS)
			{
				player->RemoveAura(BG_SV_ROOT);
			}
		}
}

// done when player leaves the battleground
void BattlegroundSV::RemovePlayer(Player *player)
{
	player->RemoveAurasDueToSpell(BG_SV_BOSS_BUFF);
	player->RemoveAurasDueToSpell(BG_SV_MINE_BUFF);
}

// unusued
void BattlegroundSV::HandleAreaTrigger(Player * /* player */, uint32 /* trigger */)
{
	if (GetStatus() != STATUS_IN_PROGRESS)
		return;
}

void BattlegroundSV::FillInitialWorldStates(WorldPacket &data)
{
	// set score
	data << uint32(BG_SV_WS_ALLIANCE_SCORE) << uint32(TeamScore[TEAM_ALLIANCE]);
	data << uint32(BG_SV_WS_HORDE_SCORE) << uint32(TeamScore[TEAM_HORDE]);

	// set world states on the map
	for (uint8 i = 0; i < BG_SV_MAX_NODE_TYPES; ++i)
		data << uint32(nodePoint[i].worldStates[nodePoint[i].nodeState]) << uint32(1);
}

bool BattlegroundSV::SetupBattleground()
{
	// set false as default
	AllianceCaptured = false;
	HordeCaptured = false;

	for (uint8 i = 0; i < MAX_OBJECT_BANNER_SPAWNS; ++i)
	{
		if (!AddObject(BG_SV_ObjSpawnlocs[i].type, BG_SV_ObjSpawnlocs[i].entry, BG_SV_ObjSpawnlocs[i].x, BG_SV_ObjSpawnlocs[i].y, BG_SV_ObjSpawnlocs[i].z, BG_SV_ObjSpawnlocs[i].o, 0, 0, 0, 0, RESPAWN_ONE_DAY))
		{
			LOG_INFO("module", "Slavery Valley: There was an error spawning gameobject {}", BG_SV_ObjSpawnlocs[i].entry);
			return false;
		}
	}

	for (uint8 i = 0; i < 1; ++i)
	{
		if (!AddCreature(BG_SV_BossSpawnlocs[i].entry, BG_SV_BossSpawnlocs[i].type, BG_SV_BossSpawnlocs[i].x, BG_SV_BossSpawnlocs[i].y, BG_SV_BossSpawnlocs[i].z, BG_SV_BossSpawnlocs[i].o, /*BG_SV_BossSpawnlocs[i].team,*/ RESPAWN_ONE_DAY))
		{
			LOG_INFO("module", "Slavery Valley: There was an error spawning creature {}", BG_SV_BossSpawnlocs[i].entry);
			return false;
		}
	}

	for (uint8 i = 0; i < BG_SV_MAX_BUFF_SPAWNS; ++i)
	{
		if (!AddObject(BG_SV_BuffSpawnlocs[i].type, BG_SV_BuffSpawnlocs[i].entry, BG_SV_BuffSpawnlocs[i].x, BG_SV_BuffSpawnlocs[i].y, BG_SV_BuffSpawnlocs[i].z, BG_SV_BuffSpawnlocs[i].o, 0, 0, 0, 0, SPEED_BUFF_RESPAWN_TIME))
		{
			LOG_INFO("module", "Slavery Valley: There was an error spawning buff {}", BG_SV_BuffSpawnlocs[i].entry);
			return false;
		}
	}

	if (!AddSpiritGuide(BG_SV_NPC_SPIRIT_GUIDE_1 + 2, BG_SV_SpiritGuidePos[4].m_positionX, BG_SV_SpiritGuidePos[4].m_positionY, BG_SV_SpiritGuidePos[4].m_positionZ, BG_SV_SpiritGuidePos[4].m_orientation, TEAM_ALLIANCE) || !AddSpiritGuide(BG_SV_NPC_SPIRIT_GUIDE_1 + 3, BG_SV_SpiritGuidePos[5].m_positionX, BG_SV_SpiritGuidePos[5].m_positionY, BG_SV_SpiritGuidePos[5].m_positionZ, BG_SV_SpiritGuidePos[5].m_orientation, TEAM_HORDE))
	{
		LOG_INFO("module", "Slavery Valley: Failed to spawn initial spirit guide!");
		return false;
	}

	return true;
}

void BattlegroundSV::SpawnBoss()
{
	if (BgCreatures[BG_SV_NPC_BOSS_A])
		DelCreature(BG_SV_NPC_BOSS_A);

	if (BgCreatures[BG_SV_NPC_BOSS_H])
		DelCreature(BG_SV_NPC_BOSS_H);

	for (uint8 i = 1; i < BG_SV_MAX_CREATURE_SPAWNS; ++i)
	{
		if (!AddCreature(BG_SV_BossSpawnlocs[i].entry, BG_SV_BossSpawnlocs[i].type, BG_SV_BossSpawnlocs[i].x, BG_SV_BossSpawnlocs[i].y, BG_SV_BossSpawnlocs[i].z, BG_SV_BossSpawnlocs[i].o, /*BG_SV_BossSpawnlocs[i].team,*/ RESPAWN_ONE_DAY))
		{
			LOG_INFO("module", "Slavery Valley: There was an error spawning creature {}", BG_SV_BossSpawnlocs[i].entry);
		}
	}
}

void BattlegroundSV::HandleKillUnit(Creature *unit, Player * /* killer */)
{
	if (GetStatus() != STATUS_IN_PROGRESS)
		return;

	// caused by killing a boss
	uint32 entry = unit->GetEntry();
	if (entry == BG_SV_ALLIANCE_BOSS)
	{
		if (nodePoint[NODE_TYPE_MINE].nodeState == BG_SV_NODE_STATE_CONTROLLED_H && nodePoint[NODE_TYPE_BOSS].nodeState == BG_SV_NODE_STATE_CONTROLLED_H && nodePoint[NODE_TYPE_PRISON].nodeState == BG_SV_NODE_STATE_CONTROLLED_H)
		{
			if (TeamScore[TEAM_ALLIANCE] < 25)
			{
				TeamScore[TEAM_ALLIANCE] -= TeamScore[TEAM_ALLIANCE];
			}
			else
				TeamScore[TEAM_ALLIANCE] -= 25;
		}
		else if (TeamScore[TEAM_ALLIANCE] < 10)
		{
			TeamScore[TEAM_ALLIANCE] -= TeamScore[TEAM_ALLIANCE];
		}
		else
		{
			TeamScore[TEAM_ALLIANCE] -= 10;
		}

		DelCreature(BG_SV_NPC_BOSS_H);
		BossSpawnTimer = BG_SV_BOSS_SPAWN_TIMER;
		BossSpawnAllowed = true;
	}
	else if (entry == BG_SV_HORDE_BOSS)
	{
		if (nodePoint[NODE_TYPE_MINE].nodeState == BG_SV_NODE_STATE_CONTROLLED_A && nodePoint[NODE_TYPE_BOSS].nodeState == BG_SV_NODE_STATE_CONTROLLED_A && nodePoint[NODE_TYPE_PRISON].nodeState == BG_SV_NODE_STATE_CONTROLLED_A)
		{
			if (TeamScore[TEAM_HORDE] < 25)
			{
				TeamScore[TEAM_HORDE] -= TeamScore[TEAM_HORDE];
			}
			else
			{
				TeamScore[TEAM_HORDE] -= 25;
			}
		}
		else if (TeamScore[TEAM_HORDE] < 10)
		{
			TeamScore[TEAM_HORDE] -= TeamScore[TEAM_HORDE];
		}
		else
		{
			TeamScore[TEAM_HORDE] -= 10;
		}

		DelCreature(BG_SV_NPC_BOSS_A);
		BossSpawnTimer = BG_SV_BOSS_SPAWN_TIMER;
		BossSpawnAllowed = true;
	}

	UpdateWorldState((BG_SV_WS_ALLIANCE_SCORE), TeamScore[TEAM_ALLIANCE]);
	UpdateWorldState((BG_SV_WS_HORDE_SCORE), TeamScore[TEAM_HORDE]);

	if (TeamScore[TEAM_ALLIANCE] < 1)
		EndBattleground(TEAM_HORDE);
	if (TeamScore[TEAM_HORDE] < 1)
		EndBattleground(TEAM_ALLIANCE);
}

void BattlegroundSV::HandleKillPlayer(Player *player, Player *killer)
{
	if (GetStatus() != STATUS_IN_PROGRESS)
		return;

	// suicide gives no points
	if (killer->GetTeamId() == player->GetTeamId())
	{
		return;
	}

	Battleground::HandleKillPlayer(player, killer);

	if (killer->GetTeamId() == TEAM_ALLIANCE)
	{
		if (AllianceCaptured == true && TeamScore[player->GetTeamId()] > 1)
		{
			TeamScore[player->GetTeamId()] -= 2;
			UpdatePlayerScore(player, SCORE_KILLING_BLOWS, 2);
		}
		else
		{
			TeamScore[player->GetTeamId()] -= 1;
			UpdatePlayerScore(player, SCORE_KILLING_BLOWS, 1);
		}
	}

	if (killer->GetTeamId() == TEAM_HORDE)
	{
		if (HordeCaptured == true && TeamScore[player->GetTeamId()] > 1)
		{
			TeamScore[player->GetTeamId()] -= 2;
			UpdatePlayerScore(player, SCORE_KILLING_BLOWS, 2);
		}
		else
		{
			TeamScore[player->GetTeamId()] -= 1;
			UpdatePlayerScore(player, SCORE_KILLING_BLOWS, 1);
		}
	}

	UpdateWorldState((killer->GetTeamId() == TEAM_ALLIANCE ? BG_SV_WS_ALLIANCE_SCORE : BG_SV_WS_HORDE_SCORE), TeamScore[killer->GetTeamId()]);
	UpdateWorldState((player->GetTeamId() == TEAM_ALLIANCE ? BG_SV_WS_ALLIANCE_SCORE : BG_SV_WS_HORDE_SCORE), TeamScore[player->GetTeamId()]);

	// we must end the battleground
	if (TeamScore[player->GetTeamId()] < 1)
		EndBattleground(killer->GetTeamId());
}

void BattlegroundSV::EndBattleground(TeamId winner)
{
	RemoveAuraOnTeam(BG_SV_MINE_BUFF, TEAM_ALLIANCE);
	RemoveAuraOnTeam(BG_SV_MINE_BUFF, TEAM_HORDE);
	RemoveAuraOnTeam(BG_SV_BOSS_BUFF, TEAM_ALLIANCE);
	RemoveAuraOnTeam(BG_SV_BOSS_BUFF, TEAM_HORDE);

	RewardHonorToTeam(BG_SV_WINNER_HONOR_AMOUNT, winner == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);
	Battleground::EndBattleground(winner);
}

void BattlegroundSV::EventPlayerClickedOnFlag(Player *player, GameObject *target_obj)
{
	if (GetStatus() != STATUS_IN_PROGRESS)
		return;

	TeamId teamId = player->GetTeamId();

	// All the node points are iterated to find the clicked one
	for (uint8 i = 0; i < BG_SV_MAX_NODE_TYPES; ++i)
	{
		if (nodePoint[i].gameobject_entry == target_obj->GetEntry())
		{
			// THIS SHOULD NEVER HAPPEN
			if (nodePoint[i].faction == teamId)
				return;

			uint32 nextBanner = GetNextBanner(&nodePoint[i], teamId, false);

			// we set the new settings of the nodePoint
			nodePoint[i].faction = teamId;
			if (nodePoint[i].gameobject_entry == BG_SV_PRISON_BANNER || nodePoint[i].gameobject_entry == BG_SV_MINE_BANNER || nodePoint[i].gameobject_entry == BG_SV_BOSS_BANNER)
			{
				nodePoint[i].last_entry = 0;
			}
			else
				nodePoint[i].last_entry = nodePoint[i].gameobject_entry;
			nodePoint[i].gameobject_entry = nextBanner;

			// this is just needed if the next banner is grey
			if (nodePoint[i].banners[BG_SV_BANNER_A_CONTESTED] == nextBanner || nodePoint[i].banners[BG_SV_BANNER_H_CONTESTED] == nextBanner)
			{
				nodePoint[i].timer = BG_SV_BANNER_STATE_CHANGE_TIME; // 1 minute for last change (real faction banner)
				nodePoint[i].needChange = true;

				if (nodePoint[i].nodeType != NODE_TYPE_PRISON)
				{
					RelocateDeadPlayers(BgCreatures[BG_SV_NPC_SPIRIT_GUIDE_1 + nodePoint[i].nodeType - 2]);
				}

				// if we are here means that the point has been lost, or it is the first capture

				if (nodePoint[i].nodeType != NODE_TYPE_PRISON)
					if (BgCreatures[BG_SV_NPC_SPIRIT_GUIDE_1 + (nodePoint[i].nodeType) - 2])
						DelCreature(BG_SV_NPC_SPIRIT_GUIDE_1 + (nodePoint[i].nodeType) - 2);

				SendMessage2ToAll(LANG_BG_SV_ASSAULTED, teamId == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, player, nodePoint[i].string);
				PlaySoundToAll(nodePoint[i].faction == TEAM_ALLIANCE ? BG_SV_SOUND_NODE_ASSAULTED_ALLIANCE : BG_SV_SOUND_NODE_ASSAULTED_HORDE);
				BG_SV_HandleContestedNodes(&nodePoint[i]);
			}
			else if (nextBanner == nodePoint[i].banners[BG_SV_BANNER_A_CONTROLLED] || nextBanner == nodePoint[i].banners[BG_SV_BANNER_H_CONTROLLED]) // if we are going to spawn the definitve faction banner, we dont need the timer anymore
			{
				nodePoint[i].timer = BG_SV_BANNER_STATE_CHANGE_TIME;
				nodePoint[i].needChange = false;
				SendMessage2ToAll(LANG_BG_SV_DEFENDED, teamId == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, player, nodePoint[i].string);
				PlaySoundToAll(nodePoint[i].faction == TEAM_ALLIANCE ? BG_SV_SOUND_NODE_CAPTURED_ALLIANCE : BG_SV_SOUND_NODE_CAPTURED_HORDE);
				BG_SV_HandleCapturedNodes(&nodePoint[i], true);
			}

			GameObject *banner = GetBGObject(nodePoint[i].gameobject_type);

			if (!banner) // this should never happen
				return;

			float cords[4] = {banner->GetPositionX(), banner->GetPositionY(), banner->GetPositionZ(), banner->GetOrientation()};

			DelObject(nodePoint[i].gameobject_type);
			if (!AddObject(nodePoint[i].gameobject_type, nodePoint[i].gameobject_entry, cords[0], cords[1], cords[2], cords[3], 0, 0, 0, 0, RESPAWN_ONE_DAY))
			{
				LOG_INFO("module", "Slavery Valley: There was an error spawning a banner (type: {}, entry: {}). Slavery Valley BG cancelled.", nodePoint[i].gameobject_type, nodePoint[i].gameobject_entry);
				EndBattleground(TEAM_NEUTRAL);
			}

			GetBGObject(nodePoint[i].gameobject_type)->SetUInt32Value(GAMEOBJECT_FACTION, nodePoint[i].faction == TEAM_ALLIANCE ? BG_SV_Factions[1] : BG_SV_Factions[0]);

			BG_SV_UpdateNodeWorldState(&nodePoint[i]);
			// we dont need iterating if we are here
			// If the needChange bool was set true, we will handle the rest in the Update Map function.
			return;
		}
	}
}

void BattlegroundSV::BG_SV_UpdateNodeWorldState(BG_SV_NodePoint *node)
{
	// updating worldstate
	if (node->gameobject_entry == node->banners[BG_SV_BANNER_A_CONTROLLED])
		node->nodeState = BG_SV_NODE_STATE_CONTROLLED_A;
	else if (node->gameobject_entry == node->banners[BG_SV_BANNER_A_CONTESTED])
		node->nodeState = BG_SV_NODE_STATE_CONFLICT_A;
	else if (node->gameobject_entry == node->banners[BG_SV_BANNER_H_CONTROLLED])
		node->nodeState = BG_SV_NODE_STATE_CONTROLLED_H;
	else if (node->gameobject_entry == node->banners[BG_SV_BANNER_H_CONTESTED])
		node->nodeState = BG_SV_NODE_STATE_CONFLICT_H;

	uint32 worldstate = node->worldStates[node->nodeState];

	// with this we are sure we dont bug the client
	for (uint8 i = 0; i < 5; ++i)
	{
		if (node->worldStates[i] == worldstate)
			continue;
		UpdateWorldState(node->worldStates[i], 0);
	}

	UpdateWorldState(worldstate, 1);
}

uint32 BattlegroundSV::GetNextBanner(BG_SV_NodePoint *node, uint32 team, bool returnDefinitve)
{
	// this is only used in the update map function
	if (returnDefinitve)
		// here is a special case, here we must return the definitve faction banner after the grey banner was spawned 1 minute
		return node->banners[(team == TEAM_ALLIANCE ? BG_SV_BANNER_A_CONTROLLED : BG_SV_BANNER_H_CONTROLLED)];

	// there were no changes, this point has never been captured by any faction or at least clicked
	if (node->last_entry == 0)
		// 1 returns the CONTESTED ALLIANCE BANNER, 3 returns the HORDE one
		return node->banners[(team == TEAM_ALLIANCE ? BG_SV_BANNER_A_CONTESTED : BG_SV_BANNER_H_CONTESTED)];

	// If the actual banner is the definitive faction banner, we must return the grey banner of the player's faction
	if (node->gameobject_entry == node->banners[BG_SV_BANNER_A_CONTROLLED] || node->gameobject_entry == node->banners[BG_SV_BANNER_H_CONTROLLED])
		return node->banners[(team == TEAM_ALLIANCE ? BG_SV_BANNER_A_CONTESTED : BG_SV_BANNER_H_CONTESTED)];

	// If the actual banner is the grey faction banner, we must return the previous banner
	if (node->gameobject_entry == node->banners[BG_SV_BANNER_A_CONTESTED] || node->banners[BG_SV_BANNER_H_CONTESTED])
		return node->last_entry;

	// we should never be here...
	LOG_INFO("module", "Slavery Valley: Unexpected return in GetNextBanner function");
	return 0;
}

void BattlegroundSV::BG_SV_HandleContestedNodes(BG_SV_NodePoint *node)
{
	// nobody has this node
	if (node->nodeType == NODE_TYPE_PRISON)
	{
		AllianceCaptured = false;
		HordeCaptured = false;
	}
	node->captured = false;

	if (node->nodeType == NODE_TYPE_MINE)
	{
		RemoveAuraOnTeam(BG_SV_MINE_BUFF, TEAM_ALLIANCE);
		RemoveAuraOnTeam(BG_SV_MINE_BUFF, TEAM_HORDE);
	}

	if (node->nodeType == NODE_TYPE_BOSS)
	{
		RemoveAuraOnTeam(BG_SV_BOSS_BUFF, TEAM_ALLIANCE);
		RemoveAuraOnTeam(BG_SV_BOSS_BUFF, TEAM_HORDE);
	}
}

void BattlegroundSV::BG_SV_HandleCapturedNodes(BG_SV_NodePoint *node, bool /* recapture */)
{
	if (node->nodeType != NODE_TYPE_FIX && node->nodeType != NODE_TYPE_PRISON)
	{
		if (!AddSpiritGuide(BG_SV_NPC_SPIRIT_GUIDE_1 + node->nodeType - 2, BG_SV_SpiritGuidePos[node->nodeType].m_positionX, BG_SV_SpiritGuidePos[node->nodeType].m_positionY, BG_SV_SpiritGuidePos[node->nodeType].m_positionZ, BG_SV_SpiritGuidePos[node->nodeType].m_orientation, node->faction))
			LOG_INFO("module", "Slavery Valley Failed to spawn spirit guide! point: {}, team: {}, ", node->nodeType, node->faction);
	}

	if (node->nodeType == NODE_TYPE_PRISON)
	{
		if (node->faction == TEAM_ALLIANCE)
		{
			AllianceCaptured = true;
			HordeCaptured = false;
		}
		if (node->faction == TEAM_HORDE)
		{
			AllianceCaptured = false;
			HordeCaptured = true;
		}
	}

	node->captured = true;

	switch (node->gameobject_type)
	{
	case BG_SV_GO_MINE_BANNER:
		RemoveAuraOnTeam(BG_SV_MINE_BUFF, (node->faction == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE));
		CastSpellOnTeam(BG_SV_MINE_BUFF, (node->faction == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE));
		break;
	case BG_SV_GO_BOSS_BANNER:
		RemoveAuraOnTeam(BG_SV_BOSS_BUFF, (node->faction == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE));
		CastSpellOnTeam(BG_SV_BOSS_BUFF, (node->faction == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE));
		break;
	default:
		break;
	}
}

void BattlegroundSV::BG_SV_StartRoot(Player *player)
{
	// player position
	float plr_x = player->GetPositionX();
	float plr_y = player->GetPositionY();

	// root point alliance
	float RootPointX_A = -237.889f;
	float RootPointY_A = 1295.035f;

	// root point horde
	float RootPointX_H = 578.759f;
	float RootPointY_H = 1002.339f;

	// if players distance to the root point is lower than this he get rooted
	float RootDist = 27.5f * 27.5f; // form: (your distance you want to root)*(your distance you want to root)

	if (player->GetTeamId() == TEAM_ALLIANCE)
	{
		float distA = (RootPointX_A - plr_x) * (RootPointX_A - plr_x) + (RootPointY_A - plr_y) * (RootPointY_A - plr_y);

		if (player->HasAura(BG_SV_ROOT))
		{
			return;
		}
		else
		{
			if (distA < RootDist)
				player->CastSpell(player, BG_SV_ROOT, true);
		}
	}
	else if (player->GetTeamId() == TEAM_HORDE)
	{
		float distH = (RootPointX_H - plr_x) * (RootPointX_H - plr_x) + (RootPointY_H - plr_y) * (RootPointY_H - plr_y);

		if (player->HasAura(BG_SV_ROOT))
		{
			return;
		}
		else
		{
			if (distH < RootDist)
				player->CastSpell(player, BG_SV_ROOT, true);
		}
	}
}

// unused
void BattlegroundSV::BG_SV_RemoveRoot(Player *player)
{
	player->RemoveAura(BG_SV_ROOT);
}

GraveyardStruct const *BattlegroundSV::GetClosestGraveyard(Player *player)
{
	TeamId teamIndex = player->GetTeamId();

	// Is there any occupied node for this team?
	std::vector<uint8> nodes;
	for (uint8 i = 0; i < BG_SV_MAX_NODE_TYPES; ++i)
		if (nodePoint[i].captured == true && nodePoint[i].faction == teamIndex)
			nodes.push_back(i);

	GraveyardStruct const *entry = sGraveyard->GetGraveyard(BG_SV_GraveyardIds[teamIndex + BG_SV_MAX_NODE_TYPES]);
	GraveyardStruct const *nearestEntry = entry;

	// If so, select the closest node to place ghost on
	if (!nodes.empty())
	{
		float pX = player->GetPositionX();
		float pY = player->GetPositionY();
		float dist = (entry->x - pX) * (entry->x - pX) + (entry->y - pY) * (entry->y - pY);
		float minDist = dist;

		for (uint8 i = 0; i < nodes.size(); ++i)
		{
			entry = sGraveyard->GetGraveyard(BG_SV_GraveyardIds[nodes[i]]);

			if (!entry)
				continue;
			dist = (entry->x - pX) * (entry->x - pX) + (entry->y - pY) * (entry->y - pY);
			if (minDist > dist)
			{
				minDist = dist;
				nearestEntry = entry;
			}
		}
		//		nodes.clear();
	}

	return nearestEntry;
}

void AddSlaveryValleyScripts()
{
	// Add Slavery Valley to battleground list
	BattlegroundMgr::queueToBg[BATTLEGROUND_QUEUE_SV] = BATTLEGROUND_SV;
	BattlegroundMgr::bgToQueue[BATTLEGROUND_SV] = BATTLEGROUND_QUEUE_SV;
	BattlegroundMgr::bgtypeToBattleground[BATTLEGROUND_SV] = new BattlegroundSV;
	BattlegroundMgr::bgTypeToTemplate[BATTLEGROUND_SV] = [](Battleground *bg_t) -> Battleground * {
		return new BattlegroundSV(*(BattlegroundSV *)bg_t);
	};
}

void BattlegroundSVScore::BuildObjectivesBlock(WorldPacket& data)
{
    data << uint32(1); // Objectives Count
    data << uint32(KillingPoints);
}

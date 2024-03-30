
#ifndef __BATTLEGROUNDSV_H
#define __BATTLEGROUNDSV_H

#include "Battleground.h"
#include "Object.h"

const uint32 BG_SV_Factions[2] =
{
    1732, // Alliance
    1735  // Horde
};

enum SlaveryValleyStrings
{
    LANG_BG_SV_START_TWO_MINUTES                = 1500,
    LANG_BG_SV_START_ONE_MINUTE                 = 1501,
    LANG_BG_SV_START_HALF_MINUTE                = 1502,
    LANG_BG_SV_HAS_BEGUN                        = 1503,

    LANG_BG_SV_TAKEN                            = 3001,
    LANG_BG_SV_DEFENDED                         = 3002,
    LANG_BG_SV_ASSAULTED                        = 3003,
    LANG_BG_SV_ALLY                             = 3004,
    LANG_BG_SV_HORDE                            = 3005,
    LANG_BG_SV_MINE                             = 3006,
    LANG_BG_SV_PRISON                           = 3007,
    LANG_BG_SV_RESTLESSGRAVEYARD                = 3008,
    LANG_BG_SV_BOSS_INC_1MIN                    = 3009,
    LANG_BG_SV_BOSS_INC_NOW                     = 3010
};

enum BG_SV_Creatures
{
    BG_SV_DEVELOPER                             = 129999,
    BG_SV_ALLIANCE_BOSS                         = 130000,
    BG_SV_HORDE_BOSS                            = 130001,
};

enum BG_SV_Spells
{
    BG_SV_ROOT                                  = 85000,
    BG_SV_MINE_BUFF                             = 85001,
    BG_SV_BOSS_BUFF                             = 85002
};

enum BG_SV_Sounds
{
    BG_SV_SOUND_NODE_CAPTURED_ALLIANCE          = 8173,
    BG_SV_SOUND_NODE_CAPTURED_HORDE             = 8213,
    BG_SV_SOUND_NODE_ASSAULTED_ALLIANCE         = 8212,
    BG_SV_SOUND_NODE_ASSAULTED_HORDE            = 8174,
    BG_SV_SOUND_BOSS_INC                        = 8456
};

enum BG_SV_Objects
{
    BG_SV_FIX_BANNER                            = 195133,
    BG_SV_PRISON_BANNER                         = 195157,
    BG_SV_MINE_BANNER                           = 195158,
    BG_SV_BOSS_BANNER                           = 195338,

    BG_SV_FIX_BANNER_A                          = 195391,
    BG_SV_FIX_BANNER_A_CONT                     = 195392,
    BG_SV_FIX_BANNER_H                          = 195393,
    BG_SV_FIX_BANNER_H_CONT                     = 195394,

    BG_SV_PRISON_BANNER_A                       = 195153,
    BG_SV_PRISON_BANNER_A_CONT                  = 195154,
    BG_SV_PRISON_BANNER_H                       = 195155,
    BG_SV_PRISON_BANNER_H_CONT                  = 195156,

    BG_SV_MINE_BANNER_H                         = 195130,
    BG_SV_MINE_BANNER_H_CONT                    = 195145,
    BG_SV_MINE_BANNER_A                         = 195132,
    BG_SV_MINE_BANNER_A_CONT                    = 195144,

    BG_SV_BOSS_BANNER_A                         = 195334,
    BG_SV_BOSS_BANNER_A_CONT                    = 195335,
    BG_SV_BOSS_BANNER_H                         = 195336,
    BG_SV_BOSS_BANNER_H_CONT                    = 195337,
};

enum BG_SV_Timer
{
    BG_SV_BANNER_STATE_CHANGE_TIME              = 60000,
    BG_SV_RELOAD_TIME                           = 500,
    BG_SV_RELAOD_TIME_INPROGRESS                = 2000,
    BG_SV_BOSS_SPAWN_TIMER                      = 300000,
    BG_SV_BOSS_WARNING                          = 240000
};

struct BG_SV_Npc
{
    uint32 type;
    uint32 entry;
    TeamId team;
    float x;
    float y;
    float z;
    float o;
};

enum BG_SV_GOs
{
    BG_SV_GO_FIX_BANNER1 = 0,
    BG_SV_GO_PRISON_BANNER,
    BG_SV_GO_MINE_BANNER,
    BG_SV_GO_BOSS_BANNER,
    BG_SV_GO_SPEEDBUFF,
    BG_SV_GO_HEALBUFF1,
    BG_SV_GO_HEALBUFF2,
    BG_SV_GO_HEALBUFF3,
    BG_SV_GO_BERSERKBUFF1,
    BG_SV_GO_BERSERKBUFF2
};

enum BG_SV_NPCs
{
    BG_SV_NPC_DEVELOPER = 0,
    BG_SV_NPC_BOSS_A,
    BG_SV_NPC_BOSS_H,

    BG_SV_NPC_SPIRIT_GUIDE_1,
    BG_SV_NPC_SPIRIT_GUIDE_2,
    BG_SV_NPC_SPIRIT_GUIDE_3,
    BG_SV_NPC_SPIRIT_GUIDE_4,
};

enum BG_SV_BannersTypes
{
    BG_SV_BANNER_A_CONTROLLED,
    BG_SV_BANNER_A_CONTESTED,
    BG_SV_BANNER_H_CONTROLLED,
    BG_SV_BANNER_H_CONTESTED
};

enum BG_SV_Max_Spawn
{
    MAX_OBJECT_BANNER_SPAWNS = BG_SV_GO_BOSS_BANNER + 1,
    BG_SV_MAX_SPIRIT_GUIDES_SPAWNS = 4,
    BG_SV_MAX_CREATURE_SPAWNS = 3,
    BG_SV_MAX_BUFF_SPAWNS = 6,
};

struct BG_SV_Go
{
    uint32 type;
    uint32 entry;
    float x;
    float y;
    float z;
    float o;
};

const BG_SV_Go BG_SV_ObjSpawnlocs[MAX_OBJECT_BANNER_SPAWNS] =
{
    { BG_SV_GO_FIX_BANNER1,     BG_SV_FIX_BANNER,       0.0f,       0.0f,       0.0f,       0.0f    },  // FIX NODE
    { BG_SV_GO_PRISON_BANNER,   BG_SV_PRISON_BANNER,    197.982f,   1137.277f,  -21.65f,    6.22f   },  // Prison Banner
    { BG_SV_GO_MINE_BANNER,     BG_SV_MINE_BANNER,      296.73f,    1361.35f,   24.10011f,  4.38f   },  // Mine Banner
    { BG_SV_GO_BOSS_BANNER,     BG_SV_BOSS_BANNER,      69.62f,     908.35f,    -0.12f,     1.738f  },  // Boss Banner
};

const BG_SV_Go BG_SV_BuffSpawnlocs[BG_SV_MAX_BUFF_SPAWNS] =
{
    { BG_SV_GO_SPEEDBUFF,    BG_OBJECTID_SPEEDBUFF_ENTRY,     211.01f, 1056.53f, -34.21f, 1.540f    },  // Prison
    { BG_SV_GO_HEALBUFF1,    BG_OBJECTID_REGENBUFF_ENTRY,     211.68f, 1076.00f, -34.21f, 4.768f    },  // Prison
    { BG_SV_GO_HEALBUFF2,    BG_OBJECTID_REGENBUFF_ENTRY,     -41.48f, 927.47f,   -6.27f, 0.888f    },  // Boss A
    { BG_SV_GO_HEALBUFF3,    BG_OBJECTID_REGENBUFF_ENTRY,     240.55f, 856.54f,  -15.64f, 1.859f    },  // Boss H
    { BG_SV_GO_BERSERKBUFF1, BG_OBJECTID_BERSERKERBUFF_ENTRY, 393.63f, 1267.29f,  -1.94f, 3.885f    },  // Mine H
    { BG_SV_GO_BERSERKBUFF2, BG_OBJECTID_BERSERKERBUFF_ENTRY, 154.59f, 1409.01f,   3.96f, 4.113f    },  // Mine A
};

const BG_SV_Npc BG_SV_BossSpawnlocs[BG_SV_MAX_CREATURE_SPAWNS] =
{
    { BG_SV_NPC_DEVELOPER,  BG_SV_DEVELOPER,        TEAM_NEUTRAL,   225.83385f, 1059.995361f, -34.212475f, 5.603189f    },
    { BG_SV_NPC_BOSS_A,     BG_SV_ALLIANCE_BOSS,    TEAM_ALLIANCE,  214.816650f, 868.122192f, -14.181594f, 1.618849f    },
    { BG_SV_NPC_BOSS_H,     BG_SV_HORDE_BOSS,       TEAM_HORDE,     -26.488609f, 966.149109f, -10.802097f, 0.0f         }
};

// worldstate ID from WorldStateUI.dbc
enum BG_SV_Objectives
{
    BG_SV_OBJECTIVE_KILLING_POINTS = 297
};

// worldstate IDs from AreaPOI.dbc
enum BG_SV_WorldStates
{
    BG_SV_WS_ALLIANCE_SCORE                 = 6000,
    BG_SV_WS_HORDE_SCORE                    = 6001,

    BG_SV_PRISON                            = 5500,
    BG_SV_PRISON_A                          = 5501,
    BG_SV_PRISON_CON_A                      = 5502,
    BG_SV_PRISON_H                          = 5503,
    BG_SV_PRISON_CON_H                      = 5504,

    // Grave icon WorldStateUI
    BG_SV_BOSS                              = 5505,
    BG_SV_BOSS_CON_A                        = 5506,
    BG_SV_BOSS_CON_H                        = 5507,
    BG_SV_BOSS_A                            = 5508,
    BG_SV_BOSS_H                            = 5509,

    BG_SV_MINE                              = 5510,
    BG_SV_MINE_CON_A                        = 5511,
    BG_SV_MINE_CON_H                        = 5512,
    BG_SV_MINE_A                            = 5513,
    BG_SV_MINE_H                            = 5514,

};

enum BG_SV_NodePointType
{
    NODE_TYPE_FIX = 0,
    NODE_TYPE_PRISON,
    NODE_TYPE_MINE,
    NODE_TYPE_BOSS,

    BG_SV_MAX_NODE_TYPES
};

enum BG_SV_NodeState
{
    BG_SV_NODE_STATE_UNCONTROLLED = 0,
    BG_SV_NODE_STATE_CONFLICT_A,
    BG_SV_NODE_STATE_CONFLICT_H,
    BG_SV_NODE_STATE_CONTROLLED_A,
    BG_SV_NODE_STATE_CONTROLLED_H
};

const uint32 BG_SV_GraveyardIds[BG_SV_MAX_NODE_TYPES + 2] = {
    0,      // Fix
    0,      // Prison
    1742,   // Mine
    1744,   // Boss
    1741,   // Alliance
    1740    // Horde
};

Position const BG_SV_SpiritGuidePos[BG_SV_MAX_NODE_TYPES + 2] =
{
    { 0.0f,       0.0f,       0.0f,      0.0f    },                 // no grave Fix Node
    { 0.0f,       0.0f,       0.0f,      0.0f    },                 // no grave Prison
    { 291.59f,    1396.37f,   28.28f,    4.805f  },                 // Mine
    { 71.018f,    897.242f,   0.426f,    1.7954f },                 // Boss
    { -290.8553f, 1270.06f,   87.672f,   1.389f  },                 // Alliance Spirit
    { 654.96862f, 955.31970f, 43.06116f, 1.597f  },                 // Horde Spirit
};

Position const BG_SV_BossPos[2] =
{
    { 214.816650f, 868.122192f, -14.181594f, 1.618849f },           // Alliance Boss
    { -26.488609f, 966.149109f, -10.802097f, 0.0f      },           // Horde Boss
};

struct BG_SV_NodePoint
{
    uint32 gameobject_type;                                         // with this we will get the GameObject of that point
    uint32 gameobject_entry;                                        // what gameobject entry is active here.
    TeamId faction;                                                 // who has this node
    BG_SV_NodePointType nodeType;                                   // here we can specify if it is graveyards, hangar etc...
    uint32 banners[4];                                              // the banners that have this point
    bool needChange;                                                // this is used for the 1 minute time period after the point is captured
    uint32 timer;                                                   // the same use for needChange
    uint32 last_entry;                                              // the last gameobject_entry
    uint32 worldStates[5];                                          // the worldstates that represent the node in the map
    BG_SV_NodeState nodeState;                                      // which state has this node
    bool captured;                                                  // used for spirit guid to port only when the node is captured
    uint32 string;                                                  // Text which is written in the chat
};

const BG_SV_NodePoint BG_SV_nodePointInitial[4] =
{
    { BG_SV_GO_FIX_BANNER1,   BG_SV_FIX_BANNER,    TEAM_NEUTRAL, NODE_TYPE_BOSS,   { BG_SV_FIX_BANNER_A,    BG_SV_FIX_BANNER_A_CONT,    BG_SV_FIX_BANNER_H,    BG_SV_FIX_BANNER_H_CONT    }, false, 0, 0, { BG_SV_PRISON, BG_SV_PRISON_CON_A, BG_SV_PRISON_CON_H, BG_SV_PRISON_A, BG_SV_PRISON_H },  BG_SV_NODE_STATE_UNCONTROLLED, false, LANG_BG_SV_PRISON            },
    { BG_SV_GO_PRISON_BANNER, BG_SV_PRISON_BANNER, TEAM_NEUTRAL, NODE_TYPE_PRISON, { BG_SV_PRISON_BANNER_A, BG_SV_PRISON_BANNER_A_CONT, BG_SV_PRISON_BANNER_H, BG_SV_PRISON_BANNER_H_CONT }, false, 0, 0, { BG_SV_PRISON, BG_SV_PRISON_CON_A, BG_SV_PRISON_CON_H, BG_SV_PRISON_A, BG_SV_PRISON_H },  BG_SV_NODE_STATE_UNCONTROLLED, false, LANG_BG_SV_PRISON            },
    { BG_SV_GO_MINE_BANNER,   BG_SV_MINE_BANNER,   TEAM_NEUTRAL, NODE_TYPE_MINE,   { BG_SV_MINE_BANNER_A,   BG_SV_MINE_BANNER_A_CONT,   BG_SV_MINE_BANNER_H,   BG_SV_MINE_BANNER_H_CONT   }, false, 0, 0, { BG_SV_MINE,   BG_SV_MINE_CON_A,   BG_SV_MINE_CON_H,   BG_SV_MINE_A,   BG_SV_MINE_H   },  BG_SV_NODE_STATE_UNCONTROLLED, false, LANG_BG_SV_MINE              },
    { BG_SV_GO_BOSS_BANNER,   BG_SV_BOSS_BANNER,   TEAM_NEUTRAL, NODE_TYPE_BOSS,   { BG_SV_BOSS_BANNER_A,   BG_SV_BOSS_BANNER_A_CONT,   BG_SV_BOSS_BANNER_H,   BG_SV_BOSS_BANNER_H_CONT   }, false, 0, 0, { BG_SV_BOSS,   BG_SV_BOSS_CON_A,   BG_SV_BOSS_CON_H,   BG_SV_BOSS_A,   BG_SV_BOSS_H   },  BG_SV_NODE_STATE_UNCONTROLLED, false, LANG_BG_SV_RESTLESSGRAVEYARD },
};

enum BG_SV_HonorRewards
{
    BG_SV_WINNER_HONOR_AMOUNT = 500
};


struct BattlegroundSVScore final : public BattlegroundScore
{
    friend class BattlegroundSV;

    protected:
        explicit BattlegroundSVScore(ObjectGuid playerGuid) : BattlegroundScore(playerGuid) { }

        void UpdateScore(uint32 type, uint32 value) override
        {
            switch (type)
            {
            case SCORE_KILLING_BLOWS:
                KillingPoints += value;
                break;
            default:
                UpdateScore(type, value);
                break;
            }
        }

        void BuildObjectivesBlock(WorldPacket& data) final;

        uint32 GetAttr1() const final override { return KillingPoints; }

        uint32 KillingPoints;
};

class BattlegroundSV : public Battleground
{
    public:
        BattlegroundSV();
        ~BattlegroundSV();

        /* inherited from BattlegroundClass */
        void AddPlayer(Player* player);
        void StartingEventCloseDoors();
        void StartingEventOpenDoors();
        void PostUpdateImpl(uint32 diff);

        void RemovePlayer(Player* player);
        void HandleAreaTrigger(Player* player, uint32 trigger);
        bool SetupBattleground();
        void HandleKillPlayer(Player* player, Player* killer);
        void HandleKillUnit(Creature* unit, Player* killer);
        void EndBattleground(TeamId winner);
        void EventPlayerClickedOnFlag(Player* source, GameObject* /*target_obj*/);

        GraveyardStruct const* GetClosestGraveyard(Player* player);

        /* Scorekeeping */
        void FillInitialWorldStates(WorldPacket& data);

        void HandlePlayerResurrect(Player* player);

        uint32 GetNodeState(uint8 nodeType) const { return (uint8)nodePoint[nodeType].nodeState; }

        void UpdateWorldStates();
        void SpawnBoss();

    private:
        BG_SV_NodePoint nodePoint[4];
        uint32 GetNextBanner(BG_SV_NodePoint* node, uint32 team, bool returnDefinitve);
        uint32 TeamScore[2];
        uint32 ReloadTimer;
        uint32 BossSpawnTimer;
        uint32 BossWarningTimer;

        void BG_SV_UpdateNodeWorldState(BG_SV_NodePoint* node);
        void BG_SV_HandleCapturedNodes(BG_SV_NodePoint* node, bool recapture);
        void BG_SV_HandleContestedNodes(BG_SV_NodePoint* node);
        void BG_SV_StartRoot(Player* player);
        void BG_SV_RemoveRoot(Player* player);

        bool HordeCaptured;
        bool AllianceCaptured;
        bool BossSpawnAllowed;
};

#endif

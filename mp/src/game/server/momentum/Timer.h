#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "momentum/tickset.h"
#include "KeyValues.h"
#include "momentum/util/mom_util.h"
#include "filesystem.h"
#include "mom_triggers.h"
#include "GameEventListener.h"
#include "tier1/checksum_sha1.h"
#include "momentum/mom_shareddefs.h"
#include "momentum/mom_gamerules.h"
#include "mom_replay_system.h"
#include "movevars_shared.h"
#include <ctime>

class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerOnehop;
class CTriggerStage;

class CTimer
{
  public:
    CTimer()
        : m_iZoneCount(0), m_iStartTick(0), m_iEndTick(0), m_iLastZone(0), m_bIsRunning(false),
          m_bWereCheatsActivated(false), m_bMapIsLinear(false), m_pStartTrigger(nullptr), m_pEndTrigger(nullptr),
          m_pCurrentCheckpoint(nullptr), m_pCurrentStage(nullptr), m_iCurrentStepCP(0), m_bUsingCPMenu(false)
    {
    }

    //-------- HUD Messages --------------------
    void DispatchResetMessage();
    void DispatchCheckpointMessage();//MOM_TODO: MOVE TO PLAYER
    //Plays the hud_timer effects MOM_TODO: Maybe consider renaming this?
    void DispatchTimerStateMessage(CBasePlayer* pPlayer, bool isRunning) const;

    // ------------- Timer state related messages --------------------------
    // Strats the timer for the given starting tick
    void Start(int startTick);
    // Stops the timer
    void Stop(bool = false);
    // Is the timer running?
    bool IsRunning() const { return m_bIsRunning; }
    // Set the running status of the timer
    void SetRunning(bool running);

    // ------------- Timer trigger related methods ----------------------------
    // Gets the current starting trigger
    CTriggerTimerStart *GetStartTrigger() const { return m_pStartTrigger.Get(); }
    // Gets the current checkpoint
    CTriggerCheckpoint *GetCurrentCheckpoint() const { return m_pCurrentCheckpoint.Get(); }

    CTriggerTimerStop *GetEndTrigger() const { return m_pEndTrigger.Get(); }
    CTriggerStage *GetCurrentStage() const { return m_pCurrentStage.Get(); }

    // Sets the given trigger as the start trigger
    void SetStartTrigger(CTriggerTimerStart *pTrigger)
    {
        m_iLastZone = 0; // Allows us to overwrite previous runs
        m_pStartTrigger.Set(pTrigger);
    }

    // Sets the current checkpoint
    void SetCurrentCheckpointTrigger(CTriggerCheckpoint *pTrigger) { m_pCurrentCheckpoint.Set(pTrigger); }

    void SetEndTrigger(CTriggerTimerStop *pTrigger) { m_pEndTrigger.Set(pTrigger); }
    void SetCurrentStage(CTriggerStage *pTrigger)
    {
        m_pCurrentStage.Set(pTrigger);
        // DispatchStageMessage();
    }
    int GetCurrentZoneNumber() const { return m_pCurrentStage.Get() && m_pCurrentStage.Get()->GetStageNumber(); }

    // Calculates the stage count
    // Stores the result on m_iStageCount
    void RequestZoneCount();
    // Gets the total stage count
    int GetZoneCount() const { return m_iZoneCount; };
    float CalculateStageTime(int stageNum);
    // Gets the time for the last run, if there was one
    float GetLastRunTime()
    {
        if (m_iEndTick == 0)
            return 0.0f;
        float originalTime = static_cast<float>(m_iEndTick - m_iStartTick) * gpGlobals->interval_per_tick;
        // apply precision fix, adding offset from start as well as subtracting offset from end.
        // offset from end is 1 tick - fraction offset, since we started trace outside of the end zone.
        return originalTime + m_flTickOffsetFix[1] - (gpGlobals->interval_per_tick - m_flTickOffsetFix[0]);
    }

    // Gets the current time for this timer
    float GetCurrentTime() const { return float(gpGlobals->tickcount - m_iStartTick) * gpGlobals->interval_per_tick; }

    //--------- CheckpointMenu stuff --------------------------------
    // Gets the current menu checkpoint index
    int GetCurrentCPMenuStep() const { return m_iCurrentStepCP; }
    // MOM_TODO: For leaderboard use later on
    bool IsUsingCPMenu() const { return m_bUsingCPMenu; }
    // Creates a checkpoint (menu) on the location of the given Entity
    void CreateCheckpoint(CBasePlayer *);
    // Removes last checkpoint (menu) form the checkpoint lists
    void RemoveLastCheckpoint();
    // Removes every checkpoint (menu) on the checkpoint list
    void RemoveAllCheckpoints()
    {
        checkpoints.RemoveAll();
        m_iCurrentStepCP = -1;
        // SetUsingCPMenu(false);
        DispatchCheckpointMessage();
    }
    // Teleports the entity to the checkpoint (menu) with the given index
    void TeleportToCP(CBasePlayer *, int);
    // Sets the current checkpoint (menu) to the desired one with that index
    void SetCurrentCPMenuStep(int pNewNum);
    // Gets the total amount of menu checkpoints
    int GetCPCount() const { return checkpoints.Size(); }
    // Sets wheter or not we're using the CPMenu
    // WARNING! No verification is done. It is up to the caller to don't give false information
    void SetUsingCPMenu(bool pIsUsingCPMenu);

    //----- Trigger_Onehop stuff -----------------------------------------
    // Removes the given Onehop form the hopped list.
    // Returns: True if deleted, False if not found.
    bool RemoveOnehopFromList(CTriggerOnehop *pTrigger);
    // Adds the give Onehop to the hopped list.
    // Returns: Its new index.
    int AddOnehopToListTail(CTriggerOnehop *pTrigger);
    // Finds a Onehop on the hopped list.
    // Returns: Its index. -1 if not found
    int FindOnehopOnList(CTriggerOnehop *pTrigger);
    // Removes all onehops from the list
    void RemoveAllOnehopsFromList() { onehops.RemoveAll(); }
    // Returns the count for the onehop list
    int GetOnehopListCount() const { return onehops.Count(); }
    // Finds the onehop with the given index on the list
    CTriggerOnehop *FindOnehopOnList(int pIndexOnList);

    //-------- Online-related timer commands -----------------------------
    // Tries to post the current time.
    void PostTime();
    // MOM_TODO: void LoadOnlineTimes();

    //------- Local-related timer commands -------------------------------
    // Loads local times from given map name
    void LoadLocalTimes(const char *);
    // Saves current time to a local file
    void SaveTime();
    void OnMapEnd(const char *);
    void OnMapStart(const char *);
    void DispatchMapInfo();
    // Practice mode- noclip mode that stops timer
    // void PracticeMove(); MOM_TODO: NOT IMPLEMENTED
    void EnablePractice(CMomentumPlayer *pPlayer);
    void DisablePractice(CMomentumPlayer *pPlayer);

    // Have the cheats been turned on in this session?
    bool GotCaughtCheating() const { return m_bWereCheatsActivated; };
    void SetCheating(bool newBool)
    {
        UTIL_ShowMessage("CHEATER", UTIL_GetLocalPlayer());
        Stop(false);
        m_bWereCheatsActivated = newBool;
    }

    void SetGameModeConVars();

  private:
    int m_iZoneCount;
    int m_iStartTick, m_iEndTick;
    int m_iLastZone;
    bool m_bIsRunning;
    bool m_bWereCheatsActivated;
    bool m_bMapIsLinear;

    CHandle<CTriggerTimerStart> m_pStartTrigger;
    CHandle<CTriggerTimerStop> m_pEndTrigger;
    CHandle<CTriggerCheckpoint> m_pCurrentCheckpoint;
    CHandle<CTriggerStage> m_pCurrentStage; // MOM_TODO: Change to m_pCurrentZone

    struct Time
    {
        // overall run stats:
        float time_sec; // The amount of seconds taken to complete
        float tickrate; // Tickrate the run was done on
        time_t date;    // Date achieved
        int flags;

        // stage specific stats:
        CMomRunStats RunStats;

        Time() : time_sec(0), tickrate(0), date(0), flags(0), RunStats() {}
    };

    struct Checkpoint
    {
        Vector pos;
        Vector vel;
        QAngle ang;
        char targetName[MAX_PLAYER_NAME_LENGTH];
        char targetClassName[MAX_PLAYER_NAME_LENGTH];
    };
    CUtlVector<Checkpoint> checkpoints;
    CUtlVector<CTriggerOnehop *> onehops;
    CUtlVector<Time*> localTimes;
    // MOM_TODO: CUtlVector<OnlineTime> onlineTimes;

    int m_iCurrentStepCP;
    bool m_bUsingCPMenu;
public:
    // PRECISION FIX:
    // this works by adding the starting offset to the final time, since the timer starts after we actually exit the
    // start trigger
    // also, subtract the ending offset from the time, since we end after we actually enter the ending trigger
    float m_flTickOffsetFix[MAX_STAGES]; // index 0 = endzone, 1 = startzone, 2 = stage 2, 3 = stage3, etc
    float m_flZoneEnterTime[MAX_STAGES];

    // creates fraction of a tick to be used as a time "offset" in precicely calculating the real run time.
    void CalculateTickIntervalOffset(CMomentumPlayer *pPlayer, const int zoneType);
    void SetIntervalOffset(int stage, float offset) { m_flTickOffsetFix[stage] = offset; }
    float m_flTickOffsetFixTraceCorners[8]; //array of floats representing the trace times from each corner of the player's collision hull
    typedef enum { ZONETYPE_END, ZONETYPE_START } zoneType;
};

class CTimeTriggerTraceEnum : public IEntityEnumerator
{
  public:
    CTimeTriggerTraceEnum(Ray_t *pRay, Vector velocity, int zoneType, int cornerNum)
        : m_iZoneType(zoneType), m_pRay(pRay), m_currVelocity(velocity), m_iCornerNumber(cornerNum)
    {
    }

    bool EnumEntity(IHandleEntity *pHandleEntity) override;

  private:
    int m_iZoneType;
    int m_iCornerNumber;
    Ray_t *m_pRay;
    Vector m_currVelocity;
};

extern CTimer *g_Timer;

#endif // TIMER_H
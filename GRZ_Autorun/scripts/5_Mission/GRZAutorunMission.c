#ifndef SERVER
modded class MissionGameplay
{
    protected ref set<int> m_GRZAutorunHeldKeys;
    protected bool m_GRZAutorunPendingKeyStop;

    protected bool GRZAutorunKeyMatchesAction(int key, int action)
    {
        TIntArray keys = new TIntArray();
        GetGame().GetInput().GetCurrentProfileActionKeys(action, keys);
        return keys.Find(key) != -1;
    }

    protected bool GRZAutorunCanKeepRunningForKey(int key)
    {
        // Explicit physical-key exceptions do not depend on the legacy
        // profile/action lookup. Q/E must never cancel the default lean run.
        if (key == KeyCode.KC_Q || key == KeyCode.KC_E || key == KeyCode.KC_TAB)
            return true;
        // For other keys, the autorun action still takes stop priority.
        UAInput autorun = GetUApi().GetInputByName("UAGRZAutorunToggle");
        if (autorun && GRZAutorunKeyMatchesAction(key, autorun.ID()))
            return false;
        // Allow the native hold/toggle freelook actions, including remapped keys.
        // The physical Alt fallback also works when profile key lookup is unavailable.
        if (key == KeyCode.KC_LMENU || key == KeyCode.KC_RMENU)
            return true;
        return GRZAutorunKeyMatchesAction(key, UALeanLeft) || GRZAutorunKeyMatchesAction(key, UALeanRight) || GRZAutorunKeyMatchesAction(key, UALookAround) || GRZAutorunKeyMatchesAction(key, UALookAroundToggle);
    }

    protected bool GRZAutorunOpeningLBScreen()
    {
        // These action names are present in the supplied LB addons. Queries
        // are optional/null-safe, so no compile dependency on LB classes exists.
        if (GRZAutorunState.Pressed("UALBMGroupOpenMap") || GRZAutorunState.Pressed("UALBMGroupOpenMapGroup") || GRZAutorunState.Pressed("UALBMOpenLeaderboard"))
            return true;
        if (!GRZAutorunState.Pressed("UADefaultAction"))
            return false;
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return false;
        ActionManagerClient manager = ActionManagerClient.Cast(player.GetActionManager());
        if (!manager)
            return false;
        ActionBase action = manager.GetRunningAction();
        if (!action)
            action = manager.GetPossibleAction(InteractActionInput);
        if (action && action.ClassName() == "ActionOpenATMMenu")
        {
            GRZAutorunState.AllowATMOpening();
            return true;
        }
        return false;
    }

    override void OnInit()
    {
        super.OnInit();
        GRZAutorunState.Stop();
        m_GRZAutorunHeldKeys = new set<int>();
        m_GRZAutorunPendingKeyStop = false;
        Print("[GRZAutorun v0.12] Alt/remapped freelook, lean and LB screen autorun support loaded.");
    }

    override void OnKeyPress(int key)
    {
        if (!m_GRZAutorunHeldKeys)
            m_GRZAutorunHeldKeys = new set<int>();
        // Ignore keyboard-repeat events for a key already held down. The
        // initial autorun key must not cancel itself while it is being held.
        if (m_GRZAutorunHeldKeys.Find(key) == -1)
        {
            m_GRZAutorunHeldKeys.Insert(key);
            if (GRZAutorunState.Active && !GRZAutorunState.IsAllowedAutorunUI() && !GRZAutorunCanKeepRunningForKey(key))
            {
                // Defer until UI input is processed: this may be the key that
                // opens chat, a custom map, or its marker dialog.
                m_GRZAutorunPendingKeyStop = true;
            }
        }
        super.OnKeyPress(key);
    }

    override void OnKeyRelease(int key)
    {
        if (m_GRZAutorunHeldKeys)
        {
            int index = m_GRZAutorunHeldKeys.Find(key);
            if (index != -1) m_GRZAutorunHeldKeys.Remove(index);
        }
        super.OnKeyRelease(key);
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        bool cancelledByKey = false;
        if (m_GRZAutorunPendingKeyStop)
        {
            bool openedUI = GRZAutorunState.IsAllowedAutorunUI() || GRZAutorunOpeningLBScreen() || GRZAutorunState.Pressed("UAChat") || GRZAutorunState.Pressed("UAMapToggle");
            if (GRZAutorunState.Active && !openedUI)
            {
                GRZAutorunState.Stop();
                cancelledByKey = true;
            }
            m_GRZAutorunPendingKeyStop = false;
        }
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!GRZAutorunState.CanRun(player))
        {
            GRZAutorunState.Stop();
            return;
        }
        // Keep the existing action ID so previously assigned bindings survive.
        // This action starts autorun; keyboard events stop it. Do not restart
        // on the same frame when pressing the bound key stopped a running mode.
        if (!cancelledByKey && !GRZAutorunState.Active && GRZAutorunState.Pressed("UAGRZAutorunToggle"))
        {
            GRZAutorunState.Start();
        }
        GRZAutorunState.Apply(player);
    }

    override void OnMissionFinish()
    {
        GRZAutorunState.Stop();
        if (m_GRZAutorunHeldKeys) m_GRZAutorunHeldKeys.Clear();
        super.OnMissionFinish();
    }
}
#endif

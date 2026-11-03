#ifndef SERVER
class GRZAutorunState
{
    static bool Active;
    private static ref set<string> s_InitialMovementKeys;

    static void Start()
    {
        s_InitialMovementKeys = new set<string>();
        array<string> actions = {"UAMoveForward", "UAMoveBack", "UAMoveLeft", "UAMoveRight"};
        foreach (string action : actions)
        {
            if (Held(action))
                s_InitialMovementKeys.Insert(action);
        }
        Active = true;
    }

    private static bool HasNewManualMovement()
    {
        bool manualMovement = false;
        array<string> actions = {"UAMoveForward", "UAMoveBack", "UAMoveLeft", "UAMoveRight"};
        foreach (string action : actions)
        {
            bool held = Held(action);
            int initialIndex = -1;
            if (s_InitialMovementKeys)
                initialIndex = s_InitialMovementKeys.Find(action);
            if (initialIndex != -1)
            {
                // Keys held when starting may remain down until released once.
                if (!held)
                    s_InitialMovementKeys.Remove(initialIndex);
            }
            else if (held)
                manualMovement = true;
        }
        return manualMovement;
    }

    static bool Pressed(string name)
    {
        UAInput input = GetUApi().GetInputByName(name);
        return input && input.LocalPress();
    }

    static bool Held(string name)
    {
        UAInput input = GetUApi().GetInputByName(name);
        return input && input.LocalValue() > 0.1;
    }

    private static int s_ATMOpeningUntil;

    static bool IsLBScreenName(string name)
    {
        return name == "LBGroupUI" || name == "LBLeaderboardGUI" || name == "LBATMMenu";
    }

    static void AllowATMOpening()
    {
        // The inspected ATM action requests the menu data from the server.
        // Permit its no-menu input-suppression interval, but not other menus.
        s_ATMOpeningUntil = GetGame().GetTime() + 2000;
    }

    static bool IsAllowedAutorunUI()
    {
        UIScriptedMenu menu = GetGame().GetUIManager().GetMenu();
        // Recognize concrete LB classes before numeric IDs: custom menu IDs
        // are owned by LB Core, not the vanilla menu constants.
        if (menu && IsLBScreenName(menu.ClassName()))
            return true;
        if (menu)
        {
            int id = menu.GetID();
            if (id == MENU_INGAME || id == MENU_OPTIONS || id == MENU_MAIN || id == MENU_LOADING || id == MENU_LOGOUT)
                return false;
        }
        UIScriptedMenu current = menu;
        for (int depth = 0; current && depth < 12; depth++)
        {
            if (IsLBScreenName(current.ClassName()))
                return true;
            int currentId = current.GetID();
            if (currentId == MENU_INVENTORY || currentId == MENU_CHAT || currentId == MENU_CHAT_INPUT || currentId == MENU_MAP)
                return true;
            current = UIScriptedMenu.Cast(current.GetParentMenu());
        }
        return !menu && GetGame().GetTime() < s_ATMOpeningUntil;
    }

    static bool CanRun(PlayerBase player)
    {
        if (!player || !player.IsAlive() || player.IsUnconscious() || player.IsRestrained() || player.IsRaised())
            return false;
        Mission mission = GetGame().GetMission();
        if (!mission)
            return false;
        // Keep a run already in progress through permitted UI input suppression.
        // Starting autorun while a menu/control restriction is active stays blocked.
        UIScriptedMenu menu = GetGame().GetUIManager().GetMenu();
        bool runningWithUI = Active && IsAllowedAutorunUI();
        if (menu && !runningWithUI)
            return false;
        if (mission.IsControlDisabled() && !runningWithUI)
            return false;
        return player.GetCurrentCommandID() == DayZPlayerConstants.COMMANDID_MOVE;
    }

    private static PlayerBase s_InputOwner;
    private static int s_LastSync;

    private static void SendState(PlayerBase player, bool running)
    {
        if (GetGame().IsMultiplayer() && !GetGame().IsServer())
            GetGame().RPCSingleParam(player, GRZAutorunProtocol.RPC_STATE, new Param1<bool>(running), true);
    }

    static void Stop()
    {
        Active = false;
        s_ATMOpeningUntil = 0;
        if (s_InitialMovementKeys)
            s_InitialMovementKeys.Clear();
        if (s_InputOwner)
        {
            SendState(s_InputOwner, false);
            HumanInputController controls = s_InputOwner.GetInputController();
            if (controls)
            {
                controls.OverrideMovementSpeed(HumanInputControllerOverrideType.DISABLED, 0.0);
                controls.OverrideMovementAngle(HumanInputControllerOverrideType.DISABLED, 0.0);
            }
        }
        s_InputOwner = null;
        s_LastSync = 0;
    }

    static void Apply(PlayerBase player)
    {
        if (!Active) return;
        if (s_InputOwner && s_InputOwner != player)
        {
            Stop();
            return;
        }
        bool manualMovement = HasNewManualMovement();
        if (IsAllowedAutorunUI())
            manualMovement = false;
        if (!CanRun(player) || manualMovement)
        {
            Stop();
            return;
        }
        HumanInputController controls = player.GetInputController();
        if (!controls)
        {
            Stop();
            return;
        }
        bool starting = !s_InputOwner;
        s_InputOwner = player;
        // Keep native forward/sprint input active between mission updates.
        // Explicitly release only these owned overrides on every stop path.
        controls.OverrideMovementSpeed(HumanInputControllerOverrideType.ENABLED, 3.0);
        controls.OverrideMovementAngle(HumanInputControllerOverrideType.ENABLED, 0.0);
        int now = GetGame().GetTime();
        if (starting || now - s_LastSync >= GRZAutorunProtocol.HEARTBEAT_MS)
        {
            SendState(player, true);
            s_LastSync = now;
        }
    }
}
#endif

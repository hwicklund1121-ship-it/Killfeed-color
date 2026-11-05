class GRZDeathScreenButtons : ScriptedWidgetEventHandler
{
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (button != MouseState.LEFT) return false;
        MissionGameplay mission = MissionGameplay.Cast(g_Game.GetMission());
        if (!mission) return false;
        string name = w.GetName();
        if (name == "RespawnButton") { mission.GRZShowSpawnChoices(true); return true; }
        if (name == "SpawnBackButton") { mission.GRZShowSpawnChoices(false); return true; }
        if (name == "SpawnCustomButton" || name == "SpawnRandomButton")
        {
            mission.GRZChooseDeathRespawn(name == "SpawnRandomButton");
            return true;
        }
        return false;
    }
};

modded class InGameMenu
{
    void GRZRespawnFromDeathScreen()
    {
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if ((player && player.IsAlive()) || !GRZDeathScreenState.Visible)
            return;
        // Reuse native cleanup, respawn request and player-respawning state.
        // The server's CreateCharacter hook chooses the active GRZ arena.
        GameRespawn(false);
    }
};

modded class MissionGameplay
{
    protected Widget m_GRZDeathArtwork;
    protected bool m_GRZSpawnChoicesVisible;
    protected bool m_GRZSpawnChoicePending;
    protected int m_GRZSpawnRequestId;
    protected int m_GRZSpawnRequestAt;
    protected ref GRZDeathScreenButtons m_GRZDeathButtons;
    protected bool m_GRZDeathMouseFocus;
    protected Widget m_GRZDeathHiddenHud;
    protected bool m_GRZDeathHudWasVisible = true;
    protected Widget m_GRZDeathLivingHud;

    override void OnInit()
    {
        super.OnInit();
        GRZDeathScreenState.Reset();
        DayZGame.Event_OnRPC.Insert(GRZReceiveDeathSpawnAck);
    }

    protected void GRZDeathFocus(bool enabled)
    {
        if (m_GRZDeathMouseFocus == enabled)
            return;
        m_GRZDeathMouseFocus = enabled;
        int delta = -1;
        if (enabled)
            delta = 1;
        g_Game.GetInput().ChangeGameFocus(delta, INPUT_DEVICE_MOUSE);
        if (enabled)
            g_Game.GetUIManager().ShowUICursor(true);
        else if (!g_Game.GetUIManager().GetMenu())
            g_Game.GetUIManager().ShowUICursor(false);
    }

    protected void GRZDeathHideHud(bool hidden)
    {
        IngameHud hud = IngameHud.Cast(GetHud());
        Widget root;
        if (hud)
            root = hud.GetHudPanelWidget();
        if (hidden)
        {
            if (root && root != m_GRZDeathHiddenHud)
            {
                if (m_GRZDeathHiddenHud)
                    m_GRZDeathHiddenHud.Show(m_GRZDeathHudWasVisible);
                m_GRZDeathHiddenHud = root;
                if (root != m_GRZDeathLivingHud)
                    m_GRZDeathHudWasVisible = true;
            }
            if (m_GRZDeathHiddenHud)
                m_GRZDeathHiddenHud.Show(false);
        }
        else if (m_GRZDeathHiddenHud)
        {
            m_GRZDeathHiddenHud.Show(m_GRZDeathHudWasVisible);
            m_GRZDeathHiddenHud = null;
        }
        if (!hidden && root)
        {
            m_GRZDeathLivingHud = root;
            m_GRZDeathHudWasVisible = root.IsVisible();
        }
    }

    void GRZShowSpawnChoices(bool show)
    {
        if (m_GRZSpawnChoicePending || !GRZDeathScreenState.Visible || GRZDeathScreenState.Alpha() < 0.999) return;
        m_GRZSpawnChoicesVisible = show;
    }

    void GRZChooseDeathRespawn(bool randomCharacter)
    {
        if (!m_GRZSpawnChoicesVisible || m_GRZSpawnChoicePending || !GRZDeathScreenState.Visible || IsPlayerRespawning()) return;
        m_GRZSpawnChoicePending = true;
        m_GRZSpawnRequestAt = g_Game.GetTime();
        m_GRZSpawnRequestId++;
        ScriptRPC request = new ScriptRPC;
        request.Write(m_GRZSpawnRequestId);
        request.Write(randomCharacter);
        request.Send(null, GRZRespawnChoiceRPC.CHOOSE, true, null);
    }

    void GRZReceiveDeathSpawnAck(PlayerIdentity sender, Object target, int rpcType, ParamsReadContext ctx)
    {
        if (g_Game.IsServer() || target || rpcType != GRZRespawnChoiceRPC.ACK || !m_GRZSpawnChoicePending) return;
        int requestId;
        bool allowed;
        if (!ctx.Read(requestId) || !ctx.Read(allowed) || requestId != m_GRZSpawnRequestId) return;
        m_GRZSpawnChoicePending = false;
        if (allowed) GRZRequestDeathRespawn();
    }

    void GRZRequestDeathRespawn()
    {
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if ((player && player.IsAlive()) || !GRZDeathScreenState.Visible || GRZDeathScreenState.Alpha() < 0.999 || IsPlayerRespawning())
            return;
        GRZDeathFocus(false);
        if (m_GRZDeathArtwork)
            m_GRZDeathArtwork.Show(false);
        InGameMenu menu = InGameMenu.Cast(g_Game.GetUIManager().EnterScriptedMenu(MENU_INGAME, null));
        if (menu)
            menu.GRZRespawnFromDeathScreen();
        else
            Error("[GRZ DeathScreen] Native respawn menu could not be opened.");
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        bool showArtwork = GRZDeathScreenState.Visible;
        if ((player && player.IsAlive()) || IsPlayerRespawning())
        {
            GRZDeathScreenState.Reset();
            showArtwork = false;
            m_GRZSpawnChoicesVisible = false;
            m_GRZSpawnChoicePending = false;
        }
        if (m_GRZSpawnChoicePending && g_Game.GetTime() - m_GRZSpawnRequestAt > 5000)
            m_GRZSpawnChoicePending = false;
        GRZDeathHideHud(GRZDeathScreenState.Visible || IsPlayerRespawning() || (player && !player.IsAlive()));
        float alpha = GRZDeathScreenState.Alpha();
        bool ready = alpha >= 0.999;
        showArtwork = showArtwork && alpha > 0;
        if (g_Game.GetUIManager().GetMenu() || g_Game.GetUIManager().IsDialogVisible())
            showArtwork = false;
        if (showArtwork && !m_GRZDeathArtwork)
        {
            m_GRZDeathArtwork = g_Game.GetWorkspace().CreateWidgets("GRZ_DeathScreen/gui/layouts/death.layout");
            m_GRZDeathArtwork.SetSort(100000);
            ImageWidget picture = ImageWidget.Cast(m_GRZDeathArtwork.FindAnyWidget("DeathPicture"));
            if (picture)
            {
                picture.LoadImageFile(0, "GRZ_DeathScreen/data/death.paa");
                picture.SetImage(0);
            }
            string fills[4] = {"RespawnButtonFill", "SpawnCustomButtonFill", "SpawnRandomButtonFill", "SpawnBackButtonFill"};
            foreach (string fillName : fills)
            {
                ImageWidget fill = ImageWidget.Cast(m_GRZDeathArtwork.FindAnyWidget(fillName));
                if (fill)
                {
                    fill.LoadImageFile(0, "GRZ_DeathScreen/data/solid_white.paa");
                    fill.SetImage(0);
                    fill.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
                }
            }
            m_GRZDeathButtons = new GRZDeathScreenButtons;
            m_GRZDeathArtwork.SetHandler(m_GRZDeathButtons);
        }
        if (m_GRZDeathArtwork)
        {
            m_GRZDeathArtwork.SetAlpha(alpha);
            m_GRZDeathArtwork.Show(showArtwork);
            Widget respawn = m_GRZDeathArtwork.FindAnyWidget("RespawnButton");
            if (respawn)
            {
                respawn.Show(ready && !m_GRZSpawnChoicesVisible);
                respawn.Enable(ready);
            }
            Widget choices = m_GRZDeathArtwork.FindAnyWidget("SpawnChoices");
            if (choices)
            {
                choices.Show(ready && m_GRZSpawnChoicesVisible);
                choices.Enable(!m_GRZSpawnChoicePending);
            }
        }
        GRZDeathFocus(showArtwork && ready);
    }

    override void OnMissionFinish()
    {
        DayZGame.Event_OnRPC.Remove(GRZReceiveDeathSpawnAck);
        GRZDeathScreenState.Reset();
        GRZDeathFocus(false);
        GRZDeathHideHud(false);
        if (m_GRZDeathArtwork)
        {
            m_GRZDeathArtwork.SetHandler(null);
            m_GRZDeathArtwork.Unlink();
            m_GRZDeathArtwork = null;
        }
        m_GRZDeathButtons = null;
        super.OnMissionFinish();
    }
};

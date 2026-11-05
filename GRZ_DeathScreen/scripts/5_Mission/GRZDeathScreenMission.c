class GRZDeathScreenButtons : ScriptedWidgetEventHandler
{
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (button == MouseState.LEFT && w.GetName() == "RespawnButton")
        {
            MissionGameplay mission = MissionGameplay.Cast(g_Game.GetMission());
            if (mission)
                mission.GRZRequestDeathRespawn();
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
        if (!player || player.IsAlive())
            return;
        // Reuse native cleanup, respawn request and player-respawning state.
        // The server's CreateCharacter hook chooses the active GRZ arena.
        GameRespawn(false);
    }
};

modded class MissionGameplay
{
    protected Widget m_GRZDeathArtwork;
    protected ref GRZDeathScreenButtons m_GRZDeathButtons;
    protected bool m_GRZDeathMouseFocus;

    override void OnInit()
    {
        super.OnInit();
        GRZDeathScreenState.Visible = false;
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

    void GRZRequestDeathRespawn()
    {
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if (!player || player.IsAlive() || IsPlayerRespawning())
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
        if (!player || player.IsAlive() || IsPlayerRespawning())
        {
            GRZDeathScreenState.Visible = false;
            showArtwork = false;
        }
        if (g_Game.GetUIManager().GetMenu() || g_Game.GetUIManager().IsDialogVisible())
            showArtwork = false;
        if (showArtwork && !m_GRZDeathArtwork)
        {
            m_GRZDeathArtwork = g_Game.GetWorkspace().CreateWidgets("GRZ_DeathScreen/gui/layouts/death.layout");
            ImageWidget picture = ImageWidget.Cast(m_GRZDeathArtwork.FindAnyWidget("DeathPicture"));
            if (picture)
            {
                picture.LoadImageFile(0, "GRZ_DeathScreen/data/death.paa");
                picture.SetImage(0);
            }
            m_GRZDeathButtons = new GRZDeathScreenButtons;
            m_GRZDeathArtwork.SetHandler(m_GRZDeathButtons);
        }
        if (m_GRZDeathArtwork)
            m_GRZDeathArtwork.Show(showArtwork);
        GRZDeathFocus(showArtwork);
    }

    override void OnMissionFinish()
    {
        GRZDeathScreenState.Visible = false;
        GRZDeathFocus(false);
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

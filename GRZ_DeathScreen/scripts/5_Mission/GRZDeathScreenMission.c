modded class MissionGameplay
{
    protected Widget m_GRZDeathArtwork;

    override void OnInit()
    {
        super.OnInit();
        GRZDeathScreenState.Visible = false;
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
        // Keep ESC, respawn and confirmation menus unobstructed.
        if (g_Game.GetUIManager().GetMenu() || g_Game.GetUIManager().IsDialogVisible())
            showArtwork = false;
        if (showArtwork && !m_GRZDeathArtwork)
        {
            m_GRZDeathArtwork = g_Game.GetWorkspace().CreateWidgets("GRZ_DeathScreen/gui/layouts/death.layout");
            ImageWidget picture = ImageWidget.Cast(m_GRZDeathArtwork);
            if (picture)
            {
                picture.LoadImageFile(0, "GRZ_DeathScreen/data/death.paa");
                picture.SetImage(0);
            }
        }
        if (m_GRZDeathArtwork)
            m_GRZDeathArtwork.Show(showArtwork);
    }

    override void OnMissionFinish()
    {
        GRZDeathScreenState.Visible = false;
        if (m_GRZDeathArtwork)
        {
            m_GRZDeathArtwork.Unlink();
            m_GRZDeathArtwork = null;
        }
        super.OnMissionFinish();
    }
};

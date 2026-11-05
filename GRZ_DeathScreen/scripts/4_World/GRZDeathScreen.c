class GRZDeathScreenState
{
    static bool Visible;
};

modded class DayZPlayerImplement
{
    override void ShowDeadScreen(bool show, float duration)
    {
        super.ShowDeadScreen(show, duration);
        if (!g_Game || g_Game.IsDedicatedServer())
            return;
        if (show && IsPlayerSelected())
        {
            GRZDeathScreenState.Visible = true;
            // Artwork already contains the death message; remove the native fade.
            g_Game.GetUIManager().ScreenFadeOut(0);
        }
        else if (!show)
        {
            GRZDeathScreenState.Visible = false;
        }
    }
};

class GRZDeathScreenState
{
    static bool Visible;
    static int StartedAt = -1;
    static const float BODY_VIEW_SECONDS = 2.0;
    static const float FADE_SECONDS = 0.8;

    static void Begin()
    {
        // Native death, cleanup RPC and corpse deletion can all notify us.
        // Only the first notification starts the animation clock.
        if (!Visible || StartedAt < 0)
            StartedAt = g_Game.GetTime();
        Visible = true;
    }

    static void Reset()
    {
        Visible = false;
        StartedAt = -1;
    }

    static float Alpha()
    {
        if (!Visible || StartedAt < 0)
            return 0;
        float elapsed = (g_Game.GetTime() - StartedAt) / 1000.0;
        return Math.Clamp((elapsed - BODY_VIEW_SECONDS) / FADE_SECONDS, 0, 1);
    }
};

modded class DayZPlayerImplement
{
    override void ShowDeadScreen(bool show, float duration)
    {
        if (!g_Game || g_Game.IsDedicatedServer())
        {
            super.ShowDeadScreen(show, duration);
            return;
        }
        if (show && IsPlayerSelected())
        {
            GRZDeathScreenState.Begin();
            // The native show=true branch draws its own immediate black fade.
            // Keep the death camera visible; the artwork fades independently.
            g_Game.GetUIManager().ScreenFadeOut(0);
            return;
        }
        super.ShowDeadScreen(show, duration);
        if (!show)
            GRZDeathScreenState.Reset();
    }
};

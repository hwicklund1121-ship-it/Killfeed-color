#ifndef SERVER
modded class MissionGameplay
{
    protected int m_GRZEarmuffsPercent = 20;
    protected bool m_GRZEarmuffsEnabled;
    protected bool m_GRZEarmuffsApplied;
    protected float m_GRZEarmuffsBaseVolume;
    protected Widget m_GRZEarmuffsPopup;
    protected float m_GRZEarmuffsPopupAge = 3;
    protected bool m_GRZEarmuffsLayoutAttempted;

    protected void GRZEarmuffsRestore()
    {
        if (m_GRZEarmuffsApplied && g_Game && g_Game.GetSoundScene())
            g_Game.GetSoundScene().SetSoundVolume(m_GRZEarmuffsBaseVolume, 0);
        m_GRZEarmuffsApplied = false;
    }

    protected void GRZEarmuffsApply()
    {
        if (!g_Game.GetSoundScene())
            return;
        if (!m_GRZEarmuffsEnabled || m_GRZEarmuffsPercent >= 100)
        {
            GRZEarmuffsRestore();
            return;
        }
        if (!m_GRZEarmuffsApplied)
            m_GRZEarmuffsBaseVolume = g_Game.GetSoundScene().GetSoundVolume();
        g_Game.GetSoundScene().SetSoundVolume(m_GRZEarmuffsBaseVolume * m_GRZEarmuffsPercent / 100.0, 0.12);
        m_GRZEarmuffsApplied = true;
    }

    protected void GRZEarmuffsShowPopup(IngameHud hud)
    {
        if (!hud || !hud.GetHudPanelWidget())
            return;
        if (!m_GRZEarmuffsLayoutAttempted)
        {
            m_GRZEarmuffsLayoutAttempted = true;
            m_GRZEarmuffsPopup = g_Game.GetWorkspace().CreateWidgets("GRZ_Earmuffs/gui/layouts/earmuffs.layout", hud.GetHudPanelWidget());
            if (!m_GRZEarmuffsPopup)
            {
                Print("[GRZ Earmuffs] Popup layout could not be loaded.");
                return;
            }
            m_GRZEarmuffsPopup.SetSort(100);
            CanvasWidget symbol = CanvasWidget.Cast(m_GRZEarmuffsPopup.FindAnyWidget("GRZEarmuffsSymbol"));
            if (symbol)
            {
                // Solid speaker cabinet and tapered cone, like the supplied icon.
                symbol.DrawLine(5, 20, 12, 20, 10, 0xFFFFFFFF);
                for (int column = 0; column <= 10; column++)
                {
                    float extent = 5 + column * 0.8;
                    symbol.DrawLine(12 + column, 20 - extent, 12 + column, 20 + extent, 1.5, 0xFFFFFFFF);
                }
                for (int wave = 0; wave < 2; wave++)
                {
                    float radius = 8 + wave * 7;
                    for (int segment = 0; segment < 8; segment++)
                    {
                        float a = -0.8 + segment * 0.2;
                        float b = a + 0.2;
                        symbol.DrawLine(22 + Math.Cos(a) * radius, 20 + Math.Sin(a) * radius, 22 + Math.Cos(b) * radius, 20 + Math.Sin(b) * radius, 2, 0xFFFFFFFF);
                    }
                }
            }
        }
        if (!m_GRZEarmuffsPopup)
            return;
        TextWidget percent = TextWidget.Cast(m_GRZEarmuffsPopup.FindAnyWidget("GRZEarmuffsPercent"));
        if (percent)
        {
            int displayed = 100;
            if (m_GRZEarmuffsEnabled)
                displayed = m_GRZEarmuffsPercent;
            percent.SetText(displayed.ToString() + "%");
            percent.SetTextExactSize(24);
        }
        m_GRZEarmuffsPopupAge = 0;
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        m_GRZEarmuffsPopupAge += timeslice;
        IngameHud hud = IngameHud.Cast(GetHud());
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        bool alive = player && player.IsAlive();
        // Suspend while menus are open, so the player's normal audio settings
        // can be edited without saving the temporary earmuffs attenuation.
        bool blocked = true;
        if (alive)
            blocked = player.IsUnconscious() || !g_Game.IsAppActive() || g_Game.GetUIManager().GetMenu() || IsPaused();
        if (blocked)
        {
            GRZEarmuffsRestore();
            if (!alive)
                m_GRZEarmuffsEnabled = false;
            if (m_GRZEarmuffsPopup)
                m_GRZEarmuffsPopup.Show(false);
            return;
        }
        UAInput toggle = GetUApi().GetInputByName("UAGRZEarmuffsToggle");
        UAInput lower = GetUApi().GetInputByName("UAGRZEarmuffsLower");
        UAInput higher = GetUApi().GetInputByName("UAGRZEarmuffsHigher");
        bool changed = false;
        if (toggle && toggle.LocalPress())
        {
            m_GRZEarmuffsEnabled = !m_GRZEarmuffsEnabled;
            changed = true;
        }
        if (m_GRZEarmuffsEnabled)
        {
            if (lower && lower.LocalPress())
            {
                m_GRZEarmuffsPercent = Math.Max(10, m_GRZEarmuffsPercent - 10);
                changed = true;
            }
            if (higher && higher.LocalPress())
            {
                m_GRZEarmuffsPercent = Math.Min(100, m_GRZEarmuffsPercent + 10);
                changed = true;
            }
        }
        if (changed)
        {
            GRZEarmuffsApply();
            GRZEarmuffsShowPopup(hud);
        }
        else if (m_GRZEarmuffsEnabled && m_GRZEarmuffsPercent < 100 && !m_GRZEarmuffsApplied)
            GRZEarmuffsApply();
        if (!m_GRZEarmuffsPopup || !hud || !hud.GetHudPanelWidget())
            return;
        float x, y, width, height;
        hud.GetHudPanelWidget().GetScreenPos(x, y);
        hud.GetHudPanelWidget().GetScreenSize(width, height);
        m_GRZEarmuffsPopup.SetScreenPos(x + 24, y + (height - 52) * 0.5);
        // Hold for 1.25 seconds, then fade over 0.75 seconds. Every press restarts it.
        m_GRZEarmuffsPopup.SetAlpha(Math.Clamp((2.0 - m_GRZEarmuffsPopupAge) / 0.75, 0, 1));
        m_GRZEarmuffsPopup.Show(m_GRZEarmuffsPopupAge < 2.0 && !hud.GetHudVisibility().IsContextFlagActive(IngameHudVisibility.HUD_HIDE_FLAGS));
    }

    override void OnMissionFinish()
    {
        GRZEarmuffsRestore();
        if (m_GRZEarmuffsPopup)
            m_GRZEarmuffsPopup.Unlink();
        m_GRZEarmuffsPopup = null;
        m_GRZEarmuffsLayoutAttempted = false;
        m_GRZEarmuffsEnabled = false;
        super.OnMissionFinish();
    }
}
#endif

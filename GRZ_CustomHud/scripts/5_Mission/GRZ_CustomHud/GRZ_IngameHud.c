// Own status layout avoids native spacer clipping and rectangular backplates.
modded class IngameHud
{
    protected Widget m_GRZStatusPanel;
    protected int m_GRZHealthStatus;
    protected int m_GRZBloodStatus;
    protected float m_GRZPulse;

    override void Init(Widget hud_panel_widget)
    {
        super.Init(hud_panel_widget);
        m_GRZStatusPanel = g_Game.GetWorkspace().CreateWidgets("GRZ_CustomHud/gui/layouts/grz_status.layout", m_HudPanelWidget);
        if (!m_GRZStatusPanel)
            return;
        ImageWidget health = ImageWidget.Cast(m_GRZStatusPanel.FindAnyWidget("GRZHealthIcon"));
        ImageWidget blood = ImageWidget.Cast(m_GRZStatusPanel.FindAnyWidget("GRZBloodIcon"));
        if (health)
        {
            health.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
            health.LoadImageFile(0, "GRZ_CustomHud/gui/textures/grz_health_ca.paa");
        }
        if (blood)
        {
            blood.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
            blood.LoadImageFile(0, "GRZ_CustomHud/gui/textures/grz_blood_ca.paa");
        }
    }

    override void DisplayNotifier(int key, int tendency, int status)
    {
        super.DisplayNotifier(key, tendency, status);
        if (key == NTFKEY_HEALTHY)
        {
            m_GRZHealthStatus = status;
        }
        if (key == NTFKEY_BLOOD)
        {
            m_GRZBloodStatus = status;
        }
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (!m_HudPanelWidget || !m_GRZStatusPanel)
            return;
        m_GRZPulse += timeslice;
        GRZHide(m_Notifiers);
        GRZHide(m_QuickbarWidget);
        GRZHide(m_Stamina);
        GRZHide(m_StaminaBackground);
        GRZHide(m_Presence);
        GRZHide(m_Badges);
        GRZHide(m_BloodType);
        GRZHide(m_BloodPosType);
        GRZHide(m_VehicleHudPanels);
        GRZHide(m_WalkieTalkie);

        bool show = !GetHudVisibility().IsContextFlagActive(IngameHudVisibility.HUD_HIDE_FLAGS);
        m_GRZStatusPanel.Show(show);
        float x, y, width, height;
        m_HudPanelWidget.GetScreenPos(x, y);
        m_HudPanelWidget.GetScreenSize(width, height);
        float scale = Math.Clamp(height / 1080.0, 0.75, 2.0);
        m_GRZStatusPanel.SetScreenPos(x + width - 232 * scale, y + height - 132 * scale);
        m_GRZStatusPanel.SetScreenSize(208 * scale, 96 * scale);
        GRZStatus("Health", m_GRZHealthStatus);
        GRZStatus("Blood", m_GRZBloodStatus);
    }

    protected void GRZHide(Widget widget)
    {
        if (widget)
            widget.Show(false);
    }

    protected void GRZStatus(string name, int status)
    {
        ImageWidget icon = ImageWidget.Cast(m_GRZStatusPanel.FindAnyWidget("GRZ" + name + "Icon"));
        if (icon)
        {
            float alpha = 1.0;
            if (status >= 5)
                alpha = 0.4 + 0.6 * Math.AbsFloat(Math.Sin(m_GRZPulse * 3.0));
            icon.SetColor(ARGB(alpha * 255, 255, 255, 255));
        }

    }
};

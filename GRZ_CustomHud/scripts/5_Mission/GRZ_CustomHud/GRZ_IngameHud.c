// Own status layout avoids native spacer clipping and rectangular backplates.
modded class IngameHud
{
    protected Widget m_GRZStatusPanel;
    protected int m_GRZHealthStatus;
    protected int m_GRZBloodStatus;
    protected float m_GRZPulse;
    protected int m_GRZHealthTrend;
    protected int m_GRZBloodTrend;
    protected int m_GRZCuts;
    protected Widget m_GRZCutIcon;
    protected TextWidget m_GRZCutCount;
    protected ref array<Widget> m_GRZHealthArrows = new array<Widget>;
    protected ref array<Widget> m_GRZBloodArrows = new array<Widget>;
    protected Widget m_GRZStaminaPanel;
    protected Widget m_GRZStaminaFill;
    protected Widget m_GRZStaminaCapacityBar;
    protected float m_GRZStaminaFraction;
    protected float m_GRZStaminaCapacity = 1.0;

    override void Init(Widget hud_panel_widget)
    {
        super.Init(hud_panel_widget);
        m_GRZStatusPanel = g_Game.GetWorkspace().CreateWidgets("GRZ_CustomHud/gui/layouts/grz_status.layout", m_HudPanelWidget);
        if (!m_GRZStatusPanel)
            return;
        m_GRZStaminaPanel = g_Game.GetWorkspace().CreateWidgets("GRZ_CustomHud/gui/layouts/grz_stamina.layout", m_HudPanelWidget);
        if (m_GRZStaminaPanel)
        {
            m_GRZStaminaFill = m_GRZStaminaPanel.FindAnyWidget("GRZStaminaFill");
            m_GRZStaminaCapacityBar = m_GRZStaminaPanel.FindAnyWidget("GRZStaminaCapacity");
        }
        GRZUpdateStamina();
        GRZBindStatusWidgets();
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
            m_GRZHealthTrend = tendency;
        }
        if (key == NTFKEY_BLOOD)
        {
            m_GRZBloodStatus = status;
            m_GRZBloodTrend = tendency;
        }
    }

    override void DisplayBadge(int key, int value)
    {
        super.DisplayBadge(key, value);
        if (key == NTFKEY_BLEEDISH)
            m_GRZCuts = Math.Max(0, value);
    }

    protected void GRZBindStatusWidgets()
    {
        // Reuse the game's loaded arrow and bleeding artwork rather than
        // guessing texture names or relying on a font's arrow glyphs.
        GRZBindArrows("Health", m_GRZHealthArrows);
        GRZBindArrows("Blood", m_GRZBloodArrows);
        if (m_Badges)
        {
            m_GRZCutIcon = m_Badges.FindAnyWidget("Bleeding");
            m_GRZCutCount = TextWidget.Cast(m_Badges.FindAnyWidget("BleedingCount"));
            if (m_GRZCutIcon)
            {
                m_GRZStatusPanel.AddChild(m_GRZCutIcon);
                m_GRZCutIcon.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNOREPOINTER);
                m_GRZCutIcon.SetColor(0xFFFFFFFF);
                m_GRZCutIcon.Show(false);
            }
            if (m_GRZCutCount)
            {
                m_GRZStatusPanel.AddChild(m_GRZCutCount);
                m_GRZCutCount.SetColor(0xFFFFFFFF);
                m_GRZCutCount.SetOutline(1, 0xFF000000);
                m_GRZCutCount.Show(false);
            }
        }
    }

    protected void GRZBindArrows(string name, array<Widget> arrows)
    {
        arrows.Clear();
        for (int direction = 0; direction < 2; direction++)
        {
            string suffix = "ArrowUp";
            if (direction == 1)
                suffix = "ArrowDown";
            for (int level = 1; level <= 3; level++)
            {
                Widget arrow;
                if (m_Notifiers)
                    arrow = m_Notifiers.FindAnyWidget(name + suffix + level.ToString());
                arrows.Insert(arrow);
                if (arrow)
                {
                    m_GRZStatusPanel.AddChild(arrow);
                    arrow.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNOREPOINTER);
                    arrow.SetColor(0xFFFFFFFF);
                    arrow.Show(false);
                }
            }
        }
    }

    protected void GRZUpdateArrows(array<Widget> arrows, int tendency, float x, float y, float scale)
    {
        int magnitude = Math.Clamp(Math.AbsInt(tendency), 0, 3);
        int selected = -1;
        if (tendency > 0)
            selected = magnitude - 1;
        else if (tendency < 0)
            selected = magnitude + 2;
        for (int i = 0; i < arrows.Count(); i++)
        {
            Widget arrow = arrows[i];
            if (!arrow)
                continue;
            arrow.SetScreenSize(32 * scale, 20 * scale);
            arrow.SetScreenPos(x, y);
            arrow.Show(i == selected);
        }
    }

    protected void GRZUpdateCuts(float x, float y, float scale)
    {
        if (m_GRZCutIcon)
        {
            m_GRZCutIcon.SetScreenSize(24 * scale, 28 * scale);
            m_GRZCutIcon.SetScreenPos(x + 2 * scale, y + 34 * scale);
            m_GRZCutIcon.Show(m_GRZCuts > 0);
        }
        if (m_GRZCutCount)
        {
            m_GRZCutCount.SetScreenSize(22 * scale, 28 * scale);
            m_GRZCutCount.SetScreenPos(x + 26 * scale, y + 34 * scale);
            m_GRZCutCount.SetTextExactSize(Math.Round(18 * scale));
            m_GRZCutCount.SetText(m_GRZCuts.ToString());
            m_GRZCutCount.Show(m_GRZCuts > 0);
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
        m_GRZStatusPanel.SetScreenPos(x + width - 280 * scale, y + height - 132 * scale);
        m_GRZStatusPanel.SetScreenSize(256 * scale, 120 * scale);
        float statusX, statusY;
        m_GRZStatusPanel.GetScreenPos(statusX, statusY);
        GRZUpdateArrows(m_GRZHealthArrows, m_GRZHealthTrend, statusX + 192 * scale, statusY + 96 * scale, scale);
        GRZUpdateArrows(m_GRZBloodArrows, m_GRZBloodTrend, statusX + 80 * scale, statusY + 96 * scale, scale);
        GRZUpdateCuts(statusX, statusY, scale);
        if (m_GRZStaminaPanel)
        {
            m_GRZStaminaPanel.Show(show);
            Widget weaponPanel = m_HudPanelWidget.FindAnyWidget("GRZWeaponPanel");
            float barX = x + 104;
            float barY = y + height - 32;
            if (weaponPanel && weaponPanel.IsVisible())
            {
                float weaponX, weaponY;
                weaponPanel.GetScreenPos(weaponX, weaponY);
                barX = weaponX + 80;
                barY = weaponY + 100;
            }
            m_GRZStaminaPanel.SetScreenPos(barX, barY);
            m_GRZStaminaPanel.SetScreenSize(300, 8);
        }
        GRZStatus("Health", m_GRZHealthStatus);
        GRZStatus("Blood", m_GRZBloodStatus);
        GRZUpdateStamina();
    }

    override void SetStamina(int value, int range)
    {
        super.SetStamina(value, range);
        float maximum = CfgGameplayHandler.GetStaminaMax();
        if (maximum <= 0)
            maximum = Math.Max(range, 1.0);
        m_GRZStaminaFraction = 0;
        m_GRZStaminaCapacity = 0;
        if (maximum > 0)
        {
            m_GRZStaminaCapacity = Math.Clamp(range / maximum, 0, 1);
            m_GRZStaminaFraction = Math.Clamp(value / maximum, 0, m_GRZStaminaCapacity);
        }
        GRZUpdateStamina();
    }

    protected void GRZUpdateStamina()
    {
        float innerWidth = 298.0 / 300.0;
        if (m_GRZStaminaCapacityBar)
        {
            m_GRZStaminaCapacityBar.SetSize(innerWidth * m_GRZStaminaCapacity, 0.75);
            m_GRZStaminaCapacityBar.Show(m_GRZStaminaCapacity > 0);
        }
        if (m_GRZStaminaFill)
        {
            m_GRZStaminaFill.SetSize(innerWidth * m_GRZStaminaFraction, 0.75);
            m_GRZStaminaFill.Show(m_GRZStaminaFraction > 0);
        }
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

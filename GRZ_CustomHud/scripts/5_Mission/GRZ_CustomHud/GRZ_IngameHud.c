// Own status layout avoids native spacer clipping and rectangular backplates.
modded class IngameHud
{
    protected Widget m_GRZStatusPanel;
    protected int m_GRZHealthStatus;
    protected int m_GRZBloodStatus;
    protected int m_GRZHealthTendency;
    protected int m_GRZBloodTendency;
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
            health.LoadImageFile(0, "GRZ_CustomHud/gui/textures/grz_health_ca.paa");
        if (blood)
            blood.LoadImageFile(0, "GRZ_CustomHud/gui/textures/grz_blood_ca.paa");
    }

    override void DisplayNotifier(int key, int tendency, int status)
    {
        super.DisplayNotifier(key, tendency, status);
        if (key == NTFKEY_HEALTHY)
        {
            m_GRZHealthStatus = status;
            m_GRZHealthTendency = tendency;
        }
        if (key == NTFKEY_BLOOD)
        {
            m_GRZBloodStatus = status;
            m_GRZBloodTendency = tendency;
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
        m_GRZStatusPanel.SetScreenPos(x + width - 264 * scale, y + height - 164 * scale);
        m_GRZStatusPanel.SetScreenSize(240 * scale, 128 * scale);
        GRZStatus("Health", m_GRZHealthStatus, m_GRZHealthTendency);
        GRZStatus("Blood", m_GRZBloodStatus, m_GRZBloodTendency);
    }

    protected void GRZHide(Widget widget)
    {
        if (widget)
            widget.Show(false);
    }

    protected void GRZStatus(string name, int status, int tendency)
    {
        ImageWidget icon = ImageWidget.Cast(m_GRZStatusPanel.FindAnyWidget("GRZ" + name + "Icon"));
        TextWidget label = TextWidget.Cast(m_GRZStatusPanel.FindAnyWidget("GRZ" + name + "State"));
        if (icon)
        {
            float alpha = 1.0;
            if (status >= 5)
                alpha = 0.4 + 0.6 * Math.AbsFloat(Math.Sin(m_GRZPulse * 3.0));
            icon.SetColor(ARGB(alpha * 255, 255, 255, 255));
        }
        // Use DayZ's actual replicated notifier stages, never guessed percentages.
        string state;
        switch (status)
        {
            case 1: state = "HIGH"; break;
            case 2: state = "GOOD"; break;
            case 3: state = "LOW"; break;
            case 4: state = "VERY LOW"; break;
            case 5: state = "CRITICAL"; break;
        }
        if (tendency > 0)
            state += " +";
        if (tendency < 0)
            state += " -";
        if (label)
            label.SetText(state);
    }
};

// Native vector ornament: no external texture conversion is required.
class GRZThornFrame
{
    static CanvasWidget Create(Widget parent)
    {
        // CanvasWidget has no script-visible type ID. Create it from a layout,
        // matching the layout-based canvas creation used by DayZ itself.
        Widget root = g_Game.GetWorkspace().CreateWidgets("GRZ_CustomHud/gui/layouts/grz_thorn_canvas.layout", parent);
        CanvasWidget canvas = CanvasWidget.Cast(root);
        if (!canvas)
            Error("GRZ Custom Hud: could not load the thorn canvas layout");
        return canvas;
    }

    static void Draw(CanvasWidget canvas, float width, float height)
    {
        canvas.Clear();
        float inset = height * 0.12;
        float span = Math.Min(width * 0.28, height * 0.60);
        float stroke = Math.Max(1.0, height * 0.022);
        float right = width - inset;
        float bottom = height - inset;
        int black = 0xFF0A0A0A;
        // Fine connecting rails leave the center transparent and readable.
        canvas.DrawLine(inset + span, inset, right - span, inset, stroke, black);
        canvas.DrawLine(inset + span, bottom, right - span, bottom, stroke, black);
        canvas.DrawLine(inset, inset + span, inset, bottom - span, stroke, black);
        canvas.DrawLine(right, inset + span, right, bottom - span, stroke, black);
        Corner(canvas, inset, inset, 1, 1, span, inset, stroke);
        Corner(canvas, right, inset, -1, 1, span, inset, stroke);
        Corner(canvas, inset, bottom, 1, -1, span, inset, stroke);
        Corner(canvas, right, bottom, -1, -1, span, inset, stroke);
    }

    protected static void Corner(CanvasWidget canvas, float x, float y, float sx, float sy, float span, float inset, float stroke)
    {
        Curve(canvas, x, y, sx, sy, span, 0, span * 0.3, 0, 0, -inset * 0.2, 0, span, stroke * 2.5);
        // Curved, tapered hooks replace the old crossed fence-wire barbs.
        Curve(canvas, x, y, sx, sy, span * 0.16, 0, span * 0.25, span * 0.55, span * 0.7, span * 0.35, span * 0.58, span * 0.12, stroke * 3.4);
        Curve(canvas, x, y, sx, sy, 0, span * 0.18, span * 0.5, span * 0.3, span * 0.38, span * 0.8, span * 0.12, span * 0.65, stroke * 3.4);
        Curve(canvas, x, y, sx, sy, span * 0.46, 0, span * 0.4, -inset * 0.85, span * 0.85, -inset * 0.85, span, -inset * 0.5, stroke * 2.3);
        Curve(canvas, x, y, sx, sy, 0, span * 0.5, -inset * 0.85, span * 0.45, -inset * 0.85, span * 0.85, -inset * 0.5, span, stroke * 2.3);
    }

    protected static void Curve(CanvasWidget canvas, float ox, float oy, float sx, float sy, float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy, float thickness)
    {
        float previousX = ox + sx * ax;
        float previousY = oy + sy * ay;
        for (int i = 1; i <= 16; i++)
        {
            float t = i / 16.0;
            float u = 1.0 - t;
            float px = u * u * u * ax + 3 * u * u * t * bx + 3 * u * t * t * cx + t * t * t * dx;
            float py = u * u * u * ay + 3 * u * u * t * by + 3 * u * t * t * cy + t * t * t * dy;
            float nextX = ox + sx * px;
            float nextY = oy + sy * py;
            canvas.DrawLine(previousX, previousY, nextX, nextY, Math.Max(0.65, thickness * (1.0 - t)), 0xFF0A0A0A);
            previousX = nextX;
            previousY = nextY;
        }
    }
};

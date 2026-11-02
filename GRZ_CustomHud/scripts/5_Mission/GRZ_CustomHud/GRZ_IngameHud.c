// Extend the stock HUD: keep DayZ's replicated status images and tendency arrows.
modded class IngameHud
{
    protected CanvasWidget m_GRZHealthWire;
    protected CanvasWidget m_GRZBloodWire;
    protected float m_GRZPulse;

    override void Init(Widget hud_panel_widget)
    {
        super.Init(hud_panel_widget);
        if (!m_Notifiers)
            return;

        m_GRZHealthWire = GRZThornFrame.Create(m_Notifiers);
        m_GRZBloodWire = GRZThornFrame.Create(m_Notifiers);
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (!m_HudPanelWidget || !m_Notifiers)
            return;

        m_GRZPulse += timeslice;
        // Reapply after vanilla visibility updates (inventory, stance and stamina).
        GRZHide(m_QuickbarWidget);
        GRZHide(m_Stamina);
        GRZHide(m_StaminaBackground);
        GRZHide(m_Presence);
        GRZHide(m_Badges);
        GRZHide(m_BloodType);
        GRZHide(m_BloodPosType);
        GRZHide(m_VehicleHudPanels);
        GRZHide(m_WalkieTalkie);
        GRZBlack(m_StanceProne);
        GRZBlack(m_StanceCrouch);
        GRZBlack(m_StanceStand);
        GRZBlack(m_StanceStandWalk);
        GRZBlack(m_StanceCar);

        // Keep the native hierarchy intact so all stock status updates still work.
        Widget child = m_Notifiers.GetChildren();
        while (child)
        {
            bool keep = child == m_GRZHealthWire || child == m_GRZBloodWire;
            keep = keep || child.GetName() == "IconHealth" || child.GetName() == "IconBlood";
            keep = keep || child.FindAnyWidget("IconHealth") != null || child.FindAnyWidget("IconBlood") != null;
            if (!keep)
                child.Show(false);
            child = child.GetSibling();
        }

        GRZStyleStatus("Health", m_GRZHealthWire);
        GRZStyleStatus("Blood", m_GRZBloodWire);
    }

    protected void GRZHide(Widget widget)
    {
        if (widget)
            widget.Show(false);
    }

    protected void GRZBlack(Widget widget)
    {
        if (widget)
            widget.SetColor(0xFF0A0A0A);
    }

    protected void GRZStyleStatus(string status, CanvasWidget wire)
    {
        ImageWidget icon = ImageWidget.Cast(m_Notifiers.FindAnyWidget("Icon" + status));
        if (!icon || !wire)
            return;

        float alpha = 1.0;
        // Black stays black even at critical levels; pulse opacity instead of red.
        if (m_TendencyStatusCritical.Contains(icon))
            alpha = 0.4 + 0.6 * Math.AbsFloat(Math.Sin(m_GRZPulse * 3.0));
        icon.SetColor(ARGB(alpha * 255, 10, 10, 10));
        for (int i = 1; i <= 3; i++)
        {
            Widget up = m_Notifiers.FindAnyWidget(status + "ArrowUp" + i.ToString());
            Widget down = m_Notifiers.FindAnyWidget(status + "ArrowDown" + i.ToString());
            if (up)
                up.SetColor(0xFF0A0A0A);
            if (down)
                down.SetColor(0xFF0A0A0A);
        }

        float x, y, w, h;
        icon.GetScreenPos(x, y);
        icon.GetScreenSize(w, h);
        float padding = h * 0.25;
        wire.SetScreenPos(x - padding, y - padding);
        wire.SetScreenSize(w + padding * 2, h + padding * 2);
        wire.Show(icon.IsVisible());
        GRZThornFrame.Draw(wire, w + padding * 2, h + padding * 2);
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

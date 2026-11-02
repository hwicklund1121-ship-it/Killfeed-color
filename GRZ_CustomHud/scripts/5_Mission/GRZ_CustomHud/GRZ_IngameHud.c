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

        int flags = WidgetFlags.VISIBLE | WidgetFlags.IGNOREPOINTER | WidgetFlags.EXACTPOS | WidgetFlags.EXACTSIZE | WidgetFlags.BLEND;
        m_GRZHealthWire = CanvasWidget.Cast(g_Game.GetWorkspace().CreateWidget(CanvasWidgetTypeID, 0, 0, 1, 1, flags, 0xFFFFFFFF, 0, m_Notifiers));
        m_GRZBloodWire = CanvasWidget.Cast(g_Game.GetWorkspace().CreateWidget(CanvasWidgetTypeID, 0, 0, 1, 1, flags, 0xFFFFFFFF, 0, m_Notifiers));
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
        GRZWireFrame.Draw(wire, w + padding * 2, h + padding * 2);
    }
};

class GRZWireFrame
{
    static void Draw(CanvasWidget canvas, float width, float height)
    {
        canvas.Clear();
        float inset = height * 0.12;
        float left = inset;
        float top = inset;
        float right = width - inset;
        float bottom = height - inset;
        float stroke = Math.Max(1.0, height * 0.025);
        int black = 0xFF0A0A0A;
        // Two restrained strands, with four small opposed barbs per indicator.
        canvas.DrawLine(left, top, right, top + 1, stroke, black);
        canvas.DrawLine(left, top + 3, right, top + 2, stroke, black);
        canvas.DrawLine(left, bottom, right, bottom - 1, stroke, black);
        canvas.DrawLine(left, bottom - 3, right, bottom - 2, stroke, black);
        canvas.DrawLine(left, top, left, bottom, stroke, black);
        canvas.DrawLine(right, top, right, bottom, stroke, black);
        float barb = height * 0.07;
        for (int i = 1; i <= 2; i++)
        {
            float bx = left + (right - left) * i / 3;
            canvas.DrawLine(bx - barb, top - barb, bx + barb, top + barb, stroke, black);
            canvas.DrawLine(bx - barb, top + barb, bx + barb, top - barb, stroke, black);
            canvas.DrawLine(bx - barb, bottom - barb, bx + barb, bottom + barb, stroke, black);
            canvas.DrawLine(bx - barb, bottom + barb, bx + barb, bottom - barb, stroke, black);
        }
    }
};

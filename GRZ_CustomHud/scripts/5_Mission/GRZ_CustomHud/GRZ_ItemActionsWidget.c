modded class ItemActionsWidget
{
    protected CanvasWidget m_GRZWeaponWire;

    override protected void OnWidgetScriptInit(Widget w)
    {
        super.OnWidgetScriptInit(w);
        int flags = WidgetFlags.VISIBLE | WidgetFlags.IGNOREPOINTER | WidgetFlags.EXACTPOS | WidgetFlags.EXACTSIZE | WidgetFlags.BLEND;
        m_GRZWeaponWire = CanvasWidget.Cast(g_Game.GetWorkspace().CreateWidget(CanvasWidgetTypeID, 0, 0, 1, 1, flags, 0xFFFFFFFF, 0, m_Root));
    }

    override protected void BuildCursor()
    {
        super.BuildCursor();
        // Vanilla owns ammo counts, jam/chamber information and reload actions.
        // Only presentation changes; no replacement weapon calculations.
        GRZTintTree(m_Root);
        Widget item = m_Root.FindAnyWidget("ia_item");
        if (!item || !m_GRZWeaponWire)
            return;

        bool show = m_EntityInHands != null && item.IsVisible();
        m_GRZWeaponWire.Show(show);
        if (!show)
            return;

        float x, y, width, height;
        item.GetScreenPos(x, y);
        item.GetScreenSize(width, height);
        float padding = height * 0.25;
        m_GRZWeaponWire.SetScreenPos(x - padding, y - padding);
        m_GRZWeaponWire.SetScreenSize(width + padding * 2, height + padding * 2);
        GRZWireFrame.Draw(m_GRZWeaponWire, width + padding * 2, height + padding * 2);
    }

    protected void GRZTintTree(Widget widget)
    {
        // Preserve existing alpha so vanilla fades still work.
        widget.SetColor(ARGB(widget.GetAlpha() * 255, 10, 10, 10));
        Widget child = widget.GetChildren();
        while (child)
        {
            GRZTintTree(child);
            child = child.GetSibling();
        }
    }
};

modded class ItemActionsWidget
{
    protected CanvasWidget m_GRZWeaponWire;

    override protected void OnWidgetScriptInit(Widget w)
    {
        super.OnWidgetScriptInit(w);
        m_GRZWeaponWire = GRZThornFrame.Create(m_Root);
    }

    override protected void BuildCursor()
    {
        super.BuildCursor();
        Widget item = m_Root.FindAnyWidget("ia_item");
        if (!item || !m_GRZWeaponWire)
            return;

        bool show = m_EntityInHands != null && item.IsVisible();
        m_GRZWeaponWire.Show(show);
        if (!show)
            return;

        // Keep native ammo, chamber, jam, fire-mode and reload calculations.
        // Clear old backing images without tinting the whole parent black.
        GRZClearBacking(item);
        Widget details = m_Root.FindAnyWidget("ia_item_subdesc");
        if (details)
            GRZClearBacking(details);
        m_Root.SetColor(0xFFFFFFFF);
        GRZWhiteText(m_Root);

        float x, y, width, height;
        item.GetScreenPos(x, y);
        item.GetScreenSize(width, height);
        float panelWidth = Math.Max(width, 360.0);
        float panelHeight = 88.0;
        float panelY = y + height - panelHeight;
        m_GRZWeaponWire.SetScreenPos(x, panelY);
        m_GRZWeaponWire.SetScreenSize(panelWidth, panelHeight);
        GRZThornFrame.Draw(m_GRZWeaponWire, panelWidth, panelHeight);

        // Position the actual vanilla text inside the ornament, not beside it.
        GRZPlaceText("ia_item_desc", x + 30, panelY + 24, panelWidth - 152, 24);
        GRZPlaceText("ia_item_quantity_text", x + panelWidth - 112, panelY + 24, 80, 24);
        GRZPlaceText("ia_item_subdesc_up", x + 30, panelY + 50, 140, 20);
        GRZPlaceText("ia_item_subdesc_down", x + panelWidth - 112, panelY + 50, 80, 20);
    }

    protected void GRZPlaceText(string name, float x, float y, float width, float height)
    {
        TextWidget text = TextWidget.Cast(m_Root.FindAnyWidget(name));
        if (!text)
            return;
        text.SetScreenPos(x, y);
        text.SetScreenSize(width, height);
        text.SetColor(0xFFFFFFFF);
        text.SetOutline(1, 0xFF000000);
    }

    protected void GRZClearBacking(Widget widget)
    {
        widget.SetColor(0xFFFFFFFF);
        Widget child = widget.GetChildren();
        while (child)
        {
            if (ImageWidget.Cast(child) || ProgressBarWidget.Cast(child))
                child.Show(false);
            else
                GRZClearBacking(child);
            child = child.GetSibling();
        }
    }

    protected void GRZWhiteText(Widget widget)
    {
        TextWidget text = TextWidget.Cast(widget);
        if (text)
        {
            text.SetColor(0xFFFFFFFF);
            text.SetOutline(1, 0xFF000000);
        }
        Widget child = widget.GetChildren();
        while (child)
        {
            GRZWhiteText(child);
            child = child.GetSibling();
        }
    }
};

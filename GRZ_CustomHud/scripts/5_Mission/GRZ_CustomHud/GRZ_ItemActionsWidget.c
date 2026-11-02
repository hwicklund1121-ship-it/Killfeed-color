modded class ItemActionsWidget
{
    protected Widget m_GRZWeaponPanel;
    protected ImageWidget m_GRZWeaponWire;

    override protected void OnWidgetScriptInit(Widget w)
    {
        super.OnWidgetScriptInit(w);
        m_GRZWeaponPanel = g_Game.GetWorkspace().CreateWidgets("GRZ_CustomHud/gui/layouts/grz_weapon_panel.layout", m_Root);
        if (m_GRZWeaponPanel)
        {
            GRZSetTextSize("GRZWeaponName", 18);
            GRZSetTextSize("GRZWeaponAmmo", 18);
            GRZSetTextSize("GRZWeaponMode", 16);
            GRZSetTextSize("GRZWeaponZeroing", 16);
            m_GRZWeaponWire = ImageWidget.Cast(m_GRZWeaponPanel.FindAnyWidget("GRZWeaponOutline"));
            if (m_GRZWeaponWire)
            {
                m_GRZWeaponWire.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
                m_GRZWeaponWire.LoadImageFile(0, "GRZ_CustomHud/gui/textures/grz_weapon_outline_ca.paa");
            }
        }
    }

    override protected void BuildCursor()
    {
        super.BuildCursor();
        if (!m_GRZWeaponPanel || !m_GRZWeaponWire)
            return;

        Widget item = m_Root.FindAnyWidget("ia_item");
        Widget details = m_Root.FindAnyWidget("ia_item_subdesc");
        // The stock parent can draw a backing outside ia_item itself. Clear
        // its own color and hide item-only wrapper branches, keeping actions.
        m_Root.SetColor(0x00000000);
        GRZHideStockBranch(item);
        GRZHideStockBranch(details);
        if (m_ItemLeft)
            m_ItemLeft.Show(false);
        bool show = m_EntityInHands != null && m_Player && !m_Player.GetItemAccessor().IsItemInHandsHidden();
        m_GRZWeaponPanel.Show(show);
        if (!show)
            return;

        float x, y, width, height;
        if (item)
        {
            item.GetScreenPos(x, y);
            item.GetScreenSize(width, height);
        }
        m_GRZWeaponPanel.SetScreenPos(x, y + height - 88);
        m_GRZWeaponPanel.SetScreenSize(440, 88);
        m_GRZWeaponWire.SetPos(0, 0);
        m_GRZWeaponWire.SetScreenSize(440, 88);
        m_GRZWeaponWire.SetSort(-1);

    }

    protected void GRZSetTextSize(string name, int size)
    {
        TextWidget text = TextWidget.Cast(m_GRZWeaponPanel.FindAnyWidget(name));
        if (text)
            text.SetTextExactSize(size);
    }

    protected void GRZHideStockBranch(Widget widget)
    {
        if (!widget)
            return;
        Widget branch = widget;
        Widget parent = branch.GetParent();
        while (parent && parent != m_Root)
        {
            // Stop before a shared container: reload and action prompts stay.
            if (parent.FindAnyWidget("ia_interact") || parent.FindAnyWidget("ia_continuous_interact") || parent.FindAnyWidget("ia_single") || parent.FindAnyWidget("ia_continuous") || parent.FindAnyWidget("GRZWeaponPanel"))
                break;
            branch = parent;
            parent = branch.GetParent();
        }
        branch.Show(false);
    }

    // Route stock data setters into our own transparent layout. Native widgets
    // are left intact for engine lookups, but never used for visible backing.
    override protected void SetItemDesc(EntityAI entity, string descText, string itemWidget, string descWidget)
    {
        if (!m_GRZWeaponPanel)
            return;
        super.SetItemDesc(entity, descText, "GRZWeaponPanel", "GRZWeaponName");
    }

    override protected void SetWeaponQuantity(int chamber, int mag, string itemWidget, string quantityPBWidget, string quantityTextWidget, bool enabled)
    {
        if (!m_GRZWeaponPanel)
            return;
        super.SetWeaponQuantity(chamber, mag, "GRZWeaponPanel", "GRZWeaponProgress", "GRZWeaponAmmo", enabled);
    }

    override protected void SetItemQuantity(int type, float current, int min, int max, string itemWidget, string quantityPBWidget, string quantityTextWidget, bool enabled)
    {
        if (!m_GRZWeaponPanel)
            return;
        super.SetItemQuantity(type, current, min, max, "GRZWeaponPanel", "GRZWeaponProgress", "GRZWeaponAmmo", enabled);
        // Keep the overlay outline-only, even for liquid/food held items.
        Widget progress = m_GRZWeaponPanel.FindAnyWidget("GRZWeaponProgress");
        if (progress)
            progress.Show(false);
    }

    override protected void SetWeaponModeAndZeroing(string itemWidget, string upWidget, string downWidget, bool enabled)
    {
        if (!m_GRZWeaponPanel)
            return;
        super.SetWeaponModeAndZeroing("GRZWeaponDetails", "GRZWeaponMode", "GRZWeaponZeroing", enabled);
    }

    override protected void SetRadioFrequency(string freq, string itemWidget, string upWidget, string downWidget, bool enabled)
    {
        if (!m_GRZWeaponPanel)
            return;
        super.SetRadioFrequency(freq, "GRZWeaponDetails", "GRZWeaponMode", "GRZWeaponZeroing", enabled);
    }
};

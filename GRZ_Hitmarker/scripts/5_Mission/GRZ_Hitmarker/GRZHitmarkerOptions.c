class GRZHitmarkerOptions extends ScriptedWidgetEventHandler
{
    protected Widget m_Section;
    protected OptionsMenu m_OptionsMenu;
    protected ref array<ref OptionSelectorMultistate> m_Selectors = new array<ref OptionSelectorMultistate>;
    protected ref GRZHitmarkerVisual m_Preview;
    protected bool m_PreviewHead;

    void GRZHitmarkerOptions(Widget section, OptionsMenu optionsMenu)
    {
        m_Section = section;
        m_OptionsMenu = optionsMenu;
        GRZHitmarkerSettings settings = GRZHitmarkerStore.Current();
        array<string> toggles = {"#options_controls_disabled", "#options_controls_enabled"};
        array<string> colors = new array<string>;
        array<string> sizes = new array<string>;
        array<string> volumes = new array<string>;
        for (int i = 0; i < 12; i++)
            colors.Insert(GRZHitmarkerSettings.ColorName(i));
        for (int size = 25; size <= 150; size += 5)
            sizes.Insert(size.ToString() + "%");
        for (int volume = 0; volume <= 100; volume += 5)
            volumes.Insert(volume.ToString() + "%");
        AddSelector("ShowValue", settings.ShowHitmarkers, toggles);
        AddSelector("DamageValue", settings.ShowDamage, toggles);
        AddSelector("SoundValue", settings.PlaySound, toggles);
        AddSelector("KillsValue", settings.HighlightKills, toggles);
        AddSelector("BodyColorValue", settings.BodyColor, colors);
        AddSelector("HeadColorValue", settings.HeadColor, colors);
        AddSelector("SizeValue", (settings.SizePercent - 25) / 5, sizes);
        AddSelector("VolumeValue", settings.VolumePercent / 5, volumes);
        TextWidget.Cast(section.FindAnyWidget("Heading")).SetTextExactSize(20);
        m_Preview = new GRZHitmarkerVisual(section.FindAnyWidget("PreviewHost"));
        section.SetHandler(this);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshPreview, 50, false);
    }

    void ~GRZHitmarkerOptions()
    {
        if (g_Game)
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(RefreshPreview);
        if (m_Preview)
            m_Preview.Dispose();
    }

    protected void AddSelector(string widgetName, int current, array<string> values)
    {
        OptionSelectorMultistate selector = new OptionSelectorMultistate(m_Section.FindAnyWidget(widgetName), current, this, false, values);
        if (widgetName == "HeadColorValue")
            selector.m_OptionChanged.Insert(HeadColorChanged);
        else if (widgetName == "BodyColorValue")
            selector.m_OptionChanged.Insert(BodyColorChanged);
        else
            selector.m_OptionChanged.Insert(OptionChanged);
        m_Selectors.Insert(selector);
    }

    protected void HeadColorChanged(int index)
    {
        m_PreviewHead = true;
        OptionChanged(index);
    }

    protected void BodyColorChanged(int index)
    {
        m_PreviewHead = false;
        OptionChanged(index);
    }

    protected void OptionChanged(int index)
    {
        RefreshPreview();
        if (m_OptionsMenu)
            m_OptionsMenu.OnChanged();
    }

    GRZHitmarkerSettings Draft()
    {
        GRZHitmarkerSettings settings = new GRZHitmarkerSettings;
        settings.ShowHitmarkers = m_Selectors[0].GetValue() == 1;
        settings.ShowDamage = m_Selectors[1].GetValue() == 1;
        settings.PlaySound = m_Selectors[2].GetValue() == 1;
        settings.HighlightKills = m_Selectors[3].GetValue() == 1;
        settings.BodyColor = m_Selectors[4].GetValue();
        settings.HeadColor = m_Selectors[5].GetValue();
        settings.SizePercent = 25 + m_Selectors[6].GetValue() * 5;
        settings.VolumePercent = m_Selectors[7].GetValue() * 5;
        return settings;
    }

    protected void SetValues(GRZHitmarkerSettings settings)
    {
        m_Selectors[0].SetValue(settings.ShowHitmarkers, false);
        m_Selectors[1].SetValue(settings.ShowDamage, false);
        m_Selectors[2].SetValue(settings.PlaySound, false);
        m_Selectors[3].SetValue(settings.HighlightKills, false);
        m_Selectors[4].SetValue(settings.BodyColor, false);
        m_Selectors[5].SetValue(settings.HeadColor, false);
        m_Selectors[6].SetValue((settings.SizePercent - 25) / 5, false);
        m_Selectors[7].SetValue(settings.VolumePercent / 5, false);
        RefreshPreview();
    }

    void RefreshPreview()
    {
        if (!m_Section || !m_Preview)
            return;
        Widget host = m_Section.FindAnyWidget("PreviewHost");
        float x, y, width, height;
        host.GetScreenPos(x, y);
        host.GetScreenSize(width, height);
        m_Preview.Draw(Draft(), m_PreviewHead, false, 0, x + width * 0.5, y + height * 0.5, 1, true);
    }

    bool IsChanged()
    {
        return !Draft().SameAs(GRZHitmarkerStore.Current());
    }

    void Apply()
    {
        if (!GRZHitmarkerStore.Save(Draft()))
            Error("GRZ Hitmarker: could not save client settings. Check the client profile folder.");
    }

    void Revert()
    {
        SetValues(GRZHitmarkerStore.Current());
    }

    void Defaults()
    {
        SetValues(new GRZHitmarkerSettings);
    }

}

modded class OptionsMenuGame
{
    protected ref GRZHitmarkerOptions m_GRZHitmarkerOptions;

    void OptionsMenuGame(Widget parent, Widget details_root, GameOptions options, OptionsMenu menu)
    {
        Widget settingsRoot = m_Root.FindAnyWidget("game_settings_root");
        if (!settingsRoot)
        {
            Error("GRZ Hitmarker: Options > Game settings container was not found.");
            return;
        }
        Widget section = g_Game.GetWorkspace().CreateWidgets("GRZ_Hitmarker/gui/layouts/options.layout", settingsRoot);
        if (!section)
            return;
        section.SetSort(200);
        m_GRZHitmarkerOptions = new GRZHitmarkerOptions(section, menu);
        settingsRoot.Update();
        m_Root.Update();
        Widget scroll = m_Root.FindAnyWidget("game_settings_scroll");
        if (scroll)
        {
            scroll.SetAlpha(1);
            scroll.Update();
        }
    }

    override bool IsChanged()
    {
        return super.IsChanged() || (m_GRZHitmarkerOptions && m_GRZHitmarkerOptions.IsChanged());
    }

    override void Apply()
    {
        super.Apply();
        if (m_GRZHitmarkerOptions)
            m_GRZHitmarkerOptions.Apply();
    }

    override void Revert()
    {
        super.Revert();
        if (m_GRZHitmarkerOptions)
            m_GRZHitmarkerOptions.Revert();
    }

    override void SetToDefaults()
    {
        super.SetToDefaults();
        if (m_GRZHitmarkerOptions)
            m_GRZHitmarkerOptions.Defaults();
    }
}

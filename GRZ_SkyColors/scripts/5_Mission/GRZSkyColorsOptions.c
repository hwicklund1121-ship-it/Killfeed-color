class GRZSkyColorOptions extends ScriptedWidgetEventHandler
{
    protected Widget m_Section;
    protected OptionsMenu m_Menu;
    protected ref OptionSelectorMultistate m_Color;

    void GRZSkyColorOptions(Widget section, OptionsMenu menu)
    {
        m_Section = section;
        m_Menu = menu;
        array<string> names = new array<string>;
        foreach (GRZSkyColor color : GRZSkyPalette.Current().PaletteEntries)
            names.Insert(color.Name);
        m_Color = new OptionSelectorMultistate(section.FindAnyWidget("SkyColorValue"), GRZSkyStore.Current().Color, this, false, names);
        m_Color.m_OptionChanged.Insert(ColorChanged);
        TextWidget.Cast(section.FindAnyWidget("Heading")).SetTextExactSize(20);
        TextWidget.Cast(section.FindAnyWidget("SkyPreviewNote")).SetTextExactSize(14);
    }

    void ~GRZSkyColorOptions()
    {
        // Covers Cancel, Esc/back, and destruction without Apply.
        if (g_Game && !g_Game.IsDedicatedServer())
            GRZSkyRuntime.EndPreview();
    }

    protected void ColorChanged(int index)
    {
        bool applied = GRZSkyRuntime.Preview(index);
        TextWidget note = TextWidget.Cast(m_Section.FindAnyWidget("SkyPreviewNote"));
        if (applied)
            note.SetText("Live preview. Apply saves; Cancel restores your previous color.");
        else if (!GRZSkyRuntime.Supported())
            note.SetText("Custom sky colors support the default and dark-night lighting presets.");
        else
            note.SetText("Preview could not load. Check the client script log.");
        if (m_Menu)
            m_Menu.OnChanged();
    }

    bool IsChanged()
    {
        return m_Color.GetValue() != GRZSkyStore.Current().Color;
    }

    void Apply()
    {
        if (!GRZSkyStore.Save(m_Color.GetValue()))
        {
            TextWidget.Cast(m_Section.FindAnyWidget("SkyPreviewNote")).SetText("Could not save sky color. Check the client profile folder.");
            return;
        }
        GRZSkyRuntime.EndPreview();
    }

    void Revert()
    {
        m_Color.SetValue(GRZSkyStore.Current().Color, false);
        GRZSkyRuntime.EndPreview();
        TextWidget.Cast(m_Section.FindAnyWidget("SkyPreviewNote")).SetText("Previous sky color restored.");
    }

    void Defaults()
    {
        m_Color.SetValue(0, false);
        ColorChanged(0);
    }
}

modded class OptionsMenuGame
{
    protected ref GRZSkyColorOptions m_GRZSkyColorOptions;

    void OptionsMenuGame(Widget parent, Widget details_root, GameOptions options, OptionsMenu menu)
    {
        Widget settingsRoot = m_Root.FindAnyWidget("game_settings_root");
        if (!settingsRoot)
        {
            Error("GRZ Sky colors: Options > Game settings container not found.");
            return;
        }
        Widget section = g_Game.GetWorkspace().CreateWidgets("GRZ_SkyColors/gui/layouts/options.layout", settingsRoot);
        if (!section)
        {
            Error("GRZ Sky colors: settings layout could not load.");
            return;
        }
        section.SetSort(210); // Immediately below GRZ Hit Marker (200).
        m_GRZSkyColorOptions = new GRZSkyColorOptions(section, menu);
        settingsRoot.Update();
        m_Root.Update();
        Widget scroll = m_Root.FindAnyWidget("game_settings_scroll");
        if (scroll)
            scroll.Update();
    }

    override bool IsChanged()
    {
        return super.IsChanged() || (m_GRZSkyColorOptions && m_GRZSkyColorOptions.IsChanged());
    }

    override void Apply()
    {
        super.Apply();
        if (m_GRZSkyColorOptions)
            m_GRZSkyColorOptions.Apply();
    }

    override void Revert()
    {
        super.Revert();
        if (m_GRZSkyColorOptions)
            m_GRZSkyColorOptions.Revert();
    }

    override void SetToDefaults()
    {
        super.SetToDefaults();
        if (m_GRZSkyColorOptions)
            m_GRZSkyColorOptions.Defaults();
    }
}

#ifndef SERVER
modded class MissionGameplay
{
    override void OnMissionStart()
    {
        super.OnMissionStart();
        GRZSkyRuntime.ApplyCurrent();
    }

    override void OnMissionFinish()
    {
        GRZSkyRuntime.RestoreBase();
        super.OnMissionFinish();
    }
}
#endif

modded class OptionsMenu
{
    override void OnHide()
    {
        super.OnHide();
        // Native menus can be hidden/cached rather than destroyed on Back.
        GRZSkyRuntime.EndPreview();
    }
}

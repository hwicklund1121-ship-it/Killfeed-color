// Client camera/UI only. Dedicated servers use their native implementations.
#ifndef SERVER
// Original GRZ panel embedded in the native Settings > Game tab.
class GRZCameraSettingsPanel extends ScriptedWidgetEventHandler
{
    private Widget m_Root;
    private OptionsMenu m_Menu;
    private float m_SavedFov;
    private float m_SavedView;
    private float m_SavedObjects;
    private Widget m_FovFill;
    private Widget m_ViewFill;
    private Widget m_ObjectsFill;
    private Widget m_FovMarker;
    private Widget m_ViewMarker;
    private Widget m_ObjectsMarker;
    private SliderWidget m_Fov;
    private SliderWidget m_View;
    private SliderWidget m_Objects;
    private TextWidget m_FovValue;
    private TextWidget m_ViewValue;
    private TextWidget m_ObjectsValue;
    private TextWidget m_Status;

    void GRZCameraSettingsPanel(Widget parent, OptionsMenu menu)
    {
        m_Root = GetGame().GetWorkspace().CreateWidgets("GRZ_Movement/gui/layouts/grz_camera.layout", parent);
        m_Fov = SliderWidget.Cast(m_Root.FindAnyWidget("GRZFov"));
        m_View = SliderWidget.Cast(m_Root.FindAnyWidget("GRZView"));
        m_Objects = SliderWidget.Cast(m_Root.FindAnyWidget("GRZObjects"));
        m_FovValue = TextWidget.Cast(m_Root.FindAnyWidget("GRZFovValue"));
        m_ViewValue = TextWidget.Cast(m_Root.FindAnyWidget("GRZViewValue"));
        m_ObjectsValue = TextWidget.Cast(m_Root.FindAnyWidget("GRZObjectsValue"));
        m_Status = TextWidget.Cast(m_Root.FindAnyWidget("GRZStatus"));
        m_FovFill = m_Root.FindAnyWidget("GRZFovFill");
        m_ViewFill = m_Root.FindAnyWidget("GRZViewFill");
        m_ObjectsFill = m_Root.FindAnyWidget("GRZObjectsFill");
        m_FovFill.SetFlags(WidgetFlags.IGNOREPOINTER);
        m_ViewFill.SetFlags(WidgetFlags.IGNOREPOINTER);
        m_ObjectsFill.SetFlags(WidgetFlags.IGNOREPOINTER);
        m_FovValue.SetOutline(1, ARGB(255, 0, 0, 0));
        m_ViewValue.SetOutline(1, ARGB(255, 0, 0, 0));
        m_ObjectsValue.SetOutline(1, ARGB(255, 0, 0, 0));
        m_FovMarker = m_Root.FindAnyWidget("GRZFovMarker");
        m_ViewMarker = m_Root.FindAnyWidget("GRZViewMarker");
        m_ObjectsMarker = m_Root.FindAnyWidget("GRZObjectsMarker");
        m_FovMarker.SetFlags(WidgetFlags.IGNOREPOINTER);
        m_ViewMarker.SetFlags(WidgetFlags.IGNOREPOINTER);
        m_ObjectsMarker.SetFlags(WidgetFlags.IGNOREPOINTER);
        m_Fov.SetMinMax(0.52, 1.5599);
        m_Fov.SetStep(0.0001);
        m_View.SetMinMax(500, 2000);
        m_View.SetStep(50);
        m_Objects.SetMinMax(500, 2000);
        m_Objects.SetStep(50);
        GRZCameraPreferences saved = GRZCameraPreferences.Current();
        m_Fov.SetCurrent(Math.Clamp(GetDayZGame().GetUserFOV(), 0.52, 1.5599));
        m_View.SetCurrent(saved.ViewDistance);
        m_Objects.SetCurrent(saved.ObjectDistance);
        RefreshValues();
        TextWidget heading = TextWidget.Cast(m_Root.FindAnyWidget("GRZSectionTitle"));
        heading.SetFlags(WidgetFlags.CENTER | WidgetFlags.VCENTER);
        heading.SetTextProportion(0.8);
        m_Root.SetHandler(this);
        m_Menu = menu;
        CaptureSavedValues();
    }

    void ~GRZCameraSettingsPanel()
    {
        GRZCameraPreferences.ClearFovPreview();
    }

    private void PreviewFov()
    {
        if (Math.AbsFloat(m_Fov.GetCurrent() - m_SavedFov) > 0.00005)
            GRZCameraPreferences.PreviewFov(m_Fov.GetCurrent());
        else
            GRZCameraPreferences.ClearFovPreview();
    }

    private void RefreshValues()
    {
        // Objects cannot render farther than the overall view distance.
        if (m_Objects.GetCurrent() > m_View.GetCurrent())
            m_Objects.SetCurrent(m_View.GetCurrent());
        UpdateMarker(m_Fov, m_FovMarker, m_FovFill);
        UpdateMarker(m_View, m_ViewMarker, m_ViewFill);
        UpdateMarker(m_Objects, m_ObjectsMarker, m_ObjectsFill);
        m_FovValue.SetText(m_Fov.GetCurrent().ToString());
        m_ViewValue.SetText(Math.Round(m_View.GetCurrent()).ToString() + " m");
        m_ObjectsValue.SetText(Math.Round(m_Objects.GetCurrent()).ToString() + " m");
    }

    private void UpdateMarker(SliderWidget slider, Widget marker, Widget fill)
    {
        float fraction = (slider.GetCurrent() - slider.GetMin()) / (slider.GetMax() - slider.GetMin());
        fraction = Math.Clamp(fraction, 0, 1);
        marker.SetPos(fraction * 0.988, 0.10);
        fill.SetSize(fraction, 1.0);
    }

    private void StyleText(string name, float proportion)
    {
        TextWidget text = TextWidget.Cast(m_Root.FindAnyWidget(name));
        if (!text)
            return;
        text.SetTextProportion(proportion);
        text.SetBold(false);
        text.SetItalic(false);
        text.SetOutline(0);
        text.SetShadow(0, ARGB(0, 0, 0, 0));
        text.SetColor(ARGB(255, 210, 210, 210));
    }

    void MatchNativeTypography(TextWidget heading, TextWidget label)
    {
        // Fixed proportions avoid the text-bounds ratio that enlarged glyphs.
        StyleText("GRZSectionTitle", 0.65);
        StyleText("GRZFovLabel", 0.55);
        StyleText("GRZViewLabel", 0.55);
        StyleText("GRZObjectsLabel", 0.55);
        StyleText("GRZFovValue", 0.55);
        StyleText("GRZViewValue", 0.55);
        StyleText("GRZObjectsValue", 0.55);
    }

    void SetSectionOrder(int order)
    {
        m_Root.SetSort(order);
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (w == m_Fov || w == m_View || w == m_Objects)
        {
            RefreshValues();
            if (w == m_Fov)
                PreviewFov();
            if (m_Menu)
                m_Menu.OnChanged();
            return true;
        }
        return super.OnChange(w, x, y, finished);
    }


    private void CaptureSavedValues()
    {
        m_SavedFov = m_Fov.GetCurrent();
        m_SavedView = m_View.GetCurrent();
        m_SavedObjects = m_Objects.GetCurrent();
    }

    bool IsChanged()
    {
        return Math.AbsFloat(m_Fov.GetCurrent() - m_SavedFov) > 0.00005 || Math.AbsFloat(m_View.GetCurrent() - m_SavedView) > 0.1 || Math.AbsFloat(m_Objects.GetCurrent() - m_SavedObjects) > 0.1;
    }

    bool Save()
    {
        if (!IsChanged())
        {
            GRZCameraPreferences.ClearFovPreview();
            return true;
        }
        GRZCameraPreferences preferences = new GRZCameraPreferences();
        preferences.Enabled = true;
        preferences.FieldOfView = m_Fov.GetCurrent();
        preferences.ViewDistance = m_View.GetCurrent();
        preferences.ObjectDistance = m_Objects.GetCurrent();
        string error;
        if (!GRZCameraPreferences.SaveAndApply(preferences, error))
        {
            m_Status.Show(true);
            m_Status.SetText("Could not save GRZ settings. Check your DayZ profile folder.");
            Print("[GRZ SETTINGS] " + error);
            return false;
        }
        CaptureSavedValues();
        m_Status.Show(false);
        return true;
    }

    void Revert()
    {
        GRZCameraPreferences.ClearFovPreview();
        m_Fov.SetCurrent(m_SavedFov);
        m_View.SetCurrent(m_SavedView);
        m_Objects.SetCurrent(m_SavedObjects);
        RefreshValues();
    }

    void Defaults()
    {
        m_Fov.SetCurrent(1.0);
        m_View.SetCurrent(2000);
        m_Objects.SetCurrent(2000);
        RefreshValues();
        PreviewFov();
    }
}

modded class OptionsMenuGame
{
    private ref GRZCameraSettingsPanel m_GRZCameraPanel;

    private TextWidget GRZFindText(Widget root, Widget excluded)
    {
        if (!root || root == excluded)
            return null;
        TextWidget text = TextWidget.Cast(root);
        if (text)
            return text;
        Widget child = root.GetChildren();
        while (child)
        {
            TextWidget found = GRZFindText(child, excluded);
            if (found)
                return found;
            child = child.GetSibling();
        }
        return null;
    }

    void OptionsMenuGame(Widget parent, Widget details_root, GameOptions options, OptionsMenu menu)
    {
        GRZCameraPreferences.Load();
        Widget nativeFov = m_Root.FindAnyWidget("fov_setting_option");
        Widget settingsRoot = m_Root.FindAnyWidget("game_settings_root");
        if (nativeFov && nativeFov.GetParent() && settingsRoot)
        {
            Widget cameraRow = nativeFov.GetParent();
            Widget cameraSection = cameraRow;
            // Find the complete category directly beneath the settings stack,
            // rather than inserting our panel into a nested single-row widget.
            while (cameraSection.GetParent() && cameraSection.GetParent() != settingsRoot && cameraSection.GetParent() != m_Root)
            {
                Widget nextParent = cameraSection.GetParent();
                // Never replace a container that also contains General/HUD.
                if (nextParent.FindAnyWidget("language_setting_option") || nextParent.FindAnyWidget("hud_setting_option"))
                    break;
                cameraSection = nextParent;
            }
            if (cameraSection == settingsRoot || cameraSection == m_Root)
            {
                Print("[GRZ SETTINGS] Camera category not found; original UI retained.");
                return;
            }
            float rowWidth, rowHeight, sectionWidth, sectionHeight;
            cameraRow.GetScreenSize(rowWidth, rowHeight);
            cameraSection.GetScreenSize(sectionWidth, sectionHeight);
            if (rowHeight < 20 || rowHeight > 70)
                rowHeight = 32;
            TextWidget nativeLabel = GRZFindText(cameraRow, nativeFov);
            TextWidget nativeHeading = GRZFindText(cameraSection, cameraRow);
            Widget originalChild = cameraSection.GetChildren();
            while (originalChild)
            {
                originalChild.Show(false);
                originalChild = originalChild.GetSibling();
            }
            nativeFov.Enable(false);
            cameraSection.SetScreenSize(sectionWidth, rowHeight * 4);
            m_GRZCameraPanel = new GRZCameraSettingsPanel(cameraSection, menu);
            cameraSection.Update();
            settingsRoot.Update();
            m_GRZCameraPanel.MatchNativeTypography(nativeHeading, nativeLabel);
            Print("[GRZ SETTINGS] Replaced native Camera category: " + cameraSection.GetName());
        }
    }

    override bool IsChanged()
    {
        return super.IsChanged() || (m_GRZCameraPanel && m_GRZCameraPanel.IsChanged());
    }

    override void Revert()
    {
        super.Revert();
        if (m_GRZCameraPanel)
            m_GRZCameraPanel.Revert();
    }

    override void SetToDefaults()
    {
        super.SetToDefaults();
        if (m_GRZCameraPanel)
            m_GRZCameraPanel.Defaults();
    }

    void GRZSaveCameraPreferences()
    {
        if (m_GRZCameraPanel)
            m_GRZCameraPanel.Save();
    }
}

modded class OptionsMenu
{
    // Stock 1.30 uses dialog 1337/result 2 for discarding edited settings.
    override bool OnModalResult(Widget w, int x, int y, int code, int result)
    {
        if (code == 1337 && result == 2 && m_GameTab)
            m_GameTab.Revert();
        return super.OnModalResult(w, x, y, code, result);
    }

    override void Apply()
    {
        super.Apply();
        // Native options may set render distances; apply GRZ after them.
        m_GameTab.GRZSaveCameraPreferences();
        GRZCameraPreferences.ApplyDistances();
        OnChanged();
    }
}

modded class MissionGameplay
{
    override void OnMissionStart()
    {
        super.OnMissionStart();
        GRZCameraPreferences.Load();
        GRZCameraPreferences.ApplyDistances();
    }
}

#endif

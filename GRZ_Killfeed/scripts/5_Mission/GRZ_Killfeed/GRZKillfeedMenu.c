class GRZKillfeedClient
{
    static ref GRZNameStyle LocalStyle = new GRZNameStyle;
    static bool ServerAuthorized;
    static ref GRZKillfeedMenu ActiveMenu;

    static bool CanLocalPreview()
    {
        return !g_Game.IsMultiplayer() || (g_Game.IsServer() && !g_Game.IsDedicatedServer()) || IsCLIParam("GRZKillfeedPreview");
    }

    static void OpenLocalPreview()
    {
        if (!g_Game.IsAppActive())
            return;
        UIScriptedMenu parent = g_Game.GetUIManager().GetMenu();
        GRZKillfeedMenu existing = GRZKillfeedMenu.Cast(parent);
        if (existing)
        {
            ActiveMenu = existing;
            existing.EnsureVisible();
            return;
        }
        ActiveMenu = GRZKillfeedMenu.Cast(g_Game.GetUIManager().EnterScriptedMenu(GRZKillfeedMenu.MENU_ID, parent));
        if (!ActiveMenu || !ActiveMenu.GetLayoutRoot())
        {
            ActiveMenu = null;
            Error("[GRZ Killfeed 0.2.6] Native menu factory did not return an initialized settings menu.");
            g_Game.Chat("GRZ Killfeed: menu creation failed. Check the client script log.", "colorStatusChannel");
            return;
        }
        if (ServerAuthorized && g_Game.IsMultiplayer() && !g_Game.IsServer())
            ActiveMenu.ApplyServerStyle(LocalStyle);
        else
            ActiveMenu.ApplyLocalStyle(LocalStyle);
        ActiveMenu.EnsureVisible();
        Print("[GRZ Killfeed 0.2.6] Native menu registered and retained.");
    }

    static void ReleaseMenu()
    {
        if (ActiveMenu)
            ActiveMenu.CloseSafely();
        ActiveMenu = null;
    }

    static void RequestMenu()
    {
        if (!g_Game.IsAppActive())
            return;
        Print("[GRZ Killfeed] Menu open requested.");
        // Temporary test behavior: show the menu immediately on every client.
        // Authorization is still required before enabling server-side saves.
        OpenLocalPreview();
        // A debug preview launch flag must not skip server saves in multiplayer.
        if (!g_Game.IsMultiplayer() || g_Game.IsServer())
            return;
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if (!player)
            return;
        ScriptRPC rpc = new ScriptRPC;
        rpc.Send(player, GRZKillfeedRPC.REQUEST_MENU, true, null);
    }

    static void TestLocal()
    {
        if (g_Game.IsDedicatedServer() || !g_Game.GetPlayer())
            return;
        Demo(LocalStyle);
        g_Game.Chat("GRZ Killfeed: local test only; this does not add a kill or killstreak.", "colorStatusChannel");
    }

    static void TestOffline()
    {
        TestLocal();
    }

    static void TestStreak()
    {
        if (g_Game.IsDedicatedServer() || !g_Game.GetPlayer())
            return;
        string playerName;
        g_Game.GetPlayerName(playerName);
        GRZKillfeedBus.Streak.Invoke(playerName, 10);
        g_Game.Chat("GRZ Killstreaks: local popup/sound test; streak count is unchanged.", "colorStatusChannel");
    }

    static string InventoryDimensions(string configPath)
    {
        array<int> dimensions = new array<int>;
        g_Game.ConfigGetIntArray(configPath, dimensions);
        if (dimensions.Count() < 2)
            return "not defined";
        return dimensions[0].ToString() + " wide x " + dimensions[1].ToString() + " high";
    }

    static void CheckInventory()
    {
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if (!player)
            return;
        string status = "NOT LOADED";
        if (g_Game.ConfigIsExisting("CfgPatches GRZ_InventorySizes"))
            status = "LOADED";
        g_Game.Chat("GRZ Inventory client patch: " + status, "colorStatusChannel");
        string gunType = "M4A1";
        Weapon_Base held = Weapon_Base.Cast(player.GetHumanInventory().GetEntityInHands());
        if (held)
            gunType = held.GetType();
        g_Game.Chat("GRZ Inventory " + gunType + ": " + InventoryDimensions("CfgWeapons " + gunType + " itemSize"), "colorStatusChannel");
        array<string> slots = {"Body", "Vest", "Legs", "Hips"};
        foreach (string slot : slots)
        {
            EntityAI worn = player.FindAttachmentBySlotName(slot);
            if (worn)
                g_Game.Chat("GRZ Inventory " + slot + " " + worn.GetType() + ": " + InventoryDimensions("CfgVehicles " + worn.GetType() + " itemsCargoSize"), "colorStatusChannel");
        }
    }

    static void Demo(GRZNameStyle style)
    {
        // Client-only preview. Never sends an announcement to other players.
        GRZKillfeedMessage message = new GRZKillfeedMessage;
        g_Game.GetPlayerName(message.Killer);
        message.Victim = "BOT";
        message.GunData.Types = {"M4A1", "ACOGOptic", "M4_RISHndgrd", "M4_OEBttstck", "M4_Suppressor", "Mag_STANAG_30Rnd"};
        message.GunData.Parents = {-1, 0, 0, 0, 0, 0};
        message.GunData.Slots = {-1, -1, -1, -1, -1, -1};
        message.Distance = 175;
        message.Streak = 1;
        message.KillerStyle = style.Clone();
        GRZKillfeedBus.Kills.Invoke(message);
    }
}

modded class ChatInputMenu
{
    protected bool m_GRZVipSubmitted;

    override void OnShow()
    {
        m_GRZVipSubmitted = false;
        super.OnShow();
    }

    protected bool GRZTryHandleVipCommand(bool finished)
    {
        if (!finished || !layoutRoot)
            return false;
        EditBoxWidget input = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("InputEditBoxWidget"));
        if (!input)
            return false;
        string command = input.GetText().Trim();
        command.ToLower();
        if (command != "/grzvip" && command != "/grzvip test" && command != "/killfeed test" && command != "/killfeed streaktest" && command != "/grzvip inventory")
            return false;
        if (m_GRZVipSubmitted)
            return true;
        m_GRZVipSubmitted = true;
        Print("[GRZ Killfeed 0.2.6] Intercepted local VIP command before public/VPP chat.");
        Close();
        if (command == "/grzvip")
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.RequestMenu, 150, false);
        else if (command == "/killfeed streaktest")
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.TestStreak, 150, false);
        else if (command == "/grzvip inventory")
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.CheckInventory, 150, false);
        else
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.TestLocal, 150, false);
        return true;
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (GRZTryHandleVipCommand(finished))
            return true;
        return super.OnChange(w, x, y, finished);
    }
}

class GRZKillfeedMenuGeometry
{
    Widget Target;
    float X;
    float Y;
    float Width;
    float Height;
    int FontSize = 16;
}

class GRZKillfeedMenu extends UIScriptedMenu
{
    static const int MENU_ID = 1647280199;
    ref GRZNameStyle Chosen = new GRZNameStyle;
    bool IsOfflinePreview;
    protected bool m_PrefixTab;
    protected bool m_GRZGunTab;
    protected Widget m_GRZGunPage;
    protected ItemPreviewWidget m_GRZGunPreview;
    protected Widget m_GRZPreviewRoot;
    protected EntityAI m_GRZPreviewGun;
    protected TextListboxWidget m_GRZGunList;
    protected int m_GRZGunCategory;
    protected int m_GRZGunIndex = -1;
    protected int m_GRZWrapIndex;
    protected float m_GRZPreviewYaw;
    protected float m_GRZPreviewPitch;
    protected float m_GRZPreviewZoom = 1.0;
    protected string m_GRZPreviewType;
    protected bool m_GRZPreviewDragging;
    protected int m_GRZPreviewMouseX;
    protected int m_GRZPreviewMouseY;
    protected ref array<int> m_GRZGunChoices = new array<int>;
    protected Widget m_NamePage;
    protected Widget m_PrefixPage;
    protected float m_MenuScale = 1.0;
    protected float m_AppliedScale = -1.0;
    protected ref array<ref GRZKillfeedMenuGeometry> m_Geometry = new array<ref GRZKillfeedMenuGeometry>;
    protected TextWidget m_PrefixPreview;
    protected ImageWidget m_PrefixPreviewIcon;
    protected EditBoxWidget m_PrefixInput;
    protected TextWidget m_Preview;
    protected TextWidget m_Status;
    protected float m_Time;
    protected bool m_LoggedUpdate;
    protected int m_ColorTarget;
    protected bool m_GRZControlsLocked;
    protected bool m_GRZDismissRequested;
    protected bool m_GRZCursorWasVisible;
    protected bool m_GRZWasInactive;

    void GRZKillfeedMenu()
    {
        if (g_Game && g_Game.GetUIManager())
            m_GRZCursorWasVisible = g_Game.GetUIManager().IsCursorVisible();
    }

    override Widget Init()
    {
        if (layoutRoot)
            return layoutRoot;
        layoutRoot = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu.layout");
        if (!layoutRoot)
        {
            Error("GRZ Killfeed: could not create gui/layouts/menu.layout.");
            return null;
        }
        Widget host = layoutRoot.FindAnyWidget("PageHost");
        if (host)
        {
            m_NamePage = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu_name_page.layout", host);
            m_PrefixPage = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu_prefix_page.layout", host);
            m_GRZGunPage = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu_guns_page.layout", host);
        }
        if (!m_NamePage || !m_PrefixPage || !m_GRZGunPage)
        {
            Error("GRZ Killfeed: could not create Name and Prefix pages.");
            layoutRoot.Unlink();
            layoutRoot = null;
            return null;
        }
        Widget nameControls = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu_animation.layout", m_NamePage.FindAnyWidget("AnimationHost"));
        ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("MenuLogo"));
        m_PrefixInput = EditBoxWidget.Cast(m_PrefixPage.FindAnyWidget("PrefixWordInput"));
        m_Status = TextWidget.Cast(layoutRoot.FindAnyWidget("Status"));
        if (!logo || !m_Status || !m_PrefixInput || !nameControls)
        {
            Error("GRZ Killfeed: menu layout is missing required widgets.");
            layoutRoot.Unlink();
            layoutRoot = null;
            return null;
        }
        logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
        logo.LoadImageFile(0, "GRZ_Killfeed/gui/textures/grz_menu_logo_ca.paa");
        logo.SetSort(0);
        m_GRZPreviewRoot = m_GRZGunPage.FindAnyWidget("VIPPreviewViewport");
        m_GRZGunPreview = GRZLoadoutItemPreview.Create(m_GRZPreviewRoot);
        if (!m_GRZGunPreview) Error("[GRZ VIP Preview] Could not create shared Loadout preview widget.");
        m_GRZGunList = TextListboxWidget.Cast(m_GRZGunPage.FindAnyWidget("VIPGunList"));
        if (m_GRZGunPreview && WidgetEventHandler.GetInstance())
        {
            WidgetEventHandler.GetInstance().RegisterOnMouseButtonDown(m_GRZGunPreview, this, "GRZVIPPreviewMouseDown");
            WidgetEventHandler.GetInstance().RegisterOnMouseWheel(m_GRZGunPreview, this, "GRZVIPPreviewWheel");
        }
        GRZApplyPanelTextures(layoutRoot);
        GRZLoadoutClientData.Changed.Insert(GRZReceiveVIPCatalogue);
        GRZLoadoutClientData.Result.Insert(GRZReceiveVIPResult);
        CaptureMenuGeometry(layoutRoot.FindAnyWidget("MenuPanel"));
        RefreshLabels();
        if (IsOfflinePreview)
            m_Status.SetText("Local preview — server access and persistence are not tested.");
        PositionPanel();
        Print("[GRZ Killfeed 0.2.6] Init: layout created.");
        return layoutRoot;
    }

    override bool UseMouse() { return true; }
    override bool UseKeyboard() { return true; }

    override void LockControls()
    {
        if (m_GRZControlsLocked)
            return;
        #ifdef FEATURE_CURSOR
        if (IsCreatedHidden())
            return;
        #endif
        m_GRZControlsLocked = true;
        super.LockControls();
        g_Game.GetMission().AddActiveInputExcludes({"menu"});
    }

    override void UnlockControls()
    {
        if (!m_GRZControlsLocked)
            return;
        m_GRZControlsLocked = false;
        super.UnlockControls();
        g_Game.GetMission().RemoveActiveInputExcludes({"menu"}, true);
        // Editor may have its own cursor without a native parent menu.
        if (m_GRZCursorWasVisible && !GetParentMenu())
            g_Game.GetUIManager().ShowUICursor(true);
    }

    override void Update(float timeslice)
    {
        if (m_GRZDismissRequested)
            return;
        if (!g_Game.IsAppActive())
        {
            GRZOnVIPAppDeactivated();
            return;
        }
        if (m_GRZWasInactive) GRZRestoreVIPFocus();
        super.Update(timeslice);
        m_Time += timeslice;
        if (m_GRZGunTab && m_GRZPreviewGun && m_GRZGunPreview && m_GRZGunPreview.GetItem() != m_GRZPreviewGun) GRZBindVIPPreview();
        GRZUpdateVIPPreviewRotation();
        if (m_GRZGunTab && m_GRZGunList)
        {
            int selectedGun = m_GRZGunList.GetSelectedRow();
            if (selectedGun >= 0 && selectedGun < m_GRZGunChoices.Count() && selectedGun != m_GRZGunIndex)
            { m_GRZGunIndex = selectedGun; GRZRefreshVIPGun(); }
        }
        if (m_Preview)
            m_Preview.SetColor(Chosen.ColorAt(m_Time));
        if (m_PrefixPreview)
        {
            m_PrefixPreview.SetColor(Chosen.ColorAt(m_Time));
            m_PrefixPreviewIcon.SetColor(Chosen.ColorAt(m_Time));
            ImageWidget.Cast(m_PrefixPage.FindAnyWidget("PrefixChoicePreview")).SetColor(Chosen.ColorAt(m_Time));
        }
        PositionPanel();
        if (!m_LoggedUpdate)
        {
            m_LoggedUpdate = true;
            float width, height;
            layoutRoot.GetScreenSize(width, height);
            Print(string.Format("[GRZ Killfeed 0.2.6] Update: root %1 x %2; visible %3.", width, height, layoutRoot.IsVisibleHierarchy()));
        }
    }

    override void OnShow()
    {
        m_GRZDismissRequested = false;
        super.OnShow();
        GRZKillfeedBus.AppDeactivated.Remove(GRZOnVIPAppDeactivated);
        GRZKillfeedBus.AppActivated.Remove(GRZOnVIPAppActivated);
        GRZKillfeedBus.AppDeactivated.Insert(GRZOnVIPAppDeactivated);
        GRZKillfeedBus.AppActivated.Insert(GRZOnVIPAppActivated);
        if (!g_Game.IsAppActive())
        {
            GRZOnVIPAppDeactivated();
            return;
        }
        EnsureVisible();
        Print("[GRZ Killfeed 0.2.6] OnShow called.");
    }

    override void OnHide()
    {
        if (!m_GRZDismissRequested && (m_GRZWasInactive || !g_Game.IsAppActive()))
        {
            GRZOnVIPAppDeactivated();
            super.OnHide();
            return;
        }
        m_GRZDismissRequested = true;
        GRZDisposeVIPGun();
        GRZLoadoutClientData.Changed.Remove(GRZReceiveVIPCatalogue);
        GRZLoadoutClientData.Result.Remove(GRZReceiveVIPResult);
        GRZKillfeedBus.AppDeactivated.Remove(GRZOnVIPAppDeactivated);
        GRZKillfeedBus.AppActivated.Remove(GRZOnVIPAppActivated);
        Print("[GRZ Killfeed 0.2.6] OnHide called.");
        super.OnHide();
    }

    override void OnVisibilityChanged(bool isVisible)
    {
        super.OnVisibilityChanged(isVisible);
        if (!isVisible) m_GRZPreviewDragging = false;
    }

    void CloseSafely()
    {
        if (m_GRZDismissRequested)
            return;
        m_GRZDismissRequested = true;
        if (m_GRZPreviewRoot) m_GRZPreviewRoot.Show(false);
        GRZKillfeedBus.AppDeactivated.Remove(GRZOnVIPAppDeactivated);
        GRZKillfeedBus.AppActivated.Remove(GRZOnVIPAppActivated);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZRestoreVIPFocus);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZKillfeedClient.OpenLocalPreview);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZKillfeedClient.RequestMenu);
        if (layoutRoot)
            layoutRoot.Show(false);
        // Native Close can finish on a later frame. Unlock now, with the
        // guard above preventing the later OnHide from decrementing twice.
        UnlockControls();
        Close();
    }

    void GRZOnVIPAppDeactivated()
    {
        if (m_GRZDismissRequested) return;
        m_GRZWasInactive = true;
        m_GRZPreviewDragging = false;
        if (m_GRZPreviewRoot) m_GRZPreviewRoot.Show(false);
    }

    void GRZOnVIPAppActivated()
    {
        if (m_GRZDismissRequested || !g_Game) return;
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZRestoreVIPFocus);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZRestoreVIPFocus, 1, false);
    }

    void GRZRestoreVIPFocus()
    {
        if (m_GRZDismissRequested || !g_Game.IsAppActive() || g_Game.GetUIManager().GetMenu() != this) return;
        m_GRZWasInactive = false;
        LockControls(); // Guarded: never adds a second native focus lock.
        EnsureVisible();
        g_Game.GetUIManager().ShowUICursor(true);
        if (m_GRZGunTab && m_GRZPreviewGun) GRZBindVIPPreview();
    }

    void EnsureVisible()
    {
        if (!layoutRoot || m_GRZDismissRequested || !g_Game.IsAppActive())
            return;
        PositionPanel();
        layoutRoot.SetSort(100000);
        layoutRoot.Show(true);
        layoutRoot.FindAnyWidget("MenuPanel").Show(true);
    }

    protected Widget ActivePage()
    {
        if (m_PrefixTab)
            return m_PrefixPage;
        return m_NamePage;
    }

    protected void CaptureMenuGeometry(Widget target)
    {
        // The shared Loadout widget uses normalized coordinates, not menu pixels.
        if (target == m_GRZGunPreview) return;
        GRZKillfeedMenuGeometry geometry = new GRZKillfeedMenuGeometry;
        geometry.Target = target;
        float originalX, originalY, originalWidth, originalHeight;
        target.GetPos(originalX, originalY);
        target.GetSize(originalWidth, originalHeight);
        geometry.X = originalX;
        geometry.Y = originalY;
        geometry.Width = originalWidth;
        geometry.Height = originalHeight;
        string widgetName = target.GetName();
        if (widgetName == "Title") geometry.FontSize = 32;
        if (widgetName == "PageTitle") geometry.FontSize = 26;
        if (widgetName == "AnimationHeading") geometry.FontSize = 20;
        if (widgetName == "Preview") geometry.FontSize = 28;
        if (widgetName == "PrefixPreviewWord") geometry.FontSize = 24;
        if (widgetName == "PreviewCaption") geometry.FontSize = 12;
        if (widgetName == "Status" || widgetName == "PageNote" || widgetName.IndexOf("SwatchLabel") == 0) geometry.FontSize = 14;
        if (widgetName == "NameTabLabel" || widgetName == "PrefixTabLabel" || widgetName == "SaveLabel" || widgetName == "CloseLabel" || widgetName == "TestLabel") geometry.FontSize = 18;
        m_Geometry.Insert(geometry);
        Widget child = target.GetChildren();
        while (child)
        {
            CaptureMenuGeometry(child);
            child = child.GetSibling();
        }
    }

    protected void PositionPanel()
    {
        if (!layoutRoot)
            return;
        float width, height;
        g_Game.GetWorkspace().GetScreenSize(width, height);
        layoutRoot.SetScreenPos(0, 0);
        layoutRoot.SetScreenSize(width, height);
        m_MenuScale = Math.Min((width * 0.86) / 1100.0, (height * 0.87) / 760.0);
        m_MenuScale = Math.Max(0.1, m_MenuScale);
        if (Math.AbsFloat(m_MenuScale - m_AppliedScale) > 0.001)
        {
            m_AppliedScale = m_MenuScale;
            foreach (GRZKillfeedMenuGeometry geometry : m_Geometry)
            {
                geometry.Target.SetPos(geometry.X * m_MenuScale, geometry.Y * m_MenuScale);
                geometry.Target.SetSize(geometry.Width * m_MenuScale, geometry.Height * m_MenuScale);
                TextWidget label = TextWidget.Cast(geometry.Target);
                if (label)
                    label.SetTextExactSize(Math.Max(1, Math.Round(geometry.FontSize * m_MenuScale)));
            }
            RefreshPreview();
            GRZApplyVIPPreviewTransform();
        }
        Widget panel = layoutRoot.FindAnyWidget("MenuPanel");
        if (panel)
            panel.SetScreenPos((width - 1100 * m_MenuScale) * 0.5, (height - 760 * m_MenuScale) * 0.5);
        GRZApplyVIPPreviewTransform();
    }

    void ApplyLocalStyle(GRZNameStyle style)
    {
        Chosen = style.Clone();
        m_PrefixInput.SetText(Chosen.PrefixWord);
        IsOfflinePreview = true;
        RefreshLabels();
        m_Status.SetText("Local preview — server access and persistence are not tested.");
    }

    override bool OnKeyDown(Widget w, int x, int y, int key)
    {
        if (key == KeyCode.KC_TAB) return true;
        if (key == KeyCode.KC_ESCAPE)
        {
            CloseSafely();
            return true;
        }
        return super.OnKeyDown(w, x, y, key);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_GRZDismissRequested)
            return true;
        string name = w.GetName();
        if (GRZVIPGunClick(name)) return true;
        GRZAnimationStyle animation = CurrentAnimation();
        for (int i = 0; i < GRZNameStyle.PALETTE_COUNT; i++)
        {
            if (name == "Swatch" + i.ToString())
            {
                if (m_ColorTarget == 0)
                    animation.ColorA = i;
                else
                    animation.ColorB = i;
                m_Status.SetText("Preview updated. Save when you're ready.");
                RefreshLabels();
                return true;
            }
        }
        switch (name)
        {
            case "NameTab":
                m_GRZGunTab = false;
                GRZDisposeVIPGun();
                m_PrefixTab = false;
                m_Status.SetText("Colors, style and speed below edit your name animation.");
                RefreshLabels();
                return true;
            case "PrefixTab":
                m_GRZGunTab = false;
                GRZDisposeVIPGun();
                m_PrefixTab = true;
                m_Status.SetText("Choose an icon and type your prefix. It uses the colors and animation from the Killfeed tab.");
                RefreshLabels();
                return true;
            case "PrefixIconChoice":
                if (GRZPrefixIcons.Count() == 0)
                {
                    m_Status.SetText("No custom icons installed yet. Your server owner can add artwork to the icon catalog.");
                    return true;
                }
                Chosen.PrefixIcon = (Chosen.PrefixIcon + 1) % (GRZPrefixIcons.Count() + 1); break;
            case "ColorA":
                m_ColorTarget = 0;
                m_Status.SetText("Choose a primary color from the swatches.");
                RefreshLabels();
                return true;
            case "ColorB":
                m_ColorTarget = 1;
                m_Status.SetText("Choose a secondary color from the swatches.");
                RefreshLabels();
                return true;
            case "Effect": animation.AnimationMode = (animation.AnimationMode + 1) % GRZNameStyle.EFFECT_COUNT; break;
            case "Speed": animation.Speed = (animation.Speed + 1) % 3; break;
            case "CloseX":
            case "Close": CloseSafely(); return true;
            case "Test":
                GRZKillfeedClient.Demo(Chosen);
                CloseSafely();
                return true;
            case "Save":
                if (IsOfflinePreview)
                {
                    GRZKillfeedClient.LocalStyle = Chosen.Clone();
                    m_Status.SetText("Applied to local previews only. Multiplayer settings are server-controlled.");
                    return true;
                }
                PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
                if (!player)
                    return true;
                ScriptRPC rpc = new ScriptRPC;
                rpc.Write(Chosen.ColorA);
                rpc.Write(Chosen.ColorB);
                rpc.Write(Chosen.AnimationMode);
                rpc.Write(Chosen.Speed);
                rpc.Write(Chosen.PrefixIcon);
                rpc.Write(Chosen.PrefixWord);
                rpc.Write(Chosen.PrefixAnimation.ColorA);
                rpc.Write(Chosen.PrefixAnimation.ColorB);
                rpc.Write(Chosen.PrefixAnimation.AnimationMode);
                rpc.Write(Chosen.PrefixAnimation.Speed);
                rpc.Send(player, GRZKillfeedRPC.SAVE_STYLE, true, null);
                m_Status.SetText("Saving on server...");
                return true;
            default: return super.OnClick(w, x, y, button);
        }
        m_Status.SetText("Preview only — press Save to apply.");
        RefreshLabels();
        return true;
    }

    void ApplyServerStyle(GRZNameStyle style)
    {
        Chosen = style.Clone();
        GRZKillfeedClient.LocalStyle = style.Clone();
        m_PrefixInput.SetText(Chosen.PrefixWord);
        IsOfflinePreview = false;
        RefreshLabels();
        m_Status.SetText("Server access confirmed. Save applies your style to the live killfeed.");
    }

    void UseLocalPreview()
    {
        IsOfflinePreview = true;
        m_Status.SetText("Local preview only. Server customization requires Steam-ID access.");
    }

    void SetResult(int status)
    {
        if (!m_Status)
            return;
        if (status == 2)
        {
            GRZKillfeedClient.LocalStyle = Chosen.Clone();
            m_Status.SetText("Saved. Your killfeed name style is active.");
        }
        else
            m_Status.SetText("Could not save. Check access and server profile write permissions.");
    }

    protected void GRZApplyPanelTextures(Widget root)
    {
        if (!root) return;
        ImageWidget image = ImageWidget.Cast(root);
        string name = root.GetName();
        if (image && name != "MenuLogo" && name != "PrefixPreviewIcon" && name != "PrefixChoicePreview")
        {
            image.LoadImageFile(0, "GRZ_Killfeed/gui/textures/grz_solid_white_ca.paa");
            image.SetImage(0);
            image.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
        }
        Widget child = root.GetChildren();
        while (child) { GRZApplyPanelTextures(child); child = child.GetSibling(); }
    }

    protected void GRZRequestVIPCatalogue()
    {
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if (!player || IsOfflinePreview) return;
        ScriptRPC rpc = new ScriptRPC;
        rpc.Send(player, GRZLoadoutRPC.REQUEST, true, null);
    }

    void GRZReceiveVIPCatalogue()
    {
        if (m_GRZGunTab) GRZFilterVIPGuns();
    }

    void GRZReceiveVIPResult(string message)
    {
        if (m_GRZGunTab && m_Status) m_Status.SetText(message);
    }

    protected GRZLoadoutGun GRZCurrentVIPGun()
    {
        GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
        if (!catalogue || m_GRZGunIndex < 0 || m_GRZGunIndex >= m_GRZGunChoices.Count()) return null;
        int gunCatalogueIndex = m_GRZGunChoices[m_GRZGunIndex];
        if (gunCatalogueIndex < 0 || gunCatalogueIndex >= catalogue.VipGuns.Count()) return null;
        return catalogue.VipGuns[gunCatalogueIndex];
    }

    protected GRZLoadoutClothing GRZCurrentVIPGear()
    {
        GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
        if (!catalogue || m_GRZGunIndex < 0 || m_GRZGunIndex >= m_GRZGunChoices.Count()) return null;
        int choice = m_GRZGunChoices[m_GRZGunIndex];
        if (choice >= 0) return null;
        int index = -choice - 1;
        if (index < 0 || index >= catalogue.VipClothing.Count()) return null;
        return catalogue.VipClothing[index];
    }

    protected string GRZCurrentVIPWrap()
    {
        GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
        if (!catalogue || m_GRZWrapIndex < 0 || m_GRZWrapIndex >= catalogue.VipWraps.Count()) return "";
        return catalogue.VipWraps[m_GRZWrapIndex];
    }

    protected void GRZFilterVIPGuns()
    {
        GRZLoadoutGun previous = GRZCurrentVIPGun();
        GRZLoadoutClothing previousGear = GRZCurrentVIPGear();
        string previousType;
        if (previous) previousType = previous.Type;
        if (previousGear) previousType = previousGear.Type;
        m_GRZGunChoices.Clear();
        m_GRZGunIndex = -1;
        m_GRZGunList.ClearItems();
        GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
        if (catalogue && catalogue.VipAuthorized)
        {
            for (int i = 0; i < catalogue.VipGuns.Count(); i++)
            {
                GRZLoadoutGun gun = catalogue.VipGuns[i];
                if (!gun || gun.Category != m_GRZGunCategory) continue;
                m_GRZGunList.AddItem(gun.Name, null, 0);
                m_GRZGunChoices.Insert(i);
                if (gun.Type == previousType) m_GRZGunIndex = m_GRZGunChoices.Count() - 1;
            }
            if (m_GRZGunCategory == 3)
            {
                for (int gearIndex = 0; gearIndex < catalogue.VipClothing.Count(); gearIndex++)
                {
                    GRZLoadoutClothing gear = catalogue.VipClothing[gearIndex];
                    m_GRZGunList.AddItem(gear.Name, null, 0);
                    // Negative indices distinguish clothing from gun catalogue indices.
                    m_GRZGunChoices.Insert(-gearIndex - 1);
                    if (gear.Type == previousType) m_GRZGunIndex = m_GRZGunChoices.Count() - 1;
                }
            }
        }
        if (m_GRZGunChoices.Count() > 0)
        {
            if (m_GRZGunIndex < 0) m_GRZGunIndex = 0;
            m_GRZGunList.SelectRow(m_GRZGunIndex);
        }
        if (!catalogue || m_GRZWrapIndex >= catalogue.VipWraps.Count()) m_GRZWrapIndex = 0;
        GRZRefreshVIPGun();
    }

    protected void GRZDisposeVIPGun()
    {
        m_GRZPreviewDragging = false;
        if (g_Game) g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZBindVIPPreview);
        if (m_GRZPreviewRoot) m_GRZPreviewRoot.Show(false);
        if (m_GRZGunPreview) m_GRZGunPreview.SetItem(null);
        if (m_GRZPreviewGun && g_Game) g_Game.ObjectDelete(m_GRZPreviewGun);
        m_GRZPreviewGun = null;
    }

    protected void GRZRefreshVIPGun()
    {
        if (!m_GRZGunPage) return;
        m_Status.SetText("Preview only — press EQUIP & SAVE to use this reskin.");
        GRZDisposeVIPGun();
        string buttons[4] = {"VIPGun20", "VIPGun50", "VIPGun408", "VIPGunOther"};
        for (int i = 0; i < 4; i++)
        {
            int fill = 0xFF19191C;
            if (i == m_GRZGunCategory) fill = 0xFF55555A;
            m_GRZGunPage.FindAnyWidget(buttons[i] + "Fill").SetColor(fill);
        }
        bool gearMode = GRZCurrentVIPGear() != null;
        string wrapWidgets[4] = {"VIPWrapHeading", "VIPWrapPrevious", "VIPWrapName", "VIPWrapNext"};
        for (int wrapWidget = 0; wrapWidget < 4; wrapWidget++) m_GRZGunPage.FindAnyWidget(wrapWidgets[wrapWidget]).Show(!gearMode);
        TextWidget.Cast(m_GRZGunPage.FindAnyWidget("VIPGearNote")).SetText("Drag to rotate / scroll to zoom. VIP suppressors attach automatically.");
        if (gearMode)
        {
            GRZLoadoutClothing selectedGear = GRZCurrentVIPGear();
            layoutRoot.FindAnyWidget("Save").Enable(selectedGear && !IsOfflinePreview);
            TextWidget gearLabel = TextWidget.Cast(m_GRZGunPage.FindAnyWidget("VIPGunName"));
            gearLabel.SetText("No animated clothing installed");
            TextWidget.Cast(m_GRZGunPage.FindAnyWidget("VIPGearNote")).SetText("Drag to inspect the animated back / scroll to zoom.");
            m_Status.SetText("Preview animated clothing, then press EQUIP & SAVE.");
            if (!selectedGear) return;
            gearLabel.SetText(selectedGear.Name);
            m_GRZPreviewGun = GRZCreateVIPPreviewObject(selectedGear.Type);
            if (!m_GRZPreviewGun) { m_Status.SetText("Clothing preview could not be created."); return; }
            GRZPrepareVIPPreview(selectedGear.Type, true);
            GRZBindVIPPreview();
            return;
        }
        GRZLoadoutGun gun = GRZCurrentVIPGun();
        layoutRoot.FindAnyWidget("Save").Enable(gun && !IsOfflinePreview);
        TextWidget label = TextWidget.Cast(m_GRZGunPage.FindAnyWidget("VIPGunName"));
        if (!gun)
        {
            label.SetText("No reskins in this section yet");
            return;
        }
        label.SetText(gun.Name);
        string wrap = GRZCurrentVIPWrap();
        string wrapName = "No wrap";
        if (wrap != "")
        {
            g_Game.ConfigGetText("CfgVehicles " + wrap + " displayName", wrapName);
            if (wrapName == "") wrapName = wrap;
        }
        TextWidget.Cast(m_GRZGunPage.FindAnyWidget("VIPWrapName")).SetText(wrapName);
        m_GRZPreviewGun = GRZCreateVIPPreviewObject(gun.Type);
        if (!m_GRZPreviewGun) { m_Status.SetText("Weapon preview model could not be created."); return; }
        bool ready = GRZLoadoutAttachments.EnsurePermanentSFGSuppressor(m_GRZPreviewGun, false);
        if (!GRZLoadoutAttachments.AttachScope(m_GRZPreviewGun, gun.ScopeType)) ready = false;
        if (!GRZVipLoadoutPolicy.EnsureSuppressor(m_GRZPreviewGun)) ready = false;
        if (wrap != "" && !m_GRZPreviewGun.GetInventory().CreateAttachment(wrap)) ready = false;
        if (!ready) m_Status.SetText("An attachment could not be previewed. The server checks compatibility before equipping.");
        Weapon_Base previewWeapon = Weapon_Base.Cast(m_GRZPreviewGun);
        if (previewWeapon)
        {
            string previewAmmo = gun.Ammo;
            if (gun.MagazineType != "") previewAmmo = gun.MagazineType;
            if (previewAmmo != "") previewWeapon.SpawnAmmo(previewAmmo);
        }
        // Bind the whole weapon hierarchy only after all preview attachments are fitted.
        GRZPrepareVIPPreview(gun.Type, false);
        GRZBindVIPPreview();
    }

    protected EntityAI GRZCreateVIPPreviewObject(string type)
    {
        return GRZLoadoutItemPreview.CreateItem(type);
    }

    protected void GRZPrepareVIPPreview(string type, bool clothing)
    {
        if (m_GRZPreviewType != type)
        {
            m_GRZPreviewType = type;
            m_GRZPreviewZoom = 1.0;
            m_GRZPreviewPitch = 0;
            m_GRZPreviewYaw = 0;
            if (clothing) m_GRZPreviewYaw = 180;
        }
    }

    void GRZBindVIPPreview()
    {
        if (!m_GRZGunTab || !m_GRZGunPreview || !m_GRZPreviewGun || m_GRZDismissRequested) return;
        if (!m_GRZPreviewRoot) return;
        m_GRZPreviewRoot.Show(true);
        m_GRZGunPreview.Show(true);
        GRZLoadoutItemPreview.Bind(m_GRZGunPreview, m_GRZPreviewGun);
        GRZApplyVIPPreviewTransform();
        float previewWidth, previewHeight;
        m_GRZGunPreview.GetScreenSize(previewWidth, previewHeight);
        Print(string.Format("[GRZ VIP Preview] Shared Loadout bound %1; visible=%2 size=%3x%4 attachments=%5", m_GRZPreviewGun.GetType(), m_GRZGunPreview.IsVisibleHierarchy(), previewWidth, previewHeight, m_GRZPreviewGun.GetInventory().AttachmentCount()));
    }

    protected void GRZApplyVIPPreviewTransform()
    {
        if (!m_GRZPreviewRoot || !m_GRZGunPreview || !m_GRZPreviewGun) return;
        // Keep the same normalized coordinate system as the normal Loadout picker.
        m_GRZGunPreview.SetSize(m_GRZPreviewZoom, m_GRZPreviewZoom);
        m_GRZGunPreview.SetPos((1 - m_GRZPreviewZoom) * 0.5, (1 - m_GRZPreviewZoom) * 0.5);
        m_GRZGunPreview.SetModelOrientation(Vector(m_GRZPreviewPitch, m_GRZPreviewYaw, 0));
    }

    bool GRZVIPPreviewMouseDown(Widget w, int x, int y, int button)
    {
        if (w != m_GRZGunPreview || button != MouseState.LEFT || !m_GRZGunTab || !m_GRZPreviewGun) return false;
        GetMousePos(m_GRZPreviewMouseX, m_GRZPreviewMouseY);
        m_GRZPreviewDragging = true;
        return true;
    }

    bool GRZVIPPreviewWheel(Widget w, int x, int y, int wheel)
    {
        if (w != m_GRZGunPreview || !m_GRZGunTab || !m_GRZPreviewGun) return false;
        m_GRZPreviewZoom = Math.Clamp(m_GRZPreviewZoom + wheel * 0.1, 0.5, 2.5);
        GRZApplyVIPPreviewTransform();
        return true;
    }

    protected void GRZUpdateVIPPreviewRotation()
    {
        if (!m_GRZPreviewDragging) return;
        if (!m_GRZGunTab || !m_GRZPreviewGun || m_GRZDismissRequested)
        {
            m_GRZPreviewDragging = false;
            return;
        }
        int mouseX, mouseY;
        GetMousePos(mouseX, mouseY);
        m_GRZPreviewYaw = m_GRZPreviewYaw + Math.Clamp(mouseX - m_GRZPreviewMouseX, -80, 80) * 0.5;
        m_GRZPreviewPitch = Math.Clamp(m_GRZPreviewPitch + Math.Clamp(mouseY - m_GRZPreviewMouseY, -80, 80) * 0.5, -75, 75);
        m_GRZPreviewMouseX = mouseX;
        m_GRZPreviewMouseY = mouseY;
        GRZApplyVIPPreviewTransform();
        if (!(GetMouseState(MouseState.LEFT) & 0x80000000)) m_GRZPreviewDragging = false;
    }

    protected bool GRZVIPGunClick(string name)
    {
        if (name == "GunTab")
        {
            m_GRZGunTab = true;
            RefreshLabels();
            GRZFilterVIPGuns();
            GRZRequestVIPCatalogue();
            return true;
        }
        if (!m_GRZGunTab) return false;
        if (name == "VIPResetView")
        {
            m_GRZPreviewZoom = 1.0;
            m_GRZPreviewPitch = 0;
            m_GRZPreviewYaw = 0;
            if (GRZCurrentVIPGear()) m_GRZPreviewYaw = 180;
            GRZApplyVIPPreviewTransform();
            return true;
        }
        if (name == "VIPGun20" || name == "VIPGun50" || name == "VIPGun408" || name == "VIPGunOther")
        {
            m_GRZGunCategory = 0;
            if (name == "VIPGun50") m_GRZGunCategory = 1;
            if (name == "VIPGun408") m_GRZGunCategory = 2;
            if (name == "VIPGunOther") m_GRZGunCategory = 3;
            GRZFilterVIPGuns();
            return true;
        }
        if (name == "VIPGunPrevious" || name == "VIPGunNext")
        {
            if (m_GRZGunChoices.Count() > 0)
            {
                int gunStep = 1;
                if (name == "VIPGunPrevious") gunStep = -1;
                m_GRZGunIndex = (m_GRZGunIndex + gunStep + m_GRZGunChoices.Count()) % m_GRZGunChoices.Count();
                m_GRZGunList.SelectRow(m_GRZGunIndex);
                GRZRefreshVIPGun();
            }
            return true;
        }
        if (name == "VIPWrapPrevious" || name == "VIPWrapNext")
        {
            GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
            if (catalogue && catalogue.VipWraps.Count() > 0)
            {
                int wrapStep = 1;
                if (name == "VIPWrapPrevious") wrapStep = -1;
                m_GRZWrapIndex = (m_GRZWrapIndex + wrapStep + catalogue.VipWraps.Count()) % catalogue.VipWraps.Count();
                GRZRefreshVIPGun();
            }
            return true;
        }
        if (name == "Save")
        {
            if (GRZCurrentVIPGear())
            {
                GRZLoadoutClothing gearSelection = GRZCurrentVIPGear();
                PlayerBase gearPlayer = PlayerBase.Cast(g_Game.GetPlayer());
                if (gearSelection && gearPlayer && !IsOfflinePreview)
                {
                    ScriptRPC gearRPC = new ScriptRPC;
                    gearRPC.Write(gearSelection.Type);
                    gearRPC.Send(gearPlayer, GRZLoadoutRPC.VIP_GEAR_APPLY, true, null);
                    m_Status.SetText("Applying and saving animated gear...");
                }
                return true;
            }
            GRZLoadoutGun gun = GRZCurrentVIPGun();
            PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
            if (gun && player && !IsOfflinePreview)
            {
                ScriptRPC rpc = new ScriptRPC;
                rpc.Write(gun.Type);
                rpc.Write(GRZCurrentVIPWrap());
                rpc.Send(player, GRZLoadoutRPC.SAVE, true, null);
                m_Status.SetText("Equipping and saving VIP loadout...");
            }
            return true;
        }
        return false;
    }

    void ~GRZKillfeedMenu()
    {
        if (m_GRZGunPreview && WidgetEventHandler.GetInstance()) WidgetEventHandler.GetInstance().UnregisterWidget(m_GRZGunPreview);
        GRZLoadoutClientData.Changed.Remove(GRZReceiveVIPCatalogue);
        GRZLoadoutClientData.Result.Remove(GRZReceiveVIPResult);
        GRZDisposeVIPGun();
        m_GRZPreviewRoot = null;
    }

    protected GRZAnimationStyle CurrentAnimation()
    {
        // Name, prefix and icon share one animation and color selection.
        return Chosen;
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (w == m_PrefixInput)
        {
            string candidate = m_PrefixInput.GetText();
            string previous = Chosen.PrefixWord;
            Chosen.PrefixWord = candidate;
            if (!Chosen.IsValid())
            {
                Chosen.PrefixWord = previous;
                m_PrefixInput.SetText(previous);
                if (GRZPrefixPolicy.Reserved(candidate))
                    m_Status.SetText("Staff titles such as Admin, Owner, Mod and Support are not allowed.");
                else
                    m_Status.SetText("Use up to 12 letters, digits, _ or -.");
            }
            else
                m_Status.SetText("Prefix preview updated. Press Save to apply.");
            RefreshLabels();
            return true;
        }
        return super.OnChange(w, x, y, finished);
    }

    protected void RefreshPreview()
    {
        if (!m_Preview || !m_PrefixPreview || !m_PrefixPreviewIcon)
            return;
        string prefixText;
        if (Chosen.PrefixWord != "" || Chosen.PrefixIcon > 0)
            prefixText = Chosen.PrefixWord + " |";
        m_PrefixPreview.SetText(prefixText);
        GRZNameStyle.ApplyIcon(m_PrefixPreviewIcon, Chosen.PrefixIcon);
        int wordWidth, wordHeight;
        m_PrefixPreview.GetTextSize(wordWidth, wordHeight);
        if (prefixText == "")
            wordWidth = 0;
        float iconWidth;
        if (Chosen.PrefixIcon > 0)
            iconWidth = 48 * m_MenuScale;
        float gap;
        if (prefixText != "")
            gap = 8 * m_MenuScale;
        string playerName;
        g_Game.GetPlayerName(playerName);
        m_Preview.SetText(playerName);
        int nameWidth, nameHeight;
        m_Preview.GetTextSize(nameWidth, nameHeight);
        int count = playerName.LengthUtf8();
        while (iconWidth + wordWidth + gap + nameWidth > 408 * m_MenuScale && count > 1)
        {
            count--;
            m_Preview.SetText(playerName.SubstringUtf8(0, count) + "...");
            m_Preview.GetTextSize(nameWidth, nameHeight);
        }
        float start = Math.Max(16 * m_MenuScale, (440 * m_MenuScale - iconWidth - wordWidth - gap - nameWidth) * 0.5);
        float baseline = 210;
        if (m_PrefixTab)
            baseline = 286;
        m_PrefixPreviewIcon.SetPos(start, (baseline - 2) * m_MenuScale);
        m_PrefixPreviewIcon.SetSize(40 * m_MenuScale, 40 * m_MenuScale);
        m_PrefixPreview.SetPos(start + iconWidth, baseline * m_MenuScale);
        m_PrefixPreview.SetSize(wordWidth + 4 * m_MenuScale, 36 * m_MenuScale);
        m_Preview.SetPos(start + iconWidth + wordWidth + gap, baseline * m_MenuScale);
        m_Preview.SetSize(Math.Max(1, nameWidth + 4 * m_MenuScale), 36 * m_MenuScale);
    }

    protected void RefreshLabels()
    {
        GRZAnimationStyle animation = CurrentAnimation();
        m_NamePage.Show(!m_PrefixTab && !m_GRZGunTab);
        m_PrefixPage.Show(m_PrefixTab && !m_GRZGunTab);
        m_GRZGunPage.Show(m_GRZGunTab);
        layoutRoot.FindAnyWidget("GunTabFill").SetColor(0xFF191919);
        layoutRoot.FindAnyWidget("Test").Show(!m_GRZGunTab);
        TextWidget.Cast(layoutRoot.FindAnyWidget("SaveLabel")).SetText("SAVE CHANGES");
        layoutRoot.FindAnyWidget("Save").Enable(true);
        if (m_GRZGunTab)
        {
            m_Preview = null;
            m_PrefixPreview = null;
            m_PrefixPreviewIcon = null;
            layoutRoot.FindAnyWidget("NameTabFill").SetColor(0xFF191919);
            layoutRoot.FindAnyWidget("PrefixTabFill").SetColor(0xFF191919);
            layoutRoot.FindAnyWidget("GunTabFill").SetColor(0xFF55555A);
            TextWidget.Cast(layoutRoot.FindAnyWidget("SaveLabel")).SetText("EQUIP & SAVE");
            m_Status.SetText("Choose a reskin and ghillie wrap. Compatible VIP suppressors are automatic.");
            GRZRefreshVIPGun();
            return;
        }
        Widget page = ActivePage();
        m_Preview = TextWidget.Cast(page.FindAnyWidget("Preview"));
        m_PrefixPreview = TextWidget.Cast(page.FindAnyWidget("PrefixPreviewWord"));
        m_PrefixPreviewIcon = ImageWidget.Cast(page.FindAnyWidget("PrefixPreviewIcon"));
        layoutRoot.FindAnyWidget("NameTabFill").SetColor(0xFF191919);
        layoutRoot.FindAnyWidget("PrefixTabFill").SetColor(0xFF191919);
        if (m_PrefixTab)
            layoutRoot.FindAnyWidget("PrefixTabFill").SetColor(0xFF55555A);
        else
            layoutRoot.FindAnyWidget("NameTabFill").SetColor(0xFF55555A);
        TextWidget.Cast(m_PrefixPage.FindAnyWidget("PrefixIconChoiceLabel")).SetText("Icon: " + GRZNameStyle.IconName(Chosen.PrefixIcon) + " >");
        GRZNameStyle.ApplyIcon(ImageWidget.Cast(m_PrefixPage.FindAnyWidget("PrefixChoicePreview")), Chosen.PrefixIcon);
        RefreshPreview();
        TextWidget.Cast(m_NamePage.FindAnyWidget("ColorALabel")).SetText("Primary: " + GRZNameStyle.ColorName(animation.ColorA));
        TextWidget.Cast(m_NamePage.FindAnyWidget("ColorBLabel")).SetText("Secondary: " + GRZNameStyle.ColorName(animation.ColorB));
        TextWidget.Cast(m_NamePage.FindAnyWidget("EffectLabel")).SetText("Style: " + GRZNameStyle.EffectName(animation.AnimationMode));
        string speed = "Normal";
        if (animation.Speed == 0)
            speed = "Slow";
        if (animation.Speed == 2)
            speed = "Fast";
        TextWidget.Cast(m_NamePage.FindAnyWidget("SpeedLabel")).SetText("Speed: " + speed);
        RefreshPalette();
    }

    protected void RefreshPalette()
    {
        Widget page = m_NamePage;
        GRZAnimationStyle animation = CurrentAnimation();
        int selected = animation.ColorA;
        if (m_ColorTarget == 1)
            selected = animation.ColorB;
        page.FindAnyWidget("PrimaryActive").SetColor(0x00000000);
        page.FindAnyWidget("SecondaryActive").SetColor(0x00000000);
        if (m_ColorTarget == 0)
            page.FindAnyWidget("PrimaryActive").SetColor(0xFFFFFFFF);
        else
            page.FindAnyWidget("SecondaryActive").SetColor(0xFFFFFFFF);
        for (int i = 0; i < GRZNameStyle.PALETTE_COUNT; i++)
        {
            string suffix = i.ToString();
            page.FindAnyWidget("SwatchFill" + suffix).SetColor(GRZNameStyle.Palette(i));
            int border = 0xFF333333;
            if (i == selected)
                border = 0xFFFFFFFF;
            page.FindAnyWidget("SwatchBorder" + suffix).SetColor(border);
            TextWidget label = TextWidget.Cast(page.FindAnyWidget("SwatchLabel" + suffix));
            label.SetTextExactSize(Math.Max(1, Math.Round(14 * m_MenuScale)));
            label.SetOutline(1, 0xFF000000);
        }
    }
}

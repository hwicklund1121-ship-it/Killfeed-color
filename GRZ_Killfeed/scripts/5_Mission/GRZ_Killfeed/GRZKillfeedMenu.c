class GRZKillfeedClient
{
    static ref GRZNameStyle LocalStyle = new GRZNameStyle;
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
            Error("[GRZ Killfeed 0.1.14] Native menu factory did not return an initialized settings menu.");
            g_Game.Chat("GRZ Killfeed: menu creation failed. Check the client script log.", "colorStatusChannel");
            return;
        }
        ActiveMenu.ApplyLocalStyle(LocalStyle);
        ActiveMenu.EnsureVisible();
        Print("[GRZ Killfeed 0.1.14] Native menu registered and retained.");
    }

    static void ReleaseMenu()
    {
        if (ActiveMenu && ActiveMenu.IsVisible())
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
        if (CanLocalPreview())
            return;
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if (!player)
            return;
        ScriptRPC rpc = new ScriptRPC;
        rpc.Send(player, GRZKillfeedRPC.REQUEST_MENU, true, null);
    }

    static void TestOffline()
    {
        if (!CanLocalPreview())
        {
            PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
            if (player)
                player.MessageStatus("GRZ Killfeed: /grzvip test is available only in offline editor sessions.");
            return;
        }
        Demo(LocalStyle);
    }

    static void Demo(GRZNameStyle style)
    {
        // Client-only preview. Never sends an announcement to other players.
        GRZKillfeedMessage message = new GRZKillfeedMessage;
        g_Game.GetPlayerName(message.Killer);
        message.Victim = "Target Dummy";
        message.GunData.Types = {"M4A1", "ACOGOptic", "M4_RISHndgrd", "M4_OEBttstck", "M4_Suppressor", "Mag_STANAG_30Rnd"};
        message.GunData.Parents = {-1, 0, 0, 0, 0, 0};
        message.GunData.Slots = {-1, -1, -1, -1, -1, -1};
        message.Distance = 175;
        message.KillerStyle = style.Clone();
        GRZKillfeedBus.Kills.Invoke(message);
    }
}

modded class ChatInputMenu
{
    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        // Intercept before vanilla ChatPlayer sends anything to public chat.
        EditBoxWidget input = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("InputEditBoxWidget"));
        if (finished && input)
        {
            string command = input.GetText().Trim();
            command.ToLower();
            if (command == "/grzvip")
            {
                Close();
                g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.RequestMenu, 150, false);
                return true;
            }
            if (command == "/grzvip test")
            {
                Close();
                g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.TestOffline, 150, false);
                return true;
            }
        }
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
        }
        if (!m_NamePage || !m_PrefixPage)
        {
            Error("GRZ Killfeed: could not create Name and Prefix pages.");
            layoutRoot.Unlink();
            layoutRoot = null;
            return null;
        }
        Widget nameControls = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu_animation.layout", m_NamePage.FindAnyWidget("AnimationHost"));
        Widget prefixControls = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu_animation.layout", m_PrefixPage.FindAnyWidget("AnimationHost"));
        ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("MenuLogo"));
        m_PrefixInput = EditBoxWidget.Cast(m_PrefixPage.FindAnyWidget("PrefixWordInput"));
        m_Status = TextWidget.Cast(layoutRoot.FindAnyWidget("Status"));
        if (!logo || !m_Status || !m_PrefixInput || !nameControls || !prefixControls)
        {
            Error("GRZ Killfeed: menu layout is missing required widgets.");
            layoutRoot.Unlink();
            layoutRoot = null;
            return null;
        }
        logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
        logo.LoadImageFile(0, "GRZ_Killfeed/gui/textures/grz_menu_logo_ca.paa");
        logo.SetSort(0);
        CaptureMenuGeometry(layoutRoot.FindAnyWidget("MenuPanel"));
        RefreshLabels();
        if (IsOfflinePreview)
            m_Status.SetText("Local preview — server access and persistence are not tested.");
        PositionPanel();
        Print("[GRZ Killfeed 0.1.14] Init: layout created.");
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
    }

    override void UnlockControls()
    {
        if (!m_GRZControlsLocked)
            return;
        m_GRZControlsLocked = false;
        super.UnlockControls();
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
            CloseSafely();
            return;
        }
        super.Update(timeslice);
        m_Time += timeslice;
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
            Print(string.Format("[GRZ Killfeed 0.1.14] Update: root %1 x %2; visible %3.", width, height, layoutRoot.IsVisibleHierarchy()));
        }
    }

    override void OnShow()
    {
        m_GRZDismissRequested = false;
        super.OnShow();
        GRZKillfeedBus.AppDeactivated.Remove(CloseSafely);
        GRZKillfeedBus.AppDeactivated.Insert(CloseSafely);
        if (!g_Game.IsAppActive())
        {
            CloseSafely();
            return;
        }
        EnsureVisible();
        Print("[GRZ Killfeed 0.1.14] OnShow called.");
    }

    override void OnHide()
    {
        m_GRZDismissRequested = true;
        GRZKillfeedBus.AppDeactivated.Remove(CloseSafely);
        Print("[GRZ Killfeed 0.1.14] OnHide called.");
        super.OnHide();
    }

    override void OnVisibilityChanged(bool isVisible)
    {
        super.OnVisibilityChanged(isVisible);
        if (!isVisible)
            CloseSafely();
    }

    void CloseSafely()
    {
        if (m_GRZDismissRequested)
            return;
        m_GRZDismissRequested = true;
        GRZKillfeedBus.AppDeactivated.Remove(CloseSafely);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZKillfeedClient.OpenLocalPreview);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZKillfeedClient.RequestMenu);
        if (layoutRoot)
            layoutRoot.Show(false);
        // Native Close can finish on a later frame. Unlock now, with the
        // guard above preventing the later OnHide from decrementing twice.
        UnlockControls();
        Close();
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
        m_MenuScale = Math.Min(1.0, Math.Min((width - 32) / 1100.0, (height - 32) / 760.0));
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
        }
        Widget panel = layoutRoot.FindAnyWidget("MenuPanel");
        if (panel)
            panel.SetScreenPos((width - 1100 * m_MenuScale) * 0.5, (height - 760 * m_MenuScale) * 0.5);
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
        if (key == KeyCode.KC_ESCAPE || key == KeyCode.KC_TAB)
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
                m_PrefixTab = false;
                m_Status.SetText("Colors, style and speed below edit your name animation.");
                RefreshLabels();
                return true;
            case "PrefixTab":
                m_PrefixTab = true;
                m_Status.SetText("Choose an icon and type your prefix. Colors and animation stay synced with your name.");
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
            m_Status.SetText("Saved. Your killfeed name style is active.");
        else
            m_Status.SetText("Could not save. Check access and server profile write permissions.");
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
        m_NamePage.Show(!m_PrefixTab);
        m_PrefixPage.Show(m_PrefixTab);
        Widget page = ActivePage();
        m_Preview = TextWidget.Cast(page.FindAnyWidget("Preview"));
        m_PrefixPreview = TextWidget.Cast(page.FindAnyWidget("PrefixPreviewWord"));
        m_PrefixPreviewIcon = ImageWidget.Cast(page.FindAnyWidget("PrefixPreviewIcon"));
        layoutRoot.FindAnyWidget("NameTabFill").SetColor(0xFF191919);
        layoutRoot.FindAnyWidget("PrefixTabFill").SetColor(0xFF191919);
        if (m_PrefixTab)
            layoutRoot.FindAnyWidget("PrefixTabFill").SetColor(0xFF981D23);
        else
            layoutRoot.FindAnyWidget("NameTabFill").SetColor(0xFF981D23);
        TextWidget.Cast(m_PrefixPage.FindAnyWidget("PrefixIconChoiceLabel")).SetText("Icon: " + GRZNameStyle.IconName(Chosen.PrefixIcon) + " >");
        GRZNameStyle.ApplyIcon(ImageWidget.Cast(m_PrefixPage.FindAnyWidget("PrefixChoicePreview")), Chosen.PrefixIcon);
        RefreshPreview();
        TextWidget.Cast(page.FindAnyWidget("ColorALabel")).SetText("Primary: " + GRZNameStyle.ColorName(animation.ColorA));
        TextWidget.Cast(page.FindAnyWidget("ColorBLabel")).SetText("Secondary: " + GRZNameStyle.ColorName(animation.ColorB));
        TextWidget.Cast(page.FindAnyWidget("EffectLabel")).SetText("Style: " + GRZNameStyle.EffectName(animation.AnimationMode));
        string speed = "Normal";
        if (animation.Speed == 0)
            speed = "Slow";
        if (animation.Speed == 2)
            speed = "Fast";
        TextWidget.Cast(page.FindAnyWidget("SpeedLabel")).SetText("Speed: " + speed);
        RefreshPalette();
    }

    protected void RefreshPalette()
    {
        Widget page = ActivePage();
        GRZAnimationStyle animation = CurrentAnimation();
        int selected = animation.ColorA;
        if (m_ColorTarget == 1)
            selected = animation.ColorB;
        page.FindAnyWidget("PrimaryActive").SetColor(0x00000000);
        page.FindAnyWidget("SecondaryActive").SetColor(0x00000000);
        if (m_ColorTarget == 0)
            page.FindAnyWidget("PrimaryActive").SetColor(0xFFEF3333);
        else
            page.FindAnyWidget("SecondaryActive").SetColor(0xFFEF3333);
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

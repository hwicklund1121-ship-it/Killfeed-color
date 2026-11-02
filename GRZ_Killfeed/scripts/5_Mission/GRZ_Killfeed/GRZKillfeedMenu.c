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
            Error("[GRZ Killfeed 0.1.13] Native menu factory did not return an initialized settings menu.");
            g_Game.Chat("GRZ Killfeed: menu creation failed. Check the client script log.", "colorStatusChannel");
            return;
        }
        ActiveMenu.ApplyLocalStyle(LocalStyle);
        ActiveMenu.EnsureVisible();
        Print("[GRZ Killfeed 0.1.13] Native menu registered and retained.");
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

class GRZKillfeedMenu extends UIScriptedMenu
{
    static const int MENU_ID = 1647280199;
    ref GRZNameStyle Chosen = new GRZNameStyle;
    bool IsOfflinePreview;
    protected bool m_PrefixTab;
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
        ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("MenuLogo"));
        m_Preview = TextWidget.Cast(layoutRoot.FindAnyWidget("Preview"));
        m_PrefixPreview = TextWidget.Cast(layoutRoot.FindAnyWidget("PrefixPreviewWord"));
        m_PrefixPreviewIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("PrefixPreviewIcon"));
        m_PrefixInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("PrefixWordInput"));
        m_Status = TextWidget.Cast(layoutRoot.FindAnyWidget("Status"));
        if (!logo || !m_Preview || !m_Status || !m_PrefixPreview || !m_PrefixPreviewIcon || !m_PrefixInput || !layoutRoot.FindAnyWidget("MenuPanel"))
        {
            Error("GRZ Killfeed: menu layout is missing required widgets.");
            layoutRoot.Unlink();
            layoutRoot = null;
            return null;
        }
        m_PrefixPreview.SetTextExactSize(24);
        logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
        logo.LoadImageFile(0, "GRZ_Killfeed/gui/textures/grz_menu_logo_ca.paa");
        logo.SetSort(0);
        string name;
        g_Game.GetPlayerName(name);
        m_Preview.SetText(name);
        m_Preview.SetTextExactSize(28);
        TextWidget.Cast(layoutRoot.FindAnyWidget("PreviewCaption")).SetTextExactSize(11);
        m_Status.SetTextExactSize(13);
        TextWidget.Cast(layoutRoot.FindAnyWidget("Title")).SetTextExactSize(26);
        TextWidget.Cast(layoutRoot.FindAnyWidget("Hint")).SetTextExactSize(14);
        RefreshLabels();
        if (IsOfflinePreview)
            m_Status.SetText("Local preview — server access and persistence are not tested.");
        PositionPanel();
        Print("[GRZ Killfeed 0.1.13] Init: layout created.");
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
            m_PrefixPreview.SetColor(Chosen.PrefixAnimation.ColorAt(m_Time));
            m_PrefixPreviewIcon.SetColor(Chosen.PrefixAnimation.ColorAt(m_Time));
        }
        PositionPanel();
        if (!m_LoggedUpdate)
        {
            m_LoggedUpdate = true;
            float width, height;
            layoutRoot.GetScreenSize(width, height);
            Print(string.Format("[GRZ Killfeed 0.1.13] Update: root %1 x %2; visible %3.", width, height, layoutRoot.IsVisibleHierarchy()));
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
        Print("[GRZ Killfeed 0.1.13] OnShow called.");
    }

    override void OnHide()
    {
        m_GRZDismissRequested = true;
        GRZKillfeedBus.AppDeactivated.Remove(CloseSafely);
        Print("[GRZ Killfeed 0.1.13] OnHide called.");
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

    protected void PositionPanel()
    {
        if (!layoutRoot)
            return;
        float width, height;
        g_Game.GetWorkspace().GetScreenSize(width, height);
        layoutRoot.SetScreenPos(0, 0);
        layoutRoot.SetScreenSize(width, height);
        Widget panel = layoutRoot.FindAnyWidget("MenuPanel");
        if (panel)
            panel.SetScreenPos(Math.Max(0, (width - 720) * 0.5), Math.Max(0, (height - 596) * 0.5));
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
                m_Status.SetText("Choose an icon and enter up to 12 letters, digits, _ or -. Colors below animate your prefix.");
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
        if (m_PrefixTab)
            return Chosen.PrefixAnimation;
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

    protected void RefreshLabels()
    {
        GRZAnimationStyle animation = CurrentAnimation();
        layoutRoot.FindAnyWidget("PrefixControls").Show(m_PrefixTab);
        layoutRoot.FindAnyWidget("NameTabFill").SetColor(0xFF191919);
        layoutRoot.FindAnyWidget("PrefixTabFill").SetColor(0xFF191919);
        if (m_PrefixTab)
            layoutRoot.FindAnyWidget("PrefixTabFill").SetColor(0xFF981D23);
        else
            layoutRoot.FindAnyWidget("NameTabFill").SetColor(0xFF981D23);
        TextWidget.Cast(layoutRoot.FindAnyWidget("PrefixIconChoiceLabel")).SetText("Icon: " + GRZNameStyle.IconName(Chosen.PrefixIcon) + " >");
        string prefixText;
        if (Chosen.PrefixWord != "")
            prefixText = "[" + Chosen.PrefixWord + "]";
        m_PrefixPreview.SetText(prefixText);
        GRZNameStyle.ApplyIcon(m_PrefixPreviewIcon, Chosen.PrefixIcon);
        int wordWidth, wordHeight;
        m_PrefixPreview.GetTextSize(wordWidth, wordHeight);
        int iconWidth;
        if (Chosen.PrefixIcon > 0)
            iconWidth = 32;
        string playerName;
        g_Game.GetPlayerName(playerName);
        m_Preview.SetText(playerName);
        int nameWidth, nameHeight;
        m_Preview.GetTextSize(nameWidth, nameHeight);
        float start = Math.Max(44, (720 - iconWidth - wordWidth - nameWidth - 8) * 0.5);
        m_PrefixPreviewIcon.SetPos(start, 242);
        m_PrefixPreview.SetPos(start + iconWidth, 238);
        m_PrefixPreview.SetSize(wordWidth + 4, 32);
        m_Preview.SetPos(start + iconWidth + wordWidth + 8, 238);
        m_Preview.SetSize(Math.Max(32, nameWidth + 4), 32);

        TextWidget.Cast(layoutRoot.FindAnyWidget("ColorALabel")).SetText("Primary: " + GRZNameStyle.ColorName(animation.ColorA));
        TextWidget.Cast(layoutRoot.FindAnyWidget("ColorBLabel")).SetText("Secondary: " + GRZNameStyle.ColorName(animation.ColorB));
        TextWidget.Cast(layoutRoot.FindAnyWidget("EffectLabel")).SetText("Style: " + GRZNameStyle.EffectName(animation.AnimationMode));
        string speed = "Normal";
        if (animation.Speed == 0)
            speed = "Slow";
        if (animation.Speed == 2)
            speed = "Fast";
        TextWidget.Cast(layoutRoot.FindAnyWidget("SpeedLabel")).SetText("Speed: " + speed);
        RefreshPalette();
    }

    protected void RefreshPalette()
    {
        GRZAnimationStyle animation = CurrentAnimation();
        int selected = animation.ColorA;
        if (m_ColorTarget == 1)
            selected = animation.ColorB;
        layoutRoot.FindAnyWidget("PrimaryActive").SetColor(0x00000000);
        layoutRoot.FindAnyWidget("SecondaryActive").SetColor(0x00000000);
        if (m_ColorTarget == 0)
            layoutRoot.FindAnyWidget("PrimaryActive").SetColor(0xFFEF3333);
        else
            layoutRoot.FindAnyWidget("SecondaryActive").SetColor(0xFFEF3333);
        for (int i = 0; i < GRZNameStyle.PALETTE_COUNT; i++)
        {
            string suffix = i.ToString();
            layoutRoot.FindAnyWidget("SwatchFill" + suffix).SetColor(GRZNameStyle.Palette(i));
            int border = 0xFF333333;
            if (i == selected)
                border = 0xFFFFFFFF;
            layoutRoot.FindAnyWidget("SwatchBorder" + suffix).SetColor(border);
            TextWidget label = TextWidget.Cast(layoutRoot.FindAnyWidget("SwatchLabel" + suffix));
            label.SetTextExactSize(12);
            label.SetOutline(1, 0xFF000000);
        }
    }
}

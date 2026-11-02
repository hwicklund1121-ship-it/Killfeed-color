class GRZKillfeedClient
{
    static ref GRZNameStyle LocalStyle = new GRZNameStyle;

    static void RequestMenu()
    {
        if (!g_Game.IsMultiplayer())
        {
            if (g_Game.GetUIManager().GetMenu())
                return;
            GRZKillfeedMenu preview = new GRZKillfeedMenu;
            preview.Chosen = LocalStyle;
            preview.IsOfflinePreview = true;
            g_Game.GetUIManager().ShowScriptedMenu(preview, null);
            return;
        }
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        if (!player)
            return;
        ScriptRPC rpc = new ScriptRPC;
        rpc.Send(player, GRZKillfeedRPC.REQUEST_MENU, true, null);
    }

    static void TestOffline()
    {
        if (g_Game.IsMultiplayer())
        {
            PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
            if (player)
                player.MessageStatus("GRZ Killfeed: /killfeed test is available only in offline editor sessions.");
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
        message.Weapon = "M4-A1";
        message.Distance = 175;
        message.KillerStyle = style;
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
            if (command == "/killfeed")
            {
                Close();
                g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.RequestMenu, 150, false);
                return true;
            }
            if (command == "/killfeed test")
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
    ref GRZNameStyle Chosen = new GRZNameStyle;
    bool IsOfflinePreview;
    protected TextWidget m_Preview;
    protected TextWidget m_Status;
    protected float m_Time;

    override Widget Init()
    {
        layoutRoot = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/menu.layout");
        ImageWidget outline = ImageWidget.Cast(layoutRoot.FindAnyWidget("MenuOutline"));
        outline.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
        outline.LoadImageFile(0, "GRZ_Killfeed/gui/textures/grz_menu_outline_ca.paa");
        outline.SetSort(100);
        m_Preview = TextWidget.Cast(layoutRoot.FindAnyWidget("Preview"));
        m_Status = TextWidget.Cast(layoutRoot.FindAnyWidget("Status"));
        string name;
        g_Game.GetPlayerName(name);
        m_Preview.SetText(name);
        RefreshLabels();
        if (IsOfflinePreview)
            m_Status.SetText("Offline editor preview — no server access or persistence is tested.");
        return layoutRoot;
    }

    override bool UseMouse() { return true; }
    override bool UseKeyboard() { return true; }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        m_Time += timeslice;
        if (m_Preview)
            m_Preview.SetColor(Chosen.ColorAt(m_Time));
        Widget panel = layoutRoot.FindAnyWidget("MenuPanel");
        float width, height;
        layoutRoot.GetScreenSize(width, height);
        panel.SetScreenPos((width - 640) * 0.5, (height - 480) * 0.5);
    }

    override bool OnKeyDown(Widget w, int x, int y, int key)
    {
        if (key == KeyCode.KC_ESCAPE)
        {
            Close();
            return true;
        }
        return super.OnKeyDown(w, x, y, key);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        string name = w.GetName();
        switch (name)
        {
            case "ColorA": Chosen.ColorA = (Chosen.ColorA + 1) % GRZNameStyle.PALETTE_COUNT; break;
            case "ColorB": Chosen.ColorB = (Chosen.ColorB + 1) % GRZNameStyle.PALETTE_COUNT; break;
            case "Effect": Chosen.Effect = (Chosen.Effect + 1) % GRZNameStyle.EFFECT_COUNT; break;
            case "Speed": Chosen.Speed = (Chosen.Speed + 1) % 3; break;
            case "Close": Close(); return true;
            case "Test":
                GRZKillfeedClient.Demo(Chosen);
                Close();
                return true;
            case "Save":
                if (IsOfflinePreview)
                {
                    GRZKillfeedClient.LocalStyle = Chosen;
                    m_Status.SetText("Applied to local previews only. Multiplayer settings are server-controlled.");
                    return true;
                }
                PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
                if (!player)
                    return true;
                ScriptRPC rpc = new ScriptRPC;
                rpc.Write(Chosen.ColorA);
                rpc.Write(Chosen.ColorB);
                rpc.Write(Chosen.Effect);
                rpc.Write(Chosen.Speed);
                rpc.Send(player, GRZKillfeedRPC.SAVE_STYLE, true, null);
                m_Status.SetText("Saving on server...");
                return true;
            default: return super.OnClick(w, x, y, button);
        }
        m_Status.SetText("Preview only — press Save to apply.");
        RefreshLabels();
        return true;
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

    protected void RefreshLabels()
    {
        TextWidget.Cast(layoutRoot.FindAnyWidget("ColorALabel")).SetText("Color 1: " + GRZNameStyle.ColorName(Chosen.ColorA));
        TextWidget.Cast(layoutRoot.FindAnyWidget("ColorBLabel")).SetText("Color 2: " + GRZNameStyle.ColorName(Chosen.ColorB));
        TextWidget.Cast(layoutRoot.FindAnyWidget("EffectLabel")).SetText("Style: " + GRZNameStyle.EffectName(Chosen.Effect));
        string speed = "Normal";
        if (Chosen.Speed == 0)
            speed = "Slow";
        if (Chosen.Speed == 2)
            speed = "Fast";
        TextWidget.Cast(layoutRoot.FindAnyWidget("SpeedLabel")).SetText("Speed: " + speed);
    }
}

class GRZKillfeedRow
{
    Widget Root;
    TextWidget Killer;
    TextWidget Victim;
    ref GRZKillfeedMessage Message;
    float Age;

    void Update(float timeslice)
    {
        Age += timeslice;
        Killer.SetColor(Message.KillerStyle.ColorAt(Age));
        Victim.SetColor(Message.VictimStyle.ColorAt(Age));
        Root.SetAlpha(Math.Clamp(8.0 - Age, 0, 1));
    }
}

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        GRZKillfeedServer.Initialize();
    }
}

modded class MissionGameplay
{
    protected Widget m_GRZFeed;
    protected ref array<ref GRZKillfeedRow> m_GRZRows = new array<ref GRZKillfeedRow>;
    protected bool m_GRZListening;

    override void OnInit()
    {
        super.OnInit();
        if (!m_GRZListening)
        {
            GRZKillfeedBus.Kills.Insert(GRZReceiveKill);
            GRZKillfeedBus.Menu.Insert(GRZReceiveMenu);
            m_GRZListening = true;
        }
    }

    override void OnMissionFinish()
    {
        GRZKillfeedBus.Kills.Remove(GRZReceiveKill);
        GRZKillfeedBus.Menu.Remove(GRZReceiveMenu);
        m_GRZListening = false;
        m_GRZRows.Clear();
        if (m_GRZFeed)
        {
            m_GRZFeed.Unlink();
            m_GRZFeed = null;
        }
        super.OnMissionFinish();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        if (!m_GRZFeed)
            return;
        for (int i = m_GRZRows.Count() - 1; i >= 0; i--)
        {
            m_GRZRows[i].Update(timeslice);
            if (m_GRZRows[i].Age >= 8.0)
            {
                m_GRZRows[i].Root.Unlink();
                m_GRZRows.Remove(i);
            }
        }
        GRZPositionRows();
        IngameHud hud = IngameHud.Cast(GetHud());
        if (hud)
            m_GRZFeed.Show(!hud.GetHudVisibility().IsContextFlagActive(IngameHudVisibility.HUD_HIDE_FLAGS));
    }

    protected bool GRZEnsureFeed()
    {
        if (m_GRZFeed)
            return true;
        IngameHud hud = IngameHud.Cast(GetHud());
        if (!hud || !hud.GetHudPanelWidget())
            return false;
        m_GRZFeed = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/feed.layout", hud.GetHudPanelWidget());
        return m_GRZFeed != null;
    }

    protected string GRZShortName(string name)
    {
        name.Replace("\n", " ");
        name.Replace("\r", " ");
        if (name.LengthUtf8() > 16)
            return name.SubstringUtf8(0, 13) + "...";
        return name;
    }

    protected void GRZFitName(TextWidget label, string name, int maximumWidth)
    {
        int count = name.LengthUtf8();
        label.SetText(name);
        int width, height;
        label.GetTextSize(width, height);
        while (width > maximumWidth && count > 1)
        {
            count--;
            label.SetText(name.SubstringUtf8(0, count) + "...");
            label.GetTextSize(width, height);
        }
    }

    protected void GRZReceiveKill(GRZKillfeedMessage message)
    {
        if (!message || !GRZEnsureFeed())
            return;
        GRZKillfeedRow row = new GRZKillfeedRow;
        row.Root = g_Game.GetWorkspace().CreateWidgets("GRZ_Killfeed/gui/layouts/row.layout", m_GRZFeed);
        if (!row.Root)
            return;
        row.Message = message;
        row.Killer = TextWidget.Cast(row.Root.FindAnyWidget("Killer"));
        row.Victim = TextWidget.Cast(row.Root.FindAnyWidget("Victim"));
        row.Killer.SetTextExactSize(17);
        row.Victim.SetTextExactSize(17);
        GRZFitName(row.Killer, GRZShortName(message.Killer), 125);
        GRZFitName(row.Victim, GRZShortName(message.Victim), 110);
        TextWidget.Cast(row.Root.FindAnyWidget("Arrow")).SetTextExactSize(17);
        TextWidget detail = TextWidget.Cast(row.Root.FindAnyWidget("Detail"));
        detail.SetText(message.Weapon + "  |  " + message.Distance.ToString() + " m");
        detail.SetTextExactSize(14);
        ImageWidget outline = ImageWidget.Cast(row.Root.FindAnyWidget("Outline"));
        outline.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.STRETCH);
        outline.LoadImageFile(0, "GRZ_Killfeed/gui/textures/grz_outline_ca.paa");
        m_GRZRows.InsertAt(row, 0);
        while (m_GRZRows.Count() > 5)
        {
            m_GRZRows[5].Root.Unlink();
            m_GRZRows.Remove(5);
        }
        row.Update(0);
        GRZPositionRows();
    }

    protected void GRZPositionRows()
    {
        IngameHud hud = IngameHud.Cast(GetHud());
        if (!hud)
            return;
        float x, y, width, height;
        hud.GetHudPanelWidget().GetScreenPos(x, y);
        hud.GetHudPanelWidget().GetScreenSize(width, height);
        m_GRZFeed.SetScreenPos(x + width - 504, y + 28);
        for (int i = 0; i < m_GRZRows.Count(); i++)
            m_GRZRows[i].Root.SetPos(0, i * 80);
    }

    protected void GRZReceiveMenu(int status, GRZNameStyle style)
    {
        GRZKillfeedMenu current = GRZKillfeedMenu.Cast(g_Game.GetUIManager().GetMenu());
        if (status == 0)
        {
            if (current)
                current.Close();
            PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
            if (player)
                player.MessageStatus("GRZ Killfeed: customization is not enabled for your Steam ID.");
            return;
        }
        if (status == 1)
        {
            if (g_Game.GetUIManager().GetMenu())
                return;
            GRZKillfeedMenu menu = new GRZKillfeedMenu;
            menu.Chosen = style;
            g_Game.GetUIManager().ShowScriptedMenu(menu, null);
        }
        else if (current)
            current.SetResult(status);
    }
}

class GRZKillfeedRow
{
    Widget Root;
    TextWidget Killer;
    TextWidget Victim;
    ref GRZKillfeedMessage Message;
    EntityAI PreviewGun;
    ItemPreviewWidget GunWidget;
    float Age;

    void CreateGunPreview()
    {
        GunWidget = ItemPreviewWidget.Cast(Root.FindAnyWidget("GunPreview"));
        GRZKillfeedGunData data = Message.GunData;
        if (!GunWidget || !data || !data.IsValid() || data.Types.Count() == 0)
        {
            if (GunWidget)
                GunWidget.Show(false);
            return;
        }
        if (!g_Game.ConfigIsExisting("CfgWeapons " + data.Types[0]))
        {
            GunWidget.Show(false);
            return;
        }
        PreviewGun = EntityAI.Cast(g_Game.CreateObjectEx(data.Types[0], "0 -1000 0", ECE_LOCAL | ECE_NOLIFETIME | ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN | ECE_NOPERSISTENCY_WORLD | ECE_NOPERSISTENCY_CHAR));
        if (!PreviewGun || !PreviewGun.IsWeapon())
        {
            if (PreviewGun)
                g_Game.ObjectDelete(PreviewGun);
            PreviewGun = null;
            GunWidget.Show(false);
            return;
        }
        array<EntityAI> nodes = new array<EntityAI>;
        nodes.Insert(PreviewGun);
        for (int i = 1; i < data.Types.Count(); i++)
        {
            EntityAI attached = null;
            EntityAI parent = nodes[data.Parents[i]];
            if (parent)
            {
                if (data.Slots[i] >= 0)
                    attached = parent.GetInventory().CreateAttachmentEx(data.Types[i], data.Slots[i]);
                if (!attached)
                    attached = parent.GetInventory().CreateAttachment(data.Types[i]);
            }
            nodes.Insert(attached);
        }
        GunWidget.SetItem(PreviewGun);
        GunWidget.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
        GunWidget.SetView(PreviewGun.GetViewIndex());
        GunWidget.SetModelPosition("0 0 1");
        GunWidget.SetModelOrientation("0 0 0");
        GunWidget.Show(true);
    }

    void Dispose()
    {
        if (GunWidget)
            GunWidget.SetItem(null);
        if (PreviewGun)
        {
            g_Game.ObjectDelete(PreviewGun);
            PreviewGun = null;
        }
        if (Root)
        {
            Root.Unlink();
            Root = null;
        }
    }

    void Update(float timeslice)
    {
        Age += timeslice;
        float fade = Math.Clamp(8.0 - Age, 0, 1);
        int killerColor = Message.KillerStyle.ColorAt(Age);
        int victimColor = Message.VictimStyle.ColorAt(Age);
        Killer.SetColor(GRZNameStyle.WithAlpha(killerColor, ((killerColor >> 24) & 255) * fade));
        Victim.SetColor(GRZNameStyle.WithAlpha(victimColor, ((victimColor >> 24) & 255) * fade));
        // Never make the transparent root opaque while fading its children.
        Root.FindAnyWidget("Backing").SetAlpha(0.45 * fade);
        array<string> names = {"Arrow", "Detail", "BorderTop", "BorderBottom", "BorderLeft", "BorderRight"};
        foreach (string name : names)
            Root.FindAnyWidget(name).SetAlpha(fade);
        if (GunWidget)
            GunWidget.SetAlpha(fade);
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
    protected bool m_GRZPreviewHeld;
    protected bool m_GRZAutoPreviewOpened;

    override void OnInit()
    {
        super.OnInit();
        if (!m_GRZListening)
        {
            GRZKillfeedBus.Kills.Insert(GRZReceiveKill);
            GRZKillfeedBus.Menu.Insert(GRZReceiveMenu);
            g_Game.GetUpdateQueue(CALL_CATEGORY_GUI).Insert(GRZHandleMenuShortcut);
            m_GRZListening = true;
        }
    }

    override void OnMissionFinish()
    {
        GRZKillfeedBus.Kills.Remove(GRZReceiveKill);
        GRZKillfeedBus.Menu.Remove(GRZReceiveMenu);
        m_GRZListening = false;
        g_Game.GetUpdateQueue(CALL_CATEGORY_GUI).Remove(GRZHandleMenuShortcut);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZKillfeedClient.OpenLocalPreview);
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZKillfeedClient.RequestMenu);
        foreach (GRZKillfeedRow row : m_GRZRows)
            row.Dispose();
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
                m_GRZRows[i].Dispose();
                m_GRZRows.Remove(i);
            }
        }
        GRZPositionRows();
        IngameHud hud = IngameHud.Cast(GetHud());
        if (hud)
            m_GRZFeed.Show(!hud.GetHudVisibility().IsContextFlagActive(IngameHudVisibility.HUD_HIDE_FLAGS));
    }

    protected void GRZHandleMenuShortcut(float timeslice)
    {
        // Registered input plus native key-state fallback; neither depends on
        // Editor forwarding MissionGameplay.OnKeyPress to this mod.
        bool held = (KeyState(KeyCode.KC_F7) & 0x8000) != 0;
        bool pressed = g_Game.GetInput().LocalPress("UAGRZKillfeedMenu", false);
        if (pressed || (held && !m_GRZPreviewHeld))
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.RequestMenu, 1, false);
        m_GRZPreviewHeld = held;
        if (!m_GRZAutoPreviewOpened && IsCLIParam("GRZKillfeedPreview"))
        {
            m_GRZAutoPreviewOpened = true;
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.OpenLocalPreview, 1000, false);
        }
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
        GRZFitName(row.Killer, GRZShortName(message.Killer), 160);
        GRZFitName(row.Victim, GRZShortName(message.Victim), 170);
        TextWidget.Cast(row.Root.FindAnyWidget("Arrow")).SetTextExactSize(17);
        TextWidget detail = TextWidget.Cast(row.Root.FindAnyWidget("Detail"));
        detail.SetText(message.Distance.ToString() + " m");
        detail.SetTextExactSize(14);
        row.CreateGunPreview();
        m_GRZRows.InsertAt(row, 0);
        while (m_GRZRows.Count() > 5)
        {
            m_GRZRows[5].Dispose();
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
        m_GRZFeed.SetScreenPos(x + width - 424, y + 28);
        for (int i = 0; i < m_GRZRows.Count(); i++)
            m_GRZRows[i].Root.SetPos(0, i * 96);
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

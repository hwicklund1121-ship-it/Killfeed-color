class GRZKillfeedRow
{
    Widget Root;
    TextWidget Killer;
    TextWidget Victim;
    TextWidget KillerPrefix;
    TextWidget VictimPrefix;
    ImageWidget KillerPrefixIcon;
    ImageWidget VictimPrefixIcon;
    ref GRZKillfeedMessage Message;
    EntityAI PreviewGun;
    ItemPreviewWidget GunWidget;
    float Age;
    int Width = 580;

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
        GunWidget.SetModelPosition("0 0 0.55");
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
        int killerPrefixColor = killerColor;
        int victimPrefixColor = victimColor;
        killerPrefixColor = GRZAnimationStyle.WithAlpha(killerPrefixColor, ((killerPrefixColor >> 24) & 255) * fade);
        victimPrefixColor = GRZAnimationStyle.WithAlpha(victimPrefixColor, ((victimPrefixColor >> 24) & 255) * fade);
        KillerPrefix.SetColor(killerPrefixColor);
        KillerPrefixIcon.SetColor(killerPrefixColor);
        VictimPrefix.SetColor(victimPrefixColor);
        VictimPrefixIcon.SetColor(victimPrefixColor);
        // Never make the transparent root opaque while fading its children.
        Root.FindAnyWidget("Backing").SetAlpha(0.45 * fade);
        array<string> names = {"Detail", "BorderTop", "BorderBottom", "BorderLeft", "BorderRight"};
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

    override void OnClientDisconnectedEvent(PlayerIdentity identity, PlayerBase player, int logoutTime, bool authFailed)
    {
        if (player)
            player.GRZResetKillstreak();
        super.OnClientDisconnectedEvent(identity, player, logoutTime, authFailed);
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
            if (IsCLIParam("GRZKillfeedPreview"))
                g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.OpenLocalPreview, 1000, false);
            m_GRZListening = true;
        }
    }

    override void OnEvent(EventType eventTypeId, Param params)
    {
        if (eventTypeId == ChatMessageEventTypeID && GRZKillfeedClient.CanLocalPreview())
        {
            ChatMessageEventParams chat = ChatMessageEventParams.Cast(params);
            if (chat)
            {
                string command = chat.param3.Trim();
                command.ToLower();
                if (command == "/grzvip")
                {
                    g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZKillfeedClient.RequestMenu, 150, false);
                    return;
                }
            }
        }
        super.OnEvent(eventTypeId, params);
    }

    override void OnMissionFinish()
    {
        GRZKillfeedBus.Kills.Remove(GRZReceiveKill);
        GRZKillfeedBus.Menu.Remove(GRZReceiveMenu);
        m_GRZListening = false;
        GRZKillfeedClient.ReleaseMenu();
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

    protected int GRZLayoutName(TextWidget label, TextWidget prefix, ImageWidget icon, string playerName, GRZNameStyle style, int left, bool alignRight)
    {
        prefix.SetTextExactSize(16);
        prefix.SetOutline(1, 0xFF000000);
        string word;
        if (style.PrefixWord != "" || style.PrefixIcon > 0)
            word = style.PrefixWord + " |";
        prefix.SetText(word);
        int prefixWidth, prefixHeight;
        prefix.GetTextSize(prefixWidth, prefixHeight);
        if (word == "")
            prefixWidth = 0;
        int iconWidth;
        if (style.PrefixIcon > 0)
            iconWidth = 22;
        int gap;
        if (prefixWidth > 0)
            gap = 4;
        int reserved = iconWidth + prefixWidth + gap;
        GRZFitName(label, playerName, 140);
        int nameWidth, nameHeight;
        label.GetTextSize(nameWidth, nameHeight);
        int start = left;
        if (alignRight)
            start += Math.Max(0, 140 - nameWidth);
        GRZNameStyle.ApplyIcon(icon, style.PrefixIcon);
        icon.SetPos(start, 19);
        prefix.SetPos(start + iconWidth, 19);
        prefix.SetSize(prefixWidth + 2, 22);
        label.SetPos(start + reserved, 16);
        label.SetSize(Math.Min(nameWidth + 2, 140), 28);
        return reserved;
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
        row.Killer.SetTextExactSize(24);
        row.Victim.SetTextExactSize(24);
        row.Killer.SetOutline(1, 0xFF000000);
        row.Victim.SetOutline(1, 0xFF000000);
        row.KillerPrefix = TextWidget.Cast(row.Root.FindAnyWidget("KillerPrefix"));
        row.VictimPrefix = TextWidget.Cast(row.Root.FindAnyWidget("VictimPrefix"));
        row.KillerPrefixIcon = ImageWidget.Cast(row.Root.FindAnyWidget("KillerPrefixIcon"));
        row.VictimPrefixIcon = ImageWidget.Cast(row.Root.FindAnyWidget("VictimPrefixIcon"));
        int killerExtra = GRZLayoutName(row.Killer, row.KillerPrefix, row.KillerPrefixIcon, GRZShortName(message.Killer), message.KillerStyle, 12, true);
        int victimExtra = GRZLayoutName(row.Victim, row.VictimPrefix, row.VictimPrefixIcon, GRZShortName(message.Victim), message.VictimStyle, 428 + killerExtra, false);
        row.Width = 580 + killerExtra + victimExtra;
        row.Root.SetSize(row.Width, 60);
        row.Root.FindAnyWidget("Backing").SetSize(row.Width - 4, 56);
        row.Root.FindAnyWidget("BorderTop").SetSize(row.Width, 2);
        row.Root.FindAnyWidget("BorderBottom").SetSize(row.Width, 2);
        row.Root.FindAnyWidget("BorderRight").SetPos(row.Width - 2, 2);
        row.Root.FindAnyWidget("GunPreview").SetPos(164 + killerExtra, 2);
        TextWidget detail = TextWidget.Cast(row.Root.FindAnyWidget("Detail"));
        detail.SetPos(352 + killerExtra, 16);
        detail.SetText(message.Distance.ToString() + " m");
        detail.SetTextExactSize(20);
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
        int maximumWidth = 580;
        foreach (GRZKillfeedRow entry : m_GRZRows)
            maximumWidth = Math.Max(maximumWidth, entry.Width);
        m_GRZFeed.SetSize(maximumWidth, 320);
        m_GRZFeed.SetScreenPos(x + width - maximumWidth - 24, y + 28);
        for (int i = 0; i < m_GRZRows.Count(); i++)
            m_GRZRows[i].Root.SetPos(maximumWidth - m_GRZRows[i].Width, i * 64);
    }

    protected void GRZReceiveMenu(int status, GRZNameStyle style)
    {
        GRZKillfeedMenu current = GRZKillfeedMenu.Cast(g_Game.GetUIManager().GetMenu());
        if (!current)
            return;
        if (status == 0)
            current.UseLocalPreview();
        else if (status == 1)
            current.ApplyServerStyle(style);
        else
            current.SetResult(status);
    }
};

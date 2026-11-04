class GRZKillfeedRow
{
    Widget Root;
    TextWidget Killer;
    TextWidget Victim;
    TextWidget KillerPrefix;
    TextWidget VictimPrefix;
    ImageWidget KillerPrefixIcon;
    ImageWidget VictimPrefixIcon;
    TextWidget WeaponFallback;
    ref GRZKillfeedMessage Message;
    EntityAI PreviewGun;
    ItemPreviewWidget GunWidget;
    float Age;
    int Width = 438;

    void CreateGunPreview()
    {
        if (!Root || !Message)
            return;
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
        if (WeaponFallback)
            WeaponFallback.Show(false);
    }

    void Dispose()
    {
        if (g_Game)
            g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(CreateGunPreview);
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
        Root.FindAnyWidget("Backing").SetAlpha(0.18 * fade);
        array<string> names = {"Detail"};
        foreach (string name : names)
            Root.FindAnyWidget(name).SetAlpha(fade);
        if (GunWidget)
            GunWidget.SetAlpha(fade);
        if (WeaponFallback)
            WeaponFallback.SetAlpha(fade);
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
    protected ref GRZKillstreakPopup m_GRZStreakPopup;

    override void OnInit()
    {
        super.OnInit();
        if (!m_GRZListening)
        {
            GRZKillfeedBus.Kills.Insert(GRZReceiveKill);
            GRZKillfeedBus.Menu.Insert(GRZReceiveMenu);
            GRZKillfeedBus.Streak.Insert(GRZReceiveStreak);
            m_GRZStreakPopup = new GRZKillstreakPopup;
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
        GRZKillfeedBus.Streak.Remove(GRZReceiveStreak);
        if (m_GRZStreakPopup)
            m_GRZStreakPopup.Dispose();
        m_GRZStreakPopup = null;
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
        IngameHud streakHud = IngameHud.Cast(GetHud());
        if (m_GRZStreakPopup && streakHud)
            m_GRZStreakPopup.Update(timeslice, streakHud.GetHudPanelWidget());
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

    protected int GRZLayoutName(TextWidget label, TextWidget prefix, ImageWidget icon, string playerName, GRZNameStyle style, int left)
    {
        prefix.SetTextExactSize(10);
        prefix.SetOutline(0, 0x00000000);
        string word;
        if (style.PrefixWord != "" || style.PrefixIcon > 0)
            word = style.PrefixWord + " |";
        GRZFitName(prefix, word, 60);
        int prefixWidth, prefixHeight;
        prefix.GetTextSize(prefixWidth, prefixHeight);
        if (word == "")
            prefixWidth = 0;
        int iconWidth;
        if (style.PrefixIcon > 0)
            iconWidth = 12;
        int gap;
        if (prefixWidth > 0)
            gap = 3;
        int reserved = iconWidth + prefixWidth + gap;
        int nameLimit = 128 - reserved;
        GRZFitName(label, playerName, nameLimit);
        int nameWidth, nameHeight;
        label.GetTextSize(nameWidth, nameHeight);
        int start = left;
        GRZNameStyle.ApplyIcon(icon, style.PrefixIcon);
        icon.SetPos(start, 6);
        prefix.SetPos(start + iconWidth, 6);
        prefix.SetSize(prefixWidth + 2, 12);
        label.SetPos(start + reserved, 4);
        label.SetSize(Math.Min(nameWidth + 2, nameLimit), 16);
        return reserved + Math.Min(nameWidth + 2, nameLimit);
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
        row.Killer.SetTextExactSize(12);
        row.Victim.SetTextExactSize(12);
        row.Killer.SetOutline(0, 0x00000000);
        row.Victim.SetOutline(0, 0x00000000);
        row.KillerPrefix = TextWidget.Cast(row.Root.FindAnyWidget("KillerPrefix"));
        row.VictimPrefix = TextWidget.Cast(row.Root.FindAnyWidget("VictimPrefix"));
        row.KillerPrefixIcon = ImageWidget.Cast(row.Root.FindAnyWidget("KillerPrefixIcon"));
        row.VictimPrefixIcon = ImageWidget.Cast(row.Root.FindAnyWidget("VictimPrefixIcon"));
        // Measure each name group instead of reserving empty fixed-width columns.
        int killerWidth = GRZLayoutName(row.Killer, row.KillerPrefix, row.KillerPrefixIcon, GRZShortName(message.Killer), message.KillerStyle, 6);
        int gunLeft = 6 + killerWidth + 6;
        int gunWidth = 100;
        int victimLeft = gunLeft + gunWidth + 6;
        int victimWidth = GRZLayoutName(row.Victim, row.VictimPrefix, row.VictimPrefixIcon, GRZShortName(message.Victim), message.VictimStyle, victimLeft);
        row.Root.FindAnyWidget("GunPreview").SetPos(gunLeft, 1);
        row.Root.FindAnyWidget("GunPreview").SetSize(gunWidth, 22);
        TextWidget detail = TextWidget.Cast(row.Root.FindAnyWidget("Detail"));
        int distanceLeft = victimLeft + victimWidth + 6;
        detail.SetPos(distanceLeft, 4);
        detail.SetText(message.Distance.ToString() + "m");
        detail.SetTextExactSize(11);
        detail.SetOutline(0, 0x00000000);
        GRZFitName(detail, message.Distance.ToString() + "m", 42);
        int distanceWidth, distanceHeight;
        detail.GetTextSize(distanceWidth, distanceHeight);
        distanceWidth += 2;
        detail.SetSize(distanceWidth, 16);
        int contentWidth = distanceLeft + distanceWidth + 6;
        row.Width = 438;
        row.Root.SetSize(row.Width, 24);
        row.Root.FindAnyWidget("Backing").SetSize(row.Width, 24);
        row.WeaponFallback = TextWidget.Cast(row.Root.FindAnyWidget("WeaponFallback"));
        if (row.WeaponFallback)
        {
            row.WeaponFallback.SetPos(gunLeft, 4);
            row.WeaponFallback.SetSize(gunWidth, 16);
            row.WeaponFallback.SetTextExactSize(10);
            row.WeaponFallback.SetOutline(0, 0x00000000);
            if (message.GunData && message.GunData.Types.Count() > 0)
                GRZFitName(row.WeaponFallback, message.GunData.Types[0], gunWidth);
        }
        // Keep the reference-size backing while packing short names close together.
        float shift = Math.Max(0, (438 - contentWidth) * 0.5);
        array<string> content = {"Killer", "KillerPrefix", "KillerPrefixIcon", "GunPreview", "WeaponFallback", "Victim", "VictimPrefix", "VictimPrefixIcon", "Detail"};
        foreach (string childName : content)
        {
            Widget child = row.Root.FindAnyWidget(childName);
            if (child)
            {
                float childX, childY;
                child.GetPos(childX, childY);
                child.SetPos(childX + shift, childY);
            }
        }
        row.Root.FindAnyWidget("GunPreview").Show(false);
        m_GRZRows.InsertAt(row, 0);
        while (m_GRZRows.Count() > 5)
        {
            m_GRZRows[5].Dispose();
            m_GRZRows.Remove(5);
        }
        row.Update(0);
        GRZPositionRows();
        // Keep script-heavy modded weapon initialization outside row delivery.
        g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(row.CreateGunPreview, 1, false);
        Print("[GRZ Killfeed 0.1.22] Live/test row added; preview deferred.");
    }

    protected void GRZReceiveStreak(string playerName, int count)
    {
        if (!m_GRZStreakPopup)
            m_GRZStreakPopup = new GRZKillstreakPopup;
        m_GRZStreakPopup.Enqueue(playerName, count);
    }

    protected void GRZPositionRows()
    {
        IngameHud hud = IngameHud.Cast(GetHud());
        if (!hud)
            return;
        float x, y, width, height;
        hud.GetHudPanelWidget().GetScreenPos(x, y);
        hud.GetHudPanelWidget().GetScreenSize(width, height);
        int maximumWidth = 0;
        foreach (GRZKillfeedRow entry : m_GRZRows)
            maximumWidth = Math.Max(maximumWidth, entry.Width);
        m_GRZFeed.SetSize(maximumWidth, 160);
        m_GRZFeed.SetScreenPos(x + width - maximumWidth - 24, y + 28);
        for (int i = 0; i < m_GRZRows.Count(); i++)
            m_GRZRows[i].Root.SetPos(maximumWidth - m_GRZRows[i].Width, i * 28);
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

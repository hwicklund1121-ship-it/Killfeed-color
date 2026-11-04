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
    int Width = 720;

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
        GunWidget.SetModelPosition("0 0 0.32");
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
        Root.FindAnyWidget("Backing").SetColor(GRZNameStyle.WithAlpha(0xFF808080, 90 * fade));
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
    protected PlayerBase m_GRZStyleOwner;
    protected bool m_GRZStyleSynced;
    protected float m_GRZStyleSyncElapsed;
    protected int m_GRZStyleSyncAttempts;

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
        m_GRZStyleOwner = null;
        m_GRZStyleSynced = false;
        GRZKillfeedClient.ServerAuthorized = false;
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
        GRZSyncSavedStyle(timeslice);
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

    protected void GRZSyncSavedStyle(float timeslice)
    {
        if (!g_Game.IsMultiplayer() || g_Game.IsServer())
            return;
        PlayerBase localPlayer = PlayerBase.Cast(g_Game.GetPlayer());
        if (localPlayer != m_GRZStyleOwner)
        {
            m_GRZStyleOwner = localPlayer;
            m_GRZStyleSynced = false;
            m_GRZStyleSyncElapsed = 0;
            m_GRZStyleSyncAttempts = 0;
            GRZKillfeedClient.LocalStyle = new GRZNameStyle;
            GRZKillfeedClient.ServerAuthorized = false;
        }
        if (!localPlayer || m_GRZStyleSynced || m_GRZStyleSyncAttempts >= 5)
            return;
        m_GRZStyleSyncElapsed += Math.Max(0, timeslice);
        if (m_GRZStyleSyncElapsed < 2.0)
            return;
        m_GRZStyleSyncElapsed = 0;
        m_GRZStyleSyncAttempts++;
        // Existing authenticated request/reply path; no menu is opened here.
        ScriptRPC rpc = new ScriptRPC;
        rpc.Send(localPlayer, GRZKillfeedRPC.REQUEST_MENU, true, null);
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
        prefix.SetTextExactSize(18);
        prefix.SetOutline(0, 0x00000000);
        string word;
        if (style.PrefixWord != "" || style.PrefixIcon > 0)
            word = style.PrefixWord + " |";
        GRZFitName(prefix, word, 88);
        int prefixWidth, prefixHeight;
        prefix.GetTextSize(prefixWidth, prefixHeight);
        if (word == "")
            prefixWidth = 0;
        int iconWidth;
        if (style.PrefixIcon > 0)
            iconWidth = 24;
        int gap;
        if (prefixWidth > 0)
            gap = 6;
        int reserved = iconWidth + prefixWidth + gap;
        int nameLimit = 190 - reserved;
        GRZFitName(label, playerName, nameLimit);
        int nameWidth, nameHeight;
        label.GetTextSize(nameWidth, nameHeight);
        int start = left;
        GRZNameStyle.ApplyIcon(icon, style.PrefixIcon);
        icon.SetPos(start, 11);
        prefix.SetPos(start + iconWidth, 11);
        prefix.SetSize(prefixWidth + 2, 24);
        label.SetPos(start + reserved, 9);
        label.SetSize(Math.Min(nameWidth + 2, nameLimit), 28);
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
        row.Killer.SetTextExactSize(22);
        row.Victim.SetTextExactSize(22);
        row.Killer.SetOutline(0, 0x00000000);
        row.Victim.SetOutline(0, 0x00000000);
        row.KillerPrefix = TextWidget.Cast(row.Root.FindAnyWidget("KillerPrefix"));
        row.VictimPrefix = TextWidget.Cast(row.Root.FindAnyWidget("VictimPrefix"));
        row.KillerPrefixIcon = ImageWidget.Cast(row.Root.FindAnyWidget("KillerPrefixIcon"));
        row.VictimPrefixIcon = ImageWidget.Cast(row.Root.FindAnyWidget("VictimPrefixIcon"));
        // Larger text/model occupy spaced columns across the full long row.
        GRZLayoutName(row.Killer, row.KillerPrefix, row.KillerPrefixIcon, GRZShortName(message.Killer), message.KillerStyle, 14);
        int gunLeft = 218;
        int gunWidth = 200;
        int victimLeft = 432;
        GRZLayoutName(row.Victim, row.VictimPrefix, row.VictimPrefixIcon, GRZShortName(message.Victim), message.VictimStyle, victimLeft);
        row.Root.FindAnyWidget("GunPreview").SetPos(gunLeft, 2);
        row.Root.FindAnyWidget("GunPreview").SetSize(gunWidth, 42);
        TextWidget detail = TextWidget.Cast(row.Root.FindAnyWidget("Detail"));
        int distanceLeft = 636;
        detail.SetPos(distanceLeft, 9);
        detail.SetText(message.Distance.ToString() + "m");
        detail.SetTextExactSize(20);
        detail.SetOutline(0, 0x00000000);
        GRZFitName(detail, message.Distance.ToString() + "m", 68);
        detail.SetSize(70, 28);
        row.Width = 720;
        row.Root.SetSize(row.Width, 46);
        row.Root.FindAnyWidget("Backing").SetSize(row.Width, 46);
        row.WeaponFallback = TextWidget.Cast(row.Root.FindAnyWidget("WeaponFallback"));
        if (row.WeaponFallback)
        {
            row.WeaponFallback.SetPos(gunLeft, 9);
            row.WeaponFallback.SetSize(gunWidth, 28);
            row.WeaponFallback.SetTextExactSize(18);
            row.WeaponFallback.SetOutline(0, 0x00000000);
            if (message.GunData && message.GunData.Types.Count() > 0)
                GRZFitName(row.WeaponFallback, message.GunData.Types[0], gunWidth);
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
        Print(string.Format("[GRZ Killfeed 0.1.27] Row added: killerEffect=%1 victimEffect=%2 prefixIcon=%3; preview deferred.", message.KillerStyle.AnimationMode, message.VictimStyle.AnimationMode, message.KillerStyle.PrefixIcon));
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
        m_GRZFeed.SetSize(maximumWidth, 250);
        m_GRZFeed.SetScreenPos(x + width - maximumWidth - 24, y + 28);
        for (int i = 0; i < m_GRZRows.Count(); i++)
            m_GRZRows[i].Root.SetPos(maximumWidth - m_GRZRows[i].Width, i * 50);
    }

    protected void GRZReceiveMenu(int status, GRZNameStyle style)
    {
        m_GRZStyleSynced = true;
        if (status == 0)
            GRZKillfeedClient.ServerAuthorized = false;
        else if (status == 1 || status == 2)
            GRZKillfeedClient.ServerAuthorized = true;
        // A save acknowledgement carries the authoritative persisted style too.
        if ((status == 1 || status == 2) && style)
            GRZKillfeedClient.LocalStyle = style.Clone();
        GRZKillfeedMenu current = GRZKillfeedMenu.Cast(g_Game.GetUIManager().GetMenu());
        if (!current)
            return;
        if (status == 0)
            current.UseLocalPreview();
        else if (status == 1 || status == 2)
        {
            current.ApplyServerStyle(style);
            if (status == 2)
                current.SetResult(status);
        }
        else
            current.SetResult(status);
    }
};

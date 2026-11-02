class GRZKillfeedServer
{
    protected static bool s_Loaded;
    protected static bool s_CanSave = true;
    protected static ref GRZKillfeedConfig s_Config;
    protected static ref GRZKillfeedSavedStyles s_Styles;
    static const string DIRECTORY = "$profile:GRZ_Killfeed";
    static const string CONFIG = "$profile:GRZ_Killfeed/config.json";
    static const string STYLES = "$profile:GRZ_Killfeed/styles.json";

    static void Initialize()
    {
        if (s_Loaded || !g_Game.IsServer())
            return;
        s_Loaded = true;
        s_Config = new GRZKillfeedConfig;
        s_Styles = new GRZKillfeedSavedStyles;
        MakeDirectory(DIRECTORY);
        string error;
        if (FileExist(CONFIG))
        {
            if (!JsonFileLoader<GRZKillfeedConfig>.LoadFile(CONFIG, s_Config, error) || !s_Config || !s_Config.AllowedSteamIds)
            {
                s_Config = new GRZKillfeedConfig; // Fail closed, preserve bad file.
                Error("GRZ Killfeed: config could not be loaded. Customization is denied until config.json is fixed and the server restarts.");
            }
        }
        else if (!JsonFileLoader<GRZKillfeedConfig>.SaveFile(CONFIG, s_Config, error))
            Error("GRZ Killfeed: could not create config.json in the server profiles directory.");

        if (FileExist(STYLES))
        {
            if (!JsonFileLoader<GRZKillfeedSavedStyles>.LoadFile(STYLES, s_Styles, error) || !s_Styles || !s_Styles.Players)
            {
                s_Styles = new GRZKillfeedSavedStyles;
                s_CanSave = false; // Never overwrite a malformed preferences file.
                Error("GRZ Killfeed: styles.json could not be loaded; saving is disabled until it is repaired and the server restarts.");
            }
        }
    }

    static bool Allowed(PlayerIdentity identity)
    {
        Initialize();
        return identity && s_Config && s_Config.AllowedSteamIds.Find(identity.GetPlainId()) >= 0;
    }

    static GRZNameStyle StyleFor(PlayerIdentity identity)
    {
        Initialize();
        if (Allowed(identity))
        {
            foreach (GRZKillfeedSavedStyle saved : s_Styles.Players)
            {
                if (saved && saved.SteamId == identity.GetPlainId() && saved.Style && saved.Style.IsValid())
                    return saved.Style;
            }
        }
        return new GRZNameStyle;
    }

    static bool SaveStyle(PlayerIdentity identity, GRZNameStyle style)
    {
        if (!Allowed(identity) || !style || !style.IsValid() || !s_CanSave)
            return false;
        GRZKillfeedSavedStyles next = new GRZKillfeedSavedStyles;
        foreach (GRZKillfeedSavedStyle saved : s_Styles.Players)
        {
            if (saved && saved.SteamId != identity.GetPlainId())
                next.Players.Insert(saved);
        }
        GRZKillfeedSavedStyle updated = new GRZKillfeedSavedStyle;
        updated.SteamId = identity.GetPlainId();
        updated.Style = style;
        next.Players.Insert(updated);
        string error;
        string temporary = DIRECTORY + "/styles.pending.json";
        if (!JsonFileLoader<GRZKillfeedSavedStyles>.SaveFile(temporary, next, error))
            return false;
        if (FileExist(STYLES) && !CopyFile(STYLES, DIRECTORY + "/styles.backup.json"))
            return false;
        if (!CopyFile(temporary, STYLES))
            return false;
        DeleteFile(temporary);
        s_Styles = next;
        return true;
    }

    static void Reply(PlayerBase player, int status)
    {
        if (!player || !player.GetIdentity())
            return;
        GRZNameStyle style = StyleFor(player.GetIdentity());
        ScriptRPC rpc = new ScriptRPC;
        rpc.Write(status);
        WriteStyle(rpc, style);
        rpc.Send(player, GRZKillfeedRPC.MENU_STATE, true, player.GetIdentity());
    }

    static void WriteStyle(ScriptRPC rpc, GRZNameStyle style)
    {
        rpc.Write(style.ColorA);
        rpc.Write(style.ColorB);
        rpc.Write(style.AnimationMode);
        rpc.Write(style.Speed);
    }

    static void PlayerKilled(PlayerBase victim, Object source)
    {
        if (!g_Game.IsServer() || !g_Game.IsMultiplayer() || !victim || !victim.GetIdentity() || !source)
            return;
        PlayerBase killer = PlayerBase.Cast(source);
        EntityAI entity = EntityAI.Cast(source);
        if (!killer && entity)
        {
            killer = PlayerBase.Cast(entity.GetHierarchyRootPlayer());
        }
        Transport vehicle = Transport.Cast(source);
        if (vehicle)
        {
            killer = PlayerBase.Cast(vehicle.CrewDriver());
        }
        // No guessing from the last player who hit a victim: indirect deaths
        // without a player-owned fatal source do not become fake player kills.
        if (!killer || killer == victim || !killer.GetIdentity())
            return;
        int distance = Math.Round(vector.Distance(killer.GetPosition(), victim.GetPosition()));
        GRZNameStyle killerStyle = StyleFor(killer.GetIdentity());
        GRZNameStyle victimStyle = StyleFor(victim.GetIdentity());
        GRZKillfeedGunData gun = new GRZKillfeedGunData;
        if (entity && entity.IsWeapon())
        {
            array<EntityAI> captured = new array<EntityAI>;
            CaptureGun(entity, -1, gun, 0, captured);
            Weapon_Base firearm = Weapon_Base.Cast(entity);
            if (firearm)
            {
                for (int muzzle = 0; muzzle < firearm.GetMuzzleCount(); muzzle++)
                    CaptureGun(firearm.GetMagazine(muzzle), 0, gun, 1, captured);
            }
        }
        string weaponJson;
        string jsonError;
        JsonFileLoader<GRZKillfeedGunData>.MakeData(gun, weaponJson, jsonError, false);
        array<Man> players = new array<Man>;
        g_Game.GetPlayers(players);
        foreach (Man man : players)
        {
            PlayerBase recipient = PlayerBase.Cast(man);
            if (!recipient || !recipient.GetIdentity())
                continue;
            ScriptRPC rpc = new ScriptRPC;
            rpc.Write(killer.GetIdentity().GetName());
            rpc.Write(victim.GetIdentity().GetName());
            rpc.Write(weaponJson);
            rpc.Write(distance);
            WriteStyle(rpc, killerStyle);
            WriteStyle(rpc, victimStyle);
            rpc.Send(recipient, GRZKillfeedRPC.KILL, true, recipient.GetIdentity());
        }
    }

    protected static void CaptureGun(EntityAI item, int parent, GRZKillfeedGunData gun, int depth, array<EntityAI> captured)
    {
        if (!item || captured.Find(item) >= 0 || gun.Types.Count() >= 32 || depth > 4)
            return;
        InventoryLocation location = new InventoryLocation;
        int slot = -1;
        if (item.GetInventory().GetCurrentInventoryLocation(location))
            slot = location.GetSlot();
        int index = gun.Types.Count();
        gun.Types.Insert(item.GetType());
        gun.Parents.Insert(parent);
        gun.Slots.Insert(slot);
        captured.Insert(item);
        for (int i = 0; i < item.GetInventory().AttachmentCount(); i++)
            CaptureGun(item.GetInventory().GetAttachmentFromIndex(i), index, gun, depth + 1, captured);
    }
}

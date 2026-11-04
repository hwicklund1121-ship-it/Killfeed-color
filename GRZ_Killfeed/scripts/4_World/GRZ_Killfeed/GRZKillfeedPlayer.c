modded class PlayerBase
{
    protected bool m_GRZKillfeedReported;
    protected int m_GRZKillfeedLastMenu;
    protected int m_GRZKillfeedLastSave;
    protected int m_GRZKillstreak;

    void GRZResetKillstreak()
    {
        if (g_Game.IsServer())
            m_GRZKillstreak = 0;
    }

    int GRZRecordKill()
    {
        if (!g_Game.IsServer() || !GetIdentity() || !IsAlive())
            return 0;
        m_GRZKillstreak++;
        if (m_GRZKillstreak % 10 == 0)
            return m_GRZKillstreak;
        return 0;
    }

    override void EEKilled(Object killer)
    {
        // Capture identity/source before vanilla death cleanup.
        if (g_Game.IsServer() && !m_GRZKillfeedReported)
        {
            m_GRZKillfeedReported = true;
            GRZResetKillstreak();
            GRZKillfeedServer.PlayerKilled(this, killer);
        }
        super.EEKilled(killer);
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type == GRZKillfeedRPC.REQUEST_MENU || rpc_type == GRZKillfeedRPC.SAVE_STYLE)
        {
            // A caller can only request or change the style of its own entity.
            if (!g_Game.IsServer() || !sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId())
                return;
            int now = g_Game.GetTime();
            if (rpc_type == GRZKillfeedRPC.REQUEST_MENU)
            {
                if (m_GRZKillfeedLastMenu && now - m_GRZKillfeedLastMenu < 1000)
                    return;
                m_GRZKillfeedLastMenu = now;
                int status = 0;
                if (GRZKillfeedServer.Allowed(sender))
                    status = 1;
                GRZKillfeedServer.Reply(this, status);
                return;
            }
            if (m_GRZKillfeedLastSave && now - m_GRZKillfeedLastSave < 1000)
            {
                GRZKillfeedServer.Reply(this, 3);
                return;
            }
            m_GRZKillfeedLastSave = now;
            GRZNameStyle chosen = new GRZNameStyle;
            if (!GRZReadStyle(ctx, chosen))
                return;
            if (!GRZKillfeedServer.Allowed(sender))
            {
                GRZKillfeedServer.Reply(this, 0);
                return;
            }
            if (GRZKillfeedServer.SaveStyle(sender, chosen))
                GRZKillfeedServer.Reply(this, 2);
            else
                GRZKillfeedServer.Reply(this, 3);
            return;
        }
        if (rpc_type == GRZKillfeedRPC.STREAK)
        {
            if (g_Game.IsServer() || sender || this != PlayerBase.Cast(g_Game.GetPlayer()))
                return;
            string streakName;
            int streakCount;
            if (!ctx.Read(streakName) || !ctx.Read(streakCount) || streakName.Length() > 128 || streakCount <= 0 || streakCount % 10 != 0)
            {
                Print("[GRZ Killstreaks] Unsupported notification payload; check matching client/server GRZ versions.");
                return;
            }
            GRZKillfeedBus.Streak.Invoke(streakName, streakCount);
            return;
        }
        if (rpc_type == GRZKillfeedRPC.KILL || rpc_type == GRZKillfeedRPC.MENU_STATE)
        {
            // Server messages only; clients cannot inject killfeed events.
            if (g_Game.IsServer() || sender || this != PlayerBase.Cast(g_Game.GetPlayer()))
                return;
            if (rpc_type == GRZKillfeedRPC.KILL)
            {
                GRZKillfeedMessage message = new GRZKillfeedMessage;
                if (!ctx.Read(message.Killer) || !ctx.Read(message.Victim) || !ctx.Read(message.WeaponJson) || !ctx.Read(message.Distance))
                    return;
                if (!GRZReadStyle(ctx, message.KillerStyle, false) || !GRZReadStyle(ctx, message.VictimStyle, false))
                {
                    Print("[GRZ Killfeed] Live kill RPC style fields could not be read.");
                    return;
                }
                if (!message.KillerStyle.IsValid())
                {
                    message.KillerStyle = message.KillerStyle.Clone();
                }
                if (!message.KillerStyle.IsValid())
                {
                    Print("[GRZ Killfeed] Client-incompatible killer style; drawing default instead of dropping the row.");
                    message.KillerStyle = new GRZNameStyle;
                }
                if (!message.VictimStyle.IsValid())
                    message.VictimStyle = message.VictimStyle.Clone();
                if (!message.VictimStyle.IsValid())
                    message.VictimStyle = new GRZNameStyle;
                Print("[GRZ Killfeed 0.1.27] Live kill RPC received.");
                string error;
                GRZKillfeedGunData gun;
                if (message.WeaponJson.Length() <= 16384 && JsonFileLoader<GRZKillfeedGunData>.LoadData(message.WeaponJson, gun, error) && gun && gun.IsValid())
                    message.GunData = gun;
                GRZKillfeedBus.Kills.Invoke(message);
            }
            else
            {
                int menuStatus;
                GRZNameStyle received = new GRZNameStyle;
                if (ctx.Read(menuStatus) && GRZReadStyle(ctx, received))
                    GRZKillfeedBus.Menu.Invoke(menuStatus, received);
            }
            return;
        }
        super.OnRPC(sender, rpc_type, ctx);
    }

    protected bool GRZReadStyle(ParamsReadContext ctx, GRZNameStyle style, bool requireValid = true)
    {
        return ctx.Read(style.ColorA) && ctx.Read(style.ColorB) && ctx.Read(style.AnimationMode) && ctx.Read(style.Speed) && ctx.Read(style.PrefixIcon) && ctx.Read(style.PrefixWord) && ctx.Read(style.PrefixAnimation.ColorA) && ctx.Read(style.PrefixAnimation.ColorB) && ctx.Read(style.PrefixAnimation.AnimationMode) && ctx.Read(style.PrefixAnimation.Speed) && (!requireValid || style.IsValid());
    }
}

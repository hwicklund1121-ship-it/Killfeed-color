modded class PlayerBase
{
    protected bool m_GRZKillfeedReported;
    protected int m_GRZKillfeedLastMenu;
    protected int m_GRZKillfeedLastSave;

    override void EEKilled(Object killer)
    {
        // Capture identity/source before vanilla death cleanup.
        if (g_Game.IsServer() && !m_GRZKillfeedReported)
        {
            m_GRZKillfeedReported = true;
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
        if (rpc_type == GRZKillfeedRPC.KILL || rpc_type == GRZKillfeedRPC.MENU_STATE)
        {
            // Server messages only; clients cannot inject killfeed events.
            if (g_Game.IsServer() || sender || this != g_Game.GetPlayer())
                return;
            if (rpc_type == GRZKillfeedRPC.KILL)
            {
                GRZKillfeedMessage message = new GRZKillfeedMessage;
                if (!ctx.Read(message.Killer) || !ctx.Read(message.Victim) || !ctx.Read(message.WeaponName) || !ctx.Read(message.Distance))
                    return;
                if (!GRZReadStyle(ctx, message.KillerStyle) || !GRZReadStyle(ctx, message.VictimStyle))
                    return;
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

    protected bool GRZReadStyle(ParamsReadContext ctx, GRZNameStyle style)
    {
        return ctx.Read(style.ColorA) && ctx.Read(style.ColorB) && ctx.Read(style.AnimationMode) && ctx.Read(style.Speed) && style.IsValid();
    }
}

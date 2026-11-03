// State only: clients cannot submit a speed, angle, position or velocity.
class GRZAutorunProtocol
{
    static const int RPC_STATE = 1789346210;
    static const int HEARTBEAT_MS = 1000;
    static const int LEASE_MS = 3500;
}

modded class PlayerBase
{
    private bool m_GRZAutorunServerActive;
    private int m_GRZAutorunLastReceipt;

    private void GRZReleaseServerAutorun()
    {
        if (!m_GRZAutorunServerActive) return;
        m_GRZAutorunServerActive = false;
        HumanInputController controls = GetInputController();
        if (controls)
        {
            controls.OverrideMovementSpeed(HumanInputControllerOverrideType.DISABLED, 0.0);
            controls.OverrideMovementAngle(HumanInputControllerOverrideType.DISABLED, 0.0);
        }
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type != GRZAutorunProtocol.RPC_STATE)
        {
            super.OnRPC(sender, rpc_type, ctx);
            return;
        }
        if (!GetGame().IsServer()) return;
        PlayerIdentity owner = GetIdentity();
        if (!sender || !owner || sender.GetId() != owner.GetId()) return;
        Param1<bool> request;
        if (!ctx.Read(request)) return;
        if (!request.param1)
        {
            GRZReleaseServerAutorun();
            return;
        }
        if (!IsAlive() || IsUnconscious() || IsRestrained() || IsRaised() || GetCurrentCommandID() != DayZPlayerConstants.COMMANDID_MOVE)
        {
            GRZReleaseServerAutorun();
            return;
        }
        HumanInputController controls = GetInputController();
        if (!controls) return;
        m_GRZAutorunServerActive = true;
        m_GRZAutorunLastReceipt = GetGame().GetTime();
        controls.OverrideMovementSpeed(HumanInputControllerOverrideType.ENABLED, 3.0);
        controls.OverrideMovementAngle(HumanInputControllerOverrideType.ENABLED, 0.0);
    }

    override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
    {
        // Only clean up an existing server lease; never inject per-tick motion.
        if (GetGame().IsServer() && m_GRZAutorunServerActive)
        {
            if (GetGame().GetTime() - m_GRZAutorunLastReceipt > GRZAutorunProtocol.LEASE_MS || !IsAlive() || IsUnconscious() || IsRestrained() || IsRaised() || pCurrentCommandID != DayZPlayerConstants.COMMANDID_MOVE)
                GRZReleaseServerAutorun();
        }
        super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
    }
}

// Client camera/UI only. Dedicated servers use their native implementations.
#ifndef SERVER
// Shared helper type: do not call a method added by a later modded-class layer.
class GRZHeldZoomCameras
{
    static bool Supports(string camera)
    {
        return camera == "DayZPlayerCamera3rdPersonErc" || camera == "DayZPlayerCamera3rdPersonErcSpr" || camera == "DayZPlayerCamera3rdPersonErcRaised" || camera == "DayZPlayerCamera3rdPersonErcRaisedMelee" || camera == "DayZPlayerCamera3rdPersonCrouch" || camera == "DayZPlayerCamera3rdPersonCrouchRaised" || camera == "DayZPlayerCamera3rdPersonProne" || camera == "DayZPlayerCamera3rdPersonProneRaised" || camera == "DayZPlayerCamera3rdPersonJump" || camera == "DayZPlayerCamera3rdPersonClimb";
    }

}

class GRZLuguCompatibility
{
    private static bool s_Checked;
    private static bool s_Loaded;

    static bool IsLoaded()
    {
        if (!s_Checked && GetGame())
        {
            s_Loaded = GetGame().ConfigIsExisting("CfgPatches LUGUINSTADROPFIX");
            s_Checked = true;
        }
        return s_Loaded;
    }
}

modded class DayZPlayerImplement
{
    bool GRZIsHoldingEyeZoom()
    {
        if (!GetGame() || GetGame().IsDedicatedServer() || !IsAlive() || GetCommand_Vehicle())
            return false;
        PlayerBase player = PlayerBase.Cast(this);
        if (player && player.IsUnconscious())
            return false;
        HumanInputController controls = GetInputController();
        return controls && controls.IsZoom() && !controls.WeaponADS() && !m_CameraOptics && !m_CameraIronsight;
    }

    private float m_GRZProneDistance = -1;
    private float m_GRZHeldZoomDistance = -1;

    void GRZReleaseHeldZoomDistance()
    {
        m_GRZHeldZoomDistance = -1;
    }

    float GRZGetHeldZoomDistance(float nativeDistance)
    {
        if (m_GRZHeldZoomDistance < 0)
            m_GRZHeldZoomDistance = nativeDistance;
        return m_GRZHeldZoomDistance;
    }

    float GRZMovementCameraDistance(float nativeDistance, bool proneCamera)
    {
        // LUGU owns the prone/recovery graph. Do not carry GRZ's stored prone
        // boom distance into its standing recovery or subsequent standing view.
        if (GRZLuguCompatibility.IsLoaded())
        {
            m_GRZProneDistance = -1;
            return nativeDistance;
        }
        HumanMovementState state = new HumanMovementState();
        GetMovementState(state);
        bool prone = state.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_PRONE || state.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_RAISEDPRONE;
        bool standing = state.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_ERECT || state.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_RAISEDERECT;
        if (GetCurrentCommandID() != DayZPlayerConstants.COMMANDID_MOVE)
        {
            m_GRZProneDistance = -1;
            return nativeDistance;
        }
        if (prone)
        {
            if (proneCamera)
                m_GRZProneDistance = nativeDistance;
            return nativeDistance;
        }
        if (!standing)
        {
            m_GRZProneDistance = -1;
            return nativeDistance;
        }
        // Keep the prone-distance floor throughout the standing recovery.
        // Do not retain the prone pivot, which could place the camera in terrain.
        if (!proneCamera && m_GRZProneDistance > 0)
            return Math.Max(nativeDistance, m_GRZProneDistance);
        return nativeDistance;
    }
}

modded class DayZPlayerCamera3rdPerson
{
    override void StdFovUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
    {
        if (!m_pPlayer || !m_pInput || !pOutResult)
            return;
        super.StdFovUpdate(pDt, pOutResult);
        DayZPlayerImplement player = DayZPlayerImplement.Cast(m_pPlayer);
        if (!player)
            return;
        string camera = ClassName();
        if (GRZHeldZoomCameras.Supports(camera) && player.GRZIsHoldingEyeZoom())
        {
            // Hold only the boom length. Native stance pivots, animation,
            // mouse orientation and engine collision remain in control.
            pOutResult.m_fDistance = player.GRZGetHeldZoomDistance(pOutResult.m_fDistance);
            return;
        }
        player.GRZReleaseHeldZoomDistance();
        // Observe prone for the recovery transition, without changing its result.
        if (camera == "DayZPlayerCamera3rdPersonProne" || camera == "DayZPlayerCamera3rdPersonProneRaised")
        {
            player.GRZMovementCameraDistance(pOutResult.m_fDistance, true);
            return;
        }
        if (camera != "DayZPlayerCamera3rdPersonErcSpr" && camera != "DayZPlayerCamera3rdPersonErc" && camera != "DayZPlayerCamera3rdPersonErcRaised" && camera != "DayZPlayerCamera3rdPersonErcRaisedMelee" && camera != "DayZPlayerCamera3rdPersonCrouch" && camera != "DayZPlayerCamera3rdPersonCrouchRaised")
            return;
        pOutResult.m_fDistance = player.GRZMovementCameraDistance(pOutResult.m_fDistance, false);
    }
}

#endif

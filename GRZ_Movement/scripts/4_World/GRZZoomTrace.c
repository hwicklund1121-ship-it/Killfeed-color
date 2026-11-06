// Client camera/UI only. Dedicated servers use their native implementations.
#ifndef SERVER
// Bounded diagnostics only: no camera or input writes.
modded class DayZPlayerCamera3rdPerson
{
    private float m_GRZZoomTraceElapsed;
    private int m_GRZZoomTraceSamples;

    override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
    {
        super.OnUpdate(pDt, pOutResult);
        if (!m_pPlayer || !m_pInput || m_pPlayer.GetEyeZoomLevel() == ECameraZoomType.NONE)
            return;
        if (m_GRZZoomTraceSamples >= 120)
            return;
        m_GRZZoomTraceElapsed += pDt;
        if (m_GRZZoomTraceElapsed < 0.1)
            return;
        m_GRZZoomTraceElapsed = 0;
        m_GRZZoomTraceSamples++;
        HumanMovementState state = new HumanMovementState();
        m_pPlayer.GetMovementState(state);
        float speed;
        vector direction;
        m_pInput.GetMovement(speed, direction);
        Print("[GRZZOOM] version=0.14 t=" + GetGame().GetTime().ToString() + " camera=" + ClassName() + " eye=" + m_pPlayer.GetEyeZoomLevel().ToString() + " fov=" + pOutResult.m_fFovAbsolute.ToString() + " multiplier=" + pOutResult.m_fFovMultiplier.ToString() + " distance=" + pOutResult.m_fDistance.ToString() + " pivot=" + pOutResult.m_CameraTM[3].ToString() + " stance=" + state.m_iStanceIdx.ToString() + " movement=" + state.m_iMovement.ToString() + " requested_speed=" + speed.ToString());
    }
}

#endif

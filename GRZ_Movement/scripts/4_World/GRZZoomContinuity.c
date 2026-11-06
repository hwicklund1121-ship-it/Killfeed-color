// Client camera/UI only. Dedicated servers use their native implementations.
#ifndef SERVER
// Carry eye-zoom FOV through on-foot standing/crouch/prone/jump/climb cameras.
// Keep native FOV smoothing; the separate distance policy holds only the boom.
modded class DayZPlayerCamera3rdPerson
{
    override void OnActivate(DayZPlayerCamera pPrevCamera, DayZPlayerCameraResult pPrevCameraResult)
    {
        if (!m_pPlayer || !m_pInput)
            return;
        super.OnActivate(pPrevCamera, pPrevCameraResult);
        if (!pPrevCamera || !pPrevCameraResult || !m_pPlayer || !m_pInput)
            return;
        if (!GRZHeldZoomCameras.Supports(ClassName()) || !GRZHeldZoomCameras.Supports(pPrevCamera.ClassName()))
            return;
        DayZPlayerImplement player = DayZPlayerImplement.Cast(m_pPlayer);
        bool heldZoom = player && player.GRZIsHoldingEyeZoom();
        if (m_pInput.WeaponADS() || m_pPlayer.GetEyeZoomLevel() == ECameraZoomType.NONE)
            return;
        if (!heldZoom && m_pPlayer.GetCurrentCommandID() != DayZPlayerConstants.COMMANDID_MOVE)
            return;
        if (pPrevCameraResult.m_fFovAbsolute > 0)
        {
            m_fFovAbsolute = pPrevCameraResult.m_fFovAbsolute;
            m_fFovAbsVel[0] = 0;
        }
    }
}

#endif

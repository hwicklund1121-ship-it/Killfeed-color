// Client camera/UI only. Dedicated servers use their native implementations.
#ifndef SERVER
// Remove locomotion-driven third-person camera motion without changing FOV,
// camera selection, lean inputs, or LUGU's prone camera path.
modded class DayZPlayerCamera3rdPersonErcSpr
{
    override float GetCurrentRoll()
    {
        return 0.0;
    }
}

modded class DayZPlayerCamera3rdPersonCrouch
{
    override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
    {
        if (!m_pPlayer || !m_pInput || !pOutResult)
            return;
        super.OnUpdate(pDt, pOutResult);

        // The native crouch camera adds m_fCameraHeight in model-space Y.
        // Subtract only that contribution, retaining stance height and lean.
        pOutResult.m_CameraTM[3][1] = pOutResult.m_CameraTM[3][1] - m_fCameraHeight;
    }
}

#endif

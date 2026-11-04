// Custom quickscope behavior, not a binary rollback of DayZ 1.29.
modded class DayZPlayerCameras
{
    override static float OnCameraChange(DayZPlayer pPlayer, int pFrom, int pTo)
    {
        float nativeTime = super.OnCameraChange(pPlayer, pFrom, pTo);
        if (!pPlayer || !Weapon_Base.Cast(pPlayer.GetEntityInHands()))
            return nativeTime;
        bool aimingFrom = pFrom == DAYZCAMERA_IRONSIGHTS || pFrom == DAYZCAMERA_OPTICS;
        bool aimingTo = pTo == DAYZCAMERA_IRONSIGHTS || pTo == DAYZCAMERA_OPTICS;
        bool normalFrom = pFrom == DAYZCAMERA_1ST || pFrom == DAYZCAMERA_3RD_ERC || pFrom == DAYZCAMERA_3RD_CRO || pFrom == DAYZCAMERA_3RD_PRO || pFrom == DAYZCAMERA_3RD_ERC_RAISED || pFrom == DAYZCAMERA_3RD_CRO_RAISED || pFrom == DAYZCAMERA_3RD_PRO_RAISED;
        bool normalTo = pTo == DAYZCAMERA_1ST || pTo == DAYZCAMERA_3RD_ERC || pTo == DAYZCAMERA_3RD_CRO || pTo == DAYZCAMERA_3RD_PRO || pTo == DAYZCAMERA_3RD_ERC_RAISED || pTo == DAYZCAMERA_3RD_CRO_RAISED || pTo == DAYZCAMERA_3RD_PRO_RAISED;
        if ((aimingTo && (normalFrom || aimingFrom)) || (aimingFrom && normalTo))
            return Math.Min(nativeTime, 0.08);
        return nativeTime;
    }
}

modded class DayZPlayerImplement
{
    override void HandleADS()
    {
        HumanInputController input = GetInputController();
        PlayerBase player = PlayerBase.Cast(this);
        // Complete the extra script raise delay only after native raised state is reached.
        // Super retains sprint, obstruction, inventory, fighting and weapon-manager checks.
        if (player && IsAlive() && !player.IsUnconscious() && input && input.WeaponADS() && IsRaised() && !IsWeaponRaiseCompleted() && !IsLiftWeapon() && !IsWeaponObstructionBlockingADS() && !IsFighting())
        {
            Weapon_Base weapon = Weapon_Base.Cast(GetEntityInHands());
            if (weapon && player.GetWeaponManager() && !player.GetWeaponManager().IsRunning() && !GetDayZPlayerInventory().IsProcessing())
                CompleteWeaponRaise();
        }
        super.HandleADS();
    }
}

modded class DayZPlayerCameraIronsights
{
    override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
    {
        if (m_weaponUsed)
        {
            // Keep recoil in the aiming model; remove additional reticle lag/bob.
            m_dynamicsStrength = 0;
            m_movementAmplitudeX = 0;
            m_movementAmplitudeY = 0;
        }
        super.OnUpdate(pDt, pOutResult);
    }
}

modded class Weapon_Base
{
    override void GRZAdjustHitscanDirection(inout vector direction)
    {
        super.GRZAdjustHitscanDirection(direction);
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (!g_Game.IsServer() || !player || !player.IsAlive() || !player.IsRaised())
            return;
        HumanInputController input = player.GetInputController();
        if (!input || (!input.WeaponADS() && !player.IsInIronsights() && !player.IsInOptics()))
            return;
        HumanCommandWeapons command = player.GetCommandModifier_Weapons();
        if (!command)
            return;
        // Same world-angle construction used by native-script weapon obstruction tests.
        // Keep hand offsets so this does not remove recoil or injury effects.
        vector angles = Vector(command.GetBaseAimingAngleLR() + player.GetOrientation()[0] + command.GetAimingHandsOffsetLR(), Math.Clamp(command.GetBaseAimingAngleUD() + command.GetAimingHandsOffsetUD(), -85, 85), 0);
        direction = angles.AnglesToVector();
        direction.Normalize();
        // Origin stays at the muzzle; existing nearest-surface collision still blocks walls.
    }
}

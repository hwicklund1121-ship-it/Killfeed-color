// Fast scope transitions retain native weapon-ready checks; no forced early ADS.
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

modded class DayZPlayerCameraIronsights
{
    override void OnUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
    {
        if (m_weaponUsed)
        {
            m_dynamicsStrength = 0;
            m_movementAmplitudeX = 0;
            m_movementAmplitudeY = 0;
        }
        super.OnUpdate(pDt, pOutResult);
        if (m_weaponUsed)
            pOutResult.m_fShootFromCamera = 1.0;
    }
}

modded class DayZPlayerCameraBase
{
    override void StdFovUpdate(float pDt, out DayZPlayerCameraResult pOutResult)
    {
        super.StdFovUpdate(pDt, pOutResult);
        if (m_pPlayer && Weapon_Base.Cast(m_pPlayer.GetEntityInHands()))
            pOutResult.m_fShootFromCamera = 1.0;
    }
}

modded class RecoilBase
{
    override vector GetRecoilModifier(Weapon_Base weapon)
    {
        vector modifier = super.GetRecoilModifier(weapon);
        PlayerBase player;
        if (weapon)
            player = PlayerBase.Cast(weapon.GetHierarchyRootPlayer());
        if (!player)
            return modifier;
        HumanInputController input = player.GetInputController();
        if (player.IsInIronsights() || player.IsInOptics() || (input && input.WeaponADS()))
            modifier = modifier * 0.10;
        // Native PostInit snapshots this once per shot, including camera Z kick.
        // Unscoping during recoil cannot restore a full-strength tail mid-shot.
        return modifier;
    }
}

modded class Weapon_Base
{
    override void GRZAdjustHitscanDirection(vector origin, array<Object> excluded, inout vector direction)
    {
        super.GRZAdjustHitscanDirection(origin, excluded, direction);
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (!g_Game.IsServer() || !player || !player.IsAlive() || !player.IsRaised())
            return;
        HumanInputController input = player.GetInputController();
        HumanCommandWeapons command = player.GetCommandModifier_Weapons();
        if (!command)
            return;
        // Same world-angle construction used by native-script weapon obstruction tests.
        // Keep hand offsets so this does not remove recoil or injury effects.
        vector angles = Vector(command.GetBaseAimingAngleLR() + player.GetOrientation()[0] + command.GetAimingHandsOffsetLR(), Math.Clamp(command.GetBaseAimingAngleUD() + command.GetAimingHandsOffsetUD(), -85, 85), 0);
        vector aimDirection = angles.AnglesToVector();
        aimDirection.Normalize();
        // Dedicated servers may have no rendered camera. Use the replicated aim
        // and head position as a fallback; never use the server's global camera.
        int head = player.GetBoneIndexByName("Head");
        vector aimOrigin = origin;
        if (head >= 0)
            aimOrigin = player.GetBonePositionWS(head);
        if (player.GetCurrentCamera() && (!input || !input.CameraIsFreeLook()))
        {
            vector cameraPos, cameraDir, cameraRot;
            player.GetCurrentCameraTransform(cameraPos, cameraDir, cameraRot);
            float cameraLength = cameraDir.Length();
            if (cameraLength > 0.5 && cameraLength < 1.5 && vector.Distance(cameraPos, player.GetPosition()) < 4.0)
            {
                cameraDir.Normalize();
                // Avoid a stale, spectator or free-look camera changing shot aim.
                if (vector.Dot(cameraDir, aimDirection) > 0.95)
                {
                    aimOrigin = cameraPos;
                    aimDirection = cameraDir;
                }
            }
        }
        vector aimPoint = aimOrigin + aimDirection * 1400.0;
        RaycastRVParams aimParams = new RaycastRVParams(aimOrigin, aimPoint, player, 0);
        aimParams.with = this;
        aimParams.type = ObjIntersectFire;
        aimParams.sorted = true;
        aimParams.flags = CollisionFlags.NEARESTCONTACT;
        array<ref RaycastRVResult> aimHits = new array<ref RaycastRVResult>;
        if (DayZPhysics.RaycastRVProxy(aimParams, aimHits, excluded) && aimHits.Count() > 0)
            aimPoint = aimHits[0].pos;
        vector towardPoint = aimPoint - origin;
        if (towardPoint.Length() > 0.01)
        {
            towardPoint.Normalize();
            // A camera contact behind the muzzle must not reverse a shot.
            if (vector.Dot(towardPoint, aimDirection) > 0)
                direction = towardPoint;
            else
                direction = aimDirection;
        }
        // The main hitscan traces muzzle-to-aim-point and can hit closer cover.
    }
}

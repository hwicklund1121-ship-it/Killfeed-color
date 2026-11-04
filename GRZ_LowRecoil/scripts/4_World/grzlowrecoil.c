modded class RecoilBase
{
    override vector GetRecoilModifier(Weapon_Base weapon)
    {
        vector recoilScale = super.GetRecoilModifier(weapon);
        if (!weapon)
            return recoilScale;
        PlayerBase recoilPlayer = PlayerBase.Cast(weapon.GetHierarchyRootPlayer());
        if (!recoilPlayer)
            return recoilScale;
        HumanInputController aimInput = recoilPlayer.GetInputController();
        bool adsShot = recoilPlayer.IsInIronsights() || recoilPlayer.IsInOptics() || (aimInput && aimInput.WeaponADS());
        if (adsShot)
            return recoilScale * 0.10;
        return recoilScale;
    }
}

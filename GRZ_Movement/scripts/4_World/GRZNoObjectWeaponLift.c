// Keep the weapon's normal aiming pose near objects. The native weapon handler
// still performs manual raise/lower, sight selection and weapon command updates.
modded class PlayerBase
{
    override void CheckLiftWeapon()
    {
        if (GetInstanceType() != DayZPlayerInstanceType.INSTANCETYPE_CLIENT)
            return;

        // Clear both obstruction responses through the engine's existing sync
        // helper. It updates local state and sends changed state to the server.
        if (m_LiftWeapon_player || m_ObstructWeapon_player > 0.0)
            SendLiftWeaponSync(false, 0.0);
    }
}

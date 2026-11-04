modded class PlayerBase
{
    override void SetBrokenLegs(int stateId)
    {
        // Keep vanilla synchronization and clear the fracture animation mask.
        super.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
    }

    override void ProcessFeetDamageServer(int pUserInt)
    {
        // Suppress the native barefoot/ruined-shoe walking cut roll and step
        // damage to footwear. Other damage and bleeding paths stay native.
    }
}

modded class BrokenLegsMdfr
{
    override void OnActivate(PlayerBase player)
    {
        player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
        player.GetNotifiersManager().DeactivateByType(eNotifiers.NTF_FRACTURE);
    }

    override void OnReconnect(PlayerBase player)
    {
        // Clear a fracture loaded from an existing character save too.
        player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
        player.GetNotifiersManager().DeactivateByType(eNotifiers.NTF_FRACTURE);
    }

    override bool DeactivateCondition(PlayerBase player)
    {
        return true;
    }
}

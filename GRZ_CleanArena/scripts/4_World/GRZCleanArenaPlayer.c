modded class PlayerBase
{
    protected EntityAI m_GRZDeathHeldItem;

    override bool CanDropEntity(notnull EntityAI item)
    {
        // Native inventory drop, drag-to-vicinity and throw actions consult this.
        // Do not lock cargo or attachments: server loadout transactions need them.
        return false;
    }

    override void EEKilled(Object killer)
    {
        if (g_Game.IsServer())
            m_GRZDeathHeldItem = GetHumanInventory().GetEntityInHands();
        super.EEKilled(killer);
        if (g_Game.IsServer() && GetIdentity())
            g_Game.RPCSingleParam(this, 763510, new Param1<bool>(true), true, GetIdentity());
        if (!g_Game.IsDedicatedServer() && IsPlayerSelected())
            ShowDeadScreen(true, 0);
        // Leave the complete damage/killfeed/hitmarker callback chain first.
        if (g_Game.IsServer())
            g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GRZCleanDeadPlayer, 250, false);
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);
        if (rpc_type == 763510 && !g_Game.IsServer() && IsPlayerSelected())
            ShowDeadScreen(true, 0);
    }

    void GRZCleanDeadPlayer()
    {
        if (!g_Game.IsServer() || IsAlive())
            return;
        // Vanilla death can detach the held weapon. Remove it too, unless
        // another living player has already acquired it.
        if (m_GRZDeathHeldItem)
        {
            PlayerBase holder = PlayerBase.Cast(m_GRZDeathHeldItem.GetHierarchyRootPlayer());
            if (!holder || holder == this || !holder.IsAlive())
                g_Game.ObjectDelete(m_GRZDeathHeldItem);
            m_GRZDeathHeldItem = null;
        }
        array<EntityAI> contents = new array<EntityAI>;
        GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, contents);
        for (int i = contents.Count() - 1; i >= 0; i--)
        {
            EntityAI item = contents[i];
            if (item && item != this)
                g_Game.ObjectDelete(item);
        }
        g_Game.ObjectDelete(this);
    }

    override void EEDelete(EntityAI parent)
    {
        if (g_Game)
        {
            // Preserve the death UI even after the controlled corpse disappears.
            if (!g_Game.IsDedicatedServer() && !IsAlive() && IsPlayerSelected())
                ShowDeadScreen(true, 0);
            g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(GRZCleanDeadPlayer);
        }
        super.EEDelete(parent);
    }
};

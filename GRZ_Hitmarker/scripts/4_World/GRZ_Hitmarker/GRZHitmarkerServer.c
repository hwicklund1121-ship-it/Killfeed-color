modded class EntityAI
{
    protected float m_GRZHitmarkerHealthBefore;
    protected bool m_GRZHitmarkerEligible;

    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        bool allowed = super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (g_Game.IsServer())
        {
            m_GRZHitmarkerEligible = allowed && IsAlive() && GRZHitmarkerServer.IsLivingTarget(this);
            if (m_GRZHitmarkerEligible)
                m_GRZHitmarkerHealthBefore = GetHealth("", "Health");
        }
        return allowed;
    }

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        if (g_Game.IsServer() && m_GRZHitmarkerEligible)
        {
            m_GRZHitmarkerEligible = false;
            GRZHitmarkerServer.Report(this, source, damageResult, dmgZone, m_GRZHitmarkerHealthBefore);
        }
    }
}

class GRZHitmarkerServer
{
    static bool IsLivingTarget(EntityAI target)
    {
        return target && (target.IsInherited(PlayerBase) || target.IsInherited(ZombieBase) || target.IsInherited(AnimalBase));
    }

    static void Report(EntityAI target, EntityAI source, TotalDamageResult result, string zone, float beforeHealth)
    {
        if (!g_Game.IsServer() || !target || !source || !result)
            return;
        PlayerBase attacker = PlayerBase.Cast(source);
        if (!attacker)
            attacker = PlayerBase.Cast(source.GetHierarchyRootPlayer());
        if (!attacker || attacker == target)
            return;
        float damage = Math.Max(0, beforeHealth - target.GetHealth("", "Health"));
        float reported = result.GetDamage(zone, "Health");
        if (damage <= 0 && reported <= 0 && result.GetDamage(zone, "Shock") <= 0)
            return;
        // Global health loss is displayed, not the ammo's nominal damage.
        string lowered = zone;
        lowered.ToLower();
        bool head = lowered.IndexOf("head") >= 0 || lowered.IndexOf("brain") >= 0;
        bool killed = !target.IsAlive() || target.GetHealth("", "Health") <= 0;
        if (!g_Game.IsMultiplayer())
        {
            if (attacker == g_Game.GetPlayer())
            {
                GRZHitmarkerEvent localHit = new GRZHitmarkerEvent;
                localHit.DamageAmount = damage;
                localHit.IsHeadHit = head;
                localHit.IsKill = killed;
                GRZHitmarkerBus.Hits.Invoke(localHit);
            }
            return;
        }
        if (!attacker.GetIdentity())
            return;
        ScriptRPC rpc = new ScriptRPC;
        rpc.Write(damage);
        rpc.Write(head);
        rpc.Write(killed);
        rpc.Send(attacker, GRZHitmarkerBus.HIT_RPC, true, attacker.GetIdentity());
    }
}

modded class PlayerBase
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type == GRZHitmarkerBus.HIT_RPC)
        {
            if (g_Game.IsServer() || sender || this != PlayerBase.Cast(g_Game.GetPlayer()))
                return;
            GRZHitmarkerEvent hit = new GRZHitmarkerEvent;
            if (ctx.Read(hit.DamageAmount) && ctx.Read(hit.IsHeadHit) && ctx.Read(hit.IsKill) && hit.DamageAmount >= 0 && hit.DamageAmount <= 100000)
                GRZHitmarkerBus.Hits.Invoke(hit);
            return;
        }
        super.OnRPC(sender, rpc_type, ctx);
    }
}

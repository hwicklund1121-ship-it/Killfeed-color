// Scripted target classes support modding; the engine Object class does not.
class GRZHitscanTargets
{
    static bool Supports(Object target)
    {
        return PlayerBase.Cast(target) != null || ZombieBase.Cast(target) != null || AnimalBase.Cast(target) != null || ItemBase.Cast(target) != null || BuildingBase.Cast(target) != null || CarScript.Cast(target) != null || BoatScript.Cast(target) != null;
    }
}

modded class PlayerBase
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

modded class ZombieBase
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

modded class AnimalBase
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

modded class ItemBase
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

modded class BuildingBase
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

modded class CarScript
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

modded class BoatScript
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

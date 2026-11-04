// Shared by the damage filter and firing hook. No later-module classes here.
class GRZHitscanPolicy
{
    static EntityAI DirectSource;
    static string DirectAmmo;
    static int DirectDepth;

    static bool SupportsAmmo(string ammo)
    {
        string path = "CfgAmmo " + ammo;
        if (ammo == "" || !g_Game.ConfigIsExisting(path))
            return false;
        string simulation;
        g_Game.ConfigGetText(path + " simulation", simulation);
        simulation.ToLower();
        if (simulation != "shotbullet")
            return false;
        // Preserve explosives and multi-pellet rounds instead of turning them into one ray.
        if (g_Game.ConfigGetFloat(path + " indirectHit") > 0 || g_Game.ConfigGetFloat(path + " explosive") > 0)
            return false;
        if (g_Game.ConfigGetInt(path + " projectilesCount") > 1)
            return false;
        return true;
    }

    static bool GetRay(EntityAI source, out vector origin, out vector direction)
    {
        if (!source || !source.IsWeapon())
            return false;
        string path = "CfgWeapons " + source.GetType();
        array<string> muzzles = new array<string>;
        g_Game.ConfigGetTextArray(path + " muzzles", muzzles);
        // This first build supports root-configured, single-muzzle firearms.
        if (muzzles.Count() > 1)
            return false;
        if (muzzles.Count() == 1 && muzzles[0] != "this")
            return false;
        string front, rear;
        g_Game.ConfigGetText(path + " muzzlePos", front);
        g_Game.ConfigGetText(path + " muzzleEnd", rear);
        if (front == "" || rear == "" || !source.MemoryPointExists(front) || !source.MemoryPointExists(rear))
            return false;
        origin = source.ModelToWorld(source.GetMemoryPointPos(front));
        vector back = source.ModelToWorld(source.GetMemoryPointPos(rear));
        direction = origin - back;
        if (direction.Length() < 0.001)
            return false;
        direction.Normalize();
        return true;
    }

    static bool RejectNative(int damageType, EntityAI source, string ammo)
    {
        if (!g_Game || !g_Game.IsServer() || damageType != DamageType.FIRE_ARM)
            return false;
        if (DirectDepth > 0 && DirectSource == source && DirectAmmo == ammo)
            return false;
        if (!SupportsAmmo(ammo))
            return false;
        vector origin, direction;
        return GetRay(source, origin, direction);
    }
}

// Native bullet damage must be rejected even for a shot whose ray misses.
// Filtering only victims hit by the ray would allow a second, delayed hit.
modded class Object
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

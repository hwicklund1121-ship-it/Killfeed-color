modded class Weapon_Base
{
    protected bool m_GRZHitscanReported;

    override void EEFired(int muzzleType, int mode, string ammoType)
    {
        super.EEFired(muzzleType, mode, ammoType);
        if (!g_Game.IsServer() || !GRZHitscanPolicy.SupportsAmmo(ammoType))
            return;
        vector origin, direction;
        if (!GRZHitscanPolicy.GetRay(this, origin, direction))
            return;
        if (!m_GRZHitscanReported)
        {
            m_GRZHitscanReported = true;
            Print("[GRZ Hitscan TEST 0.1.0] Server muzzle ray active: " + GetType() + " / " + ammoType);
        }

        // The ray begins at the muzzle, not the camera, so nearby cover blocks it.
        RaycastRVParams params = new RaycastRVParams(origin, origin + direction * 1400.0, this, 0);
        params.type = ObjIntersectFire;
        params.sorted = true;
        params.flags = CollisionFlags.NEARESTCONTACT;
        array<Object> excluded = new array<Object>;
        excluded.Insert(this);
        Man shooter = GetHierarchyRootPlayer();
        if (shooter)
        {
            params.with = shooter;
            excluded.Insert(shooter);
            array<EntityAI> carried = new array<EntityAI>;
            shooter.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried);
            foreach (EntityAI item : carried)
                excluded.Insert(item);
        }
        array<ref RaycastRVResult> results = new array<ref RaycastRVResult>;
        if (!DayZPhysics.RaycastRVProxy(params, results, excluded) || results.Count() == 0)
            return;
        RaycastRVResult hit = results[0];
        Object target = hit.obj;
        // Terrain and non-entity model proxies stop the ray without a damage call.
        if (!target || target == this || target == shooter)
            return;
        string zone = target.GetDamageZoneNameByComponentIndex(hit.component);
        EntityAI savedSource = GRZHitscanPolicy.DirectSource;
        string savedAmmo = GRZHitscanPolicy.DirectAmmo;
        GRZHitscanPolicy.DirectSource = this;
        GRZHitscanPolicy.DirectAmmo = ammoType;
        GRZHitscanPolicy.DirectDepth++;
        target.ProcessDirectDamage(DamageType.FIRE_ARM, this, zone, ammoType, target.WorldToModel(hit.pos), 1.0, ProcessDirectDamageFlags.ALL_TRANSFER);
        GRZHitscanPolicy.DirectDepth--;
        GRZHitscanPolicy.DirectSource = savedSource;
        GRZHitscanPolicy.DirectAmmo = savedAmmo;
    }
}

// Vanilla plastic explosives return true without calling the Object hook.
modded class Plastic_Explosive
{
    override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GRZHitscanPolicy.RejectNative(damageType, source, ammo))
            return false;
        return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
}

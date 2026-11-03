#ifndef SERVER
// Stabilize the native stamina indicator during autorun without changing its
// value/range or overriding the parent HUD's visibility/user settings.
modded class IngameHud
{
    override void SetStaminaBarVisibility(bool show)
    {
        if (GRZAutorunState.Active)
            show = true;
        super.SetStaminaBarVisibility(show);
    }
}
#endif

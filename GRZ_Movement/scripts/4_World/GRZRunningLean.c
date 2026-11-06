// The native sprint-start callback locks both lean inputs. Keep them available
// by omitting that callback's input locks; movement-change and sprint-end
// processing continue through their native implementations.
modded class DayZPlayerImplement
{
    override void OnSprintStart()
    {
        // GRZ permits the player's existing lean bindings during sprint.
    }
}

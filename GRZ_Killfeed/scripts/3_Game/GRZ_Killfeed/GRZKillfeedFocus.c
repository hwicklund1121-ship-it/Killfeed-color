modded class DayZGame
{
    override void OnDeactivateMessage()
    {
        // Release this menu's focus before vanilla refreshes cursor visibility.
        // This event still fires when background GUI updates are paused.
        GRZKillfeedBus.AppDeactivated.Invoke();
        super.OnDeactivateMessage();
    }
}

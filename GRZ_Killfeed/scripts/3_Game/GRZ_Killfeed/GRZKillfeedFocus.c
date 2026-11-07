modded class DayZGame
{
    override void OnDeactivateMessage()
    {
        // Preserve menu state; native focus/cursor handling still runs normally.
        GRZKillfeedBus.AppDeactivated.Invoke();
        super.OnDeactivateMessage();
    }
    override void OnActivateMessage()
    {
        super.OnActivateMessage();
        GRZKillfeedBus.AppActivated.Invoke();
    }
}

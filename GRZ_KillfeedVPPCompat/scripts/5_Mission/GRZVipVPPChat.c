modded class ChatInputMenu
{
    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        // This optional addon depends on both GRZ and VPP, making it the
        // outer handler. Other slash commands continue into VPP unchanged.
        if (GRZTryHandleVipCommand(finished))
            return true;
        return super.OnChange(w, x, y, finished);
    }
}

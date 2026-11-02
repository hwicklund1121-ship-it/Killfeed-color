modded class MissionBase
{
    override UIScriptedMenu CreateScriptedMenu(int id)
    {
        if (id == GRZKillfeedMenu.MENU_ID)
        {
            GRZKillfeedMenu menu = new GRZKillfeedMenu;
            menu.SetID(id);
            return menu;
        }
        return super.CreateScriptedMenu(id);
    }
}

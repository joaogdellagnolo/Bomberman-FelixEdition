static Routes currentRoute = MENU;
static Widget* currentWidget;

enum Routes 
{
    MENU,
    BOMBERMAN,
    GAME_INFO,
    EXIT,
};

void setRoute(Routes route) {
    switch (route)
    {
    case MENU:
        currentWidget = Menu::Instance();
        break;
    case BOMBERMAN:
        currentWidget = BomberMan::Instance();
        break; 
    default:
        break;
    };
}
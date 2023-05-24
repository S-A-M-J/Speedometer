struct MENUITEM
{
  char *segment1;                // item title
  uint32_t *segment2;
  bool (*action)(int *ptr);  // function to execute
  int *param;                // parameter for function to execute
  struct MENU *subMenu;       // submenu for this menu item
};

struct MENU
{
  char *title;              // menu title
  MENUITEM *items;        // menu items
  int numItems;             // number of menu items
  int selected;             // item that is selected
  struct MENU *parentMenu;  // parent menu of this menu
};

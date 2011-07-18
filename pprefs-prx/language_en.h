//

//COMMON 

#define PB_SYM_PSP_UP			"\x80"
#define PB_SYM_PSP_RIGHT		"\x81"
#define PB_SYM_PSP_DOWN			"\x82"
#define PB_SYM_PSP_LEFT			"\x83"
#define PB_SYM_PSP_TRIANGLE		"\x84"
#define PB_SYM_PSP_CIRCLE		"\x85"
#define PB_SYM_PSP_CROSS		"\x86"
#define PB_SYM_PSP_SQUARE		"\x87"
#define PB_SYM_PSP_NOTE			"\x88"

#define PPREFSMSG_MAINMENU_HOTOUSE "%s:Sel. hold%s:AllOFF/ON \x84:Menu \x87+\x80/\x82:Move L+R:ChangeDir"
#define PPREFSMSG_MAINMENU_HOTOUSE_2 "SELECT:reload HOME:Save&Quit START:Save&RESTARTVSH"
#define PPREFSMSG_MAINMENU_REPUSHSTART "re-press START\n,to RESTART VSH"
#define PPREFSMSG_MAINMENU_RELOAD " Reload(withoutSaving)  Really? "

#define PPREFSMSG_YESORNO "%s:YES %s:NO"
#define PPREFSMSG_YESORNO_LIST "YES","NO",NULL

#define PPREFSMSG_ADD_TOP "<<add>> select plugin you want to add "
#define PPREFSMSG_ADD_HOWTOUSE " %s:Sel. HOME:Quit L:UpDir R:OpenDir "
#define PPREFSMSG_ADD_HOWTOUSE_2 " %s:Sel. HOME:Quit L:UpDir R:OpenDir START:SelectCurrentDir "
#define PPREFSMSG_ADD_HOWTOUSE_3 " HOME:Quit L:UpDir R/%s:OpenDir START:SelectCurrentDir"

#define PPREFSMSG_SELECTBOX_HOTOUSE " %s:Sel. "
#define PPREFSMSG_SELECTBOX_HOTOUSE_2 " %s:Sel. HOME:Quit "

#define PPREFSMSG_COPYME_FIRSTCONFIRM "Save the edit?"
#define PPREFSMSG_COPYME_FIRSTCONFIRMHOTOUSE "START:Save & Go to next SELECT:Not Save & Go to next\nHOME:Quit"
#define PPREFSMSG_COPYME_SELECTLIST "Save & Go to next","Not Save & Go to next","Quit",NULL
#define PPREFSMSG_COPYME_INSERT2NDMS "Insert another MS"
#define PPREFSMSG_COPYME_INSERT2NDMSHOWTOUSE "%s:Ok,Done HOME:Quit"
#define PPREFSMSG_COPYME_DONETOCOPY "Done!"
#define PPREFSMSG_COPYME_FAILTOCOPY "Fail to copy"
#define PPREFSMSG_COPYME_ERROR "error!"
#define PPREFSMSG_COPYME_INSERTERROR "insert MS that has 'pprefs_lite'"
#define PPREFSMSG_FAILTOWRITE "Fail to write %s"
#define PPREFSMSG_FAILTOWRITEHOWTOUSE "%s:Retry %s:Skip "

#define PPREFSMSG_CONFIGMENU_TITLE " Config "
#define PPREFSMSG_DETECTKEY_1 "Detect Key   "
#define PPREFSMSG_DETECTKEY_2 "Detect Key %d"
#define PPREFSMSG_CONFIGMENU_MENU_1 "Save this config"
#define PPREFSMSG_CONFIGMENU_MENU_2 "Set default"
#define PPREFSMSG_CONFIGMENU_MENU_3 "Quit"
#define PPREFSMSG_CONFIGMENU_HOWTOUSE " %s:Sel. HOME:Quit "
#define PPREFSMSG_CONFIG_BASEPATH_LIST "Set","not Set",NULL
#define PPREFSMSG_CONFIG_BASEPATH_TITLE "BasePath"
#define PPREFSMSG_SELECTBASEPATH "select the folder that has vsh.txt,game.txt,pops.txt"
#define PPREFSMSG_CONFIG_SORTTYPE_ISOCSO "include *.iso/*.cso files ?"
#define PPREFSMSG_CONFIG_SORTTYPE_GAME150 "include GAME150 folder?"
#define PPREFSMSG_CONFIG_SORTTYPE_GAME5XX "include GAME5XX folder?"
#define PPREFSMSG_CONFIG_SORTTYPE_TYPE "Which plugins are you using?"
#define PPREFSMSG_CONFIG_SORTTYPE_TYPE_LIST "not using","gamecategories","gamecategories light(FW6.20)","gamecategories light(FW6.3x)",NULL
#define PPREFSMSG_CONFIG_SORTTYPE_NOTDISPLAY_ICON0 "Display icon?"

#define PPREFSMSG_THEOTHER "the other"

#define PPREFSMSG_BACKUPMENU {"Load","Save",NULL}
#define PPREFSMSG_BACKUPMENU_HOWOTOUSE "%s:Sel. HOME:Quit [Rtri]/[Ltri]:ChangeTextType"
#define PPREFSMSG_BACKUPMENU_NOTEXITST "The file doesn't exist"


#define PPREFSMSG_HITOBASHIRA "\x89\x8a"
#define PPREFSMSG_HITOBASHIRA_2 "\x89\x8a\2"

/*--------------------------------------*/
#ifdef PPREFS_LITE
/*--------------------------------------*/

#define PPREFSMSG_SUBMENU_TITLE " MENU "
#define PPREFSMSG_SUBMENU_LIST "add","remove","backup","config","COPY ME",NULL
#define PPREFSMSG_SUBMENU_LIST_GO "add","remove","backup","config",NULL

#define PPREFSMSG_CONFIGMENU_WHATISTHIS \
"Hot key to start this plugin", \
"Swap X and O", \
"to RESTART VSH on start menu\ntrue -> press START once  false -> press START twice", \
"Line Feed Code", \
"Folder that has vsh.txt , game.txt , pops.txt", \
"Foreground color", \
"Background color", \
"Selected color", \
"Color of explanation on bottom of screen", \
"[ON] color", \
"[OFF] color", \
"if it boots with holding this buttons,\n disable others plugins"


/*--------------------------------------*/
#else
/*--------------------------------------*/

#define PPREFSMSG_BOOTMESSAGE " pprefs \x8b\x8c\x8d\x8e\x8f\x90!  / StartButton:"

#define PPREFSMSG_SUBMENU_TITLE "menu"
#define PPREFSMSG_SUBMENU_LIST					"add","remove","backup","SORT GAME","config","edit pergame","COPY ME",NULL
#define PPREFSMSG_SUBMENU_LIST_HITOBASHIRA		"add","remove","backup","SORT GAME","config","edit pergame","COPY ME","remove FILE",NULL
#define PPREFSMSG_SUBMENU_LIST_GO				"add","remove","backup","SORT GAME","config","edit pergame",NULL
#define PPREFSMSG_SUBMENU_LIST_GO_HITOBASHIRA	"add","remove","backup","SORT GAME","config","edit pergame","edit pause game","remove FILE",NULL

#define PPREFSMSG_EDITPERGAME_SELECTPRX "select plugin"
#define PPREFSMSG_EDITPERGAME_SELECTGAME "select game"
#define PPREFSMSG_EDITPERGAME_HOTOUSE "HOME:quit START:save hold%s:removeLine LTri:Top RTri:Bottom"
#define PPREFSMSG_EDITPERGAME_HOTOUSE_2 "\x81:swap commentOut(#) \x83:swap blackList(!) \x87+\x80/\x82:move"
#define PPREFSMSG_EDITPERGAME_TOP " edit pergame "

#define PPREFSMSG_PAUSEGAMETEST "RESTART VSH after editing"
#define PPREFSMSG_CONFIGMENU_WHATISTHIS \
"Hot key to start this plugin", \
"About the message displayed on bottom-left of screen when \nthe device booted   true->display false->hide", \
"Swap X / O", \
"to RESTART VSH on start menu\ntrue -> press START once  false -> press START twice", \
"Line Feed Code", \
"Folder that has vsh.txt , game.txt , pops.txt", \
"Foreground color", \
"Background color", \
"Selected color", \
"Color of explanation on bottom of screen",  \
"[ON] color", \
"[OFF] color", \
"true->enable false->disable *thisWorksOnOnly5.xx\nThisIs\x91,soItHasSomeIssues.IfSwapEn/Disable,restartVSH", \
"button to connect USB\n(button to disconnect USB also can do)", \
"button to disconnect USB\n(button to connect USB also can do)", \
"about SORT GAME", \
"if it boots with holding this buttons,\n disable others plugins"



#define PPREFSMSG_SORTGAME_SAVE " save? "
#define PPREFSMSG_SORTGAME_HOWTOUSE "\x87+\x80/\x82:move HOME:quit START:save SEL:reload %s/R:open L:back"
#define PPREFSMSG_SORTGAME_TITLE "SORT GAME"
#define PPREFSMSG_SORTGAME_TITLE_GO "SORT GAME / After the sort,RESTART VSH"
#define PPREFSMSG_SORTGAME_PLEASECONFIG "please configure SortType in \"config\""

#endif

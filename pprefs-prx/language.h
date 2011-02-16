#ifdef PPREFS_LITE
#define PB_SYM_PSP_UP			"\x80"
#define PB_SYM_PSP_RIGHT		"\x81"
#define PB_SYM_PSP_DOWN			"\x82"
#define PB_SYM_PSP_LEFT			"\x83"
#define PB_SYM_PSP_TRIANGLE		"\x84"
#define PB_SYM_PSP_CIRCLE		"\x85"
#define PB_SYM_PSP_CROSS		"\x86"
#define PB_SYM_PSP_SQUARE		"\x87"
#define PB_SYM_PSP_NOTE			"\x88"



#define PPREFSMSG_SUBMENU_TITLE " MENU "
#define PPREFSMSG_ADD_TOP " select plugin you want to add "
#define PPREFSMSG_COPYME_ERROR "error!"
#define PPREFSMSG_COPYME_INSERTERROR "insert MS that has 'pprefs_lite'"
#define PPREFSMSG_HITOBASHIRA_2 "β2"
#define PPREFSMSG_HITOBASHIRA "β"
#define PPREFSMSG_MAINMENU_HOTOUSE " %s:Sel. \x84:Menu \x87+\x80/\x82:Move L+R:ChangeDir"
#define PPREFSMSG_MAINMENU_HOTOUSE_2 "SELECT:re-read(notSaving) HOME:Save&Quit START:Save&RESTART VSH"
#define PPREFSMSG_MAINMENU_REPUSHSTART "re-press START\n,and RESTART VSH"
#define PPREFSMSG_MAINMENU_RELOAD " re-read(not saving)  Really? "
#define PPREFSMSG_YESORNO "%s:YES %s:NO"
#define PPREFSMSG_YESORNO_LIST "YES","NO",NULL


#define PPREFSMSG_COPYME_FIRSTCONFIRM "Save your edit?"
#define PPREFSMSG_COPYME_FIRSTCONFIRMHOTOUSE "START:Save & Go to next SELECT:Not Save & Go to next\nHOME:Quit"
#define PPREFSMSG_COPYME_SELECTLIST "Save & Go to next","Not Save & Go to next","Quit",NULL
#define PPREFSMSG_COPYME_INSERT2NDMS "Insert another MS"
#define PPREFSMSG_COPYME_INSERT2NDMSHOWTOUSE "%s:Ok,Done HOME:Quit"
#define PPREFSMSG_COPYME_DONETOCOPY "Done!"
#define PPREFSMSG_COPYME_FAILTOCOPY "Fail to copy"
#define PPREFSMSG_FAILTOWRITE "Fail to write %s"
#define PPREFSMSG_FAILTOWRITEHOWTOUSE "%s:Retry %s:Skip "
#define PPREFSMSG_SELECTBOX_HOTOUSE " %s:Sel. "
#define PPREFSMSG_SELECTBOX_HOTOUSE_2 " %s:Sel. HOME:Quit "
#define PPREFSMSG_CONFIGMENU_WHATISTHIS \
	"Hot key to start this plugin", \
	"Swap X and O", \
	"to RESTART VSH on start menu ... true -> one press START false -> two press START", \
	"Line Feed Code", \
	"Folder that has vsh.txt , game.txt , pops.txt", \
	"Foreground color", \
	"Background color", \
	"Selected color", \
	"Color of explanation on bottom of screen", \

#define PPREFSMSG_SELECTBASEPATH "Where is the folder that has vsh.txt,game.txt,pops.txt?"

#define PPREFSMSG_DETECTKEY_1 "Detect Key   "
#define PPREFSMSG_DETECTKEY_2 "Detect Key %d"
#define PPREFSMSG_CONFIGMENU_TITLE " Config "
#define PPREFSMSG_CONFIGMENU_MENU_1 "Save this config"
#define PPREFSMSG_CONFIGMENU_MENU_2 "Set default"
#define PPREFSMSG_CONFIGMENU_MENU_3 "Quit"
#define PPREFSMSG_CONFIGMENU_HOWTOUSE " %s:Sel. HOME:Quit "
#define PPREFSMSG_CONFIG_BASEPATH_TITLE "BasePath"
#define PPREFSMSG_CONFIG_BASEPATH_LIST "Set","not Set",NULL

#define PPREFSMSG_ADD_HOWTOUSE " %s:Sel. HOME:Quit L:UpDir R:OpenDir "
#define PPREFSMSG_ADD_HOWTOUSE_2 " %s:Sel. HOME:Quit L:UpDir R:OpenDir START:Select current dir "
#define PPREFSMSG_ADD_HOWTOUSE_3 " HOME:Quit L:UpDir R/%s:OpenDir START:Select current dir"

#else ///////////////////////////////////////////////////////////////////////////////////


#define PB_SYM_PSP_UP			"↑"
#define PB_SYM_PSP_RIGHT		"→"
#define PB_SYM_PSP_DOWN			"↓"
#define PB_SYM_PSP_LEFT			"←"
#define PB_SYM_PSP_TRIANGLE		"△"
#define PB_SYM_PSP_CIRCLE		"○"
#define PB_SYM_PSP_CROSS		"×"
#define PB_SYM_PSP_SQUARE		"□"
#define PB_SYM_PSP_NOTE			"♪"



#define PPREFSMSG_BOOTMESSAGE " pprefs 起動準備完了! / 起動ボタン:"
#define PPREFSMSG_ADD_TOP " <<追記>> : 追記したいプラグインを選択してください "
#define PPREFSMSG_ADD_HOWTOUSE " %s:選択 HOME:やめる L:フォルダ階層上へ R:フォルダ開く "
#define PPREFSMSG_ADD_HOWTOUSE_2 " %s:選択 HOME:やめる L:フォルダ階層上へ R:フォルダ開く START:現在位置のフォルダを選択 "
#define PPREFSMSG_ADD_HOWTOUSE_3 " HOME:やめる L:フォルダ階層上へ R/%s:フォルダ開く START:現在位置のフォルダを選択"
#define PPREFSMSG_SUBMENU_LIST "追記","削除","pergame編集","SORT GAMEβ","設定","COPY ME",NULL
#define PPREFSMSG_SUBMENU_LIST_GO "追記","削除","pergame編集","SORT GAMEβ","設定",NULL
#define PPREFSMSG_SUBMENU_LIST_GO_HITOBASHIRA "追記","削除","pergame編集","SORT GAMEβ","設定","中断ゲームファイル弄る",NULL
#define PPREFSMSG_SUBMENU_HOTOUSE " %s:選択 %s:戻る "
#define PPREFSMSG_SUBMENU_TITLE "メニュー"
#define PPREFSMSG_COPYME_ERROR "エラーが発生しました"
#define PPREFSMSG_COPYME_INSERTERROR "pprefsが入っているMSを入れてください"
#define PPREFSMSG_MAINMENU_HOTOUSE " %s:選択 △:メニュー □+↑/↓:並び替え L+R:txt読書先変更"
#define PPREFSMSG_MAINMENU_HOTOUSE_2 " SELECT:編集破棄&リロード HOME:保存&終了 START:保存&VSH再起動"
#define PPREFSMSG_MAINMENU_REPUSHSTART "もう一度STARTを押すと\nRESTART VSH"
#define PPREFSMSG_MAINMENU_RELOAD " 編集を破棄してリロード? "
#define PPREFSMSG_YESORNO "%s:はい %s:いいえ"
#define PPREFSMSG_YESORNO_LIST "はい","いいえ",NULL
#define PPREFSMSG_COPYME_FIRSTCONFIRM "次の操作を実行する前に現在の編集を保存してください"
#define PPREFSMSG_COPYME_FIRSTCONFIRMHOTOUSE "START:保存して続ける SELECT:編集破棄し続ける\nHOME:やめる"
#define PPREFSMSG_COPYME_SELECTLIST "保存して続ける","編集破棄し続ける","やめる",NULL
#define PPREFSMSG_COPYME_INSERT2NDMS "2枚目のメモリースティックを\n入れてください"
#define PPREFSMSG_COPYME_INSERT2NDMSHOWTOUSE "%s:入れた HOME:コピーやめる"
#define PPREFSMSG_COPYME_DONETOCOPY "コピーが完了しました"
#define PPREFSMSG_COPYME_FAILTOCOPY "正しくコピーできませんでした"
#define PPREFSMSG_FAILTOWRITE "%sの書き込みに失敗しました"
#define PPREFSMSG_FAILTOWRITEHOWTOUSE "%s:リトライ %s:スキップ "
#define PPREFSMSG_EDITPERGAME_SELECTPRX "プラグインを選択してね"
#define PPREFSMSG_EDITPERGAME_SELECTGAME "ゲームを選択してね"
#define PPREFSMSG_EDITPERGAME_HOTOUSE "HOME:やめる START:保存 %s長押:行削除 Lトリガ:最上へ Rトリガ:最下へ"
#define PPREFSMSG_EDITPERGAME_HOTOUSE_2 "→:コメントアウト(#)切替 ←:ブラックリスト(!)切替 □+↑/↓:並び替え"
#define PPREFSMSG_EDITPERGAME_TOP " pergame編集 "
#define PPREFSMSG_SELECTBOX_HOTOUSE " %s:選択 "
#define PPREFSMSG_SELECTBOX_HOTOUSE_2 " %s:選択 HOME:やめる "
#define PPREFSMSG_HITOBASHIRA "人柱"
#define PPREFSMSG_SELECTBASEPATH "vsh.txt,game.txt,pops.txtがあるフォルダを指定してください"
#define PPREFSMSG_PAUSEGAMETEST "弄った後にRESTART VSHして下さい"
#define PPREFSMSG_DETECTKEY_1 "キー検出 残り 秒"
#define PPREFSMSG_DETECTKEY_2 "キー検出 残り%d秒"
#define PPREFSMSG_CONFIGMENU_WHATISTHIS \
	"pprefsを起動させるボタンの指定", \
	"本体を起動したときに左下に表\示される「pprefs起動準備完了!～」を・・・\ntrue→表\示 false→非表\示", \
	"×/○ボタンの役割を入れ替える\ntrue→○決定/×キャンセル false→×決定/○キャンセル(デフォルトはfalse)", \
	"トップメニューにてVSH再起動するのにSTARTを・・・\ntrue→一度押し false→二度押し", \
	"このプラグインがテキストを書き出すときに使う改行コード", \
	"vsh.txt,game.txt,pops.txtがあるフォルダのパス", \
	"通常の文字色", \
	"背景の色", \
	"選択された文字色", \
	"下に表\示される説明の文字色", \
	"USB接続機能\βを true→有効 false→無効 ※この機能\は5.XX専用\nテスト実装なのでいくつか問題がある 有効無効を切り替えたら本体(VSH)を再起動が必要", \
	"USB接続ボタン(USB切断ボタンと同じでもOK)", \
	"USB切断ボタン(USB接続ボタンと同じでもOK)", \
	"SORT GAMEに関する設定", \


#define PPREFSMSG_CONFIGMENU_TITLE " 設定 "
#define PPREFSMSG_CONFIGMENU_MENU_1 "上記の設定で保存する"
#define PPREFSMSG_CONFIGMENU_MENU_2 "デフォルト値にする"
#define PPREFSMSG_CONFIGMENU_MENU_3 "やめる"
#define PPREFSMSG_CONFIGMENU_HOWTOUSE " %s:選択 HOME:やめる "
#define PPREFSMSG_HITOBASHIRA_2 "人柱2"
#define PPREFSMSG_CONFIG_BASEPATH_LIST "指定する","指定しない",NULL
#define PPREFSMSG_CONFIG_BASEPATH_TITLE "BasePath"
#define PPREFSMSG_SORTGAME_SAVE " 保存しますか? "
#define PPREFSMSG_SORTGAME_HOWTOUSE "□+↑/↓:並び替え HOME:終了 START:保存 SELECT:編集破棄&リロード %s/R:開く L:戻る"
#define PPREFSMSG_SORTGAME_TITLE "SORT GAMEβ 並び替えたらRESTART VSHするなどしてキャッシュをクリアしてね"
#define PPREFSMSG_CONFIG_SORTTYPE_ISOCSO "*.iso/*.csoファイルを含める?"
#define PPREFSMSG_CONFIG_SORTTYPE_GAME150 "GAME150フォルダを含める?"
#define PPREFSMSG_CONFIG_SORTTYPE_GAME500 "GAME500フォルダを含める?"
#define PPREFSMSG_CONFIG_SORTTYPE_TYPE "どのpluginを使っていますか?"
#define PPREFSMSG_CONFIG_SORTTYPE_TYPE_LIST "使ってない","gamecategories","gamecategories light",NULL
#define PPREFSMSG_SORTGAME_PLEASECONFIG "\"設定\"でSortTypeの設定を行ってください"


#endif




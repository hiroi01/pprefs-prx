#ifdef LNGENG
#define PB_SYM_PSP_UP			"\x80"
#define PB_SYM_PSP_RIGHT		"\x81"
#define PB_SYM_PSP_DOWN			"\x82"
#define PB_SYM_PSP_LEFT			"\x83"
#define PB_SYM_PSP_TRIANGLE		"\x84"
#define PB_SYM_PSP_CIRCLE		"\x85"
#define PB_SYM_PSP_CROSS		"\x86"
#define PB_SYM_PSP_SQUARE		"\x87"
#define PB_SYM_PSP_NOTE			"\x88"
#else
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
#define PPREFSMSG_SUBMENU_LIST "追記","削除","pergame編集","設定","COPY ME",NULL
#define PPREFSMSG_SUBMENU_LIST_GO "追記","削除","pergame編集","設定",NULL
#define PPREFSMSG_SUBMENU_LIST_GO_HITOBASHIRA "追記","削除","pergame編集","設定","中断ゲームファイル弄る",NULL
#define PPREFSMSG_SUBMENU_HOTOUSE " %s:選択 %s:戻る "
#define PPREFSMSG_SUBMENU_TITLE "メニュー"
#define PPREFSMSG_COPYME_ERROR "エラーが発生しました"
#define PPREFSMSG_COPYME_INSERTERROR "pprefsが入っているMSを入れてください"
#define PPREFSMSG_MAINMENU_HOTOUSE " %s:選択 △:メニュー □+↑/↓:並び替え L+R:txt読書先変更"
#define PPREFSMSG_MAINMENU_HOTOUSE_2 " SELECT:編集破棄&リロード HOME:保存&終了 START:保存&VSH再起動"
#define PPREFSMSG_MAINMENU_REPUSHSTART "もう一度STARTを押すと\nRESTART VSH"
#define PPREFSMSG_MAINMENU_RELOAD " 編集を破棄して、リロードしてもよろしいですか? "
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
	"pprefsを起動させるボタンの指定(デフォルトはHOME)", \
	"本体を起動したときに左下に表\示される「pprefs起動準備完了!～」を表\示するか?\ntrue→表\示 false→非表\示\(デフォルトはtrue)", \
	"×/○ボタンの役割を入れ替える\ntrue→○決定/×キャンセル false→×決定/○キャンセル(デフォルトはfalse)", \
	"トップメニューにてVSH再起動するのにSTARTを一度押すか二度押すか\ntrue→一度押し false→二度押し(デフォルトはfalse)", \
	"このプラグインがテキストを書き出すときに使う改行コード(デフォルトはCR+LF)", \
	"vsh.txt,game.txt,pops.txtがあるフォルダのパス", \
	"通常の文字色", \
	"背景の色", \
	"選択された文字色", \
	"下に表\示される説明の文字色",
#define PPREFSMSG_CONFIGMENU_TITLE " 設定 "
#define PPREFSMSG_CONFIGMENU_MENU_1 "上記の設定で保存する"
#define PPREFSMSG_CONFIGMENU_MENU_2 "デフォルト値にする"
#define PPREFSMSG_CONFIGMENU_MENU_3 "やめる"
#define PPREFSMSG_CONFIGMENU_HOWTOUSE " %s:選択 HOME:やめる "
#define PPREFSMSG_HITOBASHIRA_2 "人柱2"
#define PPREFSMSG_CONFIG_BASEPATH_LIST "指定する","指定しない",NULL
#define PPREFSMSG_CONFIG_BASEPATH_TITLE "BasePath"
#endif

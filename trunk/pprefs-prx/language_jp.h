//

//COMMON

#define PB_SYM_PSP_UP			"↑"
#define PB_SYM_PSP_RIGHT		"→"
#define PB_SYM_PSP_DOWN			"↓"
#define PB_SYM_PSP_LEFT			"←"
#define PB_SYM_PSP_TRIANGLE		"△"
#define PB_SYM_PSP_CIRCLE		"○"
#define PB_SYM_PSP_CROSS		"×"
#define PB_SYM_PSP_SQUARE		"□"
#define PB_SYM_PSP_NOTE			"♪"

#define PPREFSMSG_MAINMENU_HOTOUSE " %s:選択 %s長押:全OFF/ON △:メニュー □+↑/↓:並び替え L+R:txt読書先変更"
#define PPREFSMSG_MAINMENU_HOTOUSE_2 " SELECT:編集破棄&リロード HOME:保存&終了 START:保存&VSH再起動"
#define PPREFSMSG_MAINMENU_REPUSHSTART "もう一度STARTを押すと\nRESTART VSH"
#define PPREFSMSG_MAINMENU_RELOAD " 編集を破棄してリロード? "

#define PPREFSMSG_YESORNO "%s:はい %s:いいえ"
#define PPREFSMSG_YESORNO_LIST "はい","いいえ",NULL

#define PPREFSMSG_ADD_TOP " <<追記>> 追記したいプラグインを選択してください "
#define PPREFSMSG_ADD_HOWTOUSE " %s:選択 HOME:やめる L:フォルダ階層上へ R:フォルダ開く "
#define PPREFSMSG_ADD_HOWTOUSE_2 " %s:選択 HOME:やめる L:フォルダ階層上へ R:フォルダ開く START:現在位置のフォルダを選択 "
#define PPREFSMSG_ADD_HOWTOUSE_3 " HOME:やめる L:フォルダ階層上へ R/%s:フォルダ開く START:現在位置のフォルダを選択"

#define PPREFSMSG_ADD_HOWTOUSE " %s:選択 HOME:やめる L:フォルダ階層上へ R:フォルダ開く "
#define PPREFSMSG_ADD_HOWTOUSE_2 " %s:選択 HOME:やめる L:フォルダ階層上へ R:フォルダ開く START:現在位置のフォルダを選択 "
#define PPREFSMSG_ADD_HOWTOUSE_3 " HOME:やめる L:フォルダ階層上へ R/%s:フォルダ開く START:現在位置のフォルダを選択"

#define PPREFSMSG_SELECTBOX_HOTOUSE " %s:選択 "
#define PPREFSMSG_SELECTBOX_HOTOUSE_2 " %s:選択 HOME:やめる "

#define PPREFSMSG_COPYME_FIRSTCONFIRM "次の操作を実行する前に現在の編集を保存してください"
#define PPREFSMSG_COPYME_FIRSTCONFIRMHOTOUSE "START:保存して続ける SELECT:編集破棄し続ける\nHOME:やめる"
#define PPREFSMSG_COPYME_SELECTLIST "保存して続ける","編集破棄し続ける","やめる",NULL
#define PPREFSMSG_COPYME_INSERT2NDMS "2枚目のメモリースティックを\n入れてください"
#define PPREFSMSG_COPYME_INSERT2NDMSHOWTOUSE "%s:入れた HOME:コピーやめる"
#define PPREFSMSG_COPYME_DONETOCOPY "コピーが完了しました"
#define PPREFSMSG_COPYME_FAILTOCOPY "正しくコピーできませんでした"
#define PPREFSMSG_COPYME_ERROR "エラーが発生しました"
#define PPREFSMSG_COPYME_INSERTERROR "pprefsが入っているMSを入れてください"

#define PPREFSMSG_FAILTOWRITE "%s%sの書き込みに失敗しました"
#define PPREFSMSG_FAILTOWRITEHOWTOUSE "%s:リトライ %s:スキップ "

#define PPREFSMSG_CONFIGMENU_TITLE " 設定 "
#define PPREFSMSG_DETECTKEY_1 "キー検出 残り 秒"
#define PPREFSMSG_DETECTKEY_2 "キー検出 残り%d秒"
#define PPREFSMSG_CONFIGMENU_MENU_1 "上記の設定で保存する"
#define PPREFSMSG_CONFIGMENU_MENU_2 "デフォルト値にする"
#define PPREFSMSG_CONFIGMENU_MENU_3 "やめる"
#define PPREFSMSG_CONFIGMENU_HOWTOUSE " %s:選択 HOME:やめる "
#define PPREFSMSG_CONFIG_BASEPATH_LIST "指定する","指定しない",NULL
#define PPREFSMSG_CONFIG_BASEPATH_TITLE "BasePath"
#define PPREFSMSG_SELECTBASEPATH "vsh.txt,game.txt,pops.txtがあるフォルダを指定してください"
#define PPREFSMSG_CONFIG_SORTTYPE_ISOCSO "*.iso/*.csoファイルを含める?"
#define PPREFSMSG_CONFIG_SORTTYPE_GAME150 "GAME150フォルダを含める?"
#define PPREFSMSG_CONFIG_SORTTYPE_GAME5XX "GAME5XXフォルダを含める?"
#define PPREFSMSG_CONFIG_SORTTYPE_TYPE "どのpluginを使っていますか?"
#define PPREFSMSG_CONFIG_SORTTYPE_TYPE_LIST "使ってない","gamecategories","gamecategories light(FW6.20)","gamecategories light(FW6.3x)",NULL
#define PPREFSMSG_CONFIG_SORTTYPE_NOTDISPLAY_ICON0 "アイコン表\示をしますか?"

#define PPREFSMSG_THEOTHER "その他"

#define PPREFSMSG_BACKUPMENU {"ロード","セーブ",NULL}
#define PPREFSMSG_BACKUPMENU_HOWOTOUSE "%s:選択 HOME:やめる [R]/[L]Textの種類切り替え"
#define PPREFSMSG_BACKUPMENU_NOTEXITST "ファイルが存在しません"

#define PPREFSMSG_HITOBASHIRA "人柱"
#define PPREFSMSG_HITOBASHIRA_2 "人柱2"

/*--------------------------------------*/
#ifdef PPREFS_LITE
/*--------------------------------------*/

#define PPREFSMSG_SUBMENU_TITLE " メニュー "
#define PPREFSMSG_SUBMENU_LIST "追記","削除","バックアップ","設定","COPY ME",NULL
#define PPREFSMSG_SUBMENU_LIST_GO "追記","削除","バックアップ","設定",NULL

#define PPREFSMSG_CONFIGMENU_WHATISTHIS \
"pprefsを起動させるボタンの指定", \
"×/○ボタンの役割を入れ替える\ntrue→○決定/×キャンセル false→×決定/○キャンセル(デフォルトはfalse)", \
"トップメニューにてVSH再起動するのにSTARTを・・・\ntrue→一度押し  false→二度押し", \
"このプラグインがテキストを書き出すときに使う改行コード", \
"vsh.txt,game.txt,pops.txtがあるフォルダのパス", \
"通常の文字色", \
"背景の色", \
"選択された文字色", \
"下に表\示される説明の文字色", \
"[ON]の色", \
"[OFF]の色", \
"本体起動時にこのボタン押していると、\nほかのプラグインを無効化する",

/*--------------------------------------*/
#else
/*--------------------------------------*/

#define PPREFSMSG_BOOTMESSAGE " pprefs 起動準備完了! / 起動ボタン:"

#define PPREFSMSG_SUBMENU_TITLE "メニュー"
#define PPREFSMSG_SUBMENU_LIST					"追記","削除","バックアップ","SORT GAME","設定","pergame編集","COPY ME",NULL
#define PPREFSMSG_SUBMENU_LIST_HITOBASHIRA		"追記","削除","バックアップ","SORT GAME","設定","pergame編集","COPY ME","remove FILE",NULL
#define PPREFSMSG_SUBMENU_LIST_GO				"追記","削除","バックアップ","SORT GAME","設定","pergame編集",NULL
#define PPREFSMSG_SUBMENU_LIST_GO_HITOBASHIRA	"追記","削除","バックアップ","SORT GAME","設定","pergame編集","中断ゲームファイル弄る","remove FILE",NULL

#define PPREFSMSG_EDITPERGAME_SELECTPRX "プラグインを選択してね"
#define PPREFSMSG_EDITPERGAME_SELECTGAME "ゲームを選択してね"
#define PPREFSMSG_EDITPERGAME_HOTOUSE "HOME:やめる START:保存 %s長押:行削除 Lトリガ:最上へ Rトリガ:最下へ"
#define PPREFSMSG_EDITPERGAME_HOTOUSE_2 "→:コメントアウト(#)切替 ←:ブラックリスト(!)切替 □+↑/↓:並び替え"
#define PPREFSMSG_EDITPERGAME_TOP " pergame編集 "

#define PPREFSMSG_PAUSEGAMETEST "弄った後にRESTART VSHして下さい"
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
"[ON]の色", \
"[OFF]の色", \
"USB接続機能\βを true→有効 false→無効 ※この機能\は5.XX専用\nテスト実装なのでいくつか問題がある 有効無効を切り替えたら本体(VSH)を再起動が必要", \
"USB接続ボタン(USB切断ボタンと同じでもOK)", \
"USB切断ボタン(USB接続ボタンと同じでもOK)", \
"SORT GAMEに関する設定", \
"本体起動時にこのボタン押していると、\nほかのプラグインを無効化する",



#define PPREFSMSG_SORTGAME_SAVE " 保存しますか? "
#define PPREFSMSG_SORTGAME_HOWTOUSE "□+↑/↓:並び替え HOME:終了 START:保存 SELECT:編集破棄&リロード %s/R:開く L:戻る"
#define PPREFSMSG_SORTGAME_TITLE "SORT GAME"
#define PPREFSMSG_SORTGAME_TITLE_GO "SORT GAME /  並び替えたらRESTART VSHして下さい"

#define PPREFSMSG_SORTGAME_PLEASECONFIG "\"設定\"でSortTypeの設定を行ってください"

#endif

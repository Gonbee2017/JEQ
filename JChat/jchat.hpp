//// JEQ/日本語チャットプラグイン/本体

// ※このプラグインはプラグインローダーであるdinput8.dllによって
// eqgame.exeプロセスにロードされ、使用されることを想定している。

//// 開発した動機
// 英語版のEQクライアントでは日本語版のように日本語を入力できない。
// チャットの入力欄で全角キーを押しても日本語入力モードにならない。
// 英語版で日本語を入力できるようにするには、MQ2のようにeqgame.exe
// を拡張して、プロセスの内側に機能を追加しなければならなかった。

//// 入力方法を模索

// 理想としてはEQのチャットウインドウで日本語を入力できるように
// したかったが、そのためにはeqgame.exeの機械語をかなり深くまで
// 解析する必要があり、自分のスキルでは難しかったため断念した。

// 妥協案としてEQのチャットウインドウとは別に、小さなウインドウを
// 新たに表示して、そこに日本語を入力できるようにしようとした。
// このウインドウを「日本語チャットバー」と呼ぶことにする。
// しかしウインドウを表示することはできたものの、その上に配置した
// 入力欄(EDITコントロール)がキーボードからの入力に反応しなかった。
// その原因はWM_CHARメッセージが送信されていないからだった。
// WM_CHARメッセージはTranslateMessage関数によって送信される。
// eqgame.exeのメッセージループではこのTranslateMessageを呼び出して
// いないらしく、それゆえWM_CHARメッセージが送信されていなかった。

// これを解決するためにDispatchMessage関数のDetourを作ることにした。
// 普通のメッセージループではGetMessage関数でメッセージを取得したら、
// TranslateMessage→DispatchMessageを立て続けに呼び出す。
// それと同じことをするために、DispatchMessageのDetour関数の中で
// TranslateMessage→DispatchMessageのTarget関数と呼び出すように
// したら、入力欄がキーボードからの入力に反応するようになった。
// しかし日本語入力モードのままだと、メインウインドウに戻り、
// キーボードから何か入力すると、その入力にIMEが反応してしまい、
// 変換ウインドウが表示され、画面が隠れてプレイの邪魔になった。
// そこでDispatchMessageのDetour関数では日本語チャットバーが
// アクティブなときにだけTranslateMessageを呼び出すことにした。

//// アイテムリンクを模索

// 次に大きな課題になったのがアイテムのリンクである。
// EQクライアントでインベントリを表示し、スロットにあるアイテムを
// 右クリック長押しすれば、そのアイテムのウインドウを表示できる。
// そしてウインドウ上にあるアイテムのアイコンをクリックすれば、
// チャットの入力欄にそのアイテムのリンクを貼り付けることができる。
// リンクは特殊な文字列として扱われ、ピンクの文字色で表示される。
// このリンクを日本語チャットのほうでも扱えるようにしようと考えた。

// 理想としてはアイテムのウインドウでアイコンがクリックされたら、
// そのイベントをインターセプトして、リンクをEQチャットではなく、
// 日本語チャットのほうに表示するようにしたかった。
// しかしMQ2のソースコードを見渡しても件のイベントに関係する
// 箇所は見つからず、自力でのexe解析も難しかったため断念した。

// 妥協案としてアイテムのリンクはEQチャットから転送することにした。
// まずリンクをいったんEQチャットの入力欄に貼り付けてもらう。
// そしてEnterキーを押すと、通常はリンクを含む発言をすることになる。
// このプラグインはそれを横取りして、発言に含まれるリンクを解析し、
// 次のような独自の形式に変換してから日本語チャットへと転送する。
// "{番号:名前}"
// リンクは'{'と'}'で囲み、番号とアイテムの名前を埋め込む。
// 例えばRationとWater FlaskのリンクをEQチャットから日本語チャットに
// 転送すると、それぞれ{0:Ration}と{1:Water Flask}のように表示される。
// その後、日本語チャットでEnterキーを押すと、今度は逆に独自の形式から
// 元の形式に戻されて、チャットにリンクを発言する、という流れになる。
// これで少し手間だが、日本語と一緒にリンクを発言できるようになった。

//// 実行方法

// 日本語チャットバーが表示されるまでの手順を以下に示す。
// ①eqgame.exeと同じフォルダに以下のファイルを配置する。
// ・dinput8.dll … プラグインローダー
// ・dinput8.ini … プラグインローダーの設定
// ・jchat.dll   … 日本語チャットプラグイン
// ・jchat.ini   … 日本語チャットプラグインの設定
// ②eqgame.exeを起動すると、Windowsによって本物のdinput8.dllの代わりに
//   ①で配置した偽物のdinput8.dllがロードされ、実行される。
//   さらに偽物のdinput8.dllによってjchat.dllがロードされ、実行される。
// ③サーバーを選択し、接続する。
// ④キャラクターを選択し、ログインする。
// ⑤ゾーンにPOPするとチャットバーが表示される。

// ただしjchat.iniについては2種類のバージョンが用意されており、
// クライアントによってRoF2向けとTitanium向けに分かれている。

// jchat.iniには以下の設定が保存されている。
// ・ウインドウの位置と幅
// ・フォントの名前
// ・アイテムリンクのサイズ
// ・eqgame.exeにあるデータや関数のオフセット
// ・よく使う発言リスト

// jchat.iniの設定は、プログラムのデータを保存する目的のものが多く、
// ユーザーが直接変更できるのはフォントの名前ぐらいしかない。
// またエラーログはjchat.logに書き込まれる。

//// チャットの機能

// 日本語チャットバーは左端の"日"のアイコンをドラッグすれば移動でき、
// 右端のつまみをドラッグすれば幅を調整できる。
// また常にメインウインドウの上に表示されるため、重ねても下に隠れない。
// ウインドウの位置と幅はクライアント終了時にjchat.iniに保存され、
// 次回起動時に読み込まれるので、毎回調整する必要はない。

// メインウインドウでEnterキーを押すと、通常はEQチャットが入力できる
// 状態になるが、このプラグインがロードされていると、そのイベントを
// インターセプトして、日本語チャットを入力できる状態にする。
// 同様にスラッシュキーもインターセプトする。

// EQチャットで発言しようとすると、発言しようとしたテキストが
// そのまま日本語チャットに転送され、発言自体がキャンセルされる。
// 発言の中にアイテムのリンクが含まれていれば、独自の形式に
// 変換されてから日本語チャットに表示される。

// 入力欄ではメモ帳などと同じようにテキストを入力・編集できる。
// 日本語の入力はもちろん、コピー・カット・ペーストなどもできる。
// テキストを入力したら、EQチャットと同じく、Enterキーで発言できる。
// /locや/targetのようなコマンドもすべて同じように実行できる。

// どのチャンネルで発言するかはチャンネル欄のコンボボックスで指定する。
// 選択できるチャンネルを以下に示す。
// ・/Auction
// ・/Gsay
// ・/gUildsay
// ・/Ooc
// ・/Reply
// ・/Say
// ・/sHout
// 入力内容がコマンドでなければ、選択中のチャンネルが付加される。
// 例えば"/Say"を選択中に"Hoge"と入力してEnterキーを押すと、
// "Hoge"の前に"/Say "が付加され、"/Say Hoge"が実行される。

// チャンネルはAlt+大文字のショートカットキーで選択できる。
// 例えばAlt+Gキーを押せば"/Gsay"に切り替えられる。

// 入力内容の最初の文字が"/#^"のいずれかであればコマンドとみなされる。
// 例えば"/Gsay"を選択中に"/tell foo bar"と入力してEnterキーを押すと、
// コマンドと判定されて"/tell foo bar"がそのまま実行される。

// 発言すると、そのテキストが「今までの発言リスト」に追加される。
// ↑キーを押せば今までの発言リストをたどることができる。
// ↓キーを押せば反対方向に戻ることができる。

// テキストを入力した後、EnterキーではなくShift+Enterキーを押せば、
// 入力欄のテキストを「よく使う発言リスト」に登録できる。
// Shift+↑キーを押せばよく使う発言リストをたどることができる。
// Shift+↓キーを押せば反対方向に戻ることができる。
// よく使う発言リストはクライアント終了時にjchat.iniに保存され、
// 次回起動時に読み込まれるので、毎回登録する必要はない。

// 今までの発言リストやよく使う発言リストをたどっているときに
// Shift+Deleteキーを押せば選択中の発言を削除できる。
// またEscキーを押せばチャットの入力をいったん保留して、
// メインウインドウの操作に戻ることができる。

#ifndef JEQ_JCHAT_HPP
#define JEQ_JCHAT_HPP

//// インクルード

#include "eqgamedef.hpp"
#include "jcommon.hpp"

#include <windows.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jeq {

//// 定数の定義

// チャンネル欄の定数。
constexpr std::size_t CHANNEL_HEIGHT = 100; // 高さ。
constexpr int CHANNEL_ID = 1;               // ID。
constexpr int CHANNEL_TOP = 2;              // 上辺のY座標。
constexpr std::size_t CHANNEL_WIDTH = 74;   // 幅。
constexpr COLORREF CHANNEL_BK_COLOR =       // 背景色。
	RGB(14, 16, 22);

// 入力欄の定数。
constexpr std::size_t EDIT_HEIGHT = 12;      // 高さ。
constexpr COLORREF EDIT_INNER_SUNKEN_COLOR = // 内側の枠線の右辺・下辺の色。
	RGB(0, 0, 0);
constexpr COLORREF EDIT_INNER_RAISED_COLOR = // 内側の枠線の左辺・上辺の色。
	RGB(0, 0, 0);
constexpr int EDIT_ID = 2;                   // ID。
constexpr COLORREF EDIT_OUTER_SUNKEN_COLOR = // 外側の枠線の右辺・下辺の色。
	RGB(132, 150, 181);
constexpr COLORREF EDIT_OUTER_RAISED_COLOR = // 外側の枠線の左辺・上辺の色。
	RGB(74, 93, 115);
constexpr int EDIT_TOP = 2;                  // 上辺のY座標。

// EQチャットのリンクの定数。
constexpr char EQCHAT_LINK_EDGE = 0x12; // 両端の記号。

// jchat.iniの定数。
constexpr char INI_CHAT_COMMAND_SYMBOLS_DEF[] = "/#^";  // コマンド記号の列の既定値。
constexpr char INI_CHAT_FONT_NAME_DEF[] =               // フォントの名前の既定値。
	"ＭＳ Ｐゴシック";
constexpr std::size_t INI_CHAT_LINK_BODY_SIZE_DEF = 56; // リンクのボディ部分のサイズの既定値。
constexpr int INI_WINDOW_LEFT_DEF = -10000;             // ウインドウの左辺のX座標の既定値。
constexpr std::size_t INI_WINDOW_MIN_WIDTH_DEF = 150;   // ウインドウの最小幅の既定値。
constexpr int INI_WINDOW_TOP_DEF = -10000;              // ウインドウの上辺のY座標の既定値。
constexpr std::size_t INI_WINDOW_WIDTH_DEF = 300;       // ウインドウの幅の既定値。

// 日本語チャットのリンクの定数。
constexpr char JCHAT_LINK_CLOSE = '}';   // 終端の記号。
constexpr char JCHAT_LINK_DIVIDER = ':'; // 番号と名前を分割する記号。
constexpr char JCHAT_LINK_ESCAPE = '$';  // 衝突回避の記号。
constexpr char JCHAT_LINK_OPEN = '{';    // 始端の記号。

// 日本語チャットバーの定数。
constexpr COLORREF JCHAT_BAR_SUNKEN_COLOR =      // 枠線の右辺・下辺の色。
	RGB(49, 48, 41);
constexpr COLORREF JCHAT_BAR_RAISED_COLOR =      // 枠線の左辺・上辺の色。
	RGB(165, 158, 115);
constexpr char JCHAT_BAR_CLASS_NAME[] = "JChat"; // クラスの名前。
constexpr std::size_t JCHAT_BAR_HEIGHT = 24;     // 高さ。
constexpr char JCHAT_BAR_NAME[] = "JChat";       // タイトル。

// その他の定数。
constexpr std::size_t FONT_SIZE = 12;   // フォントのサイズ。
constexpr std::size_t ICON_WIDTH = 24;  // アイコンの幅。
constexpr std::size_t KNOB_WIDTH = 8;   // つまみの幅。
constexpr int EDIT_LEFT =               // 入力欄の左辺のX座標。
	ICON_WIDTH + CHANNEL_WIDTH;
constexpr std::size_t EDIT_HOR_MARGIN = // 入力欄の水平方向の余白。
	EDIT_LEFT + KNOB_WIDTH;
constexpr COLORREF TEXT_COLOR =         // 文字色。
	RGB(223, 223, 224);

// 入力欄のテキストを変更するときのモードを表す。
enum class change_modes : int {
	EDIT,   // 編集することで変更する。
	CHOICE, // リストから選択することで変更する。
};

// 画面モードを表す。
enum class screen_modes : int {
	WINDOW,      // ウインドウ。
	FULL_SCREEN, // フルスクリーン。
	ICONIC,      // アイコン。
};

//// 型の定義

// DispatchMessage関数の型。
using DispatchMessage_t = LRESULT (WINAPI*)(const MSG*);

// eqgame.exeの関数やデータの集合を表す。
// クラスのメンバ関数はインスタンスにアクセスできる必要がある。
// インスタンスへのアクセスはthisポインタを介して行われる。
// VC++においてthisポインタはECXレジスタに格納されるため、
// メンバ関数内ではECXレジスタをthisポインタのためだけに使用する。
// それに対してstaticな関数はECXレジスタを他の目的で使用する。
// さてstaticな関数をメンバ関数のDetourにできるだろうか？
// Target関数もそれを呼び出すコードもECXレジスタにthisポインタが
// 格納されている前提で組まれているため、Detour関数が実行される過程で
// ECXレジスタが書き換えられるとプログラムがクラッシュしてしまう。
// ECXレジスタを保護するには、Detour関数をメンバ関数にすればよい。
// Detour関数が属するクラスは、Target関数が属するクラスとメモリの
// レイアウトが一致している必要はなく、ただECXレジスタに格納されている
// thisポインタを書き換えることなく実行できるというだけで十分である。
// eqgame.exeにある関数のDetour関数はこのクラスのメンバとして定義する。
// またTarget関数のポインタやいくつかのデータへのポインタも定義する。
class eqgame_t {
public:
	using CEverQuest_InterpretCmd_t =       // CEverQuestクラスのInterpretCmdメンバ関数の型。
		void (eqgame_t::*)(eqgame_t*,char*);
	using CEverQuest_SetGameState_t =       // CEverQuestクラスのSetGameStateメンバ関数の型。
		void (eqgame_t::*)(int);
	using CXWndManager_DrawCursor_t =       // CEverQuestクラスのDrawCursorメンバ関数の型。
		int (eqgame_t::*)();
	using KeypressHandler_HandleKeyDown_t = // KeypressHandlerクラスのHandleKeyDownメンバ関数の型。
		bool (eqgame_t::*)(const KeyCombo&);
	using KeypressHandler_HandleKeyUp_t =   // KeypressHandlerクラスのHandleKeyUpメンバ関数の型。
		bool (eqgame_t::*)(const KeyCombo&);
	using ProcessGameEvents_t = BOOL (*)(); // ProcessGameEvents関数の型。

	static const std::unordered_set<DWORD> ACTIVATE_KEY_SITUATIONS;
	static const std::unordered_set<DWORD> IGNORE_KEY_SITUATIONS;

	static eqgame_t **CEverQuest;
	static eqgame_t **CharSpawn;
	static HWND *MainWindowHandle;
	static PBYTE NotInChatMode;

	static CEverQuest_InterpretCmd_t CEverQuest_InterpretCmd_target;
	static CEverQuest_SetGameState_t CEverQuest_SetGameState_target;
	static CXWndManager_DrawCursor_t CXWndManager_DrawCursor_target;
	static KeypressHandler_HandleKeyDown_t KeypressHandler_HandleKeyDown_target; 
	static KeypressHandler_HandleKeyUp_t KeypressHandler_HandleKeyUp_target;
	static ProcessGameEvents_t ProcessGameEvents_target;

	void CEverQuest_InterpretCmd_detour(eqgame_t *player, char *cmd);
	void CEverQuest_SetGameState_detour(int game_state);
	int CXWndManager_DrawCursor_detour();
	bool KeypressHandler_HandleKeyDown_detour(const KeyCombo &key_combo);
	bool KeypressHandler_HandleKeyUp_detour(const KeyCombo &key_combo);
	static BOOL ProcessGameEvents_detour();
};

// 行のリストとその反復子のセットを表す。
// 今までの発言リストとよく使う発言リストの型として使用する。
struct lines_and_iter_t {
	// 行のリストの型。
	using lines_t = std::list<std::string>;

	lines_t lines;                        // 行のリスト。
	lines_t::iterator iter = lines.end(); // 行のリストの反復子。

	inline bool isBegin() const;
	inline bool isEnd() const;
	inline void moveBegin();
	inline void moveEnd();
};

// 日本語チャットバーを表す。
// メインウインドウと共に、自PCがスポーンしているときに表示される。
// 幅400ピクセル(既定値)、高さ24ピクセルの横に長い棒のような形をしている。
// チャットバーの上にはチャンネル欄と入力欄が配置されている。
// EQチャットと同じように発言したり、コマンドを実行することができる。
class jchat_bar_t {
public:
	static void registerClass();

	void activate(const KeyCombo &key_combo);
	void create();
	void fixPos();
	HWND getHandle();
	void transferText(const char *eqc_text);
protected:
	using message_handler_t = // メッセージハンドラの型。
		LRESULT (jchat_bar_t::*)(HWND,UINT,WPARAM,LPARAM);
	using message_map_t =     // メッセージマップの型。
		std::unordered_map<
			UINT,             // メッセージ。
			message_handler_t // メッセージハンドラ。
		>;

	// インスタンスデータを表す。
	struct data_t {
		HDC buffer_dc_handle = NULL;          // 背景バッファのデバイスコンテキストのハンドル。
		std::vector<scope_exit_t> exits;      // 脱出処理のベクタ。
		HWND handle = NULL;                   // チャットバーのハンドル。
		HFONT hfont = NULL;                   // フォントのハンドル。
		lines_and_iter_t history;             // 今までの発言リスト。
		HDC icon_active_dc_handle = NULL;     // アクティブアイコンのデバイスコンテキストのハンドル。
		HDC icon_inactive_dc_handle = NULL;   // インアクティブアイコンのデバイスコンテキストのハンドル。
		HDC knob_dc_handle = NULL;            // つまみのデバイスコンテキストのハンドル。
		std::vector<std::string> link_bodies; // アイテムリンクのボディ部分のベクタ。
		std::vector<HWND> ordered_handles;    // Tab順のハンドルのベクタ。
		std::unordered_map<HWND,int> 
			ordered_indices;                  // ハンドル→Tab順のインデックスのマップ。
		std::unordered_map<COLORREF,HPEN> 
			pen_cache;                        // ペンのキャッシュ。
		lines_and_iter_t registry;            // よく使う発言リスト。
		HBRUSH rock_bar_brush_handle = NULL;  // 岩棒ブラシのハンドル。
		HBRUSH rock_wall_brush_handle = NULL; // 岩壁ブラシのハンドル。
		POINT mw_client_pos;                  // メインウインドウのクライアント座標系における位置。
		struct {
			WNDPROC WindowProc_super = NULL; // サブクラス化前のウインドウプロシージャ。
			HWND handle = NULL;              // ハンドル。
		} channel; // チャンネル欄のデータ。
		struct {
			WNDPROC WindowProc_super = NULL; // サブクラス化前のウインドウプロシージャ。
			change_modes change_mode =       // テキストを変更するときのモード。
				change_modes::EDIT;
			bool dirty = false;              // 更新フラグ。
			HWND handle = NULL;              // ハンドル。
			std::string last_line;           // 最後に編集されていた行。
		} edit; // 入力欄のデータ。

		~data_t();
	};

	static const std::unordered_map<BYTE,int> CHANNEL_INDICES;
	static const message_map_t CHANNEL_MESSAGE_MAP;
	static const std::array<std::string,7> CHANNEL_TEXTS;
	static const message_map_t EDIT_MESSAGE_MAP;
	static const message_map_t MESSAGE_MAP;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK channel_WindowProc_sub(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK edit_WindowProc_sub(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // インスタンスデータ。

	LRESULT channel_onChar(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT channel_onSysKeyDown(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT ctl_onChar(WNDPROC super_proc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT ctl_onSysKeyDown(WNDPROC super_proc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT edit_onChar(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT edit_onKeyDown(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT edit_onSysKeyDown(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onActivate(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCommand(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCreate(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCtlColorEdit(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCtlColorListBox(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onDestroy(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onGetMinMaxInfo(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onMove(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onNCHitTest(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onNCLButtonDblClk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onPaint(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onSetCursor(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onSetFocus(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onSize(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	
	void channel_create(HWND hwnd);
	void channel_select(int ch_index);
	void deleteFromList();
	void drawEdge(HDC hdc, int left, int top, int width, int height, COLORREF raised_color, COLORREF sunken_color);
	void edit_create(HWND hwnd);
	std::string edit_getLine();
	void edit_replaceText(const std::string &text);
	void edit_selectText(int first = 0, int last = -1);
	void edit_setLine(const std::string &line = std::string(), change_modes change_mode = change_modes::EDIT);
	void execute(SHORT shift_state);
	HPEN getPen(COLORREF color);
	void loadRegistry();
	void moveFocus(HWND ctl_handle, SHORT shift_state);
	void moveThroughList(SHORT shift_state, BYTE vir_key);
	void prepare(HWND hwnd);
	void storeRegistry();
	std::string text_addChannel(const std::string &jc_text);
	std::string text_eqChatToJChat(const char *eqc_text);
	std::string text_jChatToEQChat(const std::string &jc_text);
};

// プログラムの文脈(状況)を表す。
// すべてのグローバル変数をまとめた構造体である。
// まとめることで単体テストにおいて簡単にリセットできる。
struct context_t {
	DWORD base_address;                     // eqgame.exeのベースアドレス。
	std::vector<scope_exit_t> detour_exits; // Detour用の脱出処理のベクタ。
	HINSTANCE dll_handle;                   // jchat.dllのハンドル。
	std::filesystem::path dll_path;         // jchat.dllのパス。
	std::vector<scope_exit_t> exits;        // 脱出処理のベクタ。
	int game_state = 0;                     // ゲームの状態。
	std::filesystem::path ini_path;         // jchat.iniのパス。
	jchat_bar_t jchat_bar;                  // 日本語チャットバー。
	std::shared_ptr<std::ostream> log;      // jchat.logの出力ストリーム。
	std::filesystem::path log_path;         // jchat.logのパス。
	screen_modes screen_mode;               // 画面モード。
	struct {
		struct {
			std::string command_symbols; // コマンド記号の列。
			std::string font_name;       // フォントの名前。
			std::size_t link_body_size;  // アイテムリンクのボディ部分のサイズ。
		} chat; // チャットに関する設定。
		struct {
			int left;              // 左辺のX座標。
			std::size_t min_width; // 最小の幅。
			int top;               // 上辺のY座標。
			std::size_t width;     // 幅。
		} window; // ウインドウに関する設定。
	} ini; // jchat.iniの内容。

	void fail(const error_t &err);
	void release();
	void startup(HINSTANCE hinstDLL);
};

//// グローバル変数の宣言

extern DispatchMessage_t DispatchMessage_target;
extern context_t context;

//// テンプレート関数とインライン関数の宣言

inline DWORD makeKeySituation(BYTE not_in_chat_mode, const KeyCombo &key_combo);

//// 関数の宣言

LRESULT WINAPI DispatchMessage_detour(const MSG *msg);

void bindAddressAndDetourAttach_early();
void bindAddressAndDetourAttach_lazy();
DWORD getAddress(const std::string &name);
DWORD getOffset(const std::string &name);
screen_modes getScreenMode();
void onGameStateChange();
BOOL onProcessAttach(HINSTANCE hinstDLL);
BOOL onProcessDetach();
void onScreenModeChange();
void refreshGameState(int new_game_state);
void refreshScreenMode();

//// クラスのテンプレートメンバとインラインメンバの定義

// 反復子がリストの先頭にあるかどうかを判定する。
bool // 真なら先頭、偽ならそれ以外。
lines_and_iter_t::isBegin() const {
	return iter == lines.begin();
}

// 反復子がリストの末尾にあるかどうかを判定する。
bool // 真なら末尾、偽ならそれ以外。
lines_and_iter_t::isEnd() const {
	return iter == lines.end();
}

// 反復子をリストの先頭に移動する。
void lines_and_iter_t::moveBegin() {
	iter = lines.begin();
}

// 反復子をリストの末尾に移動する。
void lines_and_iter_t::moveEnd() {
	iter = lines.end();
}

//// テンプレート関数とインライン関数の定義

// チャットモードフラグとキーの組み合わせを合成して、
// キーの状況を作成する。
DWORD // 作成したキーの状況。
makeKeySituation(
	BYTE not_in_chat_mode,    // EQチャットモードフラグ。
	const KeyCombo &key_combo // キーの組み合わせ。
) {
	return (DWORD(not_in_chat_mode) << 7) | indirect_cast<DWORD>(key_combo);
}

}

#endif // #ifndef JEQ_JCHAT_HPP

//// JEQ/日本語チャットプラグイン/本体

//// インクルード

#include "common.hpp"
#include "eqgamedef.hpp"
#include "jchat.hpp"
#include "resource.h"

#include <dinput.h>
#include <windows.h>
#include <detours.h>

#undef max
#undef min

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstring>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

//// マクロの定義

// DetourAttach関数の呼び出しを含むトランザクションを開始する。
// これ以降、DetourAttachしたら、ローカル変数detach_procsに
// その関数をDetourDetachする処理を追加すること。
#define BEGIN_DETOUR_ATTACH() \
	dtr::DetourTransactionBegin();\
	scope_exit_t abort_exit([] {\
		dtr::DetourTransactionAbort();\
	});\
	std::vector<scope_exit_t::procedure_t> detach_procs;

// jchat.iniからオフセットを読み込み、アドレスを計算し、
// eqgame_tに属するデータのポインタに代入する。
#define BIND_ADDRESS(\
	name /* eqgame_t::に続くポインタの名前。 */\
) \
	{\
		DWORD *ptr = &(DWORD&)(eqgame_t::name);\
		*ptr = getAddress(#name);\
	}

// nameが示す関数についてDetourAttach関数を呼び出す。
// Target関数ポインタの名前はname_targetになる。
// Detour関数の名前はname_detourになる。
// ただしnameは実引数で置き換えられる。
// 成功したら、detach_procsにDetourDetachする処理を追加する。
// Attach後、Target関数ポインタの値はTarget関数のアドレスから
// Trampoline関数のアドレスに入れ替わるので注意すること。
#define DETOUR_ATTACH(\
	name /* 関数の名前。 */\
) \
	{\
		PVOID *target = &(PVOID&)(name ## _target);\
		PVOID detour = indirect_cast<PVOID>(&name ## _detour);\
		dtr::DetourAttach(target, detour);\
		detach_procs.emplace_back([target, detour] {\
			dtr::DetourDetach(target, detour);\
		});\
	}

// jchat.iniからアドレスを読み込み、アドレスを計算し、
// eqgame_tに属するTarget関数ポインタに代入する。
// そしてnameが示す関数についてDetourAttach関数を呼び出す。
// Target関数ポインタの名前はeqgame_t::name_targetになる。
// Detour関数の名前はeqgame_t::name_detourになる。
// ただしnameは実引数で置き換えられる。
// 成功したら、detach_procsにDetourDetachする処理を追加する。
// Attach後、Target関数ポインタの値はTarget関数のアドレスから
// Trampoline関数のアドレスに入れ替わるので注意すること。
#define BIND_ADDRESS_AND_DETOUR_ATTACH(\
	name /* eqgame_t::に続く関数の名前。 */\
) \
	{\
		DWORD *ptr = &(DWORD&)(eqgame_t::name ## _target);\
		*ptr = getAddress(#name);\
		DETOUR_ATTACH(eqgame_t::name);\
	}

// DetourAttach関数の呼び出しを含むトランザクションを終了する。
// 成功したら、context.detour_exitsに、トランザクション中に
// DetourAttachした関数をDetourDetachする脱出処理を追加する。
#define END_DETOUR_ATTACH() \
	dtr::DetourTransactionCommit();\
	abort_exit.procedure() = nullptr;\
	for (auto detach_proc : detach_procs)\
		context.detour_exits.emplace_back(detach_proc);

namespace jeq {

//// クラスメンバの定義

// 日本語チャットバーをアクティブにするキー状況のセット。
const std::unordered_set<DWORD> eqgame_t::ACTIVATE_KEY_SITUATIONS = {
	makeKeySituation(TRUE , KeyCombo{0, 0, 0, DIK_RETURN}),
	makeKeySituation(TRUE , KeyCombo{0, 0, 0, DIK_SLASH }),
	makeKeySituation(TRUE , KeyCombo{0, 0, 1, DIK_UP    }),
	makeKeySituation(TRUE , KeyCombo{0, 0, 1, DIK_DOWN  }),
	makeKeySituation(FALSE, KeyCombo{0, 0, 0, DIK_RETURN}),
};

// 無視するキー状況のセット。
const std::unordered_set<DWORD> eqgame_t::IGNORE_KEY_SITUATIONS = {
	makeKeySituation(TRUE, KeyCombo{0, 0, 0, DIK_RETURN}),
	makeKeySituation(TRUE, KeyCombo{0, 0, 0, DIK_SLASH }),
	makeKeySituation(TRUE, KeyCombo{0, 0, 1, DIK_UP    }),
	makeKeySituation(TRUE, KeyCombo{0, 0, 1, DIK_DOWN  }),
};

eqgame_t **eqgame_t::CEverQuest;  // エバークエスト。
eqgame_t **eqgame_t::CharSpawn;	  // 自PCのスポーン。
HWND *eqgame_t::MainWindowHandle; // メインウインドウのハンドル。
PBYTE eqgame_t::NotInChatMode;	  // EQチャットモードフラグ。
                                  // セットならEQチャットに入力できる状態ではない、
                                  // クリアならEQチャットに入力できる状態である。

// CEverQuestクラスのInterpretCmdメンバのTarget関数ポインタ。
eqgame_t::CEverQuest_InterpretCmd_t 
	eqgame_t::CEverQuest_InterpretCmd_target;
// CEverQuestクラスのSetGameStateメンバのTarget関数ポインタ。
eqgame_t::CEverQuest_SetGameState_t 
	eqgame_t::CEverQuest_SetGameState_target;
// CXWndManagerクラスのDrawCursorメンバのTarget関数ポインタ。
eqgame_t::CXWndManager_DrawCursor_t 
	eqgame_t::CXWndManager_DrawCursor_target;
// KeypressHandlerクラスのHandleKeyDownメンバのTarget関数ポインタ。
eqgame_t::KeypressHandler_HandleKeyDown_t 
	eqgame_t::KeypressHandler_HandleKeyDown_target;
// KeypressHandlerクラスのHandleKeyUpメンバのTarget関数ポインタ。
eqgame_t::KeypressHandler_HandleKeyUp_t 
	eqgame_t::KeypressHandler_HandleKeyUp_target;
// ProcessGameEventsのTarget関数ポインタ。
eqgame_t::ProcessGameEvents_t 
	eqgame_t::ProcessGameEvents_target;

// CEverQuestクラスのInterpretCmdメンバのDetour関数。
// コマンドの内容を解析し、実行するために呼び出される。
// EQチャットから入力されたコマンドはそのまま日本語チャットへ転送する。
// ただし非EQチャットモードのときは何もしない。
// エラーが発生したらjchat.dllの機能を停止する。
void eqgame_t::CEverQuest_InterpretCmd_detour(
	eqgame_t *player, // プレイヤー。
	char *cmd         // 実行するコマンド。
) {
	if (!*NotInChatMode) context.jchat_bar.transferText(cmd);
	else (this->*CEverQuest_InterpretCmd_target)(player, cmd);
}

// CEverQuestクラスのSetGameStateメンバのDetour関数。
// ゲームの状態を設定するために呼び出される。
// ゲームの状態を最新の状態に更新し、Trampoline関数を呼び出す。
void eqgame_t::CEverQuest_SetGameState_detour(
	int game_state // ゲームの状態。
) {
	refreshGameState(game_state);
	(this->*CEverQuest_SetGameState_target)(game_state);
}

// CXWndManagerクラスのDrawCursorメンバのDetour関数。
// マウスカーソルを描画するために呼び出される。
// ただしマウスカーソルがチャットバーの上にあるときは描画しない。
// エラーが発生したらjchat.dllの機能を停止する。
int // おそらく0なら成功、それ以外なら失敗。
eqgame_t::CXWndManager_DrawCursor_detour() {
	try {
		if (cursor_isOverWindow(context.jchat_bar.getHandle())) return 0;
	} catch (const error &err) {
		context.fail(err);
	}
	return (this->*CXWndManager_DrawCursor_target)();
}

// KeypressHandlerクラスのHandleKeyDownメンバのDetour関数。
// メインウインドウでキーが押されたときに呼び出される。
// Enterキーなどが押されたら、Trampoline関数を呼び出さずにイベントを無視する。
// エラーが発生したらjchat.dllの機能を停止する。
bool // おそらく真なら処理済み、偽なら未処理。
eqgame_t::KeypressHandler_HandleKeyDown_detour(
	const KeyCombo &key_combo // 押されたキーの組み合わせ。
) {
	if (IGNORE_KEY_SITUATIONS.count(makeKeySituation(
			*NotInChatMode, 
			key_combo
		))
	) return true;
	return (this->*KeypressHandler_HandleKeyDown_target)(key_combo);
}

// KeypressHandlerクラスのHandleKeyUpメンバのDetour関数。
// メインウインドウでキーが離されたときに呼び出される。
// 非EQチャットモードでEnterキーなどが離されたら、
// Trampoline関数を呼び出さずに、チャットバーをアクティブにする。
// またEQチャットモードでEnterキーが離されたら、
// それはコマンドを実行しようとしていることになるので
// (コマンドは横取りされて日本語チャットに転送される)、
// その場合もチャットバーをアクティブにする。
// エラーが発生したらjchat.dllの機能を停止する。
bool // おそらく真なら処理済み、偽なら未処理。
eqgame_t::KeypressHandler_HandleKeyUp_detour(
	const KeyCombo &key_combo // 離されたキーの組み合わせ。
) {
	try {
		if (ACTIVATE_KEY_SITUATIONS.count(makeKeySituation(
				*NotInChatMode, 
				key_combo
			))
		) {
			context.jchat_bar.activate(key_combo);
			return true;
		}
	} catch (const error &err) {
		context.fail(err);
	}
	return (this->*KeypressHandler_HandleKeyUp_target)(key_combo);
}

// ProcessGameEventsのDetour関数。
// 自PCがスポーン中に短い間隔で(おそらくフレーム毎に)呼び出される。
// 各情報を最新の状態に更新し、Trampoline関数を呼び出す。
BOOL // 不明。
eqgame_t::ProcessGameEvents_detour() {
	refreshScreenMode();
	refreshGameState(PEVERQUEST(*CEverQuest)->GameState);
	return ProcessGameEvents_target();
}

// 日本語チャットバーのクラスを登録する。
// ウインドウを作成する前に1回だけ呼び出すこと。
void jchat_bar_t::registerClass() {
	WNDCLASS class_ = {};
	// 幅を変更されたら再描画し、閉じるボタンは必要ない。
	class_.style = CS_HREDRAW | CS_NOCLOSE;
	class_.lpfnWndProc = &WindowProc;
	class_.hInstance = context.dll_handle;
	// クライアント領域ではEQの矢印カーソルを使う。
	class_.hCursor = api::LoadCursor(
		context.dll_handle, 
		MAKEINTRESOURCE(IDC_EQ_ARROW)
	);
	class_.lpszClassName = JCHAT_BAR_CLASS_NAME;
	api::RegisterClass(&class_);
	context.exits.emplace_back([] {
		api::UnregisterClass(JCHAT_BAR_CLASS_NAME, context.dll_handle);
	});
}

// 日本語チャットバーをアクティブにする。
// きっかけがスラッシュキーならテキストを"/"にして、続けて入力できる
// ようにキャレットを移動してから、ウインドウをアクティブにする。
// それ以外なら入力欄を変更せず、ただアクティブにする。
void jchat_bar_t::activate(
	const KeyCombo &key_combo // きっかけになったキーの組み合わせ。
) {
	if (key_combo.key == DIK_SLASH) {
		edit_setLine("/");
		edit_selectText(1, 1);
	}
	api::SetActiveWindow(data->handle);
}

// 日本語チャットバーを作成する。
// jchat.iniから読み込んだ位置と幅で初期化する。
// ウインドウを作成した直後は非表示状態になっている。
void jchat_bar_t::create() {
	// メインウインドウのクライアント座標系における位置を初期化して、
	// スクリーン座標に変換し、チャットバーの初期位置として指定する。
	data->mw_client_pos = POINT{
		context.ini.window.left, 
		context.ini.window.top
	};
	POINT screen_pos = point_clientToScreen(
		data->mw_client_pos, 
		*eqgame_t::MainWindowHandle
	);
	data->handle = api::CreateWindowEx(
		0,
		JCHAT_BAR_CLASS_NAME, 
		JCHAT_BAR_NAME, 
		WS_POPUP, 
		screen_pos.x, 
		screen_pos.y, 
		context.ini.window.width, 
		JCHAT_BAR_HEIGHT, 
		*eqgame_t::MainWindowHandle,
		NULL,
		context.dll_handle,
		NULL
	);
}

// 日本語チャットバーの位置ズレを修正する。
// 位置ズレは画面モードの変更によって生じる。
// またチャットバーがメインウインドウの外に位置しているときに、
// フルスクリーンモードに変更されると、チャットバーが画面外に
// 出て見えなくなってしまうため、画面の中心に移動する。
void jchat_bar_t::fixPos() {
	// アイコンモードなら何もしない。
	if (context.screen_mode == screen_modes::ICONIC) return;
	// 保存しておいたメインウインドウのクライアント座標系に
	// おけるチャットバーの位置をスクリーン座標に変換する。
	POINT pos = point_clientToScreen(
		data->mw_client_pos, 
		*eqgame_t::MainWindowHandle
	);
	// アイコンと画面が重複していないなら画面の中心に移動する。
	RECT icon_rect = RECT{
		pos.x, 
		pos.y, 
		pos.x + int(ICON_WIDTH),
		pos.y + int(JCHAT_BAR_HEIGHT)
	};
	RECT screen_rect = RECT{
		0, 
		0, 
		api::GetSystemMetrics(SM_CXSCREEN), 
		api::GetSystemMetrics(SM_CYSCREEN)
	};
	if (!rect_areOverlapped(icon_rect, screen_rect)) {
		RECT bar_rect = window_getRect(data->handle);
		int bar_width_half = (bar_rect.right - bar_rect.left) >> 1;
		int bar_height_half = JCHAT_BAR_HEIGHT >> 1;
		int center_x = screen_rect.right >> 1;
		int center_y = screen_rect.bottom >> 1;
		pos.x = std::max(center_x - bar_width_half, 0);
		pos.y = center_y - bar_height_half;
	}
	// チャットバーの位置を設定する。
	api::SetWindowPos(
		data->handle,
		NULL,
		pos.x,
		pos.y,
		0, 
		0, 
		SWP_NOACTIVATE | SWP_NOSIZE
	);
}

// チャットバーのハンドルを取得する。
// ウインドウの作成前はNULLを返す。
// ウインドウの作成後は有効なハンドルを返すようになる。
// ウインドウがWM_DESTROYを受信すると、再びNULLを返すようになる。
HWND // 取得したハンドル。
jchat_bar_t::getHandle() {
	return data->handle;
}

// EQチャットから日本語チャットにテキストを転送する。
// 入力欄でテキストが選択されていれば、それを置き換える。
// またアイテムリンクは独自の形式に変換してから表示する。
void jchat_bar_t::transferText(
	const char *eqc_text // EQチャットのテキスト。
) {
	edit_replaceText(text_eqChatToJChat(eqc_text));
}

// 日本語チャットバーのインスタンスデータを破棄する。
// 作成したデバイスコンテキストなどはここで削除される。
jchat_bar_t::data_t::~data_t() {
	reverseClear(exits);
}

// 仮想キーコード→チャンネルのインデックスのマップ。
const std::unordered_map<BYTE,int> jchat_bar_t::CHANNEL_INDICES = {
	{'A', 0},
	{'G', 1},
	{'U', 2},
	{'O', 3},
	{'R', 4},
	{'S', 5},
	{'H', 6},
};

// 日本語チャットバーのチャンネル欄のメッセージマップ。
const jchat_bar_t::message_map_t jchat_bar_t::CHANNEL_MESSAGE_MAP = {
	{WM_CHAR      , &jchat_bar_t::channel_onChar      },
	{WM_SYSKEYDOWN, &jchat_bar_t::channel_onSysKeyDown},
};

// チャンネルのテキストの配列。
const std::array<std::string,7> jchat_bar_t::CHANNEL_TEXTS = {
	"/Auction",
	"/Gsay",
	"/gUildsay",
	"/Ooc",
	"/Reply",
	"/Say",
	"/sHout",
};

// 日本語チャットバーの入力欄のメッセージマップ。
const jchat_bar_t::message_map_t jchat_bar_t::EDIT_MESSAGE_MAP = {
	{WM_CHAR      , &jchat_bar_t::edit_onChar      },
	{WM_KEYDOWN   , &jchat_bar_t::edit_onKeyDown   },
	{WM_SYSKEYDOWN, &jchat_bar_t::edit_onSysKeyDown},
};

// 日本語チャットバーのメッセージマップ。
const jchat_bar_t::message_map_t jchat_bar_t::MESSAGE_MAP = {
	{WM_ACTIVATE       , &jchat_bar_t::onActivate       },
	{WM_COMMAND        , &jchat_bar_t::onCommand        },
	{WM_CREATE         , &jchat_bar_t::onCreate         },
	{WM_CTLCOLOREDIT   , &jchat_bar_t::onCtlColorEdit   },
	{WM_CTLCOLORLISTBOX, &jchat_bar_t::onCtlColorListBox},
	{WM_DESTROY        , &jchat_bar_t::onDestroy        },
	{WM_GETMINMAXINFO  , &jchat_bar_t::onGetMinMaxInfo  },
	{WM_MOVE           , &jchat_bar_t::onMove           },
	{WM_NCHITTEST      , &jchat_bar_t::onNCHitTest      },
	{WM_NCLBUTTONDBLCLK, &jchat_bar_t::onNCLButtonDblClk},
	{WM_PAINT          , &jchat_bar_t::onPaint          },
	{WM_SETCURSOR      , &jchat_bar_t::onSetCursor      },
	{WM_SETFOCUS       , &jchat_bar_t::onSetFocus       },
	{WM_SIZE           , &jchat_bar_t::onSize           },
};

// 日本語チャットバーのウインドウプロシージャ。
// ウインドウでイベントが発生するとWindowsから呼び出される。
// メッセージマップからハンドラを探し、見つかればそれを呼び出す。
// 見つからなければ既定のウインドウプロシージャに委譲する。
// エラーが発生したらjchat.dllの機能を停止する。
LRESULT CALLBACK jchat_bar_t::WindowProc(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg, 	   // ウインドウメッセージ。
	WPARAM wparam, // ワードパラメータ。
	LPARAM lparam  // ロングパラメータ。
) {
	LRESULT res = 0;
	auto &wnd = context.jchat_bar;
	auto handler = MESSAGE_MAP.find(msg);
	if (handler == MESSAGE_MAP.end())
		res = api::DefWindowProc(hwnd, msg, wparam, lparam);
	else {
		try {
			res = (wnd.*handler->second)(hwnd, msg, wparam, lparam);
		} catch (const error &err) {
			// WM_CREATEはCreateWindowEx関数の中から送信されるので、
			// ここではfail関数を呼び出さず、ログを書き込むだけにする。
			if (msg == WM_CREATE) {
				putLog(*context.log, err.getMessage());
				res = -1;
			} else context.fail(err);
		}
	} 
	return res;
}

// 日本語チャットバーのチャンネル欄のサブクラス化後のウインドウプロシージャ。
// チャンネル欄でイベントが発生するとWindowsから呼び出される。
// メッセージマップからハンドラを探し、見つかればそれを呼び出す。
// 見つからなければサブクラス化前のウインドウプロシージャに委譲する。
// エラーが発生したらjchat.dllの機能を停止する。
LRESULT // 結果。
CALLBACK jchat_bar_t::channel_WindowProc_sub(
	HWND hwnd,     // チャンネル欄のハンドル。
	UINT msg,      // ウインドウメッセージ。
	WPARAM wparam, // ワードパラメータ。
	LPARAM lparam  // ロングパラメータ。
) {
	LRESULT res = 0;
	auto &wnd = context.jchat_bar;
	auto handler = CHANNEL_MESSAGE_MAP.find(msg);
	if (handler == CHANNEL_MESSAGE_MAP.end())
		res = api::CallWindowProc(
			wnd.data->channel.WindowProc_super, 
			hwnd,
			msg, 
			wparam, 
			lparam
		);
	else {
		try {
			res = (wnd.*handler->second)(hwnd, msg, wparam, lparam);
		} catch (const error &err) {
			context.fail(err);
		}
	}
	return res;
}

// 日本語チャットバーの入力欄のサブクラス化後のウインドウプロシージャ。
// 入力欄でイベントが発生するとWindowsから呼び出される。
// メッセージマップからハンドラを探し、見つかればそれを呼び出す。
// 見つからなければサブクラス化前のウインドウプロシージャに委譲する。
// エラーが発生したらjchat.dllの機能を停止する。
LRESULT // 結果。
CALLBACK jchat_bar_t::edit_WindowProc_sub(
	HWND hwnd,     // 入力欄のハンドル。
	UINT msg,      // ウインドウメッセージ。
	WPARAM wparam, // ワードパラメータ。
	LPARAM lparam  // ロングパラメータ。
) {
	LRESULT res = 0;
	auto &wnd = context.jchat_bar;
	auto handler = EDIT_MESSAGE_MAP.find(msg);
	if (handler == EDIT_MESSAGE_MAP.end())
		res = api::CallWindowProc(
			wnd.data->edit.WindowProc_super, 
			hwnd,
			msg, 
			wparam, 
			lparam
		);
	else {
		try {
			res = (wnd.*handler->second)(hwnd, msg, wparam, lparam);
		} catch (const error &err) {
			context.fail(err);
		}
	}
	return res;
}

// 日本語チャットのチャンネル欄で文字が入力されたときに呼び出される。
// Enterキーが押されたら、入力行を実行し、今までの発言リストに追加する。
// Shift+Enterキーが押されたら、入力行をよく使う発言リストに追加する。
// Escキーが押されたらメインウインドウをアクティブにする。
// Tabキーが押されたらフォーカスを移動する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::channel_onChar(
	HWND hwnd,     // チャンネル欄のハンドル。
	UINT msg,      // WM_CHARメッセージ。
	WPARAM wparam, // 仮想キーコード。
	LPARAM lparam  // 使用しない。
) {
	return ctl_onChar(
		data->channel.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// 日本語チャットのチャンネル欄でシステムキーが押されたときに呼び出される。
// チャンネルのショートカットキーが押されたら、そのチャンネルを選択する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::channel_onSysKeyDown(
	HWND hwnd,     // チャンネル欄のハンドル。
	UINT msg,      // WM_SYSKEYDOWNメッセージ。
	WPARAM wparam, // 仮想キーコード。
	LPARAM lparam  // 使用しない。
) {
	return ctl_onSysKeyDown(
		data->channel.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// 日本語チャットのコントロールで文字が入力されたときに呼び出される。
// Enterキーが押されたら、入力行を実行し、今までの発言リストに追加する。
// Shift+Enterキーが押されたら、入力行をよく使う発言リストに追加する。
// Escキーが押されたらメインウインドウをアクティブにする。
// Tabキーが押されたらフォーカスを移動する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::ctl_onChar(
	WNDPROC super_proc, // サブクラス化前のウインドウプロシージャ。
	HWND hwnd,          // コントロールのハンドル。
	UINT msg,           // WM_CHARメッセージ。
	WPARAM wparam,      // 仮想キーコード。
	LPARAM lparam       // 使用しない。
) {
	SHORT shift_state = api::GetKeyState(VK_SHIFT);
	if (wparam == VK_RETURN) {
		execute(shift_state);
		return 0;
	} else if (wparam == VK_ESCAPE) {
		api::SetActiveWindow(*eqgame_t::MainWindowHandle);
		return 0;
	} else if (wparam == VK_TAB) {
		moveFocus(hwnd, shift_state);
		return 0;
	}
	return api::CallWindowProc(
		super_proc, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// 日本語チャットのコントロールでシステムキーが押されたときに呼び出される。
// チャンネルのショートカットキーが押されたら、そのチャンネルを選択する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::ctl_onSysKeyDown(
	WNDPROC super_proc, // サブクラス化前のウインドウプロシージャ。
	HWND hwnd,          // コントロールのハンドル。
	UINT msg,           // WM_SYSKEYDOWNメッセージ。
	WPARAM wparam,      // 仮想キーコード。
	LPARAM lparam       // 使用しない。
) {
	int ch_index;
	if ((ch_index = map_find(CHANNEL_INDICES, wparam, -1)) >= 0) {
		channel_select(ch_index);
		return 0;
	}
	return api::CallWindowProc(
		super_proc, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// 日本語チャットの入力欄で文字が入力されたときに呼び出される。
// Ctrl+Aキーが押されたら全テキストを選択する。
// Enterキーが押されたら、入力行を実行し、今までの発言リストに追加する。
// Shift+Enterキーが押されたら、入力行をよく使う発言リストに追加する。
// Escキーが押されたらメインウインドウをアクティブにする。
// Tabキーが押されたらフォーカスを移動する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::edit_onChar(
	HWND hwnd,     // 入力欄のハンドル。
	UINT msg,      // WM_CHARメッセージ。
	WPARAM wparam, // 仮想キーコード。
	LPARAM lparam  // 使用しない。
) {
	if (wparam == getCtrlAlphabetKey('A')) {
		edit_selectText();
		return 0;
	}
	return ctl_onChar(
		data->edit.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// 日本語チャットの入力欄でキーが押されたときに呼び出される。
// ↑キーや↓キーが押されたら今までの発言リストから選択する。
// Shift+↑キーやShift+↓キーが押されたら、よく使う発言リストから選択する。
// Shift+Deleteキーが押されたら、リストから選択行を削除する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::edit_onKeyDown(
	HWND hwnd,     // 入力欄のハンドル。
	UINT msg,      // WM_KEYDOWNメッセージ。
	WPARAM wparam, // 仮想キーコード。
	LPARAM lparam  // 使用しない。
) {
	SHORT shift_state = api::GetKeyState(VK_SHIFT);
	if (wparam == VK_UP || wparam == VK_DOWN) {
		moveThroughList(shift_state, wparam);
		return 0;
	} else if (shift_state < 0 && wparam == VK_DELETE) {
		deleteFromList();
		return 0;
	}
	return api::CallWindowProc(
		data->edit.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// 日本語チャットの入力欄でシステムキーが押されたときに呼び出される。
// チャンネルのショートカットキーが押されたら、そのチャンネルを選択する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::edit_onSysKeyDown(
	HWND hwnd,     // 入力欄のハンドル。
	UINT msg,      // WM_SYSKEYDOWNメッセージ。
	WPARAM wparam, // 仮想キーコード。
	LPARAM lparam  // 使用しない。
) {
	int ch_index;
	if ((ch_index = map_find(CHANNEL_INDICES, wparam, -1)) >= 0) {
		channel_select(ch_index);
		return 0;
	}
	return api::CallWindowProc(
		data->edit.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// 日本語チャットバーがアクティブ、またはインアクティブに
// なったときに呼び出される。アイコンを再描画する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::onActivate(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_ACTIVATEメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // 使用しない。
) {
	api::InvalidateRect(data->handle, NULL, TRUE);
	return 0;
}

// 日本語チャットバーへコマンドが送信されたときに呼び出される。
// 入力欄からテキストの変更を通知されたら、更新フラグをセットし、
// 今までの発言リストとよく使う発言リストの反復子をリセットする。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::onCommand(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_COMMANDメッセージ。
	WPARAM wparam, // コントロールのIDと通知コード。
	LPARAM lparam  // 使用しない。
) {
	// 更新とみなすのは編集で変更する場合だけで、
	// リストから選択する場合は更新とみなさない。
	if (LOWORD(wparam) == EDIT_ID &&
		HIWORD(wparam) == EN_CHANGE && 
		data->edit.change_mode == change_modes::EDIT
	) {
		data->edit.dirty = true;
		data->history.moveEnd();
		data->registry.moveEnd();
	}
	return 0;
}

// 日本語チャットバーを作成しているときに呼び出される。
// 入力欄をセットアップし、背景のグラフィックを準備し、
// jchat.iniからよく使う発言リストを読み込む。
// 失敗したときは、エラーをスローすることでそれを示す。
// エラーはWindowProcでキャッチされ、最終的に-1を返す。
LRESULT // 結果。0なら成功、-1なら失敗。
jchat_bar_t::onCreate(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_CREATEメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // 使用しない。
) {
	prepare(hwnd);
	channel_create(hwnd);
	edit_create(hwnd);
	loadRegistry();
	return 0;
}

// 日本語チャットバーの入力欄とチャンネルが描画されるときに呼び出される。
LRESULT // 背景を塗りつぶすためのブラシのハンドル。
jchat_bar_t::onCtlColorEdit(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_CTLCOLOREDITメッセージ。
	WPARAM wparam, // コントロールのデバイスコンテキストのハンドル。 
	LPARAM lparam  // コントロールのハンドル。
) {
	HDC ctl_dc_handle = HDC(wparam);
	HWND ctl_handle = HWND(lparam);
	api::SetTextColor(ctl_dc_handle, TEXT_COLOR);
	if (ctl_handle == data->channel.handle) {
		// チャンネル欄のボックス部分は背景のブラシを指定しても
		// 無視されるため、不透明モードと背景色を設定して塗りつぶす。
		api::SetBkMode(ctl_dc_handle, OPAQUE);
		api::SetBkColor(ctl_dc_handle, CHANNEL_BK_COLOR);
	} else if (ctl_handle == data->edit.handle) {
		// 入力欄の背景は岩棒ブラシで塗りつぶすので透明モードを設定する。
		api::SetBkMode(ctl_dc_handle, TRANSPARENT);
		// 原点としてチャットバーの左上隅に対する相対座標を設定する。
		// こうすることでチャットバーの背景とシームレスになる。
		api::SetBrushOrgEx(
			ctl_dc_handle, 
			-(EDIT_LEFT + 2), 
			-(EDIT_TOP + 4), 
			NULL
		);
	}
	return LRESULT(data->rock_bar_brush_handle);
}

// 日本語チャットバーのチャンネル欄のリスト部分が描画されるときに
// 呼び出される。チャンネル欄の背景は岩壁ブラシで塗りつぶす。
LRESULT // 背景を塗りつぶすためのブラシのハンドル。
jchat_bar_t::onCtlColorListBox(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_CTLCOLORLISTBOXメッセージ。
	WPARAM wparam, // コントロールのデバイスコンテキストのハンドル。 
	LPARAM lparam  // コントロールのハンドル。
) {
	HDC ctl_dc_handle = HDC(wparam);
	api::SetTextColor(ctl_dc_handle, TEXT_COLOR);
	api::SetBkMode(ctl_dc_handle, TRANSPARENT);
	return LRESULT(data->rock_wall_brush_handle);
}

// 日本語チャットバーが破棄されるときに呼び出される。
// jchat.iniにウインドウの最後の位置と幅、そしてよく使う
// 発言リストを保存して、インスタンスデータをリセットする。
// エラーが発生してもスローせず、終了処理を継続する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::onDestroy(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_DESTROYメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // 使用しない。
) {
	try {
		RECT rect = window_getRect(hwnd);
		ini_setKeyValue(
			context.ini_path,
			"Window",
			"Left",
			stringize(data->mw_client_pos.x)
		);
		ini_setKeyValue(
			context.ini_path,
			"Window",
			"Top",
			stringize(data->mw_client_pos.y)
		);
		ini_setKeyValue(
			context.ini_path,
			"Window",
			"Width",
			stringize(rect.right - rect.left)
		);
		storeRegistry();
	} catch (const error &err) {
		putLog(*context.log, err.getMessage());
	}
	data = std::make_shared<data_t>();
	return 0;
}

// 日本語チャットバーがドラッグされているときに、
// 最小サイズと最大サイズを取得するために呼び出される。
// 問題なのは短くされ過ぎることだけなので最小幅を指定する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::onGetMinMaxInfo(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_GETMINMAXINFOメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // MINMAXINFOのポインタ。
) {
	PMINMAXINFO info = PMINMAXINFO(lparam);
	info->ptMinTrackSize.x = context.ini.window.min_width;
	return 0;
}

// 日本語チャットバーが移動されたときに呼び出される。
// メインウインドウのクライアント座標系における位置を更新する。
LRESULT // 領域の種類。
jchat_bar_t::onMove(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_MOVEメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // X座標とY座標。
) {
	data->mw_client_pos = point_screenToClient(
		POINT{LOWORD(lparam), HIWORD(lparam)}, 
		*eqgame_t::MainWindowHandle
	);
	return 0;
}

// 日本語チャットバーの上にマウスカーソルがあるときに、
// そこがどういう領域なのかを取得するために呼び出される。
// アイコンなら移動できるようにキャプション、つまみなら
// 幅を変更できるように右辺、それ以外ならクライアント領域を返す。
LRESULT // 領域の種類。
jchat_bar_t::onNCHitTest(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_NCHITTESTメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // マウスカーソルのX座標とY座標。
) {
	POINT cursor_pos{short(LOWORD(lparam)), short(HIWORD(lparam))};
	RECT rect = window_getRect(hwnd);
	LONG edit_left = rect.left + ICON_WIDTH;
	if (cursor_pos.x < edit_left) return HTCAPTION;
	LONG edit_right = rect.right - KNOB_WIDTH;
	if (cursor_pos.x >= edit_right) return HTRIGHT;
	return HTCLIENT;
}

// 日本語チャットバーの非クライアント領域を左ダブルクリック
// されたときに呼び出される。チャットバーを最小幅まで縮める。
LRESULT // 領域の種類。
jchat_bar_t::onNCLButtonDblClk(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_NCLBUTTONDBLCLKメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // 使用しない。
) {
	// チャットバーの幅を設定する。
	api::SetWindowPos(
		data->handle,
		NULL,
		0,
		0,
		context.ini.window.min_width, 
		JCHAT_BAR_HEIGHT, 
		SWP_NOMOVE
	);
	return HTCLIENT;
}

// 日本語チャットバーが描画されるときに呼び出される。
// ダブルバッファリング方式で背景を描画する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::onPaint(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_PAINTメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // 使用しない。
) {
	PAINTSTRUCT paint;
	HDC hdc = api::BeginPaint(hwnd, &paint);
	scope_exit_t paint_exit([hwnd, &paint] {
		api::EndPaint(hwnd, &paint);
	});
	RECT rect = window_getClientRect(hwnd);
	// 岩棒ブラシで塗りつぶす。
	api::FillRect(data->buffer_dc_handle, &rect, data->rock_bar_brush_handle);
	// チャットバーがアクティブかどうかに応じてアイコンを切り替える。
	HDC icon_dc_handle;
	if (api::GetActiveWindow() == context.jchat_bar.getHandle())
		icon_dc_handle = data->icon_active_dc_handle;
	else icon_dc_handle = data->icon_inactive_dc_handle;
	// 左端にアイコンを貼り付ける。
	api::BitBlt(
		data->buffer_dc_handle,
		0,
		0,
		JCHAT_BAR_HEIGHT,
		JCHAT_BAR_HEIGHT,
		icon_dc_handle,
		0,
		0,
		SRCCOPY
	);
	// 右端につまみを貼り付ける。
	api::BitBlt(
		data->buffer_dc_handle,
		rect.right - KNOB_WIDTH,
		0,
		KNOB_WIDTH,
		JCHAT_BAR_HEIGHT,
		data->knob_dc_handle,
		0,
		0,
		SRCCOPY
	);
	// 枠線を引く。
	drawEdge(
		data->buffer_dc_handle,
		0,
		0,
		rect.right, 
		rect.bottom,
		JCHAT_BAR_RAISED_COLOR,
		JCHAT_BAR_SUNKEN_COLOR
	);
	// 入力欄の外側の枠線を引く。
	drawEdge(
		data->buffer_dc_handle,
		EDIT_LEFT,
		EDIT_TOP,
		rect.right - EDIT_HOR_MARGIN, 
		rect.bottom - EDIT_TOP * 2,
		EDIT_OUTER_RAISED_COLOR,
		EDIT_OUTER_SUNKEN_COLOR
	);
	// 入力欄の内側の枠線を引く。
	drawEdge(
		data->buffer_dc_handle,
		EDIT_LEFT + 1,
		EDIT_TOP + 1,
		rect.right - (EDIT_HOR_MARGIN + 2), 
		rect.bottom - (EDIT_TOP + 1) * 2,
		EDIT_INNER_RAISED_COLOR,
		EDIT_INNER_SUNKEN_COLOR
	);
	// 結果をウインドウにコピーする。
	api::BitBlt(
		hdc, 
		paint.rcPaint.left, 
		paint.rcPaint.top, 
		paint.rcPaint.right, 
		paint.rcPaint.bottom, 
		data->buffer_dc_handle, 
		0, 
		0, 
		SRCCOPY
	);
	return 0;
}

// 日本語チャットバーの上にマウスカーソルがあるときに、
// マウスカーソルの形状を設定するために呼び出される。
// アイコンの上にあるときはEQの4方向矢印カーソルを設定し、
// つまみの上にあるときはEQの左右矢印カーソルを設定する。
LRESULT // 真なら処理済み、偽なら未処理。
jchat_bar_t::onSetCursor(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_SETCURSORメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // 領域の種類。
) {
	if (LOWORD(lparam) == HTCAPTION) {
		api::SetCursor(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_MOVE)
		));
		return TRUE;
	} else if (LOWORD(lparam) == HTRIGHT) {
		api::SetCursor(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_EW)
		));
		return TRUE;
	}
	return api::DefWindowProc(hwnd, msg, wparam, lparam);
}

// 日本語チャットバーにフォーカスが設定されたときに呼び出される。
// すぐにキーボードから入力できるように入力欄にフォーカスを設定する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::onSetFocus(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_SETFOCUSメッセージ。
	WPARAM wparam, // 使用しない。
	LPARAM lparam  // 使用しない。
) {
	api::SetFocus(data->edit.handle);
	return 0;
}

// 日本語チャットバーのサイズが変更されたときなどに呼び出される。
// 変更後のウインドウのサイズに合わせて、入力欄のサイズを変更する。
LRESULT // 結果。0なら処理済み。
jchat_bar_t::onSize(
	HWND hwnd,     // チャットバーのハンドル。
	UINT msg,      // WM_SIZEメッセージ。
	WPARAM wparam, // サイズ変更の種類。
	LPARAM lparam  // ウインドウの幅と高さ。
) {
	if (wparam == SIZE_RESTORED && data->edit.handle)
		api::MoveWindow(
			data->edit.handle, 
			EDIT_LEFT + 2, 
			EDIT_TOP + 4, 
			LOWORD(lparam) - (EDIT_HOR_MARGIN + 2 + 2), 
			EDIT_HEIGHT, 
			TRUE
		);
	return 0;
}

// 日本語チャットのチャンネル欄を作成する。
void jchat_bar_t::channel_create(
	HWND hwnd // チャットバーのハンドル。
) {
	// チャットバーを親としてCOMBOBOXコントロールを作成する。
	data->channel.handle = api::CreateWindowEx(
		0,
		"COMBOBOX", 
		NULL, 
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 
		ICON_WIDTH, 
		CHANNEL_TOP, 
		CHANNEL_WIDTH, 
		CHANNEL_HEIGHT, 
		hwnd, 
		HMENU(CHANNEL_ID), 
		context.dll_handle, 
		NULL
	);
	// 作成したコントロールをサブクラス化し、
	// ウインドウプロシージャをカスタマイズする。
	data->channel.WindowProc_super = WNDPROC(api::SetWindowLongPtr(
		data->channel.handle, 
		GWLP_WNDPROC, 
		LONG_PTR(&channel_WindowProc_sub)
	));
	// クライアント領域のマウスカーソルをEQの矢印カーソルにする。
	api::SetClassLong(
		data->channel.handle, 
		GCL_HCURSOR,
		LONG(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_ARROW)
		))
	);
	// フォントをコントロールに設定する。
	api::SendMessage(
		data->channel.handle, 
		WM_SETFONT, 
		WPARAM(data->hfont), 
		0
	);
	// テキストを追加する。
	for (const std::string &CHANNEL_TEXT : CHANNEL_TEXTS) 
		api::SendMessage(
			data->channel.handle, 
			CB_ADDSTRING, 
			0, 
			LPARAM(CHANNEL_TEXT.data())
		);
	// "/Say"を選択する。
	channel_select(map_find(CHANNEL_INDICES, 'S'));
	// Tab順のハンドルとインデックスを追加する。
	data->ordered_handles.emplace_back(data->channel.handle);
	data->ordered_indices.emplace(
		data->channel.handle, 
		data->ordered_indices.size()
	);
}

// 日本語チャットのチャンネルを選択する。
void jchat_bar_t::channel_select(
	int ch_index // 選択するチャンネルのインデックス。
) {
	api::SendMessage(data->channel.handle, CB_SETCURSEL, ch_index, 0);
}

// 今までの発言リスト、またはよく使う発言リストから行を削除する。
// リストの反復子が指し示す行、つまり選択中の行を削除する。
// 選択中の行を削除したら、今度はその一つ後ろの行を選択する。
void jchat_bar_t::deleteFromList() {
	// 行を選択中のリストがあるかどうかを調べる。
	lines_and_iter_t *lni = nullptr;
	if (!data->history.isEnd()) lni = &data->history;
	else if (!data->registry.isEnd()) lni = &data->registry;
	if (lni) {
		// 選択中の行を削除し、一つ後ろの行を選択する。
		lni->iter = lni->lines.erase(lni->iter);
		// 一つ後ろの行があればそれを取得し、
		// なければ最後に編集していた行を取得する。
		std::string *line;
		if (lni->isEnd()) line = &data->edit.last_line;
		else line = &*lni->iter;
		// 入力欄を選択モードで変更し、全テキストを選択する。
		edit_setLine(*line, change_modes::CHOICE);
		edit_selectText();
	} else edit_setLine(); // 選択中のリストがなければ入力欄をクリアする。
}

// デバイスコンテキストに枠線を描画する。
void jchat_bar_t::drawEdge(
	HDC hdc,               // デバイスコンテキストのハンドル。
	int left,              // 左辺のX座標。
	int top,               // 上辺のY座標。
	int width,             // 幅。
	int height,            // 高さ。
	COLORREF raised_color, // 左辺・上辺の色。
	COLORREF sunken_color  // 右辺・下辺の色。
) {
	int right = left + width - 1;
	int bottom = top + height - 1;
	POINT points[3];
	// 左辺・上辺を描画する。
	// 指定色のペンを選択して、3点のポリラインを描く。
	api::SelectObject(hdc, getPen(raised_color));
	points[0] = {left     , bottom};
	points[1] = {left     , top   };
	points[2] = {right + 1, top   };
	api::Polyline(hdc, points, 3);
	// 右辺・下辺を描画する。
	// 指定色のペンを選択して、3点のポリラインを描く。
	api::SelectObject(hdc, getPen(sunken_color));
	points[0] = {right, top + 1};
	points[1] = {right, bottom };
	points[2] = {left , bottom };
	api::Polyline(hdc, points, 3);
}

// 日本語チャットの入力欄を作成する。
void jchat_bar_t::edit_create(
	HWND hwnd // チャットバーのハンドル。
) {
	// チャットバーを親としてEDITコントロールを作成する。
	data->edit.handle = api::CreateWindowEx(
		0,
		"EDIT", 
		NULL, 
		ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_VISIBLE, 
		EDIT_LEFT + 2, 
		EDIT_TOP + 4, 
		context.ini.window.width - (EDIT_HOR_MARGIN + 2 + 2), 
		EDIT_HEIGHT, 
		hwnd, 
		HMENU(EDIT_ID), 
		context.dll_handle, 
		NULL
	);
	// 作成したコントロールをサブクラス化し、
	// ウインドウプロシージャをカスタマイズする。
	data->edit.WindowProc_super = WNDPROC(api::SetWindowLongPtr(
		data->edit.handle, 
		GWLP_WNDPROC, 
		LONG_PTR(&edit_WindowProc_sub)
	));
	// クライアント領域のマウスカーソルをEQのIビームカーソルにする。
	api::SetClassLong(
		data->edit.handle, 
		GCL_HCURSOR,
		LONG(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_IBEAM)
		))
	);
	// フォントをコントロールに設定する。
	api::SendMessage(
		data->edit.handle, 
		WM_SETFONT, 
		WPARAM(data->hfont), 
		0
	);
	// Tab順のハンドルとインデックスを追加する。
	data->ordered_handles.emplace_back(data->edit.handle);
	data->ordered_indices.emplace(
		data->edit.handle, 
		data->ordered_indices.size()
	);
}

// 日本語チャットに入力された行を取得する。
std::string // 取得した行。
jchat_bar_t::edit_getLine() {
	return window_getText(data->edit.handle);
}

// 日本語チャットの入力欄で選択しているテキストを置き換える。
// 選択しているテキストがなければ、キャレットの位置に挿入する。
void jchat_bar_t::edit_replaceText(
	const std::string &text // 置き換える、または挿入するテキスト。
) {
	api::SendMessage(
		data->edit.handle, 
		EM_REPLACESEL, 
		TRUE, 
		LPARAM(text.data())
	);
}

// 日本語チャットの入力欄でテキストを選択する。
// 引数をすべて省略すると先頭から末尾までの全テキストを選択する。
void jchat_bar_t::edit_selectText(
	int first, // 選択を開始する位置。省略するとゼロ。
	int last   // 選択を終了する位置。-1なら末尾。省略すると-1。
) {
	api::SendMessage(data->edit.handle, EM_SETSEL, first, last);
}

// 日本語チャットの入力欄に行を設定する。
// 選択モードで、なおかつ更新フラグがセットされていれば、
// 設定前の行を「最後に編集されていた行」として保存する。
void jchat_bar_t::edit_setLine(
	const std::string &line, // 設定する行。省略すると空文字列。
	change_modes change_mode // 変更モード。省略するとEDIT。
) {
	data->edit.change_mode = change_mode;
	if (change_mode == change_modes::CHOICE && data->edit.dirty) {
		data->edit.last_line = window_getText(data->edit.handle);
		data->edit.dirty = false;
	}
	api::SetWindowText(data->edit.handle, line.c_str());
	data->edit.change_mode = change_modes::EDIT;
}

// 日本語チャットに入力された行を実行し、今までの発言リストに追加する。
// ただしShiftキーが押下なら、実行せず、よく使う発言リストに追加する。
void jchat_bar_t::execute(
	SHORT shift_state // Shiftキーの状態。正なら離上、負なら押下。
) {
	std::string line = edit_getLine();
	// 入力欄が空っぽなら何もしない。
	if (!line.empty()) {
		if (shift_state >= 0) {
			((*eqgame_t::CEverQuest)->*eqgame_t::CEverQuest_InterpretCmd_target)(
				*eqgame_t::CharSpawn, 
				text_jChatToEQChat(text_addChannel(line)).data()
			);
			data->history.lines.emplace_back(line);
		} else data->registry.lines.emplace_back(line);
		// 入力欄を空っぽにする。
		edit_setLine();
	}
	// Shiftキーが離上、つまり通常のコマンド実行なら、
	// メインウインドウをアクティブにし、EQの操作に戻る。
	if (shift_state >= 0) api::SetActiveWindow(*eqgame_t::MainWindowHandle);
}

// 指定された色のペンを取得する。
// まず指定された色でペンのキャッシュを探し、見つかればそれを返す。
// 見つからなければその色のペンを作成し、キャッシュに追加して返す。
HPEN // 取得したペンのハンドル。
jchat_bar_t::getPen(
	COLORREF color // ペンの色。
) {
	auto iter = data->pen_cache.find(color);
	if (iter != data->pen_cache.end()) return iter->second;
	HPEN hpen = api::CreatePen(PS_SOLID, 1, color);
	data->exits.emplace_back([hpen] {
		api::DeleteObject(hpen);
	});
	data->pen_cache.emplace(color, hpen);
	return hpen;
}

// jchat.iniからよく使う発言リストを読み込む。
// 行はRegistryセクションのLine_#キーに保存されている。
// ここで#には0から始まる連番が埋め込まれている。
void jchat_bar_t::loadRegistry() {
	// 0から始まる連番付きのキーから順番に読み込む。
	for (int i = 0;; ++i) {
		std::string line = ini_getKeyValue(
			context.ini_path, 
			"Registry", 
			string_printf("Line_%d", i)
		);
		// 取得したキーの値が空っぽなら、そこで読み込みを終了する。
		if (line.empty()) break;
		data->registry.lines.emplace_back(line);
	}
	// 反復子をリセットする。
	data->registry.moveEnd();
}

// 次のコントロールにフォーカスを移動する。
void jchat_bar_t::moveFocus(
	HWND ctl_handle,  // フォーカスを持つコントロールのハンドル。
	SHORT shift_state // Shiftキーの状態。正なら離上、負なら押下。
) {
	int size = data->ordered_handles.size();
	int index = data->ordered_indices[ctl_handle];
	if (shift_state >= 0) {
		if (++index >= size) index -= size;
	} else if (--index < 0) index += size;
	api::SetFocus(data->ordered_handles[index]);
}

// Shiftキーが押されていなければ今までの発言リスト、押されていれば
// よく使う発言リストの中を移動し、移動先の行を表示する。
// ↑キーなら一つ前に移動し、↓キーなら一つ後ろに移動する。
void jchat_bar_t::moveThroughList(
	SHORT shift_state, // Shiftキーの状態。正なら離上、負なら押下。
	BYTE vir_key       // 方向キーの仮想キーコード。
) {
	// Shiftキーが押されていなければ今までの発言リスト、
	// 押されていればよく使う発言リストの中を移動する。
	lines_and_iter_t *lni;
	if (shift_state >= 0) lni = &data->history;
	else lni = &data->registry;
	// 方向キーに応じて反復子を移動し、移動先の行を取得する。
	std::string *line = nullptr;
	if (vir_key == VK_UP) {
		if (!lni->isBegin()) line = &*--lni->iter;
	} else if (!lni->isEnd()) {
		++lni->iter;
		// 末尾に到達したら「最後に編集していた行」を取得する。
		if (lni->isEnd()) line = &data->edit.last_line;
		else line = &*lni->iter;
	}
	if (line) {
		// 入力欄を選択モードで変更し、全テキストを選択する。
		edit_setLine(*line, change_modes::CHOICE);
		edit_selectText();
		// 移動しないほうのリストの反復子をリセットする。
		if (shift_state >= 0) data->registry.moveEnd();
		else data->history.moveEnd();
	} else edit_selectText(); // 移動先がなければ全テキストを選択する。
}

// 日本語チャットバーを準備する。
// ダブルバッファリング方式で描画するためにビットマップ、
// パターンブラシ、デバイスコンテキストをロード・作成する。
// またコントロールで使用するフォントを作成する。
void jchat_bar_t::prepare(
	HWND hwnd // チャットバーのハンドル。
) {
	// 岩棒ブラシ用のビットマップをロードする。
	HBITMAP rock_bar_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ROCK_BAR)
	);
	data->exits.emplace_back([rock_bar_bmp_handle] {
		api::DeleteObject(rock_bar_bmp_handle);
	});
	// 岩棒ブラシ用のビットマップからパターンブラシを作成する。
	data->rock_bar_brush_handle = 
		api::CreatePatternBrush(rock_bar_bmp_handle);
	data->exits.emplace_back([this] {
		api::DeleteObject(data->rock_bar_brush_handle);
	});
	// 岩壁ブラシ用のビットマップをロードする。
	HBITMAP rock_wall_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ROCK_WALL)
	);
	data->exits.emplace_back([rock_wall_bmp_handle] {
		api::DeleteObject(rock_wall_bmp_handle);
	});
	// 岩壁ブラシ用のビットマップからパターンブラシを作成する。
	data->rock_wall_brush_handle = 
		api::CreatePatternBrush(rock_wall_bmp_handle);
	data->exits.emplace_back([this] {
		api::DeleteObject(data->rock_wall_brush_handle);
	});
	// ウインドウのデバイスコンテキストを取得する。
	HDC hdc = api::GetDC(hwnd);
	scope_exit_t hdc_exit([hwnd, hdc] {
		api::ReleaseDC(hwnd, hdc);
	});
	// アクティブアイコンのビットマップをロードする。
	HBITMAP icon_active_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ICON_ACTIVE)
	);
	data->exits.emplace_back([icon_active_bmp_handle] {
		api::DeleteObject(icon_active_bmp_handle);
	});
	// アクティブアイコンのデバイスコンテキストを作成する。
	data->icon_active_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->icon_active_dc_handle);
	});
	api::SelectObject(data->icon_active_dc_handle, icon_active_bmp_handle);
	// インアクティブアイコンのビットマップをロードする。
	HBITMAP icon_inactive_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ICON_INACTIVE)
	);
	data->exits.emplace_back([icon_inactive_bmp_handle] {
		api::DeleteObject(icon_inactive_bmp_handle);
	});
	// インアクティブアイコンのデバイスコンテキストを作成する。
	data->icon_inactive_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->icon_inactive_dc_handle);
	});
	api::SelectObject(data->icon_inactive_dc_handle, icon_inactive_bmp_handle);
	// つまみのビットマップをロードする。
	HBITMAP knob_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_KNOB)
	);
	data->exits.emplace_back([knob_bmp_handle] {
		api::DeleteObject(knob_bmp_handle);
	});
	// つまみのデバイスコンテキストを作成する。
	data->knob_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->knob_dc_handle);
	});
	api::SelectObject(data->knob_dc_handle, knob_bmp_handle);
	// バッファのビットマップを作成する。
	HBITMAP buffer_bmp_handle = api::CreateCompatibleBitmap(
		hdc, 
		api::GetSystemMetrics(SM_CXSCREEN), 
		JCHAT_BAR_HEIGHT
	);
	data->exits.emplace_back([buffer_bmp_handle] {
		api::DeleteObject(buffer_bmp_handle);
	});
	// バッファのデバイスコンテキストを作成する。
	data->buffer_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->buffer_dc_handle);
	});
	api::SelectObject(data->buffer_dc_handle, buffer_bmp_handle);
	// jchat.iniで設定されている名前でフォントを作成する。
	data->hfont = api::CreateFont(
		FONT_SIZE,
		0,
		0,
		0, 
		FW_NORMAL,
		FALSE, 
		FALSE,
		FALSE,
		SHIFTJIS_CHARSET, 
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY,
		DEFAULT_PITCH, 
		context.ini.chat.font_name.c_str()
	);
	data->exits.emplace_back([this] {
		api::DeleteObject(data->hfont);
	});
}

// jchat.iniによく使う発言リストを書き込む。
// 行はRegistryセクションのLine_#キーに保存する。
// ここで#には0から始まる連番を埋め込む。
void jchat_bar_t::storeRegistry() {
	// まず古いRegistryセクションを削除する。
	ini_deleteSection(context.ini_path, "Registry");
	// 0から始まる連番付きのキーに順番に書き込む。
	data->registry.moveBegin();
	for (std::size_t i = 0; !data->registry.isEnd(); ++i)
		ini_setKeyValue(
			context.ini_path, 
			"Registry", 
			string_printf("Line_%d", i), 
			*data->registry.iter++
		);
}

// 日本語チャットのテキストの前にチャンネルを付加する。
// ただしテキストがコマンドなら何もしない。
std::string // チャンネルを付加したテキスト。
jchat_bar_t::text_addChannel(
	const std::string &jc_text // 日本語チャットのテキスト。
) {
	// 最初の文字からコマンドであるかどうかを判定する。
	if (context.ini.chat.command_symbols.find(
			jc_text.front()
		) != std::string::npos
	) return jc_text;
	// 選択中のチャンネルをテキストの前に付加する。
	int ch_index = api::SendMessage(data->channel.handle, CB_GETCURSEL, 0, 0);
	return CHANNEL_TEXTS[ch_index] + " " + jc_text;
}

// EQチャットのテキストを日本語チャットのテキストに変換する。
// テキストにアイテムリンクが含まれていれば独自の形式に変換する。
// EQチャットからのテキストはAsciiなのでエンコーディングはそのままでよい。
std::string // 変換した日本語チャットのテキスト(Ascii)。
jchat_bar_t::text_eqChatToJChat(
	const char *eqc_text // 変換するEQチャットのテキスト(Ascii)。
) {
	std::ostringstream jc_text_out;
	// EQチャットにおけるリンクは特殊な文字である0x12で囲まれている。
	// そのため、まず0x12を見つけたらそこまではリテラルとなり、
	// 次の0x12まではリンクの内容となり、以降繰り返しとなる。
	for (;;) {
		// リンクの始端(0x12)を探す。
		char *begin = std::strchr(
			const_cast<char*>(eqc_text), 
			EQCHAT_LINK_EDGE
		);
		// 見つからなかったら抜ける。
		if (!begin) break;
		// 終端(0x12)を探す。
		char *end = std::strchr(
			begin + 1, 
			EQCHAT_LINK_EDGE
		);
		// 見つからなかったら抜ける。
		if (!end) break;
		// 始端までのリテラルを出力する。
		jc_text_out << std::string(eqc_text, begin - eqc_text);
		// 内容が十分に長ければ有効なリンクとみなす。
		std::size_t link_len = end - begin - 1;
		if (link_len > context.ini.chat.link_body_size) {
			// 次のような独自の形式に変換して出力する。
			// {番号:名前}
			std::size_t name_len = link_len - context.ini.chat.link_body_size;
			char *name = begin + 1 + context.ini.chat.link_body_size;
			jc_text_out << string_printf(
				"%c%d%c%.*s%c", 
				JCHAT_LINK_OPEN,          // 始端の記号('{')。
				data->link_bodies.size(), // 番号。
				JCHAT_LINK_DIVIDER,       // 番号と名前を分割する記号(':')。
				name_len,                 // 名前の長さ。
				name,                     // 名前。
				JCHAT_LINK_CLOSE          // 終端の記号('}')。
			);
			// リンクのボディ部分は、後でリンクを含んだ行を
			// EQチャットの形式に戻せるように保存しておく。
			data->link_bodies.emplace_back(
				begin + 1, 
				context.ini.chat.link_body_size
			);
		}
		// 終端の次の文字から繰り返す。
		eqc_text = end + 1;
	}
	// リンクが見つからなかったらリテラルとして出力する。
	jc_text_out << eqc_text;
	return jc_text_out.str();
}

// 日本語チャットのテキストをEQチャットのテキストに変換する。
// テキストにアイテムリンクが含まれていれば元の形式に戻す。
// 日本語チャットのテキストはシフトJISなのでutf-8に変換する。
std::string // 変換したEQチャットのテキスト(utf-8)。
jchat_bar_t::text_jChatToEQChat(
	const std::string &jc_text // 変換する日本語チャットのテキスト(シフトJIS)。
) {
	std::ostringstream eqc_text_out;
	// EQチャット→日本語チャットと同じようにアイテムリンクの
	// 変換を中心として、全角文字を考慮しながら処理していく。
	for (auto iter = jc_text.begin(); iter != jc_text.end();) {
		// 全角文字を考慮すると1文字ずつ処理していくしかない。
		char chr = *iter++;
		if (chr == JCHAT_LINK_OPEN) {
			// リンクの始端('{')であれば、分割記号(':')までは番号となる。
			std::size_t index = UINT_MAX;
			std::ostringstream index_out;
			while (iter != jc_text.end()) {
				chr = *iter++;
				if (chr == JCHAT_LINK_DIVIDER) {
					index = destringize<std::size_t,UINT_MAX>(index_out.str());
					break;
				}
				index_out << chr;
				if (iter != jc_text.end() && char_isJLead(chr)) 
					index_out << *iter++;
			}
			// 分割記号(':')から終端('}')までは名前となる。
			std::string name;
			std::ostringstream name_out;
			while (iter != jc_text.end()) {
				chr = *iter++;
				if (chr == JCHAT_LINK_CLOSE) {
					name = name_out.str();
					break;
				}
				name_out << chr;
				if (iter != jc_text.end() && char_isJLead(chr)) 
					name_out << *iter++;
			}
			// 番号と名前が有効なら、EQチャットの形式で出力する。
			if (index < data->link_bodies.size() && !name.empty()) 
				eqc_text_out <<
					BYTE(EQCHAT_LINK_EDGE) <<   // 始端(0x12)。
					data->link_bodies[index] << // ボディ部分。
					name <<                     // 名前。
					BYTE(EQCHAT_LINK_EDGE);     // 終端(0x12)。
		} else {
			// 始端('{')でなければ、そのまま出力する。
			eqc_text_out << chr;
			if (iter != jc_text.end() && char_isJLead(chr)) 
				eqc_text_out << *iter++;
		}
	}
	// 文字エンコーディングをシフトJISからutf-8に変換する。
	return string_sjisToUtf8(eqc_text_out.str());
}

// jchat.logにエラーログを書き込み、jchat.dllを停止する。
// Detourが解除されるので、EQチャットは元に戻る。
void context_t::fail(
	const error &err // 失敗の原因となったエラー。
) {
	if (log) putLog(*log, err.getMessage());
	release();
}

// jchat.dllで使用したすべてのリソースを解放する。
// チャットバーを破棄し、Detourを解除し、jchat.logを閉じる。
void context_t::release() {
	if (jchat_bar.getHandle()) api::DestroyWindow(jchat_bar.getHandle());
	try {
		// Detour用の脱出処理をクリアすると、これまでAttachした
		// すべての関数に対してDetourDetach関数が呼び出される。
		// Detachはトランザクションの中で行う。
		dtr::DetourTransactionBegin();
		scope_exit_t abort_exit([] {
			dtr::DetourTransactionAbort();
		});
		detour_exits.clear();
		dtr::DetourTransactionCommit();
		abort_exit.procedure() = nullptr;
	} catch (const error &err) {
		if (log) putLog(*log, err.getMessage());
	}
	reverseClear(exits);
	log.reset();
}

// 各ファイルのパスを設定し、jchat.logを開き、jchat.iniから読み込む。
// またeqgame.exeのベースアドレスを取得する。
void context_t::startup(
	HINSTANCE hinstDLL // DLLインスタンスのハンドル。
) {
	dll_handle = hinstDLL;
	dll_path = module_getPath(dll_handle);
	// コマンドライン引数を解析する。
	auto command_line = namedValuesToMap(parseArguments(api::GetCommandLine()));
	// コマンドライン引数のjchat.log_pathからログファイルのパスを取得する。
	// 省略ならEQフォルダのjchat.logのパスになる。
	log_path = dll_path;
	log_path.replace_extension("log");
	log_path = map_find(
		command_line, 
		log_path.filename().string() + "_path", 
		log_path.string()
	);
	// コマンドライン引数のjchat.ini_pathから設定ファイルのパスを取得する。
	// 省略ならEQフォルダのjchat.iniのパスになる。
	ini_path = dll_path;
	ini_path.replace_extension("ini");
	ini_path = map_find(
		command_line, 
		ini_path.filename().string() + "_path", 
		ini_path.string()
	);
	log = std_::make_ofstream(log_path.string(), std::ios_base::app);
	// チャットに関するセクションを読み込む。
	ini.chat.command_symbols = ini_getKeyValue(
		ini_path, 
		"Chat", 
		"CommandSymbols", 
		INI_CHAT_COMMAND_SYMBOLS_DEF
	);
	ini.chat.font_name = ini_getKeyValue(
		ini_path, 
		"Chat", 
		"FontName", 
		INI_CHAT_FONT_NAME_DEF
	);
	ini.chat.link_body_size = destringize<
		std::size_t,
		INI_CHAT_LINK_BODY_SIZE_DEF
	>(
		ini_getKeyValue(
		ini_path, 
		"Chat", 
		"LinkBodySize", 
		stringize(INI_CHAT_LINK_BODY_SIZE_DEF)
	));
	// ウインドウに関するセクションを読み込む。
	ini.window.left = destringize<
		int,
		INI_WINDOW_LEFT_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"Left", 
		stringize(INI_WINDOW_LEFT_DEF)
	));
	ini.window.min_width = destringize<
		std::size_t,
		INI_WINDOW_MIN_WIDTH_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"MinWidth", 
		stringize(INI_WINDOW_MIN_WIDTH_DEF)
	));
	ini.window.top = destringize<
		int,
		INI_WINDOW_TOP_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"Top", 
		stringize(INI_WINDOW_TOP_DEF)
	));
	ini.window.width = destringize<
		std::size_t,
		INI_WINDOW_WIDTH_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"Width", 
		stringize(INI_WINDOW_WIDTH_DEF)
	));
	// チャットバーの初期幅が最小幅より小さければ修正する。
	clampByMin(ini.window.width, ini.window.min_width);
	base_address = DWORD(api::GetModuleHandle(NULL));
}

//// グローバル変数の定義

// DispatchMessageのTarget関数ポインタ。
DispatchMessage_t DispatchMessage_target = ::DispatchMessage;

context_t context; // プログラムの文脈(状況)。

//// 関数の定義

// DispatchMessageのDetour関数。
// チャットバーがアクティブになっているときだけ、DispatchMessageの
// Trampoline関数を呼び出す前にTranslateMessage関数を呼び出す。
// これにより、メインウインドウで操作中にIMEが反応しないようにしつつ、
// チャットバーのコントロールにWM_CHARが送信されるようになる。
LRESULT // 結果。
WINAPI DispatchMessage_detour(
	const MSG *msg // 配送するメッセージ。
) {
	if (api::GetActiveWindow() == context.jchat_bar.getHandle()) 
		api::TranslateMessage(msg);
	return DispatchMessage_target(msg);
}

// jchat.iniからアドレスを読み込み、アドレスを計算し、
// eqgame_tに属するデータとTarget関数ポインタに代入する。
// そしてそれらの関数についてDetourAttach関数を呼び出す。
// この関数(early)はjchat.dllがロードされたときに呼び出される。
// この関数ではゲームの状態を取得できる関数のみをAttachする。
// それ以外の関数は後に呼び出される関数(lazy)のほうでAttachする。
// これはMQ2がKeypressHandler_HandleKeyDownのような関数をAttach
// した後にAttachすることで、先に実行されるようにするためである。
void bindAddressAndDetourAttach_early() {
	// Attachはトランザクションの中で行う。
	BEGIN_DETOUR_ATTACH();
		BIND_ADDRESS(CEverQuest);
		BIND_ADDRESS(CharSpawn);
		BIND_ADDRESS(MainWindowHandle);
		BIND_ADDRESS(NotInChatMode);
		BIND_ADDRESS_AND_DETOUR_ATTACH(CEverQuest_SetGameState);
		BIND_ADDRESS_AND_DETOUR_ATTACH(ProcessGameEvents);
	END_DETOUR_ATTACH();
}

// jchat.iniからアドレスを読み込み、アドレスを計算し、
// eqgame_tに属するTarget関数ポインタに代入する。
// そしてそれらの関数についてDetourAttach関数を呼び出す。
// (DispatchMessage関数もついでにここでAttachする。)
// この関数(lazy)はチャットバーを表示する直前に呼び出される。
// MQ2より後にDetourを追加することで、イベントの喪失を防ぐ。
void bindAddressAndDetourAttach_lazy() {
	// Attachはトランザクションの中で行う。
	BEGIN_DETOUR_ATTACH();
		DETOUR_ATTACH(DispatchMessage);
		BIND_ADDRESS_AND_DETOUR_ATTACH(CEverQuest_InterpretCmd);
		BIND_ADDRESS_AND_DETOUR_ATTACH(CXWndManager_DrawCursor);
		BIND_ADDRESS_AND_DETOUR_ATTACH(KeypressHandler_HandleKeyDown);
		BIND_ADDRESS_AND_DETOUR_ATTACH(KeypressHandler_HandleKeyUp);
	END_DETOUR_ATTACH();
}

// jchat.iniのOffsetセクションからオフセットを読み込み、
// eqgame.exeのメモリ空間における絶対アドレスを取得する。
DWORD // 取得したアドレス。
getAddress(
	const std::string &name // オフセットの名前。
) {
	return context.base_address + getOffset(name) - 0x400000ul;
}

// jchat.iniのOffsetセクションからオフセットを読み込む。
// オフセットの設定値の形式は16進数である。
// オフセットがゼロなら不正とみなしエラーをスローする。
DWORD // 取得したオフセット。
getOffset(
	const std::string &name // オフセットの名前。
) {
	DWORD offset = destringize<DWORD>(ini_getKeyValue(
		context.ini_path, 
		"Offset", 
		name,
		"0"
	), std::hex);
	if (!offset)
		throw error(string_printf(
			"Offsetセクションの%sキーが不正です。", 
			name.c_str()
		));
	return offset;
}

// 画面モードを取得する。
screen_modes // 取得した画面モード。
getScreenMode() {
	if (api::IsIconic(*eqgame_t::MainWindowHandle)) 
		return screen_modes::ICONIC;
	if (window_isTopMost(*eqgame_t::MainWindowHandle))
		return screen_modes::FULL_SCREEN;
	return screen_modes::WINDOW;
}

// ゲームの状態が変更されたときに呼び出される。
// ゲームの状態がINGAMEならチャットバーを表示し、
// それ以外ならチャットバーを非表示にする。
// エラーが発生したらjchat.dllの機能を停止する。
void onGameStateChange() {
	try {
		auto &bar = context.jchat_bar;
		if (context.game_state == GAMESTATE_INGAME) {
			// チャットバーを作成していなければ作成する。
			if (!bar.getHandle()) {
				// 画面モードを初期化する。
				context.screen_mode = getScreenMode();
				// earlyで作った以外の関数のDetourを作る。
				bindAddressAndDetourAttach_lazy();
				// チャットバーのウインドウクラスを登録する。
				jchat_bar_t::registerClass();
				// チャットバーを作成する。
				bar.create();
			}
			// 非表示なら表示し、念のために位置ズレを修正する。
			if (!api::IsWindowVisible(bar.getHandle())) {
				api::ShowWindow(bar.getHandle(), SW_SHOWNA);
				bar.fixPos();
			}
		} else if (bar.getHandle() && 
			api::IsWindowVisible(bar.getHandle())
		) api::ShowWindow(bar.getHandle(), SW_HIDE);
	} catch (const error &err) {
		context.fail(err);
	}
}

// このDLLがロードされたときに呼び出される。
// プログラムを開始し、一部の関数のDetourを作る。
// エラーが発生したらjchat.logに書き込んで、失敗を返す。
BOOL // 真なら成功、偽なら失敗。
onProcessAttach(
	HINSTANCE hinstDLL // DLLインスタンスのハンドル。
) {
	try {
		context.startup(hinstDLL);
		bindAddressAndDetourAttach_early();
	} catch (const error &err) {
		context.fail(err);
		return FALSE;
	}
	return TRUE;
}

// このDLLがアンロードされるときに呼び出される。
// チャットバーを破棄し、Detourを解除し、jchat.logを閉じる。
BOOL // 真なら成功、偽なら失敗。
onProcessDetach() {
	context.release();
	return TRUE;
}

// 画面モードが変更されたときに呼び出される。
// 変更によって生じるチャットバーの位置ズレを修正する。
void onScreenModeChange() {
	try {
		auto &bar = context.jchat_bar;
		if (bar.getHandle() &&
			api::IsWindowVisible(bar.getHandle())
		) bar.fixPos();
	} catch (const error &err) {
		context.fail(err);
	}
}

// ゲームの状態を最新の状態に更新する。
// ゲームの状態に応じてチャットバーの表示を切り替える。
void refreshGameState(
	int new_game_state // 新しいゲームの状態。
) {
	// ゲームの状態がゼロなら無視する。
	// ゼロではなく、なおかつ変更されていれば保存し、ハンドラを呼び出す。
	if (new_game_state && new_game_state != context.game_state) {
		context.game_state = new_game_state;
		onGameStateChange();
	}
}

// 画面モードを最新の状態に更新する。
// 画面モードに応じてチャットバーの位置ズレを修正する。
void refreshScreenMode() {
	// 画面モードが変更されていれば保存し、ハンドラを呼び出す。
	screen_modes new_screen_mode = getScreenMode();
	if (new_screen_mode != context.screen_mode) {
		context.screen_mode = new_screen_mode;
		onScreenModeChange();
	}
}

}

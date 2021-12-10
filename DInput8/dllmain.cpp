//// JEQ/プラグインローダー/DllMain関数

// DllMain関数は単体テスト用のDLLとリンクできないため、
// 本体のcppから抜き出し、こちらに独立して定義する。

//// インクルード

#include "dinput8.hpp"

#include <windows.h>

//// 関数の定義

// DLLがロードされたときなどにWindowsから呼び出される。
// 呼び出された理由に応じて本体の関数を呼び出す。
BOOL // 真なら成功、偽なら失敗。
WINAPI DllMain(
	HINSTANCE hinstDLL, // DLLインスタンスのハンドル。
	DWORD fdwReason,    // 呼び出された理由。
	LPVOID lpReserved   // 予約。
) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH: return jeq::onProcessAttach(hinstDLL);
	case DLL_PROCESS_DETACH: return jeq::onProcessDetach();
	}
    return TRUE;
}

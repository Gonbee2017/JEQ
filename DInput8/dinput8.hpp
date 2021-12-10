//// JEQ/プラグインローダー/本体

//// eqgame.exeを拡張

// eqgame.exeはいくつかのDLLをダイナミックリンクしているが、
// その中の一つにdinput8.dllがあり、eqgame.exeはdinput8.dllの
// DirectInput8Create関数のみをインポートしている。

// そのためdinput8.dllと同じ名前のDLLを作り、DirectInput8Createと
// 同じ名前の関数をエクスポートすれば、簡単にdinput8.dllを偽装できる。
// eqgame.exeには偽物のdinput8.dllをロードさせ、偽物のdinput8.dllで
// 改めて本物のdinput8.dllをロードし、偽物のDirectInput8Createから
// 本物のDirectInput8Createを呼び出せば、本物のdinput8.dllの機能を
// 提供しつつ、eqgame.exeプロセス内で任意のコードを実行できる。

//// プラグインとは？

// dinput8.dllを偽装することと、eqgame.exeプロセス内で
// 任意のコードを実行することはまったく違う機能である。
// そこで偽装以外の機能は偽物のdinput8.dllとは別のDLLに実装し、
// 偽物のdinput8.dllにはそれらのDLLをロードさせることにする。
// この任意の機能を持つDLLのことを「プラグイン」と呼び、偽物の
// dinput8.dllのことを「プラグインローダー」と呼ぶことにする。

// このプロジェクトのプラグインはMQ2のプラグインとは異なり、
// エクスポートしなければならない関数の規定は存在しない。
// プラグインとして必要なのはDllMain関数を実装することだけである。
// ただし大抵のプラグインはeqgame.exeにある関数のDetourを作る
// だろうから、eqgame.exeはもちろん、他のプラグインやMQ2に
// あまり影響を与えないように注意しなければならない。
// 例えばProcessGameEvents関数のDetourを作る場合、できる限り
// Trampoline関数を呼び出して、チェーンの実行をつないでほしい。

//// 実行方法

// プラグインローダーが実行されるまでの手順を以下に示す。
// ①eqgame.exeと同じフォルダに以下のファイルを配置する。
// ・dinput8.dll … プラグインローダー
// ・dinput8.ini … プラグインローダーの設定
// ②eqgame.exeを起動すると、Windowsによって本物のdinput8.dllの代わりに
//   ①で配置した偽物のdinput8.dllがロードされ、実行される。

// プラグインとしてロードするDLLはdinput8.iniファイルで設定する。
// ロードできるプラグインの数に制限はなく、何個でもロードできる。
// またエラーログはdinput8.logに書き込まれる。

#ifndef JEQ_DINPUT8_HPP
#define JEQ_DINPUT8_HPP

//// インクルード

#include "common.hpp"

#include <windows.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

//// エクスポート関数の宣言

extern "C" __declspec(dllexport) HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

namespace jeq {

//// 型の定義

// DirectInput8Create関数の型。
using DirectInput8Create_t = HRESULT (*)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);

// プログラムの文脈(状況)を表す。
// すべてのグローバル変数をまとめた構造体である。
// まとめることで単体テストにおいて簡単にリセットできる。
struct context_t {
	DirectInput8Create_t 
		DirectInput8Create_true = nullptr; // 本物のDirectInput8Create関数のポインタ。
	std::string dll_name;                  // このDLLの名前。"dinput8.dll"。
	std::vector<scope_exit_t> exits;       // 脱出処理のベクタ。
	HINSTANCE fake_dll_handle = nullptr;   // 偽物のdinput8.dll(このDLL)のハンドル。
	std::filesystem::path fake_dll_path;   // 偽物のdinput8.dll(このDLL)のパス。
	std::filesystem::path ini_path;        // dinput8.iniファイルのパス。
	std::shared_ptr<std::ostream> log;     // dinput8.logの出力ストリーム。
	std::filesystem::path log_path;        // dinput8.logのパス。
	std::vector<HMODULE> plugin_handles;   // プラグインのハンドルのベクタ。
	HMODULE true_dll_handle = nullptr;     // 本物のdinput8.dllのハンドル。
	std::filesystem::path true_dll_path;   // 本物のdinput8.dllのパス。

	void startup(HINSTANCE hinstDLL);
};

//// グローバル変数の宣言

extern context_t context;

//// 関数の宣言

void loadPlugin(const std::string &plugin_name);
void loadPlugins();
void loadTrueDLL();
BOOL onProcessAttach(HINSTANCE hinstDLL);
BOOL onProcessDetach();
std::filesystem::path searchTrueDLL();

}

#endif // #ifndef JEQ_DINPUT8_HPP

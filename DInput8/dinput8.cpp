//// JEQ/プラグインローダー/本体

//// インクルード

#include "dinput8.hpp"
#include "jcommon.hpp"

#include <windows.h>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

//// エクスポート関数の定義

// eqgame.exeからこの偽物のDirectInput8Createが呼び出されたら、
// すぐに本物のDirectInput8Createを同一の引数で呼び出すことで、
// あたかもこの関数が本物であるかのように振る舞う。
HRESULT DirectInput8Create(
	HINSTANCE hinst, 
	DWORD dwVersion, 
	REFIID riidltf, 
	LPVOID *ppvOut, 
	LPUNKNOWN punkOuter
) {
	return jeq::context.DirectInput8Create_true(
		hinst, 
		dwVersion, 
		riidltf, 
		ppvOut, 
		punkOuter
	);
}

namespace jeq {

//// グローバル変数の定義

context_t context; // プログラムの文脈(状況)。

//// クラスメンバの定義

// プログラムを開始する。
// 各ファイルのパスを設定し、dinput8.logを開く。
void context_t::startup(
	HINSTANCE hinstDLL // DLLインスタンスのハンドル。
) {
	fake_dll_handle = hinstDLL;
	fake_dll_path = module_getPath(fake_dll_handle);
	dll_name = fake_dll_path.filename().string();
	// コマンドライン引数を解析する。
	auto command_line = namedValuesToMap(parseArguments(api::GetCommandLine()));
	// コマンドライン引数のdinput8.log_pathからログファイルのパスを取得する。
	// 省略ならEQフォルダのdinput8.logのパスになる。
	log_path = fake_dll_path;
	log_path.replace_extension("log");
	log_path = map_find(
		command_line, 
		log_path.filename().string() + "_path", 
		log_path.string()
	);
	// コマンドライン引数のdinput8.ini_pathから設定ファイルのパスを取得する。
	// 省略ならEQフォルダのdinput8.iniのパスになる。
	ini_path = fake_dll_path;
	ini_path.replace_extension("ini");
	ini_path = map_find(
		command_line, 
		ini_path.filename().string() + "_path", 
		ini_path.string()
	);
	log = std_::make_ofstream(log_path.string(), std::ios_base::app);
}

//// 関数の定義

// プラグインのDLLをロードする。
void loadPlugin(
	const std::string &plugin_name // プラグインの名前。
) {
	HMODULE plugin_handle = api::LoadLibrary(plugin_name.c_str());
	context.plugin_handles.emplace_back(plugin_handle);
	context.exits.emplace_back([plugin_handle] {
		api::FreeLibrary(plugin_handle);
	});
}

// すべてのプラグインのDLLをロードする。
// プラグインはdinput.iniのPluginセクションで設定する。
// キーの名前はDLLファイルの名前、値はロードするかどうかのフラグを示す。
// ロードフラグは1ならロードすること、0ならロードしないことを意味する。
/* サンプルコード
[Plugin]
; すべてのプラグインを列挙する。
hoge.dll=1 ; hoge.dllをロードする。
fuga.dll=0 ; fuga.dllをロードしない。
*/
void loadPlugins() {
	// Pluginセクションにあるすべてのキーを取得する。
	auto plugin_keys = ini_getKeys(context.ini_path, "Plugin");
	// 各キーを巡回する。
	for (const named_value_t &plugin_key : plugin_keys) {
		// キーの値で示されているロードフラグが真なら、
		// キーの名前で示されているDLLファイルをロードする。
		if (destringize<bool>(plugin_key.value)) loadPlugin(plugin_key.name);
	}
}

// 本物のdinput8.dllを検索し、ロードし、
// DirectInput8Create関数のアドレスを取得する。
void loadTrueDLL() {
	// サーチパス(環境変数PATH)から本物のdinput8.dllを検索する。
	context.true_dll_path = searchTrueDLL();
	if (context.true_dll_path.empty())
		throw error(string_printf(
			"本物の%sが見つかりませんでした。", 
			context.dll_name.c_str()
		));
	// 見つかったパスからDLLをロードする。
	context.true_dll_handle = 
		api::LoadLibrary(context.true_dll_path.string().c_str());
	context.exits.emplace_back([] {
		api::FreeLibrary(context.true_dll_handle);
	});
	// ロードしたDLLからDirectInput8Create関数のアドレスを取得する。
	FARPROC proc = api::GetProcAddress(
		context.true_dll_handle, 
		"DirectInput8Create"
	);
	context.DirectInput8Create_true = DirectInput8Create_t(proc);
}

// このDLLがロードされたときに呼び出される。
// プログラムを開始し、本物のdinput8.dllをロードし、
// dinput.iniに設定されているすべてのプラグインをロードする。
// エラーが発生したらdinput.logに書き込んで、失敗を返す。
BOOL // 真なら成功、偽なら失敗。
onProcessAttach(
	HINSTANCE hinstDLL // DLLインスタンスのハンドル。
) {
	try {
		context.startup(hinstDLL);
		loadTrueDLL();
		loadPlugins();
	} catch (const error &err) {
		if (context.log) putLog(*context.log, err.getMessage());
		return FALSE;
	}
	return TRUE;
}

// このDLLがアンロードされるときに呼び出される。
// プラグインと本物のdinput8.dllをアンロードし、dinput8.logを閉じる。
BOOL // 真なら成功、偽なら失敗。
onProcessDetach() {
	reverseClear(context.exits);
	context.log.reset();
	return TRUE;
}

// サーチパス(環境変数PATH)から本物のdinput8.dllを検索する。
// 本物のdinput8.dllの検索にAPIのSearchPath関数を使うことはできない。
// なぜならSearchPathは真っ先にカレントディレクトリを検索するので、
// 本物より先に偽物のdinput8.dll(このDLL)を見つけてしまうからである。
std::filesystem::path // 見つかったパス。見つからなかったら空っぽ。
searchTrueDLL() {
	// PATHの値をセミコロンで分割し、ディレクトリのコンテナに格納する。
	auto search_dirs = string_split(std_::getenv("PATH"), ';', false);
	// PATHの各ディレクトリを巡回する。
	for (const std::string &search_dir : search_dirs) {
		// そのディレクトリにあるdinput8.dllのパスを作成する。
		std::filesystem::path dll_path(search_dir);
		dll_path.append(context.dll_name);
		// dinput8.dllが存在し、偽物と同じファイルでなければ、
		// それが本物のdinput8.dllであるから、そのパスを返す。
		if (std_::filesystem_exists(dll_path) && 
			!std_::filesystem_equivalent(dll_path, context.fake_dll_path)
		) return dll_path;
	}
	return std::filesystem::path();
}

}

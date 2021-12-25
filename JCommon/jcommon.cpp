//// JEQ/共通処理ライブラリ/本体

//// インクルード

#include "jcommon.hpp"

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <initializer_list>
#include <locale>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace jeq {

//// 定数の定義

// CPUコアの数。
const std::size_t HARDWARE_CONCURRENCY = std::thread::hardware_concurrency();

//// クラスメンバ関数の定義

// テストヘルパーを構築する。
test_helper_t::test_helper_t(
	const std::string &str // バッファを初期化する文字列。
) {
	data->buff = std::stringstream(str);
}

// テストヘルパーから次の行を取得する。
std::string // 取得した次の行。終端に到達していれば空文字列。
test_helper_t::getLine() {
	std::string line;
	if (!std::getline(data->buff, line)) return std::string();
	return line;
}

// テストヘルパーから次の連番の文字列を取得する。
// 連番は1から始まるので注意すること。
std::string & // 取得した連番の文字列。
test_helper_t::getSeqStr(
	std::size_t offset // ベースからのオフセット。
	                   // ゼロなら最新の連番。省略ならゼロ。
) {
	int seq = getSeq(offset);
	// 再配置されないようにnewした文字列を保存しておく。
	auto str = new std::string(stringize(seq));
	data->seq_str_buff.emplace_back(str);
	return *str;
}

// テストヘルパーの現在の連番をベースとして設定する。
void test_helper_t::setSeqBase() {
	data->seq_base = data->seq;
}

// ワークが完了したかどうかを判定する。
bool // 真なら完了、偽なら未完。
thread_pool_t::work_t::isDone() const {
	auto lock = std_::make_unique_lock(&data->mutex);
	return data->tasks_done == data->tasks_count;
}

// ワークが完了するまで待機する。
void thread_pool_t::work_t::waitUntilDone() {
	auto lock = std_::make_unique_lock(&data->mutex);
	std_::condition_variable_wait(
		&data->work_cv, 
		lock.get(), 
		[this]() -> bool {
			return data->tasks_done == data->tasks_count;
		}
	);
}

// ワークを構築する。
thread_pool_t::work_t::work_t(
	std::size_t tasks_count // タスクの数。
) {
	data->tasks_count = tasks_count;
}

// ワークに所属するタスク1個の実行が完了したら呼び出される。
void thread_pool_t::work_t::onTaskDone() {
	auto lock = std_::make_unique_lock(&data->mutex);
	++data->tasks_done;
	std_::unique_lock_unlock(lock.get());
	std_::condition_variable_notify_all(&data->work_cv);
}

// スレッドプールにタスクの非同期実行を依頼する。
thread_pool_t::work_t // 作成したワーク。
thread_pool_t::ask(
	const task_t &task // 非同期実行するタスク。
) {
	return ask(std::initializer_list<task_t>{task});
}

// ワーカースレッドを起動する。
void thread_pool_t::launch(
	std::size_t workers_count // ワーカースレッドの数。省略ならCPUコアの数。
) {
	// 指定された数のワーカースレッドを作成する。
	for (std::size_t i = 0; i < workers_count; ++i)
		data->workers.emplace_back(std_::make_thread([this] {
			worker_procedure();
		}));
}

// スレッドプールのインスタンスデータを破棄する。
// 待機中のワーカーはすぐに離脱させ、実行中のワーカーは完了次第
// 離脱させる。すべてのワーカースレッドが離脱したら制御を戻す。
thread_pool_t::data_t::~data_t() {
	if (workers.empty()) return;
	auto lock = std_::make_unique_lock(&mutex);
	leave = true;
	std_::unique_lock_unlock(lock.get());
	std_::condition_variable_notify_all(&workers_cv);
	for (auto worker : workers) std_::thread_join(worker.get());
}

// ワーカースレッドの処理。
// タスクの実行を繰り返す。
void thread_pool_t::worker_procedure() {
	for (;;) {
		auto lock = std_::make_unique_lock(&data->mutex);
		// 離脱フラグがセットされるか、または
		// キューにタスクがプッシュされれば目覚める。
		std_::condition_variable_wait(
			&data->workers_cv, 
			lock.get(), 
			[this]() -> bool {
				return data->leave || !data->tasks.empty();
			}
		);
		// 離脱ならループを脱出し、スレッドを終了する。
		if (data->leave) break;
		// キューから未実行のタスクをポップする。
		task_t task = data->tasks.front();
		data->tasks.pop();
		// ロックを解除してから、タスクを実行する。
		std_::unique_lock_unlock(lock.get());
		task();
	}
}

//// 関数の定義

// マウスカーソルの位置を取得する。
POINT // 取得した位置。
cursor_getPos() {
	POINT pos;
	api::GetCursorPos(&pos);
	return pos;
}

// マウスカーソルがウインドウの上に乗っているどうかを判定する。
// ウインドウが非表示、またはアイコン状態なら乗っていないとみなす。
bool // 真なら乗っている、偽なら乗っていない。
cursor_isOverWindow(
	HWND hwnd // ウインドウのハンドル。
) {
	if (!api::IsWindowVisible(hwnd) || 
		api::IsIconic(hwnd)
	) return false;
	POINT cursor_pos = cursor_getPos();
	RECT rect = window_getRect(hwnd);
	return cursor_pos.x >= rect.left && 
		cursor_pos.x < rect.right &&
		cursor_pos.y >= rect.top && 
		cursor_pos.y < rect.bottom;
}

// 設定ファイルからキーを削除する。
void ini_deleteKey(
	const std::string &ini_path,     // 設定ファイルのパス。
	const std::string &section_name, // セクションの名前。
	const std::string &key_name      // 削除するキーの名前。
) {
	api::WritePrivateProfileString(
		section_name.c_str(), 
		key_name.c_str(), 
		NULL,
		ini_path.c_str()
	);
}

// 設定ファイルからセクションを削除する。
void ini_deleteSection(
	const std::string &ini_path,    // 設定ファイルのパス。
	const std::string &section_name // 削除するセクションの名前。
) {
	api::WritePrivateProfileString(
		section_name.c_str(), 
		NULL,
		NULL,
		ini_path.c_str()
	);
}

// 設定ファイルからキーの値を読み込む。
std::string // 読み込んだキーの値。
ini_getKeyValue(
	const std::string &ini_path,     // 設定ファイルのパス。
	const std::string &section_name, // セクションの名前。
	const std::string &key_name,     // キーの名前。
	const std::string &def_value     // キーがなかったときの既定値。
	                                 // 省略すると空文字列。
) {
	std::string value(1024, '\0');
	DWORD len;
	for (;;) {
		len = api::GetPrivateProfileString(
			section_name.c_str(), 
			key_name.c_str(),
			def_value.c_str(),
			LPTSTR(value.data()), 
			DWORD(value.size()), 
			ini_path.c_str()
		);
		if (len != value.size() - 1) break;
		value.resize(value.size() * 2);
	}
	if (!len) return def_value;
	value.resize(len);
	value.shrink_to_fit();
	return value;
}

// 設定ファイルからセクションを読み込む。
// GetPrivateProfileSectionで読み込んだセクションは
// '\0'区切りでキーが並び、終端は'\0'が2個続く。
// またキーは'='で名前と値が区切られている。
std::string // 読み込んだセクション。
ini_loadSection(
	const std::string &ini_path,    // 設定ファイルのパス。
	const std::string &section_name // 読み込むセクションの名前。
) {
	std::string section(1024, '\0');
	DWORD len;
	for (;;) {
		len = api::GetPrivateProfileSection(
			section_name.c_str(), 
			LPTSTR(section.data()), 
			DWORD(section.size()), 
			ini_path.c_str()
		);
		if (len != section.size() - 2) break;
		section.resize(section.size() * 2);
	}
	section.resize(len + 2);
	section.shrink_to_fit();
	return section;
}

// 設定ファイルにキーの値を書き込む。
void ini_setKeyValue(
	const std::string &ini_path,     // 設定ファイルのパス。
	const std::string &section_name, // セクションの名前。
	const std::string &key_name,     // キーの名前。
	const std::string key_value      // 書き込むキーの値。
) {
	api::WritePrivateProfileString(
		section_name.c_str(), 
		key_name.c_str(), 
		key_value.c_str(), 
		ini_path.c_str()
	);
}

// モジュールのパスを取得する。
std::string // 取得したパス。
module_getPath(
	HMODULE hmod // モジュールのハンドル。
) {
	char buff[MAX_PATH] = {};
	api::GetModuleFileName(hmod, buff, MAX_PATH);
	return buff;
}

// 位置をクライアント座標系からスクリーン座標系に変換する。
POINT // 変換した位置。
point_clientToScreen(
	POINT pos, // 変換する位置。
	HWND hwnd  // ウインドウのハンドル。
) {
	api::ClientToScreen(hwnd, &pos);
	return pos;
}

// 位置をクライアント座標系からスクリーン座標系に変換する。
POINT // 変換した位置。
point_screenToClient(
	POINT pos, // 変換する位置。
	HWND hwnd  // ウインドウのハンドル。
) {
	api::ScreenToClient(hwnd, &pos);
	return pos;
}

// ログファイルにメッセージを書き込む。
// メッセージは日時と共に書き込まれ、次のような形式になる。
// "YYYY/MM/DD HH:mm:SS.sss メッセージ"
// ここでYYYYには年、MMには月、DDには日、HHには時、
// mmには分、SSには秒、sssにはミリ秒を埋め込む。
void putLog(
	std::ostream *log,     // ログファイルの出力ストリーム。
	const std::string &mes // 書き込むメッセージ。
) {
	// 現在の日時と共に書き込む。
	using namespace std::chrono;
	system_clock::time_point  now = std_::chrono_system_clock_now();
	std::time_t time = system_clock::to_time_t(now);
	std::tm *tm = std::localtime(&time);
	// ミリ秒も加える。
	milliseconds epoch_msec = 
		duration_cast<milliseconds>(now.time_since_epoch());
	int msec = epoch_msec.count() % 1000;
	std_::ostream_putLine(log, string_printf(
		"%04d/%02d/%02d %02d:%02d:%02d.%03d %s",
		1900 + tm->tm_year,
		1 + tm->tm_mon,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec,
		msec,
		mes.c_str()
	));
}

// 2個の矩形が重複しているかどうかを判定する。
bool // 真なら重複している、偽なら重複していない。
rect_areOverlapped(
	const RECT &rect1, // 1個目の矩形。
	const RECT &rect2  // 2個目の矩形。
) {
	return section_areOverlapped(
		rect1.left, 
		rect1.right, 
		rect2.left, 
		rect2.right
	) && section_areOverlapped(
		rect1.top, 
		rect1.bottom, 
		rect2.top, 
		rect2.bottom
	);
}

// シフトJISの文字列の部分を置換する。
std::string // 部分を置換した文字列(シフトJIS)。
sjis_replace(
	const std::string &str,     // 部分を置換する文字列(シフトJIS)。
	const std::string &pre_sub, // 置換前の部分(シフトJIS)。
	const std::string &post_sub // 置換後の部分(シフトJIS)。
) {
	std::ostringstream result_out;
	auto iter = str.begin();
	while (iter != str.end()) {
		if ((str.end() - iter) >= int(pre_sub.length()) &&
			std::equal(iter, iter + pre_sub.length(), pre_sub.begin(), pre_sub.end())
		) {
			result_out << post_sub;
			iter += pre_sub.length();
		} else {
			char chr = *iter++;
			result_out << chr;
			if (iter != str.end() && sjis_isLead(chr))
				result_out << *iter++;
		}
	}
	return result_out.str();
}

// シフトJISの文字列に含まれる大文字を小文字に変換する。
std::string // 変換した文字列(シフトJIS)。
sjis_toLower(
	const std::string &src // 変換する文字列(シフトJIS)。
) {
	std::string dest;
	for (auto iter = src.begin(); iter != src.end();) {
		char chr = *iter++;
		if (sjis_isLead(chr)) {
			dest.append(1, chr);
			if (iter != src.end()) dest.append(1, *iter++);
		} else dest.append(1, std::tolower(chr));
	}
	return dest;
}

// 書式に従って可変長引数を文字列に埋め込む。
// std::sprintfと違って結果をstd::stringで返す。
std::string // 埋め込んだ文字列。
string_printf(
	const char *fmt, ... // 埋め込む可変長引数。
) {
	char buff[1024];
	va_list args;
	va_start(args, fmt);
	std::vsprintf(buff, fmt, args);
	va_end(args);
	return std::string(buff);
}

// 文字列の部分を置換する。
std::string // 部分を置換した文字列。
string_replace(
	const std::string &str,     // 部分を置換する文字列。
	const std::string &pre_sub, // 置換前の部分。
	const std::string &post_sub // 置換後の部分。
) {
	std::ostringstream result_out;
	auto iter = str.begin();
	for (;;) {
		auto sub_begin = std::search(
			iter,
			str.end(),
			pre_sub.begin(),
			pre_sub.end()
		);
		if (sub_begin == str.end()) break;
		result_out << std::string(iter, sub_begin) << post_sub;
		iter = sub_begin + pre_sub.length();
	}
	result_out << std::string(iter, str.end());
	return result_out.str();
}

// 文字列をシフトJISからutf-8に変換する。
std::string // 変換したutf-8の文字列。
string_sjisToUtf8(
	const std::string &src // 変換するシフトJISの文字列。
) {
	return wstring_utf16ToUtf8(string_sjisToUtf16(src));
}

// 文字列をシフトJISからutf-16に変換する。
std::wstring // 変換したutf-16の文字列。
string_sjisToUtf16(
	const std::string &src // 変換するシフトJISのの文字列。
) {
	// 変換後の長さを取得する。
	int dest_len = MultiByteToWideChar(
		CP_ACP, 
		0, 
		src.data(), 
		-1, 
		nullptr, 
		0
	);
	// 変換後の文字列を格納する領域を確保する。
	std::wstring dest(dest_len, L'\0');
	// 変換する。
	if (!MultiByteToWideChar(
		CP_ACP, 
		0, 
		src.data(), 
		-1, 
		LPWSTR(dest.data()), 
		int(dest.size())
	)) return std::wstring();
	// 終端の'\0'を切り捨てる。
	dest.resize(dest_len - 1);
	return dest;
}

// 文字列をutf8からシフトJISに変換する。
std::string // 変換したシフトJISの文字列。
string_utf8ToSJIS(
	const std::string &src // 変換するutf-8の文字列。
) {
	return wstring_utf16ToSJIS(string_utf8ToUtf16(src));
}

// 文字列をutf-8からutf-16に変換する。
std::wstring // 変換したutf-16の文字列。
string_utf8ToUtf16(
	const std::string &src // 変換するutf-8のの文字列。
) {
	// 変換後の長さを取得する。
	int dest_len = MultiByteToWideChar(
		CP_UTF8, 
		0, 
		src.data(), 
		-1, 
		nullptr, 
		0
	);
	// 変換後の文字列を格納する領域を確保する。
	std::wstring dest(dest_len, L'\0');
	// 変換する。
	if (!MultiByteToWideChar(
		CP_UTF8, 
		0, 
		src.data(), 
		-1, 
		LPWSTR(dest.data()), 
		int(dest.size())
	)) return std::wstring();
	// 終端の'\0'を切り捨てる。
	dest.resize(dest_len - 1);
	return dest;
}

// utf-8の文字列内の長い単語を短い単語に分割する。
// ここで単語とはスペース(' ')かタブ('\t')で区切られた部分のことである。
// また長い単語とは長さが15バイトを超える単語のことである。
// 例えばu8"あいうえおかきくけこ"という30バイト×1個の長い単語は、
// u8"あいうえお かきくけこ"という15バイト×2個の短い単語に分割される。
// また1バイトの文字と2バイト以上の文字の間には必ずスペース(' ')を挿入する。
// 例えばu8"あaいiうuえeおo"ならu8"あ a い i う u え e お o"になる。
// ただし、どれだけ長くても1バイトの文字の間にはスペース(' ')を挿入しない。
// 例えばu8"1234567890123456"ならu8"1234567890123456"になる。
std::string // 短い単語に分割した文字列(utf-8)。
utf8_divideLongWords(
	const std::string &message, // 短い単語に分割する文字列(utf-8)。
	std::size_t link_body_size  // リンクのボディ部分のサイズ。
) {
	std::ostringstream result_out;
	std::size_t word_len = 0;
	std::size_t letter_size;
	std::size_t pre_letter_size = 0;
	bool link = false;
	auto iter = message.begin();
	while (iter != message.end()) {
		char chr = *iter++;
		if (chr == ' ' || chr == '\t') {
			letter_size = 0;
			result_out << chr;
			word_len = 0;
		} else if (chr == EQCHAT_LINK_EDGE) {
			letter_size = 0;
			result_out << chr;
			if (!link) {
				for (std::size_t i = 0; i < link_body_size; ++i) {
					if (iter == message.end()) break;
					result_out << *iter++;
				}
			}
			link ^= 1;
			word_len = 0;
		} else {
			letter_size = utf8_getLetterSize(chr);
			if ((letter_size == 1 && pre_letter_size > 1) ||
				(letter_size > 1 && pre_letter_size == 1) ||
				(letter_size > 1 && pre_letter_size > 1 && 
					word_len + letter_size > EQCHAT_GOOD_WORD_MAX_LENGTH
				)
			) {
				result_out << ' ';
				word_len = 0;
			}
			result_out << chr;
			for (std::size_t i = 1; i < letter_size; ++i) {
				if (iter == message.end()) break;
				result_out << *iter++;
			}
			word_len += letter_size;
		}
		pre_letter_size = letter_size;
	}
	return result_out.str();
}

// ウインドウのクライアント領域の矩形を取得する。
RECT // 取得したクライアント領域の矩形。
window_getClientRect(
	HWND hwnd // ウインドウのハンドル。
) {
	RECT rect;
	api::GetClientRect(hwnd, &rect);
	return rect;
}

// ウインドウの矩形を取得する。
RECT // 取得したウインドウの矩形。
window_getRect(
	HWND hwnd // ウインドウのハンドル。
) {
	RECT rect;
	api::GetWindowRect(hwnd, &rect);
	return rect;
}

// ウインドウのテキストを取得する。
std::string // 取得したテキスト。
window_getText(
	HWND hwnd // ウインドウのハンドル。
) {
	std::string text(1024, '\0');
	int len;
	for (;;) {
		len = api::GetWindowText(
			hwnd, 
			LPSTR(text.data()), 
			int(text.size())
		);
		if (len != int(text.size()) - 1) break;
		text.resize(text.size() * 2);
	}
	text.resize(len);
	text.shrink_to_fit();
	return text;
}

// 最上位ウインドウかどうかを判定する。
bool // 真なら最上位、偽ならそれ以外。
window_isTopMost(
	HWND hwnd // ウインドウのハンドル。
) {
	return api::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST;
}

// 文字列をutf-16からシフトJISに変換する。
std::string // 変換したシフトJISの文字列。
wstring_utf16ToSJIS(
	const std::wstring &src // 変換するutf-16の文字列。
) {
	// 変換後の長さを取得する。
	int dest_len = WideCharToMultiByte(
		CP_ACP, 
		0, 
		src.data(), 
		-1, 
		nullptr, 
		0, 
		nullptr, 
		nullptr
	);
	// 変換後の文字列を格納する領域を確保する。
	std::string dest(dest_len, '\0');
	// 変換する。
	if (!WideCharToMultiByte(
		CP_ACP, 
		0, 
		src.data(), 
		-1, 
		LPSTR(dest.data()), 
		int(dest.size()), 
		nullptr, 
		nullptr
	)) return std::string();
	// 終端の'\0'を切り捨てる。
	dest.resize(dest_len - 1);
	return dest;
}

// 文字列をutf-16からutf-8に変換する。
std::string // 変換したutf-8の文字列。
wstring_utf16ToUtf8(
	const std::wstring &src // 変換するutf-16の文字列。
) {
	// 変換後の長さを取得する。
	int dest_len = WideCharToMultiByte(
		CP_UTF8, 
		0, 
		src.data(), 
		-1, 
		nullptr, 
		0, 
		nullptr, 
		nullptr
	);
	// 変換後の文字列を格納する領域を確保する。
	std::string dest(dest_len, '\0');
	// 変換する。
	if (!WideCharToMultiByte(
		CP_UTF8, 
		0, 
		src.data(), 
		-1, 
		LPSTR(dest.data()), 
		int(dest.size()), 
		nullptr, 
		nullptr
	)) return std::string();
	// 終端の'\0'を切り捨てる。
	dest.resize(dest_len - 1);
	return dest;
}

}

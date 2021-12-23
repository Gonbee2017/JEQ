//// JEQ/共通処理ライブラリ/本体

// 複数のEXEやDLLで共通する処理をこのライブラリにまとめて配置する。
// ただし今のところ共通していなくても、いつかしそうな汎用的な処理、
// 例えば文字エンコーディング変換のような処理もここに配置する。
// またC++は14と17の両方、プラットフォームはx86とx64の両方に対応する。

#ifndef JEQ_JCOMMON_HPP
#define JEQ_JCOMMON_HPP

//// インクルード

#include "api.hpp"
#include "dtr.hpp"
#include "error.hpp"
#include "std.hpp"

#include <windows.h>

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <ios>
#include <istream>
#include <ostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

//// マクロの定義

// スパイクラスの定義を開始する。
// スパイクラスは単体テスト関数内で定義する。対象とするクラスを
// 継承することで、protectedメンバにアクセスできるようにする。
#define BEGIN_DEF_SPY_CLASS(\
	target /* テストの対象となるクラスの名前。 */\
) \
	struct spy_ ## target : public target {\
		static void procedure() {

// スパイクラスの定義を名前空間付きで開始する。
// スパイクラスは単体テスト関数内で定義する。対象とするクラスを
// 継承することで、protectedメンバにアクセスできるようにする。
#define BEGIN_DEF_SPY_CLASS_N(\
	namespace_, /* テストの対象となるクラスの名前空間。 */\
	target      /* テストの対象となるクラスの名前。 */\
) \
	struct spy_ ## target : public namespace_::target {\
		static void procedure() {

// スパイクラスの定義を終了する。
#define END_DEF_SPY_CLASS(\
	target /* テストの対象となるクラスの名前。 */\
) \
		}\
	};\
	spy_ ## target::procedure();

namespace jeq {

//// 定数式の定義

// EQチャットのリンクの定数。
constexpr char EQCHAT_LINK_EDGE = 0x12; // 両端の記号。

//// 定数の宣言

extern const std::size_t HARDWARE_CONCURRENCY;

//// 型の定義

// 大文字/小文字を区別しない文字列の比較関数を表す。
struct ignore_case_compare_t {
	inline bool operator ()(const std::string &lhs, const std::string &rhs) const;
};

// 名前付きの値を表す。
struct named_value_t {
	std::string name;  // 名前。
	std::string value; // 値。
};

// スコープから脱出するときの処理を表す。
// インスタンスデータが破棄されるときに設定された処理を実行する。
/* サンプルコード
{ // このブロックの中でデバイスコンテキストを使用する。
	HDC hdc = api::GetDC(NULL); // デスクトップのデバイスコンテキストを取得する。
	scope_exit_t hdc_exit([hdc] { // デバイスコンテキスト用の脱出処理を定義する。
		api::ReleaseDC(NULL, hdc); // 実行されるとデバイスコンテキストを解放する。
	});
	... // デバイスコンテキストを使用する。
} // ブロックのスコープから脱出するときに設定したラムダが実行される。
*/
class scope_exit_t {
public:
	// 処理の型。
	using procedure_t = std::function<void()>;

	scope_exit_t() = default;
	inline explicit scope_exit_t(const procedure_t &proc);
	inline procedure_t &procedure();
protected:
	// インスタンスデータを表す。
	struct data_t {
		procedure_t proc; // 処理。

		inline ~data_t();
	};

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // インスタンスデータ。
};

// 単体テストを助ける機能を表す。
// モック関数が呼び出されると関数名や引数の値などを記録する。
// また連番からパラメータを生成する。
/* サンプルコード
// cursor_isOverWindowの単体テストを行う。
test_helper_t help; // ヘルパー。
// IsWindowVisibleのモック関数としてラムダを設定する。
api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
	help << "IsWindowVisible\n"; // ラムダが呼び出されたことを記録する。
	help << int(hWnd) << '\n'; // 第一引数のhWndを記録する。
	return FALSE; // ウインドウが非表示ならcursor_isOverWindowは偽を返すはず。
};
// cursor_isOverWindowはまずapi::IsWindowVisible、つまり前述のラムダを呼び出す。
Assert::IsFalse(cursor_isOverWindow(help.getSeq<HWND>())); // 偽を返すか？
// 呼び出し履歴をチェックする。
Assert::AreEqual(std::string("IsWindowVisible"), help.getLine()); // ラムダが呼び出されたか？
Assert::AreEqual(std::string("1"), help.getLine()); // 渡されたhWndは正しいか？
Assert::AreEqual(std::string(), help.getLine()); // 想定以上に記録されていないか？
*/
class test_helper_t {
public:
	test_helper_t() = default;
	test_helper_t(const std::string &str);
	std::string getLine();
	template <class X = int>
	X getSeq(std::size_t offset = 0);
	template <class X>
	std::shared_ptr<X> getSeqPtr(std::size_t offset = 0);
	std::string &getSeqStr(std::size_t offset = 0);
	template <typename X>
	test_helper_t &operator <<(const X &x);
	void setSeqBase();
protected:
	// インスタンスデータを表す。
	struct data_t {
		std::stringstream buff; // バッファ。
		int seq = 0;            // 連番。
		int seq_base = 0;       // 連番のベース。
		std::vector<std::unique_ptr<std::string>> 
			seq_str_buff;       // 連番の文字列のバッファ。
	};

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // インスタンスデータ。
};

// 単体テストで使用するトランポリン関数を表す。
// チェック用のstd::functionを呼び出す関数を定義する。
/* サンプルコード
// hoge関数がfoo関数を正しく呼び出せるかをチェックする。
using test_foo_t = test_trampoline_t<void>; // トランポリン関数を生成。
test_helper_t help; // ヘルパー。
// fooのモック関数としてラムダを設定する。
test_foo_t::target = [&] {
	help << "foo\n"; // ラムダが呼び出されたことを記録する。
};
// fooのトランポリン関数を設定する。
foo_target = test_foo_t::trampoline;
// hogeはfoo_target、つまりtest_foo_t::trampolineを呼び出す。
// test_foo_t::trampolineはtest_foo_t::target、つまり前述のラムダを呼び出す。
hoge();
// 呼び出し履歴をチェックする。
Assert::AreEqual(std::string("foo"), help.getLine()); // ラムダが呼び出されたか？
*/
template <class Ret, class ...Args> 
struct test_trampoline_t {
	static std::function<Ret(Args...)> target;

	static Ret trampoline(Args ...args);
	static Ret WINAPI trampoline_api(Args ...args);

	Ret trampoline_member(Args ...args);
};

// スレッドプールを表す。
// タスクの実行を依頼すると、プーリングされているスレッドの中から
// 待機中のスレッドを目覚めさせて、そのスレッド上で非同期に実行する。
/* サンプルコード
// スレッドプールを作成する。
thread_pool_t thread_pool;
// ワーカースレッドを起動する。
thread_pool.launch();
// 加算を行うタスクを作成する。
int result;
thread_pool_t::task_t task = [&result] {
	result = 1 + 2;
};
// タスクの非同期実行を依頼する。
auto work = thread_pool.ask(task);
...
// ワーク(この例ではタスク１個のみ）の実行が完了するまで待機する。
work.waitUntilDone();
// 結果を出力する。
std::cout << result << std::endl;
*/
class thread_pool_t {
public:
	// タスクの型。
	using task_t = std::function<void()>;

	// 実行中のタスクのグループを表す。
	class work_t {
		friend thread_pool_t;
	public:
		work_t() = default;
		bool isDone() const;
		void waitUntilDone();
	protected:
		// インスタンスデータを表す。
		struct data_t {
			std::mutex mutex;            // 排他制御のためのミューテックス。
			std::size_t tasks_done = 0;  // 完了したタスクの数。
			std::size_t tasks_count = 0; // タスクの数。
			std::condition_variable 
				work_cv;                 // ワークに通知するための条件変数。
		};

		std::shared_ptr<data_t> data = std::make_shared<data_t>(); // インスタンスデータ。

		explicit work_t(std::size_t tasks_count);
		void onTaskDone();
	};

	template <class Container>
	work_t ask(const Container &tasks);
	work_t ask(const task_t &task);
	void launch(std::size_t workers_count = HARDWARE_CONCURRENCY);
protected:
	// ロックの型。
	using lock_t = std::unique_lock<std::mutex>;

	// インスタンスデータを表す。
	struct data_t {
		bool leave = false;       // 離脱フラグ。
		                          // セットならワーカーが処理から離脱する。
		std::mutex mutex;         // 排他制御のためのミューテックス。
		std::queue<task_t> tasks; // タスクのキュー。
		std::vector<std::shared_ptr<std::thread>> 
			workers;              // ワーカーのベクタ。
		std::condition_variable 
			workers_cv;           // ワーカーに通知するための条件変数。

		~data_t();
	};

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // インスタンスデータ。

	void worker_procedure();
};

//// テンプレート関数とインライン関数の宣言

inline bool char_isJLead(char chr);
template <class X>
void clampByMin(X &value, const X &min);
template <class X>
X destringize(const std::string &str, X def);
template <class X, class Manip>
X destringize(const std::string &str, X def, Manip &&manip);
inline BYTE getCtrlAlphabetKey(char alphabet);
template <class PostX, class PreX>
PostX indirect_cast(PreX x);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<named_value_t> ini_getKeys(const std::string &ini_path, const std::string &section_name);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<named_value_t> ini_parseSection(const std::string &section);

template <class Map>
typename Map::mapped_type map_find(const Map &map, const typename Map::key_type &key, const typename Map::mapped_type &def_mapped = typename Map::mapped_type());
template <class NamedValues>
std::map<std::string,std::string,ignore_case_compare_t> namedValuesToMap(const NamedValues &named_values);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<named_value_t> parseArguments(const std::string &args);

template <class Container>
void reverseClear(Container &container);
template <class Number>
bool section_areOverlapped(const Number &begin1, const Number &end1, const Number &begin2, const Number &end2);
template <class X>
std::string stringize(const X &x);
template <class X, class Manip>
std::string stringize(const X &x, Manip &&manip);
template <class X, class Lead, class ...Trails>
std::string stringize(const X &x, Lead &&lead, Trails &&...trails);

#if _MSVC_LANG >= 201703L
template <template <class> class Container = std::vector>
#else
template <template <class> class Container>
#endif
Container<std::string> string_split(const std::string &str,	char delim,	bool allow_empty = true);

//// 非公開なテンプレート関数とインライン関数の宣言

template <class X>
X _destringizeFrom(std::istream &in, X def);
template <class X, class Manip>
X _destringizeFrom(std::istream &in, X def, Manip &&manip);
template <class X>
void _stringizeTo(std::ostream &out, const X &x);
template <class X, class Manip>
void _stringizeTo(std::ostream &out, const X &x, Manip &&manip);
template <class X, class Lead, class ...Trails>
void _stringizeTo(std::ostream &out, const X &x, Lead &&lead, Trails &&...trails);

//// 関数の定義

POINT cursor_getPos();
bool cursor_isOverWindow(HWND hwnd);
void ini_deleteKey(const std::string &ini_path, const std::string &section_name, const std::string &key_name);
void ini_deleteSection(const std::string &ini_path, const std::string &section_name);
std::string ini_getKeyValue(const std::string &ini_path, const std::string &section_name, const std::string &key_name, const std::string &def_value = std::string());
std::string ini_loadSection(const std::string &ini_path, const std::string &section_name);
void ini_setKeyValue(const std::string &ini_path, const std::string &section_name, const std::string &key_name, const std::string key_value);
std::string module_getPath(HMODULE hmod);
POINT point_clientToScreen(POINT pos, HWND hwnd);
POINT point_screenToClient(POINT pos, HWND hwnd);
void putLog(std::ostream *log, const std::string &mes);
bool rect_areOverlapped(const RECT &rect1, const RECT &rect2);
std::string string_printf(const char *fmt, ...);
std::string string_sjisToUtf8(const std::string &src);
std::wstring string_sjisToUtf16(const std::string &src);
std::string string_toLower(const std::string &src);
RECT window_getClientRect(HWND hwnd);
RECT window_getRect(HWND hwnd);
std::string window_getText(HWND hwnd);
bool window_isTopMost(HWND hwnd);
std::string wstring_utf16ToUtf8(const std::wstring &src);

//// クラスのテンプレートメンバとインラインメンバの定義

// 2個の文字列を大文字/小文字の区別なしで比較する。
bool // 真ならlhs<rhs、偽ならlhs>=rhs。
ignore_case_compare_t::operator ()(
	const std::string &lhs, // 左側の文字列。
	const std::string &rhs  // 右側の文字列。
) const {
	return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
}

// 脱出処理を構築する。
scope_exit_t::scope_exit_t(
	const procedure_t &proc // 実行される処理。
) {
	data->proc = proc;
}

// 脱出処理のインスタンスデータが破棄されるときに実行される処理を参照する。
scope_exit_t::procedure_t & // 参照した実行される処理。
scope_exit_t::procedure() {
	return data->proc;
}

// 脱出処理のインスタンスデータを破棄する。
// 処理が設定されていればここで実行する。
scope_exit_t::data_t::~data_t() {
	if (proc) proc();
}

// テストヘルパーから次の連番を取得する。
// 連番は1から始まるので注意すること。
template <
	class X // 何かの型。
> X // 取得した連番をキャストした何か。
test_helper_t::getSeq(
	std::size_t offset // ベースからのオフセット。
	                   // ゼロなら最新の連番。省略ならゼロ。
) {
	++data->seq;
	int seq;
	if (offset) seq = data->seq_base + int(offset);
	else seq = data->seq;
	return X(seq);
}

// テストヘルパーから次の連番を何かのアドレスとするポインタを取得する。
// ポインタのデリータは何もしない。
template <
	class X // 何かの型。
> std::shared_ptr<X> // 取得した連番を何かのアドレスとするポインタ。
test_helper_t::getSeqPtr(
	std::size_t offset // ベースからのオフセット。
                       // ゼロなら最新の連番。省略ならゼロ。
) {
	return std::shared_ptr<X>(getSeq<X*>(offset), [](X*) {});
}

// テストヘルパーに何かを記録する。
template <
	typename X // 記録する何かの型。
> test_helper_t & // 呼び出し履歴。
test_helper_t::operator <<(
	const X &x // 記録する何か。
) {
	data->buff << x;
	return *this;
}

// 単体テストでトランポリン関数から呼び出されるターゲット関数。
template <
	class Ret,    // 戻り値の型。
	class ...Args // 引数の型。
> std::function<Ret(Args...)> test_trampoline_t<Ret,Args...>::target;

// 単体テストでターゲット関数を呼び出すトランポリン関数。
template <
	class Ret,    // 戻り値の型。
	class ...Args // 引数の型。
> Ret // 戻り値。
test_trampoline_t<Ret,Args...>::trampoline(
	Args ...args // 引数。
) {
	return target(args...);
}

// 単体テストでターゲット関数を呼び出すAPI向けトランポリン関数。
template <
	class Ret,    // 戻り値の型。
	class ...Args // 引数の型。
> Ret // 戻り値。
WINAPI test_trampoline_t<Ret,Args...>::trampoline_api(
	Args ...args // 引数。
) {
	return target(args...);
}

// 単体テストでターゲット関数を呼び出すメンバ向けトランポリン関数。
template <
	class Ret,    // 戻り値の型。
	class ...Args // 引数の型。
> Ret // 戻り値。
test_trampoline_t<Ret,Args...>::trampoline_member(
	Args ...args // 引数。
) {
	return target(args...);
}

// スレッドプールにタスクコンテナの非同期実行を依頼する。
template <
	class Container // タスクコンテナの型。
> thread_pool_t::work_t // 作成したワーク。
thread_pool_t::ask(
	const Container &tasks // 非同期実行するタスクのコンテナ。
) {
	// タスクが所属するワークを作成する。
	work_t work(tasks.size());
	// タスクをキューにプッシュする。
	auto lock = std_::make_unique_lock(&data->mutex);
	for (const task_t &task : tasks)
		data->tasks.push([work, task]() mutable {
			// このラムダはワーカースレッドで非同期に実行される。
			// タスク本体を実行し、完了したらハンドラを呼び出す。
			task();
			work.onTaskDone();
		});
	std_::unique_lock_unlock(lock.get());
	// ワーカーを目覚めさせて、キューにプッシュしたタスクを実行させる。
	std_::condition_variable_notify_all(&data->workers_cv);
	return work;
}

//// テンプレート関数とインライン関数の定義

// 文字がシフトJISの1バイト目かどうかを判定する。
bool // 真ならシフトJISの1バイト目、偽ならそれ以外。
char_isJLead(
	char chr // 文字。
) {
	BYTE byte = BYTE(chr);
	return (byte >= 0x81 && byte <= 0x9f) || (byte >= 0xe0 && byte <= 0xfc);
}

// 値を最小値以上にする。
template <
	class X // 値の型。
>
void clampByMin(
	X &value,    // 値。minより小さいときはminを代入する。
	const X &min // 最小値。
) {
	if (value < min) value = min;
}

// 文字列を何かに変換する。
/* サンプルコード
// 文字列を整数に変換する。
int num = destringize<int>("123", 0);
*/
template <
	class X // 何かの型。
> X // 変換した何か。失敗したときは既定値。
destringize(
	const std::string &str, // 変換する文字列。
	X def                   // 既定値。
) {
	if (str.empty()) return X();
	std::istringstream in(str);
	return _destringizeFrom<X>(in, def);
}

// 文字列を何かに変換する。
// また変換の方法をマニピュレータで指定する。
/* サンプルコード
// 16進数の文字列を整数に変換する。
int num = destringize<int>("abc", 0, std::hex);
*/
template <
	class X,    // 何かの型。
	class Manip // マニピュレータの型。
> X // 変換した何か。失敗したときは既定値。
destringize(
	const std::string &str, // 変換する文字列。
	X def,                  // 既定値。
	Manip &&manip           // マニピュレータ。
) {
	if (str.empty()) return X();
	std::istringstream in(str);
	return _destringizeFrom<X>(in, def, manip);
}

// Ctrl+アルファベットの仮想キーコードを取得する。
BYTE // 取得した仮想キーコード。
getCtrlAlphabetKey(
	char alphabet // 取得するアルファベット(大文字)。
) {
	return 0x01 + (alphabet - 'A');
}

// 何かを間接的にキャストする。
// たいていの値は直にキャストできるが、例えばメンバ関数のポインタは
// 直にキャストできない。そこで値をいったん一時変数に代入して、
// そのポインタをキャストすることで、間接的に値をキャストする。
/* サンプルコード
class Hoge {
public: void foo() { ... }
};
...
// Hogeクラスのメンバ関数fooのアドレスを取得したい。
DWORD ng1 = DWORD(&Hoge::foo); // 直にキャストしようとするとエラーになる。
DWORD ng2 = reinterpret_cast<DWORD>(&Hoge::foo); // これもエラーになる。
DWORD ok = indirect_cast<DWORD>(&Hoge::foo); // 間接的にキャストすればOK。
*/
template <
	class PostX, // キャスト後の何かの型。
	class PreX   // キャスト前の何かの型。
> PostX // キャストした何か。
indirect_cast(
	PreX x // キャストする何か。
) {
	return *(PostX*)(&x);
}

// 設定ファイルのセクションにあるすべてのキーを取得する。
template <
	template <class> class Container // キーのコンテナの型。
> Container<named_value_t> // 取得したキーのコンテナ。
ini_getKeys(
	const std::string &ini_path,    // 設定ファイルのパス。
	const std::string &section_name // セクションの名前。
) {
	return ini_parseSection<Container>(ini_loadSection(ini_path, section_name));
}

// 設定ファイルのセクションを解析する。
// GetPrivateProfileSectionで読み込んだセクションは
// '\0'区切りでキーが並び、終端は'\0'が2個続く。
// またキーは'='で名前と値が区切られている。
// ここではセクションを解析してキーのコンテナを作成する。
template <
	template <class> class Container // キーのコンテナの型。
> Container<named_value_t> // 作成したキーのコンテナ。
ini_parseSection(
	const std::string &section // セクション。
) {
	Container<named_value_t> keys;
	named_value_t key = {};
	std::string *cur = &key.name;
	char pre_chr = '\0';
	auto chr_iter = section.begin();
	for (;;) {
		char chr = *chr_iter++;
		if (chr) {
			if (char_isJLead(chr)) {
				cur->append(1, chr);
				chr = *chr_iter++;
				if (chr) cur->append(1, chr);
			} else if (chr == '=') cur = &key.value;
			else cur->append(1, chr);
		} else if (pre_chr) {
			keys.emplace_back(key);
			key = {};
			cur = &key.name;
		} else break;
		pre_chr = chr;
	}
	if (!key.name.empty()) keys.emplace_back(key);
	return keys;
}

// マップから探す。
template <
	class Map // マップの型。
> typename Map::mapped_type // 見つかったマップ値。見つからなかったら既定値。
map_find(
	const Map &map,                             // マップ。
	const typename Map::key_type &key,          // 探すキー。
	const typename Map::mapped_type &def_mapped // 既定値。
) {
	auto iter = map.find(key);
	if (iter == map.end()) return def_mapped;
	return iter->second;
}

// 名前付きの値のコンテナをマップに変換する。
// 名前が重複している場合、最初の要素の値になる。
// 名前は大文字/小文字を区別しない。
template <
	class NamedValues // 名前付きの値のコンテナの型。
> std::map<std::string,std::string,ignore_case_compare_t> // 変換したマップ。
namedValuesToMap(
	const NamedValues &named_values // 変換する名前付きの値のコンテナ。
) {
	std::map<std::string,std::string,ignore_case_compare_t> map;
	for (const named_value_t &named_value : named_values)
		map.emplace(named_value.name, named_value.value);
	return map;
}

// 引数の文字列を解析し、名前付きの値のコンテナを作成する。
// 引数は何個でも記述可能で、引数と引数の間はスペースで区切る。
// 引数1 引数2 ... 引数n
// それぞれ引数は次のような形式になる。
// "NAME[=VALUE]"
// ここでNAMEは引数の名前、VALUEは引数の値を示す。
// 名前と値にスペース(' ')とイコール('=')を含めたいときは、
// それらを含む文字列を引用符('"')で囲む。
// 引用符で囲んだ文字列の中に引用符を含めたいときは、
// 引用符を連続で書けばよい。
template <
	template <class> class Container // 名前付きの値のコンテナの型。
> Container<named_value_t> // 作成した名前付きの値のコンテナ。
parseArguments(
	const std::string &args
) {
	Container<named_value_t> named_values;
	std::string name;
	std::string value;
	std::string *token = &name;
	auto flush = [&] {
		if (!name.empty()) {
			named_values.emplace_back(named_value_t{name, value});
			name.clear();
			value.clear();
		}
	};
	for (auto iter = args.begin(); iter != args.end();)	{
		char chr = *iter++;
		if (chr == '"') {
			char pre_chr = '\0';
			while (iter != args.end()) {
				chr = *iter++;
				if (chr == '"') {
					if (pre_chr == '"') {
						token->append(1, chr);
						pre_chr = '\0';
					} else pre_chr = chr;
				} else if (pre_chr == '"') break;
				else {
					token->append(1, chr);
					if (iter != args.end() && char_isJLead(chr)) {
						token->append(1, *iter++);
						pre_chr = '\0';
					} else pre_chr = chr;
				}
			}
			if (chr == '"') break;
		}
		if (chr == ' ') {
			flush();
			token = &name;
		} else if (token == &name && chr == '=')
			token = &value;
		else {
			token->append(1, chr);
			if (iter != args.end() && char_isJLead(chr))
				token->append(1, *iter++);
		}
	}
	flush();
	return named_values;
}

// コンテナを逆順にクリアする。
// リソースの獲得には、それ以前に獲得した別のリソースを利用することが多い。
// 例えばパターンブラシの作成には、ロード済みのビットマップが必要になる。
// これらを使用し終えて不要になったら、ビットマップ→ブラシという獲得した
// 順ではなく、逆のブラシ→ビットマップという順で削除しなければならない。
// そのためリソースを獲得した順でコンテナに削除処理を追加しておいて、
// 不要になったら追加したときとは逆の順で削除処理を実行すればうまくいく。
/* サンプルコード
std::vector<scope_exit_t> exits; // 削除処理のコンテナ。
// ビットマップをロードする。
HBITMAP hbmp = api::LoadBitmap(hinst, MAKEINTRESOURCE(IDB_X));
// ビットマップ用の削除処理を追加する。
exits.emplace_back([hbmp] {
	api::DeleteObject(hbmp); // ①実行されるとビットマップを削除する。
});
// ビットマップからパターンブラシを作成する。
HBRUSH hbrush = api::CreatePatternBrush(hbmp);
// パターンブラシ用の削除処理を追加する。
exits.emplace_back([hbrush] {
	api::DeleteObject(hbrush); // ②実行されるとパターンブラシを削除する。
});
... // パターンブラシを使用する。
// コンテナを逆順にクリアすれば、削除処理は②→①の順で実行される。
reverseClear(exits);
*/
template <
	class Container // コンテナの型。
> void reverseClear(
	Container &container // コンテナ。
) {
	while (!container.empty()) container.pop_back();
}

// ２個の区間が重複しているかどうかを判定する。
// 区間は[begin, end)であり、endは含まない。
template <
	class Number // 数値の型。
> bool // 真なら重複している、偽なら重複していない。
section_areOverlapped(
	const Number &begin1, // １個目の区間の始端値。
	const Number &end1,   // １個目の区間の終端値。
	const Number &begin2, // ２個目の区間の始端値。
	const Number &end2    // ２個目の区間の終端値。
) {
	return (begin1 >= begin2 && begin1 < end2) ||
		(end1 > begin2 && end1 <= end2) ||
		(begin1 < begin2 && end1 > end2);
}

// 何かを文字列に変換する。
/* サンプルコード
// 文字列を整数に変換する。
std::string str = stringize(123);
*/
template <
	class X // 何かの型。
> std::string // 変換した文字列。
stringize(
	const X &x // 変換する何か。
) {
	std::ostringstream out;
	_stringizeTo<X>(out, x);
	return out.str();
}

// 何かを文字列に変換する。
// また変換の方法をマニピュレータで指定する。
/* サンプルコード
// 整数を16進数の文字列に変換する。
std::string str = stringize(0xabc, std::hex);
*/
template <
	class X,    // 何かの型。
	class Manip // マニピュレータの型。
> std::string // 変換した文字列。
stringize(
	const X &x,   // 変換する何か。
	Manip &&manip // マニピュレータ。
) {
	std::ostringstream out;
	_stringizeTo<X>(out, x, manip);
	return out.str();
}

// 何かを文字列に変換する。
// また変換の方法を複数のマニピュレータで指定する。
/* サンプルコード
// 整数を8桁の16進数の文字列に変換する。
std::string str = stringize(0xabc, std::hex, std::setw(8), std::setfill('0'));
*/
template <
	class X,        // 何かの型。
	class Lead,     // 1個目のマニピュレータの型。
	class ...Trails // 2個目以降のマニピュレータの型。
> std::string // 変換した文字列。
stringize(
	const X &x,        // 変換する何か。
	Lead &&lead,       // 1個目のマニピュレータ。
	Trails &&...trails // 2個目以降のマニピュレータ。
) {
	std::ostringstream out;
	_stringizeTo<X>(out, x, lead, trails...);
	return out.str();
}

// 文字列を区切り文字で分割する。
template <
	template <class> class Container // 文字列のコンテナの型。
> Container<std::string> // 分割した文字列のコンテナ。
string_split(
	const std::string &str, // 分割する文字列。
	char delim,             // 区切り文字。
	bool allow_empty        // 空文字列をコンテナに追加するかどうか。
	                        // 真なら追加し、偽なら追加しない。省略すると真。
) {
	Container<std::string> toks;
	std::string tok;
	auto flush = [allow_empty, &toks, &tok] {
		if (allow_empty || !tok.empty()) {
			toks.emplace_back(tok);
			tok.clear();
		}
	};
	for (auto iter = str.begin(); iter != str.end();) {
		char chr = *iter++;
		if (chr == delim) flush();
		else {
			tok.push_back(chr);
			if (char_isJLead(chr) && iter != str.end()) 
				tok.push_back(*iter++);
		}
	}
	flush();
	return toks;
}

//// 非公開なテンプレート関数とインライン関数の定義

// ストリームから何かを入力する。
template <
	class X // 何かの型。
> X // 入力した何か。失敗したときは既定値。
_destringizeFrom(
	std::istream &in, // 入力ストリーム。
	X def             // 既定値。
) {
	X x;
	in.exceptions(std::ios_base::failbit);
	try {
		in >> x;
	} catch (const std::ios_base::failure&) {
		x = def;
	}
	return x;
}

// ストリームから何かを入力する。
// また入力の方法をマニピュレータで指定する。
template <
	class X,    // 何かの型。
	class Manip // マニピュレータの型。
> X // 入力した何か。失敗したときは既定値。
_destringizeFrom(
	std::istream &in, // 入力ストリーム。
	X def,            // 既定値。
	Manip &&manip     // マニピュレータ。
) {
	in >> manip;
	return _destringizeFrom<X>(in, def);
}

// 何かをストリームに出力する。
template <
	class X // 何かの型。
> void _stringizeTo(
	std::ostream &out, // 出力ストリーム。
	const X &x         // 出力する何か。
) {
	out << x;
}

// 何かをストリームに出力する。
// また出力の方法をマニピュレータで指定する。
template <
	class X,    // 何かの型。
	class Manip // マニピュレータの型。
> void _stringizeTo(
	std::ostream &out, // 出力ストリーム。
	const X &x,        // 出力する何か。
	Manip &&manip      // マニピュレータ。
) {
	out << manip;
	_stringizeTo(out, x);
}

// 何かをストリームに出力する。
// また出力の方法を複数のマニピュレータで指定する。
template <
	class X,        // 何かの型。
	class Lead,     // 1個目のマニピュレータの型。
	class ...Trails // 2個目以降のマニピュレータの型。
> void _stringizeTo(
	std::ostream &out, // 出力ストリーム。
	const X &x,        // 出力する何か。
	Lead &&lead,       // 1個目のマニピュレータ。
	Trails &&...trails // 2個目以降のマニピュレータ。
) {
	out << lead;
	_stringizeTo(out, x, trails...);
}

}

#endif // #ifndef JEQ_JCOMMON_HPP

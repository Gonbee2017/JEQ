//// JEQ/共通処理ライブラリ/インポートヘッダ

//// インポートとは？

// リソースにアクセスする関数を呼び出す関数は単体テストがしにくい。
// 例えばstd::getenvは環境変数というリソースにアクセスするので、
// std::getenvを呼び出す関数を単体テストしようとすると、
// 対象となる環境変数を事前に設定しておく必要がある。
// またはGetWindowTextならウインドウを作成しておかなければならない。
// このような準備、アクセス、後始末を繰り返し行うのは効率が悪いので、
// リソースにアクセスする関数については直に呼び出さずに
// std::function経由で間接的に呼び出すようにする。
// 関数ポインタ経由ではなくstd::function経由にするのは、
// 単体テストで本当の関数の代わりに呼び出されることになる
// モック関数をラムダとして定義できるようにするためである。

// またライブラリ関数のエラーハンドリングは煩雑になりやすい。
// 例えばGetWindowTextは引数に不正なウインドウハンドルを渡されると
// エラーを返す。このようなエラーを呼び出しごとにチェックすると、
// ソースコードが長くなり、見通しも悪くなってしまう。
// エラーを返すライブラリ関数については、専用のラッパーを定義するよう
// にして、そこでエラーをチェックし、エラーなら例外をスローする。
// 単体テストしやすいように本当の関数はstd::function経由で呼び出す。

// 前述の2つの理由により、std::functionを介してライブラリ関数を
// 間接的に呼び出すようにすることを「インポート」と呼ぶことにする。

// ただしライブラリ関数であっても、リソースにアクセスせず、
// なおかつエラーをチェックする必要のない関数であれば、
// インポートの対象にはせず、直に呼び出すようにする。
// 例えばstd::strchrやstd::stringなどがそれに該当する。

// このヘッダではインポートを簡潔に書くためのマクロを定義している。
// 実際のインポートは特定のライブラリごとに行う。

#ifndef JEQ_IMPORT_HPP
#define JEQ_IMPORT_HPP

//// インクルード

#include <functional>

// インポートを宣言する。
/* サンプルコード
DECL_IMP(GetKeyState, SHORT, int nVirtKey);
*/
#define DECL_IMP(\
	name,     /* 関数名。 */\
	ret_type, /* 戻り値の型。 */\
	...       /* すべての引数の型と名前。 */\
) \
	using name ## _t = std::function<ret_type(__VA_ARGS__)>;\
	extern name ## _t name;

// インポートを宣言する。
// エラーチェックを行うラッパーとして宣言する。
/* サンプルコード
DECL_IMP_WRAP(SetWindowText, BOOL, HWND hWnd, LPCSTR lpString);
*/
#define DECL_IMP_WRAP(\
	name,     /* 関数名。 */\
	ret_type, /* 戻り値の型。 */\
	...       /* すべての引数の型と名前。 */\
) \
	using name ## _t = std::function<ret_type(__VA_ARGS__)>;\
	extern name ## _t name ## _true;\
	ret_type name(__VA_ARGS__);

// インポートを定義する。
// 左辺のみを定義するので、続けて初期化すること。
/* サンプルコード
DEF_IMP(chrono_system_clock_now) = std::chrono::system_clock::now;
*/
#define DEF_IMP(\
	name  /* 関数名。 */\
) \
	name ## _t name

// インポートを定義する。
// 大域名前空間の同一名関数で初期化する。
/* サンプルコード
DEF_IMP_EASY(SendMessage);
*/
#define DEF_IMP_EASY(\
	name  /* 関数名。 */\
) \
	name ## _t name = ::name;

// インポートを定義する。
// ラッパー関数が呼び出す本当の関数を定義する。
/* サンプルコード
DEF_IMP_TRUE(filesystem_equivalent) = [](
	const std::filesystem::path &p1, 
	const std::filesystem::path &p2
) -> bool {
	return std::filesystem::equivalent(p1, p2);
};
*/
#define DEF_IMP_TRUE(name) \
	name ## _t name ## _true

// インポートを定義する。
// ラッパー関数の仮引数までを定義するので、続けて本体を定義すること。
/* サンプルコード
DEF_IMP_WRAP(
	filesystem_equivalent,
	bool,
	const std::filesystem::path &p1, 
	const std::filesystem::path &p2
) {
	try {
		return filesystem_equivalent_true(p1, p2);
	} catch (const std::system_error &err) {
		throw error(__FUNCTION__, err.code().value());
	}
}
*/
#define DEF_IMP_WRAP(\
	name,     /* 関数名。 */\
	ret_type, /* 戻り値の型。 */\
	...       /* すべての引数の型と名前。 */\
) \
	ret_type name(__VA_ARGS__)

// インポートを定義する。
// ラッパー関数が呼び出す本当の関数を大域名前空間の同一名関数で初期化する。
// ラッパー関数の仮引数までを定義するので、続けて本体を定義すること。
/* サンプルコード
DEF_IMP_WRAP_EASY(SetActiveWindow, HWND, HWND hWnd) {
	HWND ret = SetActiveWindow_true(hWnd);
	if (!ret) throw error(__FUNCTION__);
	return ret;
}
*/
#define DEF_IMP_WRAP_EASY(\
	name,     /* 関数名。 */\
	ret_type, /* 戻り値の型。 */\
	...       /* すべての引数の型と名前。 */\
) \
	name ## _t name ## _true = ::name;\
	ret_type name(__VA_ARGS__)

#endif // #ifndef JEQ_IMPORT_HPP

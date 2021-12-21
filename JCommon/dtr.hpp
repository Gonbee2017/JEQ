//// JEQ/共通処理ライブラリ/Detours関数インポート

// このプロジェクトで使用しているDetours関数をインポートする。
// Detours関数は実行コードなどのリソースに必ずアクセスするので、
// すべての関数がインポートの対象になる。

//// Detourとは？

// 「Detour」の概念について簡単に説明する。
// Detourはその名の通り、関数呼び出しをフックするための「迂回路」である。
// 関数のDetourを作るときは、対象となるTarget関数(T関数)と迂回路となる
// Detour関数(D関数)と元の関数を呼ぶためのTrampoline関数(t関数)を用意する。

// T関数にD関数をAttachすると、まず「T関数の先頭部分」が、「D関数への
// ジャンプ命令」に書き換えられる。そして「T関数の先頭部分」はt関数の先頭に
// 移され、続けて「T関数の先頭直後のアドレス」へのジャンプ命令が追加される。

// AttachされたT関数が呼び出されると、入ってすぐにD関数へジャンプするため、
// あたかもD関数が直接呼び出されたかのように見える。
// 一方、t関数が呼び出されると、最初に「T関数の先頭部分」が実行され、続けて
// 「T関数の先頭直後」へジャンプするため、T関数が呼び出されたように見える。

// Attach後の各関数の概略を以下に示す。
// ・T関数
//   ①D関数の①へジャンプ
//   ②何らかの処理
//   ③リターン
// ・D関数
//   ①何らかの処理
//   ②t関数を呼び出す
//   ③何らかの処理
//   ④リターン
// ・t関数
//   ①書き換えられる前のT関数の①
//   ②T関数の②へジャンプ

// この状態でT関数を呼び出したときの実行の流れを以下に示す。
// ↓T関数の①
// ↓D関数の①②
//   ↓t関数の①②
//   ↓T関数の②③
// ↓D関数の③④
// このようにD関数はT関数の迂回路として、T関数の前後に処理を追加したり、
// T関数自体を実行するかどうかを制御したりできるようになる。

// なおDetourは一つの関数に対して何個でも作ることができる。
// 例えば前述のT関数に対してさらにD'関数をAttachしたとする。
// Attach(二回目)後の各関数の概略を以下に示す。
// ・T関数
//   ①D'関数の①へジャンプ
//   ②何らかの処理
//   ③リターン
// ・D'関数
//   ①何らかの処理
//   ②t'関数を呼び出す
//   ③何らかの処理
//   ④リターン
// ・t'関数
//   ①D関数の①へジャンプ
//   ②T関数の②へジャンプ
// ・D関数
//   ①何らかの処理
//   ②t関数を呼び出す
//   ③何らかの処理
//   ④リターン
// ・t関数
//   ①書き換えられる前のT関数の①
//   ②T関数の②へジャンプ

// この状態でT関数を呼び出したときの実行の流れを以下に示す。
// ↓T 関数の①
// ↓D'関数の①②
//   ↓t'関数の①
//   ↓D 関数の①②
//     ↓t 関数の①②
//     ↓T 関数の②③
//   ↓D 関数の③④
// ↓D'関数の③④
// このようにD'関数はT関数ではなくD関数の迂回路として機能する。
// 複数のDetourはチェーンを形成し、より後に追加されたほうから、
// より先に追加されたほうへと順番に実行されることになる。
// ただし途中でTrampoline関数を呼び出さなければ連鎖は途切れる。

#ifndef JEQ_DTR_HPP
#define JEQ_DTR_HPP

//// インクルード

#include "error.hpp"
#include "import.hpp"

#include <windows.h>

#include <string>

namespace jeq { namespace dtr {

//// 型の定義

// Detours関数のエラーを表す。
// 関数名とエラーナンバーを保持する。
class error_t : public jeq::error_t {
public:
	error_t() = default;
	error_t(const std::string &func_name, LONG number);
	LONG getNumber() const;
	const std::string &getFunctionName() const;
protected:
	std::string func_name; // 関数名。
	LONG number = 0;       // エラーナンバー。0(NO_ERROR)で初期化。
};

//// インポートの宣言

DECL_IMP_WRAP(DetourAttach, LONG, PVOID *ppPointer, PVOID pDetour);
DECL_IMP(DetourDetach, LONG, PVOID *ppPointer, PVOID pDetour);
DECL_IMP(DetourTransactionAbort, LONG);
DECL_IMP_WRAP(DetourTransactionBegin, LONG);
DECL_IMP_WRAP(DetourTransactionCommit, LONG);

}}

#endif // #ifndef JEQ_API_HPP

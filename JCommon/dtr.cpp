//// JEQ/共通処理ライブラリ/Detours関数インポート

//// インクルード

#include "dtr.hpp"
#include "jcommon.hpp"

#include <windows.h>
#include <detours.h>

namespace jeq::dtr {

//// クラスメンバの定義

// 標準関数のエラーを構築する。
// エラーメッセージを作成する。
error::error(
	const std::string &func_name, // 関数名。
	LONG number                   // エラーナンバー。
) : func_name(func_name), 
	number(number) 
{
	msg = string_printf("%sが失敗しました。(%d)", func_name.c_str(), number);
}

// 関数名を取得する。
const std::string & // 取得した関数名。
error::getFunctionName() const {
	return func_name;
}

// エラーナンバーを取得する。
LONG // 取得したエラーナンバー。
error::getNumber() const {
	return number;
}

//// インポートの定義

DEF_IMP_DEF(DetourAttach) = [] (PVOID *ppPointer, PVOID pDetour) -> LONG {
	return ::DetourAttach(ppPointer, pDetour);
};

DEF_IMP_WRAP(DetourAttach, LONG, PVOID *ppPointer, PVOID pDetour) {
	LONG ret = DetourAttach_true(ppPointer, pDetour);
	if (ret != NO_ERROR) throw error(__FUNCTION__, ret);
	return ret;
}

DEF_IMP(DetourDetach) = [] (PVOID *ppPointer, PVOID pDetour) -> LONG {
	return ::DetourDetach(ppPointer, pDetour);
};

DEF_IMP_EASY(DetourTransactionAbort);

DEF_IMP_WRAP_EASY(DetourTransactionBegin, LONG) {
	LONG ret = DetourTransactionBegin_true();
	if (ret != NO_ERROR) throw error(__FUNCTION__, ret);
	return ret;
}

DEF_IMP_WRAP_EASY(DetourTransactionCommit, LONG) {
	LONG ret = DetourTransactionCommit_true();
	if (ret != NO_ERROR) throw error(__FUNCTION__, ret);
	return ret;
}

}

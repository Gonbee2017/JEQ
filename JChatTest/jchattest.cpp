#include "CppUnitTest.h"
#include "jchat.hpp"
#include "jcommon.hpp"
#include "resource.h"

#include <dinput.h>
#include <windows.h>
#include <detours.h>

#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(jchat_test) {

TEST_METHOD(test_DispatchMessage_detour) {
	using test_DispatchMessage_t = test_trampoline_t<LRESULT,const MSG*>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 日本語チャットバーがアクティブではないときに正しく動作できるか？
		test_helper_t help;
		DispatchMessage_target = &test_DispatchMessage_t::trampoline_api;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = HWND(101);
		test_DispatchMessage_t::target = [&](const MSG *msg) -> LRESULT {
			help << "DispatchMessage\n";
			help << int(msg) << '\n';
			return help.getSeq<LRESULT>();
		};
		api::GetActiveWindow = [&]() -> HWND {
			help << "GetActiveWindow\n";
			return HWND(102);
		};
		Assert::AreEqual(2, int(DispatchMessage_detour(help.getSeq<const MSG*>())));
		Assert::AreEqual(std::string("GetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("DispatchMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 日本語チャットバーがアクティブのときに正しく動作できるか？
		test_helper_t help;
		DispatchMessage_target = &test_DispatchMessage_t::trampoline_api;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = HWND(101);
		test_DispatchMessage_t::target = [&](const MSG *msg) -> LRESULT {
			help << "DispatchMessage\n";
			help << int(msg) << '\n';
			return help.getSeq<LRESULT>();
		};
		api::GetActiveWindow = [&]() -> HWND {
			help << "GetActiveWindow\n";
			return HWND(101);
		};
		api::TranslateMessage = [&](const MSG *lpMsg) -> BOOL {
			help << "TranslateMessage\n";
			help << int(lpMsg) << '\n';
			return TRUE;
		};
		Assert::AreEqual(2, int(DispatchMessage_detour(help.getSeq<const MSG*>())));
		Assert::AreEqual(std::string("GetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("TranslateMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DispatchMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_bindAddressAndDetourAttach_early) {
	{ // エラーをスローできるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "");
			return 0;
		};
		dtr::DetourTransactionAbort = [&]() -> LONG {
			help << "DetourTransactionAbort\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		try {
			bindAddressAndDetourAttach_early();
			Assert::Fail();
		} catch (const error_t &err) {
			Assert::AreEqual(string_printf("Offsetセクションの%sキーが不正です。", "CEverQuest"), err.getMessage());
		}
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionAbort"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		context.base_address = 0x400000 + help.getSeq();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, help.getSeqStr().c_str());
			return std::strlen(lpReturnedString);
		};
		dtr::DetourAttach_true = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourAttach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourDetach = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourDetach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		bindAddressAndDetourAttach_early();
		Assert::AreEqual(5, indirect_cast<int>(eqgame_t::CEverQuest));
		Assert::AreEqual(6, indirect_cast<int>(eqgame_t::CharSpawn));
		Assert::AreEqual(7, indirect_cast<int>(eqgame_t::MainWindowHandle));
		Assert::AreEqual(8, indirect_cast<int>(eqgame_t::NotInChatMode));
		Assert::AreEqual(9, indirect_cast<int>(eqgame_t::CEverQuest_SetGameState_target));
		Assert::AreEqual(10, indirect_cast<int>(eqgame_t::ProcessGameEvents_target));
		Assert::AreEqual(2, int(context.detour_exits.size()));
		context.detour_exits.clear();
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CharSpawn"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("MainWindowHandle"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("NotInChatMode"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_SetGameState"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_SetGameState_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_SetGameState_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("ProcessGameEvents"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::ProcessGameEvents_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::ProcessGameEvents_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_SetGameState_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_SetGameState_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::ProcessGameEvents_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::ProcessGameEvents_detour))), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_bindAddressAndDetourAttach_lazy) {
	{ // エラーをスローできるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "");
			return 0;
		};
		dtr::DetourAttach_true = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourAttach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourTransactionAbort = [&]() -> LONG {
			help << "DetourTransactionAbort\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		try {
			bindAddressAndDetourAttach_lazy();
			Assert::Fail();
		} catch (const error_t &err) {
			Assert::AreEqual(string_printf("Offsetセクションの%sキーが不正です。", "CEverQuest_InterpretCmd"), err.getMessage());
		}
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(DispatchMessage_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&DispatchMessage_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_InterpretCmd"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionAbort"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context.ini_path = help.getSeqStr();
		context.base_address = 0x400000 + help.getSeq();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, help.getSeqStr().c_str());
			return std::strlen(lpReturnedString);
		};
		dtr::DetourAttach_true = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourAttach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourDetach = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourDetach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		bindAddressAndDetourAttach_lazy();
		Assert::AreEqual(5, indirect_cast<int>(eqgame_t::CEverQuest_InterpretCmd_target));
		Assert::AreEqual(6, indirect_cast<int>(eqgame_t::CXWndManager_DrawCursor_target));
		Assert::AreEqual(7, indirect_cast<int>(eqgame_t::KeypressHandler_HandleKeyDown_target));
		Assert::AreEqual(8, indirect_cast<int>(eqgame_t::KeypressHandler_HandleKeyUp_target));
		Assert::AreEqual(5, int(context.detour_exits.size()));
		context.detour_exits.clear();
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(DispatchMessage_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&DispatchMessage_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_InterpretCmd"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_InterpretCmd_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_InterpretCmd_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CXWndManager_DrawCursor"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CXWndManager_DrawCursor_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CXWndManager_DrawCursor_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyDown_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyDown_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyUp_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyUp_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(DispatchMessage_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&DispatchMessage_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_InterpretCmd_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_InterpretCmd_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CXWndManager_DrawCursor_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CXWndManager_DrawCursor_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyDown_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyDown_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyUp_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyUp_detour))), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_context_fail) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // jchat.logの出力ストリームがないときに正しく動作できるか？
		test_helper_t help;
		context = {};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		context.fail(error_t(help.getSeqStr()));
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // jchat.logの出力ストリームがあるときに正しく動作できるか？
		test_helper_t help;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		context.fail(error_t(help.getSeqStr()));
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine().substr(24));
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_context_release) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // jchat.logの出力ストリームがないときに正しく動作できるか？
		test_helper_t help;
		context = {};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		context.release();
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // DetachのCommitに失敗したときに失敗できるか？
		test_helper_t help;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		context.detour_exits.emplace_back([&] {
			help << "detour_exits_procedure\n";
		});
		context.exits.emplace_back([&] {
			help << "exits_procedure\n";
		});
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		dtr::DetourTransactionAbort = [&]() -> LONG {
			help << "DetourTransactionAbort\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return ERROR_INVALID_OPERATION;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		context.release();
		Assert::AreEqual(0, int(context.detour_exits.size()));
		Assert::AreEqual(0, int(context.log.get()));
		Assert::AreEqual(0, int(context.exits.size()));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("detour_exits_procedure"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionAbort"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::dtr::DetourTransactionCommit", ERROR_INVALID_OPERATION), help.getLine().substr(24));
		Assert::AreEqual(std::string("exits_procedure"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		context.detour_exits.emplace_back([&] {
			help << "detour_exits_procedure\n";
		});
		context.exits.emplace_back([&] {
			help << "exits_procedure\n";
		});
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		context.release();
		Assert::AreEqual(0, int(context.detour_exits.size()));
		Assert::AreEqual(0, int(context.log.get()));
		Assert::AreEqual(0, int(context.exits.size()));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("detour_exits_procedure"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("exits_procedure"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_context_startup) {
	{ // jchat.iniのキーの値が空文字列であり、
	  // コマンドライン引数でパスが指定されていないときに正しく動作できるか？
		test_helper_t help;
		context = {};
		api::GetCommandLine = [&]() -> LPSTR {
			help << "GetCommandLine\n";
			return "";
		};
		api::GetModuleFileName_true = [&](
			HMODULE hModule,
			PTSTR pFilename,
			DWORD nSize
		) -> DWORD {
			help << "GetModuleFileName\n";
			help << int(hModule) << '\n';
			char *p = pFilename;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "\\"); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "."); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str());
			return std::strlen(pFilename);
		};
		api::GetModuleHandle_true = [&](LPCSTR lpModuleName) -> HMODULE {
			help << "GetModuleHandle\n";
			help << int(lpModuleName) << '\n';
			return HMODULE(0x400000 + help.getSeq());
		};
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "");
			return 0;
		};
		std_::make_ofstream_true = [&](
			const std::string &file, 
			std::ios_base::openmode mode
		) -> std::shared_ptr<std::ostream> {
			help << "make_ofstream\n";
			help << file << '\n';
			help << int(mode) << '\n';
			return help.getSeqPtr<std::ostream>();
		};
		std_::ostream_exceptions_set_true = [&](
			std::ostream *out, 
			std::ios::iostate except
		) {
			help << "ostream_exceptions_set\n";
			help << int(out) << '\n';
			help << int(except) << '\n';
		};
		context.startup(help.getSeq<HINSTANCE>());
		Assert::AreEqual(1, int(context.dll_handle));
		Assert::AreEqual(std::string("2\\3.4"), context.dll_path.string());
		Assert::AreEqual(std::string("2\\3.log"), context.log_path.string());
		Assert::AreEqual(std::string("2\\3.ini"), context.ini_path.string());
		Assert::AreEqual(std::string("/#^"), context.ini.chat.command_symbols);
		Assert::AreEqual(std::string("ＭＳ Ｐゴシック"), context.ini.chat.font_name);
		Assert::AreEqual(56, int(context.ini.chat.link_body_size));
		Assert::AreEqual(-10000, context.ini.window.left);
		Assert::AreEqual(150, int(context.ini.window.min_width));
		Assert::AreEqual(-10000, context.ini.window.top);
		Assert::AreEqual(300, int(context.ini.window.width));
		Assert::AreEqual(0x400006, int(context.base_address));
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("2\\3.log"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("CommandSymbols"), help.getLine());
		Assert::AreEqual(std::string("/#^"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("FontName"), help.getLine());
		Assert::AreEqual(std::string("ＭＳ Ｐゴシック"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("LinkBodySize"), help.getLine());
		Assert::AreEqual(std::string("56"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Left"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("MinWidth"), help.getLine());
		Assert::AreEqual(std::string("150"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Top"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Width"), help.getLine());
		Assert::AreEqual(std::string("300"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetModuleHandle"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // jchat.iniのキーの値が空文字列ではなく、
	  // コマンドライン引数でパスが指定されているときに正しく動作できるか？
		test_helper_t help;
		context = {};
		api::GetCommandLine = [&]() -> LPSTR {
			help << "GetCommandLine\n";
			return "3.log_path=hoge 3.ini_path=fuga";
		};
		api::GetModuleFileName_true = [&](
			HMODULE hModule,
			PTSTR pFilename,
			DWORD nSize
		) -> DWORD {
			help << "GetModuleFileName\n";
			help << int(hModule) << '\n';
			char *p = pFilename;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "\\"); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "."); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str());
			return std::strlen(pFilename);
		};
		api::GetModuleHandle_true = [&](LPCSTR lpModuleName) -> HMODULE {
			help << "GetModuleHandle\n";
			help << int(lpModuleName) << '\n';
			return HMODULE(0x400000 + help.getSeq());
		};
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			static const char *RESS[] = {
				"/#^@",
				"ＭＳ Ｐ明朝",
				"45",
				"10",
				"40",
				"20",
				"30",
			};
			static const char **res = RESS;
			std::strcpy(lpReturnedString, *res);
			return std::strlen(*res++);
		};
		std_::make_ofstream_true = [&](
			const std::string &file, 
			std::ios_base::openmode mode
		) -> std::shared_ptr<std::ostream> {
			help << "make_ofstream\n";
			help << file << '\n';
			help << int(mode) << '\n';
			return help.getSeqPtr<std::ostream>();
		};
		std_::ostream_exceptions_set_true = [&](
			std::ostream *out, 
			std::ios::iostate except
		) {
			help << "ostream_exceptions_set\n";
			help << int(out) << '\n';
			help << int(except) << '\n';
		};
		context.startup(help.getSeq<HINSTANCE>());
		Assert::AreEqual(1, int(context.dll_handle));
		Assert::AreEqual(std::string("2\\3.4"), context.dll_path.string());
		Assert::AreEqual(std::string("hoge"), context.log_path.string());
		Assert::AreEqual(std::string("fuga"), context.ini_path.string());
		Assert::AreEqual(std::string("/#^@"), context.ini.chat.command_symbols);
		Assert::AreEqual(std::string("ＭＳ Ｐ明朝"), context.ini.chat.font_name);
		Assert::AreEqual(45, int(context.ini.chat.link_body_size));
		Assert::AreEqual(10, context.ini.window.left);
		Assert::AreEqual(40, int(context.ini.window.min_width));
		Assert::AreEqual(20, context.ini.window.top);
		Assert::AreEqual(40, int(context.ini.window.width));
		Assert::AreEqual(0x400006, int(context.base_address));
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("hoge"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("CommandSymbols"), help.getLine());
		Assert::AreEqual(std::string("/#^"), help.getLine());
		Assert::AreEqual(std::string("fuga"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("FontName"), help.getLine());
		Assert::AreEqual(std::string("ＭＳ Ｐゴシック"), help.getLine());
		Assert::AreEqual(std::string("fuga"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("LinkBodySize"), help.getLine());
		Assert::AreEqual(std::string("56"), help.getLine());
		Assert::AreEqual(std::string("fuga"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Left"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("fuga"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("MinWidth"), help.getLine());
		Assert::AreEqual(std::string("150"), help.getLine());
		Assert::AreEqual(std::string("fuga"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Top"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("fuga"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Width"), help.getLine());
		Assert::AreEqual(std::string("300"), help.getLine());
		Assert::AreEqual(std::string("fuga"), help.getLine());
		Assert::AreEqual(std::string("GetModuleHandle"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_eqgame_CEverQuest_InterpretCmd_detour) {
	using test_CEverQuest_InterpretCmd_t = test_trampoline_t<void,eqgame_t*,char*>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // チャットモードではないときに正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		BYTE NotInChatMode = TRUE;
		eqgame_t::NotInChatMode = &NotInChatMode;
		eqgame_t::CEverQuest_InterpretCmd_target = 
			indirect_cast<eqgame_t::CEverQuest_InterpretCmd_t>(&test_CEverQuest_InterpretCmd_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		test_CEverQuest_InterpretCmd_t::target = [&](eqgame_t *player, char *cmd) {
			help << "CEverQuest_InterpretCmd\n";
			help << int(player) << '\n';
			help << cmd << '\n';
		};
		help.setSeqBase();
		game->CEverQuest_InterpretCmd_detour(help.getSeq<eqgame_t*>(1), help.getSeqStr(2).data());
		Assert::AreEqual(std::string("CEverQuest_InterpretCmd"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // チャットモードのときに正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		BYTE NotInChatMode = FALSE;
		eqgame_t::NotInChatMode = &NotInChatMode;
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		context.ini.chat.link_body_size = 2;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << LPTSTR(lParam) << '\n';
			return 0;
		};
		help.setSeqBase();
		game->CEverQuest_InterpretCmd_detour(help.getSeq<eqgame_t*>(1), help.getSeqStr(2).data());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(EM_REPLACESEL), help.getLine());
		Assert::AreEqual(stringize(TRUE), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_eqgame_CEverQuest_SetGameState_detour) {
	using test_CEverQuest_SetGameState_t = test_trampoline_t<void,int>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作するか？
		test_helper_t help;
		eqgame_t::CEverQuest_SetGameState_target = 
			indirect_cast<eqgame_t::CEverQuest_SetGameState_t>(&test_CEverQuest_SetGameState_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		context.game_state = 0;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		test_CEverQuest_SetGameState_t::target = [&](int game_state) -> void {
			help << "CEverQuest_SetGameState_target\n";
			help << game_state << '\n';
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		game->CEverQuest_SetGameState_detour(GAMESTATE_CHARSELECT);
		Assert::AreEqual(GAMESTATE_CHARSELECT, context.game_state);
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_SetGameState_target"), help.getLine());
		Assert::AreEqual(stringize(GAMESTATE_CHARSELECT), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_eqgame_CXWndManager_DrawCursor_detour) {
	using test_CXWndManager_DrawCursor_t = test_trampoline_t<int>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 失敗できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		eqgame_t::CXWndManager_DrawCursor_target = 
			indirect_cast<eqgame_t::CXWndManager_DrawCursor_t>(&test_CXWndManager_DrawCursor_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		test_CXWndManager_DrawCursor_t::target = [&]() -> int {
			help << "CXWndManager_DrawCursor_target\n";
			return help.getSeq();
		};
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			return FALSE;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::AreEqual(6, game->CXWndManager_DrawCursor_detour());
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetCursorPos", 5), help.getLine().substr(24));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("CXWndManager_DrawCursor_target"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // マウスカーソルが日本語チャットバーの上にないときに正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		eqgame_t::CXWndManager_DrawCursor_target = 
			indirect_cast<eqgame_t::CXWndManager_DrawCursor_t>(&test_CXWndManager_DrawCursor_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		test_CXWndManager_DrawCursor_t::target = [&]() -> int {
			help << "CXWndManager_DrawCursor_target\n";
			return help.getSeq();
		};
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			*lpPoint = POINT{9, 9};
			return TRUE;
		};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{10, 10, 20, 20};
			return TRUE;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		Assert::AreEqual(4, game->CXWndManager_DrawCursor_detour());
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("CXWndManager_DrawCursor_target"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // マウスカーソルが日本語チャットバーの上にあるときに正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			*lpPoint = POINT{10, 10};
			return TRUE;
		};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{10, 10, 20, 20};
			return TRUE;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		Assert::AreEqual(0, game->CXWndManager_DrawCursor_detour());
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_eqgame_KeypressHandler_HandleKeyDown_detour) {
	using test_KeypressHandler_HandleKeyDown_t = test_trampoline_t<bool,const KeyCombo&>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // キーが無視されるか？
		test_helper_t help;
		BYTE NotInChatMode = TRUE;
		eqgame_t::NotInChatMode = &NotInChatMode;
		eqgame_t *game = help.getSeq<eqgame_t*>();
		Assert::IsTrue(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 0, DIK_RETURN}));
		Assert::IsTrue(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 0, DIK_SLASH}));
		Assert::IsTrue(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 1, DIK_UP}));
		Assert::IsTrue(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 1, DIK_DOWN}));
	}
	{ // キーが無視されないか？
		test_helper_t help;
		BYTE NotInChatMode;
		eqgame_t::NotInChatMode = &NotInChatMode;
		eqgame_t::KeypressHandler_HandleKeyDown_target = 
			indirect_cast<eqgame_t::KeypressHandler_HandleKeyDown_t>(&test_KeypressHandler_HandleKeyDown_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		test_KeypressHandler_HandleKeyDown_t::target = [&](const KeyCombo &key_combo) -> bool {
			help << "KeypressHandler_HandleKeyDown_target\n";
			help << int(key_combo.alt) << '\n';
			help << int(key_combo.ctrl) << '\n';
			help << int(key_combo.shift) << '\n';
			help << int(key_combo.key) << '\n';
			return help.getSeq() % 2;
		};
		NotInChatMode = FALSE;
		Assert::IsFalse(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 0, DIK_RETURN}));
		Assert::IsTrue (game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 0, DIK_SLASH}));
		Assert::IsFalse(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 1, DIK_UP}));
		Assert::IsTrue (game->KeypressHandler_HandleKeyDown_detour (KeyCombo{0, 0, 1, DIK_DOWN}));
		NotInChatMode = TRUE;
		Assert::IsFalse(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 1, 0, DIK_RETURN}));
		Assert::IsTrue (game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 1, DIK_SLASH}));
		Assert::IsFalse(game->KeypressHandler_HandleKeyDown_detour(KeyCombo{0, 0, 0, DIK_A}));
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_RETURN), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_SLASH), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(DIK_UP), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(DIK_DOWN), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_RETURN), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(DIK_SLASH), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_A), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
	
TEST_METHOD(test_eqgame_KeypressHandler_HandleKeyUp_detour) {
	using test_KeypressHandler_HandleKeyUp_t = test_trampoline_t<bool,const KeyCombo&>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 失敗できるか？
		test_helper_t help;
		BYTE NotInChatMode = FALSE;
		eqgame_t::NotInChatMode = &NotInChatMode;
		eqgame_t::KeypressHandler_HandleKeyUp_target = 
			indirect_cast<eqgame_t::KeypressHandler_HandleKeyUp_t>(&test_KeypressHandler_HandleKeyUp_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		test_KeypressHandler_HandleKeyUp_t::target = [&](const KeyCombo &key_combo) -> bool {
			help << "KeypressHandler_HandleKeyUp_target\n";
			help << int(key_combo.alt) << '\n';
			help << int(key_combo.ctrl) << '\n';
			help << int(key_combo.shift) << '\n';
			help << int(key_combo.key) << '\n';
			return help.getSeq() % 2;
		};
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return NULL;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::IsTrue(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 0, DIK_RETURN}));
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetActiveWindow", 4), help.getLine().substr(24));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_RETURN), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // チャットバーがアクティブになるか？
		test_helper_t help;
		BYTE NotInChatMode;
		eqgame_t::NotInChatMode = &NotInChatMode;
		eqgame_t::KeypressHandler_HandleKeyUp_target = 
			indirect_cast<eqgame_t::KeypressHandler_HandleKeyUp_t>(&test_KeypressHandler_HandleKeyUp_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		NotInChatMode = TRUE;
		Assert::IsTrue(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 0, DIK_RETURN}));
		Assert::IsTrue(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 0, DIK_SLASH}));
		Assert::IsTrue(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 1, DIK_UP}));
		Assert::IsTrue(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 1, DIK_DOWN}));
		NotInChatMode = FALSE;
		Assert::IsTrue(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 0, DIK_RETURN}));
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("/"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // チャットバーがアクティブにならないか？
		test_helper_t help;
		BYTE NotInChatMode;
		eqgame_t::NotInChatMode = &NotInChatMode;
		eqgame_t::KeypressHandler_HandleKeyUp_target = 
			indirect_cast<eqgame_t::KeypressHandler_HandleKeyUp_t>(&test_KeypressHandler_HandleKeyUp_t::trampoline_member);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		test_KeypressHandler_HandleKeyUp_t::target = [&](const KeyCombo &key_combo) -> bool {
			help << "KeypressHandler_HandleKeyUp_target\n";
			help << int(key_combo.alt) << '\n';
			help << int(key_combo.ctrl) << '\n';
			help << int(key_combo.shift) << '\n';
			help << int(key_combo.key) << '\n';
			return help.getSeq() % 2;
		};
		NotInChatMode = FALSE;
		Assert::IsFalse(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 0, DIK_SLASH}));
		Assert::IsTrue (game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 1, DIK_UP}));
		Assert::IsFalse(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 1, DIK_DOWN}));
		NotInChatMode = TRUE;
		Assert::IsTrue (game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 1, 0, DIK_RETURN}));
		Assert::IsFalse(game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 1, DIK_SLASH}));
		Assert::IsTrue (game->KeypressHandler_HandleKeyUp_detour(KeyCombo{0, 0, 0, DIK_A}));
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_SLASH), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(DIK_UP), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(DIK_DOWN), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_RETURN), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(DIK_SLASH), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp_target"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(DIK_A), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_eqgame_ProcessGameEvents_detour) {
	using test_ProcessGameEvents_t = test_trampoline_t<BOOL>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作するか？
		test_helper_t help;
		EVERQUEST CEverQuest;
		CEverQuest.GameState = GAMESTATE_CHARSELECT;
		eqgame_t *CEverQuest_ptr = (eqgame_t*)(&CEverQuest);
		eqgame_t::CEverQuest = &CEverQuest_ptr;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		eqgame_t::ProcessGameEvents_target = 
			indirect_cast<eqgame_t::ProcessGameEvents_t>(&test_ProcessGameEvents_t::trampoline);
		eqgame_t *game = help.getSeq<eqgame_t*>();
		context = {};
		context.game_state = 0;
		context.screen_mode = screen_modes::ICONIC;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		test_ProcessGameEvents_t::target = [&]() -> BOOL {
			help << "ProcessGameEvents_target\n";
			return TRUE;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		Assert::IsTrue(game->ProcessGameEvents_detour());
		Assert::AreEqual(GAMESTATE_CHARSELECT, context.game_state);
		Assert::AreEqual(int(screen_modes::ICONIC), int(context.screen_mode));
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("ProcessGameEvents_target"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
		
TEST_METHOD(test_getAddress) {
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		context.base_address = 0x400000 + help.getSeq();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, help.getSeqStr().c_str());
			return std::strlen(lpReturnedString);
		};
		Assert::AreEqual(2 + 4, int(getAddress(help.getSeqStr())));
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_getOffset) {
	{ // キーの値が設定されていないときにエラーをスローできるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "");
			return 0;
		};
		try {
			getOffset(help.getSeqStr());
			Assert::Fail();
		} catch (const error_t &err) {
			Assert::AreEqual(string_printf("Offsetセクションの%sキーが不正です。", "2"), err.getMessage());
		}
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // キーの値が不正のときにエラーをスローできるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "g");
			return 1;
		};
		try {
			getOffset(help.getSeqStr());
			Assert::Fail();
		} catch (const error_t &err) {
			Assert::AreEqual(string_printf("Offsetセクションの%sキーが不正です。", "2"), err.getMessage());
		}
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "f");
			return 1;
		};
		Assert::AreEqual(0xf, int(getOffset(help.getSeqStr())));
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
			
TEST_METHOD(test_getScreenMode) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // アイコンモードを取得できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		Assert::AreEqual(int(screen_modes::ICONIC), int(getScreenMode()));
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // フルスクリーンモードを取得できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		api::GetWindowLong_true = [&](HWND hWnd, int nIndex) -> LONG {
			help << "GetWindowLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			return WS_EX_TOPMOST;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		Assert::AreEqual(int(screen_modes::FULL_SCREEN), int(getScreenMode()));
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(GWL_EXSTYLE), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // ウインドウモードを取得できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return 0;
		};
		api::GetWindowLong_true = [&](HWND hWnd, int nIndex) -> LONG {
			help << "GetWindowLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			return 0;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		Assert::AreEqual(int(screen_modes::WINDOW), int(getScreenMode()));
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(GWL_EXSTYLE), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_WindowProc) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // メッセージマップにないメッセージのときに正しく動作するか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::DefWindowProc = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "DefWindowProc\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return help.getSeq<LRESULT>();
		};
		Assert::AreEqual(2, int(WindowProc(bar->data->handle, WM_USER, 0, 0)));
		Assert::AreEqual(std::string("DefWindowProc"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_USER), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // WM_CREATEのハンドラがエラーをスローしたときに失敗できるか？
		test_helper_t help;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		context.dll_handle = help.getSeq<HINSTANCE>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::LoadBitmap_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpBitmapName
		) -> HBITMAP {
			help << "LoadBitmap\n";
			help << int(hInstance) << '\n';
			help << int(lpBitmapName) << '\n';
			return NULL;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::AreEqual(-1, int(WindowProc(bar->data->handle, WM_CREATE, 0, 0)));
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ROCK_BAR))), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::LoadBitmapA", 4), help.getLine().substr(24));
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // WM_SETFOCUSのハンドラがエラーをスローしたときに失敗できるか？
		test_helper_t help;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.handle = help.getSeq<HWND>();
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetFocus_true = [&](HWND hWnd) -> HWND {
			help << "SetFocus\n";
			help << int(hWnd) << '\n';
			return NULL;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::AreEqual(0, int(WindowProc(bar->data->handle, WM_SETFOCUS, 0, 0)));
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetFocus", 4), help.getLine().substr(24));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SetFocus_true = [&](HWND hWnd) -> HWND {
			help << "SetFocus\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		Assert::AreEqual(0, int(WindowProc(bar->data->handle, WM_SETFOCUS, 0, 0)));
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_activate) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // Enterキーを押されたときにアクティブにできるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		bar->activate(KeyCombo{0, 0, 0, DIK_RETURN});
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // スラッシュキーを押されたときにアクティブにできるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		bar->activate(KeyCombo{0, 0, 0, DIK_SLASH});
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("/"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_channel_WindowProc_sub) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // メッセージマップにないメッセージのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		bar->data->channel.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return help.getSeq<LRESULT>();
		};
		Assert::AreEqual(3, int(channel_WindowProc_sub(bar->data->channel.handle, WM_USER, 0, 0)));
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_USER), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // WM_CHARのハンドラがエラーをスローしたときに失敗できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->channel.handle = help.getSeq<HWND>();
		bar->data->channel.WindowProc_super = help.getSeq<WNDPROC>();
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return NULL;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::AreEqual(0, int(channel_WindowProc_sub(bar->data->channel.handle, WM_CHAR, VK_ESCAPE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetActiveWindow", 6), help.getLine().substr(24));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		bar->data->channel.WindowProc_super = help.getSeq<WNDPROC>();
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		Assert::AreEqual(0, int(channel_WindowProc_sub(bar->data->channel.handle, WM_CHAR, VK_ESCAPE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
	
TEST_METHOD(test_jchat_bar_channel_create) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.dll_handle = help.getSeq<HINSTANCE>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->hfont = help.getSeq<HFONT>();
		api::CreateWindowEx_true = [&](
			DWORD dwExStyle, 
			PCTSTR pszClassName, 
			PCTSTR pszWindowName, 
			DWORD dwStyle, 
			int x, 
			int y, 
			int nWidth, 
			int nHeight, 
			HWND hWndParent, 
			HMENU hMenu, 
			HINSTANCE hInstance, 
			PVOID pParam
		) -> HWND {
			help << "CreateWindowEx\n";
			help << dwExStyle << '\n';
			help << pszClassName << '\n';
			if (pszWindowName) help << pszWindowName << '\n';
			else help << "(null)\n";
			help << dwStyle << '\n';
			help << x << '\n';
			help << y << '\n';
			help << nWidth << '\n';
			help << nHeight << '\n';
			help << int(hWndParent) << '\n';
			help << int(hMenu) << '\n';
			help << int(hInstance) << '\n';
			help << int(pParam) << '\n';
			return help.getSeq<HWND>();
		};
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			help << int(hInstance) << '\n';
			help << int(lpCursorName) << '\n';
			return help.getSeq<HCURSOR>();
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			if (Msg == CB_ADDSTRING) help << LPCTSTR(lParam) << '\n';
			else help << lParam << '\n';
			return 0;
		};
		api::SetClassLong_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG dwNewLong
		) -> DWORD {
			help << "SetClassLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << dwNewLong << '\n';
			return help.getSeq<DWORD>();
		};
		api::SetWindowLongPtr_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG_PTR dwNewLong
		) -> LONG_PTR {
			help << "SetWindowLongPtr\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << dwNewLong << '\n';
			return help.getSeq<LONG_PTR>();
		};
		bar->channel_create(help.getSeq<HWND>());
		Assert::AreEqual(4, int(bar->data->channel.handle));
		Assert::AreEqual(5, int(bar->data->channel.WindowProc_super));
		Assert::AreEqual(1, int(bar->data->ordered_handles.size()));
		Assert::AreEqual(4, int(bar->data->ordered_handles[0]));
		Assert::AreEqual(1, int(bar->data->ordered_indices.size()));
		Assert::AreEqual(0, int(bar->data->ordered_indices[HWND(4)]));
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("COMBOBOX"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(stringize(CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE), help.getLine());
		Assert::AreEqual(stringize(ICON_WIDTH), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_TOP), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_WIDTH), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_ID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string("SetWindowLongPtr"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(GWLP_WNDPROC), help.getLine());
		Assert::AreEqual(stringize(LONG_PTR(&channel_WindowProc_sub)), help.getLine());
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_ARROW))), help.getLine());
		Assert::AreEqual(std::string("SetClassLong"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(GCL_HCURSOR), help.getLine());
		Assert::AreEqual(stringize("6"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(WM_SETFONT), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Auction"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Gsay"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/gUildsay"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Ooc"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Reply"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Say"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/sHout"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_channel_onChar) {
	using test_CEverQuest_InterpretCmd_t = test_trampoline_t<void,eqgame_t*,char*>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // スペースが入力されたときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		bar->data->channel.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		Assert::AreEqual(0, int(bar->channel_onChar(bar->data->channel.handle, WM_CHAR, VK_SPACE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_CHAR), help.getLine());
		Assert::AreEqual(stringize(VK_SPACE), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_channel_onSysKeyDown) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		bar->data->channel.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		Assert::AreEqual(0, int(bar->channel_onSysKeyDown(bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('X'), 0)));
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_SYSKEYDOWN), help.getLine());
		Assert::AreEqual(stringize(int('X')), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_channel_select) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return help.getSeq<LRESULT>();
		};
		bar->channel_select(help.getSeq());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
		
TEST_METHOD(test_jchat_bar_create) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.dll_handle = help.getSeq<HINSTANCE>();
		context.ini.window.left = help.getSeq();
		context.ini.window.top = help.getSeq();
		context.ini.window.width = help.getSeq<std::size_t>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			lpPoint->x = help.getSeq<LONG>();
			lpPoint->y = help.getSeq<LONG>();
			return TRUE;
		};
		api::CreateWindowEx_true = [&](
			DWORD dwExStyle, 
			PCTSTR pszClassName, 
			PCTSTR pszWindowName, 
			DWORD dwStyle, 
			int x, 
			int y, 
			int nWidth, 
			int nHeight, 
			HWND hWndParent, 
			HMENU hMenu, 
			HINSTANCE hInstance, 
			PVOID pParam
		) -> HWND {
			help << "CreateWindowEx\n";
			help << dwExStyle << '\n';
			help << pszClassName << '\n';
			help << pszWindowName << '\n';
			help << dwStyle << '\n';
			help << x << '\n';
			help << y << '\n';
			help << nWidth << '\n';
			help << nHeight << '\n';
			help << int(hWndParent) << '\n';
			help << int(hMenu) << '\n';
			help << int(hInstance) << '\n';
			help << int(pParam) << '\n';
			return help.getSeq<HWND>();
		};
		bar->create();
		Assert::AreEqual(8, int(bar->data->handle));
		Assert::AreEqual(3, int(bar->data->mw_client_pos.x));
		Assert::AreEqual(4, int(bar->data->mw_client_pos.y));
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(stringize(WS_POPUP), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_ctl_onChar) {
	using test_CEverQuest_InterpretCmd_t = test_trampoline_t<void,eqgame_t*,char*>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // Enterが入力されたときに正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		eqgame_t *ever_quest = help.getSeq<eqgame_t*>();
		eqgame_t::CEverQuest = &ever_quest;
		eqgame_t::CEverQuest_InterpretCmd_target = 
			indirect_cast<eqgame_t::CEverQuest_InterpretCmd_t>(&test_CEverQuest_InterpretCmd_t::trampoline_member);
		eqgame_t *char_spawn = help.getSeq<eqgame_t*>();
		eqgame_t::CharSpawn = &char_spawn;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->channel.handle = help.getSeq<HWND>();
		test_CEverQuest_InterpretCmd_t::target = [&](eqgame_t *player, char *cmd) {
			help << "CEverQuest_InterpretCmd\n";
			help << int(player) << '\n';
			help << cmd << '\n';
		};
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			std::strcpy(lpString, help.getSeqStr().c_str());
			return std::strlen(lpString);
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return LRESULT(5);
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << '\'' << lpString << "'\n";
			return TRUE;
		};
		Assert::AreEqual(0, int(bar->ctl_onChar(help.getSeq<WNDPROC>(), bar->data->channel.handle, WM_CHAR, VK_RETURN, 0)));
		Assert::AreEqual(1, int(bar->data->history.lines.size()));
		Assert::AreEqual(std::string("7"), bar->data->history.lines.back());
		Assert::AreEqual(0, int(bar->data->registry.lines.size()));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_InterpretCmd"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("/Say 7"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // Escが入力されたときに正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return HWND(12);
		};
		Assert::AreEqual(0, int(bar->ctl_onChar(help.getSeq<WNDPROC>(), bar->data->channel.handle, WM_CHAR, VK_ESCAPE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // スペースが入力されたときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		Assert::AreEqual(0, int(bar->ctl_onChar(help.getSeq<WNDPROC>(), bar->data->channel.handle, WM_CHAR, VK_SPACE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_CHAR), help.getLine());
		Assert::AreEqual(stringize(VK_SPACE), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_ctl_onSysKeyDown) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		bar->data->channel.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return help.getSeq<LRESULT>();
		};
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('A'), 0)));
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('G'), 0)));
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('U'), 0)));
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('O'), 0)));
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('R'), 0)));
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('S'), 0)));
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('H'), 0)));
		Assert::AreEqual(0, int(bar->ctl_onSysKeyDown(bar->data->channel.WindowProc_super, bar->data->channel.handle, WM_SYSKEYDOWN, WPARAM('X'), 0)));
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_SYSKEYDOWN), help.getLine());
		Assert::AreEqual(stringize(int('X')), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_data) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく破棄できるか？
		test_helper_t help;
		{
			data_t data;
			data.exits.emplace_back([&] {
				help << "exits\n";
				help << "1" << '\n';
			});
			data.exits.emplace_back([&] {
				help << "exits\n";
				help << "2" << '\n';
			});
		}
		Assert::AreEqual(std::string("exits"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("exits"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_deleteFromList) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // リストが空っぽのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.last_line = help.getSeqStr();
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << "'" << lpString << "'\n";
			return TRUE;
		};
		bar->deleteFromList();
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 今までの発言リストの最後の行を削除できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->history.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->history.moveEnd();
		--bar->data->history.iter;
		bar->data->registry.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->registry.moveEnd();
		bar->data->edit.last_line = help.getSeqStr();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		bar->deleteFromList();
		Assert::AreEqual(1, int(bar->data->history.lines.size()));
		Assert::AreEqual(std::string("2"), bar->data->history.lines.front());
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::AreEqual(2, int(bar->data->registry.lines.size()));
		Assert::IsTrue(bar->data->registry.isEnd());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 今までの発言リストの最初の行を削除できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->history.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->history.moveBegin();
		bar->data->registry.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->registry.moveEnd();
		bar->data->edit.last_line = help.getSeqStr();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		bar->deleteFromList();
		Assert::AreEqual(1, int(bar->data->history.lines.size()));
		Assert::AreEqual(std::string("3"), *bar->data->history.iter);
		Assert::AreEqual(2, int(bar->data->registry.lines.size()));
		Assert::IsTrue(bar->data->registry.isEnd());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // よく使う発言リストの最後の行を削除できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->history.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->history.moveEnd();
		bar->data->registry.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->registry.moveEnd();
		--bar->data->registry.iter;
		bar->data->edit.last_line = help.getSeqStr();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		bar->deleteFromList();
		Assert::AreEqual(2, int(bar->data->history.lines.size()));
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::AreEqual(1, int(bar->data->registry.lines.size()));
		Assert::AreEqual(std::string("4"), bar->data->registry.lines.front());
		Assert::IsTrue(bar->data->registry.isEnd());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // よく使う発言リストの最初の行を削除できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->history.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->history.moveEnd();
		bar->data->registry.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->registry.moveBegin();
		bar->data->edit.last_line = help.getSeqStr();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		bar->deleteFromList();
		Assert::AreEqual(2, int(bar->data->history.lines.size()));
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::AreEqual(1, int(bar->data->registry.lines.size()));
		Assert::AreEqual(std::string("5"), *bar->data->registry.iter);
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_drawEdge) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		api::CreatePen_true = [&](
			int iStyle, 
			int cWidth, 
			COLORREF color
		) -> HPEN {
			help << "CreatePen\n";
			help << int(color) << '\n';
			return help.getSeq<HPEN>();
		};
		api::DeleteObject = [&](HGDIOBJ ho) -> BOOL {
			help << "DeleteObject\n";
			help << int(ho) << '\n';
			return TRUE;
		};
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			help << int(hdc) << '\n';
			help << int(h) << '\n';
			return help.getSeq<HGDIOBJ>();
		};
		api::Polyline_true = [&](
			HDC hdc, 
			const POINT *apt, 
			int cpt
		) -> BOOL {
			help << "Polyline\n";
			help << int(hdc) << '\n';
			for (int i = 0; i < cpt; ++i) 
				help << '(' << apt[i].x  << ',' << apt[i].y << ")\n";
			return TRUE;
		};
		help.setSeqBase();
		bar->drawEdge(
			help.getSeq<HDC>(1), 
			help.getSeq(2), 
			help.getSeq(3), 
			help.getSeq(4), 
			help.getSeq(5), 
			help.getSeq<COLORREF>(6), 
			help.getSeq<COLORREF>(7)
		);
		Assert::AreEqual(2, int(bar->data->pen_cache.size()));
		Assert::AreEqual(8, int(bar->data->pen_cache.at(COLORREF(6))));
		Assert::AreEqual(10, int(bar->data->pen_cache.at(COLORREF(7))));
		Assert::AreEqual(2, int(bar->data->exits.size()));
		reverseClear(bar->data->exits);
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(COLORREF(6)), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("(2,7)"), help.getLine());
		Assert::AreEqual(std::string("(2,3)"), help.getLine());
		Assert::AreEqual(std::string("(6,3)"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(COLORREF(7)), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("(5,4)"), help.getLine());
		Assert::AreEqual(std::string("(5,7)"), help.getLine());
		Assert::AreEqual(std::string("(2,7)"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_WindowProc_sub) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // メッセージマップにないメッセージのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return help.getSeq<LRESULT>();
		};
		Assert::AreEqual(3, int(edit_WindowProc_sub(bar->data->edit.handle, WM_USER, 0, 0)));
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_USER), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // WM_CHARのハンドラがエラーをスローしたときに失敗できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.WindowProc_super = help.getSeq<WNDPROC>();
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return NULL;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::AreEqual(0, int(edit_WindowProc_sub(bar->data->edit.handle, WM_CHAR, VK_ESCAPE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetActiveWindow", 6), help.getLine().substr(24));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.WindowProc_super = help.getSeq<WNDPROC>();
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		Assert::AreEqual(0, int(edit_WindowProc_sub(bar->data->edit.handle, WM_CHAR, VK_ESCAPE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_create) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.dll_handle = help.getSeq<HINSTANCE>();
		context.ini.window.width = (EDIT_HOR_MARGIN + 2 + 2) + help.getSeq();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->hfont = help.getSeq<HFONT>();
		api::CreateWindowEx_true = [&](
			DWORD dwExStyle, 
			PCTSTR pszClassName, 
			PCTSTR pszWindowName, 
			DWORD dwStyle, 
			int x, 
			int y, 
			int nWidth, 
			int nHeight, 
			HWND hWndParent, 
			HMENU hMenu, 
			HINSTANCE hInstance, 
			PVOID pParam
		) -> HWND {
			help << "CreateWindowEx\n";
			help << dwExStyle << '\n';
			help << pszClassName << '\n';
			if (pszWindowName) help << pszWindowName << '\n';
			else help << "(null)\n";
			help << dwStyle << '\n';
			help << x << '\n';
			help << y << '\n';
			help << nWidth << '\n';
			help << nHeight << '\n';
			help << int(hWndParent) << '\n';
			help << int(hMenu) << '\n';
			help << int(hInstance) << '\n';
			help << int(pParam) << '\n';
			return help.getSeq<HWND>();
		};
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			help << int(hInstance) << '\n';
			help << int(lpCursorName) << '\n';
			return help.getSeq<HCURSOR>();
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetClassLong_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG dwNewLong
		) -> DWORD {
			help << "SetClassLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << dwNewLong << '\n';
			return help.getSeq<DWORD>();
		};
		api::SetWindowLongPtr_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG_PTR dwNewLong
		) -> LONG_PTR {
			help << "SetWindowLongPtr\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << dwNewLong << '\n';
			return help.getSeq<LONG_PTR>();
		};
		bar->edit_create(help.getSeq<HWND>());
		Assert::AreEqual(5, int(bar->data->edit.handle));
		Assert::AreEqual(6, int(bar->data->edit.WindowProc_super));
		Assert::AreEqual(1, int(bar->data->ordered_handles.size()));
		Assert::AreEqual(5, int(bar->data->ordered_handles[0]));
		Assert::AreEqual(1, int(bar->data->ordered_indices.size()));
		Assert::AreEqual(0, int(bar->data->ordered_indices[HWND(5)]));
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("EDIT"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(stringize(ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_VISIBLE), help.getLine());
		Assert::AreEqual(stringize(EDIT_LEFT + 2), help.getLine());
		Assert::AreEqual(stringize(EDIT_TOP + 4), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(EDIT_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(EDIT_ID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string("SetWindowLongPtr"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(GWLP_WNDPROC), help.getLine());
		Assert::AreEqual(stringize(LONG_PTR(&edit_WindowProc_sub)), help.getLine());
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_IBEAM))), help.getLine());
		Assert::AreEqual(std::string("SetClassLong"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(GCL_HCURSOR), help.getLine());
		Assert::AreEqual(stringize("7"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(WM_SETFONT), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_getLine) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 空文字列の行を取得できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return 0;
		};
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			std::strcpy(lpString, "");
			return 0;
		};
		Assert::AreEqual(std::string(), bar->edit_getLine());
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			std::strcpy(lpString, help.getSeqStr().c_str());
			return std::strlen(lpString);
		};
		Assert::AreEqual(std::string("2"), bar->edit_getLine());
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_onChar) {
	using test_CEverQuest_InterpretCmd_t = test_trampoline_t<void,eqgame_t*,char*>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // Ctrl+Aキーが押されたときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		Assert::AreEqual(0, int(bar->edit_onChar(bar->data->edit.handle, WM_CHAR, getCtrlAlphabetKey('A'), 0)));
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // スペースキーが押されたときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		Assert::AreEqual(0, int(bar->edit_onChar(bar->data->edit.handle, WM_CHAR, VK_SPACE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_CHAR), help.getLine());
		Assert::AreEqual(stringize(VK_SPACE), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_onKeyDown) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 最後の行を選択しようと↑キーが押されたときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->history.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->history.moveEnd();
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return 0;
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		Assert::AreEqual(0, int(bar->edit_onKeyDown(bar->data->edit.handle, WM_KEYDOWN, VK_UP, 0)));
		Assert::AreEqual(2, int(bar->data->history.lines.size()));
		Assert::AreEqual(std::string("3"), *bar->data->history.iter);
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 最後の行を削除しようとShift+Deleteキーが押されたときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->history.lines = {help.getSeqStr(), help.getSeqStr()};
		bar->data->history.moveEnd();
		--bar->data->history.iter;
		bar->data->edit.last_line = help.getSeqStr();
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return -1;
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		Assert::AreEqual(0, int(bar->edit_onKeyDown(bar->data->edit.handle, WM_KEYDOWN, VK_DELETE, 0)));
		Assert::AreEqual(1, int(bar->data->history.lines.size()));
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // スペースキーが押されたときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::GetKeyState = [&](int nVirtKey) -> SHORT {
			help << "GetKeyState\n";
			help << nVirtKey << '\n';
			return -1;
		};
		Assert::AreEqual(0, int(bar->edit_onKeyDown(bar->data->edit.handle, WM_KEYDOWN, VK_SPACE, 0)));
		Assert::AreEqual(std::string("GetKeyState"), help.getLine());
		Assert::AreEqual(stringize(VK_SHIFT), help.getLine());
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_KEYDOWN), help.getLine());
		Assert::AreEqual(stringize(VK_SPACE), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_onSysKeyDown) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.WindowProc_super = help.getSeq<WNDPROC>();
		api::CallWindowProc = [&](
			WNDPROC lpPrevWndFunc, 
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "CallWindowProc\n";
			help << int(lpPrevWndFunc) << '\n';
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		Assert::AreEqual(0, int(bar->edit_onSysKeyDown(bar->data->edit.handle, WM_SYSKEYDOWN, WPARAM('X'), 0)));
		Assert::AreEqual(std::string("CallWindowProc"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(WM_SYSKEYDOWN), help.getLine());
		Assert::AreEqual(stringize(int('X')), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_replaceText) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << LPTSTR(lParam) << '\n';
			return 0;
		};
		bar->edit_replaceText(help.getSeqStr());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_REPLACESEL), help.getLine());
		Assert::AreEqual(stringize(TRUE), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
	
TEST_METHOD(test_jchat_bar_edit_selectText) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 全テキストを選択できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		bar->edit_selectText();
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 一部のテキストを選択できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		help.setSeqBase();
		bar->edit_selectText(help.getSeq(1), help.getSeq(2));
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_edit_setLine) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 空っぽの行を編集モードで設定できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.dirty = true;
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << '\'' << lpString << "'\n";
			help << int(bar->data->edit.change_mode) << '\n';
			return TRUE;
		};
		bar->edit_setLine();
		Assert::AreEqual(int(change_modes::EDIT), int(bar->data->edit.change_mode));
		Assert::IsTrue(bar->data->edit.dirty);
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(stringize(int(change_modes::EDIT)), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 空っぽではない行を選択モードで設定できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->edit.dirty = true;
		api::GetWindowText_true = [&](
			HWND hWnd,
			LPSTR lpString,
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			std::strcpy(lpString, help.getSeqStr().c_str());
			return std::strlen(lpString);
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			help << int(bar->data->edit.change_mode) << '\n';
			return TRUE;
		};
		bar->edit_setLine(help.getSeqStr(), change_modes::CHOICE);
		Assert::AreEqual(int(change_modes::EDIT), int(bar->data->edit.change_mode));
		Assert::IsFalse(bar->data->edit.dirty);
		Assert::AreEqual(std::string("3"), bar->data->edit.last_line);
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(int(change_modes::CHOICE)), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_execute) {
	using test_CEverQuest_InterpretCmd_t = test_trampoline_t<void,eqgame_t*,char*>;
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 空っぽの行を実行できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return 0;
		};
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			std::strcpy(lpString, "");
			return 0;
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		bar->execute(0);
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 空っぽではない行を実行できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		eqgame_t *ever_quest = help.getSeq<eqgame_t*>();
		eqgame_t::CEverQuest = &ever_quest;
		eqgame_t::CEverQuest_InterpretCmd_target = 
			indirect_cast<eqgame_t::CEverQuest_InterpretCmd_t>(&test_CEverQuest_InterpretCmd_t::trampoline_member);
		eqgame_t *CharSpawn = help.getSeq<eqgame_t*>();
		eqgame_t::CharSpawn = &CharSpawn;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->channel.handle = help.getSeq<HWND>();
		test_CEverQuest_InterpretCmd_t::target = [&](eqgame_t *player, char *cmd) {
			help << "CEverQuest_InterpretCmd\n";
			help << int(player) << '\n';
			help << cmd << '\n';
		};
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			std::strcpy(lpString, help.getSeqStr().c_str());
			return std::strlen(lpString);
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return LRESULT(5);
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << '\'' << lpString << "'\n";
			return TRUE;
		};
		bar->execute(0);
		Assert::AreEqual(1, int(bar->data->history.lines.size()));
		Assert::AreEqual(std::string("6"), bar->data->history.lines.back());
		Assert::IsTrue(bar->data->registry.lines.empty());
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_InterpretCmd"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("/Say 6"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // よく使う発言リストに登録できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			std::strcpy(lpString, help.getSeqStr().c_str());
			return std::strlen(lpString);
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << '\'' << lpString << "'\n";
			return TRUE;
		};
		bar->execute(-1);
		Assert::IsTrue(bar->data->history.lines.empty());
		Assert::AreEqual(1, int(bar->data->registry.lines.size()));
		Assert::AreEqual(std::string("2"), bar->data->registry.lines.back());
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_fixPos) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // アイコンモードのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		context.screen_mode = screen_modes::ICONIC;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->fixPos();
	}
	{ // 位置ズレを修正できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.screen_mode = screen_modes::WINDOW;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->mw_client_pos.x = help.getSeq<LONG>();
		bar->data->mw_client_pos.y = help.getSeq<LONG>();
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			lpPoint->x = help.getSeq<LONG>();
			lpPoint->y = help.getSeq<LONG>();
			return TRUE;
		};
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			help << nIndex << '\n';
			return 100;
		};
		api::SetWindowPos_true = [&](
			HWND hWnd, 
			HWND hWndInsertAfter, 
			int X, 
			int Y, 
			int cx, 
			int cy, 
			UINT uFlags
		) -> BOOL {
			help << "SetWindowPos\n";
			help << int(hWnd) << '\n';
			help << int(hWndInsertAfter) << '\n';
			help << X << '\n';
			help << Y << '\n';
			help << cx << '\n';
			help << cy << '\n';
			help << uFlags << '\n';
			return TRUE;
		};
		bar->fixPos();
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CXSCREEN), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CYSCREEN), help.getLine());
		Assert::AreEqual(std::string("SetWindowPos"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(SWP_NOACTIVATE | SWP_NOSIZE), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // センタリングできるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.screen_mode = screen_modes::FULL_SCREEN;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->mw_client_pos.x = help.getSeq<LONG>();
		bar->data->mw_client_pos.y = help.getSeq<LONG>();
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			lpPoint->x = -100;
			lpPoint->y = -100;
			return TRUE;
		};
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			help << nIndex << '\n';
			return 200;
		};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{-100, -100, 0, -76};
			return TRUE;
		};
		api::SetWindowPos_true = [&](
			HWND hWnd, 
			HWND hWndInsertAfter, 
			int X, 
			int Y, 
			int cx, 
			int cy, 
			UINT uFlags
		) -> BOOL {
			help << "SetWindowPos\n";
			help << int(hWnd) << '\n';
			help << int(hWndInsertAfter) << '\n';
			help << X << '\n';
			help << Y << '\n';
			help << cx << '\n';
			help << cy << '\n';
			help << uFlags << '\n';
			return TRUE;
		};
		bar->fixPos();
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CXSCREEN), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CYSCREEN), help.getLine());
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("SetWindowPos"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("50"), help.getLine());
		Assert::AreEqual(std::string("88"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(SWP_NOACTIVATE | SWP_NOSIZE), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_getHandle) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		Assert::AreEqual(1, int(bar->getHandle()));
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_getPen) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		api::CreatePen_true = [&](
			int iStyle, 
			int cWidth, 
			COLORREF color
		) -> HPEN {
			help << "CreatePen\n";
			help << iStyle << '\n';
			help << cWidth << '\n';
			help << color << '\n';
			return help.getSeq<HPEN>();
		};
		api::DeleteObject = [&](HGDIOBJ ho) -> BOOL {
			help << "DeleteObject\n";
			help << int(ho) << '\n';
			return TRUE;
		};
		Assert::AreEqual(1, int(bar->getPen(RGB(0, 0, 1))));
		Assert::AreEqual(1, int(bar->getPen(RGB(0, 0, 1))));
		Assert::AreEqual(2, int(bar->getPen(RGB(0, 0, 2))));
		Assert::AreEqual(2, int(bar->data->pen_cache.size()));
		Assert::AreEqual(1, int(bar->data->pen_cache.at(RGB(0, 0, 1))));
		Assert::AreEqual(2, int(bar->data->pen_cache.at(RGB(0, 0, 2))));
		Assert::AreEqual(2, int(bar->data->exits.size()));
		reverseClear(bar->data->exits);
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(RGB(0, 0, 1)), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(RGB(0, 0, 2)), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
		
TEST_METHOD(test_jchat_bar_loadRegistry) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << '\'' << lpDefault << "'\n";
			help << lpFileName << '\n';
			static const char *RESS[] = {
				"Hoge",
				"Fuga",
				"",
			};
			static const char **res = RESS;
			std::strcpy(lpReturnedString, *res);
			return std::strlen(*res++);
		};
		bar->loadRegistry();
		Assert::AreEqual(2, int(bar->data->registry.lines.size()));
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->data->registry.moveBegin();
		Assert::AreEqual(std::string("Hoge"), *bar->data->registry.iter++);
		Assert::AreEqual(std::string("Fuga"), *bar->data->registry.iter++);
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("Line_0"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("Line_1"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("Line_2"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_moveFocus) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->ordered_handles = {help.getSeq<HWND>(), help.getSeq<HWND>(), help.getSeq<HWND>()};
		for (int i = 0; i < int(bar->data->ordered_handles.size()); ++i)
			bar->data->ordered_indices.emplace(bar->data->ordered_handles[i], i);
		api::SetFocus_true = [&](HWND hWnd) -> HWND {
			help << "SetFocus\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		bar->moveFocus(HWND(1), 0);
		bar->moveFocus(HWND(2), 0);
		bar->moveFocus(HWND(3), 0);
		bar->moveFocus(HWND(1), -1);
		bar->moveFocus(HWND(3), -1);
		bar->moveFocus(HWND(2), -1);
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_moveThroughList) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // リストが空っぽのときに移動できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		bar->moveThroughList(0, VK_UP);
		bar->moveThroughList(0, VK_DOWN);
		bar->moveThroughList(-1, VK_UP);
		bar->moveThroughList(-1, VK_DOWN);
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // リストが空っぽではないときに移動できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->history.lines = {"Hoge", "Fuga"};
		bar->data->history.moveEnd();
		bar->data->registry.lines = {"Foo", "Bar"};
		bar->data->registry.moveEnd();
		bar->data->edit.last_line = "Last";
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return 0;
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		bar->moveThroughList(0, VK_UP);
		Assert::AreEqual(std::string("Fuga"), *bar->data->history.iter);
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->moveThroughList(0, VK_UP);
		Assert::IsTrue(bar->data->history.isBegin());
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->moveThroughList(0, VK_UP);
		Assert::IsTrue(bar->data->history.isBegin());
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->moveThroughList(0, VK_DOWN);
		Assert::AreEqual(std::string("Fuga"), *bar->data->history.iter);
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->moveThroughList(0, VK_DOWN);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->moveThroughList(0, VK_DOWN);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->moveThroughList(-1, VK_UP);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::AreEqual(std::string("Bar"), *bar->data->registry.iter);
		bar->moveThroughList(-1, VK_UP);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::IsTrue(bar->data->registry.isBegin());
		bar->moveThroughList(-1, VK_UP);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::IsTrue(bar->data->registry.isBegin());
		bar->moveThroughList(-1, VK_DOWN);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::AreEqual(std::string("Bar"), *bar->data->registry.iter);
		bar->moveThroughList(-1, VK_DOWN);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::IsTrue(bar->data->registry.isEnd());
		bar->moveThroughList(-1, VK_DOWN);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::IsTrue(bar->data->registry.isEnd());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Fuga"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Hoge"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Fuga"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Last"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Bar"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Foo"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Bar"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("Last"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_SETSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("-1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onCommand) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 編集モードのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.change_mode = change_modes::EDIT;
		bar->data->edit.dirty = false;
		bar->data->history.lines = {help.getSeqStr()};
		bar->data->history.moveBegin();
		bar->data->registry.lines = {help.getSeqStr()};
		bar->data->registry.moveBegin();
		Assert::AreEqual(0, int(bar->onCommand(bar->data->handle, WM_COMMAND, MAKEWPARAM(EDIT_ID, EN_CHANGE), 0)));
		Assert::IsTrue(bar->data->edit.dirty);
		Assert::IsTrue(bar->data->history.isEnd());
		Assert::IsTrue(bar->data->registry.isEnd());
	}
	{ // 選択モードのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.change_mode = change_modes::CHOICE;
		bar->data->edit.dirty = false;
		bar->data->history.lines = {help.getSeqStr()};
		bar->data->history.moveBegin();
		bar->data->registry.lines = {help.getSeqStr()};
		bar->data->registry.moveBegin();
		Assert::AreEqual(0, int(bar->onCommand(bar->data->handle, WM_COMMAND, MAKEWPARAM(EDIT_ID, EN_CHANGE), 0)));
		Assert::IsFalse(bar->data->edit.dirty);
		Assert::IsFalse(bar->data->history.isEnd());
		Assert::IsFalse(bar->data->registry.isEnd());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onCreate) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.dll_handle = help.getSeq<HINSTANCE>();
		context.ini_path = help.getSeqStr();
		context.ini.chat.font_name = help.getSeqStr();
		context.ini.window.width = (EDIT_HOR_MARGIN + 2 + 2) + help.getSeq();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::CreateCompatibleBitmap_true = [&](
			HDC hdc, 
			int cx, 
			int cy
		) -> HBITMAP {
			help << "CreateCompatibleBitmap\n";
			help << int(hdc) << '\n';
			help << cx << '\n';
			help << cy << '\n';
			return help.getSeq<HBITMAP>();
		};
		api::CreateCompatibleDC_true = [&](HDC hdc) -> HDC {
			help << "CreateCompatibleDC\n";
			help << int(hdc) << '\n';
			return help.getSeq<HDC>();
		};
		api::CreateFont_true = [&](
			int cHeight, 
			int cWidth, 
			int cEscapement, 
			int cOrientation, 
			int cWeight, 
			DWORD bItalic, 
			DWORD bUnderline, 
			DWORD bStrikeOut, 
			DWORD iCharSet, 
			DWORD iOutPrecision, 
			DWORD iClipPrecision, 
			DWORD iQuality, 
			DWORD iPitchAndFamily, 
			LPCSTR pszFaceName
		) -> HFONT {
			help << "CreateFont\n";
			help << cHeight << '\n';
			help << cWidth << '\n';
			help << cEscapement << '\n';
			help << cOrientation << '\n';
			help << cWeight << '\n';
			help << bItalic << '\n';
			help << bUnderline << '\n';
			help << bStrikeOut << '\n';
			help << iCharSet << '\n';
			help << iOutPrecision << '\n';
			help << iClipPrecision << '\n';
			help << iQuality << '\n';
			help << iPitchAndFamily << '\n';
			help << pszFaceName << '\n';
			return help.getSeq<HFONT>();
		};
		api::CreatePatternBrush_true = [&](HBITMAP hbm) -> HBRUSH {
			help << "CreatePatternBrush\n";
			help << int(hbm) << '\n';
			return help.getSeq<HBRUSH>();
		};
		api::CreateWindowEx_true = [&](
			DWORD dwExStyle, 
			PCTSTR pszClassName, 
			PCTSTR pszWindowName, 
			DWORD dwStyle, 
			int x, 
			int y, 
			int nWidth, 
			int nHeight, 
			HWND hWndParent, 
			HMENU hMenu, 
			HINSTANCE hInstance, 
			PVOID pParam
		) -> HWND {
			help << "CreateWindowEx\n";
			help << dwExStyle << '\n';
			help << pszClassName << '\n';
			if (pszWindowName) help << pszWindowName << '\n';
			else help << "(null)\n";
			help << dwStyle << '\n';
			help << x << '\n';
			help << y << '\n';
			help << nWidth << '\n';
			help << nHeight << '\n';
			help << int(hWndParent) << '\n';
			help << int(hMenu) << '\n';
			help << int(hInstance) << '\n';
			help << int(pParam) << '\n';
			return help.getSeq<HWND>();
		};
		api::DeleteDC = [&](HDC hdc) -> BOOL {
			help << "DeleteDC\n";
			help << int(hdc) << '\n';
			return TRUE;
		};
		api::DeleteObject = [&](HGDIOBJ ho) -> BOOL {
			help << "DeleteObject\n";
			help << int(ho) << '\n';
			return TRUE;
		};
		api::GetDC_true = [&](HWND hWnd) -> HDC {
			help << "GetDC\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HDC>();
		};
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << '\'' << lpDefault << "'\n";
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "");
			return 0;
		};
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			help << nIndex << '\n';
			return help.getSeq();
		};
		api::LoadBitmap_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpBitmapName
		) -> HBITMAP {
			help << "LoadBitmap\n";
			help << int(hInstance) << '\n';
			help << int(lpBitmapName) << '\n';
			return help.getSeq<HBITMAP>();
		};
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			help << int(hInstance) << '\n';
			help << int(lpCursorName) << '\n';
			return help.getSeq<HCURSOR>();
		};
		api::ReleaseDC = [&](HWND hWnd, HDC hDC) -> int {
			help << "ReleaseDC\n";
			help << int(hWnd) << '\n';
			help << int(hDC) << '\n';
			return 1;
		};
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			help << int(hdc) << '\n';
			help << int(h) << '\n';
			return help.getSeq<HGDIOBJ>();
		};
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			if (Msg == CB_ADDSTRING) help << LPCTSTR(lParam) << '\n';
			else help << lParam << '\n';
			return 0;
		};
		api::SetClassLong_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG dwNewLong
		) -> DWORD {
			help << "SetClassLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << dwNewLong << '\n';
			return help.getSeq<DWORD>();
		};
		api::SetWindowLongPtr_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG_PTR dwNewLong
		) -> LONG_PTR {
			help << "SetWindowLongPtr\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << dwNewLong << '\n';
			return help.getSeq<LONG_PTR>();
		};
		Assert::AreEqual(0, int(bar->onCreate(bar->data->handle, WM_CREATE, WPARAM(0), LPARAM(NULL))));
		Assert::AreEqual(7, int(bar->data->rock_bar_brush_handle));
		Assert::AreEqual(9, int(bar->data->rock_wall_brush_handle));
		Assert::AreEqual(12, int(bar->data->icon_active_dc_handle));
		Assert::AreEqual(15, int(bar->data->icon_inactive_dc_handle));
		Assert::AreEqual(18, int(bar->data->knob_dc_handle));
		Assert::AreEqual(22, int(bar->data->buffer_dc_handle));
		Assert::AreEqual(24, int(bar->data->hfont));
		Assert::AreEqual(25, int(bar->data->channel.handle));
		Assert::AreEqual(26, int(bar->data->channel.WindowProc_super));
		Assert::AreEqual(29, int(bar->data->edit.handle));
		Assert::AreEqual(30, int(bar->data->edit.WindowProc_super));
		Assert::AreEqual(2, int(bar->data->ordered_handles.size()));
		Assert::AreEqual(25, int(bar->data->ordered_handles[0]));
		Assert::AreEqual(29, int(bar->data->ordered_handles[1]));
		Assert::AreEqual(2, int(bar->data->ordered_indices.size()));
		Assert::AreEqual(0, int(bar->data->ordered_indices[HWND(25)]));
		Assert::AreEqual(1, int(bar->data->ordered_indices[HWND(29)]));
		Assert::AreEqual(13, int(bar->data->exits.size()));
		reverseClear(bar->data->exits);
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ROCK_BAR))), help.getLine());
		Assert::AreEqual(std::string("CreatePatternBrush"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ROCK_WALL))), help.getLine());
		Assert::AreEqual(std::string("CreatePatternBrush"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("GetDC"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ICON_ACTIVE))), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("11"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ICON_INACTIVE))), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("15"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_KNOB))), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("18"), help.getLine());
		Assert::AreEqual(std::string("17"), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CXSCREEN), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleBitmap"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("20"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("22"), help.getLine());
		Assert::AreEqual(std::string("21"), help.getLine());
		Assert::AreEqual(std::string("CreateFont"), help.getLine());
		Assert::AreEqual(stringize(FONT_SIZE), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(FW_NORMAL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(SHIFTJIS_CHARSET), help.getLine());
		Assert::AreEqual(stringize(OUT_DEFAULT_PRECIS), help.getLine());
		Assert::AreEqual(stringize(CLIP_DEFAULT_PRECIS), help.getLine());
		Assert::AreEqual(stringize(DEFAULT_QUALITY), help.getLine());
		Assert::AreEqual(stringize(DEFAULT_PITCH), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("ReleaseDC"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("COMBOBOX"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(stringize(CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE), help.getLine());
		Assert::AreEqual(stringize(ICON_WIDTH), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_TOP), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_WIDTH), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_ID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string("SetWindowLongPtr"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(GWLP_WNDPROC), help.getLine());
		Assert::AreEqual(stringize(LONG_PTR(&channel_WindowProc_sub)), help.getLine());
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_ARROW))), help.getLine());
		Assert::AreEqual(std::string("SetClassLong"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(GCL_HCURSOR), help.getLine());
		Assert::AreEqual(std::string("27"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(WM_SETFONT), help.getLine());
		Assert::AreEqual(std::string("24"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Auction"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Gsay"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/gUildsay"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Ooc"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Reply"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/Say"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_ADDSTRING), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("/sHout"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("25"), help.getLine());
		Assert::AreEqual(stringize(CB_SETCURSEL), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("EDIT"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(stringize(ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_VISIBLE), help.getLine());
		Assert::AreEqual(stringize(EDIT_LEFT + 2), help.getLine());
		Assert::AreEqual(stringize(EDIT_TOP + 4), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(EDIT_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(EDIT_ID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string("SetWindowLongPtr"), help.getLine());
		Assert::AreEqual(std::string("29"), help.getLine());
		Assert::AreEqual(stringize(GWLP_WNDPROC), help.getLine());
		Assert::AreEqual(stringize(LONG_PTR(&edit_WindowProc_sub)), help.getLine());
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_IBEAM))), help.getLine());
		Assert::AreEqual(std::string("SetClassLong"), help.getLine());
		Assert::AreEqual(std::string("29"), help.getLine());
		Assert::AreEqual(stringize(GCL_HCURSOR), help.getLine());
		Assert::AreEqual(stringize("31"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("29"), help.getLine());
		Assert::AreEqual(stringize(WM_SETFONT), help.getLine());
		Assert::AreEqual(std::string("24"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("Line_0"), help.getLine());
		Assert::AreEqual(std::string("''"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("24"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("22"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("21"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("18"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("17"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("15"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("11"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("9"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onCtlColorEdit) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // チャンネル欄のときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->channel.handle = help.getSeq<HWND>();
		bar->data->rock_bar_brush_handle = help.getSeq<HBRUSH>();
		api::SetBkColor_true = [&](
			HDC hdc, 
			COLORREF color
		) -> COLORREF {
			help << "SetBkColor\n";
			help << int(hdc) << '\n';
			help << color << '\n';
			return 0;
		};
		api::SetBkMode_true = [&](HDC hdc, int mode) -> int {
			help << "SetBkMode\n";
			help << int(hdc) << '\n';
			help << mode << '\n';
			return 1;
		};
		api::SetTextColor_true = [&](
			HDC hdc, 
			COLORREF color
		) -> COLORREF {
			help << "SetTextColor\n";
			help << int(hdc) << '\n';
			help << color << '\n';
			return 0;
		};
		Assert::AreEqual(3, int(bar->onCtlColorEdit(bar->data->handle, WM_CTLCOLOREDIT, WPARAM(help.getSeq<HDC>()), LPARAM(bar->data->channel.handle))));
		Assert::AreEqual(std::string("SetTextColor"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(TEXT_COLOR), help.getLine());
		Assert::AreEqual(std::string("SetBkMode"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(OPAQUE), help.getLine());
		Assert::AreEqual(std::string("SetBkColor"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(CHANNEL_BK_COLOR), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 入力欄のときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.handle = help.getSeq<HWND>();
		bar->data->rock_bar_brush_handle = help.getSeq<HBRUSH>();
		api::SetBrushOrgEx_true = [&](
			HDC hdc, 
			int x, 
			int y, 
			LPPOINT lppt
		) -> BOOL {
			help << "SetBrushOrgEx\n";
			help << int(hdc) << '\n';
			help << x << '\n';
			help << y << '\n';
			help << int(lppt) << '\n';
			return TRUE;
		};
		api::SetBkMode_true = [&](HDC hdc, int mode) -> int {
			help << "SetBkMode\n";
			help << int(hdc) << '\n';
			help << mode << '\n';
			return 1;
		};
		api::SetTextColor_true = [&](
			HDC hdc, 
			COLORREF color
		) -> COLORREF {
			help << "SetTextColor\n";
			help << int(hdc) << '\n';
			help << color << '\n';
			return 0;
		};
		Assert::AreEqual(3, int(bar->onCtlColorEdit(bar->data->handle, WM_CTLCOLOREDIT, WPARAM(help.getSeq<HDC>()), LPARAM(bar->data->edit.handle))));
		Assert::AreEqual(std::string("SetTextColor"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(TEXT_COLOR), help.getLine());
		Assert::AreEqual(std::string("SetBkMode"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(TRANSPARENT), help.getLine());
		Assert::AreEqual(std::string("SetBrushOrgEx"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(-(EDIT_LEFT + 2)), help.getLine());
		Assert::AreEqual(stringize(-(EDIT_TOP + 4)), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onDestroy) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 失敗できるか？
		test_helper_t help;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::AreEqual(0, int(bar->onDestroy(bar->data->handle, WM_DESTROY, 0, 0)));
		Assert::AreEqual(0, int(bar->data->handle));
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetWindowRect", 3), help.getLine().substr(24));
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->mw_client_pos.x = help.getSeq<LONG>();
		bar->data->mw_client_pos.y = help.getSeq<LONG>();
		bar->data->registry.lines = {help.getSeqStr()};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{help.getSeq<LONG>(), help.getSeq<LONG>(), help.getSeq<LONG>(), help.getSeq<LONG>()};
			return TRUE;
		};
		api::WritePrivateProfileString_true = [&](
			LPCSTR lpAppName, 
			LPCSTR lpKeyName, 
			LPCSTR lpString, 
			LPCSTR lpFileName
		) -> BOOL {
			help << "WritePrivateProfileString\n";
			help << lpAppName << '\n';
			if (lpKeyName) help << lpKeyName << '\n';
			else help << "(null)\n";
			if (lpString) help << lpString << '\n';
			else help << "(null)\n";
			help << lpFileName << '\n';
			return TRUE;
		};
		Assert::AreEqual(0, int(bar->onDestroy(bar->data->handle, WM_DESTROY, 0, 0)));
		Assert::AreEqual(0, int(bar->data->handle));
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Left"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Top"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Width"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("Line_0"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onGetMinMaxInfo) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini.window.min_width = help.getSeq<std::size_t>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		MINMAXINFO info = {};
		Assert::AreEqual(0, int(bar->onGetMinMaxInfo(bar->data->handle, WM_GETMINMAXINFO, 0, LPARAM(&info))));
		Assert::AreEqual(1, int(info.ptMinTrackSize.x));
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onMove) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::ScreenToClient_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ScreenToClient\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			lpPoint->x = help.getSeq<LONG>();
			lpPoint->y = help.getSeq<LONG>();
			return TRUE;
		};
		help.setSeqBase();
		Assert::AreEqual(0, int(bar->onMove(bar->data->handle, WM_MOVE, 0, MAKELPARAM(help.getSeq<LONG>(1), help.getSeq<LONG>(2)))));
		Assert::AreEqual(5, int(bar->data->mw_client_pos.x));
		Assert::AreEqual(6, int(bar->data->mw_client_pos.y));
		Assert::AreEqual(std::string("ScreenToClient"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onNCHitTest) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{100, 200, 400, 224};
			return TRUE;
		};
		Assert::AreEqual(HTCAPTION, int(bar->onNCHitTest(bar->data->handle, WM_NCHITTEST, 0, MAKELPARAM(100, 200))));
		Assert::AreEqual(HTRIGHT, int(bar->onNCHitTest(bar->data->handle, WM_NCHITTEST, 0, MAKELPARAM(492, 200))));
		Assert::AreEqual(HTCLIENT, int(bar->onNCHitTest(bar->data->handle, WM_NCHITTEST, 0, MAKELPARAM(300, 200))));
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onPaint) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // アクティブのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->rock_bar_brush_handle = help.getSeq<HBRUSH>();
		bar->data->buffer_dc_handle = help.getSeq<HDC>();
		bar->data->icon_active_dc_handle = help.getSeq<HDC>();
		bar->data->icon_inactive_dc_handle = help.getSeq<HDC>();
		bar->data->knob_dc_handle = help.getSeq<HDC>();
		api::BeginPaint_true = [&](
			HWND hWnd, 
			LPPAINTSTRUCT lpPaint
		) -> HDC {
			help << "BeginPaint\n";
			help << int(hWnd) << '\n';
			lpPaint->rcPaint = RECT{0, 0, 400, 24};
			return help.getSeq<HDC>();
		};
		api::BitBlt_true = [&](
			HDC hdc, 
			int x, 
			int y, 
			int cx, 
			int cy, 
			HDC hdcSrc, 
			int x1, 
			int y1, 
			DWORD rop
		) -> BOOL {
			help << "BitBlt\n";
			help << int(hdc) << '\n';
			help << x << '\n';
			help << y << '\n';
			help << cx << '\n';
			help << cy << '\n';
			help << int(hdcSrc) << '\n';
			help << x1 << '\n';
			help << y1 << '\n';
			help << rop << '\n';
			return TRUE;
		};
		api::CreatePen_true = [&](
			int iStyle, 
			int cWidth, 
			COLORREF color
		) -> HPEN {
			help << "CreatePen\n";
			help << iStyle << '\n';
			help << cWidth << '\n';
			help << color << '\n';
			return help.getSeq<HPEN>();
		};
		api::DeleteObject = [&](HGDIOBJ ho) -> BOOL {
			help << "DeleteObject\n";
			help << int(ho) << '\n';
			return TRUE;
		};
		api::FillRect_true = [&]( 
			HDC hDC, 
			const RECT *lprc, 
			HBRUSH hbr
		) -> int {
			help << "FillRect\n";
			help << int(hDC) << '\n';
			help << int(hbr) << '\n';
			return 1;
		};
		api::GetActiveWindow = [&]() -> HWND {
			help << "GetActiveWindow\n";
			return bar->data->handle;
		};
		api::GetClientRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetClientRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{0, 0, 400, 24};
			return TRUE;
		};
		api::Polyline_true = [&](
			HDC hdc, 
			const POINT *apt, 
			int cpt
		) -> BOOL {
			help << "Polyline\n";
			help << int(hdc) << '\n';
			for (int i = 0; i < cpt; ++i) 
				help << apt[i].x  << ',' << apt[i].y << '\n';
			return TRUE;
		};
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			help << int(hdc) << '\n';
			help << int(h) << '\n';
			return help.getSeq<HGDIOBJ>();
		};
		Assert::AreEqual(0, int(bar->onPaint(bar->data->handle, WM_PAINT, 0, 0)));
		Assert::AreEqual(5, int(bar->data->exits.size()));
		reverseClear(bar->data->exits);
		Assert::AreEqual(std::string("BeginPaint"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetClientRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("FillRect"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("GetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(SRCCOPY), help.getLine());
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(400 - KNOB_WIDTH), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(KNOB_WIDTH), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(SRCCOPY), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_RAISED_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("0,23"), help.getLine());
		Assert::AreEqual(std::string("0,0"), help.getLine());
		Assert::AreEqual(std::string("400,0"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_SUNKEN_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("399,1"), help.getLine());
		Assert::AreEqual(std::string("399,23"), help.getLine());
		Assert::AreEqual(std::string("0,23"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EDIT_OUTER_RAISED_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("98,21"), help.getLine());
		Assert::AreEqual(std::string("98,2"), help.getLine());
		Assert::AreEqual(std::string("392,2"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EDIT_OUTER_SUNKEN_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("391,3"), help.getLine());
		Assert::AreEqual(std::string("391,21"), help.getLine());
		Assert::AreEqual(std::string("98,21"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EDIT_INNER_RAISED_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("99,20"), help.getLine());
		Assert::AreEqual(std::string("99,3"), help.getLine());
		Assert::AreEqual(std::string("391,3"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("390,4"), help.getLine());
		Assert::AreEqual(std::string("390,20"), help.getLine());
		Assert::AreEqual(std::string("99,20"), help.getLine());
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(400), help.getLine());
		Assert::AreEqual(stringize(24), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(SRCCOPY), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // インアクティブのときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->rock_bar_brush_handle = help.getSeq<HBRUSH>();
		bar->data->buffer_dc_handle = help.getSeq<HDC>();
		bar->data->icon_active_dc_handle = help.getSeq<HDC>();
		bar->data->icon_inactive_dc_handle = help.getSeq<HDC>();
		bar->data->knob_dc_handle = help.getSeq<HDC>();
		api::BeginPaint_true = [&](
			HWND hWnd, 
			LPPAINTSTRUCT lpPaint
		) -> HDC {
			help << "BeginPaint\n";
			help << int(hWnd) << '\n';
			lpPaint->rcPaint = RECT{0, 0, 400, 24};
			return help.getSeq<HDC>();
		};
		api::BitBlt_true = [&](
			HDC hdc, 
			int x, 
			int y, 
			int cx, 
			int cy, 
			HDC hdcSrc, 
			int x1, 
			int y1, 
			DWORD rop
		) -> BOOL {
			help << "BitBlt\n";
			help << int(hdc) << '\n';
			help << x << '\n';
			help << y << '\n';
			help << cx << '\n';
			help << cy << '\n';
			help << int(hdcSrc) << '\n';
			help << x1 << '\n';
			help << y1 << '\n';
			help << rop << '\n';
			return TRUE;
		};
		api::CreatePen_true = [&](
			int iStyle, 
			int cWidth, 
			COLORREF color
		) -> HPEN {
			help << "CreatePen\n";
			help << iStyle << '\n';
			help << cWidth << '\n';
			help << color << '\n';
			return help.getSeq<HPEN>();
		};
		api::DeleteObject = [&](HGDIOBJ ho) -> BOOL {
			help << "DeleteObject\n";
			help << int(ho) << '\n';
			return TRUE;
		};
		api::FillRect_true = [&]( 
			HDC hDC, 
			const RECT *lprc, 
			HBRUSH hbr
		) -> int {
			help << "FillRect\n";
			help << int(hDC) << '\n';
			help << int(hbr) << '\n';
			return 1;
		};
		api::GetActiveWindow = [&]() -> HWND {
			help << "GetActiveWindow\n";
			return HWND(int(bar->data->handle) + 1);
		};
		api::GetClientRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetClientRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{0, 0, 400, 24};
			return TRUE;
		};
		api::Polyline_true = [&](
			HDC hdc, 
			const POINT *apt, 
			int cpt
		) -> BOOL {
			help << "Polyline\n";
			help << int(hdc) << '\n';
			for (int i = 0; i < cpt; ++i) 
				help << apt[i].x  << ',' << apt[i].y << '\n';
			return TRUE;
		};
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			help << int(hdc) << '\n';
			help << int(h) << '\n';
			return help.getSeq<HGDIOBJ>();
		};
		Assert::AreEqual(0, int(bar->onPaint(bar->data->handle, WM_PAINT, 0, 0)));
		Assert::AreEqual(5, int(bar->data->exits.size()));
		reverseClear(bar->data->exits);
		Assert::AreEqual(std::string("BeginPaint"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetClientRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("FillRect"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("GetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(SRCCOPY), help.getLine());
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(400 - KNOB_WIDTH), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(KNOB_WIDTH), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(SRCCOPY), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_RAISED_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("0,23"), help.getLine());
		Assert::AreEqual(std::string("0,0"), help.getLine());
		Assert::AreEqual(std::string("400,0"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_SUNKEN_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("399,1"), help.getLine());
		Assert::AreEqual(std::string("399,23"), help.getLine());
		Assert::AreEqual(std::string("0,23"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EDIT_OUTER_RAISED_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("98,21"), help.getLine());
		Assert::AreEqual(std::string("98,2"), help.getLine());
		Assert::AreEqual(std::string("392,2"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EDIT_OUTER_SUNKEN_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("391,3"), help.getLine());
		Assert::AreEqual(std::string("391,21"), help.getLine());
		Assert::AreEqual(std::string("98,21"), help.getLine());
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(stringize(PS_SOLID), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EDIT_INNER_RAISED_COLOR), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("99,20"), help.getLine());
		Assert::AreEqual(std::string("99,3"), help.getLine());
		Assert::AreEqual(std::string("391,3"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("390,4"), help.getLine());
		Assert::AreEqual(std::string("390,20"), help.getLine());
		Assert::AreEqual(std::string("99,20"), help.getLine());
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(400), help.getLine());
		Assert::AreEqual(stringize(24), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(0), help.getLine());
		Assert::AreEqual(stringize(SRCCOPY), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onSetCursor) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = HWND(101);
		api::DefWindowProc = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam,
			LPARAM lParam
		) -> LRESULT {
			help << "DefWindowProc\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			return TRUE;
		};
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			help << int(hInstance) << '\n';
			help << int(lpCursorName) << '\n';
			return help.getSeq<HCURSOR>();
		};
		api::SetCursor = [&](HCURSOR hCursor) -> HCURSOR {
			help << "SetCursor\n";
			help << int(hCursor) << '\n';
			return help.getSeq<HCURSOR>();
		};
		Assert::AreEqual(TRUE, int(bar->onSetCursor(bar->data->handle, WM_SETCURSOR, WPARAM(HWND(101)), MAKELPARAM(HTCAPTION, WM_MOUSEMOVE))));
		Assert::AreEqual(TRUE, int(bar->onSetCursor(bar->data->handle, WM_SETCURSOR, WPARAM(HWND(101)), MAKELPARAM(HTRIGHT, WM_MOUSEMOVE))));
		Assert::AreEqual(TRUE, int(bar->onSetCursor(bar->data->handle, WM_SETCURSOR, WPARAM(HWND(101)), MAKELPARAM(HTCLIENT, WM_MOUSEMOVE))));
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_MOVE))), help.getLine());
		Assert::AreEqual(std::string("SetCursor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_EW))), help.getLine());
		Assert::AreEqual(std::string("SetCursor"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DefWindowProc"), help.getLine());
		Assert::AreEqual(std::string("101"), help.getLine());
		Assert::AreEqual(stringize(WM_SETCURSOR), help.getLine());
		Assert::AreEqual(std::string("101"), help.getLine());
		Assert::AreEqual(stringize(MAKELPARAM(HTCLIENT, WM_MOUSEMOVE)), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onSetFocus) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SetFocus_true = [&](HWND hWnd) -> HWND {
			help << "SetFocus\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		Assert::AreEqual(0, int(bar->onSetFocus(bar->data->handle, WM_SETFOCUS, NULL, 0)));
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_onSize) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 最小化のときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		Assert::AreEqual(0, int(bar->onSize(bar->data->handle, WM_SIZE, SIZE_MINIMIZED, MAKELPARAM(300, 24))));
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // サイズ変更のときに正しく動作できるか？
		test_helper_t help;
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::MoveWindow_true = [&](
			HWND hWnd, 
			int X, 
			int Y, 
			int nWidth, 
			int nHeight, 
			BOOL bRepaint
		) -> BOOL {
			help << "MoveWindow\n";
			help << int(hWnd) << '\n';
			help << X << '\n';
			help << Y << '\n';
			help << nWidth << '\n';
			help << nHeight << '\n';
			help << bRepaint << '\n';
			return TRUE;
		};
		Assert::AreEqual(0, int(bar->onSize(bar->data->handle, WM_SIZE, SIZE_RESTORED, MAKELPARAM(300, 24))));
		bar->data->edit.handle = help.getSeq<HWND>();
		Assert::AreEqual(0, int(bar->onSize(bar->data->handle, WM_SIZE, SIZE_RESTORED, MAKELPARAM(300, 24))));
		Assert::AreEqual(std::string("MoveWindow"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(EDIT_LEFT + 2), help.getLine());
		Assert::AreEqual(stringize(EDIT_TOP + 4), help.getLine());
		Assert::AreEqual(stringize(300 - ((EDIT_HOR_MARGIN + 2) + 2)), help.getLine());
		Assert::AreEqual(stringize(EDIT_HEIGHT), help.getLine());
		Assert::AreEqual(stringize(TRUE), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_prepare) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.dll_handle = help.getSeq<HINSTANCE>();
		context.ini.chat.font_name = help.getSeqStr();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		api::CreateCompatibleBitmap_true = [&](
			HDC hdc, 
			int cx, 
			int cy
		) -> HBITMAP {
			help << "CreateCompatibleBitmap\n";
			help << int(hdc) << '\n';
			help << cx << '\n';
			help << cy << '\n';
			return help.getSeq<HBITMAP>();
		};
		api::CreateCompatibleDC_true = [&](HDC hdc) -> HDC {
			help << "CreateCompatibleDC\n";
			help << int(hdc) << '\n';
			return help.getSeq<HDC>();
		};
		api::CreateFont_true = [&](
			int cHeight, 
			int cWidth, 
			int cEscapement, 
			int cOrientation, 
			int cWeight, 
			DWORD bItalic, 
			DWORD bUnderline, 
			DWORD bStrikeOut, 
			DWORD iCharSet, 
			DWORD iOutPrecision, 
			DWORD iClipPrecision, 
			DWORD iQuality, 
			DWORD iPitchAndFamily, 
			LPCSTR pszFaceName
		) -> HFONT {
			help << "CreateFont\n";
			help << cHeight << '\n';
			help << cWidth << '\n';
			help << cEscapement << '\n';
			help << cOrientation << '\n';
			help << cWeight << '\n';
			help << bItalic << '\n';
			help << bUnderline << '\n';
			help << bStrikeOut << '\n';
			help << iCharSet << '\n';
			help << iOutPrecision << '\n';
			help << iClipPrecision << '\n';
			help << iQuality << '\n';
			help << iPitchAndFamily << '\n';
			help << pszFaceName << '\n';
			return help.getSeq<HFONT>();
		};
		api::CreatePatternBrush_true = [&](HBITMAP hbm) -> HBRUSH {
			help << "CreatePatternBrush\n";
			help << int(hbm) << '\n';
			return help.getSeq<HBRUSH>();
		};
		api::DeleteDC = [&](HDC hdc) -> BOOL {
			help << "DeleteDC\n";
			help << int(hdc) << '\n';
			return TRUE;
		};
		api::DeleteObject = [&](HGDIOBJ ho) -> BOOL {
			help << "DeleteObject\n";
			help << int(ho) << '\n';
			return TRUE;
		};
		api::GetDC_true = [&](HWND hWnd) -> HDC {
			help << "GetDC\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HDC>();
		};
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			help << nIndex << '\n';
			return help.getSeq();
		};
		api::LoadBitmap_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpBitmapName
		) -> HBITMAP {
			help << "LoadBitmap\n";
			help << int(hInstance) << '\n';
			help << int(lpBitmapName) << '\n';
			return help.getSeq<HBITMAP>();
		};
		api::ReleaseDC = [&](HWND hWnd, HDC hDC) -> int {
			help << "ReleaseDC\n";
			help << int(hWnd) << '\n';
			help << int(hDC) << '\n';
			return 1;
		};
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			help << int(hdc) << '\n';
			help << int(h) << '\n';
			return help.getSeq<HGDIOBJ>();
		};
		bar->prepare(help.getSeq<HWND>());
		Assert::AreEqual(5, int(bar->data->rock_bar_brush_handle));
		Assert::AreEqual(7, int(bar->data->rock_wall_brush_handle));
		Assert::AreEqual(10, int(bar->data->icon_active_dc_handle));
		Assert::AreEqual(13, int(bar->data->icon_inactive_dc_handle));
		Assert::AreEqual(16, int(bar->data->knob_dc_handle));
		Assert::AreEqual(20, int(bar->data->buffer_dc_handle));
		Assert::AreEqual(13, int(bar->data->exits.size()));
		reverseClear(bar->data->exits);
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ROCK_BAR))), help.getLine());
		Assert::AreEqual(std::string("CreatePatternBrush"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ROCK_WALL))), help.getLine());
		Assert::AreEqual(std::string("CreatePatternBrush"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("GetDC"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ICON_ACTIVE))), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("9"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_ICON_INACTIVE))), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("13"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDB_KNOB))), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("15"), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CXSCREEN), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleBitmap"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("18"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("20"), help.getLine());
		Assert::AreEqual(std::string("19"), help.getLine());
		Assert::AreEqual(std::string("CreateFont"), help.getLine());
		Assert::AreEqual(stringize(FONT_SIZE), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(FW_NORMAL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(SHIFTJIS_CHARSET), help.getLine());
		Assert::AreEqual(stringize(OUT_DEFAULT_PRECIS), help.getLine());
		Assert::AreEqual(stringize(CLIP_DEFAULT_PRECIS), help.getLine());
		Assert::AreEqual(stringize(DEFAULT_QUALITY), help.getLine());
		Assert::AreEqual(stringize(DEFAULT_PITCH), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("ReleaseDC"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("22"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("20"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("19"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("15"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("13"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("DeleteDC"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("9"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("DeleteObject"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_register) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.dll_handle = help.getSeq<HINSTANCE>();
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			help << int(hInstance) << '\n';
			help << int(lpCursorName) << '\n';
			return help.getSeq<HCURSOR>();
		};
		api::RegisterClass_true = [&](WNDCLASSA *lpWndClass) -> ATOM {
			help << "RegisterClass\n";
			help << lpWndClass->style << '\n';
			help << int(lpWndClass->lpfnWndProc) << '\n';
			help << int(lpWndClass->hInstance) << '\n';
			help << int(lpWndClass->hCursor) << '\n';
			help << lpWndClass->lpszClassName << '\n';
			return help.getSeq<ATOM>();
		};
		api::UnregisterClass = [&](
			LPCSTR lpClassName, 
			HINSTANCE hInstance
		) -> BOOL {
			help << "UnregisterClass\n";
			help << lpClassName << '\n';
			help << int(hInstance) << '\n';
			return TRUE;
		};
		spy_jchat_bar_t::registerClass();
		Assert::AreEqual(1, int(context.exits.size()));
		reverseClear(context.exits);
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_ARROW))), help.getLine());
		Assert::AreEqual(std::string("RegisterClass"), help.getLine());
		Assert::AreEqual(stringize(int(CS_HREDRAW | CS_NOCLOSE)), help.getLine());
		Assert::AreEqual(stringize(int(&spy_jchat_bar_t::WindowProc)), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(std::string("UnregisterClass"), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_storeRegistry) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->registry.lines = {help.getSeqStr(), help.getSeqStr()};
		api::WritePrivateProfileString_true = [&](
			LPCSTR lpAppName, 
			LPCSTR lpKeyName, 
			LPCSTR lpString, 
			LPCSTR lpFileName
		) -> BOOL {
			help << "WritePrivateProfileString\n";
			help << lpAppName << '\n';
			if (lpKeyName) help << lpKeyName << '\n';
			else help << "(null)\n";
			if (lpString) help << lpString << '\n';
			else help << "(null)\n";
			help << lpFileName << '\n';
			return TRUE;
		};
		bar->storeRegistry();
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(std::string("(null)"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("Line_0"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Registry"), help.getLine());
		Assert::AreEqual(std::string("Line_1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_text_addChannel) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // チャンネルを付加しないか？
		test_helper_t help;
		context = {};
		context.ini.chat.command_symbols = "/#^";
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		Assert::AreEqual(std::string("/Hoge"), bar->text_addChannel("/Hoge"));
		Assert::AreEqual(std::string("#Hoge"), bar->text_addChannel("#Hoge"));
		Assert::AreEqual(std::string("^Hoge"), bar->text_addChannel("^Hoge"));
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // チャンネルを付加できるか？
		test_helper_t help;
		context = {};
		context.ini.chat.command_symbols = "/#^";
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->channel.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << lParam << '\n';
			static const int RESS[] = {0, 1, 2, 3, 4, 5, 6};
			static const int *res = RESS;
			return LRESULT(*res++);
		};
		Assert::AreEqual(std::string("/Auction Hoge"), bar->text_addChannel("Hoge"));
		Assert::AreEqual(std::string("/Gsay Hoge"), bar->text_addChannel("Hoge"));
		Assert::AreEqual(std::string("/gUildsay Hoge"), bar->text_addChannel("Hoge"));
		Assert::AreEqual(std::string("/Ooc Hoge"), bar->text_addChannel("Hoge"));
		Assert::AreEqual(std::string("/Reply Hoge"), bar->text_addChannel("Hoge"));
		Assert::AreEqual(std::string("/Say Hoge"), bar->text_addChannel("Hoge"));
		Assert::AreEqual(std::string("/sHout Hoge"), bar->text_addChannel("Hoge"));
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(CB_GETCURSEL), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_text_eqChatToJChat) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // リンクを含まないテキストを変換できるか？
		context = {};
		context.ini.chat.link_body_size = 2;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		const char *cmd = "Hoge";
		std::string text = bar->text_eqChatToJChat(cmd);
		Assert::AreEqual(std::string("Hoge"), text);
		Assert::IsTrue(bar->data->link_bodies.empty());
	}
	{ // リンクを1個含むテキストを変換できるか？
		context = {};
		context.ini.chat.link_body_size = 2;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		const char *cmd = "Hoge\x12""12Foo\x12""Fuga";
		std::string text = bar->text_eqChatToJChat(cmd);
		Assert::AreEqual(std::string("Hoge{0:Foo}Fuga"), text);
		Assert::AreEqual(std::size_t(1), bar->data->link_bodies.size());
		Assert::AreEqual(std::string("12"), bar->data->link_bodies[0]);
	}
	{ // リンクを2個含むテキストを変換できるか？
		context = {};
		context.ini.chat.link_body_size = 3;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		const char *cmd = "Hoge\x12""234Foo\x12""Fuga\x12""345Bar\x12""Piyo";
		bar->data->link_bodies = {"123"};
		std::string text = bar->text_eqChatToJChat(cmd);
		Assert::AreEqual(std::string("Hoge{1:Foo}Fuga{2:Bar}Piyo"), text);
		Assert::AreEqual(std::size_t(3), bar->data->link_bodies.size());
		Assert::AreEqual(std::string("234"), bar->data->link_bodies[1]);
		Assert::AreEqual(std::string("345"), bar->data->link_bodies[2]);
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_text_jChatToEQChat) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // リンクを含まないテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		std::string text = "Hoge";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"Hoge"), cmd);
	}
	{ // 分割記号のないリンクを含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->link_bodies = {"12"};
		std::string text = "Hoge{0Fuga";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"Hoge"), cmd);
	}
	{ // 終端のないリンクを含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->link_bodies = {"12"};
		std::string text = "Hoge{0:FooFuga";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"Hoge"), cmd);
	}
	{ // 負の番号のリンクを含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->link_bodies = {"12"};
		std::string text = "Hoge{-1:Foo}Fuga";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"HogeFuga"), cmd);
	}
	{ // サイズオーバーの番号のリンクを含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->link_bodies = {"12"};
		std::string text = "Hoge{2:Foo}Fuga";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"HogeFuga"), cmd);
	}
	{ // リンクを1個含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->link_bodies = {"12"};
		std::string text = "Hoge{0:Foo}Fuga";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"Hoge\x12""12Foo\x12""Fuga"), cmd);
	}
	{ // リンクを2個含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->link_bodies = {"123", "234", "345"};
		std::string text = "Hoge{1:Foo}Fuga{2:Bar}Piyo";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"Hoge\x12""234Foo\x12""Fuga\x12""345Bar\x12""Piyo"), cmd);
	}
	{ // MQ2の変数名を1個含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->link_bodies = {"12"};
		std::string text = "Hoge${Foo}Fuga";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"Hoge${Foo}Fuga"), cmd);
	}
	{ // 15バイトを超えるマルチバイト文字を含むテキストを変換できるか？
		context = {};
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		std::string text = "あいうえおかきくけこ";
		std::string cmd = bar->text_jChatToEQChat(text);
		Assert::AreEqual(std::string(u8"あいうえお かきくけこ"), cmd);
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_jchat_bar_transferText) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini.chat.link_body_size = 2;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->edit.handle = help.getSeq<HWND>();
		api::SendMessage = [&](
			HWND hWnd, 
			UINT Msg, 
			WPARAM wParam, 
			LPARAM lParam
		) -> LRESULT {
			help << "SendMessage\n";
			help << int(hWnd) << '\n';
			help << Msg << '\n';
			help << wParam << '\n';
			help << LPTSTR(lParam) << '\n';
			return 0;
		};
		bar->transferText("Hoge\x12""12Foo\x12""Fuga");
		Assert::AreEqual(1, int(bar->data->link_bodies.size()));
		Assert::AreEqual(std::string("12"), bar->data->link_bodies[0]);
		Assert::AreEqual(std::string("SendMessage"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(EM_REPLACESEL), help.getLine());
		Assert::AreEqual(stringize(TRUE), help.getLine());
		Assert::AreEqual(std::string("Hoge{0:Foo}Fuga"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_line_list) {
	{ // 正しく動作できるか？
		lines_and_iter_t list;
		Assert::IsTrue(list.lines.empty());
		Assert::IsTrue(list.iter == list.lines.begin());
		Assert::IsTrue(list.iter == list.lines.end());
		Assert::IsTrue(list.isBegin());
		Assert::IsTrue(list.isEnd());

		list.lines.emplace_back("Hoge");
		list.moveBegin();
		Assert::IsTrue(list.iter == list.lines.begin());
		Assert::IsTrue(list.isBegin());
		Assert::IsFalse(list.isEnd());
		Assert::AreEqual(std::string("Hoge"), *list.iter);
		list.moveEnd();
		Assert::IsTrue(list.iter == list.lines.end());
		Assert::IsFalse(list.isBegin());
		Assert::IsTrue(list.isEnd());
		Assert::AreEqual(std::string("Hoge"), *--list.iter);

		list.lines.emplace_back("Fuga");
		list.moveBegin();
		Assert::IsTrue(list.iter == list.lines.begin());
		Assert::IsTrue(list.isBegin());
		Assert::IsFalse(list.isEnd());
		Assert::AreEqual(std::string("Hoge"), *list.iter);
		list.moveEnd();
		Assert::IsTrue(list.iter == list.lines.end());
		Assert::IsFalse(list.isBegin());
		Assert::IsTrue(list.isEnd());
		Assert::AreEqual(std::string("Fuga"), *--list.iter);
	}
}

TEST_METHOD(test_makeKeySituation) {
	Assert::AreEqual(0x04030201, int(makeKeySituation(FALSE, KeyCombo{1, 2, 3, 4})));
	Assert::AreEqual(0x04030281, int(makeKeySituation(TRUE , KeyCombo{1, 2, 3, 4})));
}

TEST_METHOD(test_onGameStateChange) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 失敗できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		context.game_state = GAMESTATE_INGAME;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetWindowLong_true = [&](HWND hWnd, int nIndex) -> LONG {
			help << "GetWindowLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			return 0;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		onGameStateChange();
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(GWL_EXSTYLE), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetWindowLongA", 3), help.getLine().substr(24));
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // ゲーム中、チャットバーを未作成のときに正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.dll_handle = help.getSeq<HINSTANCE>();
		context.ini_path = help.getSeqStr();
		context.game_state = GAMESTATE_INGAME;
		context.ini.window.left = help.getSeq();
		context.ini.window.top = help.getSeq();
		context.ini.window.width = help.getSeq<std::size_t>();
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			lpPoint->x = help.getSeq<LONG>();
			lpPoint->y = help.getSeq<LONG>();
			return TRUE;
		};
		api::CreateWindowEx_true = [&](
			DWORD dwExStyle, 
			PCTSTR pszClassName, 
			PCTSTR pszWindowName, 
			DWORD dwStyle, 
			int x, 
			int y, 
			int nWidth, 
			int nHeight, 
			HWND hWndParent, 
			HMENU hMenu, 
			HINSTANCE hInstance, 
			PVOID pParam
		) -> HWND {
			help << "CreateWindowEx\n";
			help << dwExStyle << '\n';
			help << pszClassName << '\n';
			help << pszWindowName << '\n';
			help << dwStyle << '\n';
			help << x << '\n';
			help << y << '\n';
			help << nWidth << '\n';
			help << nHeight << '\n';
			help << int(hWndParent) << '\n';
			help << int(hMenu) << '\n';
			help << int(hInstance) << '\n';
			help << int(pParam) << '\n';
			return help.getSeq<HWND>();
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return 0;
		};
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, help.getSeqStr().c_str());
			return std::strlen(lpReturnedString);
		};
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			help << nIndex << '\n';
			return 200;
		};
		api::GetWindowLong_true = [&](HWND hWnd, int nIndex) -> LONG {
			help << "GetWindowLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			return 0;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			help << int(hInstance) << '\n';
			help << int(lpCursorName) << '\n';
			return help.getSeq<HCURSOR>();
		};
		api::RegisterClass_true = [&](WNDCLASSA *lpWndClass) -> ATOM {
			help << "RegisterClass\n";
			help << lpWndClass->style << '\n';
			help << int(lpWndClass->lpfnWndProc) << '\n';
			help << int(lpWndClass->hInstance) << '\n';
			help << int(lpWndClass->hCursor) << '\n';
			help << lpWndClass->lpszClassName << '\n';
			return help.getSeq<ATOM>();
		};
		api::SetWindowPos_true = [&](
			HWND hWnd, 
			HWND hWndInsertAfter, 
			int X, 
			int Y, 
			int cx, 
			int cy, 
			UINT uFlags
		) -> BOOL {
			help << "SetWindowPos\n";
			help << int(hWnd) << '\n';
			help << int(hWndInsertAfter) << '\n';
			help << X << '\n';
			help << Y << '\n';
			help << cx << '\n';
			help << cy << '\n';
			help << uFlags << '\n';
			return TRUE;
		};
		api::ShowWindow = [&](HWND hWnd, int nCmdShow) -> BOOL {
			help << "ShowWindow\n";
			help << int(hWnd) << '\n';
			help << nCmdShow << '\n';
			return FALSE;
		};
		api::UnregisterClass = [&](
			LPCSTR lpClassName, 
			HINSTANCE hInstance
		) -> BOOL {
			help << "UnregisterClass\n";
			help << lpClassName << '\n';
			help << int(hInstance) << '\n';
			return TRUE;
		};
		dtr::DetourAttach_true = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourAttach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourDetach = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourDetach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		onGameStateChange();
		Assert::AreEqual(int(screen_modes::WINDOW), int(context.screen_mode));
		Assert::AreEqual(15, int(bar->data->handle));
		Assert::AreEqual(5, int(context.detour_exits.size()));
		context.detour_exits.clear();
		Assert::AreEqual(1, int(context.exits.size()));
		reverseClear(context.exits);
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(GWL_EXSTYLE), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(DispatchMessage_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&DispatchMessage_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_InterpretCmd"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_InterpretCmd_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_InterpretCmd_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CXWndManager_DrawCursor"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CXWndManager_DrawCursor_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CXWndManager_DrawCursor_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyDown"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyDown_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyDown_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("KeypressHandler_HandleKeyUp"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyUp_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyUp_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(int(MAKEINTRESOURCE(IDC_EQ_ARROW))), help.getLine());
		Assert::AreEqual(std::string("RegisterClass"), help.getLine());
		Assert::AreEqual(stringize(int(CS_HREDRAW | CS_NOCLOSE)), help.getLine());
		Assert::AreEqual(stringize(int(&spy_jchat_bar_t::WindowProc)), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("11"), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(stringize(WS_POPUP), help.getLine());
		Assert::AreEqual(std::string("13"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(stringize(JCHAT_BAR_HEIGHT), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(stringize(NULL), help.getLine());
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("15"), help.getLine());
		Assert::AreEqual(std::string("ShowWindow"), help.getLine());
		Assert::AreEqual(std::string("15"), help.getLine());
		Assert::AreEqual(stringize(SW_SHOWNA), help.getLine());
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CXSCREEN), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CYSCREEN), help.getLine());
		Assert::AreEqual(std::string("SetWindowPos"), help.getLine());
		Assert::AreEqual(std::string("15"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("16"), help.getLine());
		Assert::AreEqual(std::string("17"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(SWP_NOACTIVATE | SWP_NOSIZE), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(DispatchMessage_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&DispatchMessage_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_InterpretCmd_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_InterpretCmd_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CXWndManager_DrawCursor_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CXWndManager_DrawCursor_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyDown_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyDown_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::KeypressHandler_HandleKeyUp_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::KeypressHandler_HandleKeyUp_detour))), help.getLine());
		Assert::AreEqual(std::string("UnregisterClass"), help.getLine());
		Assert::AreEqual(std::string(JCHAT_BAR_NAME), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // キャラセレクト中、チャットバーを未作成のときに正しく動作できるか？
		test_helper_t help;
		context = {};
		context.game_state = GAMESTATE_CHARSELECT;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		onGameStateChange();
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // キャラセレクト中、チャットバーを表示しているときに正しく動作できるか？
		test_helper_t help;
		context = {};
		context.game_state = GAMESTATE_CHARSELECT;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::ShowWindow = [&](HWND hWnd, int nCmdShow) -> BOOL {
			help << "ShowWindow\n";
			help << int(hWnd) << '\n';
			help << nCmdShow << '\n';
			return FALSE;
		};
		onGameStateChange();
		Assert::AreEqual(1, int(bar->data->handle));
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("ShowWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(SW_HIDE), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

TEST_METHOD(test_onProcessAttach) {
	{ // jchat.logを開けなかったときに失敗できるか？
		test_helper_t help;
		context = {};
		api::GetCommandLine = [&]() -> LPSTR {
			help << "GetCommandLine\n";
			return "";
		};
		api::GetModuleFileName_true = [&](
			HMODULE hModule,
			PTSTR pFilename,
			DWORD nSize
		) -> DWORD {
			help << "GetModuleFileName\n";
			help << int(hModule) << '\n';
			char *p = pFilename;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "\\"); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "."); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str());
			return std::strlen(pFilename);
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::make_ofstream_true = [&](
			const std::string &file, 
			std::ios_base::openmode mode
		) -> std::shared_ptr<std::ostream> {
			help << "make_ofstream\n";
			help << file << '\n';
			help << int(mode) << '\n';
			return help.getSeqPtr<std::ostream>();
		};
		std_::ostream_exceptions_set_true = [&](
			std::ostream *out, 
			std::ios::iostate except
		) {
			help << "ostream_exceptions_set\n";
			help << int(out) << '\n';
			help << int(except) << '\n';
			throw std::ios_base::failure(help.getSeqStr());
		};
		Assert::IsFalse(onProcessAttach(help.getSeq<HINSTANCE>()));
		Assert::AreEqual(0, int(context.log.get()));
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("2\\3.log"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // jchat.logを開いた後に失敗できるか？
		test_helper_t help;
		context = {};
		api::GetCommandLine = [&]() -> LPSTR {
			help << "GetCommandLine\n";
			return "";
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetModuleFileName_true = [&](
			HMODULE hModule,
			PTSTR pFilename,
			DWORD nSize
		) -> DWORD {
			help << "GetModuleFileName\n";
			help << int(hModule) << '\n';
			char *p = pFilename;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "\\"); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "."); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str());
			return std::strlen(pFilename);
		};
		api::GetModuleHandle_true = [&](LPCSTR lpModuleName) -> HMODULE {
			help << "GetModuleHandle\n";
			help << int(lpModuleName) << '\n';
			return NULL;
		};
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, "");
			return 0;
		};
		std_::make_ofstream_true = [&](
			const std::string &file, 
			std::ios_base::openmode mode
		) -> std::shared_ptr<std::ostream> {
			help << "make_ofstream\n";
			help << file << '\n';
			help << int(mode) << '\n';
			return help.getSeqPtr<std::ostream>();
		};
		std_::ostream_exceptions_set_true = [&](
			std::ostream *out, 
			std::ios::iostate except
		) {
			help << "ostream_exceptions_set\n";
			help << int(out) << '\n';
			help << int(except) << '\n';
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		Assert::IsFalse(onProcessAttach(help.getSeq<HINSTANCE>()));
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("2\\3.log"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("CommandSymbols"), help.getLine());
		Assert::AreEqual(std::string("/#^"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("FontName"), help.getLine());
		Assert::AreEqual(std::string("ＭＳ Ｐゴシック"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("LinkBodySize"), help.getLine());
		Assert::AreEqual(std::string("56"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Left"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("MinWidth"), help.getLine());
		Assert::AreEqual(std::string("150"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Top"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Width"), help.getLine());
		Assert::AreEqual(std::string("300"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetModuleHandle"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetModuleHandleA", 6), help.getLine().substr(24));
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		api::GetCommandLine = [&]() -> LPSTR {
			help << "GetCommandLine\n";
			return "";
		};
		api::GetModuleFileName_true = [&](
			HMODULE hModule,
			PTSTR pFilename,
			DWORD nSize
		) -> DWORD {
			help << "GetModuleFileName\n";
			help << int(hModule) << '\n';
			char *p = pFilename;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "\\"); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "."); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str());
			return std::strlen(pFilename);
		};
		api::GetModuleHandle_true = [&](LPCSTR lpModuleName) -> HMODULE {
			help << "GetModuleHandle\n";
			help << int(lpModuleName) << '\n';
			return HMODULE(0x400000 + help.getSeq());
		};
		api::GetPrivateProfileString = [&](
			LPCSTR lpAppName,
			LPCTSTR lpKeyName,
			LPCTSTR lpDefault,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpDefault << '\n';
			help << lpFileName << '\n';
			std::strcpy(lpReturnedString, help.getSeqStr().c_str());
			return std::strlen(lpReturnedString);
		};
		dtr::DetourAttach_true = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourAttach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourDetach = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourDetach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::make_ofstream_true = [&](
			const std::string &file, 
			std::ios_base::openmode mode
		) -> std::shared_ptr<std::ostream> {
			help << "make_ofstream\n";
			help << file << '\n';
			help << int(mode) << '\n';
			return help.getSeqPtr<std::ostream>();
		};
		std_::ostream_exceptions_set_true = [&](
			std::ostream *out, 
			std::ios::iostate except
		) {
			help << "ostream_exceptions_set\n";
			help << int(out) << '\n';
			help << int(except) << '\n';
		};
		Assert::IsTrue(onProcessAttach(help.getSeq<HINSTANCE>()));
		Assert::AreEqual(2, int(context.detour_exits.size()));
		context.detour_exits.clear();
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("2\\3.log"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("CommandSymbols"), help.getLine());
		Assert::AreEqual(std::string("/#^"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("FontName"), help.getLine());
		Assert::AreEqual(std::string("ＭＳ Ｐゴシック"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Chat"), help.getLine());
		Assert::AreEqual(std::string("LinkBodySize"), help.getLine());
		Assert::AreEqual(std::string("56"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Left"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("MinWidth"), help.getLine());
		Assert::AreEqual(std::string("150"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Top"), help.getLine());
		Assert::AreEqual(std::string("-10000"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Window"), help.getLine());
		Assert::AreEqual(std::string("Width"), help.getLine());
		Assert::AreEqual(std::string("300"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetModuleHandle"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CharSpawn"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("MainWindowHandle"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("NotInChatMode"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("CEverQuest_SetGameState"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_SetGameState_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_SetGameState_detour))), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("Offset"), help.getLine());
		Assert::AreEqual(std::string("ProcessGameEvents"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::ProcessGameEvents_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::ProcessGameEvents_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::CEverQuest_SetGameState_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::CEverQuest_SetGameState_detour))), help.getLine());
		Assert::AreEqual(std::string("DetourDetach"), help.getLine());
		Assert::AreEqual(stringize(int(&(PVOID&)(eqgame_t::ProcessGameEvents_target))), help.getLine());
		Assert::AreEqual(stringize(int(indirect_cast<PVOID>(&eqgame_t::ProcessGameEvents_detour))), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
	
TEST_METHOD(test_onProcessDetach) {
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		Assert::IsTrue(onProcessDetach());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_onScreenModeChange) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // アイコンモードのときに正しく動作できるか？
		context = {};
		context.screen_mode = screen_modes::ICONIC;
		onScreenModeChange();
	}
	{ // 失敗できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		context.screen_mode = screen_modes::WINDOW;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->mw_client_pos.x = help.getSeq<LONG>();
		bar->data->mw_client_pos.y = help.getSeq<LONG>();
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			return FALSE;
		};
		api::DestroyWindow = [&](HWND hWnd) -> BOOL {
			help << "DestroyWindow\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		onScreenModeChange();
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::ClientToScreen", 6), help.getLine().substr(24));
		Assert::AreEqual(std::string("DestroyWindow"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 位置ズレを修正できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.screen_mode = screen_modes::WINDOW;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		bar->data->mw_client_pos.x = help.getSeq<LONG>();
		bar->data->mw_client_pos.y = help.getSeq<LONG>();
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			lpPoint->x = help.getSeq<LONG>();
			lpPoint->y = help.getSeq<LONG>();
			return TRUE;
		};
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			help << nIndex << '\n';
			return 200;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::SetWindowPos_true = [&](
			HWND hWnd, 
			HWND hWndInsertAfter, 
			int X, 
			int Y, 
			int cx, 
			int cy, 
			UINT uFlags
		) -> BOOL {
			help << "SetWindowPos\n";
			help << int(hWnd) << '\n';
			help << int(hWndInsertAfter) << '\n';
			help << X << '\n';
			help << Y << '\n';
			help << cx << '\n';
			help << cy << '\n';
			help << uFlags << '\n';
			return TRUE;
		};
		onScreenModeChange();
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CXSCREEN), help.getLine());
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(stringize(SM_CYSCREEN), help.getLine());
		Assert::AreEqual(std::string("SetWindowPos"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(stringize(SWP_NOACTIVATE | SWP_NOSIZE), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
		
TEST_METHOD(test_refreshGameState) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // ゲームの状態がゼロ→ゼロのときに無視できるか？
		test_helper_t help;
		context = {};
		context.game_state = 0;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		refreshGameState(0);
		Assert::AreEqual(0, context.game_state);
	}
	{ // ゲームの状態がキャラセレクト→キャラセレクトのときに無視できるか？
		test_helper_t help;
		context = {};
		context.game_state = GAMESTATE_CHARSELECT;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		refreshGameState(0);
		Assert::AreEqual(GAMESTATE_CHARSELECT, context.game_state);
	}
	{ // ゲームの状態がゼロ→キャラセレクトのときにイベントを発生できるか？
		test_helper_t help;
		context = {};
		context.game_state = 0;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		refreshGameState(GAMESTATE_CHARSELECT);
		Assert::AreEqual(GAMESTATE_CHARSELECT, context.game_state);
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}
	
TEST_METHOD(test_refreshScreenMode) {
	BEGIN_DEF_SPY_CLASS(jchat_bar_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		HWND MainWindowHandle = help.getSeq<HWND>();
		eqgame_t::MainWindowHandle = &MainWindowHandle;
		context = {};
		context.screen_mode = screen_modes::WINDOW;
		auto bar = (spy_jchat_bar_t*)(&context.jchat_bar);
		bar->data->handle = help.getSeq<HWND>();
		api::GetWindowLong_true = [&](HWND hWnd, int nIndex) -> LONG {
			help << "GetWindowLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			return WS_EX_TOPMOST;
		};
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		refreshScreenMode();
		Assert::AreEqual(int(screen_modes::FULL_SCREEN), int(context.screen_mode));
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(GWL_EXSTYLE), help.getLine());
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(jchat_bar_t);
}

};

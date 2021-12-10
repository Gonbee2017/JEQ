#include "common.hpp"

#include <CppUnitTest.h>
#include <windows.h>

#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(api_test) {
					
TEST_METHOD(test_BeginPaint) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::BeginPaint_true = [&](
			HWND hWnd, 
			LPPAINTSTRUCT lpPaint
		) -> HDC {
			help << "BeginPaint\n";
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::BeginPaint(help.getSeq<HWND>(1), help.getSeq<LPPAINTSTRUCT>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::BeginPaint", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("BeginPaint"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::BeginPaint_true = [&](
			HWND hWnd, 
			LPPAINTSTRUCT lpPaint
		) -> HDC {
			help << "BeginPaint\n";
			help << int(hWnd) << '\n';
			help << int(lpPaint) << '\n';
			return help.getSeq<HDC>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::BeginPaint(help.getSeq<HWND>(1), help.getSeq<LPPAINTSTRUCT>(2))));
		Assert::AreEqual(std::string("BeginPaint"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
						
TEST_METHOD(test_BitBlt) {
	{ // エラーをスローできるか？
		test_helper_t help;
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
			return FALSE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::BitBlt(
				help.getSeq<HDC>(1), 
				help.getSeq(2), 
				help.getSeq(3), 
				help.getSeq(4), 
				help.getSeq(5), 
				help.getSeq<HDC>(6), 
				help.getSeq(7), 
				help.getSeq(8), 
				help.getSeq(9)
			);
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::BitBlt", 10), err.getMessage());
		}
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::IsTrue(api::BitBlt(
			help.getSeq<HDC>(1), 
			help.getSeq(2), 
			help.getSeq(3), 
			help.getSeq(4), 
			help.getSeq(5), 
			help.getSeq<HDC>(6), 
			help.getSeq(7), 
			help.getSeq(8), 
			help.getSeq(9)
		));
		Assert::AreEqual(std::string("BitBlt"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("9"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ClientToScreen) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			return FALSE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::ClientToScreen(help.getSeq<HWND>(1), help.getSeq<LPPOINT>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::ClientToScreen", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << int(lpPoint) << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(api::ClientToScreen(help.getSeq<HWND>(1), help.getSeq<LPPOINT>(2)));
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_CreateCompatibleBitmap) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::CreateCompatibleBitmap_true = [&](
			HDC hdc, 
			int cx, 
			int cy
		) -> HBITMAP {
			help << "CreateCompatibleBitmap\n";
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::CreateCompatibleBitmap(help.getSeq<HDC>(1), help.getSeq(2), help.getSeq(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::CreateCompatibleBitmap", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("CreateCompatibleBitmap"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::CreateCompatibleBitmap_true = [&](
			HDC hdc, 
			int cx, 
			int cy
		) -> HBITMAP {
			help << "CreateCompatibleBitmap\n";
			help << int(hdc) << '\n';
			help << cx << '\n';
			help << cy << '\n';
			return HBITMAP(help.getSeq());
		};
		help.setSeqBase();
		Assert::AreEqual(4, int(api::CreateCompatibleBitmap(help.getSeq<HDC>(1), help.getSeq(2), help.getSeq(3))));
		Assert::AreEqual(std::string("CreateCompatibleBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
	
TEST_METHOD(test_CreateCompatibleDC) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::CreateCompatibleDC_true = [&](HDC hdc) -> HDC {
			help << "CreateCompatibleDC\n";
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			api::CreateCompatibleDC(help.getSeq<HDC>(1));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::CreateCompatibleDC", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::CreateCompatibleDC_true = [&](HDC hdc) -> HDC {
			help << "CreateCompatibleDC\n";
			help << int(hdc) << '\n';
			return help.getSeq<HDC>();
		};
		Assert::AreEqual(2, int(api::CreateCompatibleDC(help.getSeq<HDC>())));
		Assert::AreEqual(std::string("CreateCompatibleDC"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
	
TEST_METHOD(test_CreateFont) {
	{ // エラーをスローできるか？
		test_helper_t help;
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
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::CreateFont(
				help.getSeq(1),
				help.getSeq(2),
				help.getSeq(3),
				help.getSeq(4),
				help.getSeq(5),
				help.getSeq<DWORD>(6),
				help.getSeq<DWORD>(7),
				help.getSeq<DWORD>(8),
				help.getSeq<DWORD>(9),
				help.getSeq<DWORD>(10),
				help.getSeq<DWORD>(11),
				help.getSeq<DWORD>(12),
				help.getSeq<DWORD>(13),
				help.getSeqStr(14).c_str()
			);
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::CreateFontA", 15), err.getMessage());
		}
		Assert::AreEqual(std::string("CreateFont"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::AreEqual(15, int(api::CreateFont(
			help.getSeq(1),
			help.getSeq(2),
			help.getSeq(3),
			help.getSeq(4),
			help.getSeq(5),
			help.getSeq<DWORD>(6),
			help.getSeq<DWORD>(7),
			help.getSeq<DWORD>(8),
			help.getSeq<DWORD>(9),
			help.getSeq<DWORD>(10),
			help.getSeq<DWORD>(11),
			help.getSeq<DWORD>(12),
			help.getSeq<DWORD>(13),
			help.getSeqStr(14).c_str()
		)));
		Assert::AreEqual(std::string("CreateFont"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("9"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("11"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string("13"), help.getLine());
		Assert::AreEqual(std::string("14"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_CreatePatternBrush) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::CreatePatternBrush_true = [&](HBITMAP hbm) -> HBRUSH {
			help << "CreatePatternBrush\n";
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			api::CreatePatternBrush(help.getSeq<HBITMAP>());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::CreatePatternBrush", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("CreatePatternBrush"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::CreatePatternBrush_true = [&](HBITMAP hbm) -> HBRUSH {
			help << "CreatePatternBrush\n";
			help << int(hbm) << '\n';
			return help.getSeq<HBRUSH>();
		};
		Assert::AreEqual(2, int(api::CreatePatternBrush(help.getSeq<HBITMAP>())));
		Assert::AreEqual(std::string("CreatePatternBrush"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_CreatePen) {
	{ // エラーをスルーするか？
		test_helper_t help;
		api::CreatePen_true = [&](
			int iStyle, 
			int cWidth, 
			COLORREF color
		) -> HPEN {
			help << "CreatePen\n";
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::CreatePen(help.getSeq(1), help.getSeq(2), help.getSeq<COLORREF>(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::CreatePen", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::AreEqual(4, int(api::CreatePen(help.getSeq(1), help.getSeq(2), help.getSeq<COLORREF>(3))));
		Assert::AreEqual(std::string("CreatePen"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_CreateWindowEx) {
	{ // エラーをスローできるか？
		test_helper_t help;
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
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::CreateWindowEx(
				help.getSeq<DWORD>(1),
				help.getSeqStr(2).c_str(),
				help.getSeqStr(3).c_str(),
				help.getSeq<DWORD>(4),
				help.getSeq(5),
				help.getSeq(6),
				help.getSeq(7),
				help.getSeq(8),
				help.getSeq<HWND>(9),
				help.getSeq<HMENU>(10),
				help.getSeq<HINSTANCE>(11),
				help.getSeq<PVOID>(12)
			);
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::CreateWindowExA", 13), err.getMessage());
		}
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::AreEqual(13, int(api::CreateWindowEx(
			help.getSeq<DWORD>(1),
			help.getSeqStr(2).c_str(),
			help.getSeqStr(3).c_str(),
			help.getSeq<DWORD>(4),
			help.getSeq(5),
			help.getSeq(6),
			help.getSeq(7),
			help.getSeq(8),
			help.getSeq<HWND>(9),
			help.getSeq<HMENU>(10),
			help.getSeq<HINSTANCE>(11),
			help.getSeq<PVOID>(12)
		)));
		Assert::AreEqual(std::string("CreateWindowEx"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("9"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("11"), help.getLine());
		Assert::AreEqual(std::string("12"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_FillRect) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::FillRect_true = [&]( 
			HDC hDC, 
			const RECT *lprc, 
			HBRUSH hbr
		) -> int {
			help << "FillRect\n";
			return 0;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::FillRect(help.getSeq<HDC>(1), help.getSeq<const RECT*>(2), help.getSeq<HBRUSH>(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::FillRect", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("FillRect"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::FillRect_true = [&]( 
			HDC hDC, 
			const RECT *lprc, 
			HBRUSH hbr
		) -> int {
			help << "FillRect\n";
			help << int(hDC) << '\n';
			help << int(lprc) << '\n';
			help << int(hbr) << '\n';
			return help.getSeq();
		};
		help.setSeqBase();
		Assert::AreEqual(4, api::FillRect(help.getSeq<HDC>(1), help.getSeq<const RECT*>(2), help.getSeq<HBRUSH>(3)));
		Assert::AreEqual(std::string("FillRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetClientRect) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetClientRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetClientRect\n";
			return FALSE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::GetClientRect(help.getSeq<HWND>(1), help.getSeq<LPRECT>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetClientRect", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("GetClientRect"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetClientRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetClientRect\n";
			help << int(hWnd) << '\n';
			help << int(lpRect) << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(api::GetClientRect(help.getSeq<HWND>(1), help.getSeq<LPRECT>(2)));
		Assert::AreEqual(std::string("GetClientRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetCursorPos) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			return FALSE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			api::GetCursorPos(help.getSeq<LPPOINT>());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetCursorPos", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			help << int(lpPoint) << '\n';
			return TRUE;
		};
		Assert::IsTrue(api::GetCursorPos(help.getSeq<LPPOINT>()));
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetDC) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetDC_true = [&](HWND hWnd) -> HDC {
			help << "GetDC\n";
			return NULL;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			api::GetDC(help.getSeq<HWND>());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetDC", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("GetDC"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetDC_true = [&](HWND hWnd) -> HDC {
			help << "GetDC\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HDC>();
		};
		Assert::AreEqual(2, int(api::GetDC(help.getSeq<HWND>())));
		Assert::AreEqual(std::string("GetDC"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetModuleFileName) {
	{ // エラーをスローできるか？
		test_helper_t help;
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
			return 0;
		};
		try {
			help.setSeqBase();
			api::GetModuleFileName(help.getSeq<HMODULE>(1), help.getSeqStr(2).data(), help.getSeq<DWORD>(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetModuleFileNameA", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetModuleFileName_true = [&](
			HMODULE hModule, 
			PTSTR pFilename, 
			DWORD nSize
		) -> DWORD {
			help << "GetModuleFileName\n";
			help << int(hModule) << '\n';
			help << pFilename << '\n';
			help << nSize << '\n';
			return help.getSeq<DWORD>();
		};
		help.setSeqBase();
		Assert::AreEqual(4, int(api::GetModuleFileName(help.getSeq<HMODULE>(1), help.getSeqStr(2).data(), help.getSeq<DWORD>(3))));
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetModuleHandle) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetModuleHandle_true = [&](LPCSTR lpModuleName) -> HMODULE {
			help << "GetModuleHandle\n";
			return NULL;
		};
		try {
			api::GetModuleHandle(help.getSeqStr().c_str());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetModuleHandleA", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("GetModuleHandle"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetModuleHandle_true = [&](LPCSTR lpModuleName) -> HMODULE {
			help << "GetModuleHandle\n";
			help << lpModuleName << '\n';
			return help.getSeq<HMODULE>();
		};
		Assert::AreEqual(2, int(api::GetModuleHandle(help.getSeqStr().c_str())));
		Assert::AreEqual(std::string("GetModuleHandle"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetProcAddress) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetProcAddress_true = [&](
			HMODULE hModule, 
			LPCSTR lpProcName
		) -> FARPROC {
			help << "GetProcAddress\n";
			return NULL;
		};
		try {
			help.setSeqBase();
			api::GetProcAddress(help.getSeq<HMODULE>(), help.getSeqStr().c_str());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetProcAddress", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("GetProcAddress"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetProcAddress_true = [&](
			HMODULE hModule, 
			LPCSTR lpProcName
		) -> FARPROC {
			help << "GetProcAddress\n";
			help << int(hModule) << '\n';
			help << lpProcName << '\n';
			return help.getSeq<FARPROC>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::GetProcAddress(help.getSeq<HMODULE>(1), help.getSeqStr(2).c_str())));
		Assert::AreEqual(std::string("GetProcAddress"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetSystemMetrics) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			return 0;
		};
		try {
			api::GetSystemMetrics(help.getSeq());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetSystemMetrics", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetSystemMetrics_true = [&](int nIndex) -> int {
			help << "GetSystemMetrics\n";
			help << nIndex << '\n';
			return help.getSeq();
		};
		Assert::AreEqual(2, api::GetSystemMetrics(help.getSeq()));
		Assert::AreEqual(std::string("GetSystemMetrics"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetWindowLong) {
	{ // エラーをスローできるか？
		test_helper_t help;
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
		try {
			help.setSeqBase();
			api::GetWindowLong(help.getSeq<HWND>(1), help.getSeq(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetWindowLongA", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // ゼロを返せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::AreEqual(0, int(api::GetWindowLong(help.getSeq<HWND>(1), help.getSeq(2))));
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // ゼロ以外を返せるか？
		test_helper_t help;
		api::GetWindowLong_true = [&](HWND hWnd, int nIndex) -> LONG {
			help << "GetWindowLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			return help.getSeq<LONG>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::GetWindowLong(help.getSeq<HWND>(1), help.getSeq(2))));
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetWindowRect) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::GetWindowRect(help.getSeq<HWND>(1), help.getSeq<LPRECT>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetWindowRect", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			help << int(lpRect) << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(api::GetWindowRect(help.getSeq<HWND>(1), help.getSeq<LPRECT>(2)));
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_GetWindowText) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			return 0;
		};
		try {
			help.setSeqBase();
			api::GetWindowText(help.getSeq<HWND>(1), help.getSeq<LPSTR>(2), help.getSeq(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::GetWindowTextA", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 空文字列を取得できるか？
		test_helper_t help;
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
			help << int(lpString) << '\n';
			help << nMaxCount << '\n';
			return 0;
		};
		help.setSeqBase();
		Assert::AreEqual(0, api::GetWindowText(help.getSeq<HWND>(1), help.getSeq<LPSTR>(2), help.getSeq(3)));
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 長さ1の文字列を取得できるか？
		test_helper_t help;
		api::GetWindowText_true = [&](
			HWND hWnd, 
			LPSTR lpString, 
			int nMaxCount
		) -> int {
			help << "GetWindowText\n";
			help << int(hWnd) << '\n';
			help << int(lpString) << '\n';
			help << nMaxCount << '\n';
			return 1;
		};
		help.setSeqBase();
		Assert::AreEqual(1, api::GetWindowText(help.getSeq<HWND>(1), help.getSeq<LPSTR>(2), help.getSeq(3)));
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_InvalidateRect) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::InvalidateRect_true = [&](
			HWND hWnd, 
			const RECT *lpRect, 
			BOOL bErase
		) -> BOOL {
			help << "InvalidateRect\n";
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::InvalidateRect(help.getSeq<HWND>(1), help.getSeq<const RECT*>(2), help.getSeq(3) % 2);
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::InvalidateRect", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("InvalidateRect"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::InvalidateRect_true = [&](
			HWND hWnd, 
			const RECT *lpRect, 
			BOOL bErase
		) -> BOOL {
			help << "InvalidateRect\n";
			help << int(hWnd) << '\n';
			help << int(lpRect) << '\n';
			help << bErase << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(int(api::InvalidateRect(help.getSeq<HWND>(1), help.getSeq<const RECT*>(2), help.getSeq(3) % 2)));
		Assert::AreEqual(std::string("InvalidateRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_LoadBitmap) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::LoadBitmap_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpBitmapName
		) -> HBITMAP {
			help << "LoadBitmap\n";
			return NULL;
		};
		try {
			help.setSeqBase();
			api::LoadBitmap(help.getSeq<HINSTANCE>(), help.getSeqStr().c_str());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::LoadBitmapA", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::LoadBitmap_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpBitmapName
		) -> HBITMAP {
			help << "LoadBitmap\n";
			help << int(hInstance) << '\n';
			help << lpBitmapName << '\n';
			return help.getSeq<HBITMAP>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::LoadBitmap(help.getSeq<HINSTANCE>(1), help.getSeqStr(2).c_str())));
		Assert::AreEqual(std::string("LoadBitmap"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_LoadCursor) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			return NULL;
		};
		try {
			help.setSeqBase();
			api::LoadCursor(help.getSeq<HINSTANCE>(1), help.getSeqStr(2).c_str());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::LoadCursorA", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::LoadCursor_true = [&](
			HINSTANCE hInstance, 
			LPCSTR lpCursorName
		) -> HCURSOR {
			help << "LoadCursor\n";
			help << int(hInstance) << '\n';
			help << lpCursorName << '\n';
			return help.getSeq<HCURSOR>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::LoadCursor(help.getSeq<HINSTANCE>(1), help.getSeqStr(2).c_str())));
		Assert::AreEqual(std::string("LoadCursor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_LoadLibrary) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::LoadLibrary_true = [&](LPCSTR lpLibFileName) -> HMODULE {
			help << "LoadLibrary\n";
			return NULL;
		};
		try {
			api::LoadLibrary(help.getSeqStr().c_str());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::LoadLibraryA", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::LoadLibrary_true = [&](LPCSTR lpLibFileName) -> HMODULE {
			help << "LoadLibrary\n";
			help << lpLibFileName << '\n';
			return help.getSeq<HMODULE>();
		};
		Assert::AreEqual(2, int(api::LoadLibrary(help.getSeqStr().c_str())));
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_MoveWindow) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::MoveWindow_true = [&](
			HWND hWnd, 
			int X, 
			int Y, 
			int nWidth, 
			int nHeight, 
			BOOL bRepaint
		) -> BOOL {
			help << "MoveWindow\n";
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::MoveWindow(help.getSeq<HWND>(1), help.getSeq(2), help.getSeq(3), help.getSeq(4), help.getSeq(5), help.getSeq(6) % 2);
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::MoveWindow", 7), err.getMessage());
		}
		Assert::AreEqual(std::string("MoveWindow"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::IsTrue(api::MoveWindow(help.getSeq<HWND>(1), help.getSeq(2), help.getSeq(3), help.getSeq(4), help.getSeq(5), help.getSeq(6) % 2));
		Assert::AreEqual(std::string("MoveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_Polyline) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::Polyline_true = [&](
			HDC hdc, 
			const POINT *apt, 
			int cpt
		) -> BOOL {
			help << "Polyline\n";
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::Polyline(help.getSeq<HDC>(1), help.getSeq<const POINT*>(2), help.getSeq(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::Polyline", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::Polyline_true = [&](
			HDC hdc, 
			const POINT *apt, 
			int cpt
		) -> BOOL {
			help << "Polyline\n";
			help << int(hdc) << '\n';
			help << int(apt) << '\n';
			help << cpt << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(api::Polyline(help.getSeq<HDC>(1), help.getSeq<const POINT*>(2), help.getSeq(3)));
		Assert::AreEqual(std::string("Polyline"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_RegisterClass) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::RegisterClass_true = [&](WNDCLASSA *lpWndClass) -> ATOM {
			help << "RegisterClass\n";
			return 0;
		};
		try {
			api::RegisterClass(help.getSeq<WNDCLASSA*>());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::RegisterClassA", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("RegisterClass"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::RegisterClass_true = [&](WNDCLASSA *lpWndClass) -> ATOM {
			help << "RegisterClass\n";
			help << int(lpWndClass) << '\n';
			return help.getSeq<ATOM>();
		};
		Assert::AreEqual(2, int(api::RegisterClass(help.getSeq<WNDCLASSA*>())));
		Assert::AreEqual(std::string("RegisterClass"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ScreenToClient) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::ScreenToClient_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ScreenToClient\n";
			return FALSE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		try {
			help.setSeqBase();
			api::ScreenToClient(help.getSeq<HWND>(1), help.getSeq<LPPOINT>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::ScreenToClient", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("ScreenToClient"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::ScreenToClient_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ScreenToClient\n";
			help << int(hWnd) << '\n';
			help << int(lpPoint) << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(api::ScreenToClient(help.getSeq<HWND>(1), help.getSeq<LPPOINT>(2)));
		Assert::AreEqual(std::string("ScreenToClient"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SelectObject) {
	{ // NULLによるエラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			return NULL;
		};
		try {
			help.setSeqBase();
			api::SelectObject(help.getSeq<HDC>(1), help.getSeq<HGDIOBJ>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SelectObject", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // HGDI_ERRORによるエラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			return HGDI_ERROR;
		};
		try {
			help.setSeqBase();
			api::SelectObject(help.getSeq<HDC>(1), help.getSeq<HGDIOBJ>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SelectObject", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SelectObject_true = [&](HDC hdc, HGDIOBJ h) -> HGDIOBJ {
			help << "SelectObject\n";
			help << int(hdc) << '\n';
			help << int(h) << '\n';
			return help.getSeq<HGDIOBJ>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::SelectObject(help.getSeq<HDC>(1), help.getSeq<HGDIOBJ>(2))));
		Assert::AreEqual(std::string("SelectObject"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetActiveWindow) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			return NULL;
		};
		try {
			api::SetActiveWindow(help.getSeq<HWND>());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetActiveWindow", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetActiveWindow_true = [&](HWND hWnd) -> HWND {
			help << "SetActiveWindow\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		Assert::AreEqual(2, int(api::SetActiveWindow(help.getSeq<HWND>())));
		Assert::AreEqual(std::string("SetActiveWindow"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetBkColor) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetBkColor_true = [&](
			HDC hdc, 
			COLORREF color
		) -> COLORREF {
			help << "SetBkColor\n";
			return CLR_INVALID;
		};
		try {
			help.setSeqBase();
			api::SetBkColor(help.getSeq<HDC>(1), help.getSeq<COLORREF>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetBkColor", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("SetBkColor"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetBkColor_true = [&](
			HDC hdc, 
			COLORREF color
		) -> COLORREF {
			help << "SetBkColor\n";
			help << int(hdc) << '\n';
			help << color << '\n';
			return help.getSeq<COLORREF>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::SetBkColor(help.getSeq<HDC>(1), help.getSeq<COLORREF>(2))));
		Assert::AreEqual(std::string("SetBkColor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetBkMode) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetBkMode_true = [&](HDC hdc, int mode) -> int {
			help << "SetBkMode\n";
			return 0;
		};
		try {
			help.setSeqBase();
			api::SetBkMode(help.getSeq<HDC>(1), help.getSeq(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetBkMode", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("SetBkMode"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetBkMode_true = [&](HDC hdc, int mode) -> int {
			help << "SetBkMode\n";
			help << int(hdc) << '\n';
			help << mode << '\n';
			return help.getSeq();
		};
		help.setSeqBase();
		Assert::AreEqual(3, api::SetBkMode(help.getSeq<HDC>(1), help.getSeq(2)));
		Assert::AreEqual(std::string("SetBkMode"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetBrushOrgEx) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetBrushOrgEx_true = [&](
			HDC hdc, 
			int x, 
			int y, 
			LPPOINT lppt
		) -> BOOL {
			help << "SetBrushOrgEx\n";
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::SetBrushOrgEx(help.getSeq<HDC>(1), help.getSeq(2), help.getSeq(3), help.getSeq<LPPOINT>(4));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetBrushOrgEx", 5), err.getMessage());
		}
		Assert::AreEqual(std::string("SetBrushOrgEx"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::IsTrue(api::SetBrushOrgEx(help.getSeq<HDC>(1), help.getSeq(2), help.getSeq(3), help.getSeq<LPPOINT>(4)));
		Assert::AreEqual(std::string("SetBrushOrgEx"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetClassLong) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetClassLong_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG dwNewLong
		) -> DWORD {
			help << "SetClassLong\n";
			return 0;
		};
		try {
			help.setSeqBase();
			api::SetClassLong(help.getSeq<HWND>(1), help.getSeq(2), help.getSeq<LONG>(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetClassLongA", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("SetClassLong"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetClassLong_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG dwNewLong
		) -> DWORD {
			help << "SetClassLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << dwNewLong << '\n';
			return help.getSeq();
		};
		help.setSeqBase();
		Assert::AreEqual(4, int(api::SetClassLong(help.getSeq<HWND>(1), help.getSeq(2), help.getSeq<LONG>(3))));
		Assert::AreEqual(std::string("SetClassLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetFocus) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetFocus_true = [&](HWND hWnd) -> HWND {
			help << "SetFocus\n";
			return NULL;
		};
		try {
			api::SetFocus(help.getSeq<HWND>());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetFocus", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetFocus_true = [&](HWND hWnd) -> HWND {
			help << "SetFocus\n";
			help << int(hWnd) << '\n';
			return help.getSeq<HWND>();
		};
		Assert::AreEqual(2, int(api::SetFocus(help.getSeq<HWND>())));
		Assert::AreEqual(std::string("SetFocus"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetTextColor) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetTextColor_true = [&](
			HDC hdc, 
			COLORREF color
		) -> COLORREF {
			help << "SetTextColor\n";
			return CLR_INVALID;
		};
		try {
			help.setSeqBase();
			api::SetTextColor(help.getSeq<HDC>(1), help.getSeq<COLORREF>(2));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetTextColor", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("SetTextColor"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetTextColor_true = [&](
			HDC hdc, 
			COLORREF color
		) -> COLORREF {
			help << "SetTextColor\n";
			help << int(hdc) << '\n';
			help << color << '\n';
			return help.getSeq<COLORREF>();
		};
		help.setSeqBase();
		Assert::AreEqual(3, int(api::SetTextColor(help.getSeq<HDC>(1), help.getSeq<COLORREF>(2))));
		Assert::AreEqual(std::string("SetTextColor"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetWindowLongPtr) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetWindowLongPtr_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG_PTR dwNewLong
		) -> LONG_PTR {
			help << "SetWindowLongPtr\n";
			return 0;
		};
		try {
			help.setSeqBase();
			api::SetWindowLongPtr(help.getSeq<HWND>(1), help.getSeq(2), help.getSeq<LONG_PTR>(3));
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetWindowLongA", 4), err.getMessage());
		}
		Assert::AreEqual(std::string("SetWindowLongPtr"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetWindowLongPtr_true = [&](
			HWND hWnd, 
			int nIndex, 
			LONG_PTR dwNewLong
		) -> LONG_PTR {
			help << "SetWindowLongPtr\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			help << int(dwNewLong) << '\n';
			return help.getSeq<LONG_PTR>();
		};
		help.setSeqBase();
		Assert::AreEqual(4, int(api::SetWindowLongPtr(help.getSeq<HWND>(1), help.getSeq(2), help.getSeq<LONG_PTR>(3))));
		Assert::AreEqual(std::string("SetWindowLongPtr"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetWindowPos) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
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
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::SetWindowPos(
				help.getSeq<HWND>(1), 
				help.getSeq<HWND>(2), 
				help.getSeq(3), 
				help.getSeq(4), 
				help.getSeq(5), 
				help.getSeq(6), 
				help.getSeq<UINT>(7)
			);
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetWindowPos", 8), err.getMessage());
		}
		Assert::AreEqual(std::string("SetWindowPos"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::IsTrue(api::SetWindowPos(
			help.getSeq<HWND>(1), 
			help.getSeq<HWND>(2), 
			help.getSeq(3), 
			help.getSeq(4), 
			help.getSeq(5), 
			help.getSeq(6), 
			help.getSeq<UINT>(7)
		));
		Assert::AreEqual(std::string("SetWindowPos"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_SetWindowText) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::SetWindowText(help.getSeq<HWND>(1), help.getSeqStr(2).c_str());
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::SetWindowTextA", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::SetWindowText_true = [&](
			HWND hWnd, 
			LPCSTR lpString
		) -> BOOL {
			help << "SetWindowText\n";
			help << int(hWnd) << '\n';
			help << lpString << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(api::SetWindowText(help.getSeq<HWND>(1), help.getSeqStr(2).c_str()));
		Assert::AreEqual(std::string("SetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_WritePrivateProfileString) {
	{ // エラーをスローできるか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::WritePrivateProfileString_true = [&](
			LPCSTR lpAppName,
			LPCSTR lpKeyName,
			LPCSTR lpString,
			LPCSTR lpFileName
		) -> BOOL {
			help << "WritePrivateProfileString\n";
			return FALSE;
		};
		try {
			help.setSeqBase();
			api::WritePrivateProfileString(
				help.getSeqStr(1).c_str(), 
				help.getSeqStr(2).c_str(), 
				help.getSeqStr(3).c_str(), 
				help.getSeqStr(4).c_str()
			);
			Assert::Fail();
		} catch (const api::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::WritePrivateProfileStringA", 5), err.getMessage());
		}
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		api::WritePrivateProfileString_true = [&](
			LPCSTR lpAppName,
			LPCSTR lpKeyName,
			LPCSTR lpString,
			LPCSTR lpFileName
		) -> BOOL {
			help << "WritePrivateProfileString\n";
			help << lpAppName << '\n';
			help << lpKeyName << '\n';
			help << lpString << '\n';
			help << lpFileName << '\n';
			return TRUE;
		};
		help.setSeqBase();
		Assert::IsTrue(api::WritePrivateProfileString(
			help.getSeqStr(1).c_str(), 
			help.getSeqStr(2).c_str(), 
			help.getSeqStr(3).c_str(), 
			help.getSeqStr(4).c_str()
		));
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_error) {
	{ // ヌルコンストラクタの結果は正しいか？
		api::error err;
		Assert::AreEqual(std::string(), err.getFunctionName());
		Assert::AreEqual(std::string(), err.getMessage());
		Assert::AreEqual(0, int(err.getNumber()));
	}
	{ // コンストラクタ(std::string)の結果は正しいか？
		test_helper_t help;
		api::GetLastError = [&]() -> DWORD {
			return help.getSeq<DWORD>();
		};
		api::error err(help.getSeqStr().c_str());
		Assert::AreEqual(std::string("1"), err.getFunctionName());
		Assert::AreEqual(std::string("1が失敗しました。(2)"), err.getMessage());
		Assert::AreEqual(2, int(err.getNumber()));
	}
	{ // コンストラクタ(std::string,DWORD)の結果は正しいか？
		test_helper_t help;
		help.setSeqBase();
		api::error err(help.getSeqStr(1).c_str(), help.getSeq<DWORD>(2));
		Assert::AreEqual(std::string("1"), err.getFunctionName());
		Assert::AreEqual(std::string("1が失敗しました。(2)"), err.getMessage());
		Assert::AreEqual(2, int(err.getNumber()));
	}
}

};

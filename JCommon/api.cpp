//// JEQ/共通処理ライブラリ/Win32 API関数インポート

//// インクルード

#include "api.hpp"
#include "jcommon.hpp"

#include <windows.h>

namespace jeq { namespace api {

//// クラスメンバ関数の定義

// Win32 API関数のエラーを構築する。
// エラーナンバーはGetLastErrorから取得する。
error_t::error_t(
	const std::string &func_name // 関数名。
) : error_t(func_name, api::GetLastError()) {}

// Win32 API関数のエラーを構築する。
// エラーメッセージを作成する。
error_t::error_t(
	const std::string &func_name, // 関数名。
	DWORD number                  // エラーナンバー。
) :	func_name(func_name), 
	number(number) 
{
	msg = string_printf("%sが失敗しました。(%d)", func_name.c_str(), number);
}

// 関数名を取得する。
const std::string & // 取得した関数名。
error_t::getFunctionName() const {
	return func_name;
}

// エラーナンバーを取得する。
DWORD // 取得したエラーナンバー。
error_t::getNumber() const {
	return number;
}

//// インポートの定義

DEF_IMP_WRAP_EASY(
	BeginPaint, 
	HDC, 
	HWND hWnd, 
	LPPAINTSTRUCT lpPaint
) {
	HDC ret = BeginPaint_true(hWnd, lpPaint);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	BitBlt, 
	BOOL, 
	HDC hdc, 
	int x, 
	int y, 
	int cx, 
	int cy, 
	HDC hdcSrc, 
	int x1, 
	int y1, 
	DWORD rop
) {
	BOOL ret = BitBlt_true(
		hdc, 
		x, 
		y, 
		cx, 
		cy, 
		hdcSrc, 
		x1, 
		y1, 
		rop
	);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(CallWindowProc);

DEF_IMP_WRAP_EASY(ClientToScreen, BOOL, HWND hWnd, LPPOINT lpPoint) {
	BOOL ret = ClientToScreen_true(hWnd, lpPoint);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	CreateCompatibleBitmap, 
	HBITMAP,
	HDC hdc, 
	int cx, 
	int cy
) {
	HBITMAP ret = CreateCompatibleBitmap_true(hdc, cx, cy);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(CreateCompatibleDC, HDC, HDC hdc) {
	HDC ret = CreateCompatibleDC_true(hdc);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	CreateFont, 
	HFONT, 
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
) {
	HFONT ret = CreateFont_true(
		cHeight, 
		cWidth, 
		cEscapement, 
		cOrientation, 
		cWeight, 
		bItalic, 
		bUnderline, 
		bStrikeOut, 
		iCharSet, 
		iOutPrecision, 
		iClipPrecision, 
		iQuality, 
		iPitchAndFamily, 
		pszFaceName
	);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(CreatePatternBrush, HBRUSH, HBITMAP hbm) {
	HBRUSH ret = CreatePatternBrush_true(hbm);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	CreatePen, 
	HPEN, 
	int iStyle, 
	int cWidth, 
	COLORREF color
) {
	HPEN ret = CreatePen_true(iStyle, cWidth, color);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	CreateWindowEx, 
	HWND, 
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
) {
	HWND ret = CreateWindowEx_true(
		dwExStyle, 
		pszClassName, 
		pszWindowName, 
		dwStyle, 
		x, 
		y, 
		nWidth, 
		nHeight, 
		hWndParent, 
		hMenu, 
		hInstance, 
		pParam
	);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(DefWindowProc);
DEF_IMP_EASY(DeleteDC);
DEF_IMP_EASY(DeleteObject);
DEF_IMP_EASY(DestroyWindow);
DEF_IMP_EASY(EndPaint);

DEF_IMP_WRAP_EASY(
	FillRect, 
	int, 
	HDC hDC, 
	const RECT *lprc, 
	HBRUSH hbr
) {
	int ret = FillRect_true(hDC, lprc, hbr);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(FreeLibrary);
DEF_IMP_EASY(GetActiveWindow);

DEF_IMP_WRAP_EASY(
	GetClientRect, 
	BOOL, 
	HWND hWnd, 
	LPRECT lpRect
) {
	BOOL ret = GetClientRect_true(hWnd, lpRect);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(GetCommandLine);

DEF_IMP_WRAP_EASY(GetCursorPos, BOOL, LPPOINT lpPoint) {
	BOOL ret = GetCursorPos_true(lpPoint);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(GetDC, HDC, HWND hWnd) {
	HDC ret = GetDC_true(hWnd);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(GetKeyState);
DEF_IMP_EASY(GetLastError);

DEF_IMP_WRAP_EASY(
	GetModuleFileName, 
	DWORD, 
	HMODULE hModule, 
	PTSTR pFilename, 
	DWORD nSize
) {
	DWORD ret = GetModuleFileName_true(hModule, pFilename, nSize);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(GetModuleHandle, HMODULE, LPCSTR lpModuleName) {
	HMODULE ret = GetModuleHandle_true(lpModuleName);
	if (!ret) throw api::error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(GetPrivateProfileSection);
DEF_IMP_EASY(GetPrivateProfileString);

DEF_IMP_WRAP_EASY(
	GetProcAddress,
	FARPROC,
	HMODULE hModule, 
	LPCSTR lpProcName
) {
	FARPROC ret = GetProcAddress_true(hModule, lpProcName);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(GetSystemMetrics, int, int nIndex) {
	int ret = GetSystemMetrics_true(nIndex);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	GetWindowLong, 
	LONG, 
	HWND hWnd, 
	int nIndex
) {
	LONG ret = GetWindowLong_true(hWnd, nIndex);
	if (!ret) {
		DWORD number = api::GetLastError();
		if (number) throw error_t(__FUNCTION__, number);
	}
	return ret;
}

DEF_IMP_WRAP_EASY(
	GetWindowRect,
	BOOL,
	HWND hWnd, 
	LPRECT lpRect
) {
	int ret = GetWindowRect_true(hWnd, lpRect);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	GetWindowText,
	int,
	HWND hWnd, 
	LPSTR lpString, 
	int nMaxCount
) {
	int ret = GetWindowText_true(hWnd, lpString, nMaxCount);
	if (!ret) {
		DWORD number = api::GetLastError();
		if (number)	throw error_t(__FUNCTION__, number);
	}
	return ret;
}

DEF_IMP_WRAP_EASY(
	InvalidateRect, 
	BOOL, 
	HWND hWnd, 
	const RECT *lpRect, 
	BOOL bErase
) {
	BOOL ret = InvalidateRect_true(hWnd, lpRect, bErase);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(IsIconic);
DEF_IMP_EASY(IsWindowVisible);

DEF_IMP_WRAP_EASY(
	LoadBitmap, 
	HBITMAP, 
	HINSTANCE hInstance, 
	LPCSTR lpBitmapName
) {
	HBITMAP ret = LoadBitmap_true(hInstance, lpBitmapName);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	LoadCursor, 
	HCURSOR, 
	HINSTANCE hInstance, 
	LPCSTR lpCursorName
) {
	HCURSOR ret = LoadCursor_true(hInstance, lpCursorName);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(LoadLibrary, HMODULE,	LPCSTR lpLibFileName) {
	HMODULE ret = LoadLibrary_true(lpLibFileName);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	MoveWindow, 
	BOOL, 
	HWND hWnd, 
	int X, 
	int Y, 
	int nWidth, 
	int nHeight, 
	BOOL bRepaint
) {
	BOOL ret = MoveWindow_true(hWnd, X, Y, nWidth, nHeight, bRepaint);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	Polyline, 
	BOOL, 
	HDC hdc, 
	const POINT *apt, 
	int cpt
) {
	BOOL ret = Polyline_true(hdc, apt, cpt);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(RegisterClass, ATOM, WNDCLASSA *lpWndClass) {
	ATOM ret = RegisterClass_true(lpWndClass);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(ReleaseDC);

DEF_IMP_WRAP_EASY(ScreenToClient, BOOL, HWND hWnd, LPPOINT lpPoint) {
	BOOL ret = ScreenToClient_true(hWnd, lpPoint);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SelectObject, 
	HGDIOBJ, 
	HDC hdc, 
	HGDIOBJ h
) {
	HGDIOBJ ret = SelectObject_true(hdc, h);
	if (!ret || ret == HGDI_ERROR) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(SendMessage);

DEF_IMP_WRAP_EASY(SetActiveWindow, HWND, HWND hWnd) {
	HWND ret = SetActiveWindow_true(hWnd);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetBkColor, 
	COLORREF, 
	HDC hdc, 
	COLORREF color
) {
	COLORREF ret = SetBkColor_true(hdc, color);
	if (ret == CLR_INVALID) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetBkMode, 
	int, 
	HDC hdc, 
	int mode
) {
	int ret = SetBkMode_true(hdc, mode);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetBrushOrgEx, 
	BOOL, 
	HDC hdc, 
	int x, 
	int y, 
	LPPOINT lppt
) {
	BOOL ret = SetBrushOrgEx_true(hdc, x, y, lppt);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetClassLong, 
	DWORD, 
	HWND hWnd, 
	int nIndex, 
	LONG dwNewLong
) {
	DWORD ret = SetClassLong_true(hWnd, nIndex, dwNewLong);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(SetCursor);

DEF_IMP_WRAP_EASY(SetFocus, HWND, HWND hWnd) {
	HWND ret = SetFocus_true(hWnd);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetTextColor, 
	COLORREF, 
	HDC hdc, 
	COLORREF color
) {
	COLORREF ret = SetTextColor_true(hdc, color);
	if (ret == CLR_INVALID) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetWindowLongPtr, 
	LONG_PTR, 
	HWND hWnd, 
	int nIndex, 
	LONG_PTR dwNewLong
) {
	LONG_PTR ret = SetWindowLongPtr_true(hWnd, nIndex, dwNewLong);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetWindowPos, 
	BOOL, 
	HWND hWnd, 
	HWND hWndInsertAfter, 
	int X, 
	int Y, 
	int cx, 
	int cy, 
	UINT uFlags
) {
	BOOL ret = SetWindowPos_true(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_WRAP_EASY(
	SetWindowText, 
	BOOL, 
	HWND hWnd, 
	LPCSTR lpString
) {
	BOOL ret = SetWindowText_true(hWnd, lpString);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

DEF_IMP_EASY(ShowWindow);
DEF_IMP_EASY(TranslateMessage);
DEF_IMP_EASY(UnregisterClass);

DEF_IMP_WRAP_EASY(
	WritePrivateProfileString,
	BOOL,
	LPCSTR lpAppName,
	LPCSTR lpKeyName,
	LPCSTR lpString,
	LPCSTR lpFileName
) {
	BOOL ret = WritePrivateProfileString_true(
		lpAppName,
		lpKeyName,
		lpString,
		lpFileName
	);
	if (!ret) throw error_t(__FUNCTION__);
	return ret;
}

}}

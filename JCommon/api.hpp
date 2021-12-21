//// JEQ/���ʏ������C�u����/Win32 API�֐��C���|�[�g

// ���̃v���W�F�N�g�Ŏg�p���Ă���Win32 API�֐����C���|�[�g����B
// Win32 API�֐��̓E�C���h�E�Ȃǂ̃��\�[�X�ɕK���A�N�Z�X����̂ŁA
// ���ׂĂ̊֐����C���|�[�g�̑ΏۂɂȂ�B

#ifndef JEQ_API_HPP
#define JEQ_API_HPP

//// �C���N���[�h

#include "error.hpp"
#include "import.hpp"

#include <windows.h>

#include <string>

namespace jeq { namespace api {

//// �^�̒�`

// Win32 API�֐��̃G���[��\���B
// �֐����ƃG���[�i���o�[��ێ�����B
class error_t : public jeq::error_t {
public:
	error_t() = default;
	explicit error_t(const std::string &func_name);
	error_t(const std::string &func_name, DWORD number);
	DWORD getNumber() const;
	const std::string &getFunctionName() const;
protected:
	std::string func_name; // �֐����B
	DWORD number = 0;      // �G���[�i���o�[�B
};

//// �C���|�[�g�̐錾

DECL_IMP_WRAP(BeginPaint, HDC, HWND hWnd, LPPAINTSTRUCT lpPaint);
DECL_IMP_WRAP(BitBlt, BOOL, HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);
DECL_IMP(CallWindowProc, LRESULT, WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
DECL_IMP_WRAP(ClientToScreen, BOOL, HWND hWnd, LPPOINT lpPoint);
DECL_IMP_WRAP(CreateCompatibleBitmap, HBITMAP, HDC hdc, int cx, int cy);
DECL_IMP_WRAP(CreateCompatibleDC, HDC, HDC hdc);
DECL_IMP_WRAP(CreateFont, HFONT, int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName);
DECL_IMP_WRAP(CreatePatternBrush, HBRUSH, HBITMAP hbm);
DECL_IMP_WRAP(CreatePen, HPEN, int iStyle, int cWidth, COLORREF color);
DECL_IMP_WRAP(CreateWindowEx, HWND, DWORD dwExStyle, PCTSTR pszClassName, PCTSTR pszWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, PVOID pParam);
DECL_IMP(DefWindowProc, LRESULT, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
DECL_IMP(DeleteDC, BOOL, HDC hdc);
DECL_IMP(DeleteObject, BOOL, HGDIOBJ ho);
DECL_IMP(DestroyWindow, BOOL, HWND hWnd);
DECL_IMP(EndPaint, BOOL, HWND hWnd, const PAINTSTRUCT *lpPaint);
DECL_IMP_WRAP(FillRect, int, HDC hDC, const RECT *lprc, HBRUSH hbr);
DECL_IMP(FreeLibrary, BOOL, HMODULE hLibModule);
DECL_IMP(GetActiveWindow, HWND);
DECL_IMP_WRAP(GetClientRect, BOOL, HWND hWnd, LPRECT lpRect);
DECL_IMP(GetCommandLine, LPSTR);
DECL_IMP_WRAP(GetCursorPos, BOOL, LPPOINT lpPoint);
DECL_IMP_WRAP(GetDC, HDC, HWND hWnd);
DECL_IMP(GetKeyState, SHORT, int nVirtKey);
DECL_IMP(GetLastError, DWORD);
DECL_IMP_WRAP(GetModuleFileName, DWORD, HMODULE hModule, PTSTR pFilename, DWORD nSize);
DECL_IMP_WRAP(GetModuleHandle, HMODULE, LPCSTR lpModuleName);
DECL_IMP(GetPrivateProfileSection, DWORD, LPCTSTR lpAppName, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName);
DECL_IMP(GetPrivateProfileString, DWORD, LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName);
DECL_IMP_WRAP(GetProcAddress, FARPROC, HMODULE hModule, LPCSTR lpProcName);
DECL_IMP_WRAP(GetSystemMetrics, int, int nIndex);
DECL_IMP_WRAP(GetWindowLong, LONG, HWND hWnd, int nIndex);
DECL_IMP_WRAP(GetWindowRect, BOOL, HWND hWnd, LPRECT lpRect);
DECL_IMP_WRAP(GetWindowText, int, HWND hWnd, LPSTR lpString, int nMaxCount);
DECL_IMP_WRAP(InvalidateRect, BOOL, HWND hWnd, const RECT *lpRect, BOOL bErase);
DECL_IMP(IsIconic, BOOL, HWND hWnd);
DECL_IMP(IsWindowVisible, BOOL, HWND hWnd);
DECL_IMP_WRAP(LoadBitmap, HBITMAP, HINSTANCE hInstance, LPCSTR lpBitmapName);
DECL_IMP_WRAP(LoadCursor, HCURSOR, HINSTANCE hInstance, LPCSTR lpCursorName);
DECL_IMP_WRAP(LoadLibrary, HMODULE, LPCSTR lpLibFileName);
DECL_IMP_WRAP(MoveWindow, BOOL, HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
DECL_IMP_WRAP(Polyline, BOOL, HDC hdc, const POINT *apt, int cpt);
DECL_IMP_WRAP(RegisterClass, ATOM, WNDCLASSA *lpWndClass);
DECL_IMP(ReleaseDC, int, HWND hWnd, HDC hDC);
DECL_IMP_WRAP(ScreenToClient, BOOL, HWND hWnd, LPPOINT lpPoint);
DECL_IMP_WRAP(SelectObject, HGDIOBJ, HDC hdc, HGDIOBJ h);
DECL_IMP(SendMessage, LRESULT, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
DECL_IMP_WRAP(SetActiveWindow, HWND, HWND hWnd);
DECL_IMP_WRAP(SetBkColor, COLORREF, HDC hdc, COLORREF color);
DECL_IMP_WRAP(SetBkMode, int, HDC hdc, int mode);
DECL_IMP_WRAP(SetBrushOrgEx, BOOL, HDC hdc, int x, int y, LPPOINT lppt);
DECL_IMP_WRAP(SetClassLong, DWORD, HWND hWnd, int nIndex, LONG dwNewLong);
DECL_IMP(SetCursor, HCURSOR, HCURSOR hCursor);
DECL_IMP_WRAP(SetFocus, HWND, HWND hWnd);
DECL_IMP_WRAP(SetTextColor, COLORREF, HDC hdc, COLORREF color);
DECL_IMP_WRAP(SetWindowLongPtr, LONG_PTR, HWND hWnd, int nIndex, LONG_PTR dwNewLong);
DECL_IMP_WRAP(SetWindowPos, BOOL, HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
DECL_IMP_WRAP(SetWindowText, BOOL, HWND hWnd, LPCSTR lpString);
DECL_IMP(ShowWindow, BOOL, HWND hWnd, int nCmdShow);
DECL_IMP(TranslateMessage, BOOL, const MSG *lpMsg);
DECL_IMP(UnregisterClass, BOOL, LPCSTR lpClassName, HINSTANCE hInstance);
DECL_IMP_WRAP(WritePrivateProfileString, BOOL, LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName);

}}

#endif // #ifndef JEQ_API_HPP

#include "jcommon.hpp"

#include <CppUnitTest.h>
#include <windows.h>

#include <chrono>
#include <climits>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <iomanip>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(jcommon_test) {

TEST_METHOD(test_clampByMin) {
	// 境界値で正しく動作できるか？
	{
		int value = 10;
		clampByMin(value, 10);
		Assert::AreEqual(10, value);
	}
	{
		int value = 9;
		clampByMin(value, 10);
		Assert::AreEqual(10, value);
	}
}

TEST_METHOD(test_cursor_getPos) {
	{ // 正しく動作できるか？
		test_helper_t help;
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			lpPoint->x = help.getSeq<LONG>();
			lpPoint->y = help.getSeq<LONG>();
			return TRUE;
		};
		POINT pos = cursor_getPos();
		Assert::AreEqual(LONG(1), pos.x);
		Assert::AreEqual(LONG(2), pos.y);
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_cursor_isOverWindow) {
	{ // ウインドウが非表示のときに偽を返せるか？
		test_helper_t help;
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return FALSE;
		};
		Assert::IsFalse(cursor_isOverWindow(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // ウインドウが最小化されているときに偽を返せるか？
		test_helper_t help;
		api::IsIconic = [&](HWND hWnd) -> BOOL {
			help << "IsIconic\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		api::IsWindowVisible = [&](HWND hWnd) -> BOOL {
			help << "IsWindowVisible\n";
			help << int(hWnd) << '\n';
			return TRUE;
		};
		Assert::IsFalse(cursor_isOverWindow(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // マウスカーソルがウインドウに乗っていないときに偽を返せるか？
		test_helper_t help;
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			*lpPoint = POINT{100, 99};
			return TRUE;
		};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{100, 100, 200, 200};
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
		Assert::IsFalse(cursor_isOverWindow(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // マウスポインタがウインドウに乗っているときに真を返せるか？
		test_helper_t help;
		api::GetCursorPos_true = [&](LPPOINT lpPoint) -> BOOL {
			help << "GetCursorPos\n";
			*lpPoint = POINT{199, 199};
			return TRUE;
		};
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			*lpRect = RECT{100, 100, 200, 200};
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
		Assert::IsTrue(cursor_isOverWindow(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("IsWindowVisible"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("IsIconic"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCursorPos"), help.getLine());
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_destringize) {
	// intに変換できるか？
	Assert::AreEqual(1 , destringize<int>("1" ,  0));
	Assert::AreEqual(-1, destringize<int>("-1",  0));
	Assert::AreEqual(0 , destringize<int>(""  ,  0));
	Assert::AreEqual(0 , destringize<int>("a" ,  0));
	Assert::AreEqual(-1, destringize<int>("a" , -1));

	// DWORDに変換できるか？
	Assert::AreEqual(0x00000001ul, destringize<DWORD>("1", 0x00000000ul, std::hex));
	Assert::AreEqual(0x0000000ful, destringize<DWORD>("f", 0x00000000ul, std::hex));
	Assert::AreEqual(0x00000000ul, destringize<DWORD>("" , 0x00000000ul, std::hex));
	Assert::AreEqual(0x00000000ul, destringize<DWORD>("g", 0x00000000ul, std::hex));
	Assert::AreEqual(0xfffffffful, destringize<DWORD>("g", 0xfffffffful, std::hex));

	// boolに変換できるか？
	Assert::IsFalse(destringize<bool>("0", false));
	Assert::IsTrue (destringize<bool>("1", false));
	Assert::IsFalse(destringize<bool>("" , false));
	Assert::IsFalse(destringize<bool>("a", false));
	Assert::IsTrue (destringize<bool>("a", true ));
}

TEST_METHOD(test_getCtrlAlphabetKey) {
	// 境界値で正しく動作できるか？
	Assert::AreEqual(BYTE(0x01), getCtrlAlphabetKey('A'));
	Assert::AreEqual(BYTE(0x1a), getCtrlAlphabetKey('Z'));
}

TEST_METHOD(test_ignore_case_compare) {
	// 正しく動作できるか？
	Assert::IsFalse(ignore_case_compare_t()("hoge", "hoge"));
	Assert::IsFalse(ignore_case_compare_t()("hoge", "HOGE"));
	Assert::IsFalse(ignore_case_compare_t()("HOGE", "hoge"));
	Assert::IsFalse(ignore_case_compare_t()("hoge", "fuga"));
	Assert::IsTrue (ignore_case_compare_t()("fuga", "hoge"));
}

TEST_METHOD(test_indirect_cast) {
	// 正しく動作できるか？
	Assert::AreEqual(0x34, int(indirect_cast<BYTE>(WORD(0x1234))));
	Assert::AreEqual(0x5678, int(indirect_cast<WORD>(DWORD(0x12345678))));
}

TEST_METHOD(test_ini_deleteKey) {
	{ // 正しく動作できるか？
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
			help << int(lpString) << '\n';
			help << lpFileName << '\n';
			return TRUE;
		};
		help.setSeqBase();
		ini_deleteKey(help.getSeqStr(1), help.getSeqStr(2), help.getSeqStr(3));
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ini_deleteSection) {
	{ // 正しく動作できるか？
		test_helper_t help;
		api::WritePrivateProfileString_true = [&](
			LPCSTR lpAppName,
			LPCSTR lpKeyName,
			LPCSTR lpString,
			LPCSTR lpFileName
		) -> BOOL {
			help << "WritePrivateProfileString\n";
			help << lpAppName << '\n';
			help << int(lpKeyName) << '\n';
			help << int(lpString) << '\n';
			help << lpFileName << '\n';
			return TRUE;
		};
		help.setSeqBase();
		ini_deleteSection(help.getSeqStr(1), help.getSeqStr(2));
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ini_getKeyValue) {
	{ // 既定値を取得できるか？
		test_helper_t help;
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
			return 0;
		};
		help.setSeqBase();
		Assert::AreEqual(std::string("4"), ini_getKeyValue(help.getSeqStr(1), help.getSeqStr(2), help.getSeqStr(3), help.getSeqStr(4)));
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // デフォルトではない値を取得できるか？
		test_helper_t help;
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
		help.setSeqBase();
		Assert::AreEqual(std::string("5"), ini_getKeyValue(help.getSeqStr(1), help.getSeqStr(2), help.getSeqStr(3), help.getSeqStr(4)));
		Assert::AreEqual(std::string("GetPrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ini_getSection) {
	{ // 空っぽのセクションを取得できるか？
		test_helper_t help;
		api::GetPrivateProfileSection = [&](
			LPCSTR lpAppName,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileSection\n";
			help << lpAppName << '\n';
			help << lpFileName << '\n';
			char *p = lpReturnedString;
			*p++ = '\0';
			*p++ = '\0';
			return p - lpReturnedString - 2;
		};
		help.setSeqBase();
		auto keys = ini_getKeys(help.getSeqStr(1), help.getSeqStr(2));
		Assert::IsTrue(keys.empty());
		Assert::AreEqual(std::string("GetPrivateProfileSection"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 空っぽではないセクションを取得できるか？
		test_helper_t help;
		api::GetPrivateProfileSection = [&](
			LPCSTR lpAppName,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileSection\n";
			help << lpAppName << '\n';
			help << lpFileName << '\n';
			char *p = lpReturnedString;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "="); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p) + 1;
			*p++ = '\0';
			return p - lpReturnedString - 2;
		};
		help.setSeqBase();
		auto keys = ini_getKeys(help.getSeqStr(1), help.getSeqStr(2));
		Assert::AreEqual(1, int(keys.size()));
		Assert::AreEqual(std::string("3"), keys[0].name);
		Assert::AreEqual(std::string("4"), keys[0].value);
		Assert::AreEqual(std::string("GetPrivateProfileSection"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ini_loadSection) {
	{ // 空っぽのセクションをロードできるか？
		test_helper_t help;
		api::GetPrivateProfileSection = [&](
			LPCSTR lpAppName,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileSection\n";
			help << lpAppName << '\n';
			help << lpFileName << '\n';
			char *p = lpReturnedString;
			*p++ = '\0';
			*p++ = '\0';
			return p - lpReturnedString - 2;
		};
		help.setSeqBase();
		std::string section = ini_loadSection(help.getSeqStr(1), help.getSeqStr(2));
		{
			int i = 0;
			Assert::AreEqual('\0', section[i++]);
			Assert::AreEqual('\0', section[i++]);
			Assert::AreEqual(i, int(section.length()));
		}
		Assert::AreEqual(std::string("GetPrivateProfileSection"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 空っぽではないセクションをロードできるか？
		test_helper_t help;
		api::GetPrivateProfileSection = [&](
			LPCSTR lpAppName,
			LPSTR lpReturnedString,
			DWORD nSize,
			LPCSTR lpFileName
		) -> DWORD {
			help << "GetPrivateProfileSection\n";
			help << lpAppName << '\n';
			help << lpFileName << '\n';
			char *p = lpReturnedString;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "="); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p) + 1;
			*p++ = '\0';
			return p - lpReturnedString - 2;
		};
		help.setSeqBase();
		std::string section = ini_loadSection(help.getSeqStr(1), help.getSeqStr(2));
		{
			int i = 0;
			std::string str;
			str = "3"; Assert::AreEqual(str, section.substr(i, str.length())); i += str.length();
			str = "="; Assert::AreEqual(str, section.substr(i, str.length())); i += str.length();
			str = "4"; Assert::AreEqual(str, section.substr(i, str.length())); i += str.length();
			Assert::AreEqual('\0', section[i++]);
			Assert::AreEqual('\0', section[i++]);
			Assert::AreEqual(i, int(section.length()));
		}
		Assert::AreEqual(std::string("GetPrivateProfileSection"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ini_parseSection) {
	{ // 正しく動作できるか？
		test_helper_t help;
		std::string section;
		section += help.getSeqStr();
		section += "=";
		section += help.getSeqStr();
		section.resize(section.size() + 1, '\0');
		section += help.getSeqStr();
		section += "=";
		section += help.getSeqStr();
		section.resize(section.size() + 2, '\0');
		auto keys = ini_parseSection(section);
		Assert::AreEqual(2, int(keys.size()));
		Assert::AreEqual(std::string("1"), keys[0].name);
		Assert::AreEqual(std::string("2"), keys[0].value);
		Assert::AreEqual(std::string("3"), keys[1].name);
		Assert::AreEqual(std::string("4"), keys[1].value);
	}
}

TEST_METHOD(test_ini_setKeyValue) {
	{ // 正しく動作できるか？
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
		ini_setKeyValue(help.getSeqStr(1), help.getSeqStr(2), help.getSeqStr(3), help.getSeqStr(4));
		Assert::AreEqual(std::string("WritePrivateProfileString"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_longestRunOfOnes) {
	// 正しく動作できるか？
	Assert::AreEqual( 0, int(longestRunOfOnes<BYTE>(0x00)));
	Assert::AreEqual( 0, int(longestRunOfOnes<BYTE>(0x40)));
	Assert::AreEqual( 1, int(longestRunOfOnes<BYTE>(0x80)));
	Assert::AreEqual( 1, int(longestRunOfOnes<BYTE>(0xa0)));
	Assert::AreEqual( 2, int(longestRunOfOnes<BYTE>(0xc0)));
	Assert::AreEqual( 2, int(longestRunOfOnes<BYTE>(0xd0)));
	Assert::AreEqual( 7, int(longestRunOfOnes<BYTE>(0xfe)));
	Assert::AreEqual( 8, int(longestRunOfOnes<BYTE>(0xff)));
	Assert::AreEqual( 0, int(longestRunOfOnes<WORD>(0x7fff)));
	Assert::AreEqual( 1, int(longestRunOfOnes<WORD>(0xbfff)));
	Assert::AreEqual(15, int(longestRunOfOnes<WORD>(0xfffe)));
	Assert::AreEqual(16, int(longestRunOfOnes<WORD>(0xffff)));
}

TEST_METHOD(test_map_find) {
	{ // 正しく動作できるか？
		std::map<int,std::string> strs = {
			{1, "Hoge"},
			{3, "Fuga"},
		};
		Assert::AreEqual(std::string("Hoge"), map_find(strs, 1));
		Assert::AreEqual(std::string(), map_find(strs, 2));
		Assert::AreEqual(std::string("Fuga"), map_find(strs, 3, "Foo"));
		Assert::AreEqual(std::string("Bar"), map_find(strs, 4, "Bar"));
	}
}

TEST_METHOD(test_module_getPath) {
	{ // 正しく動作できるか？
		test_helper_t help;
		api::GetModuleFileName_true = [&](
			HMODULE hModule,
			PTSTR pFilename,
			DWORD nSize
		) -> DWORD {
			help << "GetModuleFileName\n";
			help << int(hModule) << '\n';
			std::strcpy(pFilename, help.getSeqStr().c_str());
			return std::strlen(pFilename);
		};
		Assert::AreEqual(std::string("2"), module_getPath(help.getSeq<HMODULE>()));
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_namedValuesToMap) {
	{ // 正しく動作できるか？
		std::vector<named_value_t> named_values = {
			{"hoge", "foo"},
			{"fuga", "bar"},
			{"FUGA", "baz"},
		};
		auto map = namedValuesToMap(named_values);
		Assert::AreEqual(2, int(map.size()));
		Assert::AreEqual(std::string("foo"), map.at("hoge"));
		Assert::AreEqual(std::string("bar"), map.at("FUGA"));
	}
}

TEST_METHOD(test_parseArguments) {
	{ // 値なし引数のときに正しく動作できるか？
		auto args = parseArguments("hoge");
		Assert::AreEqual(1, int(args.size()));
		Assert::AreEqual(std::string("hoge"), args[0].name);
		Assert::AreEqual(std::string(), args[0].value);
	}
	{ // 値あり引数のときに正しく動作できるか？
		auto args = parseArguments("hoge=foo");
		Assert::AreEqual(1, int(args.size()));
		Assert::AreEqual(std::string("hoge"), args[0].name);
		Assert::AreEqual(std::string("foo"), args[0].value);
	}
	{ // 引数が2個のときに正しく動作できるか？
		auto args = parseArguments("hoge=foo fuga=bar");
		Assert::AreEqual(2, int(args.size()));
		Assert::AreEqual(std::string("hoge"), args[0].name);
		Assert::AreEqual(std::string("foo"), args[0].value);
		Assert::AreEqual(std::string("fuga"), args[1].name);
		Assert::AreEqual(std::string("bar"), args[1].value);
	}
	{ // スペースを含む引数のときに正しく動作できるか？
		auto args = parseArguments("\"hoge hoge\"=\"foo foo\"");
		Assert::AreEqual(1, int(args.size()));
		Assert::AreEqual(std::string("hoge hoge"), args[0].name);
		Assert::AreEqual(std::string("foo foo"), args[0].value);
	}
	{ // 引用符を含む引数のときに正しく動作できるか？
		auto args = parseArguments("\"ho\"\"ge\"=\"f\"\"oo\"");
		Assert::AreEqual(1, int(args.size()));
		Assert::AreEqual(std::string("ho\"ge"), args[0].name);
		Assert::AreEqual(std::string("f\"oo"), args[0].value);
	}
	{ // 引用符を含めそこねた引数のときに正しく動作できるか？
		auto args = parseArguments("\"ho\"ge\"=\"f\"oo\"");
		Assert::AreEqual(1, int(args.size()));
		Assert::AreEqual(std::string("hoge=foo"), args[0].name);
		Assert::AreEqual(std::string(), args[0].value);
	}
}

TEST_METHOD(test_point_clientToScreen) {
	{ // 正しく動作できるか？
		test_helper_t help;
		api::ClientToScreen_true = [&](HWND hWnd, LPPOINT lpPoint) -> BOOL {
			help << "ClientToScreen\n";
			help << int(hWnd) << '\n';
			help << lpPoint->x << '\n';
			help << lpPoint->y << '\n';
			lpPoint->x = help.getSeq<LONG>();
			lpPoint->y = help.getSeq<LONG>();
			return TRUE;
		};
		help.setSeqBase();
		POINT pos = point_clientToScreen(POINT{help.getSeq<LONG>(1), help.getSeq<LONG>(2)}, help.getSeq<HWND>(3));
		Assert::AreEqual(4, int(pos.x));
		Assert::AreEqual(5, int(pos.y));
		Assert::AreEqual(std::string("ClientToScreen"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_point_screenToClient) {
	{ // 正しく動作できるか？
		test_helper_t help;
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
		POINT pos = point_screenToClient(POINT{help.getSeq<LONG>(1), help.getSeq<LONG>(2)}, help.getSeq<HWND>(3));
		Assert::AreEqual(4, int(pos.x));
		Assert::AreEqual(5, int(pos.y));
		Assert::AreEqual(std::string("ScreenToClient"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_putLog) {
	{ // 正しく動作できるか？
		test_helper_t help;
		std::tm tm_;
		tm_.tm_isdst = -1;
		std::chrono::system_clock::time_point now_;
		std_::chrono_system_clock_now = [&now_]() -> 
			std::chrono::system_clock::time_point 
		{
			return now_;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		tm_.tm_year = 100 + help.getSeq();
		tm_.tm_mon = help.getSeq() - 1;
		tm_.tm_mday = help.getSeq();
		tm_.tm_hour = help.getSeq();
		tm_.tm_min = help.getSeq();
		tm_.tm_sec = help.getSeq();
		now_ = std::chrono::system_clock::from_time_t(std::mktime(&tm_));
		now_ += std::chrono::milliseconds(help.getSeq());
		help.setSeqBase();
		putLog(help.getSeq<std::ostream*>(1), help.getSeqStr(2));
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("8"), help.getLine());
		Assert::AreEqual(std::string("2001/02/03 04:05:06.007 9"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_rect_areOverlapped) {
	//     0123456     0123456
	// 1: 0+-+     2: 0
	//    1| |        1
	//    2+-+        2
	//    3           3   +-+
	//    4           4   | |
	//    5           5   +-+
	//    6           6
	Assert::IsFalse(rect_areOverlapped(
		RECT{0, 0, 3, 3},
		RECT{3, 3, 6, 6}
	));
	//     0123456     0123456
	// 1: 0        2: 0+-+
	//    1           1| |
	//    2           2+-+
	//    3   +-+     3
	//    4   | |     4
	//    5   +-+     5
	//    6           6
	Assert::IsFalse(rect_areOverlapped(
		RECT{3, 3, 6, 6},
		RECT{0, 0, 3, 3}
	));
	//     0123456     0123456
	// 1: 0 +-+    2: 0
	//    1 | |       1
	//    2 +-+       2
	//    3           3   +-+
	//    4           4   | |
	//    5           5   +-+
	//    6           6
	Assert::IsFalse(rect_areOverlapped(
		RECT{1, 0, 4, 3},
		RECT{3, 3, 6, 6}
	));
	//     0123456     0123456
	// 1: 0		   2: 0 +-+
	//    1		      1 | |
	//    2		      2 +-+
	//    3   +-+     3
	//    4   | |     4
	//    5   +-+     5
	//    6		      6
	Assert::IsFalse(rect_areOverlapped(
		RECT{3, 3, 6, 6},
		RECT{1, 0, 4, 3}
	));
	//     0123456     0123456
	// 1: 0        2: 0
	//    1+-+        1
	//    2| |        2
	//    3+-+        3   +-+
	//    4           4   | |
	//    5           5   +-+
	//    6           6
	Assert::IsFalse(rect_areOverlapped(
		RECT{0, 1, 3, 4},
		RECT{3, 3, 6, 6}
	));
	//     0123456     0123456
	// 2: 0		   1: 0
	//    1		      1+-+
	//    2		      2| |
	//    3   +-+     3+-+
	//    4   | |     4
	//    5   +-+     5
	//    6		      6
	Assert::IsFalse(rect_areOverlapped(
		RECT{3, 3, 6, 6},
		RECT{0, 1, 3, 4}
	));
	//     0123456     0123456
	// 1: 0        2: 0
	//    1 +-+       1
	//    2 | |       2
	//    3	+-+       3   +-+
	//    4           4   | |
	//    5           5   +-+
	//    6           6
	Assert::IsTrue(rect_areOverlapped(
		RECT{1, 1, 4, 4},
		RECT{3, 3, 6, 6}
	));
	//     0123456     0123456
	// 1: 0        2: 0
	//    1           1 +-+	  
	//    2           2 | |	  
	//    3   +-+     3	+-+	  
	//    4   | |     4
	//    5   +-+     5
	//    6           6
	Assert::IsTrue(rect_areOverlapped(
		RECT{3, 3, 6, 6},
		RECT{1, 1, 4, 4}
	));
	//     0123456     0123456
	// 1: 0        2: 0
	//    1 +---+     1
	//    2 |   |     2  +-+
	//    3	|	|     3  | |
	//    4	|	|     4  +-+
	//    5	+---+     5
	//    6           6
	Assert::IsTrue(rect_areOverlapped(
		RECT{1, 1, 6, 6},
		RECT{2, 2, 5, 5}
	));
	//     0123456     0123456
	// 2: 0        1: 0
	//	  1           1 +---+
	//	  2  +-+      2 |   |
	//    3  | |      3	|	|
	//    4  +-+      4	|	|
	//	  5           5	+---+
	//    6           6
	Assert::IsTrue(rect_areOverlapped(
		RECT{2, 2, 5, 5},
		RECT{1, 1, 6, 6}
	));
}

TEST_METHOD(test_reverseClear) {
	{ // 正しく動作できるか？
		test_helper_t help;
		std::vector<scope_exit_t> exits;
		exits.emplace_back([&] {
			help << 1 << '\n';
		});
		exits.emplace_back([&] {
			help << 2 << '\n';
		});
		reverseClear(exits);
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_scope_exit) {
	{ // 処理を呼び出せるか？
		test_helper_t help;
		{
			scope_exit_t exit([&] {
				help << "procedure";
			});
		}
		Assert::AreEqual(std::string("procedure"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 処理をリセットできるか？
		test_helper_t help;
		{
			scope_exit_t exit([&] {
				help << "procedure";
			});
			exit.procedure() = nullptr;
		}
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_section_areOverlapped) {
	//    0123456
	// 1: +-+
	// 2:    +-+
	Assert::IsFalse(section_areOverlapped(0, 3, 3, 6));
	//    0123456
	// 1:    +-+
	// 2: +-+
	Assert::IsFalse(section_areOverlapped(3, 6, 0, 3));
	//    0123456
	// 1:  +-+
	// 2:    +-+
	Assert::IsTrue (section_areOverlapped(1, 4, 3, 6));
	//    0123456
	// 1:    +-+
	// 2:  +-+
	Assert::IsTrue (section_areOverlapped(3, 6, 1, 4));
	//    0123456
	// 1:  +---+
	// 2:   +-+
	Assert::IsTrue (section_areOverlapped(1, 6, 2, 5));
	//    0123456
	// 1:   +-+
	// 2:  +---+
	Assert::IsTrue (section_areOverlapped(2, 5, 1, 6));
}

TEST_METHOD(test_sjis_isLead) {
	// 境界値で正しく動作できるか？
	Assert::IsFalse(sjis_isLead(char(0x80)));
	Assert::IsTrue (sjis_isLead(char(0x81)));
	Assert::IsTrue (sjis_isLead(char(0x9f)));
	Assert::IsFalse(sjis_isLead(char(0xa0)));
	Assert::IsFalse(sjis_isLead(char(0xdf)));
	Assert::IsTrue (sjis_isLead(char(0xe0)));
	Assert::IsTrue (sjis_isLead(char(0xfc)));
	Assert::IsFalse(sjis_isLead(char(0xfd)));
}

TEST_METHOD(test_sjis_replace) {
	// 正しく動作できるか？
	Assert::AreEqual(std::string("AちAちbろAちbろcは"), sjis_replace("aいaいbろaいbろcは", "aい", "Aち"));
	Assert::AreEqual(std::string("aいAちBりAちBりcは"), sjis_replace("aいaいbろaいbろcは", "aいbろ", "AちBり"));
	Assert::AreEqual(std::string("aいaいbろAちBりCぬ"), sjis_replace("aいaいbろaいbろcは", "aいbろcは", "AちBりCぬ"));

}

TEST_METHOD(test_sjis_split) {
	{ // 半角の文字列を分割できるか？空文字列は許す。
		std::string str = "a,bb,,ccc,,,";
		auto toks = sjis_split(str, ',');
		Assert::AreEqual(7, int(toks.size()));
		Assert::AreEqual(std::string("a"),   toks[0]);
		Assert::AreEqual(std::string("bb"),  toks[1]);
		Assert::AreEqual(std::string(""),    toks[2]);
		Assert::AreEqual(std::string("ccc"), toks[3]);
		Assert::AreEqual(std::string(""),    toks[4]);
		Assert::AreEqual(std::string(""),    toks[5]);
		Assert::AreEqual(std::string(""),    toks[6]);
	}
	{ // 半角の文字列を分割できるか？空文字列は許さない。
		std::string str = "a,bb,,ccc,,,";
		auto toks = sjis_split(str, ',', false);
		Assert::AreEqual(3, int(toks.size()));
		Assert::AreEqual(std::string("a"),   toks[0]);
		Assert::AreEqual(std::string("bb"),  toks[1]);
		Assert::AreEqual(std::string("ccc"), toks[2]);
	}
	{ // 全角の文字列を分割できるか？空文字列は許す。
		std::string str = "い,ろろ,,ははは,,,";
		auto toks = sjis_split(str, ',');
		Assert::AreEqual(7, int(toks.size()));
		Assert::AreEqual(std::string("い"),     toks[0]);
		Assert::AreEqual(std::string("ろろ"),   toks[1]);
		Assert::AreEqual(std::string(""),       toks[2]);
		Assert::AreEqual(std::string("ははは"), toks[3]);
		Assert::AreEqual(std::string(""),       toks[4]);
		Assert::AreEqual(std::string(""),       toks[5]);
		Assert::AreEqual(std::string(""),       toks[6]);
	}
	{ // 全角の文字列を分割できるか？空文字列は許さない。
		std::string str = "い,ろろ,,ははは,,,";
		auto toks = sjis_split(str, ',', false);
		Assert::AreEqual(std::size_t(3), toks.size());
		Assert::AreEqual(std::string("い"),     toks[0]);
		Assert::AreEqual(std::string("ろろ"),   toks[1]);
		Assert::AreEqual(std::string("ははは"), toks[2]);
	}
}

TEST_METHOD(test_sjis_toLower) {
	// 正しく動作できるか？
	Assert::AreEqual(std::string("hいoろgはe"), sjis_toLower("HいoろGはe"));
	Assert::AreEqual(std::string("ちfりuぬgるaを"), sjis_toLower("ちfりUぬgるAを"));
}

TEST_METHOD(test_string_printf) {
	// 正しく動作できるか？
	Assert::AreEqual(std::string("0012 0.34 Hoge"), string_printf("%04d %.2f %s", 12, 0.345, "Hoge"));
}

TEST_METHOD(test_string_replace) {
	// 正しく動作できるか？
	Assert::AreEqual(std::string(u8"AちAちbろAちbろcは"), string_replace(u8"aいaいbろaいbろcは", u8"aい", u8"Aち"));
	Assert::AreEqual(std::string(u8"aいAちBりAちBりcは"), string_replace(u8"aいaいbろaいbろcは", u8"aいbろ", u8"AちBり"));
	Assert::AreEqual(std::string(u8"aいaいbろAちBりCぬ"), string_replace(u8"aいaいbろaいbろcは", u8"aいbろcは", u8"AちBりCぬ"));
}

TEST_METHOD(test_string_sjisToUtf16) {
	// 半角の文字列を変換できるか？
	Assert::AreEqual(std::wstring(L"Hoge"), string_sjisToUtf16("Hoge"));
	// 全角の文字列を変換できるか？
	Assert::AreEqual(std::wstring(L"ホゲ"), string_sjisToUtf16("ホゲ"));
}

TEST_METHOD(test_string_sjisToUtf8) {
	// 半角の文字列を変換できるか？
	Assert::AreEqual(std::string(u8"Hoge"), string_sjisToUtf8("Hoge"));
	// 全角の文字列を変換できるか？
	Assert::AreEqual(std::string(u8"ホゲ"), string_sjisToUtf8("ホゲ"));
}

TEST_METHOD(test_string_utf8ToSJIS) {
	// 半角の文字列を変換できるか？
	Assert::AreEqual(std::string("Hoge"), string_utf8ToSJIS(u8"Hoge"));
	// 全角の文字列を変換できるか？
	Assert::AreEqual(std::string("ホゲ"), string_utf8ToSJIS(u8"ホゲ"));
}

TEST_METHOD(test_string_utf8ToUtf16) {
	// 半角の文字列を変換できるか？
	Assert::AreEqual(std::wstring(L"Hoge"), string_utf8ToUtf16(u8"Hoge"));
	// 全角の文字列を変換できるか？
	Assert::AreEqual(std::wstring(L"ホゲ"), string_utf8ToUtf16(u8"ホゲ"));
}

TEST_METHOD(test_stringize) {
	{ // intを変換できるか？
		Assert::AreEqual(std::string("1"), stringize(1));
		Assert::AreEqual(std::string("-1"), stringize(-1));
		Assert::AreEqual(std::string("12345678"), stringize(0x12345678ul, std::hex));
		Assert::AreEqual(std::string("0X00FEDCBA98"), stringize(
			0xfedcba98ul, 
			std::hex, 
			std::showbase, 
			std::internal, 
			std::uppercase, 
			std::setw(12), 
			std::setfill('0')
		));
	}
	{ // boolを変換できるか？
		Assert::AreEqual(std::string("0"), stringize(false));
		Assert::AreEqual(std::string("1"), stringize(true));
		Assert::AreEqual(std::string("false"), stringize(false, std::boolalpha));
		Assert::AreEqual(std::string("true"), stringize(true, std::boolalpha));
	}
}

TEST_METHOD(test_test_helper) {
	{ // ヌクコンストラクタは正しく動作できるか？
		test_helper_t help;
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // コンストラクタ(std::string)は正しく動作できるか？
		test_helper_t help("Hoge\nFuga\n");
		Assert::AreEqual(std::string("Hoge"), help.getLine());
		Assert::AreEqual(std::string("Fuga"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // operator<<は正しく動作できるか？
		test_helper_t help;
		help << "Hoge\n";
		help << "Fuga\n";
		Assert::AreEqual(std::string("Hoge"), help.getLine());
		Assert::AreEqual(std::string("Fuga"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // getSeqとgetSeqPtrとgetSeqStrとsetSeqBaseは正しく動作できるか？
		test_helper_t help;
		Assert::AreEqual(1, help.getSeq());
		Assert::AreEqual(2, int(help.getSeqPtr<int>().get()));
		Assert::AreEqual(std::string("3"), help.getSeqStr());
		help.setSeqBase();
		Assert::AreEqual(13, help.getSeq(10));
		Assert::AreEqual(14, int(help.getSeqPtr<int>(11).get()));
		Assert::AreEqual(std::string("15"), help.getSeqStr(12));
	}
}

TEST_METHOD(test_test_trampoline) {
	using test_X_t = test_trampoline_t<int,int>;
	{ // trampolineは正しく呼び出せるか？
		test_helper_t help;
		test_X_t::target = [&](int num) -> int {
			help << "X\n";
			help << num << '\n';
			return help.getSeq();
		};
		Assert::AreEqual(2, test_X_t::trampoline(help.getSeq()));
		Assert::AreEqual(std::string("X"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // trampoline_apiは正しく呼び出せるか？
		test_helper_t help;
		test_X_t::target = [&](int num) -> int {
			help << "X\n";
			help << num << '\n';
			return help.getSeq();
		};
		Assert::AreEqual(2, test_X_t::trampoline_api(help.getSeq()));
		Assert::AreEqual(std::string("X"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // trampoline_memberは正しく呼び出せるか？
		test_helper_t help;
		test_X_t::target = [&](int num) -> int {
			help << "X\n";
			help << num << '\n';
			return help.getSeq();
		};
		test_X_t x;
		Assert::AreEqual(2, x.trampoline_member(help.getSeq()));
		Assert::AreEqual(std::string("X"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_thread_pool_ask) {
	BEGIN_DEF_SPY_CLASS(thread_pool_t);
	BEGIN_DEF_SPY_CLASS_N(thread_pool_t, work_t);
	{ // タスクを依頼できるか？
		test_helper_t help;
		thread_pool_t thread_pool_;
		spy_thread_pool_t *thread_pool = (spy_thread_pool_t*)(&thread_pool_);
		thread_pool_t::task_t task = [&] {
			help << "task\n";
		};
		std_::condition_variable_notify_all_true = [&](
			std::condition_variable *cv
		) {
			help << "condition_variable_notify_all\n";
			help << int(cv) << '\n';
		};
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		std_::unique_lock_unlock_true = [&](
			std::unique_lock<std::mutex> *lock
		) {
			help << "unique_lock_unlock\n";
			help << int(lock) << '\n';
		};
		auto work_ = thread_pool->ask(task);
		spy_work_t *work = (spy_work_t*)(&work_);
		Assert::AreEqual(1, int(thread_pool->data->tasks.size()));
		Assert::AreEqual(1, int(work->data->tasks_count));
		Assert::AreEqual(0, int(work->data->tasks_done));
		thread_pool->data->tasks.front()();
		Assert::AreEqual(1, int(work->data->tasks_done));
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->mutex)), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->workers_cv)), help.getLine());
		Assert::AreEqual(std::string("task"), help.getLine());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&work->data->mutex)), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(int(&work->data->work_cv)), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // タスクのコンテナを依頼できるか？
		test_helper_t help;
		thread_pool_t thread_pool_;
		spy_thread_pool_t *thread_pool = (spy_thread_pool_t*)(&thread_pool_);
		std::vector<thread_pool_t::task_t> tasks = {[&] {
			help << "task1\n";
		}, [&] {
			help << "task2\n";
		}};
		std_::condition_variable_notify_all_true = [&](
			std::condition_variable *cv
		) {
			help << "condition_variable_notify_all\n";
			help << int(cv) << '\n';
		};
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		std_::unique_lock_unlock_true = [&](
			std::unique_lock<std::mutex> *lock
		) {
			help << "unique_lock_unlock\n";
			help << int(lock) << '\n';
		};
		auto work_ = thread_pool->ask(tasks);
		spy_work_t *work = (spy_work_t*)(&work_);
		Assert::AreEqual(2, int(thread_pool->data->tasks.size()));
		Assert::AreEqual(2, int(work->data->tasks_count));
		Assert::AreEqual(0, int(work->data->tasks_done));
		thread_pool->data->tasks.front()();
		Assert::AreEqual(1, int(work->data->tasks_done));
		thread_pool->data->tasks.pop();
		thread_pool->data->tasks.front()();
		Assert::AreEqual(2, int(work->data->tasks_done));
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->mutex)), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->workers_cv)), help.getLine());
		Assert::AreEqual(std::string("task1"), help.getLine());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&work->data->mutex)), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(int(&work->data->work_cv)), help.getLine());
		Assert::AreEqual(std::string("task2"), help.getLine());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&work->data->mutex)), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(int(&work->data->work_cv)), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(work_t);
	END_DEF_SPY_CLASS(thread_pool_t);
}

TEST_METHOD(test_thread_pool_launch) {
	BEGIN_DEF_SPY_CLASS(thread_pool_t);
	{ // ワーカースレッドの数を省略したときに正しく動作できるか？
		test_helper_t help;
		thread_pool_t thread_pool_;
		spy_thread_pool_t *thread_pool = (spy_thread_pool_t*)(&thread_pool_);
		std_::condition_variable_notify_all_true = [&](
			std::condition_variable *cv
		) {
			help << "condition_variable_notify_all\n";
			help << int(cv) << '\n';
		};
		std_::make_thread_true = [&](
			const std::function<void()> &proc
		) -> std::shared_ptr<std::thread> {
			help << "make_thread\n";
			return help.getSeqPtr<std::thread>();
		};
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		std_::thread_join_true = [&](std::thread *thread) {
			help << "thread_join\n";
			help << int(thread) << '\n';
		};
		std_::unique_lock_unlock_true = [&](
			std::unique_lock<std::mutex> *lock
		) {
			help << "unique_lock_unlock\n";
			help << int(lock) << '\n';
		};
		thread_pool->launch();
		Assert::AreEqual(HARDWARE_CONCURRENCY, thread_pool->data->workers.size());
		for (std::size_t i = 0; i < HARDWARE_CONCURRENCY; ++i)
			Assert::AreEqual(int(i + 1), int(thread_pool->data->workers[i].get()));
		int mutex_address = int(&thread_pool->data->mutex);
		int workers_cv_address = int(&thread_pool->data->workers_cv);
		*thread_pool = {};
		for (std::size_t i = 0; i < HARDWARE_CONCURRENCY; ++i)
			Assert::AreEqual(std::string("make_thread"), help.getLine());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(mutex_address), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(workers_cv_address), help.getLine());
		for (std::size_t i = 0; i < HARDWARE_CONCURRENCY; ++i) {
			Assert::AreEqual(std::string("thread_join"), help.getLine());
			Assert::AreEqual(stringize(i + 1), help.getLine());
		}
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // ワーカースレッドの数を指定したときに正しく動作できるか？
		test_helper_t help;
		thread_pool_t thread_pool_;
		spy_thread_pool_t *thread_pool = (spy_thread_pool_t*)(&thread_pool_);
		std_::condition_variable_notify_all_true = [&](
			std::condition_variable *cv
		) {
			help << "condition_variable_notify_all\n";
			help << int(cv) << '\n';
		};
		std_::make_thread_true = [&](
			const std::function<void()> &proc
		) -> std::shared_ptr<std::thread> {
			help << "make_thread\n";
			return help.getSeqPtr<std::thread>();
		};
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		std_::thread_join_true = [&](std::thread *thread) {
			help << "thread_join\n";
			help << int(thread) << '\n';
		};
		std_::unique_lock_unlock_true = [&](
			std::unique_lock<std::mutex> *lock
		) {
			help << "unique_lock_unlock\n";
			help << int(lock) << '\n';
		};
		thread_pool->launch(10);
		Assert::AreEqual(10, int(thread_pool->data->workers.size()));
		for (std::size_t i = 0; i < 10; ++i)
			Assert::AreEqual(int(i + 1), int(thread_pool->data->workers[i].get()));
		int mutex_address = int(&thread_pool->data->mutex);
		int workers_cv_address = int(&thread_pool->data->workers_cv);
		*thread_pool = {};
		for (std::size_t i = 0; i < 10; ++i)
			Assert::AreEqual(std::string("make_thread"), help.getLine());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(mutex_address), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("11"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(workers_cv_address), help.getLine());
		for (std::size_t i = 0; i < 10; ++i) {
			Assert::AreEqual(std::string("thread_join"), help.getLine());
			Assert::AreEqual(stringize(i + 1), help.getLine());
		}
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(thread_pool_t);
}

TEST_METHOD(test_thread_pool_multithread) {
	using lock_t = std::unique_lock<std::mutex>;
	std_::condition_variable_notify_all_true = std_::condition_variable_notify_all_DEF;
	std_::condition_variable_wait_true = std_::condition_variable_wait_DEF;
	std_::make_thread_true = std_::make_thread_DEF;
	std_::make_unique_lock_true = std_::make_unique_lock_DEF;
	std_::thread_join_true = std_::thread_join_DEF;
	std_::unique_lock_unlock_true = std_::unique_lock_unlock_DEF;
	{ // 1個のタスクを非同期実行できるか？
		thread_pool_t thread_pool;
		int result = 0;
		std::mutex mutex;
		std::condition_variable cv;
		bool go = false;
		thread_pool_t::task_t task = [&] {
			auto lock = std_::make_unique_lock(&mutex);
			std_::condition_variable_wait(&cv, lock.get(), [&]() -> bool {
				return go;
			});
			result = 1 + 2;
		};
		thread_pool.launch();
		auto work = thread_pool.ask(task);
		Assert::IsFalse(work.isDone());
		go = true;
		std_::condition_variable_notify_all(&cv);
		work.waitUntilDone();
		Assert::IsTrue(work.isDone());
		Assert::AreEqual(3, result);
	}
	{ // 5個のタスクを非同期実行できるか？
	  // indexが2->4->0->3->1の順で計算する。
		thread_pool_t thread_pool;
		int result = 0;
		std::mutex mutex;
		std::condition_variable cv;
		int index = 5;
		std::vector<thread_pool_t::task_t> tasks;
		tasks.emplace_back([&] {
			auto lock = std_::make_unique_lock(&mutex);
			std_::condition_variable_wait(&cv, lock.get(), [&]() -> bool {
				return index == 0;
			});
			result += 3;
			index = 3;
			std_::condition_variable_notify_all(&cv);
		});
		tasks.emplace_back([&] {
			auto lock = std_::make_unique_lock(&mutex);
			std_::condition_variable_wait(&cv, lock.get(), [&]() -> bool {
				return index == 1;
			});
			result += 5;
			index = 5;
			std_::condition_variable_notify_all(&cv);
		});
		tasks.emplace_back([&] {
			auto lock = std_::make_unique_lock(&mutex);
			std_::condition_variable_wait(&cv, lock.get(), [&]() -> bool {
				return index == 2;
			});
			result += 1;
			index = 4;
			std_::condition_variable_notify_all(&cv);
		});
		tasks.emplace_back([&] {
			auto lock = std_::make_unique_lock(&mutex);
			std_::condition_variable_wait(&cv, lock.get(), [&]() -> bool {
				return index == 3;
			});
			result *= 4;
			index = 1;
			std_::condition_variable_notify_all(&cv);
		});
		tasks.emplace_back([&] {
			auto lock = std_::make_unique_lock(&mutex);
			std_::condition_variable_wait(&cv, lock.get(), [&]() -> bool {
				return index == 4;
			});
			result *= 2;
			index = 0;
			std_::condition_variable_notify_all(&cv);
		});
		thread_pool.launch();
		auto work = thread_pool.ask(tasks);
		Assert::IsFalse(work.isDone());
		index = 2;
		std_::condition_variable_notify_all(&cv);
		work.waitUntilDone();
		Assert::IsTrue(work.isDone());
		Assert::AreEqual(25, result);
	}
	{ // 10×10個のタスクを非同期実行できるか？
		static const int N = 10;
		thread_pool_t thread_pool;
		int result[N][N];
		thread_pool_t::work_t works[N];
		thread_pool.launch(10);
		for (int i = 0; i < N; ++i) {
			std::vector<thread_pool_t::task_t> tasks;
			for (int j = 0; j < N; ++j)
				tasks.emplace_back([&result, i, j] {
					result[i][j] = N * i + j;
				});
			works[i] = thread_pool.ask(tasks);
		}
		for (int i = 0; i < N; ++i) {
			works[i].waitUntilDone();
			Assert::IsTrue(works[i].isDone());
			for (int j = 0; j < N; ++j) 
				Assert::AreEqual(N * i + j, result[i][j]);
		}
	}
}

TEST_METHOD(test_thread_pool_work_isDone) {
	BEGIN_DEF_SPY_CLASS_N(thread_pool_t, work_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		spy_work_t work;
		work.data->tasks_count = 1;
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		work.data->tasks_done = 0;
		Assert::IsFalse(work.isDone());
		work.data->tasks_done = 1;
		Assert::IsTrue(work.isDone());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&work.data->mutex)), help.getLine());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&work.data->mutex)), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(work_t);
}

TEST_METHOD(test_thread_pool_work_onTaskDone) {
	BEGIN_DEF_SPY_CLASS_N(thread_pool_t, work_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		spy_work_t work;
		std_::condition_variable_notify_all_true = [&](
			std::condition_variable *cv
		) {
			help << "condition_variable_notify_all\n";
			help << int(cv) << '\n';
		};
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		std_::unique_lock_unlock_true = [&](
			std::unique_lock<std::mutex> *lock
		) {
			help << "unique_lock_unlock\n";
			help << int(lock) << '\n';
		};
		work.onTaskDone();
		Assert::AreEqual(1, int(work.data->tasks_done));
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&work.data->mutex)), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("condition_variable_notify_all"), help.getLine());
		Assert::AreEqual(stringize(int(&work.data->work_cv)), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(work_t);
}

TEST_METHOD(test_thread_pool_work_waitUntilDone) {
	BEGIN_DEF_SPY_CLASS_N(thread_pool_t, work_t);
	{ // 正しく動作できるか？
		test_helper_t help;
		spy_work_t work;
		work.data->tasks_count = 1;
		std_::condition_variable_wait_true = [&](
			std::condition_variable *cv, 
			std::unique_lock<std::mutex> *lock, 
			const std::function<bool()> &pred
		) {
			help << "condition_variable_wait\n";
			help << int(cv) << '\n';
			help << int(lock) << '\n';
			work.data->tasks_done = 0;
			help << pred() << '\n';
			work.data->tasks_done = 1;
			help << pred() << '\n';
		};
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		work.waitUntilDone();
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&work.data->mutex)), help.getLine());
		Assert::AreEqual(std::string("condition_variable_wait"), help.getLine());
		Assert::AreEqual(stringize(int(&work.data->work_cv)), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(work_t);
}

TEST_METHOD(test_thread_pool_worker_procedure) {
	BEGIN_DEF_SPY_CLASS(thread_pool_t);
	{
		test_helper_t help;
		thread_pool_t thread_pool_;
		spy_thread_pool_t *thread_pool = (spy_thread_pool_t*)(&thread_pool_);
		int index = 0;
		thread_pool_t::task_t task = [&] {
			help << "task\n";
		};
		std_::condition_variable_notify_all_true = [&](
			std::condition_variable *cv
		) {
			help << "condition_variable_notify_all\n";
			help << int(cv) << '\n';
		};
		std_::condition_variable_wait_true = [&](
			std::condition_variable *cv, 
			std::unique_lock<std::mutex> *lock, 
			const std::function<bool()> &pred
		) {
			help << "condition_variable_wait\n";
			help << int(cv) << '\n';
			help << int(lock) << '\n';
			if (index == 0) {
				help << pred() << '\n';
				thread_pool->data->tasks.push(task);
				help << pred() << '\n';
			} else if (index == 1) {
				help << pred() << '\n';
				thread_pool->data->leave = true;
				help << pred() << '\n';
			} else Assert::Fail();
			++index;
		};
		std_::make_unique_lock_true = [&](
			std::mutex *mutex
		) -> std::shared_ptr<std::unique_lock<std::mutex>> {
			help << "make_unique_lock\n";
			help << int(mutex) << '\n';
			return help.getSeqPtr<std::unique_lock<std::mutex>>();
		};
		std_::unique_lock_unlock_true = [&](
			std::unique_lock<std::mutex> *lock
		) {
			help << "unique_lock_unlock\n";
			help << int(lock) << '\n';
		};
		thread_pool->worker_procedure();
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->mutex)), help.getLine());
		Assert::AreEqual(std::string("condition_variable_wait"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->workers_cv)), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("unique_lock_unlock"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("task"), help.getLine());
		Assert::AreEqual(std::string("make_unique_lock"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->mutex)), help.getLine());
		Assert::AreEqual(std::string("condition_variable_wait"), help.getLine());
		Assert::AreEqual(stringize(int(&thread_pool->data->workers_cv)), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("0"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	END_DEF_SPY_CLASS(thread_pool_t);
}

TEST_METHOD(test_utf8_divideLongWords) {
	{ // 1バイト文字のみを含むメッセージを分割しないか？
		Assert::AreEqual(std::string(u8"1234567890123456"), utf8_divideLongWords(u8"1234567890123456", 1));
	}
	{ // マルチバイト文字のみを含むメッセージを分割できるか？
		Assert::AreEqual(std::string(u8"あいうえお"), utf8_divideLongWords(u8"あいうえお", 1));
		Assert::AreEqual(std::string(u8"あいうえお かきくけこ"), utf8_divideLongWords(u8"あいうえおかきくけこ", 1));
		Assert::AreEqual(std::string(u8"あいうえお かきくけこ"), utf8_divideLongWords(u8"あいうえお かきくけこ", 1));
		Assert::AreEqual(std::string(u8"あい うえおかき く けこ"), utf8_divideLongWords(u8"あい うえおかきく けこ", 1));
	}
	{ // 1バイト文字とマルチバイト文字の両方を含むメッセージを分割できるか？
		Assert::AreEqual(std::string(u8"あ a い i う u え e お o"), utf8_divideLongWords(u8"あaいiうuえeおo", 1));
	}
	{ // リンクを含むメッセージを分割できるか？
		Assert::AreEqual(
			std::string(u8"あいうえお\x12""1かきくけこ\x12""さしすせそ"), 
			utf8_divideLongWords(u8"あいうえお\x12""1かきくけこ\x12""さしすせそ", 1)
		);
		Assert::AreEqual(
			std::string(u8"あいうえお\x12""1かきくけこ さしすせそ\x12""たちつてと"), 
			utf8_divideLongWords(u8"あいうえお\x12""1かきくけこさしすせそ\x12""たちつてと", 1)
		);
		Assert::AreEqual(
			std::string(u8"あいうえお\x12""12かきくけこ さしすせそ\x12""たちつてと"), 
			utf8_divideLongWords(u8"あいうえお\x12""12かきくけこさしすせそ\x12""たちつてと", 2)
		);
	}
}

TEST_METHOD(test_utf8_getLetterSize) {
	// 正しく動作できるか？
	Assert::AreEqual(1            , int(utf8_getLetterSize(0x40u)));
	Assert::AreEqual(0            , int(utf8_getLetterSize(0xa0u)));
	Assert::AreEqual(2            , int(utf8_getLetterSize(0xd0u)));
	Assert::AreEqual(3            , int(utf8_getLetterSize(0xe8u)));
	Assert::AreEqual(4            , int(utf8_getLetterSize(0xf4u)));
	Assert::AreEqual(int(UINT_MAX), int(utf8_getLetterSize(0xfau)));
}

TEST_METHOD(test_window_getClientRect) {
	{ // 正しく動作できるか？
		test_helper_t help;
		api::GetClientRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetClientRect\n";
			help << int(hWnd) << '\n';
			lpRect->left = help.getSeq<LONG>();
			lpRect->top = help.getSeq<LONG>();
			lpRect->right = help.getSeq<LONG>();
			lpRect->bottom = help.getSeq<LONG>();
			return TRUE;
		};
		RECT rect = window_getClientRect(help.getSeq<HWND>());
		Assert::AreEqual(2, int(rect.left));
		Assert::AreEqual(3, int(rect.top));
		Assert::AreEqual(4, int(rect.right));
		Assert::AreEqual(5, int(rect.bottom));
		Assert::AreEqual(std::string("GetClientRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_window_getRect) {
	{ // 正しく動作できるか？
		test_helper_t help;
		api::GetWindowRect_true = [&](HWND hWnd, LPRECT lpRect) -> BOOL {
			help << "GetWindowRect\n";
			help << int(hWnd) << '\n';
			lpRect->left = help.getSeq<LONG>();
			lpRect->top = help.getSeq<LONG>();
			lpRect->right = help.getSeq<LONG>();
			lpRect->bottom = help.getSeq<LONG>();
			return TRUE;
		};
		RECT rect = window_getRect(help.getSeq<HWND>());
		Assert::AreEqual(2, int(rect.left));
		Assert::AreEqual(3, int(rect.top));
		Assert::AreEqual(4, int(rect.right));
		Assert::AreEqual(5, int(rect.bottom));
		Assert::AreEqual(std::string("GetWindowRect"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_window_getText) {
	{ // 空っぽのテキストを取得できるか？
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
			*lpString = '\0';
			return 0;
		};
		Assert::AreEqual(std::string(), window_getText(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 空っぽではないテキストを取得できるか？
		test_helper_t help;
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
		Assert::AreEqual(std::string("2"), window_getText(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("GetWindowText"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_window_isTopMost) {
	{ // 最上位ウインドウを判定できるか？
		test_helper_t help;
		api::GetWindowLong_true = [&](HWND hWnd, int nIndex) -> LONG {
			help << "GetWindowLong\n";
			help << int(hWnd) << '\n';
			help << nIndex << '\n';
			return WS_EX_TOPMOST;
		};
		Assert::IsTrue(window_isTopMost(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(GWL_EXSTYLE), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 最上位ではないウインドウを判定できるか？
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
		Assert::IsFalse(window_isTopMost(help.getSeq<HWND>()));
		Assert::AreEqual(std::string("GetWindowLong"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(stringize(GWL_EXSTYLE), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_wstring_utf16ToSJIS) {
	// 半角の文字列を変換できるか？
	Assert::AreEqual(std::string("Hoge"), wstring_utf16ToSJIS(L"Hoge"));
	// 全角の文字列を変換できるか？
	Assert::AreEqual(std::string("ホゲ"), wstring_utf16ToSJIS(L"ホゲ"));
}

TEST_METHOD(test_wstring_utf16ToUtf8) {
	// 半角の文字列を変換できるか？
	Assert::AreEqual(std::string(u8"Hoge"), wstring_utf16ToUtf8(L"Hoge"));
	// 全角の文字列を変換できるか？
	Assert::AreEqual(std::string(u8"ホゲ"), wstring_utf16ToUtf8(L"ホゲ"));
}

};

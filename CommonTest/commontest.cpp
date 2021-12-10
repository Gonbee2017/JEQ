#include "common.hpp"

#include <CppUnitTest.h>
#include <windows.h>

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <filesystem>
#include <iomanip>
#include <map>
#include <string>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(common_test) {

TEST_METHOD(test_char_isJLead) {
	// 境界値で正しく動作できるか？
	Assert::IsFalse(char_isJLead(char(0x80)));
	Assert::IsTrue (char_isJLead(char(0x81)));
	Assert::IsTrue (char_isJLead(char(0x9f)));
	Assert::IsFalse(char_isJLead(char(0xa0)));
	Assert::IsFalse(char_isJLead(char(0xdf)));
	Assert::IsTrue (char_isJLead(char(0xe0)));
	Assert::IsTrue (char_isJLead(char(0xfc)));
	Assert::IsFalse(char_isJLead(char(0xfd)));
}

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
	Assert::AreEqual(1 , destringize<int   >("1" ));
	Assert::AreEqual(-1, destringize<int   >("-1"));
	Assert::AreEqual(0 , destringize<int   >(""  ));
	Assert::AreEqual(0 , destringize<int   >("a" ));
	Assert::AreEqual(-1, destringize<int,-1>("a" ));

	// DWORDに変換できるか？
	Assert::AreEqual(0x00000001ul, destringize<DWORD             >("1", std::hex));
	Assert::AreEqual(0x0000000ful, destringize<DWORD             >("f", std::hex));
	Assert::AreEqual(0x00000000ul, destringize<DWORD             >("" , std::hex));
	Assert::AreEqual(0x00000000ul, destringize<DWORD             >("g", std::hex));
	Assert::AreEqual(0xfffffffful, destringize<DWORD,0xfffffffful>("g", std::hex));

	// boolに変換できるか？
	Assert::IsFalse(destringize<bool     >("0"));
	Assert::IsTrue (destringize<bool     >("1"));
	Assert::IsFalse(destringize<bool     >("" ));
	Assert::IsFalse(destringize<bool     >("a"));
	Assert::IsTrue (destringize<bool,true>("a"));
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
		std::filesystem::path mod_path = module_getPath(help.getSeq<HMODULE>());
		Assert::AreEqual(std::string("2"), mod_path.string());
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
		std::stringstream log;
		tm_.tm_year = 100 + help.getSeq();
		tm_.tm_mon = help.getSeq() - 1;
		tm_.tm_mday = help.getSeq();
		tm_.tm_hour = help.getSeq();
		tm_.tm_min = help.getSeq();
		tm_.tm_sec = help.getSeq();
		now_ = std::chrono::system_clock::from_time_t(std::mktime(&tm_));
		now_ += std::chrono::milliseconds(help.getSeq());
		putLog(log, help.getSeqStr());
		help = {log.str()};
		Assert::AreEqual(std::string("2001/02/03 04:05:06.007 8"), help.getLine());
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

TEST_METHOD(test_string_printf) {
	// 正しく動作できるか？
	Assert::AreEqual(std::string("0012 0.34 Hoge"), string_printf("%04d %.2f %s", 12, 0.345, "Hoge"));
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

TEST_METHOD(test_string_split) {
	{ // 半角の文字列を分割できるか？空文字列は許す。
		std::string str = "a,bb,,ccc,,,";
		auto toks = string_split(str, ',');
		Assert::AreEqual(std::size_t(7), toks.size());
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
		auto toks = string_split(str, ',', false);
		Assert::AreEqual(std::size_t(3), toks.size());
		Assert::AreEqual(std::string("a"),   toks[0]);
		Assert::AreEqual(std::string("bb"),  toks[1]);
		Assert::AreEqual(std::string("ccc"), toks[2]);
	}
	{ // 全角の文字列を分割できるか？空文字列は許す。
		std::string str = "い,ろろ,,ははは,,,";
		auto toks = string_split(str, ',');
		Assert::AreEqual(std::size_t(7), toks.size());
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
		auto toks = string_split(str, ',', false);
		Assert::AreEqual(std::size_t(3), toks.size());
		Assert::AreEqual(std::string("い"),     toks[0]);
		Assert::AreEqual(std::string("ろろ"),   toks[1]);
		Assert::AreEqual(std::string("ははは"), toks[2]);
	}
}

TEST_METHOD(test_string_toLower) {
	// 正しく動作できるか？
	Assert::AreEqual(std::string("hいoろgはe"), string_toLower("HいoろGはe"));
	Assert::AreEqual(std::string("ちfりuぬgるaを"), string_toLower("ちfりUぬgるAを"));
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
	{ // getSeqとgetSeqStrとsetSeqBaseは正しく動作できるか？
		test_helper_t help;
		Assert::AreEqual(1, help.getSeq());
		Assert::AreEqual(std::string("2"), help.getSeqStr());
		help.setSeqBase();
		Assert::AreEqual(12, help.getSeq(10));
		Assert::AreEqual(std::string("13"), help.getSeqStr(11));
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

TEST_METHOD(test_wstring_utf16ToUtf8) {
	// 半角の文字列を変換できるか？
	Assert::AreEqual(std::string(u8"Hoge"), wstring_utf16ToUtf8(L"Hoge"));
	// 全角の文字列を変換できるか？
	Assert::AreEqual(std::string(u8"ホゲ"), wstring_utf16ToUtf8(L"ホゲ"));
}

};

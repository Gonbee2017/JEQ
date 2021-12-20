#include "dinput8.hpp"
#include "jcommon.hpp"

#include <CppUnitTest.h>
#include <windows.h>

#include <cstddef>
#include <cstring>
#include <filesystem>
#include <ios>
#include <memory>
#include <ostream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(dinput8_test) {
public:

TEST_METHOD(test_DirectInput8Create) {
	using test_DirectInput8Create_t = test_trampoline_t<HRESULT,HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN>;
	{ // 正しく呼び出せるか？
		test_helper_t help;
		test_DirectInput8Create_t::target = [&](
			HINSTANCE hinst,
			DWORD version,
			REFIID riidltf,
			LPVOID *out,
			LPUNKNOWN outer
		) -> HRESULT {
			help << "DirectInput8Create\n";
			help << int(hinst) << '\n';
			help << version << '\n';
			help << riidltf.Data1 << '\n';
			help << int(out) << '\n';
			help << int(outer) << '\n';
			return 0;
		};
		context.DirectInput8Create_true = &test_DirectInput8Create_t::trampoline;
		help.setSeqBase();
		DirectInput8Create(
			help.getSeq<HINSTANCE>(1), 
			help.getSeq<DWORD>(2), 
			IID{help.getSeq<unsigned long>(3)}, 
			help.getSeq<LPVOID*>(4), 
			help.getSeq<LPUNKNOWN>(5)
		);
		Assert::AreEqual(std::string("DirectInput8Create"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_context_startup) {
	{ // コマンドライン引数でパスが指定されていないときに正しく動作できるか？
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
		context.startup(help.getSeq<HMODULE>());
		Assert::AreEqual(1, int(context.fake_dll_handle));
		Assert::AreEqual(5, int(context.log.get()));
		Assert::AreEqual(std::string("2\\3.4"), context.fake_dll_path.string());
		Assert::AreEqual(std::string("3.4"), context.dll_name);
		Assert::AreEqual(std::string("2\\3.log"), context.log_path.string());
		Assert::AreEqual(std::string("2\\3.ini"), context.ini_path.string());
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("2\\3.log"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // コマンドライン引数でパスが指定されているときに正しく動作できるか？
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
		context.startup(help.getSeq<HMODULE>());
		Assert::AreEqual(1, int(context.fake_dll_handle));
		Assert::AreEqual(5, int(context.log.get()));
		Assert::AreEqual(std::string("2\\3.4"), context.fake_dll_path.string());
		Assert::AreEqual(std::string("3.4"), context.dll_name);
		Assert::AreEqual(std::string("hoge"), context.log_path.string());
		Assert::AreEqual(std::string("fuga"), context.ini_path.string());
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("hoge"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_loadPlugin) {
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.log = help.getSeqPtr<std::ostream>();
		int index = 0;
		api::FreeLibrary = [&](HMODULE hLibModule) -> BOOL {
			help << "FreeLibrary\n";
			help << int(hLibModule) << '\n';
			return TRUE;
		};
		api::GetLastError = [&]() -> DWORD {
			help << "GetLastError\n";
			return help.getSeq<DWORD>();
		};
		api::LoadLibrary_true = [&](LPCSTR lpLibFileName) -> HMODULE {
			help << "LoadLibrary\n";
			help << lpLibFileName << '\n';
			HMODULE hmodule;
			if (index == 1) hmodule = NULL;
			else hmodule = help.getSeq<HMODULE>();
			++index;
			return hmodule;
		};
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		loadPlugin(help.getSeqStr());
		loadPlugin(help.getSeqStr());
		loadPlugin(help.getSeqStr());
		Assert::AreEqual(2, int(context.plugin_handles.size()));
		Assert::AreEqual(3, int(context.plugin_handles[0]));
		Assert::AreEqual(7, int(context.plugin_handles[1]));
		Assert::AreEqual(2, int(context.exits.size()));
		reverseClear(context.exits);
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("GetLastError"), help.getLine());
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::api::LoadLibraryA", 5), help.getLine().substr(24));
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("FreeLibrary"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("FreeLibrary"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_loadPlugins) {
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.ini_path = help.getSeqStr();
		api::FreeLibrary = [&](HMODULE hLibModule) -> BOOL {
			help << "FreeLibrary\n";
			help << int(hLibModule) << '\n';
			return TRUE;
		};
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
			std::strcpy(p, "=1"); p += std::strlen(p) + 1;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "=0"); p += std::strlen(p) + 1;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, "=1"); p += std::strlen(p) + 1;
			*p++ = '\0';
			return p - lpReturnedString - 2;
		};
		api::LoadLibrary_true = [&](LPCSTR lpLibFileName) -> HMODULE {
			help << "LoadLibrary\n";
			help << lpLibFileName << '\n';
			return help.getSeq<HMODULE>();
		};
		loadPlugins();
		Assert::AreEqual(2, int(context.plugin_handles.size()));
		Assert::AreEqual(5, int(context.plugin_handles[0]));
		Assert::AreEqual(6, int(context.plugin_handles[1]));
		Assert::AreEqual(2, int(context.exits.size()));
		reverseClear(context.exits);
		Assert::AreEqual(std::string("GetPrivateProfileSection"), help.getLine());
		Assert::AreEqual(std::string("Plugin"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("FreeLibrary"), help.getLine());
		Assert::AreEqual(std::string("6"), help.getLine());
		Assert::AreEqual(std::string("FreeLibrary"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_loadTrueDLL) {
	{ // エラーをスローできるか？
		test_helper_t help;
		context = {};
		context.dll_name = help.getSeqStr();
		std_::getenv_true = [&](const char *name) -> char* {
			help << "getenv\n";
			help << name << '\n';
			return "";
		};
		try {
			loadTrueDLL();
			Assert::Fail();
		} catch (const error_t &err) {
			Assert::AreEqual(string_printf("本物の%sが見つかりませんでした。", "1"), err.getMessage());
		}
		Assert::AreEqual(std::string("getenv"), help.getLine());
		Assert::AreEqual(std::string("PATH"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.fake_dll_path = help.getSeqStr();
		context.dll_name = help.getSeqStr();
		api::FreeLibrary = [&](HMODULE hLibModule) -> BOOL {
			help << "FreeLibrary\n";
			help << int(hLibModule) << '\n';
			return TRUE;
		};
		api::GetProcAddress_true = [&](HMODULE hModule, LPCSTR  lpProcName) -> FARPROC {
			help << "GetProcAddress\n";
			help << int(hModule) << '\n';
			help << lpProcName << '\n';
			return help.getSeq<FARPROC>();
		};
		api::LoadLibrary_true = [&](LPCSTR lpLibFileName) -> HMODULE {
			help << "LoadLibrary\n";
			help << lpLibFileName << '\n';
			return help.getSeq<HMODULE>();
		};
		std_::filesystem_equivalent_true = [&](
			const std::filesystem::path &p1,
			const std::filesystem::path &p2
		) -> bool {
			help << "filesystem_equivalent\n";
			help << p1.string() << '\n';
			help << p2.string() << '\n';
			return false;
		};
		std_::filesystem_exists = [&](
			const std::filesystem::path &p
		) -> bool {
			help << "filesystem_exists\n";
			help << p.string() << '\n';
			return true;
		};
		std_::getenv_true = [&](const char *name) -> char* {
			help << "getenv\n";
			help << name << '\n';
			return help.getSeqStr().data();
		};
		loadTrueDLL();
		Assert::AreEqual(std::string("3\\2"), context.true_dll_path.string());
		Assert::AreEqual(4, int(context.true_dll_handle));
		Assert::AreEqual(1, int(context.exits.size()));
		reverseClear(context.exits);
		Assert::AreEqual(5, int(context.DirectInput8Create_true));
		Assert::AreEqual(std::string("getenv"), help.getLine());
		Assert::AreEqual(std::string("PATH"), help.getLine());
		Assert::AreEqual(std::string("filesystem_exists"), help.getLine());
		Assert::AreEqual(std::string("3\\2"), help.getLine());
		Assert::AreEqual(std::string("filesystem_equivalent"), help.getLine());
		Assert::AreEqual(std::string("3\\2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("3\\2"), help.getLine());
		Assert::AreEqual(std::string("GetProcAddress"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string("DirectInput8Create"), help.getLine());
		Assert::AreEqual(std::string("FreeLibrary"), help.getLine());
		Assert::AreEqual(std::string("4"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_onProcessAttach) {
	{ // dinput8.logを開けなかったときに失敗できるか？
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
		Assert::IsFalse(onProcessAttach(help.getSeq<HMODULE>()));
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("2\\3.log"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		api::FreeLibrary = [&](HMODULE hLibModule) -> BOOL {
			help << "FreeLibrary\n";
			help << int(hLibModule) << '\n';
			return TRUE;
		};
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
			std::strcpy(p, "=1"); p += std::strlen(p); p += std::strlen(p) + 1;
			*p++ = '\0';
			return p - lpReturnedString - 2;
		};
		api::GetProcAddress_true = [&](HMODULE hModule, LPCSTR  lpProcName) -> FARPROC {
			help << "GetProcAddress\n";
			help << int(hModule) << '\n';
			help << lpProcName << '\n';
			return help.getSeq<FARPROC>();
		};
		api::LoadLibrary_true = [&](LPCSTR lpLibFileName) -> HMODULE {
			help << "LoadLibrary\n";
			help << lpLibFileName << '\n';
			return help.getSeq<HMODULE>();
		};
		std_::filesystem_equivalent_true = [&](
			const std::filesystem::path &p1,
			const std::filesystem::path &p2
		) -> bool {
			help << "filesystem_equivalent\n";
			help << p1.string() << '\n';
			help << p2.string() << '\n';
			return false;
		};
		std_::filesystem_exists = [&](
			const std::filesystem::path &p
		) -> bool {
			help << "filesystem_exists\n";
			help << p.string() << '\n';
			return true;
		};
		std_::getenv_true = [&](const char *name) -> char* {
			help << "getenv\n";
			help << name << '\n';
			return help.getSeqStr().data();
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
		Assert::IsTrue(onProcessAttach(help.getSeq<HMODULE>()));
		Assert::AreEqual(1, int(context.fake_dll_handle));
		Assert::AreEqual(5, int(context.log.get()));
		Assert::AreEqual(std::string("2\\3.4"), context.fake_dll_path.string());
		Assert::AreEqual(std::string("3.4"), context.dll_name);
		Assert::AreEqual(std::string("2\\3.log"), context.log_path.string());
		Assert::AreEqual(std::string("2\\3.ini"), context.ini_path.string());
		Assert::IsTrue(bool(context.log));
		Assert::AreEqual(std::string("6\\3.4"), context.true_dll_path.string());
		Assert::AreEqual(7, int(context.true_dll_handle));
		Assert::AreEqual(8, int(context.DirectInput8Create_true));
		Assert::AreEqual(1, int(context.plugin_handles.size()));
		Assert::AreEqual(10, int(context.plugin_handles[0]));
		Assert::AreEqual(2, int(context.exits.size()));
		reverseClear(context.exits);
		Assert::AreEqual(std::string("GetModuleFileName"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("GetCommandLine"), help.getLine());
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("2\\3.log"), help.getLine());
		Assert::AreEqual(stringize(int(std::ios_base::app)), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("5"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string("getenv"), help.getLine());
		Assert::AreEqual(std::string("PATH"), help.getLine());
		Assert::AreEqual(std::string("filesystem_exists"), help.getLine());
		Assert::AreEqual(std::string("6\\3.4"), help.getLine());
		Assert::AreEqual(std::string("filesystem_equivalent"), help.getLine());
		Assert::AreEqual(std::string("6\\3.4"), help.getLine());
		Assert::AreEqual(std::string("2\\3.4"), help.getLine());
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("6\\3.4"), help.getLine());
		Assert::AreEqual(std::string("GetProcAddress"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string("DirectInput8Create"), help.getLine());
		Assert::AreEqual(std::string("GetPrivateProfileSection"), help.getLine());
		Assert::AreEqual(std::string("Plugin"), help.getLine());
		Assert::AreEqual(std::string("2\\3.ini"), help.getLine());
		Assert::AreEqual(std::string("LoadLibrary"), help.getLine());
		Assert::AreEqual(std::string("9"), help.getLine());
		Assert::AreEqual(std::string("FreeLibrary"), help.getLine());
		Assert::AreEqual(std::string("10"), help.getLine());
		Assert::AreEqual(std::string("FreeLibrary"), help.getLine());
		Assert::AreEqual(std::string("7"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_onProcessDetach) {
	{ // 正しく動作できるか？
		test_helper_t help;
		context = {};
		context.exits.emplace_back([&] {
			help << "procedure\n";
			help << 1 << '\n';
		});
		context.exits.emplace_back([&] {
			help << "procedure\n";
			help << 2 << '\n';
		});
		Assert::IsTrue(onProcessDetach());
		Assert::AreEqual(0, int(context.exits.size()));
		Assert::AreEqual(std::string("procedure"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("procedure"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_searchTrueDLL) {
	{ // 検索パスがゼロ個のときに空っぽのパスを返せるか？
		test_helper_t help;
		std_::getenv_true = [&](const char *name) -> char* {
			help << "getenv\n";
			help << name << '\n';
			return "";
		};
		std::filesystem::path true_dll_path = searchTrueDLL();
		Assert::AreEqual(std::string(), true_dll_path.string());
		Assert::AreEqual(std::string("getenv"), help.getLine());
		Assert::AreEqual(std::string("PATH"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 検索パスが3個のときに正しく検索できるか？
		test_helper_t help;
		context.fake_dll_path = help.getSeqStr();
		context.dll_name = help.getSeqStr();
		std_::filesystem_equivalent_true = [&] (
			const std::filesystem::path &p1,
			const std::filesystem::path &p2
		) -> bool {
			help << "filesystem_equivalent\n";
			help << p1.string() << '\n';
			help << p2.string() << '\n';
			static const bool RESS[] = {true, false};
			static const bool *res = RESS;
			return *res++;
		};
		std_::filesystem_exists = [&] (const std::filesystem::path &p) -> bool {
			help << "filesystem_exists\n";
			help << p.string() << '\n';
			static const bool RESS[] = {false, true, true};
			static const bool *res = RESS;
			return *res++;
		};
		std_::getenv_true = [&](const char *name) -> char* {
			help << "getenv\n";
			help << name << '\n';
			static char buff[MAX_PATH];
			char *p = buff;
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, ";"); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			std::strcpy(p, ";"); p += std::strlen(p);
			std::strcpy(p, help.getSeqStr().c_str()); p += std::strlen(p);
			return buff;
		};
		Assert::AreEqual(std::string("5\\2"), searchTrueDLL().string());
		Assert::AreEqual(std::string("getenv"), help.getLine());
		Assert::AreEqual(std::string("PATH"), help.getLine());
		Assert::AreEqual(std::string("filesystem_exists"), help.getLine());
		Assert::AreEqual(std::string("3\\2"), help.getLine());
		Assert::AreEqual(std::string("filesystem_exists"), help.getLine());
		Assert::AreEqual(std::string("4\\2"), help.getLine());
		Assert::AreEqual(std::string("filesystem_equivalent"), help.getLine());
		Assert::AreEqual(std::string("4\\2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("filesystem_exists"), help.getLine());
		Assert::AreEqual(std::string("5\\2"), help.getLine());
		Assert::AreEqual(std::string("filesystem_equivalent"), help.getLine());
		Assert::AreEqual(std::string("5\\2"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

};

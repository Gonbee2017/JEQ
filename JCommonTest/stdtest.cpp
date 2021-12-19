#include "jcommon.hpp"

#include <CppUnitTest.h>

#include <filesystem>
#include <ios>
#include <string>
#include <system_error>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(std_test) {
	
TEST_METHOD(test_error) {
	{ // ヌルコンストラクタは正しく動作できるか？
		std_::error err;
		Assert::AreEqual(std::string(), err.getFunctionName());
		Assert::AreEqual(std::string(), err.getMessage());
		Assert::AreEqual(0, err.getNumber());
	}
	{ // コンストラクタ(std::string)は正しく動作できるか？
		test_helper_t help;
		std_::geterrno = [&]() -> int {
			return help.getSeq();
		};
		std_::error err(help.getSeqStr());
		Assert::AreEqual(std::string("1"), err.getFunctionName());
		Assert::AreEqual(std::string("1が失敗しました。(2)"), err.getMessage());
		Assert::AreEqual(2, err.getNumber());
	}
	{ // コンストラクタ(std::string,int)は正しく動作できるか？
		test_helper_t help;
		help.setSeqBase();
		std_::error err(help.getSeqStr(1), help.getSeq(2));
		Assert::AreEqual(std::string("1"), err.getFunctionName());
		Assert::AreEqual(std::string("1が失敗しました。(2)"), err.getMessage());
		Assert::AreEqual(2, err.getNumber());
	}
}

TEST_METHOD(test_filesystem_equivalent) {
	{ // エラーをスローできるか？
		test_helper_t help;
		std_::filesystem_equivalent_true = [&](
			const std::filesystem::path &p1, 
			const std::filesystem::path &p2
		) -> bool {
			help << "filesystem_equivalent\n";
			throw std::system_error(std::error_code(help.getSeq(), std::generic_category()));
		};
		try {
			help.setSeqBase();
			std_::filesystem_equivalent(help.getSeqStr(1), help.getSeqStr(2));
			Assert::Fail();
		} catch (const std_::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::std_::filesystem_equivalent", 3), err.getMessage());
		}
		Assert::AreEqual(std::string("filesystem_equivalent"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		std_::filesystem_equivalent_true = [&](
			const std::filesystem::path &p1, 
			const std::filesystem::path &p2
		) -> bool {
			help << "filesystem_equivalent\n";
			help << p1.string() << '\n';
			help << p2.string() << '\n';
			return help.getSeq() % 2;
		};
		help.setSeqBase();
		Assert::IsTrue(std_::filesystem_equivalent(help.getSeqStr(1), help.getSeqStr(2)));
		Assert::AreEqual(std::string("filesystem_equivalent"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_getenv) {
	{ // エラーをスローできるか？
		test_helper_t help;
		std_::getenv_true = [&](const char *name) -> char* {
			help << "getenv\n";
			return NULL;
		};
		std_::geterrno = [&]() -> int {
			help << "geterrno\n";
			return help.getSeq<DWORD>();
		};
		try {
			std_::getenv(help.getSeqStr().c_str());
			Assert::Fail();
		} catch (const std_::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::std_::getenv", 2), err.getMessage());
		}
		Assert::AreEqual(std::string("getenv"), help.getLine());
		Assert::AreEqual(std::string("geterrno"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		std_::getenv_true = [&](const char *name) -> char* {
			help << "getenv\n";
			help << name << '\n';
			return help.getSeqStr().data();
		};
		Assert::AreEqual(std::string("2"), std::string(std_::getenv(help.getSeqStr().c_str())));
		Assert::AreEqual(std::string("getenv"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_make_ofstream) {
	{ // エラーをスローできるか？
		test_helper_t help;
		std_::make_ofstream_true = [&](
			const std::string &s, 
			std::ios_base::openmode mode
		) -> std::shared_ptr<std::ostream> {
			help << "make_ofstream\n";
			return help.getSeqPtr<std::ostream>();
		};
		std_::ostream_exceptions_set_true = [&](
			std::ostream *out, 
			std::ios::iostate except
		) {
			help << "ostream_exceptions_set\n";
			throw std::ios_base::failure(help.getSeqStr());
		};
		try {
			help.setSeqBase();
			std_::make_ofstream(help.getSeqStr(1), help.getSeq<std::ios_base::openmode>(2));
			Assert::Fail();
		} catch (const std_::error &err) {
			Assert::AreEqual(string_printf("%sが失敗しました。(%d)", "jeq::std_::make_ofstream", 1), err.getMessage());
		}
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // 正しく呼び出せるか？
		test_helper_t help;
		std_::make_ofstream_true = [&](
			const std::string &s, 
			std::ios_base::openmode mode
		) -> std::shared_ptr<std::ostream> {
			help << "make_ofstream\n";
			help << s << '\n';
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
		help.setSeqBase();
		Assert::AreEqual(3, int(std_::make_ofstream(help.getSeqStr(1), help.getSeq<std::ios_base::openmode>(2)).get()));
		Assert::AreEqual(std::string("make_ofstream"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("3"), help.getLine());
		Assert::AreEqual(stringize(std::ios_base::failbit), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ostream_exceptions_set) {
	{ // 正しく呼び出せるか？
		test_helper_t help;
		std_::ostream_exceptions_set_true = [&](
			std::ostream *out, 
			std::ios::iostate except
		) {
			help << "ostream_exceptions_set\n";
			help << int(out) << '\n';
			help << int(except) << '\n';
		};
		help.setSeqBase();
		std_::ostream_exceptions_set(help.getSeq<std::ostream*>(1), help.getSeq<std::ios::iostate>(2));
		Assert::AreEqual(std::string("ostream_exceptions_set"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

TEST_METHOD(test_ostream_putLine) {
	{ // 正しく呼び出せるか？
		test_helper_t help;
		std_::ostream_putLine_true = [&](
			const std::ostream *out,
			const std::string &line
		) {
			help << "ostream_putLine\n";
			help << int(out) << '\n';
			help << line << '\n';
		};
		help.setSeqBase();
		std_::ostream_putLine(help.getSeq<std::ostream*>(1), help.getSeqStr(2));
		Assert::AreEqual(std::string("ostream_putLine"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}

};

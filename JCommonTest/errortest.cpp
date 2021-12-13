#include "jcommon.hpp"
#include "error.hpp"

#include <CppUnitTest.h>

#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(error_test) {

TEST_METHOD(test_error) {
	{ // ヌルコンストラクタは正しく動作できるか？
		error err;
		Assert::AreEqual(std::string(), err.getMessage());
	}
	{ // コンストラクタ(std::string)は正しく動作できるか？
		test_helper_t hist;
		error err(hist.getSeqStr());
		Assert::AreEqual(std::string("1"), err.getMessage());
	}
}

};

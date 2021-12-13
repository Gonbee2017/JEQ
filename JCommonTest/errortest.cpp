#include "jcommon.hpp"
#include "error.hpp"

#include <CppUnitTest.h>

#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(error_test) {

TEST_METHOD(test_error) {
	{ // �k���R���X�g���N�^�͐���������ł��邩�H
		error err;
		Assert::AreEqual(std::string(), err.getMessage());
	}
	{ // �R���X�g���N�^(std::string)�͐���������ł��邩�H
		test_helper_t hist;
		error err(hist.getSeqStr());
		Assert::AreEqual(std::string("1"), err.getMessage());
	}
}

};

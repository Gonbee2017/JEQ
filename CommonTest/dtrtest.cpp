#include "common.hpp"

#include <CppUnitTest.h>
#include <windows.h>
#include <detours.h>

#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace jeq;

TEST_CLASS(dtr_test) {
						
TEST_METHOD(test_DetourAttach) {
	{ // �G���[���X���[�ł��邩�H
		test_helper_t help;
		dtr::DetourAttach_true = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourAttach\n";
			return ERROR_INVALID_OPERATION;
		};
		try {
			help.setSeqBase();
			dtr::DetourAttach(help.getSeq<PVOID*>(1), help.getSeq<PVOID>(2));
			Assert::Fail();
		} catch (const dtr::error &err) {
			Assert::AreEqual(string_printf("%s�����s���܂����B(%d)", "jeq::dtr::DetourAttach", ERROR_INVALID_OPERATION), err.getMessage());
		}
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // �������Ăяo���邩�H
		test_helper_t help;
		dtr::DetourAttach_true = [&](
			PVOID *ppPointer, 
			PVOID pDetour
		) -> LONG {
			help << "DetourAttach\n";
			help << int(ppPointer) << '\n';
			help << int(pDetour) << '\n';
			return NO_ERROR;
		};
		help.setSeqBase();
		Assert::AreEqual(NO_ERROR, dtr::DetourAttach(help.getSeq<PVOID*>(1), help.getSeq<PVOID>(2)));
		Assert::AreEqual(std::string("DetourAttach"), help.getLine());
		Assert::AreEqual(std::string("1"), help.getLine());
		Assert::AreEqual(std::string("2"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
				
TEST_METHOD(test_DetourTransactionBegin) {
	{ // �G���[���X���[�ł��邩�H
		test_helper_t help;
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return ERROR_INVALID_OPERATION;
		};
		try {
			dtr::DetourTransactionBegin();
			Assert::Fail();
		} catch (const dtr::error &err) {
			Assert::AreEqual(string_printf("%s�����s���܂����B(%d)", "jeq::dtr::DetourTransactionBegin", ERROR_INVALID_OPERATION), err.getMessage());
		}
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // �������Ăяo���邩�H
		test_helper_t help;
		dtr::DetourTransactionBegin_true = [&]() -> LONG {
			help << "DetourTransactionBegin\n";
			return NO_ERROR;
		};
		Assert::AreEqual(NO_ERROR, dtr::DetourTransactionBegin());
		Assert::AreEqual(std::string("DetourTransactionBegin"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
					
TEST_METHOD(test_DetourTransactionCommit) {
	{ // �G���[���X���[�ł��邩�H
		test_helper_t help;
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return ERROR_INVALID_OPERATION;
		};
		try {
			dtr::DetourTransactionCommit();
			Assert::Fail();
		} catch (const dtr::error &err) {
			Assert::AreEqual(string_printf("%s�����s���܂����B(%d)", "jeq::dtr::DetourTransactionCommit", ERROR_INVALID_OPERATION), err.getMessage());
		}
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
	{ // �������Ăяo���邩�H
		test_helper_t help;
		dtr::DetourTransactionCommit_true = [&]() -> LONG {
			help << "DetourTransactionCommit\n";
			return NO_ERROR;
		};
		Assert::AreEqual(NO_ERROR, dtr::DetourTransactionCommit());
		Assert::AreEqual(std::string("DetourTransactionCommit"), help.getLine());
		Assert::AreEqual(std::string(), help.getLine());
	}
}
	
TEST_METHOD(test_error) {
	{ // �k���R���X�g���N�^�͐���������ł��邩�H
		dtr::error err;
		Assert::AreEqual(std::string(), err.getFunctionName());
		Assert::AreEqual(std::string(), err.getMessage());
		Assert::AreEqual(0, int(err.getNumber()));
	}
	{ // �R���X�g���N�^(std::string,LONG)�͐���������ł��邩�H
		test_helper_t help;
		help.setSeqBase();
		dtr::error err(help.getSeqStr(1), help.getSeq<LONG>(2));
		Assert::AreEqual(std::string("1"), err.getFunctionName());
		Assert::AreEqual(std::string("1�����s���܂����B(2)"), err.getMessage());
		Assert::AreEqual(2, int(err.getNumber()));
	}
}

};

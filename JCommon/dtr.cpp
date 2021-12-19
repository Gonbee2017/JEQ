//// JEQ/���ʏ������C�u����/Detours�֐��C���|�[�g

//// �C���N���[�h

#include "dtr.hpp"
#include "jcommon.hpp"

#include <windows.h>
#include <detours.h>

namespace jeq::dtr {

//// �N���X�����o�̒�`

// �W���֐��̃G���[���\�z����B
// �G���[���b�Z�[�W���쐬����B
error::error(
	const std::string &func_name, // �֐����B
	LONG number                   // �G���[�i���o�[�B
) : func_name(func_name), 
	number(number) 
{
	msg = string_printf("%s�����s���܂����B(%d)", func_name.c_str(), number);
}

// �֐������擾����B
const std::string & // �擾�����֐����B
error::getFunctionName() const {
	return func_name;
}

// �G���[�i���o�[���擾����B
LONG // �擾�����G���[�i���o�[�B
error::getNumber() const {
	return number;
}

//// �C���|�[�g�̒�`

DEF_IMP_DEF(DetourAttach) = [] (PVOID *ppPointer, PVOID pDetour) -> LONG {
	return ::DetourAttach(ppPointer, pDetour);
};

DEF_IMP_WRAP(DetourAttach, LONG, PVOID *ppPointer, PVOID pDetour) {
	LONG ret = DetourAttach_true(ppPointer, pDetour);
	if (ret != NO_ERROR) throw error(__FUNCTION__, ret);
	return ret;
}

DEF_IMP(DetourDetach) = [] (PVOID *ppPointer, PVOID pDetour) -> LONG {
	return ::DetourDetach(ppPointer, pDetour);
};

DEF_IMP_EASY(DetourTransactionAbort);

DEF_IMP_WRAP_EASY(DetourTransactionBegin, LONG) {
	LONG ret = DetourTransactionBegin_true();
	if (ret != NO_ERROR) throw error(__FUNCTION__, ret);
	return ret;
}

DEF_IMP_WRAP_EASY(DetourTransactionCommit, LONG) {
	LONG ret = DetourTransactionCommit_true();
	if (ret != NO_ERROR) throw error(__FUNCTION__, ret);
	return ret;
}

}

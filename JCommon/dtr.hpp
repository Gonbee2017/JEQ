//// JEQ/���ʏ������C�u����/Detours�֐��C���|�[�g

// ���̃v���W�F�N�g�Ŏg�p���Ă���Detours�֐����C���|�[�g����B
// Detours�֐��͎��s�R�[�h�Ȃǂ̃��\�[�X�ɕK���A�N�Z�X����̂ŁA
// ���ׂĂ̊֐����C���|�[�g�̑ΏۂɂȂ�B

//// Detour�Ƃ́H

// �uDetour�v�̊T�O�ɂ��ĊȒP�ɐ�������B
// Detour�͂��̖��̒ʂ�A�֐��Ăяo�����t�b�N���邽�߂́u�I��H�v�ł���B
// �֐���Detour�����Ƃ��́A�ΏۂƂȂ�Target�֐�(T�֐�)�ƉI��H�ƂȂ�
// Detour�֐�(D�֐�)�ƌ��̊֐����ĂԂ��߂�Trampoline�֐�(t�֐�)��p�ӂ���B

// T�֐���D�֐���Attach����ƁA�܂��uT�֐��̐擪�����v���A�uD�֐��ւ�
// �W�����v���߁v�ɏ�����������B�����āuT�֐��̐擪�����v��t�֐��̐擪��
// �ڂ���A�����āuT�֐��̐擪����̃A�h���X�v�ւ̃W�����v���߂��ǉ������B

// Attach���ꂽT�֐����Ăяo�����ƁA�����Ă�����D�֐��փW�����v���邽�߁A
// ��������D�֐������ڌĂяo���ꂽ���̂悤�Ɍ�����B
// ����At�֐����Ăяo�����ƁA�ŏ��ɁuT�֐��̐擪�����v�����s����A������
// �uT�֐��̐擪����v�փW�����v���邽�߁AT�֐����Ăяo���ꂽ�悤�Ɍ�����B

// Attach��̊e�֐��̊T�����ȉ��Ɏ����B
// �ET�֐�
//   �@D�֐��̇@�փW�����v
//   �A���炩�̏���
//   �B���^�[��
// �ED�֐�
//   �@���炩�̏���
//   �At�֐����Ăяo��
//   �B���炩�̏���
//   �C���^�[��
// �Et�֐�
//   �@������������O��T�֐��̇@
//   �AT�֐��̇A�փW�����v

// ���̏�Ԃ�T�֐����Ăяo�����Ƃ��̎��s�̗�����ȉ��Ɏ����B
// ��T�֐��̇@
// ��D�֐��̇@�A
//   ��t�֐��̇@�A
//   ��T�֐��̇A�B
// ��D�֐��̇B�C
// ���̂悤��D�֐���T�֐��̉I��H�Ƃ��āAT�֐��̑O��ɏ�����ǉ�������A
// T�֐����̂����s���邩�ǂ����𐧌䂵����ł���悤�ɂȂ�B

// �Ȃ�Detour�͈�̊֐��ɑ΂��ĉ��ł���邱�Ƃ��ł���B
// �Ⴆ�ΑO�q��T�֐��ɑ΂��Ă����D'�֐���Attach�����Ƃ���B
// Attach(����)��̊e�֐��̊T�����ȉ��Ɏ����B
// �ET�֐�
//   �@D'�֐��̇@�փW�����v
//   �A���炩�̏���
//   �B���^�[��
// �ED'�֐�
//   �@���炩�̏���
//   �At'�֐����Ăяo��
//   �B���炩�̏���
//   �C���^�[��
// �Et'�֐�
//   �@D�֐��̇@�փW�����v
//   �AT�֐��̇A�փW�����v
// �ED�֐�
//   �@���炩�̏���
//   �At�֐����Ăяo��
//   �B���炩�̏���
//   �C���^�[��
// �Et�֐�
//   �@������������O��T�֐��̇@
//   �AT�֐��̇A�փW�����v

// ���̏�Ԃ�T�֐����Ăяo�����Ƃ��̎��s�̗�����ȉ��Ɏ����B
// ��T �֐��̇@
// ��D'�֐��̇@�A
//   ��t'�֐��̇@
//   ��D �֐��̇@�A
//     ��t �֐��̇@�A
//     ��T �֐��̇A�B
//   ��D �֐��̇B�C
// ��D'�֐��̇B�C
// ���̂悤��D'�֐���T�֐��ł͂Ȃ�D�֐��̉I��H�Ƃ��ċ@�\����B
// ������Detour�̓`�F�[�����`�����A����ɒǉ����ꂽ�ق�����A
// ����ɒǉ����ꂽ�ق��ւƏ��ԂɎ��s����邱�ƂɂȂ�B
// �������r����Trampoline�֐����Ăяo���Ȃ���ΘA���͓r�؂��B

#ifndef JEQ_DTR_HPP
#define JEQ_DTR_HPP

//// �C���N���[�h

#include "error.hpp"
#include "import.hpp"

#include <windows.h>

#include <string>

namespace jeq { namespace dtr {

//// �^�̒�`

// Detours�֐��̃G���[��\���B
// �֐����ƃG���[�i���o�[��ێ�����B
class error_t : public jeq::error_t {
public:
	error_t() = default;
	error_t(const std::string &func_name, LONG number);
	LONG getNumber() const;
	const std::string &getFunctionName() const;
protected:
	std::string func_name; // �֐����B
	LONG number = 0;       // �G���[�i���o�[�B0(NO_ERROR)�ŏ������B
};

//// �C���|�[�g�̐錾

DECL_IMP_WRAP(DetourAttach, LONG, PVOID *ppPointer, PVOID pDetour);
DECL_IMP(DetourDetach, LONG, PVOID *ppPointer, PVOID pDetour);
DECL_IMP(DetourTransactionAbort, LONG);
DECL_IMP_WRAP(DetourTransactionBegin, LONG);
DECL_IMP_WRAP(DetourTransactionCommit, LONG);

}}

#endif // #ifndef JEQ_API_HPP

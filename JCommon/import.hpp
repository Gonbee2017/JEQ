//// JEQ/���ʏ������C�u����/�C���|�[�g�w�b�_

//// �C���|�[�g�Ƃ́H

// ���\�[�X�ɃA�N�Z�X����֐����Ăяo���֐��͒P�̃e�X�g�����ɂ����B
// �Ⴆ��std::getenv�͊��ϐ��Ƃ������\�[�X�ɃA�N�Z�X����̂ŁA
// std::getenv���Ăяo���֐���P�̃e�X�g���悤�Ƃ���ƁA
// �ΏۂƂȂ���ϐ������O�ɐݒ肵�Ă����K�v������B
// �܂���GetWindowText�Ȃ�E�C���h�E���쐬���Ă����Ȃ���΂Ȃ�Ȃ��B
// ���̂悤�ȏ����A�A�N�Z�X�A��n�����J��Ԃ��s���̂͌����������̂ŁA
// ���\�[�X�ɃA�N�Z�X����֐��ɂ��Ă͒��ɌĂяo������
// std::function�o�R�ŊԐړI�ɌĂяo���悤�ɂ���B
// �֐��|�C���^�o�R�ł͂Ȃ�std::function�o�R�ɂ���̂́A
// �P�̃e�X�g�Ŗ{���̊֐��̑���ɌĂяo����邱�ƂɂȂ�
// ���b�N�֐��������_�Ƃ��Ē�`�ł���悤�ɂ��邽�߂ł���B

// �܂����C�u�����֐��̃G���[�n���h�����O�͔ώG�ɂȂ�₷���B
// �Ⴆ��GetWindowText�͈����ɕs���ȃE�C���h�E�n���h����n������
// �G���[��Ԃ��B���̂悤�ȃG���[���Ăяo�����ƂɃ`�F�b�N����ƁA
// �\�[�X�R�[�h�������Ȃ�A���ʂ��������Ȃ��Ă��܂��B
// �G���[��Ԃ����C�u�����֐��ɂ��ẮA��p�̃��b�p�[���`����悤
// �ɂ��āA�����ŃG���[���`�F�b�N���A�G���[�Ȃ��O���X���[����B
// �P�̃e�X�g���₷���悤�ɖ{���̊֐���std::function�o�R�ŌĂяo���B

// �O�q��2�̗��R�ɂ��Astd::function����ă��C�u�����֐���
// �ԐړI�ɌĂяo���悤�ɂ��邱�Ƃ��u�C���|�[�g�v�ƌĂԂ��Ƃɂ���B

// ���������C�u�����֐��ł����Ă��A���\�[�X�ɃA�N�Z�X�����A
// �Ȃ����G���[���`�F�b�N����K�v�̂Ȃ��֐��ł���΁A
// �C���|�[�g�̑Ώۂɂ͂����A���ɌĂяo���悤�ɂ���B
// �Ⴆ��std::strchr��std::string�Ȃǂ�����ɊY������B

// ���̃w�b�_�ł̓C���|�[�g���Ȍ��ɏ������߂̃}�N�����`���Ă���B
// ���ۂ̃C���|�[�g�͓���̃��C�u�������Ƃɍs���B

#ifndef JEQ_IMPORT_HPP
#define JEQ_IMPORT_HPP

//// �C���N���[�h

#include <functional>

// �C���|�[�g��錾����B
/* �T���v���R�[�h
DECL_IMP(GetKeyState, SHORT, int nVirtKey);
*/
#define DECL_IMP(\
	name,     /* �֐����B */\
	ret_type, /* �߂�l�̌^�B */\
	...       /* ���ׂĂ̈����̌^�Ɩ��O�B */\
) \
	using name ## _t = std::function<ret_type(__VA_ARGS__)>;\
	extern name ## _t name;

// �C���|�[�g��錾����B
// �G���[�`�F�b�N���s�����b�p�[�Ƃ��Đ錾����B
/* �T���v���R�[�h
DECL_IMP_WRAP(SetWindowText, BOOL, HWND hWnd, LPCSTR lpString);
*/
#define DECL_IMP_WRAP(\
	name,     /* �֐����B */\
	ret_type, /* �߂�l�̌^�B */\
	...       /* ���ׂĂ̈����̌^�Ɩ��O�B */\
) \
	using name ## _t = std::function<ret_type(__VA_ARGS__)>;\
	extern name ## _t name ## _true;\
	ret_type name(__VA_ARGS__);

// �C���|�[�g���`����B
// ���ӂ݂̂��`����̂ŁA�����ď��������邱�ƁB
/* �T���v���R�[�h
DEF_IMP(chrono_system_clock_now) = std::chrono::system_clock::now;
*/
#define DEF_IMP(\
	name  /* �֐����B */\
) \
	name ## _t name

// �C���|�[�g���`����B
// ��於�O��Ԃ̓��ꖼ�֐��ŏ���������B
/* �T���v���R�[�h
DEF_IMP_EASY(SendMessage);
*/
#define DEF_IMP_EASY(\
	name  /* �֐����B */\
) \
	name ## _t name = ::name;

// �C���|�[�g���`����B
// ���b�p�[�֐����Ăяo���{���̊֐����`����B
/* �T���v���R�[�h
DEF_IMP_TRUE(filesystem_equivalent) = [](
	const std::filesystem::path &p1, 
	const std::filesystem::path &p2
) -> bool {
	return std::filesystem::equivalent(p1, p2);
};
*/
#define DEF_IMP_TRUE(name) \
	name ## _t name ## _true

// �C���|�[�g���`����B
// ���b�p�[�֐��̉������܂ł��`����̂ŁA�����Ė{�̂��`���邱�ƁB
/* �T���v���R�[�h
DEF_IMP_WRAP(
	filesystem_equivalent,
	bool,
	const std::filesystem::path &p1, 
	const std::filesystem::path &p2
) {
	try {
		return filesystem_equivalent_true(p1, p2);
	} catch (const std::system_error &err) {
		throw error(__FUNCTION__, err.code().value());
	}
}
*/
#define DEF_IMP_WRAP(\
	name,     /* �֐����B */\
	ret_type, /* �߂�l�̌^�B */\
	...       /* ���ׂĂ̈����̌^�Ɩ��O�B */\
) \
	ret_type name(__VA_ARGS__)

// �C���|�[�g���`����B
// ���b�p�[�֐����Ăяo���{���̊֐����於�O��Ԃ̓��ꖼ�֐��ŏ���������B
// ���b�p�[�֐��̉������܂ł��`����̂ŁA�����Ė{�̂��`���邱�ƁB
/* �T���v���R�[�h
DEF_IMP_WRAP_EASY(SetActiveWindow, HWND, HWND hWnd) {
	HWND ret = SetActiveWindow_true(hWnd);
	if (!ret) throw error(__FUNCTION__);
	return ret;
}
*/
#define DEF_IMP_WRAP_EASY(\
	name,     /* �֐����B */\
	ret_type, /* �߂�l�̌^�B */\
	...       /* ���ׂĂ̈����̌^�Ɩ��O�B */\
) \
	name ## _t name ## _true = ::name;\
	ret_type name(__VA_ARGS__)

#endif // #ifndef JEQ_IMPORT_HPP

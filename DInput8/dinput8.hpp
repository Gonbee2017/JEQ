//// JEQ/�v���O�C�����[�_�[/�{��

//// eqgame.exe���g��

// eqgame.exe�͂�������DLL���_�C�i�~�b�N�����N���Ă��邪�A
// ���̒��̈��dinput8.dll������Aeqgame.exe��dinput8.dll��
// DirectInput8Create�֐��݂̂��C���|�[�g���Ă���B

// ���̂���dinput8.dll�Ɠ������O��DLL�����ADirectInput8Create��
// �������O�̊֐����G�N�X�|�[�g����΁A�ȒP��dinput8.dll���U���ł���B
// eqgame.exe�ɂ͋U����dinput8.dll�����[�h�����A�U����dinput8.dll��
// ���߂Ė{����dinput8.dll�����[�h���A�U����DirectInput8Create����
// �{����DirectInput8Create���Ăяo���΁A�{����dinput8.dll�̋@�\��
// �񋟂��Aeqgame.exe�v���Z�X���ŔC�ӂ̃R�[�h�����s�ł���B

//// �v���O�C���Ƃ́H

// dinput8.dll���U�����邱�ƂƁAeqgame.exe�v���Z�X����
// �C�ӂ̃R�[�h�����s���邱�Ƃ͂܂������Ⴄ�@�\�ł���B
// �����ŋU���ȊO�̋@�\�͋U����dinput8.dll�Ƃ͕ʂ�DLL�Ɏ������A
// �U����dinput8.dll�ɂ͂�����DLL�����[�h�����邱�Ƃɂ���B
// ���̔C�ӂ̋@�\������DLL�̂��Ƃ��u�v���O�C���v�ƌĂсA�U����
// dinput8.dll�̂��Ƃ��u�v���O�C�����[�_�[�v�ƌĂԂ��Ƃɂ���B

// ���̃v���W�F�N�g�̃v���O�C����MQ2�̃v���O�C���Ƃ͈قȂ�A
// �G�N�X�|�[�g���Ȃ���΂Ȃ�Ȃ��֐��̋K��͑��݂��Ȃ��B
// �v���O�C���Ƃ��ĕK�v�Ȃ̂�DllMain�֐����������邱�Ƃ����ł���B
// ���������̃v���O�C����eqgame.exe�ɂ���֐���Detour�����
// ���낤����Aeqgame.exe�͂������A���̃v���O�C����MQ2��
// ���܂�e����^���Ȃ��悤�ɒ��ӂ��Ȃ���΂Ȃ�Ȃ��B
// �Ⴆ��ProcessGameEvents�֐���Detour�����ꍇ�A�ł������
// Trampoline�֐����Ăяo���āA�`�F�[���̎��s���Ȃ��łق����B

//// ���s���@

// �v���O�C�����[�_�[�����s�����܂ł̎菇���ȉ��Ɏ����B
// �@eqgame.exe�Ɠ����t�H���_�Ɉȉ��̃t�@�C����z�u����B
// �Edinput8.dll �c �v���O�C�����[�_�[
// �Edinput8.ini �c �v���O�C�����[�_�[�̐ݒ�
// �Aeqgame.exe���N������ƁAWindows�ɂ���Ė{����dinput8.dll�̑����
//   �@�Ŕz�u�����U����dinput8.dll�����[�h����A���s�����B

// �v���O�C���Ƃ��ă��[�h����DLL��dinput8.ini�t�@�C���Őݒ肷��B
// ���[�h�ł���v���O�C���̐��ɐ����͂Ȃ��A���ł����[�h�ł���B
// �܂��G���[���O��dinput8.log�ɏ������܂��B

#ifndef JEQ_DINPUT8_HPP
#define JEQ_DINPUT8_HPP

//// �C���N���[�h

#include "common.hpp"

#include <windows.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

//// �G�N�X�|�[�g�֐��̐錾

extern "C" __declspec(dllexport) HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

namespace jeq {

//// �^�̒�`

// DirectInput8Create�֐��̌^�B
using DirectInput8Create_t = HRESULT (*)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);

// �v���O�����̕���(��)��\���B
// ���ׂẴO���[�o���ϐ����܂Ƃ߂��\���̂ł���B
// �܂Ƃ߂邱�ƂŒP�̃e�X�g�ɂ����ĊȒP�Ƀ��Z�b�g�ł���B
struct context_t {
	DirectInput8Create_t 
		DirectInput8Create_true = nullptr; // �{����DirectInput8Create�֐��̃|�C���^�B
	std::string dll_name;                  // ����DLL�̖��O�B"dinput8.dll"�B
	std::vector<scope_exit_t> exits;       // �E�o�����̃x�N�^�B
	HINSTANCE fake_dll_handle = nullptr;   // �U����dinput8.dll(����DLL)�̃n���h���B
	std::filesystem::path fake_dll_path;   // �U����dinput8.dll(����DLL)�̃p�X�B
	std::filesystem::path ini_path;        // dinput8.ini�t�@�C���̃p�X�B
	std::shared_ptr<std::ostream> log;     // dinput8.log�̏o�̓X�g���[���B
	std::filesystem::path log_path;        // dinput8.log�̃p�X�B
	std::vector<HMODULE> plugin_handles;   // �v���O�C���̃n���h���̃x�N�^�B
	HMODULE true_dll_handle = nullptr;     // �{����dinput8.dll�̃n���h���B
	std::filesystem::path true_dll_path;   // �{����dinput8.dll�̃p�X�B

	void startup(HINSTANCE hinstDLL);
};

//// �O���[�o���ϐ��̐錾

extern context_t context;

//// �֐��̐錾

void loadPlugin(const std::string &plugin_name);
void loadPlugins();
void loadTrueDLL();
BOOL onProcessAttach(HINSTANCE hinstDLL);
BOOL onProcessDetach();
std::filesystem::path searchTrueDLL();

}

#endif // #ifndef JEQ_DINPUT8_HPP

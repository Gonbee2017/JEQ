//// JEQ/���{��`���b�g�v���O�C��/�{��

// �����̃v���O�C���̓v���O�C�����[�_�[�ł���dinput8.dll�ɂ����
// eqgame.exe�v���Z�X�Ƀ��[�h����A�g�p����邱�Ƃ�z�肵�Ă���B

//// �J���������@
// �p��ł�EQ�N���C�A���g�ł͓��{��ł̂悤�ɓ��{�����͂ł��Ȃ��B
// �`���b�g�̓��͗��őS�p�L�[�������Ă����{����̓��[�h�ɂȂ�Ȃ��B
// �p��łœ��{�����͂ł���悤�ɂ���ɂ́AMQ2�̂悤��eqgame.exe
// ���g�����āA�v���Z�X�̓����ɋ@�\��ǉ����Ȃ���΂Ȃ�Ȃ������B

//// ���͕��@��͍�

// ���z�Ƃ��Ă�EQ�̃`���b�g�E�C���h�E�œ��{�����͂ł���悤��
// �������������A���̂��߂ɂ�eqgame.exe�̋@�B������Ȃ�[���܂�
// ��͂���K�v������A�����̃X�L���ł͓���������ߒf�O�����B

// �Ë��ĂƂ���EQ�̃`���b�g�E�C���h�E�Ƃ͕ʂɁA�����ȃE�C���h�E��
// �V���ɕ\�����āA�����ɓ��{�����͂ł���悤�ɂ��悤�Ƃ����B
// ���̃E�C���h�E���u���{��`���b�g�o�[�v�ƌĂԂ��Ƃɂ���B
// �������E�C���h�E��\�����邱�Ƃ͂ł������̂́A���̏�ɔz�u����
// ���͗�(EDIT�R���g���[��)���L�[�{�[�h����̓��͂ɔ������Ȃ������B
// ���̌�����WM_CHAR���b�Z�[�W�����M����Ă��Ȃ����炾�����B
// WM_CHAR���b�Z�[�W��TranslateMessage�֐��ɂ���đ��M�����B
// eqgame.exe�̃��b�Z�[�W���[�v�ł͂���TranslateMessage���Ăяo����
// ���Ȃ��炵���A����䂦WM_CHAR���b�Z�[�W�����M����Ă��Ȃ������B

// ������������邽�߂�DispatchMessage�֐���Detour����邱�Ƃɂ����B
// ���ʂ̃��b�Z�[�W���[�v�ł�GetMessage�֐��Ń��b�Z�[�W���擾������A
// TranslateMessage��DispatchMessage�𗧂đ����ɌĂяo���B
// ����Ɠ������Ƃ����邽�߂ɁADispatchMessage��Detour�֐��̒���
// TranslateMessage��DispatchMessage��Target�֐��ƌĂяo���悤��
// ������A���͗����L�[�{�[�h����̓��͂ɔ�������悤�ɂȂ����B
// ���������{����̓��[�h�̂܂܂��ƁA���C���E�C���h�E�ɖ߂�A
// �L�[�{�[�h���牽�����͂���ƁA���̓��͂�IME���������Ă��܂��A
// �ϊ��E�C���h�E���\������A��ʂ��B��ăv���C�̎ז��ɂȂ����B
// ������DispatchMessage��Detour�֐��ł͓��{��`���b�g�o�[��
// �A�N�e�B�u�ȂƂ��ɂ���TranslateMessage���Ăяo�����Ƃɂ����B

//// �A�C�e�������N��͍�

// ���ɑ傫�ȉۑ�ɂȂ����̂��A�C�e���̃����N�ł���B
// EQ�N���C�A���g�ŃC���x���g����\�����A�X���b�g�ɂ���A�C�e����
// �E�N���b�N����������΁A���̃A�C�e���̃E�C���h�E��\���ł���B
// �����ăE�C���h�E��ɂ���A�C�e���̃A�C�R�����N���b�N����΁A
// �`���b�g�̓��͗��ɂ��̃A�C�e���̃����N��\��t���邱�Ƃ��ł���B
// �����N�͓���ȕ�����Ƃ��Ĉ����A�s���N�̕����F�ŕ\�������B
// ���̃����N����{��`���b�g�̂ق��ł�������悤�ɂ��悤�ƍl�����B

// ���z�Ƃ��Ă̓A�C�e���̃E�C���h�E�ŃA�C�R�����N���b�N���ꂽ��A
// ���̃C�x���g���C���^�[�Z�v�g���āA�����N��EQ�`���b�g�ł͂Ȃ��A
// ���{��`���b�g�̂ق��ɕ\������悤�ɂ����������B
// ������MQ2�̃\�[�X�R�[�h�����n���Ă����̃C�x���g�Ɋ֌W����
// �ӏ��͌����炸�A���͂ł�exe��͂�����������ߒf�O�����B

// �Ë��ĂƂ��ăA�C�e���̃����N��EQ�`���b�g����]�����邱�Ƃɂ����B
// �܂������N����������EQ�`���b�g�̓��͗��ɓ\��t���Ă��炤�B
// ������Enter�L�[�������ƁA�ʏ�̓����N���܂ޔ��������邱�ƂɂȂ�B
// ���̃v���O�C���͂��������肵�āA�����Ɋ܂܂�郊���N����͂��A
// ���̂悤�ȓƎ��̌`���ɕϊ����Ă�����{��`���b�g�ւƓ]������B
// "{�ԍ�:���O}"
// �����N��'{'��'}'�ň͂݁A�ԍ��ƃA�C�e���̖��O�𖄂ߍ��ށB
// �Ⴆ��Ration��Water Flask�̃����N��EQ�`���b�g������{��`���b�g��
// �]������ƁA���ꂼ��{0:Ration}��{1:Water Flask}�̂悤�ɕ\�������B
// ���̌�A���{��`���b�g��Enter�L�[�������ƁA���x�͋t�ɓƎ��̌`������
// ���̌`���ɖ߂���āA�`���b�g�Ƀ����N�𔭌�����A�Ƃ�������ɂȂ�B
// ����ŏ�����Ԃ����A���{��ƈꏏ�Ƀ����N�𔭌��ł���悤�ɂȂ����B

//// ���s���@

// ���{��`���b�g�o�[���\�������܂ł̎菇���ȉ��Ɏ����B
// �@eqgame.exe�Ɠ����t�H���_�Ɉȉ��̃t�@�C����z�u����B
// �Edinput8.dll �c �v���O�C�����[�_�[
// �Edinput8.ini �c �v���O�C�����[�_�[�̐ݒ�
// �Ejchat.dll   �c ���{��`���b�g�v���O�C��
// �Ejchat.ini   �c ���{��`���b�g�v���O�C���̐ݒ�
// �Aeqgame.exe���N������ƁAWindows�ɂ���Ė{����dinput8.dll�̑����
//   �@�Ŕz�u�����U����dinput8.dll�����[�h����A���s�����B
//   ����ɋU����dinput8.dll�ɂ����jchat.dll�����[�h����A���s�����B
// �B�T�[�o�[��I�����A�ڑ�����B
// �C�L�����N�^�[��I�����A���O�C������B
// �D�]�[����POP����ƃ`���b�g�o�[���\�������B

// ������jchat.ini�ɂ��Ă�2��ނ̃o�[�W�������p�ӂ���Ă���A
// �N���C�A���g�ɂ����RoF2������Titanium�����ɕ�����Ă���B

// jchat.ini�ɂ͈ȉ��̐ݒ肪�ۑ�����Ă���B
// �E�E�C���h�E�̈ʒu�ƕ�
// �E�t�H���g�̖��O
// �E�A�C�e�������N�̃T�C�Y
// �Eeqgame.exe�ɂ���f�[�^��֐��̃I�t�Z�b�g
// �E�悭�g���������X�g

// jchat.ini�̐ݒ�́A�v���O�����̃f�[�^��ۑ�����ړI�̂��̂������A
// ���[�U�[�����ڕύX�ł���̂̓t�H���g�̖��O���炢�����Ȃ��B
// �܂��G���[���O��jchat.log�ɏ������܂��B

//// �`���b�g�̋@�\

// ���{��`���b�g�o�[�͍��[��"��"�̃A�C�R�����h���b�O����Έړ��ł��A
// �E�[�̂܂݂��h���b�O����Ε��𒲐��ł���B
// �܂���Ƀ��C���E�C���h�E�̏�ɕ\������邽�߁A�d�˂Ă����ɉB��Ȃ��B
// �E�C���h�E�̈ʒu�ƕ��̓N���C�A���g�I������jchat.ini�ɕۑ�����A
// ����N�����ɓǂݍ��܂��̂ŁA���񒲐�����K�v�͂Ȃ��B

// ���C���E�C���h�E��Enter�L�[�������ƁA�ʏ��EQ�`���b�g�����͂ł���
// ��ԂɂȂ邪�A���̃v���O�C�������[�h����Ă���ƁA���̃C�x���g��
// �C���^�[�Z�v�g���āA���{��`���b�g����͂ł����Ԃɂ���B
// ���l�ɃX���b�V���L�[���C���^�[�Z�v�g����B

// EQ�`���b�g�Ŕ������悤�Ƃ���ƁA�������悤�Ƃ����e�L�X�g��
// ���̂܂ܓ��{��`���b�g�ɓ]������A�������̂��L�����Z�������B
// �����̒��ɃA�C�e���̃����N���܂܂�Ă���΁A�Ǝ��̌`����
// �ϊ�����Ă�����{��`���b�g�ɕ\�������B

// ���͗��ł̓������ȂǂƓ����悤�Ƀe�L�X�g����́E�ҏW�ł���B
// ���{��̓��͂͂������A�R�s�[�E�J�b�g�E�y�[�X�g�Ȃǂ��ł���B
// �e�L�X�g����͂�����AEQ�`���b�g�Ɠ������AEnter�L�[�Ŕ����ł���B
// /loc��/target�̂悤�ȃR�}���h�����ׂē����悤�Ɏ��s�ł���B

// �ǂ̃`�����l���Ŕ������邩�̓`�����l�����̃R���{�{�b�N�X�Ŏw�肷��B
// �I���ł���`�����l�����ȉ��Ɏ����B
// �E/Auction
// �E/Gsay
// �E/gUildsay
// �E/Ooc
// �E/Reply
// �E/Say
// �E/sHout
// ���͓��e���R�}���h�łȂ���΁A�I�𒆂̃`�����l�����t�������B
// �Ⴆ��"/Say"��I�𒆂�"Hoge"�Ɠ��͂���Enter�L�[�������ƁA
// "Hoge"�̑O��"/Say "���t������A"/Say Hoge"�����s�����B

// �`�����l����Alt+�啶���̃V���[�g�J�b�g�L�[�őI���ł���B
// �Ⴆ��Alt+G�L�[��������"/Gsay"�ɐ؂�ւ�����B

// ���͓��e�̍ŏ��̕�����"/#^"�̂����ꂩ�ł���΃R�}���h�Ƃ݂Ȃ����B
// �Ⴆ��"/Gsay"��I�𒆂�"/tell foo bar"�Ɠ��͂���Enter�L�[�������ƁA
// �R�}���h�Ɣ��肳���"/tell foo bar"�����̂܂܎��s�����B

// ��������ƁA���̃e�L�X�g���u���܂ł̔������X�g�v�ɒǉ������B
// ���L�[�������΍��܂ł̔������X�g�����ǂ邱�Ƃ��ł���B
// ���L�[�������Δ��Ε����ɖ߂邱�Ƃ��ł���B

// �e�L�X�g����͂�����AEnter�L�[�ł͂Ȃ�Shift+Enter�L�[�������΁A
// ���͗��̃e�L�X�g���u�悭�g���������X�g�v�ɓo�^�ł���B
// Shift+���L�[�������΂悭�g���������X�g�����ǂ邱�Ƃ��ł���B
// Shift+���L�[�������Δ��Ε����ɖ߂邱�Ƃ��ł���B
// �悭�g���������X�g�̓N���C�A���g�I������jchat.ini�ɕۑ�����A
// ����N�����ɓǂݍ��܂��̂ŁA����o�^����K�v�͂Ȃ��B

// ���܂ł̔������X�g��悭�g���������X�g�����ǂ��Ă���Ƃ���
// Shift+Delete�L�[�������ΑI�𒆂̔������폜�ł���B
// �܂�Esc�L�[�������΃`���b�g�̓��͂���������ۗ����āA
// ���C���E�C���h�E�̑���ɖ߂邱�Ƃ��ł���B

#ifndef JEQ_JCHAT_HPP
#define JEQ_JCHAT_HPP

//// �C���N���[�h

#include "eqgamedef.hpp"
#include "jcommon.hpp"

#include <windows.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jeq {

//// �萔�̒�`

// �`�����l�����̒萔�B
constexpr std::size_t CHANNEL_HEIGHT = 100; // �����B
constexpr int CHANNEL_ID = 1;               // ID�B
constexpr int CHANNEL_TOP = 2;              // ��ӂ�Y���W�B
constexpr std::size_t CHANNEL_WIDTH = 74;   // ���B
constexpr COLORREF CHANNEL_BK_COLOR =       // �w�i�F�B
	RGB(14, 16, 22);

// ���͗��̒萔�B
constexpr std::size_t EDIT_HEIGHT = 12;      // �����B
constexpr COLORREF EDIT_INNER_SUNKEN_COLOR = // �����̘g���̉E�ӁE���ӂ̐F�B
	RGB(0, 0, 0);
constexpr COLORREF EDIT_INNER_RAISED_COLOR = // �����̘g���̍��ӁE��ӂ̐F�B
	RGB(0, 0, 0);
constexpr int EDIT_ID = 2;                   // ID�B
constexpr COLORREF EDIT_OUTER_SUNKEN_COLOR = // �O���̘g���̉E�ӁE���ӂ̐F�B
	RGB(132, 150, 181);
constexpr COLORREF EDIT_OUTER_RAISED_COLOR = // �O���̘g���̍��ӁE��ӂ̐F�B
	RGB(74, 93, 115);
constexpr int EDIT_TOP = 2;                  // ��ӂ�Y���W�B

// EQ�`���b�g�̃����N�̒萔�B
constexpr char EQCHAT_LINK_EDGE = 0x12; // ���[�̋L���B

// jchat.ini�̒萔�B
constexpr char INI_CHAT_COMMAND_SYMBOLS_DEF[] = "/#^";  // �R�}���h�L���̗�̊���l�B
constexpr char INI_CHAT_FONT_NAME_DEF[] =               // �t�H���g�̖��O�̊���l�B
	"�l�r �o�S�V�b�N";
constexpr std::size_t INI_CHAT_LINK_BODY_SIZE_DEF = 56; // �����N�̃{�f�B�����̃T�C�Y�̊���l�B
constexpr int INI_WINDOW_LEFT_DEF = -10000;             // �E�C���h�E�̍��ӂ�X���W�̊���l�B
constexpr std::size_t INI_WINDOW_MIN_WIDTH_DEF = 150;   // �E�C���h�E�̍ŏ����̊���l�B
constexpr int INI_WINDOW_TOP_DEF = -10000;              // �E�C���h�E�̏�ӂ�Y���W�̊���l�B
constexpr std::size_t INI_WINDOW_WIDTH_DEF = 300;       // �E�C���h�E�̕��̊���l�B

// ���{��`���b�g�̃����N�̒萔�B
constexpr char JCHAT_LINK_CLOSE = '}';   // �I�[�̋L���B
constexpr char JCHAT_LINK_DIVIDER = ':'; // �ԍ��Ɩ��O�𕪊�����L���B
constexpr char JCHAT_LINK_ESCAPE = '$';  // �Փˉ���̋L���B
constexpr char JCHAT_LINK_OPEN = '{';    // �n�[�̋L���B

// ���{��`���b�g�o�[�̒萔�B
constexpr COLORREF JCHAT_BAR_SUNKEN_COLOR =      // �g���̉E�ӁE���ӂ̐F�B
	RGB(49, 48, 41);
constexpr COLORREF JCHAT_BAR_RAISED_COLOR =      // �g���̍��ӁE��ӂ̐F�B
	RGB(165, 158, 115);
constexpr char JCHAT_BAR_CLASS_NAME[] = "JChat"; // �N���X�̖��O�B
constexpr std::size_t JCHAT_BAR_HEIGHT = 24;     // �����B
constexpr char JCHAT_BAR_NAME[] = "JChat";       // �^�C�g���B

// ���̑��̒萔�B
constexpr std::size_t FONT_SIZE = 12;   // �t�H���g�̃T�C�Y�B
constexpr std::size_t ICON_WIDTH = 24;  // �A�C�R���̕��B
constexpr std::size_t KNOB_WIDTH = 8;   // �܂݂̕��B
constexpr int EDIT_LEFT =               // ���͗��̍��ӂ�X���W�B
	ICON_WIDTH + CHANNEL_WIDTH;
constexpr std::size_t EDIT_HOR_MARGIN = // ���͗��̐��������̗]���B
	EDIT_LEFT + KNOB_WIDTH;
constexpr COLORREF TEXT_COLOR =         // �����F�B
	RGB(223, 223, 224);

// ���͗��̃e�L�X�g��ύX����Ƃ��̃��[�h��\���B
enum class change_modes : int {
	EDIT,   // �ҏW���邱�ƂŕύX����B
	CHOICE, // ���X�g����I�����邱�ƂŕύX����B
};

// ��ʃ��[�h��\���B
enum class screen_modes : int {
	WINDOW,      // �E�C���h�E�B
	FULL_SCREEN, // �t���X�N���[���B
	ICONIC,      // �A�C�R���B
};

//// �^�̒�`

// DispatchMessage�֐��̌^�B
using DispatchMessage_t = LRESULT (WINAPI*)(const MSG*);

// eqgame.exe�̊֐���f�[�^�̏W����\���B
// �N���X�̃����o�֐��̓C���X�^���X�ɃA�N�Z�X�ł���K�v������B
// �C���X�^���X�ւ̃A�N�Z�X��this�|�C���^����čs����B
// VC++�ɂ�����this�|�C���^��ECX���W�X�^�Ɋi�[����邽�߁A
// �����o�֐����ł�ECX���W�X�^��this�|�C���^�̂��߂����Ɏg�p����B
// ����ɑ΂���static�Ȋ֐���ECX���W�X�^�𑼂̖ړI�Ŏg�p����B
// ����static�Ȋ֐��������o�֐���Detour�ɂł��邾�낤���H
// Target�֐���������Ăяo���R�[�h��ECX���W�X�^��this�|�C���^��
// �i�[����Ă���O��őg�܂�Ă��邽�߁ADetour�֐������s�����ߒ���
// ECX���W�X�^��������������ƃv���O�������N���b�V�����Ă��܂��B
// ECX���W�X�^��ی삷��ɂ́ADetour�֐��������o�֐��ɂ���΂悢�B
// Detour�֐���������N���X�́ATarget�֐���������N���X�ƃ�������
// ���C�A�E�g����v���Ă���K�v�͂Ȃ��A����ECX���W�X�^�Ɋi�[����Ă���
// this�|�C���^�����������邱�ƂȂ����s�ł���Ƃ��������ŏ\���ł���B
// eqgame.exe�ɂ���֐���Detour�֐��͂��̃N���X�̃����o�Ƃ��Ē�`����B
// �܂�Target�֐��̃|�C���^�₢�����̃f�[�^�ւ̃|�C���^����`����B
class eqgame_t {
public:
	using CEverQuest_InterpretCmd_t =       // CEverQuest�N���X��InterpretCmd�����o�֐��̌^�B
		void (eqgame_t::*)(eqgame_t*,char*);
	using CEverQuest_SetGameState_t =       // CEverQuest�N���X��SetGameState�����o�֐��̌^�B
		void (eqgame_t::*)(int);
	using CXWndManager_DrawCursor_t =       // CEverQuest�N���X��DrawCursor�����o�֐��̌^�B
		int (eqgame_t::*)();
	using KeypressHandler_HandleKeyDown_t = // KeypressHandler�N���X��HandleKeyDown�����o�֐��̌^�B
		bool (eqgame_t::*)(const KeyCombo&);
	using KeypressHandler_HandleKeyUp_t =   // KeypressHandler�N���X��HandleKeyUp�����o�֐��̌^�B
		bool (eqgame_t::*)(const KeyCombo&);
	using ProcessGameEvents_t = BOOL (*)(); // ProcessGameEvents�֐��̌^�B

	static const std::unordered_set<DWORD> ACTIVATE_KEY_SITUATIONS;
	static const std::unordered_set<DWORD> IGNORE_KEY_SITUATIONS;

	static eqgame_t **CEverQuest;
	static eqgame_t **CharSpawn;
	static HWND *MainWindowHandle;
	static PBYTE NotInChatMode;

	static CEverQuest_InterpretCmd_t CEverQuest_InterpretCmd_target;
	static CEverQuest_SetGameState_t CEverQuest_SetGameState_target;
	static CXWndManager_DrawCursor_t CXWndManager_DrawCursor_target;
	static KeypressHandler_HandleKeyDown_t KeypressHandler_HandleKeyDown_target; 
	static KeypressHandler_HandleKeyUp_t KeypressHandler_HandleKeyUp_target;
	static ProcessGameEvents_t ProcessGameEvents_target;

	void CEverQuest_InterpretCmd_detour(eqgame_t *player, char *cmd);
	void CEverQuest_SetGameState_detour(int game_state);
	int CXWndManager_DrawCursor_detour();
	bool KeypressHandler_HandleKeyDown_detour(const KeyCombo &key_combo);
	bool KeypressHandler_HandleKeyUp_detour(const KeyCombo &key_combo);
	static BOOL ProcessGameEvents_detour();
};

// �s�̃��X�g�Ƃ��̔����q�̃Z�b�g��\���B
// ���܂ł̔������X�g�Ƃ悭�g���������X�g�̌^�Ƃ��Ďg�p����B
struct lines_and_iter_t {
	// �s�̃��X�g�̌^�B
	using lines_t = std::list<std::string>;

	lines_t lines;                        // �s�̃��X�g�B
	lines_t::iterator iter = lines.end(); // �s�̃��X�g�̔����q�B

	inline bool isBegin() const;
	inline bool isEnd() const;
	inline void moveBegin();
	inline void moveEnd();
};

// ���{��`���b�g�o�[��\���B
// ���C���E�C���h�E�Ƌ��ɁA��PC���X�|�[�����Ă���Ƃ��ɕ\�������B
// ��400�s�N�Z��(����l)�A����24�s�N�Z���̉��ɒ����_�̂悤�Ȍ`�����Ă���B
// �`���b�g�o�[�̏�ɂ̓`�����l�����Ɠ��͗����z�u����Ă���B
// EQ�`���b�g�Ɠ����悤�ɔ���������A�R�}���h�����s���邱�Ƃ��ł���B
class jchat_bar_t {
public:
	static void registerClass();

	void activate(const KeyCombo &key_combo);
	void create();
	void fixPos();
	HWND getHandle();
	void transferText(const char *eqc_text);
protected:
	using message_handler_t = // ���b�Z�[�W�n���h���̌^�B
		LRESULT (jchat_bar_t::*)(HWND,UINT,WPARAM,LPARAM);
	using message_map_t =     // ���b�Z�[�W�}�b�v�̌^�B
		std::unordered_map<
			UINT,             // ���b�Z�[�W�B
			message_handler_t // ���b�Z�[�W�n���h���B
		>;

	// �C���X�^���X�f�[�^��\���B
	struct data_t {
		HDC buffer_dc_handle = NULL;          // �w�i�o�b�t�@�̃f�o�C�X�R���e�L�X�g�̃n���h���B
		std::vector<scope_exit_t> exits;      // �E�o�����̃x�N�^�B
		HWND handle = NULL;                   // �`���b�g�o�[�̃n���h���B
		HFONT hfont = NULL;                   // �t�H���g�̃n���h���B
		lines_and_iter_t history;             // ���܂ł̔������X�g�B
		HDC icon_active_dc_handle = NULL;     // �A�N�e�B�u�A�C�R���̃f�o�C�X�R���e�L�X�g�̃n���h���B
		HDC icon_inactive_dc_handle = NULL;   // �C���A�N�e�B�u�A�C�R���̃f�o�C�X�R���e�L�X�g�̃n���h���B
		HDC knob_dc_handle = NULL;            // �܂݂̃f�o�C�X�R���e�L�X�g�̃n���h���B
		std::vector<std::string> link_bodies; // �A�C�e�������N�̃{�f�B�����̃x�N�^�B
		std::vector<HWND> ordered_handles;    // Tab���̃n���h���̃x�N�^�B
		std::unordered_map<HWND,int> 
			ordered_indices;                  // �n���h����Tab���̃C���f�b�N�X�̃}�b�v�B
		std::unordered_map<COLORREF,HPEN> 
			pen_cache;                        // �y���̃L���b�V���B
		lines_and_iter_t registry;            // �悭�g���������X�g�B
		HBRUSH rock_bar_brush_handle = NULL;  // ��_�u���V�̃n���h���B
		HBRUSH rock_wall_brush_handle = NULL; // ��ǃu���V�̃n���h���B
		POINT mw_client_pos;                  // ���C���E�C���h�E�̃N���C�A���g���W�n�ɂ�����ʒu�B
		struct {
			WNDPROC WindowProc_super = NULL; // �T�u�N���X���O�̃E�C���h�E�v���V�[�W���B
			HWND handle = NULL;              // �n���h���B
		} channel; // �`�����l�����̃f�[�^�B
		struct {
			WNDPROC WindowProc_super = NULL; // �T�u�N���X���O�̃E�C���h�E�v���V�[�W���B
			change_modes change_mode =       // �e�L�X�g��ύX����Ƃ��̃��[�h�B
				change_modes::EDIT;
			bool dirty = false;              // �X�V�t���O�B
			HWND handle = NULL;              // �n���h���B
			std::string last_line;           // �Ō�ɕҏW����Ă����s�B
		} edit; // ���͗��̃f�[�^�B

		~data_t();
	};

	static const std::unordered_map<BYTE,int> CHANNEL_INDICES;
	static const message_map_t CHANNEL_MESSAGE_MAP;
	static const std::array<std::string,7> CHANNEL_TEXTS;
	static const message_map_t EDIT_MESSAGE_MAP;
	static const message_map_t MESSAGE_MAP;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK channel_WindowProc_sub(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK edit_WindowProc_sub(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	std::shared_ptr<data_t> data = std::make_shared<data_t>(); // �C���X�^���X�f�[�^�B

	LRESULT channel_onChar(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT channel_onSysKeyDown(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT ctl_onChar(WNDPROC super_proc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT ctl_onSysKeyDown(WNDPROC super_proc, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT edit_onChar(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT edit_onKeyDown(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT edit_onSysKeyDown(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onActivate(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCommand(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCreate(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCtlColorEdit(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onCtlColorListBox(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onDestroy(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onGetMinMaxInfo(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onMove(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onNCHitTest(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onNCLButtonDblClk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onPaint(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onSetCursor(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onSetFocus(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT onSize(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	
	void channel_create(HWND hwnd);
	void channel_select(int ch_index);
	void deleteFromList();
	void drawEdge(HDC hdc, int left, int top, int width, int height, COLORREF raised_color, COLORREF sunken_color);
	void edit_create(HWND hwnd);
	std::string edit_getLine();
	void edit_replaceText(const std::string &text);
	void edit_selectText(int first = 0, int last = -1);
	void edit_setLine(const std::string &line = std::string(), change_modes change_mode = change_modes::EDIT);
	void execute(SHORT shift_state);
	HPEN getPen(COLORREF color);
	void loadRegistry();
	void moveFocus(HWND ctl_handle, SHORT shift_state);
	void moveThroughList(SHORT shift_state, BYTE vir_key);
	void prepare(HWND hwnd);
	void storeRegistry();
	std::string text_addChannel(const std::string &jc_text);
	std::string text_eqChatToJChat(const char *eqc_text);
	std::string text_jChatToEQChat(const std::string &jc_text);
};

// �v���O�����̕���(��)��\���B
// ���ׂẴO���[�o���ϐ����܂Ƃ߂��\���̂ł���B
// �܂Ƃ߂邱�ƂŒP�̃e�X�g�ɂ����ĊȒP�Ƀ��Z�b�g�ł���B
struct context_t {
	DWORD base_address;                     // eqgame.exe�̃x�[�X�A�h���X�B
	std::vector<scope_exit_t> detour_exits; // Detour�p�̒E�o�����̃x�N�^�B
	HINSTANCE dll_handle;                   // jchat.dll�̃n���h���B
	std::filesystem::path dll_path;         // jchat.dll�̃p�X�B
	std::vector<scope_exit_t> exits;        // �E�o�����̃x�N�^�B
	int game_state = 0;                     // �Q�[���̏�ԁB
	std::filesystem::path ini_path;         // jchat.ini�̃p�X�B
	jchat_bar_t jchat_bar;                  // ���{��`���b�g�o�[�B
	std::shared_ptr<std::ostream> log;      // jchat.log�̏o�̓X�g���[���B
	std::filesystem::path log_path;         // jchat.log�̃p�X�B
	screen_modes screen_mode;               // ��ʃ��[�h�B
	struct {
		struct {
			std::string command_symbols; // �R�}���h�L���̗�B
			std::string font_name;       // �t�H���g�̖��O�B
			std::size_t link_body_size;  // �A�C�e�������N�̃{�f�B�����̃T�C�Y�B
		} chat; // �`���b�g�Ɋւ���ݒ�B
		struct {
			int left;              // ���ӂ�X���W�B
			std::size_t min_width; // �ŏ��̕��B
			int top;               // ��ӂ�Y���W�B
			std::size_t width;     // ���B
		} window; // �E�C���h�E�Ɋւ���ݒ�B
	} ini; // jchat.ini�̓��e�B

	void fail(const error_t &err);
	void release();
	void startup(HINSTANCE hinstDLL);
};

//// �O���[�o���ϐ��̐錾

extern DispatchMessage_t DispatchMessage_target;
extern context_t context;

//// �e���v���[�g�֐��ƃC�����C���֐��̐錾

inline DWORD makeKeySituation(BYTE not_in_chat_mode, const KeyCombo &key_combo);

//// �֐��̐錾

LRESULT WINAPI DispatchMessage_detour(const MSG *msg);

void bindAddressAndDetourAttach_early();
void bindAddressAndDetourAttach_lazy();
DWORD getAddress(const std::string &name);
DWORD getOffset(const std::string &name);
screen_modes getScreenMode();
void onGameStateChange();
BOOL onProcessAttach(HINSTANCE hinstDLL);
BOOL onProcessDetach();
void onScreenModeChange();
void refreshGameState(int new_game_state);
void refreshScreenMode();

//// �N���X�̃e���v���[�g�����o�ƃC�����C�������o�̒�`

// �����q�����X�g�̐擪�ɂ��邩�ǂ����𔻒肷��B
bool // �^�Ȃ�擪�A�U�Ȃ炻��ȊO�B
lines_and_iter_t::isBegin() const {
	return iter == lines.begin();
}

// �����q�����X�g�̖����ɂ��邩�ǂ����𔻒肷��B
bool // �^�Ȃ疖���A�U�Ȃ炻��ȊO�B
lines_and_iter_t::isEnd() const {
	return iter == lines.end();
}

// �����q�����X�g�̐擪�Ɉړ�����B
void lines_and_iter_t::moveBegin() {
	iter = lines.begin();
}

// �����q�����X�g�̖����Ɉړ�����B
void lines_and_iter_t::moveEnd() {
	iter = lines.end();
}

//// �e���v���[�g�֐��ƃC�����C���֐��̒�`

// �`���b�g���[�h�t���O�ƃL�[�̑g�ݍ��킹���������āA
// �L�[�̏󋵂��쐬����B
DWORD // �쐬�����L�[�̏󋵁B
makeKeySituation(
	BYTE not_in_chat_mode,    // EQ�`���b�g���[�h�t���O�B
	const KeyCombo &key_combo // �L�[�̑g�ݍ��킹�B
) {
	return (DWORD(not_in_chat_mode) << 7) | indirect_cast<DWORD>(key_combo);
}

}

#endif // #ifndef JEQ_JCHAT_HPP

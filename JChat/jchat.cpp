//// JEQ/���{��`���b�g�v���O�C��/�{��

//// �C���N���[�h

#include "common.hpp"
#include "eqgamedef.hpp"
#include "jchat.hpp"
#include "resource.h"

#include <dinput.h>
#include <windows.h>
#include <detours.h>

#undef max
#undef min

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstring>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

//// �}�N���̒�`

// DetourAttach�֐��̌Ăяo�����܂ރg�����U�N�V�������J�n����B
// ����ȍ~�ADetourAttach������A���[�J���ϐ�detach_procs��
// ���̊֐���DetourDetach���鏈����ǉ����邱�ƁB
#define BEGIN_DETOUR_ATTACH() \
	dtr::DetourTransactionBegin();\
	scope_exit_t abort_exit([] {\
		dtr::DetourTransactionAbort();\
	});\
	std::vector<scope_exit_t::procedure_t> detach_procs;

// jchat.ini����I�t�Z�b�g��ǂݍ��݁A�A�h���X���v�Z���A
// eqgame_t�ɑ�����f�[�^�̃|�C���^�ɑ������B
#define BIND_ADDRESS(\
	name /* eqgame_t::�ɑ����|�C���^�̖��O�B */\
) \
	{\
		DWORD *ptr = &(DWORD&)(eqgame_t::name);\
		*ptr = getAddress(#name);\
	}

// name�������֐��ɂ���DetourAttach�֐����Ăяo���B
// Target�֐��|�C���^�̖��O��name_target�ɂȂ�B
// Detour�֐��̖��O��name_detour�ɂȂ�B
// ������name�͎������Œu����������B
// ����������Adetach_procs��DetourDetach���鏈����ǉ�����B
// Attach��ATarget�֐��|�C���^�̒l��Target�֐��̃A�h���X����
// Trampoline�֐��̃A�h���X�ɓ���ւ��̂Œ��ӂ��邱�ƁB
#define DETOUR_ATTACH(\
	name /* �֐��̖��O�B */\
) \
	{\
		PVOID *target = &(PVOID&)(name ## _target);\
		PVOID detour = indirect_cast<PVOID>(&name ## _detour);\
		dtr::DetourAttach(target, detour);\
		detach_procs.emplace_back([target, detour] {\
			dtr::DetourDetach(target, detour);\
		});\
	}

// jchat.ini����A�h���X��ǂݍ��݁A�A�h���X���v�Z���A
// eqgame_t�ɑ�����Target�֐��|�C���^�ɑ������B
// ������name�������֐��ɂ���DetourAttach�֐����Ăяo���B
// Target�֐��|�C���^�̖��O��eqgame_t::name_target�ɂȂ�B
// Detour�֐��̖��O��eqgame_t::name_detour�ɂȂ�B
// ������name�͎������Œu����������B
// ����������Adetach_procs��DetourDetach���鏈����ǉ�����B
// Attach��ATarget�֐��|�C���^�̒l��Target�֐��̃A�h���X����
// Trampoline�֐��̃A�h���X�ɓ���ւ��̂Œ��ӂ��邱�ƁB
#define BIND_ADDRESS_AND_DETOUR_ATTACH(\
	name /* eqgame_t::�ɑ����֐��̖��O�B */\
) \
	{\
		DWORD *ptr = &(DWORD&)(eqgame_t::name ## _target);\
		*ptr = getAddress(#name);\
		DETOUR_ATTACH(eqgame_t::name);\
	}

// DetourAttach�֐��̌Ăяo�����܂ރg�����U�N�V�������I������B
// ����������Acontext.detour_exits�ɁA�g�����U�N�V��������
// DetourAttach�����֐���DetourDetach����E�o������ǉ�����B
#define END_DETOUR_ATTACH() \
	dtr::DetourTransactionCommit();\
	abort_exit.procedure() = nullptr;\
	for (auto detach_proc : detach_procs)\
		context.detour_exits.emplace_back(detach_proc);

namespace jeq {

//// �N���X�����o�̒�`

// ���{��`���b�g�o�[���A�N�e�B�u�ɂ���L�[�󋵂̃Z�b�g�B
const std::unordered_set<DWORD> eqgame_t::ACTIVATE_KEY_SITUATIONS = {
	makeKeySituation(TRUE , KeyCombo{0, 0, 0, DIK_RETURN}),
	makeKeySituation(TRUE , KeyCombo{0, 0, 0, DIK_SLASH }),
	makeKeySituation(TRUE , KeyCombo{0, 0, 1, DIK_UP    }),
	makeKeySituation(TRUE , KeyCombo{0, 0, 1, DIK_DOWN  }),
	makeKeySituation(FALSE, KeyCombo{0, 0, 0, DIK_RETURN}),
};

// ��������L�[�󋵂̃Z�b�g�B
const std::unordered_set<DWORD> eqgame_t::IGNORE_KEY_SITUATIONS = {
	makeKeySituation(TRUE, KeyCombo{0, 0, 0, DIK_RETURN}),
	makeKeySituation(TRUE, KeyCombo{0, 0, 0, DIK_SLASH }),
	makeKeySituation(TRUE, KeyCombo{0, 0, 1, DIK_UP    }),
	makeKeySituation(TRUE, KeyCombo{0, 0, 1, DIK_DOWN  }),
};

eqgame_t **eqgame_t::CEverQuest;  // �G�o�[�N�G�X�g�B
eqgame_t **eqgame_t::CharSpawn;	  // ��PC�̃X�|�[���B
HWND *eqgame_t::MainWindowHandle; // ���C���E�C���h�E�̃n���h���B
PBYTE eqgame_t::NotInChatMode;	  // EQ�`���b�g���[�h�t���O�B
                                  // �Z�b�g�Ȃ�EQ�`���b�g�ɓ��͂ł����Ԃł͂Ȃ��A
                                  // �N���A�Ȃ�EQ�`���b�g�ɓ��͂ł����Ԃł���B

// CEverQuest�N���X��InterpretCmd�����o��Target�֐��|�C���^�B
eqgame_t::CEverQuest_InterpretCmd_t 
	eqgame_t::CEverQuest_InterpretCmd_target;
// CEverQuest�N���X��SetGameState�����o��Target�֐��|�C���^�B
eqgame_t::CEverQuest_SetGameState_t 
	eqgame_t::CEverQuest_SetGameState_target;
// CXWndManager�N���X��DrawCursor�����o��Target�֐��|�C���^�B
eqgame_t::CXWndManager_DrawCursor_t 
	eqgame_t::CXWndManager_DrawCursor_target;
// KeypressHandler�N���X��HandleKeyDown�����o��Target�֐��|�C���^�B
eqgame_t::KeypressHandler_HandleKeyDown_t 
	eqgame_t::KeypressHandler_HandleKeyDown_target;
// KeypressHandler�N���X��HandleKeyUp�����o��Target�֐��|�C���^�B
eqgame_t::KeypressHandler_HandleKeyUp_t 
	eqgame_t::KeypressHandler_HandleKeyUp_target;
// ProcessGameEvents��Target�֐��|�C���^�B
eqgame_t::ProcessGameEvents_t 
	eqgame_t::ProcessGameEvents_target;

// CEverQuest�N���X��InterpretCmd�����o��Detour�֐��B
// �R�}���h�̓��e����͂��A���s���邽�߂ɌĂяo�����B
// EQ�`���b�g������͂��ꂽ�R�}���h�͂��̂܂ܓ��{��`���b�g�֓]������B
// ��������EQ�`���b�g���[�h�̂Ƃ��͉������Ȃ��B
// �G���[������������jchat.dll�̋@�\���~����B
void eqgame_t::CEverQuest_InterpretCmd_detour(
	eqgame_t *player, // �v���C���[�B
	char *cmd         // ���s����R�}���h�B
) {
	if (!*NotInChatMode) context.jchat_bar.transferText(cmd);
	else (this->*CEverQuest_InterpretCmd_target)(player, cmd);
}

// CEverQuest�N���X��SetGameState�����o��Detour�֐��B
// �Q�[���̏�Ԃ�ݒ肷�邽�߂ɌĂяo�����B
// �Q�[���̏�Ԃ��ŐV�̏�ԂɍX�V���ATrampoline�֐����Ăяo���B
void eqgame_t::CEverQuest_SetGameState_detour(
	int game_state // �Q�[���̏�ԁB
) {
	refreshGameState(game_state);
	(this->*CEverQuest_SetGameState_target)(game_state);
}

// CXWndManager�N���X��DrawCursor�����o��Detour�֐��B
// �}�E�X�J�[�\����`�悷�邽�߂ɌĂяo�����B
// �������}�E�X�J�[�\�����`���b�g�o�[�̏�ɂ���Ƃ��͕`�悵�Ȃ��B
// �G���[������������jchat.dll�̋@�\���~����B
int // �����炭0�Ȃ琬���A����ȊO�Ȃ玸�s�B
eqgame_t::CXWndManager_DrawCursor_detour() {
	try {
		if (cursor_isOverWindow(context.jchat_bar.getHandle())) return 0;
	} catch (const error &err) {
		context.fail(err);
	}
	return (this->*CXWndManager_DrawCursor_target)();
}

// KeypressHandler�N���X��HandleKeyDown�����o��Detour�֐��B
// ���C���E�C���h�E�ŃL�[�������ꂽ�Ƃ��ɌĂяo�����B
// Enter�L�[�Ȃǂ������ꂽ��ATrampoline�֐����Ăяo�����ɃC�x���g�𖳎�����B
// �G���[������������jchat.dll�̋@�\���~����B
bool // �����炭�^�Ȃ珈���ς݁A�U�Ȃ疢�����B
eqgame_t::KeypressHandler_HandleKeyDown_detour(
	const KeyCombo &key_combo // �����ꂽ�L�[�̑g�ݍ��킹�B
) {
	if (IGNORE_KEY_SITUATIONS.count(makeKeySituation(
			*NotInChatMode, 
			key_combo
		))
	) return true;
	return (this->*KeypressHandler_HandleKeyDown_target)(key_combo);
}

// KeypressHandler�N���X��HandleKeyUp�����o��Detour�֐��B
// ���C���E�C���h�E�ŃL�[�������ꂽ�Ƃ��ɌĂяo�����B
// ��EQ�`���b�g���[�h��Enter�L�[�Ȃǂ������ꂽ��A
// Trampoline�֐����Ăяo�����ɁA�`���b�g�o�[���A�N�e�B�u�ɂ���B
// �܂�EQ�`���b�g���[�h��Enter�L�[�������ꂽ��A
// ����̓R�}���h�����s���悤�Ƃ��Ă��邱�ƂɂȂ�̂�
// (�R�}���h�͉���肳��ē��{��`���b�g�ɓ]�������)�A
// ���̏ꍇ���`���b�g�o�[���A�N�e�B�u�ɂ���B
// �G���[������������jchat.dll�̋@�\���~����B
bool // �����炭�^�Ȃ珈���ς݁A�U�Ȃ疢�����B
eqgame_t::KeypressHandler_HandleKeyUp_detour(
	const KeyCombo &key_combo // �����ꂽ�L�[�̑g�ݍ��킹�B
) {
	try {
		if (ACTIVATE_KEY_SITUATIONS.count(makeKeySituation(
				*NotInChatMode, 
				key_combo
			))
		) {
			context.jchat_bar.activate(key_combo);
			return true;
		}
	} catch (const error &err) {
		context.fail(err);
	}
	return (this->*KeypressHandler_HandleKeyUp_target)(key_combo);
}

// ProcessGameEvents��Detour�֐��B
// ��PC���X�|�[�����ɒZ���Ԋu��(�����炭�t���[������)�Ăяo�����B
// �e�����ŐV�̏�ԂɍX�V���ATrampoline�֐����Ăяo���B
BOOL // �s���B
eqgame_t::ProcessGameEvents_detour() {
	refreshScreenMode();
	refreshGameState(PEVERQUEST(*CEverQuest)->GameState);
	return ProcessGameEvents_target();
}

// ���{��`���b�g�o�[�̃N���X��o�^����B
// �E�C���h�E���쐬����O��1�񂾂��Ăяo�����ƁB
void jchat_bar_t::registerClass() {
	WNDCLASS class_ = {};
	// ����ύX���ꂽ��ĕ`�悵�A����{�^���͕K�v�Ȃ��B
	class_.style = CS_HREDRAW | CS_NOCLOSE;
	class_.lpfnWndProc = &WindowProc;
	class_.hInstance = context.dll_handle;
	// �N���C�A���g�̈�ł�EQ�̖��J�[�\�����g���B
	class_.hCursor = api::LoadCursor(
		context.dll_handle, 
		MAKEINTRESOURCE(IDC_EQ_ARROW)
	);
	class_.lpszClassName = JCHAT_BAR_CLASS_NAME;
	api::RegisterClass(&class_);
	context.exits.emplace_back([] {
		api::UnregisterClass(JCHAT_BAR_CLASS_NAME, context.dll_handle);
	});
}

// ���{��`���b�g�o�[���A�N�e�B�u�ɂ���B
// �����������X���b�V���L�[�Ȃ�e�L�X�g��"/"�ɂ��āA�����ē��͂ł���
// �悤�ɃL�����b�g���ړ����Ă���A�E�C���h�E���A�N�e�B�u�ɂ���B
// ����ȊO�Ȃ���͗���ύX�����A�����A�N�e�B�u�ɂ���B
void jchat_bar_t::activate(
	const KeyCombo &key_combo // ���������ɂȂ����L�[�̑g�ݍ��킹�B
) {
	if (key_combo.key == DIK_SLASH) {
		edit_setLine("/");
		edit_selectText(1, 1);
	}
	api::SetActiveWindow(data->handle);
}

// ���{��`���b�g�o�[���쐬����B
// jchat.ini����ǂݍ��񂾈ʒu�ƕ��ŏ���������B
// �E�C���h�E���쐬��������͔�\����ԂɂȂ��Ă���B
void jchat_bar_t::create() {
	// ���C���E�C���h�E�̃N���C�A���g���W�n�ɂ�����ʒu�����������āA
	// �X�N���[�����W�ɕϊ����A�`���b�g�o�[�̏����ʒu�Ƃ��Ďw�肷��B
	data->mw_client_pos = POINT{
		context.ini.window.left, 
		context.ini.window.top
	};
	POINT screen_pos = point_clientToScreen(
		data->mw_client_pos, 
		*eqgame_t::MainWindowHandle
	);
	data->handle = api::CreateWindowEx(
		0,
		JCHAT_BAR_CLASS_NAME, 
		JCHAT_BAR_NAME, 
		WS_POPUP, 
		screen_pos.x, 
		screen_pos.y, 
		context.ini.window.width, 
		JCHAT_BAR_HEIGHT, 
		*eqgame_t::MainWindowHandle,
		NULL,
		context.dll_handle,
		NULL
	);
}

// ���{��`���b�g�o�[�̈ʒu�Y�����C������B
// �ʒu�Y���͉�ʃ��[�h�̕ύX�ɂ���Đ�����B
// �܂��`���b�g�o�[�����C���E�C���h�E�̊O�Ɉʒu���Ă���Ƃ��ɁA
// �t���X�N���[�����[�h�ɕύX�����ƁA�`���b�g�o�[����ʊO��
// �o�Č����Ȃ��Ȃ��Ă��܂����߁A��ʂ̒��S�Ɉړ�����B
void jchat_bar_t::fixPos() {
	// �A�C�R�����[�h�Ȃ牽�����Ȃ��B
	if (context.screen_mode == screen_modes::ICONIC) return;
	// �ۑ����Ă��������C���E�C���h�E�̃N���C�A���g���W�n��
	// ������`���b�g�o�[�̈ʒu���X�N���[�����W�ɕϊ�����B
	POINT pos = point_clientToScreen(
		data->mw_client_pos, 
		*eqgame_t::MainWindowHandle
	);
	// �A�C�R���Ɖ�ʂ��d�����Ă��Ȃ��Ȃ��ʂ̒��S�Ɉړ�����B
	RECT icon_rect = RECT{
		pos.x, 
		pos.y, 
		pos.x + int(ICON_WIDTH),
		pos.y + int(JCHAT_BAR_HEIGHT)
	};
	RECT screen_rect = RECT{
		0, 
		0, 
		api::GetSystemMetrics(SM_CXSCREEN), 
		api::GetSystemMetrics(SM_CYSCREEN)
	};
	if (!rect_areOverlapped(icon_rect, screen_rect)) {
		RECT bar_rect = window_getRect(data->handle);
		int bar_width_half = (bar_rect.right - bar_rect.left) >> 1;
		int bar_height_half = JCHAT_BAR_HEIGHT >> 1;
		int center_x = screen_rect.right >> 1;
		int center_y = screen_rect.bottom >> 1;
		pos.x = std::max(center_x - bar_width_half, 0);
		pos.y = center_y - bar_height_half;
	}
	// �`���b�g�o�[�̈ʒu��ݒ肷��B
	api::SetWindowPos(
		data->handle,
		NULL,
		pos.x,
		pos.y,
		0, 
		0, 
		SWP_NOACTIVATE | SWP_NOSIZE
	);
}

// �`���b�g�o�[�̃n���h�����擾����B
// �E�C���h�E�̍쐬�O��NULL��Ԃ��B
// �E�C���h�E�̍쐬��͗L���ȃn���h����Ԃ��悤�ɂȂ�B
// �E�C���h�E��WM_DESTROY����M����ƁA�Ă�NULL��Ԃ��悤�ɂȂ�B
HWND // �擾�����n���h���B
jchat_bar_t::getHandle() {
	return data->handle;
}

// EQ�`���b�g������{��`���b�g�Ƀe�L�X�g��]������B
// ���͗��Ńe�L�X�g���I������Ă���΁A�����u��������B
// �܂��A�C�e�������N�͓Ǝ��̌`���ɕϊ����Ă���\������B
void jchat_bar_t::transferText(
	const char *eqc_text // EQ�`���b�g�̃e�L�X�g�B
) {
	edit_replaceText(text_eqChatToJChat(eqc_text));
}

// ���{��`���b�g�o�[�̃C���X�^���X�f�[�^��j������B
// �쐬�����f�o�C�X�R���e�L�X�g�Ȃǂ͂����ō폜�����B
jchat_bar_t::data_t::~data_t() {
	reverseClear(exits);
}

// ���z�L�[�R�[�h���`�����l���̃C���f�b�N�X�̃}�b�v�B
const std::unordered_map<BYTE,int> jchat_bar_t::CHANNEL_INDICES = {
	{'A', 0},
	{'G', 1},
	{'U', 2},
	{'O', 3},
	{'R', 4},
	{'S', 5},
	{'H', 6},
};

// ���{��`���b�g�o�[�̃`�����l�����̃��b�Z�[�W�}�b�v�B
const jchat_bar_t::message_map_t jchat_bar_t::CHANNEL_MESSAGE_MAP = {
	{WM_CHAR      , &jchat_bar_t::channel_onChar      },
	{WM_SYSKEYDOWN, &jchat_bar_t::channel_onSysKeyDown},
};

// �`�����l���̃e�L�X�g�̔z��B
const std::array<std::string,7> jchat_bar_t::CHANNEL_TEXTS = {
	"/Auction",
	"/Gsay",
	"/gUildsay",
	"/Ooc",
	"/Reply",
	"/Say",
	"/sHout",
};

// ���{��`���b�g�o�[�̓��͗��̃��b�Z�[�W�}�b�v�B
const jchat_bar_t::message_map_t jchat_bar_t::EDIT_MESSAGE_MAP = {
	{WM_CHAR      , &jchat_bar_t::edit_onChar      },
	{WM_KEYDOWN   , &jchat_bar_t::edit_onKeyDown   },
	{WM_SYSKEYDOWN, &jchat_bar_t::edit_onSysKeyDown},
};

// ���{��`���b�g�o�[�̃��b�Z�[�W�}�b�v�B
const jchat_bar_t::message_map_t jchat_bar_t::MESSAGE_MAP = {
	{WM_ACTIVATE       , &jchat_bar_t::onActivate       },
	{WM_COMMAND        , &jchat_bar_t::onCommand        },
	{WM_CREATE         , &jchat_bar_t::onCreate         },
	{WM_CTLCOLOREDIT   , &jchat_bar_t::onCtlColorEdit   },
	{WM_CTLCOLORLISTBOX, &jchat_bar_t::onCtlColorListBox},
	{WM_DESTROY        , &jchat_bar_t::onDestroy        },
	{WM_GETMINMAXINFO  , &jchat_bar_t::onGetMinMaxInfo  },
	{WM_MOVE           , &jchat_bar_t::onMove           },
	{WM_NCHITTEST      , &jchat_bar_t::onNCHitTest      },
	{WM_NCLBUTTONDBLCLK, &jchat_bar_t::onNCLButtonDblClk},
	{WM_PAINT          , &jchat_bar_t::onPaint          },
	{WM_SETCURSOR      , &jchat_bar_t::onSetCursor      },
	{WM_SETFOCUS       , &jchat_bar_t::onSetFocus       },
	{WM_SIZE           , &jchat_bar_t::onSize           },
};

// ���{��`���b�g�o�[�̃E�C���h�E�v���V�[�W���B
// �E�C���h�E�ŃC�x���g�����������Windows����Ăяo�����B
// ���b�Z�[�W�}�b�v����n���h����T���A������΂�����Ăяo���B
// ������Ȃ���Ί���̃E�C���h�E�v���V�[�W���ɈϏ�����B
// �G���[������������jchat.dll�̋@�\���~����B
LRESULT CALLBACK jchat_bar_t::WindowProc(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg, 	   // �E�C���h�E���b�Z�[�W�B
	WPARAM wparam, // ���[�h�p�����[�^�B
	LPARAM lparam  // �����O�p�����[�^�B
) {
	LRESULT res = 0;
	auto &wnd = context.jchat_bar;
	auto handler = MESSAGE_MAP.find(msg);
	if (handler == MESSAGE_MAP.end())
		res = api::DefWindowProc(hwnd, msg, wparam, lparam);
	else {
		try {
			res = (wnd.*handler->second)(hwnd, msg, wparam, lparam);
		} catch (const error &err) {
			// WM_CREATE��CreateWindowEx�֐��̒����瑗�M�����̂ŁA
			// �����ł�fail�֐����Ăяo�����A���O���������ނ����ɂ���B
			if (msg == WM_CREATE) {
				putLog(*context.log, err.getMessage());
				res = -1;
			} else context.fail(err);
		}
	} 
	return res;
}

// ���{��`���b�g�o�[�̃`�����l�����̃T�u�N���X����̃E�C���h�E�v���V�[�W���B
// �`�����l�����ŃC�x���g�����������Windows����Ăяo�����B
// ���b�Z�[�W�}�b�v����n���h����T���A������΂�����Ăяo���B
// ������Ȃ���΃T�u�N���X���O�̃E�C���h�E�v���V�[�W���ɈϏ�����B
// �G���[������������jchat.dll�̋@�\���~����B
LRESULT // ���ʁB
CALLBACK jchat_bar_t::channel_WindowProc_sub(
	HWND hwnd,     // �`�����l�����̃n���h���B
	UINT msg,      // �E�C���h�E���b�Z�[�W�B
	WPARAM wparam, // ���[�h�p�����[�^�B
	LPARAM lparam  // �����O�p�����[�^�B
) {
	LRESULT res = 0;
	auto &wnd = context.jchat_bar;
	auto handler = CHANNEL_MESSAGE_MAP.find(msg);
	if (handler == CHANNEL_MESSAGE_MAP.end())
		res = api::CallWindowProc(
			wnd.data->channel.WindowProc_super, 
			hwnd,
			msg, 
			wparam, 
			lparam
		);
	else {
		try {
			res = (wnd.*handler->second)(hwnd, msg, wparam, lparam);
		} catch (const error &err) {
			context.fail(err);
		}
	}
	return res;
}

// ���{��`���b�g�o�[�̓��͗��̃T�u�N���X����̃E�C���h�E�v���V�[�W���B
// ���͗��ŃC�x���g�����������Windows����Ăяo�����B
// ���b�Z�[�W�}�b�v����n���h����T���A������΂�����Ăяo���B
// ������Ȃ���΃T�u�N���X���O�̃E�C���h�E�v���V�[�W���ɈϏ�����B
// �G���[������������jchat.dll�̋@�\���~����B
LRESULT // ���ʁB
CALLBACK jchat_bar_t::edit_WindowProc_sub(
	HWND hwnd,     // ���͗��̃n���h���B
	UINT msg,      // �E�C���h�E���b�Z�[�W�B
	WPARAM wparam, // ���[�h�p�����[�^�B
	LPARAM lparam  // �����O�p�����[�^�B
) {
	LRESULT res = 0;
	auto &wnd = context.jchat_bar;
	auto handler = EDIT_MESSAGE_MAP.find(msg);
	if (handler == EDIT_MESSAGE_MAP.end())
		res = api::CallWindowProc(
			wnd.data->edit.WindowProc_super, 
			hwnd,
			msg, 
			wparam, 
			lparam
		);
	else {
		try {
			res = (wnd.*handler->second)(hwnd, msg, wparam, lparam);
		} catch (const error &err) {
			context.fail(err);
		}
	}
	return res;
}

// ���{��`���b�g�̃`�����l�����ŕ��������͂��ꂽ�Ƃ��ɌĂяo�����B
// Enter�L�[�������ꂽ��A���͍s�����s���A���܂ł̔������X�g�ɒǉ�����B
// Shift+Enter�L�[�������ꂽ��A���͍s���悭�g���������X�g�ɒǉ�����B
// Esc�L�[�������ꂽ�烁�C���E�C���h�E���A�N�e�B�u�ɂ���B
// Tab�L�[�������ꂽ��t�H�[�J�X���ړ�����B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::channel_onChar(
	HWND hwnd,     // �`�����l�����̃n���h���B
	UINT msg,      // WM_CHAR���b�Z�[�W�B
	WPARAM wparam, // ���z�L�[�R�[�h�B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	return ctl_onChar(
		data->channel.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// ���{��`���b�g�̃`�����l�����ŃV�X�e���L�[�������ꂽ�Ƃ��ɌĂяo�����B
// �`�����l���̃V���[�g�J�b�g�L�[�������ꂽ��A���̃`�����l����I������B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::channel_onSysKeyDown(
	HWND hwnd,     // �`�����l�����̃n���h���B
	UINT msg,      // WM_SYSKEYDOWN���b�Z�[�W�B
	WPARAM wparam, // ���z�L�[�R�[�h�B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	return ctl_onSysKeyDown(
		data->channel.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// ���{��`���b�g�̃R���g���[���ŕ��������͂��ꂽ�Ƃ��ɌĂяo�����B
// Enter�L�[�������ꂽ��A���͍s�����s���A���܂ł̔������X�g�ɒǉ�����B
// Shift+Enter�L�[�������ꂽ��A���͍s���悭�g���������X�g�ɒǉ�����B
// Esc�L�[�������ꂽ�烁�C���E�C���h�E���A�N�e�B�u�ɂ���B
// Tab�L�[�������ꂽ��t�H�[�J�X���ړ�����B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::ctl_onChar(
	WNDPROC super_proc, // �T�u�N���X���O�̃E�C���h�E�v���V�[�W���B
	HWND hwnd,          // �R���g���[���̃n���h���B
	UINT msg,           // WM_CHAR���b�Z�[�W�B
	WPARAM wparam,      // ���z�L�[�R�[�h�B
	LPARAM lparam       // �g�p���Ȃ��B
) {
	SHORT shift_state = api::GetKeyState(VK_SHIFT);
	if (wparam == VK_RETURN) {
		execute(shift_state);
		return 0;
	} else if (wparam == VK_ESCAPE) {
		api::SetActiveWindow(*eqgame_t::MainWindowHandle);
		return 0;
	} else if (wparam == VK_TAB) {
		moveFocus(hwnd, shift_state);
		return 0;
	}
	return api::CallWindowProc(
		super_proc, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// ���{��`���b�g�̃R���g���[���ŃV�X�e���L�[�������ꂽ�Ƃ��ɌĂяo�����B
// �`�����l���̃V���[�g�J�b�g�L�[�������ꂽ��A���̃`�����l����I������B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::ctl_onSysKeyDown(
	WNDPROC super_proc, // �T�u�N���X���O�̃E�C���h�E�v���V�[�W���B
	HWND hwnd,          // �R���g���[���̃n���h���B
	UINT msg,           // WM_SYSKEYDOWN���b�Z�[�W�B
	WPARAM wparam,      // ���z�L�[�R�[�h�B
	LPARAM lparam       // �g�p���Ȃ��B
) {
	int ch_index;
	if ((ch_index = map_find(CHANNEL_INDICES, wparam, -1)) >= 0) {
		channel_select(ch_index);
		return 0;
	}
	return api::CallWindowProc(
		super_proc, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// ���{��`���b�g�̓��͗��ŕ��������͂��ꂽ�Ƃ��ɌĂяo�����B
// Ctrl+A�L�[�������ꂽ��S�e�L�X�g��I������B
// Enter�L�[�������ꂽ��A���͍s�����s���A���܂ł̔������X�g�ɒǉ�����B
// Shift+Enter�L�[�������ꂽ��A���͍s���悭�g���������X�g�ɒǉ�����B
// Esc�L�[�������ꂽ�烁�C���E�C���h�E���A�N�e�B�u�ɂ���B
// Tab�L�[�������ꂽ��t�H�[�J�X���ړ�����B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::edit_onChar(
	HWND hwnd,     // ���͗��̃n���h���B
	UINT msg,      // WM_CHAR���b�Z�[�W�B
	WPARAM wparam, // ���z�L�[�R�[�h�B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	if (wparam == getCtrlAlphabetKey('A')) {
		edit_selectText();
		return 0;
	}
	return ctl_onChar(
		data->edit.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// ���{��`���b�g�̓��͗��ŃL�[�������ꂽ�Ƃ��ɌĂяo�����B
// ���L�[�⁫�L�[�������ꂽ�獡�܂ł̔������X�g����I������B
// Shift+���L�[��Shift+���L�[�������ꂽ��A�悭�g���������X�g����I������B
// Shift+Delete�L�[�������ꂽ��A���X�g����I���s���폜����B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::edit_onKeyDown(
	HWND hwnd,     // ���͗��̃n���h���B
	UINT msg,      // WM_KEYDOWN���b�Z�[�W�B
	WPARAM wparam, // ���z�L�[�R�[�h�B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	SHORT shift_state = api::GetKeyState(VK_SHIFT);
	if (wparam == VK_UP || wparam == VK_DOWN) {
		moveThroughList(shift_state, wparam);
		return 0;
	} else if (shift_state < 0 && wparam == VK_DELETE) {
		deleteFromList();
		return 0;
	}
	return api::CallWindowProc(
		data->edit.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// ���{��`���b�g�̓��͗��ŃV�X�e���L�[�������ꂽ�Ƃ��ɌĂяo�����B
// �`�����l���̃V���[�g�J�b�g�L�[�������ꂽ��A���̃`�����l����I������B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::edit_onSysKeyDown(
	HWND hwnd,     // ���͗��̃n���h���B
	UINT msg,      // WM_SYSKEYDOWN���b�Z�[�W�B
	WPARAM wparam, // ���z�L�[�R�[�h�B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	int ch_index;
	if ((ch_index = map_find(CHANNEL_INDICES, wparam, -1)) >= 0) {
		channel_select(ch_index);
		return 0;
	}
	return api::CallWindowProc(
		data->edit.WindowProc_super, 
		hwnd, 
		msg, 
		wparam, 
		lparam
	);
}

// ���{��`���b�g�o�[���A�N�e�B�u�A�܂��̓C���A�N�e�B�u��
// �Ȃ����Ƃ��ɌĂяo�����B�A�C�R�����ĕ`�悷��B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::onActivate(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_ACTIVATE���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	api::InvalidateRect(data->handle, NULL, TRUE);
	return 0;
}

// ���{��`���b�g�o�[�փR�}���h�����M���ꂽ�Ƃ��ɌĂяo�����B
// ���͗�����e�L�X�g�̕ύX��ʒm���ꂽ��A�X�V�t���O���Z�b�g���A
// ���܂ł̔������X�g�Ƃ悭�g���������X�g�̔����q�����Z�b�g����B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::onCommand(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_COMMAND���b�Z�[�W�B
	WPARAM wparam, // �R���g���[����ID�ƒʒm�R�[�h�B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	// �X�V�Ƃ݂Ȃ��͕̂ҏW�ŕύX����ꍇ�����ŁA
	// ���X�g����I������ꍇ�͍X�V�Ƃ݂Ȃ��Ȃ��B
	if (LOWORD(wparam) == EDIT_ID &&
		HIWORD(wparam) == EN_CHANGE && 
		data->edit.change_mode == change_modes::EDIT
	) {
		data->edit.dirty = true;
		data->history.moveEnd();
		data->registry.moveEnd();
	}
	return 0;
}

// ���{��`���b�g�o�[���쐬���Ă���Ƃ��ɌĂяo�����B
// ���͗����Z�b�g�A�b�v���A�w�i�̃O���t�B�b�N���������A
// jchat.ini����悭�g���������X�g��ǂݍ��ށB
// ���s�����Ƃ��́A�G���[���X���[���邱�Ƃł���������B
// �G���[��WindowProc�ŃL���b�`����A�ŏI�I��-1��Ԃ��B
LRESULT // ���ʁB0�Ȃ琬���A-1�Ȃ玸�s�B
jchat_bar_t::onCreate(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_CREATE���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	prepare(hwnd);
	channel_create(hwnd);
	edit_create(hwnd);
	loadRegistry();
	return 0;
}

// ���{��`���b�g�o�[�̓��͗��ƃ`�����l�����`�悳���Ƃ��ɌĂяo�����B
LRESULT // �w�i��h��Ԃ����߂̃u���V�̃n���h���B
jchat_bar_t::onCtlColorEdit(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_CTLCOLOREDIT���b�Z�[�W�B
	WPARAM wparam, // �R���g���[���̃f�o�C�X�R���e�L�X�g�̃n���h���B 
	LPARAM lparam  // �R���g���[���̃n���h���B
) {
	HDC ctl_dc_handle = HDC(wparam);
	HWND ctl_handle = HWND(lparam);
	api::SetTextColor(ctl_dc_handle, TEXT_COLOR);
	if (ctl_handle == data->channel.handle) {
		// �`�����l�����̃{�b�N�X�����͔w�i�̃u���V���w�肵�Ă�
		// ��������邽�߁A�s�������[�h�Ɣw�i�F��ݒ肵�ēh��Ԃ��B
		api::SetBkMode(ctl_dc_handle, OPAQUE);
		api::SetBkColor(ctl_dc_handle, CHANNEL_BK_COLOR);
	} else if (ctl_handle == data->edit.handle) {
		// ���͗��̔w�i�͊�_�u���V�œh��Ԃ��̂œ������[�h��ݒ肷��B
		api::SetBkMode(ctl_dc_handle, TRANSPARENT);
		// ���_�Ƃ��ă`���b�g�o�[�̍�����ɑ΂��鑊�΍��W��ݒ肷��B
		// �������邱�ƂŃ`���b�g�o�[�̔w�i�ƃV�[�����X�ɂȂ�B
		api::SetBrushOrgEx(
			ctl_dc_handle, 
			-(EDIT_LEFT + 2), 
			-(EDIT_TOP + 4), 
			NULL
		);
	}
	return LRESULT(data->rock_bar_brush_handle);
}

// ���{��`���b�g�o�[�̃`�����l�����̃��X�g�������`�悳���Ƃ���
// �Ăяo�����B�`�����l�����̔w�i�͊�ǃu���V�œh��Ԃ��B
LRESULT // �w�i��h��Ԃ����߂̃u���V�̃n���h���B
jchat_bar_t::onCtlColorListBox(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_CTLCOLORLISTBOX���b�Z�[�W�B
	WPARAM wparam, // �R���g���[���̃f�o�C�X�R���e�L�X�g�̃n���h���B 
	LPARAM lparam  // �R���g���[���̃n���h���B
) {
	HDC ctl_dc_handle = HDC(wparam);
	api::SetTextColor(ctl_dc_handle, TEXT_COLOR);
	api::SetBkMode(ctl_dc_handle, TRANSPARENT);
	return LRESULT(data->rock_wall_brush_handle);
}

// ���{��`���b�g�o�[���j�������Ƃ��ɌĂяo�����B
// jchat.ini�ɃE�C���h�E�̍Ō�̈ʒu�ƕ��A�����Ă悭�g��
// �������X�g��ۑ����āA�C���X�^���X�f�[�^�����Z�b�g����B
// �G���[���������Ă��X���[�����A�I���������p������B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::onDestroy(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_DESTROY���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	try {
		RECT rect = window_getRect(hwnd);
		ini_setKeyValue(
			context.ini_path,
			"Window",
			"Left",
			stringize(data->mw_client_pos.x)
		);
		ini_setKeyValue(
			context.ini_path,
			"Window",
			"Top",
			stringize(data->mw_client_pos.y)
		);
		ini_setKeyValue(
			context.ini_path,
			"Window",
			"Width",
			stringize(rect.right - rect.left)
		);
		storeRegistry();
	} catch (const error &err) {
		putLog(*context.log, err.getMessage());
	}
	data = std::make_shared<data_t>();
	return 0;
}

// ���{��`���b�g�o�[���h���b�O����Ă���Ƃ��ɁA
// �ŏ��T�C�Y�ƍő�T�C�Y���擾���邽�߂ɌĂяo�����B
// ���Ȃ̂͒Z������߂��邱�Ƃ����Ȃ̂ōŏ������w�肷��B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::onGetMinMaxInfo(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_GETMINMAXINFO���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // MINMAXINFO�̃|�C���^�B
) {
	PMINMAXINFO info = PMINMAXINFO(lparam);
	info->ptMinTrackSize.x = context.ini.window.min_width;
	return 0;
}

// ���{��`���b�g�o�[���ړ����ꂽ�Ƃ��ɌĂяo�����B
// ���C���E�C���h�E�̃N���C�A���g���W�n�ɂ�����ʒu���X�V����B
LRESULT // �̈�̎�ށB
jchat_bar_t::onMove(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_MOVE���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // X���W��Y���W�B
) {
	data->mw_client_pos = point_screenToClient(
		POINT{LOWORD(lparam), HIWORD(lparam)}, 
		*eqgame_t::MainWindowHandle
	);
	return 0;
}

// ���{��`���b�g�o�[�̏�Ƀ}�E�X�J�[�\��������Ƃ��ɁA
// �������ǂ������̈�Ȃ̂����擾���邽�߂ɌĂяo�����B
// �A�C�R���Ȃ�ړ��ł���悤�ɃL���v�V�����A�܂݂Ȃ�
// ����ύX�ł���悤�ɉE�ӁA����ȊO�Ȃ�N���C�A���g�̈��Ԃ��B
LRESULT // �̈�̎�ށB
jchat_bar_t::onNCHitTest(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_NCHITTEST���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �}�E�X�J�[�\����X���W��Y���W�B
) {
	POINT cursor_pos{short(LOWORD(lparam)), short(HIWORD(lparam))};
	RECT rect = window_getRect(hwnd);
	LONG edit_left = rect.left + ICON_WIDTH;
	if (cursor_pos.x < edit_left) return HTCAPTION;
	LONG edit_right = rect.right - KNOB_WIDTH;
	if (cursor_pos.x >= edit_right) return HTRIGHT;
	return HTCLIENT;
}

// ���{��`���b�g�o�[�̔�N���C�A���g�̈�����_�u���N���b�N
// ���ꂽ�Ƃ��ɌĂяo�����B�`���b�g�o�[���ŏ����܂ŏk�߂�B
LRESULT // �̈�̎�ށB
jchat_bar_t::onNCLButtonDblClk(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_NCLBUTTONDBLCLK���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	// �`���b�g�o�[�̕���ݒ肷��B
	api::SetWindowPos(
		data->handle,
		NULL,
		0,
		0,
		context.ini.window.min_width, 
		JCHAT_BAR_HEIGHT, 
		SWP_NOMOVE
	);
	return HTCLIENT;
}

// ���{��`���b�g�o�[���`�悳���Ƃ��ɌĂяo�����B
// �_�u���o�b�t�@�����O�����Ŕw�i��`�悷��B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::onPaint(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_PAINT���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	PAINTSTRUCT paint;
	HDC hdc = api::BeginPaint(hwnd, &paint);
	scope_exit_t paint_exit([hwnd, &paint] {
		api::EndPaint(hwnd, &paint);
	});
	RECT rect = window_getClientRect(hwnd);
	// ��_�u���V�œh��Ԃ��B
	api::FillRect(data->buffer_dc_handle, &rect, data->rock_bar_brush_handle);
	// �`���b�g�o�[���A�N�e�B�u���ǂ����ɉ����ăA�C�R����؂�ւ���B
	HDC icon_dc_handle;
	if (api::GetActiveWindow() == context.jchat_bar.getHandle())
		icon_dc_handle = data->icon_active_dc_handle;
	else icon_dc_handle = data->icon_inactive_dc_handle;
	// ���[�ɃA�C�R����\��t����B
	api::BitBlt(
		data->buffer_dc_handle,
		0,
		0,
		JCHAT_BAR_HEIGHT,
		JCHAT_BAR_HEIGHT,
		icon_dc_handle,
		0,
		0,
		SRCCOPY
	);
	// �E�[�ɂ܂݂�\��t����B
	api::BitBlt(
		data->buffer_dc_handle,
		rect.right - KNOB_WIDTH,
		0,
		KNOB_WIDTH,
		JCHAT_BAR_HEIGHT,
		data->knob_dc_handle,
		0,
		0,
		SRCCOPY
	);
	// �g���������B
	drawEdge(
		data->buffer_dc_handle,
		0,
		0,
		rect.right, 
		rect.bottom,
		JCHAT_BAR_RAISED_COLOR,
		JCHAT_BAR_SUNKEN_COLOR
	);
	// ���͗��̊O���̘g���������B
	drawEdge(
		data->buffer_dc_handle,
		EDIT_LEFT,
		EDIT_TOP,
		rect.right - EDIT_HOR_MARGIN, 
		rect.bottom - EDIT_TOP * 2,
		EDIT_OUTER_RAISED_COLOR,
		EDIT_OUTER_SUNKEN_COLOR
	);
	// ���͗��̓����̘g���������B
	drawEdge(
		data->buffer_dc_handle,
		EDIT_LEFT + 1,
		EDIT_TOP + 1,
		rect.right - (EDIT_HOR_MARGIN + 2), 
		rect.bottom - (EDIT_TOP + 1) * 2,
		EDIT_INNER_RAISED_COLOR,
		EDIT_INNER_SUNKEN_COLOR
	);
	// ���ʂ��E�C���h�E�ɃR�s�[����B
	api::BitBlt(
		hdc, 
		paint.rcPaint.left, 
		paint.rcPaint.top, 
		paint.rcPaint.right, 
		paint.rcPaint.bottom, 
		data->buffer_dc_handle, 
		0, 
		0, 
		SRCCOPY
	);
	return 0;
}

// ���{��`���b�g�o�[�̏�Ƀ}�E�X�J�[�\��������Ƃ��ɁA
// �}�E�X�J�[�\���̌`���ݒ肷�邽�߂ɌĂяo�����B
// �A�C�R���̏�ɂ���Ƃ���EQ��4�������J�[�\����ݒ肵�A
// �܂݂̏�ɂ���Ƃ���EQ�̍��E���J�[�\����ݒ肷��B
LRESULT // �^�Ȃ珈���ς݁A�U�Ȃ疢�����B
jchat_bar_t::onSetCursor(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_SETCURSOR���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �̈�̎�ށB
) {
	if (LOWORD(lparam) == HTCAPTION) {
		api::SetCursor(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_MOVE)
		));
		return TRUE;
	} else if (LOWORD(lparam) == HTRIGHT) {
		api::SetCursor(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_EW)
		));
		return TRUE;
	}
	return api::DefWindowProc(hwnd, msg, wparam, lparam);
}

// ���{��`���b�g�o�[�Ƀt�H�[�J�X���ݒ肳�ꂽ�Ƃ��ɌĂяo�����B
// �����ɃL�[�{�[�h������͂ł���悤�ɓ��͗��Ƀt�H�[�J�X��ݒ肷��B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::onSetFocus(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_SETFOCUS���b�Z�[�W�B
	WPARAM wparam, // �g�p���Ȃ��B
	LPARAM lparam  // �g�p���Ȃ��B
) {
	api::SetFocus(data->edit.handle);
	return 0;
}

// ���{��`���b�g�o�[�̃T�C�Y���ύX���ꂽ�Ƃ��ȂǂɌĂяo�����B
// �ύX��̃E�C���h�E�̃T�C�Y�ɍ��킹�āA���͗��̃T�C�Y��ύX����B
LRESULT // ���ʁB0�Ȃ珈���ς݁B
jchat_bar_t::onSize(
	HWND hwnd,     // �`���b�g�o�[�̃n���h���B
	UINT msg,      // WM_SIZE���b�Z�[�W�B
	WPARAM wparam, // �T�C�Y�ύX�̎�ށB
	LPARAM lparam  // �E�C���h�E�̕��ƍ����B
) {
	if (wparam == SIZE_RESTORED && data->edit.handle)
		api::MoveWindow(
			data->edit.handle, 
			EDIT_LEFT + 2, 
			EDIT_TOP + 4, 
			LOWORD(lparam) - (EDIT_HOR_MARGIN + 2 + 2), 
			EDIT_HEIGHT, 
			TRUE
		);
	return 0;
}

// ���{��`���b�g�̃`�����l�������쐬����B
void jchat_bar_t::channel_create(
	HWND hwnd // �`���b�g�o�[�̃n���h���B
) {
	// �`���b�g�o�[��e�Ƃ���COMBOBOX�R���g���[�����쐬����B
	data->channel.handle = api::CreateWindowEx(
		0,
		"COMBOBOX", 
		NULL, 
		CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 
		ICON_WIDTH, 
		CHANNEL_TOP, 
		CHANNEL_WIDTH, 
		CHANNEL_HEIGHT, 
		hwnd, 
		HMENU(CHANNEL_ID), 
		context.dll_handle, 
		NULL
	);
	// �쐬�����R���g���[�����T�u�N���X�����A
	// �E�C���h�E�v���V�[�W�����J�X�^�}�C�Y����B
	data->channel.WindowProc_super = WNDPROC(api::SetWindowLongPtr(
		data->channel.handle, 
		GWLP_WNDPROC, 
		LONG_PTR(&channel_WindowProc_sub)
	));
	// �N���C�A���g�̈�̃}�E�X�J�[�\����EQ�̖��J�[�\���ɂ���B
	api::SetClassLong(
		data->channel.handle, 
		GCL_HCURSOR,
		LONG(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_ARROW)
		))
	);
	// �t�H���g���R���g���[���ɐݒ肷��B
	api::SendMessage(
		data->channel.handle, 
		WM_SETFONT, 
		WPARAM(data->hfont), 
		0
	);
	// �e�L�X�g��ǉ�����B
	for (const std::string &CHANNEL_TEXT : CHANNEL_TEXTS) 
		api::SendMessage(
			data->channel.handle, 
			CB_ADDSTRING, 
			0, 
			LPARAM(CHANNEL_TEXT.data())
		);
	// "/Say"��I������B
	channel_select(map_find(CHANNEL_INDICES, 'S'));
	// Tab���̃n���h���ƃC���f�b�N�X��ǉ�����B
	data->ordered_handles.emplace_back(data->channel.handle);
	data->ordered_indices.emplace(
		data->channel.handle, 
		data->ordered_indices.size()
	);
}

// ���{��`���b�g�̃`�����l����I������B
void jchat_bar_t::channel_select(
	int ch_index // �I������`�����l���̃C���f�b�N�X�B
) {
	api::SendMessage(data->channel.handle, CB_SETCURSEL, ch_index, 0);
}

// ���܂ł̔������X�g�A�܂��͂悭�g���������X�g����s���폜����B
// ���X�g�̔����q���w�������s�A�܂�I�𒆂̍s���폜����B
// �I�𒆂̍s���폜������A���x�͂��̈���̍s��I������B
void jchat_bar_t::deleteFromList() {
	// �s��I�𒆂̃��X�g�����邩�ǂ����𒲂ׂ�B
	lines_and_iter_t *lni = nullptr;
	if (!data->history.isEnd()) lni = &data->history;
	else if (!data->registry.isEnd()) lni = &data->registry;
	if (lni) {
		// �I�𒆂̍s���폜���A����̍s��I������B
		lni->iter = lni->lines.erase(lni->iter);
		// ����̍s������΂�����擾���A
		// �Ȃ���΍Ō�ɕҏW���Ă����s���擾����B
		std::string *line;
		if (lni->isEnd()) line = &data->edit.last_line;
		else line = &*lni->iter;
		// ���͗���I�����[�h�ŕύX���A�S�e�L�X�g��I������B
		edit_setLine(*line, change_modes::CHOICE);
		edit_selectText();
	} else edit_setLine(); // �I�𒆂̃��X�g���Ȃ���Γ��͗����N���A����B
}

// �f�o�C�X�R���e�L�X�g�ɘg����`�悷��B
void jchat_bar_t::drawEdge(
	HDC hdc,               // �f�o�C�X�R���e�L�X�g�̃n���h���B
	int left,              // ���ӂ�X���W�B
	int top,               // ��ӂ�Y���W�B
	int width,             // ���B
	int height,            // �����B
	COLORREF raised_color, // ���ӁE��ӂ̐F�B
	COLORREF sunken_color  // �E�ӁE���ӂ̐F�B
) {
	int right = left + width - 1;
	int bottom = top + height - 1;
	POINT points[3];
	// ���ӁE��ӂ�`�悷��B
	// �w��F�̃y����I�����āA3�_�̃|�����C����`���B
	api::SelectObject(hdc, getPen(raised_color));
	points[0] = {left     , bottom};
	points[1] = {left     , top   };
	points[2] = {right + 1, top   };
	api::Polyline(hdc, points, 3);
	// �E�ӁE���ӂ�`�悷��B
	// �w��F�̃y����I�����āA3�_�̃|�����C����`���B
	api::SelectObject(hdc, getPen(sunken_color));
	points[0] = {right, top + 1};
	points[1] = {right, bottom };
	points[2] = {left , bottom };
	api::Polyline(hdc, points, 3);
}

// ���{��`���b�g�̓��͗����쐬����B
void jchat_bar_t::edit_create(
	HWND hwnd // �`���b�g�o�[�̃n���h���B
) {
	// �`���b�g�o�[��e�Ƃ���EDIT�R���g���[�����쐬����B
	data->edit.handle = api::CreateWindowEx(
		0,
		"EDIT", 
		NULL, 
		ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_VISIBLE, 
		EDIT_LEFT + 2, 
		EDIT_TOP + 4, 
		context.ini.window.width - (EDIT_HOR_MARGIN + 2 + 2), 
		EDIT_HEIGHT, 
		hwnd, 
		HMENU(EDIT_ID), 
		context.dll_handle, 
		NULL
	);
	// �쐬�����R���g���[�����T�u�N���X�����A
	// �E�C���h�E�v���V�[�W�����J�X�^�}�C�Y����B
	data->edit.WindowProc_super = WNDPROC(api::SetWindowLongPtr(
		data->edit.handle, 
		GWLP_WNDPROC, 
		LONG_PTR(&edit_WindowProc_sub)
	));
	// �N���C�A���g�̈�̃}�E�X�J�[�\����EQ��I�r�[���J�[�\���ɂ���B
	api::SetClassLong(
		data->edit.handle, 
		GCL_HCURSOR,
		LONG(api::LoadCursor(
			context.dll_handle, 
			MAKEINTRESOURCE(IDC_EQ_IBEAM)
		))
	);
	// �t�H���g���R���g���[���ɐݒ肷��B
	api::SendMessage(
		data->edit.handle, 
		WM_SETFONT, 
		WPARAM(data->hfont), 
		0
	);
	// Tab���̃n���h���ƃC���f�b�N�X��ǉ�����B
	data->ordered_handles.emplace_back(data->edit.handle);
	data->ordered_indices.emplace(
		data->edit.handle, 
		data->ordered_indices.size()
	);
}

// ���{��`���b�g�ɓ��͂��ꂽ�s���擾����B
std::string // �擾�����s�B
jchat_bar_t::edit_getLine() {
	return window_getText(data->edit.handle);
}

// ���{��`���b�g�̓��͗��őI�����Ă���e�L�X�g��u��������B
// �I�����Ă���e�L�X�g���Ȃ���΁A�L�����b�g�̈ʒu�ɑ}������B
void jchat_bar_t::edit_replaceText(
	const std::string &text // �u��������A�܂��͑}������e�L�X�g�B
) {
	api::SendMessage(
		data->edit.handle, 
		EM_REPLACESEL, 
		TRUE, 
		LPARAM(text.data())
	);
}

// ���{��`���b�g�̓��͗��Ńe�L�X�g��I������B
// ���������ׂďȗ�����Ɛ擪���疖���܂ł̑S�e�L�X�g��I������B
void jchat_bar_t::edit_selectText(
	int first, // �I�����J�n����ʒu�B�ȗ�����ƃ[���B
	int last   // �I�����I������ʒu�B-1�Ȃ疖���B�ȗ������-1�B
) {
	api::SendMessage(data->edit.handle, EM_SETSEL, first, last);
}

// ���{��`���b�g�̓��͗��ɍs��ݒ肷��B
// �I�����[�h�ŁA�Ȃ����X�V�t���O���Z�b�g����Ă���΁A
// �ݒ�O�̍s���u�Ō�ɕҏW����Ă����s�v�Ƃ��ĕۑ�����B
void jchat_bar_t::edit_setLine(
	const std::string &line, // �ݒ肷��s�B�ȗ�����Ƌ󕶎���B
	change_modes change_mode // �ύX���[�h�B�ȗ������EDIT�B
) {
	data->edit.change_mode = change_mode;
	if (change_mode == change_modes::CHOICE && data->edit.dirty) {
		data->edit.last_line = window_getText(data->edit.handle);
		data->edit.dirty = false;
	}
	api::SetWindowText(data->edit.handle, line.c_str());
	data->edit.change_mode = change_modes::EDIT;
}

// ���{��`���b�g�ɓ��͂��ꂽ�s�����s���A���܂ł̔������X�g�ɒǉ�����B
// ������Shift�L�[�������Ȃ�A���s�����A�悭�g���������X�g�ɒǉ�����B
void jchat_bar_t::execute(
	SHORT shift_state // Shift�L�[�̏�ԁB���Ȃ痣��A���Ȃ牟���B
) {
	std::string line = edit_getLine();
	// ���͗�������ۂȂ牽�����Ȃ��B
	if (!line.empty()) {
		if (shift_state >= 0) {
			((*eqgame_t::CEverQuest)->*eqgame_t::CEverQuest_InterpretCmd_target)(
				*eqgame_t::CharSpawn, 
				text_jChatToEQChat(text_addChannel(line)).data()
			);
			data->history.lines.emplace_back(line);
		} else data->registry.lines.emplace_back(line);
		// ���͗�������ۂɂ���B
		edit_setLine();
	}
	// Shift�L�[������A�܂�ʏ�̃R�}���h���s�Ȃ�A
	// ���C���E�C���h�E���A�N�e�B�u�ɂ��AEQ�̑���ɖ߂�B
	if (shift_state >= 0) api::SetActiveWindow(*eqgame_t::MainWindowHandle);
}

// �w�肳�ꂽ�F�̃y�����擾����B
// �܂��w�肳�ꂽ�F�Ńy���̃L���b�V����T���A������΂����Ԃ��B
// ������Ȃ���΂��̐F�̃y�����쐬���A�L���b�V���ɒǉ����ĕԂ��B
HPEN // �擾�����y���̃n���h���B
jchat_bar_t::getPen(
	COLORREF color // �y���̐F�B
) {
	auto iter = data->pen_cache.find(color);
	if (iter != data->pen_cache.end()) return iter->second;
	HPEN hpen = api::CreatePen(PS_SOLID, 1, color);
	data->exits.emplace_back([hpen] {
		api::DeleteObject(hpen);
	});
	data->pen_cache.emplace(color, hpen);
	return hpen;
}

// jchat.ini����悭�g���������X�g��ǂݍ��ށB
// �s��Registry�Z�N�V������Line_#�L�[�ɕۑ�����Ă���B
// ������#�ɂ�0����n�܂�A�Ԃ����ߍ��܂�Ă���B
void jchat_bar_t::loadRegistry() {
	// 0����n�܂�A�ԕt���̃L�[���珇�Ԃɓǂݍ��ށB
	for (int i = 0;; ++i) {
		std::string line = ini_getKeyValue(
			context.ini_path, 
			"Registry", 
			string_printf("Line_%d", i)
		);
		// �擾�����L�[�̒l������ۂȂ�A�����œǂݍ��݂��I������B
		if (line.empty()) break;
		data->registry.lines.emplace_back(line);
	}
	// �����q�����Z�b�g����B
	data->registry.moveEnd();
}

// ���̃R���g���[���Ƀt�H�[�J�X���ړ�����B
void jchat_bar_t::moveFocus(
	HWND ctl_handle,  // �t�H�[�J�X�����R���g���[���̃n���h���B
	SHORT shift_state // Shift�L�[�̏�ԁB���Ȃ痣��A���Ȃ牟���B
) {
	int size = data->ordered_handles.size();
	int index = data->ordered_indices[ctl_handle];
	if (shift_state >= 0) {
		if (++index >= size) index -= size;
	} else if (--index < 0) index += size;
	api::SetFocus(data->ordered_handles[index]);
}

// Shift�L�[��������Ă��Ȃ���΍��܂ł̔������X�g�A������Ă����
// �悭�g���������X�g�̒����ړ����A�ړ���̍s��\������B
// ���L�[�Ȃ��O�Ɉړ����A���L�[�Ȃ����Ɉړ�����B
void jchat_bar_t::moveThroughList(
	SHORT shift_state, // Shift�L�[�̏�ԁB���Ȃ痣��A���Ȃ牟���B
	BYTE vir_key       // �����L�[�̉��z�L�[�R�[�h�B
) {
	// Shift�L�[��������Ă��Ȃ���΍��܂ł̔������X�g�A
	// ������Ă���΂悭�g���������X�g�̒����ړ�����B
	lines_and_iter_t *lni;
	if (shift_state >= 0) lni = &data->history;
	else lni = &data->registry;
	// �����L�[�ɉ����Ĕ����q���ړ����A�ړ���̍s���擾����B
	std::string *line = nullptr;
	if (vir_key == VK_UP) {
		if (!lni->isBegin()) line = &*--lni->iter;
	} else if (!lni->isEnd()) {
		++lni->iter;
		// �����ɓ��B������u�Ō�ɕҏW���Ă����s�v���擾����B
		if (lni->isEnd()) line = &data->edit.last_line;
		else line = &*lni->iter;
	}
	if (line) {
		// ���͗���I�����[�h�ŕύX���A�S�e�L�X�g��I������B
		edit_setLine(*line, change_modes::CHOICE);
		edit_selectText();
		// �ړ����Ȃ��ق��̃��X�g�̔����q�����Z�b�g����B
		if (shift_state >= 0) data->registry.moveEnd();
		else data->history.moveEnd();
	} else edit_selectText(); // �ړ��悪�Ȃ���ΑS�e�L�X�g��I������B
}

// ���{��`���b�g�o�[����������B
// �_�u���o�b�t�@�����O�����ŕ`�悷�邽�߂Ƀr�b�g�}�b�v�A
// �p�^�[���u���V�A�f�o�C�X�R���e�L�X�g�����[�h�E�쐬����B
// �܂��R���g���[���Ŏg�p����t�H���g���쐬����B
void jchat_bar_t::prepare(
	HWND hwnd // �`���b�g�o�[�̃n���h���B
) {
	// ��_�u���V�p�̃r�b�g�}�b�v�����[�h����B
	HBITMAP rock_bar_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ROCK_BAR)
	);
	data->exits.emplace_back([rock_bar_bmp_handle] {
		api::DeleteObject(rock_bar_bmp_handle);
	});
	// ��_�u���V�p�̃r�b�g�}�b�v����p�^�[���u���V���쐬����B
	data->rock_bar_brush_handle = 
		api::CreatePatternBrush(rock_bar_bmp_handle);
	data->exits.emplace_back([this] {
		api::DeleteObject(data->rock_bar_brush_handle);
	});
	// ��ǃu���V�p�̃r�b�g�}�b�v�����[�h����B
	HBITMAP rock_wall_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ROCK_WALL)
	);
	data->exits.emplace_back([rock_wall_bmp_handle] {
		api::DeleteObject(rock_wall_bmp_handle);
	});
	// ��ǃu���V�p�̃r�b�g�}�b�v����p�^�[���u���V���쐬����B
	data->rock_wall_brush_handle = 
		api::CreatePatternBrush(rock_wall_bmp_handle);
	data->exits.emplace_back([this] {
		api::DeleteObject(data->rock_wall_brush_handle);
	});
	// �E�C���h�E�̃f�o�C�X�R���e�L�X�g���擾����B
	HDC hdc = api::GetDC(hwnd);
	scope_exit_t hdc_exit([hwnd, hdc] {
		api::ReleaseDC(hwnd, hdc);
	});
	// �A�N�e�B�u�A�C�R���̃r�b�g�}�b�v�����[�h����B
	HBITMAP icon_active_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ICON_ACTIVE)
	);
	data->exits.emplace_back([icon_active_bmp_handle] {
		api::DeleteObject(icon_active_bmp_handle);
	});
	// �A�N�e�B�u�A�C�R���̃f�o�C�X�R���e�L�X�g���쐬����B
	data->icon_active_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->icon_active_dc_handle);
	});
	api::SelectObject(data->icon_active_dc_handle, icon_active_bmp_handle);
	// �C���A�N�e�B�u�A�C�R���̃r�b�g�}�b�v�����[�h����B
	HBITMAP icon_inactive_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_ICON_INACTIVE)
	);
	data->exits.emplace_back([icon_inactive_bmp_handle] {
		api::DeleteObject(icon_inactive_bmp_handle);
	});
	// �C���A�N�e�B�u�A�C�R���̃f�o�C�X�R���e�L�X�g���쐬����B
	data->icon_inactive_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->icon_inactive_dc_handle);
	});
	api::SelectObject(data->icon_inactive_dc_handle, icon_inactive_bmp_handle);
	// �܂݂̃r�b�g�}�b�v�����[�h����B
	HBITMAP knob_bmp_handle = api::LoadBitmap(
		context.dll_handle, 
		MAKEINTRESOURCE(IDB_KNOB)
	);
	data->exits.emplace_back([knob_bmp_handle] {
		api::DeleteObject(knob_bmp_handle);
	});
	// �܂݂̃f�o�C�X�R���e�L�X�g���쐬����B
	data->knob_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->knob_dc_handle);
	});
	api::SelectObject(data->knob_dc_handle, knob_bmp_handle);
	// �o�b�t�@�̃r�b�g�}�b�v���쐬����B
	HBITMAP buffer_bmp_handle = api::CreateCompatibleBitmap(
		hdc, 
		api::GetSystemMetrics(SM_CXSCREEN), 
		JCHAT_BAR_HEIGHT
	);
	data->exits.emplace_back([buffer_bmp_handle] {
		api::DeleteObject(buffer_bmp_handle);
	});
	// �o�b�t�@�̃f�o�C�X�R���e�L�X�g���쐬����B
	data->buffer_dc_handle = api::CreateCompatibleDC(hdc);
	data->exits.emplace_back([this] {
		api::DeleteDC(data->buffer_dc_handle);
	});
	api::SelectObject(data->buffer_dc_handle, buffer_bmp_handle);
	// jchat.ini�Őݒ肳��Ă��閼�O�Ńt�H���g���쐬����B
	data->hfont = api::CreateFont(
		FONT_SIZE,
		0,
		0,
		0, 
		FW_NORMAL,
		FALSE, 
		FALSE,
		FALSE,
		SHIFTJIS_CHARSET, 
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY,
		DEFAULT_PITCH, 
		context.ini.chat.font_name.c_str()
	);
	data->exits.emplace_back([this] {
		api::DeleteObject(data->hfont);
	});
}

// jchat.ini�ɂ悭�g���������X�g���������ށB
// �s��Registry�Z�N�V������Line_#�L�[�ɕۑ�����B
// ������#�ɂ�0����n�܂�A�Ԃ𖄂ߍ��ށB
void jchat_bar_t::storeRegistry() {
	// �܂��Â�Registry�Z�N�V�������폜����B
	ini_deleteSection(context.ini_path, "Registry");
	// 0����n�܂�A�ԕt���̃L�[�ɏ��Ԃɏ������ށB
	data->registry.moveBegin();
	for (std::size_t i = 0; !data->registry.isEnd(); ++i)
		ini_setKeyValue(
			context.ini_path, 
			"Registry", 
			string_printf("Line_%d", i), 
			*data->registry.iter++
		);
}

// ���{��`���b�g�̃e�L�X�g�̑O�Ƀ`�����l����t������B
// �������e�L�X�g���R�}���h�Ȃ牽�����Ȃ��B
std::string // �`�����l����t�������e�L�X�g�B
jchat_bar_t::text_addChannel(
	const std::string &jc_text // ���{��`���b�g�̃e�L�X�g�B
) {
	// �ŏ��̕�������R�}���h�ł��邩�ǂ����𔻒肷��B
	if (context.ini.chat.command_symbols.find(
			jc_text.front()
		) != std::string::npos
	) return jc_text;
	// �I�𒆂̃`�����l�����e�L�X�g�̑O�ɕt������B
	int ch_index = api::SendMessage(data->channel.handle, CB_GETCURSEL, 0, 0);
	return CHANNEL_TEXTS[ch_index] + " " + jc_text;
}

// EQ�`���b�g�̃e�L�X�g����{��`���b�g�̃e�L�X�g�ɕϊ�����B
// �e�L�X�g�ɃA�C�e�������N���܂܂�Ă���ΓƎ��̌`���ɕϊ�����B
// EQ�`���b�g����̃e�L�X�g��Ascii�Ȃ̂ŃG���R�[�f�B���O�͂��̂܂܂ł悢�B
std::string // �ϊ��������{��`���b�g�̃e�L�X�g(Ascii)�B
jchat_bar_t::text_eqChatToJChat(
	const char *eqc_text // �ϊ�����EQ�`���b�g�̃e�L�X�g(Ascii)�B
) {
	std::ostringstream jc_text_out;
	// EQ�`���b�g�ɂ����郊���N�͓���ȕ����ł���0x12�ň͂܂�Ă���B
	// ���̂��߁A�܂�0x12���������炻���܂ł̓��e�����ƂȂ�A
	// ����0x12�܂ł̓����N�̓��e�ƂȂ�A�ȍ~�J��Ԃ��ƂȂ�B
	for (;;) {
		// �����N�̎n�[(0x12)��T���B
		char *begin = std::strchr(
			const_cast<char*>(eqc_text), 
			EQCHAT_LINK_EDGE
		);
		// ������Ȃ������甲����B
		if (!begin) break;
		// �I�[(0x12)��T���B
		char *end = std::strchr(
			begin + 1, 
			EQCHAT_LINK_EDGE
		);
		// ������Ȃ������甲����B
		if (!end) break;
		// �n�[�܂ł̃��e�������o�͂���B
		jc_text_out << std::string(eqc_text, begin - eqc_text);
		// ���e���\���ɒ�����ΗL���ȃ����N�Ƃ݂Ȃ��B
		std::size_t link_len = end - begin - 1;
		if (link_len > context.ini.chat.link_body_size) {
			// ���̂悤�ȓƎ��̌`���ɕϊ����ďo�͂���B
			// {�ԍ�:���O}
			std::size_t name_len = link_len - context.ini.chat.link_body_size;
			char *name = begin + 1 + context.ini.chat.link_body_size;
			jc_text_out << string_printf(
				"%c%d%c%.*s%c", 
				JCHAT_LINK_OPEN,          // �n�[�̋L��('{')�B
				data->link_bodies.size(), // �ԍ��B
				JCHAT_LINK_DIVIDER,       // �ԍ��Ɩ��O�𕪊�����L��(':')�B
				name_len,                 // ���O�̒����B
				name,                     // ���O�B
				JCHAT_LINK_CLOSE          // �I�[�̋L��('}')�B
			);
			// �����N�̃{�f�B�����́A��Ń����N���܂񂾍s��
			// EQ�`���b�g�̌`���ɖ߂���悤�ɕۑ����Ă����B
			data->link_bodies.emplace_back(
				begin + 1, 
				context.ini.chat.link_body_size
			);
		}
		// �I�[�̎��̕�������J��Ԃ��B
		eqc_text = end + 1;
	}
	// �����N��������Ȃ������烊�e�����Ƃ��ďo�͂���B
	jc_text_out << eqc_text;
	return jc_text_out.str();
}

// ���{��`���b�g�̃e�L�X�g��EQ�`���b�g�̃e�L�X�g�ɕϊ�����B
// �e�L�X�g�ɃA�C�e�������N���܂܂�Ă���Ό��̌`���ɖ߂��B
// ���{��`���b�g�̃e�L�X�g�̓V�t�gJIS�Ȃ̂�utf-8�ɕϊ�����B
std::string // �ϊ�����EQ�`���b�g�̃e�L�X�g(utf-8)�B
jchat_bar_t::text_jChatToEQChat(
	const std::string &jc_text // �ϊ�������{��`���b�g�̃e�L�X�g(�V�t�gJIS)�B
) {
	std::ostringstream eqc_text_out;
	// EQ�`���b�g�����{��`���b�g�Ɠ����悤�ɃA�C�e�������N��
	// �ϊ��𒆐S�Ƃ��āA�S�p�������l�����Ȃ��珈�����Ă����B
	for (auto iter = jc_text.begin(); iter != jc_text.end();) {
		// �S�p�������l�������1�������������Ă��������Ȃ��B
		char chr = *iter++;
		if (chr == JCHAT_LINK_OPEN) {
			// �����N�̎n�[('{')�ł���΁A�����L��(':')�܂ł͔ԍ��ƂȂ�B
			std::size_t index = UINT_MAX;
			std::ostringstream index_out;
			while (iter != jc_text.end()) {
				chr = *iter++;
				if (chr == JCHAT_LINK_DIVIDER) {
					index = destringize<std::size_t,UINT_MAX>(index_out.str());
					break;
				}
				index_out << chr;
				if (iter != jc_text.end() && char_isJLead(chr)) 
					index_out << *iter++;
			}
			// �����L��(':')����I�[('}')�܂ł͖��O�ƂȂ�B
			std::string name;
			std::ostringstream name_out;
			while (iter != jc_text.end()) {
				chr = *iter++;
				if (chr == JCHAT_LINK_CLOSE) {
					name = name_out.str();
					break;
				}
				name_out << chr;
				if (iter != jc_text.end() && char_isJLead(chr)) 
					name_out << *iter++;
			}
			// �ԍ��Ɩ��O���L���Ȃ�AEQ�`���b�g�̌`���ŏo�͂���B
			if (index < data->link_bodies.size() && !name.empty()) 
				eqc_text_out <<
					BYTE(EQCHAT_LINK_EDGE) <<   // �n�[(0x12)�B
					data->link_bodies[index] << // �{�f�B�����B
					name <<                     // ���O�B
					BYTE(EQCHAT_LINK_EDGE);     // �I�[(0x12)�B
		} else {
			// �n�[('{')�łȂ���΁A���̂܂܏o�͂���B
			eqc_text_out << chr;
			if (iter != jc_text.end() && char_isJLead(chr)) 
				eqc_text_out << *iter++;
		}
	}
	// �����G���R�[�f�B���O���V�t�gJIS����utf-8�ɕϊ�����B
	return string_sjisToUtf8(eqc_text_out.str());
}

// jchat.log�ɃG���[���O���������݁Ajchat.dll���~����B
// Detour�����������̂ŁAEQ�`���b�g�͌��ɖ߂�B
void context_t::fail(
	const error &err // ���s�̌����ƂȂ����G���[�B
) {
	if (log) putLog(*log, err.getMessage());
	release();
}

// jchat.dll�Ŏg�p�������ׂẴ��\�[�X���������B
// �`���b�g�o�[��j�����ADetour���������Ajchat.log�����B
void context_t::release() {
	if (jchat_bar.getHandle()) api::DestroyWindow(jchat_bar.getHandle());
	try {
		// Detour�p�̒E�o�������N���A����ƁA����܂�Attach����
		// ���ׂĂ̊֐��ɑ΂���DetourDetach�֐����Ăяo�����B
		// Detach�̓g�����U�N�V�����̒��ōs���B
		dtr::DetourTransactionBegin();
		scope_exit_t abort_exit([] {
			dtr::DetourTransactionAbort();
		});
		detour_exits.clear();
		dtr::DetourTransactionCommit();
		abort_exit.procedure() = nullptr;
	} catch (const error &err) {
		if (log) putLog(*log, err.getMessage());
	}
	reverseClear(exits);
	log.reset();
}

// �e�t�@�C���̃p�X��ݒ肵�Ajchat.log���J���Ajchat.ini����ǂݍ��ށB
// �܂�eqgame.exe�̃x�[�X�A�h���X���擾����B
void context_t::startup(
	HINSTANCE hinstDLL // DLL�C���X�^���X�̃n���h���B
) {
	dll_handle = hinstDLL;
	dll_path = module_getPath(dll_handle);
	// �R�}���h���C����������͂���B
	auto command_line = namedValuesToMap(parseArguments(api::GetCommandLine()));
	// �R�}���h���C��������jchat.log_path���烍�O�t�@�C���̃p�X���擾����B
	// �ȗ��Ȃ�EQ�t�H���_��jchat.log�̃p�X�ɂȂ�B
	log_path = dll_path;
	log_path.replace_extension("log");
	log_path = map_find(
		command_line, 
		log_path.filename().string() + "_path", 
		log_path.string()
	);
	// �R�}���h���C��������jchat.ini_path����ݒ�t�@�C���̃p�X���擾����B
	// �ȗ��Ȃ�EQ�t�H���_��jchat.ini�̃p�X�ɂȂ�B
	ini_path = dll_path;
	ini_path.replace_extension("ini");
	ini_path = map_find(
		command_line, 
		ini_path.filename().string() + "_path", 
		ini_path.string()
	);
	log = std_::make_ofstream(log_path.string(), std::ios_base::app);
	// �`���b�g�Ɋւ���Z�N�V������ǂݍ��ށB
	ini.chat.command_symbols = ini_getKeyValue(
		ini_path, 
		"Chat", 
		"CommandSymbols", 
		INI_CHAT_COMMAND_SYMBOLS_DEF
	);
	ini.chat.font_name = ini_getKeyValue(
		ini_path, 
		"Chat", 
		"FontName", 
		INI_CHAT_FONT_NAME_DEF
	);
	ini.chat.link_body_size = destringize<
		std::size_t,
		INI_CHAT_LINK_BODY_SIZE_DEF
	>(
		ini_getKeyValue(
		ini_path, 
		"Chat", 
		"LinkBodySize", 
		stringize(INI_CHAT_LINK_BODY_SIZE_DEF)
	));
	// �E�C���h�E�Ɋւ���Z�N�V������ǂݍ��ށB
	ini.window.left = destringize<
		int,
		INI_WINDOW_LEFT_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"Left", 
		stringize(INI_WINDOW_LEFT_DEF)
	));
	ini.window.min_width = destringize<
		std::size_t,
		INI_WINDOW_MIN_WIDTH_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"MinWidth", 
		stringize(INI_WINDOW_MIN_WIDTH_DEF)
	));
	ini.window.top = destringize<
		int,
		INI_WINDOW_TOP_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"Top", 
		stringize(INI_WINDOW_TOP_DEF)
	));
	ini.window.width = destringize<
		std::size_t,
		INI_WINDOW_WIDTH_DEF
	>(ini_getKeyValue(
		ini_path, 
		"Window", 
		"Width", 
		stringize(INI_WINDOW_WIDTH_DEF)
	));
	// �`���b�g�o�[�̏��������ŏ�����菬������ΏC������B
	clampByMin(ini.window.width, ini.window.min_width);
	base_address = DWORD(api::GetModuleHandle(NULL));
}

//// �O���[�o���ϐ��̒�`

// DispatchMessage��Target�֐��|�C���^�B
DispatchMessage_t DispatchMessage_target = ::DispatchMessage;

context_t context; // �v���O�����̕���(��)�B

//// �֐��̒�`

// DispatchMessage��Detour�֐��B
// �`���b�g�o�[���A�N�e�B�u�ɂȂ��Ă���Ƃ������ADispatchMessage��
// Trampoline�֐����Ăяo���O��TranslateMessage�֐����Ăяo���B
// ����ɂ��A���C���E�C���h�E�ő��쒆��IME���������Ȃ��悤�ɂ��A
// �`���b�g�o�[�̃R���g���[����WM_CHAR�����M�����悤�ɂȂ�B
LRESULT // ���ʁB
WINAPI DispatchMessage_detour(
	const MSG *msg // �z�����郁�b�Z�[�W�B
) {
	if (api::GetActiveWindow() == context.jchat_bar.getHandle()) 
		api::TranslateMessage(msg);
	return DispatchMessage_target(msg);
}

// jchat.ini����A�h���X��ǂݍ��݁A�A�h���X���v�Z���A
// eqgame_t�ɑ�����f�[�^��Target�֐��|�C���^�ɑ������B
// �����Ă����̊֐��ɂ���DetourAttach�֐����Ăяo���B
// ���̊֐�(early)��jchat.dll�����[�h���ꂽ�Ƃ��ɌĂяo�����B
// ���̊֐��ł̓Q�[���̏�Ԃ��擾�ł���֐��݂̂�Attach����B
// ����ȊO�̊֐��͌�ɌĂяo�����֐�(lazy)�̂ق���Attach����B
// �����MQ2��KeypressHandler_HandleKeyDown�̂悤�Ȋ֐���Attach
// �������Attach���邱�ƂŁA��Ɏ��s�����悤�ɂ��邽�߂ł���B
void bindAddressAndDetourAttach_early() {
	// Attach�̓g�����U�N�V�����̒��ōs���B
	BEGIN_DETOUR_ATTACH();
		BIND_ADDRESS(CEverQuest);
		BIND_ADDRESS(CharSpawn);
		BIND_ADDRESS(MainWindowHandle);
		BIND_ADDRESS(NotInChatMode);
		BIND_ADDRESS_AND_DETOUR_ATTACH(CEverQuest_SetGameState);
		BIND_ADDRESS_AND_DETOUR_ATTACH(ProcessGameEvents);
	END_DETOUR_ATTACH();
}

// jchat.ini����A�h���X��ǂݍ��݁A�A�h���X���v�Z���A
// eqgame_t�ɑ�����Target�֐��|�C���^�ɑ������B
// �����Ă����̊֐��ɂ���DetourAttach�֐����Ăяo���B
// (DispatchMessage�֐������łɂ�����Attach����B)
// ���̊֐�(lazy)�̓`���b�g�o�[��\�����钼�O�ɌĂяo�����B
// MQ2�����Detour��ǉ����邱�ƂŁA�C�x���g�̑r����h���B
void bindAddressAndDetourAttach_lazy() {
	// Attach�̓g�����U�N�V�����̒��ōs���B
	BEGIN_DETOUR_ATTACH();
		DETOUR_ATTACH(DispatchMessage);
		BIND_ADDRESS_AND_DETOUR_ATTACH(CEverQuest_InterpretCmd);
		BIND_ADDRESS_AND_DETOUR_ATTACH(CXWndManager_DrawCursor);
		BIND_ADDRESS_AND_DETOUR_ATTACH(KeypressHandler_HandleKeyDown);
		BIND_ADDRESS_AND_DETOUR_ATTACH(KeypressHandler_HandleKeyUp);
	END_DETOUR_ATTACH();
}

// jchat.ini��Offset�Z�N�V��������I�t�Z�b�g��ǂݍ��݁A
// eqgame.exe�̃�������Ԃɂ������΃A�h���X���擾����B
DWORD // �擾�����A�h���X�B
getAddress(
	const std::string &name // �I�t�Z�b�g�̖��O�B
) {
	return context.base_address + getOffset(name) - 0x400000ul;
}

// jchat.ini��Offset�Z�N�V��������I�t�Z�b�g��ǂݍ��ށB
// �I�t�Z�b�g�̐ݒ�l�̌`����16�i���ł���B
// �I�t�Z�b�g���[���Ȃ�s���Ƃ݂Ȃ��G���[���X���[����B
DWORD // �擾�����I�t�Z�b�g�B
getOffset(
	const std::string &name // �I�t�Z�b�g�̖��O�B
) {
	DWORD offset = destringize<DWORD>(ini_getKeyValue(
		context.ini_path, 
		"Offset", 
		name,
		"0"
	), std::hex);
	if (!offset)
		throw error(string_printf(
			"Offset�Z�N�V������%s�L�[���s���ł��B", 
			name.c_str()
		));
	return offset;
}

// ��ʃ��[�h���擾����B
screen_modes // �擾������ʃ��[�h�B
getScreenMode() {
	if (api::IsIconic(*eqgame_t::MainWindowHandle)) 
		return screen_modes::ICONIC;
	if (window_isTopMost(*eqgame_t::MainWindowHandle))
		return screen_modes::FULL_SCREEN;
	return screen_modes::WINDOW;
}

// �Q�[���̏�Ԃ��ύX���ꂽ�Ƃ��ɌĂяo�����B
// �Q�[���̏�Ԃ�INGAME�Ȃ�`���b�g�o�[��\�����A
// ����ȊO�Ȃ�`���b�g�o�[���\���ɂ���B
// �G���[������������jchat.dll�̋@�\���~����B
void onGameStateChange() {
	try {
		auto &bar = context.jchat_bar;
		if (context.game_state == GAMESTATE_INGAME) {
			// �`���b�g�o�[���쐬���Ă��Ȃ���΍쐬����B
			if (!bar.getHandle()) {
				// ��ʃ��[�h������������B
				context.screen_mode = getScreenMode();
				// early�ō�����ȊO�̊֐���Detour�����B
				bindAddressAndDetourAttach_lazy();
				// �`���b�g�o�[�̃E�C���h�E�N���X��o�^����B
				jchat_bar_t::registerClass();
				// �`���b�g�o�[���쐬����B
				bar.create();
			}
			// ��\���Ȃ�\�����A�O�̂��߂Ɉʒu�Y�����C������B
			if (!api::IsWindowVisible(bar.getHandle())) {
				api::ShowWindow(bar.getHandle(), SW_SHOWNA);
				bar.fixPos();
			}
		} else if (bar.getHandle() && 
			api::IsWindowVisible(bar.getHandle())
		) api::ShowWindow(bar.getHandle(), SW_HIDE);
	} catch (const error &err) {
		context.fail(err);
	}
}

// ����DLL�����[�h���ꂽ�Ƃ��ɌĂяo�����B
// �v���O�������J�n���A�ꕔ�̊֐���Detour�����B
// �G���[������������jchat.log�ɏ�������ŁA���s��Ԃ��B
BOOL // �^�Ȃ琬���A�U�Ȃ玸�s�B
onProcessAttach(
	HINSTANCE hinstDLL // DLL�C���X�^���X�̃n���h���B
) {
	try {
		context.startup(hinstDLL);
		bindAddressAndDetourAttach_early();
	} catch (const error &err) {
		context.fail(err);
		return FALSE;
	}
	return TRUE;
}

// ����DLL���A�����[�h�����Ƃ��ɌĂяo�����B
// �`���b�g�o�[��j�����ADetour���������Ajchat.log�����B
BOOL // �^�Ȃ琬���A�U�Ȃ玸�s�B
onProcessDetach() {
	context.release();
	return TRUE;
}

// ��ʃ��[�h���ύX���ꂽ�Ƃ��ɌĂяo�����B
// �ύX�ɂ���Đ�����`���b�g�o�[�̈ʒu�Y�����C������B
void onScreenModeChange() {
	try {
		auto &bar = context.jchat_bar;
		if (bar.getHandle() &&
			api::IsWindowVisible(bar.getHandle())
		) bar.fixPos();
	} catch (const error &err) {
		context.fail(err);
	}
}

// �Q�[���̏�Ԃ��ŐV�̏�ԂɍX�V����B
// �Q�[���̏�Ԃɉ����ă`���b�g�o�[�̕\����؂�ւ���B
void refreshGameState(
	int new_game_state // �V�����Q�[���̏�ԁB
) {
	// �Q�[���̏�Ԃ��[���Ȃ疳������B
	// �[���ł͂Ȃ��A�Ȃ����ύX����Ă���Εۑ����A�n���h�����Ăяo���B
	if (new_game_state && new_game_state != context.game_state) {
		context.game_state = new_game_state;
		onGameStateChange();
	}
}

// ��ʃ��[�h���ŐV�̏�ԂɍX�V����B
// ��ʃ��[�h�ɉ����ă`���b�g�o�[�̈ʒu�Y�����C������B
void refreshScreenMode() {
	// ��ʃ��[�h���ύX����Ă���Εۑ����A�n���h�����Ăяo���B
	screen_modes new_screen_mode = getScreenMode();
	if (new_screen_mode != context.screen_mode) {
		context.screen_mode = new_screen_mode;
		onScreenModeChange();
	}
}

}

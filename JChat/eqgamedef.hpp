//// JEQ/日本語チャットプラグイン/eqgame.exeの定数や型の定義

#ifndef JEQ_EQGAMEDEF_HPP
#define JEQ_EQGAMEDEF_HPP

//// インクルード

#include <windows.h>

//// 定数の定義

#define GAMESTATE_CHARSELECT    1
#define GAMESTATE_CHARCREATE    2
#define GAMESTATE_SOMETHING     4
#define GAMESTATE_INGAME        5
#define GAMESTATE_PRECHARSELECT -1
#define GAMESTATE_LOGGINGIN     253
#define GAMESTATE_UNLOADING     255

namespace jeq {

//// 型の定義

typedef struct _CHATCHANNEL_TEXTS {
/*0x000*/ PCHAR  ChannelName[0xa];
/*0x004*/
} CHATCHANNEL_TEXTS, *PCHATCHANNEL_TEXTS;

typedef struct _CHATSERVICE {
/*0x000*/ BYTE   Unknown0x0[0xc];
/*0x00c*/ struct _CHATCHANNEL_TEXTS *ChannelList;
/*0x010*/ DWORD  ActiveChannels;   // number of channels joined
/*0x014*/ BYTE   Unknown0x14[0x8];
/*0x01c*/ CHAR   ChatID[0x30];     // "ServerName.CharName" (0x60?)
/*0x04c*/ BYTE   Unknown0x4c[0x30];
/*0x07c*/ CHAR   ServerName[0x30]; // "ServerName."
/*0x0b0*/ // more data
} CHATSERVICE, *PCHATSERVICE;

typedef struct _EVERQUEST {
/*0x000*/ BYTE   Unknown[0x2a4];
/*0x2a4*/ struct _CHATSERVICE *ChatService;
/*0x2a8*/ BYTE   Unknown0x2a8[0x8];
/*0x2b0*/ bool   bJoinedChannel;
/*0x2b1*/ CHAR   ChannelPlayerName[0x100];
/*0x3b1*/ CHAR   ChannelName[0xa][0x30];
/*0x591*/ BYTE   Unknown0x591[0x3];
/*0x594*/ DWORD  ChannelNumber[0xa];
/*0x5bc*/ DWORD  ChannelQty;
/*0x5c0*/ BYTE   Unknown0x5c0[0x8];
/*0x5c8*/ DWORD  GameState;
/*0x5cc*/ // more data
} EVERQUEST, *PEVERQUEST;

struct KeyCombo {
	BYTE alt;
	BYTE ctrl;
	BYTE shift;
	BYTE key;
};

}

#endif // #ifndef JEQ_EQGAMEDEF_HPP

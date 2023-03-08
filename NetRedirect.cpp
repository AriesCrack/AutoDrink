// dllmain.cpp : Define o ponto de entrada para o aplicativo DLL.
#include "pch.h"
#include "NetRedirect.h"
#include "Common.h"
#include <conio.h>
#include <atlstr.h>
#include <mutex>
// load Microsoft Detour Lib
#include "detours.h"
#include <vector>
#include <mmsystem.h>
#include <cmath>
#include <queue>
#include <sstream>

#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "detours.lib")

using namespace std;

#define RETYR_NUM 50000
#define NON_ITEM_AMOUNT 888888
#define CharNameLen 24

#define TIMEOUT_YEAR 2023
#define TIMEOUT_MONTH 3
#define TIMEOUT_DAY 7

int* AidAddress = reinterpret_cast <int*>(0x012E622C); // NORACAT, search: 11281721  會長號: 12100031
int* CharName = reinterpret_cast <int*>(0x012EC228); // NORACAT
int CHAR_NAME = 0x012EC228; //NORACAT 010D7540  25b4
int* HPIndexValue = reinterpret_cast <int*>(0x012E9C20);
int* MaxHPTableValue = reinterpret_cast <int*>(0x012E9C24);
int* SPIndexValue = reinterpret_cast <int*>(0x012E9C28); //0x010C287C; //29549
int* MaxSPTableValue = reinterpret_cast <int*>(0x012E9C2C);
int* CLOAKING_MEMORY_REGION = reinterpret_cast <int*>(0x01059CA0);
// 1. 穿狂蟻跟放偽裝
// 2. 搜尋 8bytes, 000F423F000000B8(只會找到一個)  坐下: 000f423f0000026e
// 3. 搜尋 4bytes, 搜剛剛搜到的memory位置address, 失敗就-4繼續搜
int* MAP_NAME = reinterpret_cast <int*>(0x00F0FED8);  // 要搜到順位第二個
//
// 1. 搜尋8 BYTES, FFFFFFFF
// 2. 一個一個填2035找
//
int* MONSTER_REGION_1 = reinterpret_cast <int*>(0x012EA09C); // 2035細長華金 3468西瓜  3455盤子  1085香菇  2166龍蛋  1109小惡魔
//int* MONSTER_REGION_2 = reinterpret_cast <int*>(0x01297AEC);
//int* MONSTER_REGION_3 = reinterpret_cast <int*>(0x01297AEC);
int* STATUS_INDEX = reinterpret_cast <int*>(0x012E9E88); // 622
int* WINDOWS_LOCK_1 = reinterpret_cast <int*>(0x00F0FED4); // 鎖定視角  // 第一個地圖的前四個byte
int* PLAYER_CORRD_X = reinterpret_cast <int*>(0x012D2A04); // 人物的X座標  必須搜尋8 HEX 含X跟Y
int* PLAYER_CORRD_Y = reinterpret_cast <int*>(0x012D2A08); // 人物的Y座標
void* PACKET_RECV_ADDRESS = (void*)0x12D2A18; // 封包接收的地址, 人物座標X + 0x14
//int* PC_ACTION_REGION_1 = reinterpret_cast <int*>(0x00AD99BC); // 人物動作改檔 搜尋009D86C6
//int* PC_ACTION_REGION_2 = reinterpret_cast <int*>(0x00AD99C0); // 人物動作改檔

//byte* ATTACK_PACKAGE_REGION = reinterpret_cast <byte*>(0x019349C); // 3804 攻擊封包的儲存位置
// 主教放自己天賜
// 搜尋Aarray
// 38 04 0A 00 22 00 39 25 AC

__int64* ExpValue = reinterpret_cast <__int64*>(0x012E6240);
__int64* MaxExpValue = reinterpret_cast <__int64*>(0x012E6248);
__int64* MaxJobValue = reinterpret_cast <__int64*>(0x012E6250);
__int64* JobValue = reinterpret_cast <__int64*>(0x012E6258);

int* GM_AID_ADDRESS = reinterpret_cast <int*>(0x01286DF0);
//int* GM_AID_ADDRESS_2 = reinterpret_cast <int*>(0x012B22B8);
// 先搜尋23769077 (2000000) , 嘗試改成12100031, 確定是GM後
// 搜尋該位置減去0x4, 就是該memory位置

#if !PACKET_MODE
int* AUTO_HOTKEY_LOCATION_1 = reinterpret_cast <int*>(0x012E51E2); // 自動料理快捷鍵最後的位置
int* AUTO_HOTKEY_LOCATION_2 = reinterpret_cast <int*>(0x012E51E6); // 往後4
int* AUTO_HOTKEY_LOCATION_3 = reinterpret_cast <int*>(0x012E51DE); // 往前4
// 將最後一個快捷鍵放新手專用翅膀, 搜尋 12323, 2byte, fast scan關掉
// 蕃薯 516
#endif

#if !PrivateServerOrNot
int* MoneyIndexValue = reinterpret_cast <int*>(0x012E6300);
int* MaxWeightTableValue = reinterpret_cast <int*>(0x012E630C);
int* WeightIndexValue = reinterpret_cast <int*>(0x012E6310);
int* HOMUN_INTIMACY = reinterpret_cast <int*>(0x012E9C8C); // 生命體親密度
int* PET_HUNGRY = reinterpret_cast <int*>(0x12E5C78); // 寵物飢餓度 名子位置 + 0x3C

int* DAT_FILE_PATH_INDEX = reinterpret_cast <int*>(0x00F32508); // 總檔案路徑  搜尋data\

//int* EMB_FILE_PATH_INDEX_1 = reinterpret_cast <int*>(0x00D87DD7); // 總檔案路徑  搜尋_tmpEmblem\ 要把Writable用灰色, 有三個, 分別為第二第三
//int* EMB_FILE_PATH_INDEX_2 = reinterpret_cast <int*>(0x00D9105F);

//int* RESOLUTION_CORRD_X = reinterpret_cast <int*>(0x01004328); // 解析度X 搜尋1440, 找到後往上移, 游標亂移動, 有跳動就是
//int* RESOLUTION_CORRD_Y = reinterpret_cast <int*>(0x0100432C); // 解析度Y
int* CURSOR_CORRD_X = reinterpret_cast <int*>(0x00EF1A50); // 游標X 搜尋上面游標 找到可以改的
int* CURSOR_CORRD_Y = reinterpret_cast <int*>(0x00EF1A54); // 游標Y
//int* REAL_CORRD_X = reinterpret_cast <int*>(0x00ED0188); // 真實綠框X 搜尋自身座標下面的XY
//int* REAL_CORRD_Y = reinterpret_cast <int*>(0x00ED018C); // 真實綠框Y

// 幻覺搜尋流程: 10CD79E8 or 00002710, 把2710 改成0000
// 先找一隻基因中幻覺, 中了之後搜尋00002710,
// 他會在Ragexe的範圍之間
// 搜尋00002710找code
// 將call function 的位置 往下移到解幻覺的 move [xxx], 00000000
// 
// 搜尋array 10 27 00 00 C6 05
// go to address mov [] , 2710
int* ILLUSION_DOPING_REGION = reinterpret_cast <int*>(0x00F405D4); // 改變2710的絕對位置

int* GUILD_CURRENT = reinterpret_cast <int*>(0x012877B8); // 當前公會ID
// 搜尋 1593944 EG的公會ID

#endif

//string gRoWindowsName = "Ragnarok";
//string gRoWindowsName1 = "Ragnarok1";

SYSTEMTIME sys;

// 地圖資料
char** gMapData = NULL;
int gWidth = 0;
int gHeight = 0;

//
// Define Packet value
//
int ItemListStart = 0x0B08;
int ItemListStackable = 0x0B09;
int ItemListNonStackable = 0x0B39;
int ItemListEnd = 0x0B0B;
int ItemListEndSize = 0x0000;
int InventoryUsed = 0x01C8;
int InventoryItemAdded = 0x0B41;
int InventoryItemRemoveD = 0x07FA;
int InventoryRemoved = 0x00AF;
int MapProperty = 0x0091;
int SkillDamage = 0x01DE;
int SkillUsedNoDamage = 0x09CB;
int SkillUseFailed = 0x0110;
int SkillCast = 0x07FB;
int ActorLost = 0x0080;
int ActorGetInfo = 0x0A30;
int ActorMoved = 0x09FD;
int ActorConnected = 0x09FE;
int ActorExists = 0x09FF;
int CharacterMoves = 0x0086;
int ActorPosition = 0x0088;
int HighJump = 0x08D2;
int ItemAppeared = 0x0ADD;
int ItemExists = 0x009D;
int MonkSpirits = 0x01D0;
int SystemChat = 0x009A;
int EquipItem = 0x0999;
int UnequipItem = 0x099A;
int MapLoaded = 0x007D;
int StatInfo = 0x00B0;
int PartyLeader = 0x07FC;
int PartyLeave = 0x0105;
int GuildEmblem = 0x0152;
int GuildAlliesEnemyList = 0x014C;
#if !PrivateServerOrNot
int PartyJoin = 0x0A43;
int PartyUsersInfo = 0x0A44;
int SkillList = 0x0B32;
#else 
int PartyJoin = 0x0AE4;
int PartyUsersInfo = 0x0AE5;
int SkillList = 0x010F;
#endif

int LOGIN_RSW = 0x69676F6C;  // Login.rew text

HMODULE hModule;
//HANDLE hThread;
//bool keepMainThread = true;

//// Connection to the X-Kore server that Kore created
//static SOCKET koreClient = INVALID_SOCKET;
//static bool koreClientIsAlive = false;
static SOCKET roServer = INVALID_SOCKET;
//static string roSendBuf("");	// Data to send to the RO client
//static string xkoreSendBuf("");	// Data to send to the X-Kore server
bool imalive = false;

//
// Global variable
//
SOCKET s_server;

mutex gMutex;
mutex gMutex2;
mutex gMutexSkill;
mutex gMutexItem;
mutex gMutexPlayerList;
mutex gMutexTarget;

mutex gMutexMapData;

// 角色資料
char path[50] = ".\\data\\setting\\";
char gCharNameStr[CharNameLen] = { "" };
int gPassFlag = 0; // 紀錄序號通過
int gIsConnectFlag = 0; // 紀錄連線成功

int MapLoadedFlag = 0;

#if !PrivateServerOrNot
int ActorAction = 0x02E1;  // 私服 0x08C8

int AidSkillBufferOffset = 8; // 私服 6
int ItemBufferOffset = 4; // 私服 2

int ItemListStartLen = 5; // 私服 6
int ActorGetInfoLen = 109; // 私服 106
int ItemListEndLen = 7; // 私服 4

int PvpOption = 0; // 私服 1

int OriginalSendOffset = 0; // 私服2

char ItemBuffer[10] = { 0x0A, 0x00, 0xA7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int gAttackHitSkillBuffer = 0;
int gDetectHitSkillBuffer = 0;

int gDetectHitTriggerFlag = 0;            // 偵測被放什麼技能, 被系統偵測後觸發之標籤

#else
int ActorAction = 0x08C8;  // 私服 0x08C8

int AidSkillBufferOffset = 6; // 私服 6
int ItemBufferOffset = 2; // 私服 2

int ItemListStartLen = 6; // 私服 6
int ActorGetInfoLen = 106; // 私服 106
int ItemListEndLen = 4; // 私服 4

int PvpOption = 1; // 私服 1

int OriginalSendOffset = 2; // 私服2

char ItemBuffer[8] = { 0x39, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int gAttackHitSkillBuffer = 0;
int gDetectHitSkillBuffer = 0;

int gDetectHitTriggerFlag = 0;            // 偵測被放什麼技能, 被系統偵測後觸發之標籤

#endif

// 紀錄死亡筆記本
int gDieNoteAid = 0;
int gDieNoteDamage = 0;
int gDieNoteSkill = 0;

// 經驗效率計算
int gExpReportSwitch = 0;
int gExpReportClear = 0;
int gExpReportObtain = 0;
__int64 gExpTotalObtainBaseValue = 0;
__int64 gExpTotalObtainJobValue = 0;
int gExpReportTime = 0;

int gNearSupportCorrdX = 0;
int gNearSupportCorrdY = 0;
int gPartyInfoLen = 0; // 隊伍人數總數量
int gPartyInfoAid[15] = { 0 };
int gPartyInfoJob[15] = { 0 };
int gPartyInfoOnline[15] = { 0 };
int gPartyInfoX[15] = { 0 };
int gPartyInfoY[15] = { 0 };
char gPartyInfoMap[15][16] = { 0 };
int gPauseDrink = 0;
int gDrinkSwitchFirstDetectFlag = 0;                // 第一次跑才偵測
int MapChangeFlag = 1;
HWND gHwnd;
int gStorageStatus = 0;  // 開倉狀態
int gRunOnceForAuthority = 0;
int gCatchGuildId = 0;                     // 欲抓取之公會ID
int gDetectGreedTriggerFlag = 0;    // 貪婪觸發開關
int CloakingCounter = 0; // 用來計算秒數
int CloakingTryNum = 0; // 用來計算試了幾次
int gStorageSuccess = 0; // 開倉是否成功

PickUpItemControl* gItemException; // 例外物品清單
int gItemExceptionLine = 0; // 撿取物品清單行數
int gItemExceptionAllSetting = 0; // 撿取物品策略

RecvpacketControl* gPcaketControl; // 封包處理
int gPcaketControlLine = 0; // 封包處理行數

int gIsTeleport = 0; // 是否正在瞬移
int gTeleportTrigger = 0; // 強制瞬移標籤
int gAutoFindTargetAndAttackTrigger = 0; // 正在智能打怪
int gSteal = 0; // 偷竊成功與否
int gCharSpirits = 0; // 角色總氣彈
int gSkillIdAck = 0; // 偵測放招的ack
int gSkillAck = 0; // 偵測放招的ack
int gNearAttackAck = 0; // 偵測自動放招的ack
int gNearAttackAckTotal = 0; // 偵測自動放招的ack
int gDetactSpecialAttack = 0; // 偵測受到致命技能攻擊
int gDetactSpecialAttackIdleCounter = 0;  // 偵測受到致命技能攻擊 閒置時間計數器
int gSmartBloodAttack = 0; // 偵測使用羅剎壓血, 一秒內不重複壓血
int gSmartBloodAttackIdleCounter = 0;  // 偵測使用羅剎壓血, 一秒內不重複壓血 閒置時間計數器
int gGmModeBackUpValue[10] = { 0 }; // 備份GM AID
int gSmartBloodTrigger = 0; // 智能壓血觸發
int gSmartEquipTrigger[6] = { 0 }; // 智能切裝觸發
int gSmartEquipTriggerIdleCounter[6] = { 0 };
int gDetactYouAreCast = 0; // 偵測你在放技能
int gDetactYouAreCastGhostring = 0; // 偵測你在放技能(幽波)
int gDetactYouAreCastIdleCounter = 0;  // 偵測你在放技能 閒置時間計數器
int gDetactYouAreCastGhostringIdleCounter = 0;  // 偵測你在放技能 閒置時間計數器(幽波)
int gCastEquipTrigger[7] = { 0 }; // 智能施放技能切裝觸發

int gAttackHitTriggerFlag1 = 0;            // 施放什麼技能, 被系統偵測後觸發之標籤
int gAttackHitTriggerFlag2 = 0;            // 施放什麼技能, 被系統偵測後觸發之標籤
int gAttackHitTriggerFlag3 = 0;            // 施放什麼技能, 被系統偵測後觸發之標籤
int gAttackHitGroundCoordX1 = 0;
int gAttackHitGroundCoordY1 = 0;
int gAttackHitGroundCoordX2 = 0;
int gAttackHitGroundCoordY2 = 0;
int gAttackHitGroundCoordX3 = 0;
int gAttackHitGroundCoordY3 = 0;
int gNearAttackScreenMonsterTrigger = 0;             // 畫面內有幾隻怪物再停止瞬移
int gAutoTelePointTrigger = 0; // 看見傳點瞬移
int gAutoSmartCloakFlag = 1; // 強制關閉智能偽裝的flag
int gAutoSmartCloakIdleCounter = 1; // 強制關閉智能偽裝的flag

int gAutoTeleLosTryCountCurrent = 0; // 移動失敗次數總計數

//
// 表情放招
//
int gEmoSwitch[5] = { 0 };
int gEmoSkill[5] = { 0 };
int gEmoMode[5] = { 0 };
int gEmoType[5] = { 0 };
int gEmoIsSelf[5] = { 0 };

//
// 自動補料理變術定義
//
int gAutoAddBufferItem = 0;
int gAutoBufferItemID[40] = { 0 };
int gAutoBufferStateID[40] = { 0 };
int gAutoBufferItemSwitch[40] = { 0 };

// 自動狀態變數定義
int gAutoBufferKey[9] = { 0 };
int gAutoBuffer[9] = { 0 };
int gAutoBufferSwitch[9] = { 0 };
int gAutoBufferDelay[9] = { 0 };

// 自動技能狀態變數定義
int gAutoBufferSkillSwitch[5] = { 0 };
int gAutoBufferSkillName[5] = { 0 };
int gAutoBufferSkillState[5] = { 0 };
int gAutoBufferSkillLv[5] = { 0 };
int gAutoBufferSkillDelay[5] = { 0 };

//
// Config Switch variable
//
int gDrinkSwitch = 0;                 // 是否開起喝水
int gAutoHpDrinkValue1 = 0;           // HP低於多少%吃水
int gAutoHpDrinkValue2 = 0;           // HP2低於多少%吃水
int gAutoSpDrinkValue = 0;            // SP低於多少%吃水
int gAutoHpScanCode1 = 0;             // HP的鍵盤SCAN CODE
int gAutoHpScanCode2 = 0;             // HP2的鍵盤SCAN CODE
int gAutoSpScanCode = 0;              // SP的鍵盤SCAN CODE
int gDrinkHpDelay1 = 80;              // HP1喝水的間格要幾毫秒
int gDrinkHpDelay2 = 80;              // HP2喝水的間格要幾毫秒
int gDrinkSpDelay = 80;               // SP喝水的間格要幾毫秒
int gAutoSkillDelay = 0;              // 所有技能的延遲
int gAttackHitDelay1 = 0;             // 技能的延遲
int gAttackHitDelay2 = 0;             // 技能的延遲
int gAttackHitDelay3 = 0;             // 技能的延遲
int gDetectSpecialDelay1 = 0;         // 技能的延遲
int gDetectSpecialDelay2 = 0;         // 技能的延遲
int gDetectSpecialDelay3 = 0;         // 技能的延遲
int gNearAttackDelay = 0;             // 技能的延遲
int gNearAttackMonsterDelay = 0;      // 技能的延遲
int gNearAttackStopWhenState = 0;     // 此狀態不攻擊(PVP限定)
int gAttackHitSwitch1 = 0;            // 是否開啟攻擊後接技能
int gAttackHitSkillID1 = 0;           // 施放什麼技能, 就被系統偵測  如飛腳踢
int gAttackHitModify1 = 0;            // 為上者, 偵測後施放的技能    如無知
int gAttackHitModSkillLv1 = 3;        // 為上者, 偵測後施放的技能等級
int gAttackHitModSkillCount1 = 5;     // 為上者, 施放次數
int gAttackHitSupport1 = 0;           // 放招的技能是否是對自己使用
int gAttackHitGround1 = 0;            // 放招的技能是否是地板技能
int gAttackHitFallen1 = 0;            // 修羅大纏後接招
int gAttackHitSwitch2 = 0;            // 是否開啟攻擊後接技能
int gAttackHitSkillID2 = 0;           // 施放什麼技能, 就被系統偵測  如飛腳踢
int gAttackHitModify2 = 0;            // 為上者, 偵測後施放的技能    如無知
int gAttackHitModSkillLv2 = 3;        // 為上者, 偵測後施放的技能等級
int gAttackHitModSkillCount2 = 5;     // 為上者, 施放次數
int gAttackHitSupport2 = 0;           // 放招的技能是否是對自己使用
int gAttackHitGround2 = 0;            // 放招的技能是否是地板技能
int gAttackHitFallen2 = 0;            // 修羅大纏後接招
int gAttackHitSwitch3 = 0;            // 是否開啟攻擊後接技能
int gAttackHitSkillID3 = 0;           // 施放什麼技能, 就被系統偵測  如飛腳踢
int gAttackHitModify3 = 0;            // 為上者, 偵測後施放的技能    如無知
int gAttackHitModSkillLv3 = 3;        // 為上者, 偵測後施放的技能等級
int gAttackHitModSkillCount3 = 5;     // 為上者, 施放次數
int gAttackHitSupport3 = 0;           // 放招的技能是否是對自己使用
int gAttackHitGround3 = 0;            // 放招的技能是否是地板技能
int gAttackHitFallen3 = 0;            // 修羅大纏後接招

int gDetectSpecialSkillSwitch1 = 0;   // 自動反擊
int gDetectSpecialSkillId1 = 0;       // 自動反擊
int gDetectSpecialModify1 = 0;        // 自動反擊
int gDetectSpecialModSkillLv1 = 0;    // 自動反擊
int gDetectSpecialModSkillCount1 = 0; // 自動反擊
int gDetectSpecialSupport1 = 0;       // 自動反擊
int gDetectSpecialGround1 = 0;        // 自動反擊
int gDetectSpecialSkillSwitch2 = 0;   // 自動反擊
int gDetectSpecialSkillId2 = 0;       // 自動反擊
int gDetectSpecialModify2 = 0;        // 自動反擊
int gDetectSpecialModSkillLv2 = 0;    // 自動反擊
int gDetectSpecialModSkillCount2 = 0; // 自動反擊
int gDetectSpecialSupport2 = 0;       // 自動反擊
int gDetectSpecialGround2 = 0;        // 自動反擊
int gDetectSpecialSkillSwitch3 = 0;   // 自動反擊
int gDetectSpecialSkillId3 = 0;       // 自動反擊
int gDetectSpecialModify3 = 0;        // 自動反擊
int gDetectSpecialModSkillLv3 = 0;    // 自動反擊
int gDetectSpecialModSkillCount3 = 0; // 自動反擊
int gDetectSpecialSupport3 = 0;       // 自動反擊
int gDetectSpecialGround3 = 0;        // 自動反擊

int gCanSee = 1;                      // 是否看見偽裝
int gAutoStandSwitch = 0;             // 是否自動起立
int gDebuff01 = 0;                    // 負面狀態01的ID
int gDebuffScanCode01 = 0;            // 負面狀態01的鍵盤SCAN CODE
int gDebuff02 = 0;                    // 負面狀態02的ID
int gDebuffScanCode02 = 0;            // 負面狀態02的鍵盤SCAN CODE
int gDebuff03 = 0;                    // 負面狀態03的ID
int gDebuffScanCode03 = 0;            // 負面狀態03的鍵盤SCAN CODE
int gDebuff04 = 0;                    // 負面狀態04的ID
int gDebuffScanCode04 = 0;            // 負面狀態04的鍵盤SCAN CODE
int gDiamonddustRemove = 0;           // 是否自動解除鑽石狀態的開關
//int gDiamonddustScanCode01 = 0;       // 本身裝備的鍵盤代碼
//int gDiamonddustScanCode02 = 0;       // 扣除魔力的鍵盤代碼
equipDataStruct gDiamonddustEquip01;  // 本身裝備的鍵盤代碼
equipDataStruct gDiamonddustEquip02;  // 扣除魔力的鍵盤代碼
int gIllusionRemove = 0;              // 是否自動解幻覺
int gModifyMapSwitch = 0;             // 修改格子地圖
int MonsterValue = 3455;              // 2035細長華金 3468西瓜  3455盤子  1085香菇  2166龍蛋  1109小惡魔
int SwitchOnFlag = 0;                 // 秒七變身開關, 預設1

int gLockWindowSwitch = 0;                    // 使用者固定視窗開關
int gLockWindowValue = 17443;                 // 使用者固定視窗值
int gAutoBufferLoop = 0;                      // 自動補料理開關
int gSmartCastingAuto = 0;                    // 智能施法開關
int gSmartCastingScanCode1 = 0;               // 智能施法開關
int gSmartCastingScanCode2 = 0;               // 智能施法開關
int gSmartCastingScanCode3 = 0;               // 智能施法開關
int gSmartCastingScanCode4 = 0;               // 智能施法開關
int gSmartCastingScanCode5 = 0;               // 智能施法開關
int gSmartCastingScanCode6 = 0;               // 智能施法開關
int gHomunIntimacySwitch = 0;                 // 智能吃生命體開關
int gHomunIntimacyValue = 0;                  // 智能吃生命體親密度數值
int gHomunIntimacyAuto = 0;                   // 智能吃生命體親密度
int gNearAttackSwitch = 0;                    // 是否開啟敵人靠近自動放招   1 = 開
int gNearAttackSkill = 0;                     // 自動放招的技能ID
int gNearAttackSkilv = 0;                     // 自動放招的技能等級
int gNearAttackCount = 0;                     // 要丟幾次技能 建議最多設10
int gNearAttackSupport = 0;                   // 放招的技能是否是對自己使用
int gNearAttackGround = 0;                    // 放招的技能是否是地板技能
int gNearAttackDist = 0;                      // 接近多少距離就放招
int gNearSkillPvpStopSwitch = 0;              // 技能成功後暫停
int gNearSkillPvpStopTime = 10000;            // 技能成功後暫停
int gNearAttackMonster = 0;                   // 是否針對怪物
int gNearAttackDie = 0;                       // 掛機時死亡斷線
int gNearAttackScreenMonsterSwitch = 0;       // 畫面內有幾隻怪物再停止瞬移
int gNearAttackScreenMonster = 0;             // 畫面內有幾隻怪物再停止瞬移
int gNearAttackMonsterSpecialSwitch = 0;      // 針對特定怪物開關
char gNearAttackMonsterSpecial01[24] = "";    // 特定怪物01
char gNearAttackMonsterSpecial02[24] = "";    // 特定怪物02
char gNearAttackMonsterSpecial03[24] = "";    // 特定怪物03
char gNearAttackMonsterSpecial04[24] = "";    // 特定怪物04
char gNearAttackMonsterSpecial05[24] = "";    // 特定怪物05
int gNearAttackMonsterIgnoreSwitch = 0;       // 是否忽略特定怪物
char gNearAttackMonsterIgnore01[24] = "";     // 忽略特定怪物01
char gNearAttackMonsterIgnore02[24] = "";     // 忽略特定怪物02
char gNearAttackMonsterIgnore03[24] = "";     // 忽略特定怪物03
char gNearAttackMonsterIgnore04[24] = "";     // 忽略特定怪物04
char gNearAttackMonsterIgnore05[24] = "";     // 忽略特定怪物05
int gMoveSwitch = 0;                          // 是否自動移動打怪開關
int gMoveRandSwitch = 0;                      // 是否地圖隨機移動開關
int gMoveDelay = 0;                           // 自動移動延遲
int gMoveDist = 0;                            // 自動移動偵測距離
int gBodyRelocation = 0;                      // 躬身代替走路
int gAttackPvpDot = 0;                        // Pvp連點模式
int gNearAttackAidMode = 0;                   // 是否針對敵人AID模式
int gGuidId01 = 1593944;                      // 同盟公會ID
int gGuidId02 = 0;                            // 同盟公會ID
int gGuidId03 = 0;                            // 同盟公會ID
int gGuidId04 = 0;                            // 同盟公會ID
int gGuidId05 = 0;                            // 同盟公會ID
int gGuildIdCatch[6] = { 0 };                 // 同盟公會ID
int gTotalSwitch = 0;                         // 內掛總開關
int gPcActionSwitch = 0;                      // 人物後搖開關
int gAutoFollowSwitch = 0;                    // 是否開啟自動跟隨 1 = 開
int gAutoFollowAid = 0;                       // 是否開啟自動跟隨
int gAutoFollowDist = 0;                      // 是否開啟自動跟隨
int gNearSupportSwitch = 0;                   // 是否開啟自動輔助 1 = 開
int gNearSupportSkill = 0;                    // 自動輔助的技能ID
int gNearSupportSkilv = 0;                    // 自動輔助的技能等級
int gNearSupportSupport = 0;                  // 放招的技能是否是對自己使用
int gNearSupportDist = 0;                     // 自動輔助的距離
int gNearSupportDelay = 0;                    // 自動輔助延遲
int gNearSupportAidOnly = 0;                  // 自動輔助針對AID
int gNearSupportAidTable[10] = { 0 };         // 好友的AID
int gNearEnemyAidTable[10] = { 0 };           // 敵人的AID
int gCatchGuildIdMode = 0;                    // 抓取公會ID模式
int gCatchGuildIdAid = 0;                     // 欲抓取之人的AID
int gAutoGreedSwitch = 0;                     // 自動貪婪開關
int gAutoGreedScanCode = 0;                   // 自動貪婪按鍵
int gAutoItemTakeSwitch = 0;                  // 自動撿物品開關
int gAutoItemTakeSmart = 0;                   // 聰明自動撿物
int gAutoItemTakeDist = 0;                    // 自動撿物距離
int gAutoItemTakeForce = 0;                   // 自動撿物 撿完才飛
int gAutoItemTakeTryCount = 0;                // 自動撿物 嘗試撿取次數
int gAutoTeleSwitch = 0;                      // 自動瞬移開關
int gAutoTeleLOS = 0;                         // 自動瞬移開關
int gAutoTeleLosTryCount = 0;                 // 自動瞬移開關
int gAutoTelePoint = 0;                       // 瞬移迴避傳點
int gAutoTeleScanCode = 0;                    // 自動瞬移按鍵
int gAutoTeleEnter = 0;                       // 自動瞬移是否按enter
int gAutoTeleSpace = 0;                       // 自動瞬移是否按space
int gAutoTeleIdle = 0;                        // 自動瞬移閒置時間
int gAutoTeleIdleCounter = 0;                 // 閒置時間計數器
int gAutoMonsterTeleIdleCounter = 0;          // 被集結暫停計數器 (值大於零代表不移動不瞬移)
int gAutoEquipSwitch[8] = { 0 };              // 特殊切裝
int gAutoEquipScanCode[8] = { 0 };            // 特殊切裝
int gAutoEquipBackScanCode[8] = { 0 };        // 特殊切裝
int gAutoEquipSkill[8] = { 0 };               // 特殊切裝
int gAutoEquipSkilv[8] = { 0 };               // 特殊切裝
int gAutoEquipDelay[8] = { 0 };               // 特殊切裝
int gAutoAttackEquipSwitch[8] = { 0 };        // 攻擊特殊切裝
int gAutoAttackEquipScanCode[8] = { 0 };      // 攻擊特殊切裝
int gAutoAttackEquipScanCode01[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipScanCode02[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipScanCode03[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipScanCode04[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipScanCode05[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipScanCode06[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipScanCode07[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipScanCode08[5] = { 0 };    // 攻擊特殊切裝
int gAutoAttackEquipDelay[8] = { 0 };         // 攻擊特殊切裝
int gAutoPetFeedSwitch = 0;                   // 自動餵食寵物
int gAutoPetFeedLock = 0;                     // 自動餵食寵物
int gAutoPetFeedLock2 = 0;                    // 自動餵食寵物
int gAutoPetFeedLock_X = 0;                   // 自動餵食寵物
int gAutoPetFeedLock_Y = 0;                   // 自動餵食寵物
int gAutoPetFeedLock2_X = 0;                  // 自動餵食寵物
int gAutoPetFeedLock2_Y = 0;                  // 自動餵食寵物
int gAutoCure = 0;                            // 自動點穴快
int gAutoMacroSwitch = 0;                     // 自動連按開關
int gAutoMacroScanCode = 0;                   // 自動連按按鍵
int gAutoMacroDelay = 0;                      // 自動連按延遲
int gAutoMacroSwitch2 = 0;                    // 自動連按開關
int gAutoMacroScanCode2 = 0;                  // 自動連按按鍵
int gAutoMacroDelay2 = 0;                     // 自動連按延遲
int gAutoStorageSwitch = 0;                   // 自動倉庫開關
int gAutoStorageWeight = 0;                   // 自動倉庫負重設定
int gHotKeyStorageSwitch = 0;                 // 熱鍵倉庫開關
int gHotKeyStorageScanCode = 0;               // 熱鍵倉庫按鍵
int gAutoFindTargetSmartSwitch = 0;           // 偷竊開關
int gElemeMasterSuperSwitch = 0;              // 元素破壞專用連打
int gIgnoranceSuperSwitch = 0;                // 無知衰弱接替
// 技能列表
#define ElemeMasterSuper 3
#define ElemeMasterSuperPvp 5
int gCurrentSkillCount = 0;                   // 技能輪指針
int gCurrentSkillPvpCount = ElemeMasterSuper;                // 技能輪指針
int gCurrentSkillTable[ElemeMasterSuperPvp] = {
    5370, //#雷霆陣地#
    5372, //#灼熱岩漿# 
    5371, //#毒液沼澤#
    5373, //#大地驅動#
    5369, //#鑽石風暴#
};
// 技能列表 無知衰弱
int gCurrentIgnoranceSkillCount = 0;                   // 技能輪指針
#define IgnoranceSuper 2
int gCurrentIgnoranceSkillTable[IgnoranceSuper] = {
    2294, //2294#面具-無知#
    2297  //2297#面具-衰弱#
};
int gAutoSpiritSwitch = 0;                    // 自動補氣彈開關
int gAutoSpiritScanCode = 0;                  // 自動補氣彈按鍵
int gAutoSpiritSpiritValue = 0;               // 自動補氣彈數量
int gAutoSmartDrinkSwitch = 0;                // 智能喝水開關
int gAutoCureDelay = 0;                       // 自動點穴快延遲
int gAutoSpiritDelay = 0;                     // 自動狂續延遲
int gSmartBloodSwitch = 0;                    // 自動壓血開關
//int gSmartBloodScanCode1 = 0;               // 自動壓血按鍵1
//int gSmartBloodScanCode2 = 0;               // 自動壓血按鍵2
equipDataStruct gSmartBloodEquip01;           // 自動壓血裝備
equipDataStruct gSmartBloodEquip02;           // 自動壓血裝備
equipDataStruct gSmartBloodEquip03;           // 自動壓血裝備
equipDataStruct gSmartBloodEquip04;           // 自動壓血裝備
int gSmartBloodHp = 0;                        // 自動壓血生命百分比
int gLeaderSwitch = 0;                        // 隊長AID 被集結後停止用
int gLeaderAid = 0;                           // 隊長AID 被集結後停止用
int gLeaderAttackStopSwitch = 0;              // 隊長AID 被集結後停止用
int gLeaderDist = 0;                          // 隊長AID 被集結後停止用
int gLeaderDelay = 0;                         // 隊長AID 被集結後停止用
int gLeaderApGive = 0;                        // 隊長AID 被集結後停止用
int gLeaderIdleCounter = 0;                   // 轉換地圖後 將此值設定大於0 當此值大於0 才去判斷附近是否有隊長
int gStopAutoAttackIdleCounter = 0;           // 過傳點暫停使用自瞄
int gStopAutoAttackIdle = 5000;                  // 過傳點暫停使用自瞄
int gAutoSmartEquipSwitch[6] = { 0 };            // 受攻擊智能切裝開關
int gAutoSmartEquipSkill[6] = { 0 };             // 受攻擊智能切裝技能
int gAutoSmartEquipScanCodeStart01[5] = { 0 };   // 受攻擊智能切裝按鍵前
int gAutoSmartEquipScanCodeEnd01[5] = { 0 };     // 受攻擊智能切裝按鍵後
int gAutoSmartEquipScanCodeStart02[5] = { 0 };   // 受攻擊智能切裝按鍵前
int gAutoSmartEquipScanCodeEnd02[5] = { 0 };     // 受攻擊智能切裝按鍵後
int gAutoSmartEquipScanCodeStart03[5] = { 0 };   // 受攻擊智能切裝按鍵前
int gAutoSmartEquipScanCodeEnd03[5] = { 0 };     // 受攻擊智能切裝按鍵後
int gAutoSmartEquipDelay[6] = { 0 };             // 受攻擊智能切裝延遲
equipDataStruct gAutoSmartEquipItemStart04[5];   // 受攻擊智能切裝 裝備前
equipDataStruct gAutoSmartEquipItemEnd04[5];     // 受攻擊智能切裝 裝備後
equipDataStruct gAutoSmartEquipItemStart05[5];   // 受攻擊智能切裝 裝備前
equipDataStruct gAutoSmartEquipItemEnd05[5];     // 受攻擊智能切裝 裝備後
equipDataStruct gAutoSmartEquipItemStart06[5];   // 受攻擊智能切裝 裝備前
equipDataStruct gAutoSmartEquipItemEnd06[5];     // 受攻擊智能切裝 裝備後
int gSmartRefTPotionSwitch = 0;                  // 自動吃黃金抵抗藥水(只限TE)
int gSmartRefTPotionScanCode = 0;                // 自動吃黃金抵抗藥水按鍵
int gSmartGhostringSwitch = 0;                   // 自動吃幽波捲(只限城戰)
int gSmartGhostringScanCode = 0;                 // 自動吃幽波捲按鍵
int gAutoWarpSwitch[3] = { 0 };                  // 自動傳陣開關
int gAutoWarpCorrdX[3] = { 0 };                  // 自動傳陣座標x
int gAutoWarpCorrdY[3] = { 0 };                  // 自動傳陣座標y
//char gAutoWarpMap[3][16] = { 0 };                // 自動傳陣地圖
char gAutoWarpMap[3] = { 0 };                // 自動傳陣地圖
int gAutoWarpDelay = 0;                          // 自動傳陣延遲
int gAutoPharmacySwitch = 0;                     // 自動配藥開關
int gAutoPharmacyTriggerScanCode = 0;            // 自動配藥觸發按鍵
int gAutoPharmacyScanCode = 0;                   // 自動配藥配藥按鍵
int gAutoPharmacyType = 0;                       // 自動配藥種類
int gAutoPharmacyItemNo = 0;                     // 自動配藥物品編號
int gAutoPharmacyCount = 0;                      // 自動配藥製作次數
int gAutoPharmacyDelay = 0;                      // 自動配藥延遲毫秒
int gGmModeSwitch = 0;                           // 變身GM模式, 可看見公會圖
int gGmModeAid[10] = { 0 };                      // 變身GM模式, 可看見公會圖
int gCatchBroadCastSwitch = 1;                   // 抓取廣播訊息
int gAutoCastEquipSwitch[7] = { 0 };             // 施放技能後切裝開關
int gAutoCastEquipSkill[7] = { 0 };              // 施放技能後切裝技能
int gAutoCastEquipScanCode01[5] = { 0 };         // 施放技能後切裝按鍵
int gAutoCastEquipScanCode02[5] = { 0 };         // 施放技能後切裝按鍵
equipDataStruct gAutoCastEquipitemStart03[5];    // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemStart04[5];    // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemStart05[5];    // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemStart06[5];    // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemStart07[5];    // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemEnd03[5];      // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemEnd04[5];      // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemEnd05[5];      // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemEnd06[5];      // 施放技能後切裝 裝備
equipDataStruct gAutoCastEquipitemEnd07[5];      // 施放技能後切裝 裝備
int gAutoCastEquipDelay[7] = { 0 };              // 施放技能後切裝延遲
int gOneKeyEquipSwitch01 = 0;                    // 一鍵切裝開關
int gOneKeyEquipSwitch02 = 0;                    // 一鍵切裝開關
int gOneKeyEquipScanCode01 = 0;                  // 一鍵切裝觸發按鍵
int gOneKeyEquipScanCode02 = 0;                  // 一鍵切裝觸發按鍵
equipDataStruct gOneKeyEquipItem01[20];          // 一鍵切裝裝備資料
equipDataStruct gOneKeyEquipItem02[20];          // 一鍵切裝裝備資料
int gAutoApSwitch = 0;                           // 自動AP
int gAutoApSkill = 0;                            // 自動AP
int gAutoApLv = 0;                               // 自動AP
int gAutoApGround = 0;                           // 自動AP
int gAutoApIdle = 0;                             // 自動AP
int gAutoApValue = 0;                            // 自動AP
int gAutoApDelay = 0;                            // 自動AP
int gCurrentApValue = 0;                         // 目前角色AP
int gAutoApIdleCounter = 0;                      // 閒置時間計數器
int gConvenioGetBufferStopSwitch[5];
int gConvenioGetBufferStopBuffer[5];
int gConvenioTotalSwitch = 0;                    // 集結後放招
int gConvenioSwitch[14] = { 0 };                 // 集結後放招
int gConvenioSkill[14] = { 0 };                  // 集結後放招
int gConvenioLv[14] = { 0 };                     // 集結後放招
int gConvenioDelay[14] = { 0 };                  // 集結後放招
int gConvenioJob[14] = { 0 };                    // 集結後放招
int gConvenioTrigger = 0;                        // 集結後放招
int gConvenioLeaderSwitch = 0;                   // 集結後放招隊長
int gConvenioLeaderState = 0;                    // 集結後放招隊長
int gConvenioLeaderScanCode = 0;                 // 集結後放招隊長
int gConvenioLeaderDelay = 0;                    // 集結後放招隊長
int gCatchModeSwitch = 0;                        // 擷取開關
int gHotKeyAutoBufferSwitch = 0;                 // 熱鍵補狀態開關
int gSmartCloakSwitch = 0;                       // 智能偽裝切披肩
equipDataStruct gSmartCloakEquip;                // 智能偽裝切披肩
//int gSmartCloakScanCode = 0;                     // 智能偽裝切披肩
int gSmartCloakDelay = 0;                        // 智能偽裝切披肩
int gApAutoSkillSwitch = 0;                      // AP自動施放
int gApAutoSkillScanCode = 0;                    // AP自動施放
int gApAutoSkillValue = 0;                       // AP自動施放

int gGvgDamageSwitch = 0;                        // 城戰顯傷

int gCpuSpeedSwitch = 0;                         // CPU Speed開關
int gCpuSpeedCore = 0;                           // CPU 核心
int gCpuSpeedCoreTotal = 0;                      // CPU 總核心數

int gJobFilterSwitch = 0;                        // 自鎖職業
#define JOB_NUM 18
int gJobFilterList[JOB_NUM] = { 0 };             // 自鎖職業

#define PVP_MAP_COUNT 9
const char gPvpMapDataTable[PVP_MAP_COUNT][16]{
    {"pvp_y_8."},
    {"gefg_cas."},
    {"payg_cas."},
    {"aldeg_cas."},
    {"prtg_cas."},
    {"schg_cas."},
    {"arug_cas."},
    {"te_aldecas."},
    {"te_prtcas."}
};

#define TE_MAP_COUNT 2
const char gTeMapDataTable[TE_MAP_COUNT][16]{
    {"te_aldecas."},
    {"te_prtcas."}
};

#define PVP_FORBIDDEN_MAP_COUNT 2
const char gPvpForbiddenMapDataTable[PVP_FORBIDDEN_MAP_COUNT][16]{
    {"pvp_y_8."},
    {"pvp_y_room."}
};

//
// Function Declare
//
void init();
void HookWs2Functions();
BOOL ConnectServer();
void initialization();
BOOL TestFunc();
BOOL LoadDrinkSetting();
BOOL AutoHpDrink();
BOOL AutoHpDrink2();
BOOL SmartBloodFunction();
BOOL AutoCastEquipStartFunction(int Index, int CheckEquipSuccess);
BOOL AutoCastEquipEndFunction();
BOOL AutoCastEquipFunction1();
BOOL AutoCastEquipFunction2();
BOOL AutoSmartEquipFunction1();
BOOL AutoSmartEquipFunction2();
BOOL AutoSmartEquipFunction3();
BOOL AutoSmartEquipFunction4();
BOOL AutoSmartEquipFunction5();
BOOL AutoSmartEquipFunction6();
BOOL AutoSpDrink();
BOOL AutoDetectStatusDo();
BOOL AutoBufferFunction(int CallByFunction);
//BOOL DetectKeyboardDevice();
BOOL PacketAnalyze(char* dataBufferDeliver, int len);
BOOL PacketAnalyze2(char* dataBufferDeliver, int len);
int SearchRcvPacketName(int PacketName, int* PacketLen);
BOOL GetPackageBuffer();
BOOL AutoCastEquipFunction();
BOOL SmartCastingSkill();
BOOL SmartCastingSkillKeyboardSupport();
BOOL SmartCastingSkillKeyboard();
BOOL ModifyMap();
BOOL AutoWarpFunction();
BOOL PcActionModifyFunction();
BOOL CheckSpecialMonsterFunction(char CharNameStr[CharNameLen], char CharNameStr2[CharNameLen]);
BOOL CheckIgnoreMonsterFunction(char CharNameStr[CharNameLen], char CharNameStr2[CharNameLen]);
BOOL CheckIgnoreMonsterFunction2(char CharNameStr[CharNameLen], char CharNameStr2[CharNameLen]);
BOOL AutoCureFunction();
BOOL AutoApFunction();
BOOL AutoAttackHitFunction1();
BOOL AutoAttackHitFunction2();
BOOL AutoAttackHitFunction3();
BOOL OneKeyEquipFunction();
BOOL NearAttackScreenMonsterFunction(void);
BOOL ConvenioFunction(void);
void SendEquipCommand(equipDataStruct EquipItem, int UpdateIsEquip, int ForceEquip, int delay, int typeOption, int CheckEquipSuccess);
void StorageProcessFunction(void);
void ClearTriggerFlag(void);
void ClearAllTriggerFlag(void);
int CheckState(void);
int CheckPetHungryState(void);
int CheckDebuffState(int DebuffValue);
int PacketMatchOrNot(char* buffer, int Count, int Packet);
int IsYourAidOrNot(char* buffer, int Count);
int IsGuidMember(char* buffer, int Count);
int IsEnemyAid2(int Aid);
int IsPvpMap(void);
int IsTeMap(void);
int IsPvpForbiddenMap(void);
int IsGmAid(char* buffer, int Count);
int IsSpecialAid(char* buffer, int Count);
int IsSpecialAid2(int Aid);
int IsEnemyAid(char* buffer, int Count);
int GetConsoleHwnd(void);
int FindTotalRoCharName(void);
int CatchCharNameAndAid(void);
int CatchAidList(char* buffer, char* CharNameStr, int AidValue, int GuildId, int Job, int Weapon, int Shield);
VOID PickUpItemException(VOID);
int IsExceptionPickUpItemOrNot(int ItemNameId);
int SendSkillCastFunction(int SkillId, int SkillLv, int Target);
int SendSkillGroundFunction(int SkillId, int SkillLv, int CorrdX, int CorrdY);
int ConvertEquipSetting(CString* SettingBuffer, equipDataStruct *EquipItem);
int MonsterGeneratorFunction(char* MonsterStr, int MonsterId);
int MonsterReadFunction(char* MonsterStr, int MonsterId);
void ClearCharNameFunction (void);
BOOL SkillSendByKeboard(int Aid, int IsMyself, int ScanCode, int CorrdX, int CorrdY);
int PetLockXY(void);
void SendItemTakePackageFunction(int ItemAid);
BOOL AttackToPlayerFunction(void);
void ClearExpReport(void);
void ExpReportGetFunction(void);
int CalculateTwoPointLength(int X1, int Y1, int X2, int Y2, int* MoveX, int* MoveY, int Follow, int Step);
int CheckLos(int X0, int Y0, int X1, int Y1);
void GetCurrentMapName(char* MapName);
int IsRoFocus(void);
int FindSkillName(int SkillId, char SkillName[100]);
int FindItemName(int ItemId, char *ItemName);
int FindJobName(int JobId, char JobName[30]);
BOOL DieNoteFunction(void);
int fromBase62(const char* ItemChar, int IsRefine);
int powValue(int Base, int BaseBy, int Count);
void solveMessage(string *BroadCastMessageStr);

extern "C" {
    int (WINAPI* OriginalRecv) (SOCKET s, char* buf, int len, int flags) = recv;
    int (WINAPI* OriginalRecvFrom) (SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen) = recvfrom;
    int (WINAPI* OriginalSend) (SOCKET s, const char* buf, int len, int flags) = send;
    int (WINAPI* OriginalSendTo) (SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen) = sendto;
    int (WINAPI* OriginalConnect) (SOCKET s, const struct sockaddr* name, int namelen) = connect;
    int (WINAPI* OriginalSelect) (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timeval* timeout) = select;
    int (WINAPI* OriginalWSARecv) (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) = WSARecv;
    int (WINAPI* OriginalWSARecvFrom) (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, struct sockaddr* lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) = WSARecvFrom;
    int (WINAPI* OriginalWSASend) (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) = WSASend;
    int (WINAPI* OriginalWSASendTo) (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const sockaddr* lpTo, int iToLen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) = WSASendTo;
    int (WINAPI* OriginalWSAAsyncSelect) (SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent) = WSAAsyncSelect;

    void* (__cdecl* memcpy_get)(void* dest, const void* src, size_t count);// = GetProcAddress(hDLL, "memcpy");

    void(__cdecl* OriginalRoSend)(char* src, int len1, char* dest, int len2, int IsTrue);// = (int* (__cdecl*)(const char*, int)) 0x5D64B0;
    //int(__cdecl* OriginalRoSend)(int len, const char* src);
}



struct MonsterControl {
    int MonsterId;
    char MonsterStr[25];
};

struct ItemControl {
    int ItemId;
    int Miminum;
    int AutoStore;
    int AutoSell;
};



struct node
{
    char *data;
    int dataLen;
    node* next;
};

class linked_list
{
private:
    node* head, * tail;
public:
    linked_list()
    {
        head = NULL;
        tail = NULL;
    }

    void add_node(char* buffer, int len)
    {
        lock_guard<mutex> mLock(gMutex);
        node* tmp = new node;

        char* bufferAlloc = new char[len + 1];
        tmp->data = bufferAlloc;
        memcpy(tmp->data, buffer, len);
        //tmp->data = buffer;
        tmp->dataLen = len;
        tmp->next = NULL;

        if (head == NULL)
        {
            //debug("Add node head");
            head = tmp;
            tail = tmp;
        }
        else
        {
            //debug("Add node tail");
            tail->next = tmp;
            tail = tail->next;
        }
    }

    int pass_data_buffer()
    {
        gMutex.lock();
        if (head == NULL || head->data == NULL || head->dataLen == 0) {
            //debug("pass_data_buffer head null");
            gMutex.unlock();
            return 0;
        }
        else {
            //debug("pass_data_buffer data pass");
            gMutex.unlock();
            PacketAnalyze(head->data, head->dataLen);
            return 1;
        }
    }

    void delete_head_node()
    {
        lock_guard<mutex> mLock(gMutex);
        if (head != NULL) {
            node* tmp;
            tmp = head; //先把head值存起來
            head = head->next; //把head指向下一個節點
            delete [] tmp->data;
            delete tmp; //把temp刪掉
            //debug("Delete node Success.");
        }
    }
};


struct skillNode
{
    int Id; // 技能ID
    int TargetType; // 技能類型
    int Lv; // 技能等級
    int Sp; // 技能消耗SP
    int Range; // 技能範圍
    int Upgrateable; // 是否可以升等
    int Lv2; // 等級2
    skillNode* next;
};


class skill_linked_list
{
private:
    skillNode* skillHead, * skillTail;
public:
    skill_linked_list()
    {
        skillHead = NULL;
        skillTail = NULL;
    }

    void add_skill_node(int Id, int TargetType, int Lv, int Sp, int Range, int Upgrateable, int Lv2)
    {
        lock_guard<mutex> mLock(gMutexSkill);
        skillNode* tmp = new skillNode;

        tmp->Id = Id;
        tmp->TargetType = TargetType;
        tmp->Lv = Lv;
        tmp->Sp = Sp;
        tmp->Range = Range;
        tmp->Upgrateable = Upgrateable;
        tmp->Lv2 = Lv2;
        tmp->next = NULL;

        if (skillHead == NULL)
        {
            skillHead = tmp;
            skillTail = tmp;
        }
        else
        {
            skillTail->next = tmp;
            skillTail = skillTail->next;
        }
    }

    void delete_skill_node(int Id)
    {
        lock_guard<mutex> mLock(gMutexSkill);
        skillNode* current = skillHead,
            * previous = 0;
        while (current != 0 && current->Id != Id) {   // Traversal
            previous = current;                       // 如果current指向NULL
            current = current->next;                  // 或是current->data == x
        }                                             // 即結束while loop

        if (current == 0) {                           // list沒有要刪的skillNode, 或是list為empty

        }
        else if (current == skillHead) {          // 要刪除的skillNode剛好在list的開頭
            skillHead = current->next;            // 把itemHead移到下一個skillNode
            delete current;                      // 如果list只有一個skillNode, 那麼skillHead就會指向NULL
            current = 0;                         // 當指標被delete後, 將其指向NULL, 可以避免不必要bug                   
        }
        else if (current == skillTail) {          // 要刪除的skillNode剛好在最尾端
            previous->next = current->next;      // 而且skillNode不為skillHead, 此時previous不為NULL
            skillTail = previous;
            delete current;
            current = 0;
        }
        else {                                   // 其餘情況, list中有欲刪除的skillNode, 
            previous->next = current->next;      // 而且skillNode不為skillHead, 此時previous不為NULL
            delete current;
            current = 0;
        }
        return;
    }

    void clear_skill_node()
    {
        lock_guard<mutex> mLock(gMutexSkill);
        while (skillHead != 0) {            // Traversal
            skillNode* current = skillHead;
            skillHead = skillHead->next;
            delete current;
            current = 0;
        }
    }

    int IsSkillIdCanUse(int Id)
    {
        lock_guard<mutex> mLock(gMutexSkill);

        skillNode* current = skillHead;

        while (current != 0 && current->Id != Id) {   // Traversal

            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的skillNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find skill, return 1
        //
        if (current->Lv && current->TargetType) { // 技能等級必須大於0, 且不能是被動技能
            return current->Lv; // 回傳技能等級
        }
        else {
            return 0;
        }
    }

    int SearchSkillRange(int Id)
    {
        lock_guard<mutex> mLock(gMutexSkill);

        skillNode* current = skillHead;

        while (current != 0 && current->Id != Id) {   // Traversal

            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的skillNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find skill, return 1
        //
        if (current->Lv && current->TargetType) { // 技能等級必須大於0, 且不能是被動技能
            return current->Range; // 回傳技能等級
        }
        else {
            return 0;
        }
    }

    // 將技能資訊寫入setting
    int write_all_skill_data_in_setting()
    {
#if !PrivateServerOrNot
        //lock_guard<mutex> mLock(gMutexSkill);
#endif
        skillNode* current = skillHead;
        // 清空技能序列
        char IndexText[10];
        char skillProfileText[3] = "_S";
        for (int i = 0; i < 300; i++) {
            sprintf(IndexText, "%d", i); // 轉化數字變成文字
            strcat_s(IndexText, skillProfileText); // 在數字後面+上_S  EX: 12_S
            ::WritePrivateProfileString(gCharNameStr, IndexText, _T(""), path);
        }
        //
        // 寫入目前角色技能
        //
        int Index = 0;

        char IdText[6];
        char TargetTypeText[6];
        char LvText[6];
        char SpText[6];
        char RangeText[6];
        char UpgrateableText[6];
        char Lv2Text[6];
        char commaText[2] = ",";

        while (current != 0) {   // Traversal
            if (current->Lv && current->TargetType) { // 技能等級必須大於1, 且不能是被動技能
                char SkillText[100];

                sprintf(IndexText, "%d", Index); // 轉化數字變成文字
                strcat_s(IndexText, skillProfileText); // 在數字後面+上_S  EX: 12_S

                sprintf(IdText, "%d", current->Id); // 轉化數字變成文字
                sprintf(TargetTypeText, "%d", current->TargetType); // 轉化數字變成文字
                sprintf(LvText, "%d", current->Lv); // 轉化數字變成文字
                sprintf(SpText, "%d", current->Sp); // 轉化數字變成文字
                sprintf(RangeText, "%d", current->Range); // 轉化數字變成文字
                sprintf(UpgrateableText, "%d", current->Upgrateable); // 轉化數字變成文字
                sprintf(Lv2Text, "%d", current->Lv2); // 轉化數字變成文字

                strcpy_s(SkillText, IdText);
                strcat_s(SkillText, commaText);
                strcat_s(SkillText, TargetTypeText);
                strcat_s(SkillText, commaText);
                strcat_s(SkillText, LvText);
                strcat_s(SkillText, commaText);
                strcat_s(SkillText, SpText);
                strcat_s(SkillText, commaText);
                strcat_s(SkillText, RangeText);
                strcat_s(SkillText, commaText);
                strcat_s(SkillText, UpgrateableText);
                strcat_s(SkillText, commaText);
                strcat_s(SkillText, Lv2Text);
                strcat_s(SkillText, commaText);

                printf("SkillText: %s\n", SkillText);

                ::WritePrivateProfileString(gCharNameStr, IndexText, SkillText, path);
                Index++;
            }
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find skill
            //
            return 0;
        }
        //
        // Find skill, return 1
        //

        return 1;
    }

};



struct itemNode
{
    int type;
    int Id;
    int itemId;
    int itemAmount;
    int refineView; // 是否精煉
    int isEquip; // 是否裝備
    int equipType;
    int slot1;
    int slot2;
    int slot3;
    int slot4;
    int failCount;
    itemNode* next;
};


class item_linked_list
{
private:
    itemNode* itemHead, * itemTail;
public:
    item_linked_list()
    {
        itemHead = NULL;
        itemTail = NULL;
    }

    void add_item_node(int type, int Id, int itemId, int itemAmount, int refineView, int isEquip, int equipType, int slot1, int slot2, int slot3, int slot4)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* tmp = new itemNode;
        tmp->type = type;
        tmp->Id = Id;
        tmp->itemId = itemId;
        tmp->itemAmount = itemAmount;

        tmp->refineView = refineView;
        tmp->isEquip = isEquip;
        tmp->equipType = equipType;
        tmp->slot1 = slot1;
        tmp->slot2 = slot2;
        tmp->slot3 = slot3;
        tmp->slot4 = slot4;

        tmp->failCount = 0;

        tmp->next = NULL;
        //printf("add item node: type:%d Id:%d itemId:%d\n", tmp->type, tmp->Id, tmp->itemId);
        if (itemHead == NULL)
        {
            itemHead = tmp;
            itemTail = tmp;
        }
        else
        {
            itemTail->next = tmp;
            itemTail = itemTail->next;
        }
    }

    void delete_item_node(int itemId)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead,
            * previous = 0;
        while (current != 0 && current->itemId != itemId) {   // Traversal
            previous = current;                       // 如果current指向NULL
            current = current->next;                  // 或是current->data == x
        }                                             // 即結束while loop

        if (current == 0) {                           // list沒有要刪的itemNode, 或是list為empty
            //std::cout << "There is no " << itemId << " in list.\n";
            // return;
        }
        else if (current == itemHead) {          // 要刪除的itemNode剛好在list的開頭
            itemHead = current->next;            // 把itemHead移到下一個itemNode
            delete current;                      // 如果list只有一個itemNode, 那麼itemHead就會指向NULL
            current = 0;                         // 當指標被delete後, 將其指向NULL, 可以避免不必要bug
            // return;                     
        }
        else if (current == itemTail) {          // 要刪除的itemNode剛好在最尾端
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            itemTail = previous;
            delete current;
            current = 0;
            // return;
        }
        else {                                   // 其餘情況, list中有欲刪除的itemNode, 
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            delete current;
            current = 0;
            // return;
        }
        return;
    }

    void delete_item_node_Id(int Id)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead,
            * previous = 0;
        while (current != 0 && current->Id != Id) {   // Traversal
            previous = current;                       // 如果current指向NULL
            current = current->next;                  // 或是current->data == x
        }                                             // 即結束while loop

        if (current == 0) {                           // list沒有要刪的itemNode, 或是list為empty
            std::cout << "There is no " << Id << " in list.\n";
            // return;
        }
        else if (current == itemHead) {          // 要刪除的itemNode剛好在list的開頭
            itemHead = current->next;            // 把itemHead移到下一個itemNode
            delete current;                      // 如果list只有一個itemNode, 那麼itemHead就會指向NULL
            current = 0;                         // 當指標被delete後, 將其指向NULL, 可以避免不必要bug
            // return;                     
        }
        else if (current == itemTail) {          // 要刪除的itemNode剛好在最尾端
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            itemTail = previous;
            delete current;
            current = 0;
            // return;
        }
        else {                                   // 其餘情況, list中有欲刪除的itemNode, 
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            delete current;
            current = 0;
            // return;
        }
        return;
    }

    void clear_item_node()
    {
        lock_guard<mutex> mLock(gMutexItem);
        while (itemHead != 0) {            // Traversal
            itemNode* current = itemHead;
            itemHead = itemHead->next;
            delete current;
            current = 0;
        }
    }

    int search_item(int itemId)
    {
        lock_guard<mutex> mLock(gMutexItem);
        //debug("search1");
        itemNode* current = itemHead;
        //itemNode* previous = new itemNode;
        //debug("search2");
        while (current != 0 && current->itemId != itemId) {   // Traversal
            //previous = current;                             // 如果current指向NULL
            //printf("search%d\n", count);
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //std::cout << "There is no " << itemId << " in list.\n";
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        if (current->itemAmount > 0) {
            return current->Id;
        }
        else {
            return NON_ITEM_AMOUNT;
        }
    }

    int search_item_Id(int Id)
    {
        lock_guard<mutex> mLock(gMutexItem);
        //debug("search1");
        itemNode* current = itemHead;
        //itemNode* previous = new itemNode;
        //debug("search2");
        while (current != 0 && current->Id != Id) {   // Traversal
            //previous = current;                             // 如果current指向NULL
            //printf("search%d\n", count);
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //std::cout << "There is no " << Id << " in list.\n";
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        if (current->itemAmount > 0) {
            return current->itemId;
        }
        else {
            return NON_ITEM_AMOUNT;
        }
    }

    // 搜尋物品數量, 傳itemId進去, 回傳數量
    int search_itemAmount(int itemId)
    {
        lock_guard<mutex> mLock(gMutexItem);
        //debug("search1");
        itemNode* current = itemHead;
        //itemNode* previous = new itemNode;
        //debug("search2");
        while (current != 0 && current->itemId != itemId) {   // Traversal
            //previous = current;                             // 如果current指向NULL
            //printf("search%d\n", count);
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //std::cout << "There is no " << itemId << " in list.\n";
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        //if (current->itemAmount > 0) {
        //    return current->itemAmount;
        //}
        //else {
        //    return 0;
        //}

        return current->itemAmount;
    }


    int update_item_amount(int itemId, int itemAmount)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead;
        while (current != 0 && current->itemId != itemId) {   // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        current->itemAmount = itemAmount;
        return current->Id;
    }

    int update_item_isEquip (int Id, int IsEquip)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead;
        while (current != 0 && current->Id != Id) {   // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        current->isEquip = IsEquip;
        current->failCount = 0;
        return 1;
    }

    // 搜尋裝備, 
    // 傳入 精練 物品名稱 卡片 是否裝備 ID名稱
    // 回傳 是否找到
    int search_equip_data (
        IN int refineView, 
        IN int itemId, 
        IN int slot1,
        IN int slot2,
        IN int slot3,
        IN int slot4, 
        OUT int *isEquip,
        OUT int *equipType,
        OUT int *Id
    )
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead;
        while (current != 0 && !(
            (current->refineView == refineView) && 
            (current->itemId == itemId) &&
            (current->slot1 == slot1) &&
            (current->slot2 == slot2) &&
            (current->slot3 == slot3) &&
            (current->slot4 == slot4))) {   // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        *isEquip = current->isEquip;
        *equipType = current->equipType;
        *Id = current->Id;

        return 1;
    }


    // 根據type, 印出所有物品資料
    int print_all_item_data_byType (int type)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead;
        while (current != 0) {   // Traversal
            if (type == current->type) {
                if (type == 1) { // 裝備
                    printf("裝備: +%d %d (%d) [%d, %d, %d, %d] 類型: %d 是否裝備: %d\n", current->refineView, current->itemId, current->Id, current->slot1, current->slot2, current->slot3, current->slot4, current->equipType, current->isEquip);
                } else if (type == 0) { // 物品
                    printf("物品: %d (%d) 數量: %d\n", current->itemId, current->Id, current->itemAmount);
                }
            
            }
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //

        return 1;
    }

    // 根據type, 回報報告
    int exp_report_all_item_data_byType(int type)
    {
        lock_guard<mutex> mLock(gMutexItem);
        char ExpReportStr[48] = ".\\data\\log\\ExpReport_";
        char txt[5] = ".txt";
        strcat(ExpReportStr, gCharNameStr);
        strcat(ExpReportStr, txt);
        ofstream fout(ExpReportStr, ios::app);   // 檔案如果存在就清空

        if (!fout.fail() && *MaxExpValue > 0 && *MaxJobValue > 0) {

            if (type == 99) {
                fout << "------------擊殺報告------------" << endl;
            }
            itemNode* current = itemHead;
            while (current != 0) {   // Traversal
                if (type == current->type) {
                    if (type == 1) { // 裝備
                        printf("裝備: +%d %d (%d) [%d, %d, %d, %d] 類型: %d 是否裝備: %d\n", current->refineView, current->itemId, current->Id, current->slot1, current->slot2, current->slot3, current->slot4, current->equipType, current->isEquip);
                    }
                    else if (type == 0) { // 物品
                        printf("物品: %d (%d) 數量: %d\n", current->itemId, current->Id, current->itemAmount);
                        fout << current->itemId << " x " << current->itemAmount << endl;
                    }
                    else if (type == 99) { // 魔物
                        //printf("物品: %d (%d) 數量: %d\n", current->itemId, current->Id, current->itemAmount);
                        fout << current->itemId << " X " << current->itemAmount << endl;
                    }

                }
                current = current->next;                          // 或是current->data == x
            }                                                     // 即結束while loop

            if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
                //
                // Not find item
                //
                fout.close();
                return 0;
            }
            //
            // Find item, return 1
            //
            fout.close();
            return 1;

        }
        else {
            fout.close();
            return 0;
        }
    }


    // 將所有裝備資料寫入setting
    int write_all_equip_data_in_setting()
    {
#if !PrivateServerOrNot
        //lock_guard<mutex> mLock(gMutexItem);
#endif
        itemNode* current = itemHead;
        //LPCTSTR charPath = _T(".\\data\\setting.txt");
        // 清空裝備序列
        char IndexTextClear[20];
        for (int i = 0; i < 150; i++) {
            sprintf(IndexTextClear, "%d", i); // 轉化數字變成文字
            ::WritePrivateProfileString(gCharNameStr, IndexTextClear, _T(""), path);
        }

        int Index = 0;
        char commaText[2] = ",";
        
        while (current != 0) {   // Traversal
            // 寫入目前身上所有裝備
            char EquipText[100];
            char IndexText[20];

            char refineViewText[4];
            char itemIdText[10];
            char slot1Text[10];
            char slot2Text[10];
            char slot3Text[10];
            char slot4Text[10];
            
            if (1 == current->type) { 

                printf("裝備: +%d %d (%d) [%d, %d, %d, %d] 類型: %d 是否裝備: %d\n", current->refineView, current->itemId, current->Id, current->slot1, current->slot2, current->slot3, current->slot4, current->equipType, current->isEquip);
                sprintf(IndexText, "%d", Index); // 轉化數字變成文字

                sprintf(refineViewText, "%d", current->refineView); // 轉化數字變成文字
                sprintf(itemIdText, "%d", current->itemId); // 轉化數字變成文字
                sprintf(slot1Text, "%d", current->slot1); // 轉化數字變成文字
                sprintf(slot2Text, "%d", current->slot2); // 轉化數字變成文字
                sprintf(slot3Text, "%d", current->slot3); // 轉化數字變成文字
                sprintf(slot4Text, "%d", current->slot4); // 轉化數字變成文字

                strcpy_s(EquipText, refineViewText);
                strcat_s(EquipText, commaText);

                strcat_s(EquipText, itemIdText);
                strcat_s(EquipText, commaText);

                strcat_s(EquipText, slot1Text);
                strcat_s(EquipText, commaText);
                strcat_s(EquipText, slot2Text);
                strcat_s(EquipText, commaText);
                strcat_s(EquipText, slot3Text);
                strcat_s(EquipText, commaText);
                strcat_s(EquipText, slot4Text);
                strcat_s(EquipText, commaText);

                printf("EquipText: %s\n", EquipText);

                ::WritePrivateProfileString(gCharNameStr, IndexText, EquipText, path);
                Index++;
            }
            else if (0 == current->type) {
                
                printf("物品: %d (%d) 數量: %d\n", current->itemId, current->Id, current->itemAmount);
                sprintf(IndexText, "%d", Index); // 轉化數字變成文字

                sprintf(refineViewText, "%d", 999); // 轉化數字變成文字 999代表物品
                sprintf(itemIdText, "%d", current->itemId); // 轉化數字變成文字
                sprintf(slot1Text, "%d", current->itemAmount); // 轉化數字變成文字 物品數量
                
                strcpy_s(EquipText, refineViewText);
                strcat_s(EquipText, commaText);

                strcat_s(EquipText, itemIdText);
                strcat_s(EquipText, commaText);

                strcat_s(EquipText, slot1Text);
                strcat_s(EquipText, commaText);

                printf("ItemText: %s\n", EquipText);

                ::WritePrivateProfileString(gCharNameStr, IndexText, EquipText, path);
                Index++;
            }
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //

        return 1;
    }


    //
    // 更新物品數量, itemAmount參數為變化量
    //
    int update_item_amount_add_delete(int itemId, int itemAmount)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead;
        while (current != 0 && current->itemId != itemId) {   // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        current->itemAmount += itemAmount;
        return current->Id;
    }

    //
    // 更新物品數量, 藉由物品身上ID為參數, itemAmount參數為變化量
    //
    int update_item_amount_add_delete_byId(int Id, int itemAmount)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead;
        while (current != 0 && current->Id != Id) {       // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        current->itemAmount += itemAmount;

        return current->itemAmount;
    }

    //
    // 更新裝備的裝備成功or失敗, 藉由物品身上ID為參數, failCount參數為變化量
    //
    int update_item_failCount_byId(int IsSuccess, int Id)
    {
        lock_guard<mutex> mLock(gMutexItem);
        itemNode* current = itemHead;
        while (current != 0 && current->Id != Id) {       // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //
        if (IsSuccess) {
            current->failCount = 0;
        }
        else {
            current->failCount = current->failCount + 1;
        }

        // 如果該裝備失敗三次, 防呆, 將所有該類型的裝備卸除
        if (current->failCount >= 3) {
            printf("卸除type: %d同樣類型的所有裝備\n", current->equipType);
            itemNode* current2 = itemHead;

            while (current2 != 0) {   // Traversal
                if (current2->type == 1 && current2->equipType == current->equipType) {
                    // 為該失敗類型的同種裝備
                    current2->failCount = 0;
                    current2->isEquip = 0;
                    printf("卸除裝備: %d (%d) type: %d\n", current2->itemId, current2->Id, current2->equipType);
                }
                current2 = current2->next;                          // 或是current2->data == x
            }
        
        }

        return 1;
    }

};


struct playerNode
{
    int Aid;
    int Time;
    int Flag;
    playerNode* next;
};

class player_linked_list
{
private:
    playerNode* playerHead, * playerTail;
public:
    player_linked_list()
    {
        playerHead = NULL;
        playerTail = NULL;
    }

    void add_player_node(int Aid)
    {
        lock_guard<mutex> mLock(gMutexPlayerList);
        playerNode* tmp = new playerNode;
        tmp->Aid = Aid;
        tmp->Time = 0;
        tmp->Flag = 0;
        tmp->next = NULL;
        //printf("Add player node: AID:%d \n", tmp->Aid);
        if (playerHead == NULL)
        {
            playerHead = tmp;
            playerTail = tmp;
        }
        else
        {
            playerTail->next = tmp;
            playerTail = playerTail->next;
        }
    }

    void add_player_node_2(int Aid, int Time, int Flag)
    {
        lock_guard<mutex> mLock(gMutexPlayerList);
        playerNode* tmp = new playerNode;
        tmp->Aid = Aid;
        tmp->Time = Time;
        tmp->Flag = Flag;
        tmp->next = NULL;
        //printf("Add player node: AID:%d \n", tmp->Aid);
        if (playerHead == NULL)
        {
            playerHead = tmp;
            playerTail = tmp;
        }
        else
        {
            playerTail->next = tmp;
            playerTail = playerTail->next;
        }
    }

    void clear_player_node()
    {
        lock_guard<mutex> mLock(gMutexPlayerList);
        while (playerHead != 0) {            // Traversal
            playerNode* current = playerHead;
            playerHead = playerHead->next;
            delete current;
            current = 0;
        }
    }

    int search_player(int Aid)
    {
        lock_guard<mutex> mLock(gMutexPlayerList);
        playerNode* current = playerHead;
        while (current != 0 && current->Aid != Aid) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的playerNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        return 1;

    }

    int search_player_2(int Aid, int Flag)
    {
        lock_guard<mutex> mLock(gMutexPlayerList);
        playerNode* current = playerHead;
        while (current != 0 && (current->Aid != Aid || current->Flag != Flag)) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop
        if (current == 0) {                                   // list沒有要刪的playerNode, 或是list為empty
            //
            // Not find AID
            //
            return 1;
        }
        //
        // Find AID, return 1
        //

        if (Flag == 0) {
            if (current->Time == 0) {
                return 1;
            }
            else {
                return 0;
            }
        }
        else if (Flag == current->Flag) {
            if (current->Time == 0) {
                return 1;
            }
            else {
                return 0;
            }
        }
        return 1;
    }

    void delete_player_node_Id(int Aid, int Flag)
    {
        lock_guard<mutex> mLock(gMutexPlayerList);
        playerNode* current = playerHead,
            * previous = 0;
        while (current != 0 && (current->Aid != Aid || current->Flag != Flag)) {   // Traversal
            previous = current;                       // 如果current指向NULL
            current = current->next;                  // 或是current->data == x
        }                                             // 即結束while loop

        if (current == 0) {                           // list沒有要刪的itemNode, 或是list為empty
            // return;
        }
        else if (current == playerHead) {          // 要刪除的itemNode剛好在list的開頭
            playerHead = current->next;            // 把itemHead移到下一個itemNode
            delete current;                      // 如果list只有一個itemNode, 那麼itemHead就會指向NULL
            current = 0;                         // 當指標被delete後, 將其指向NULL, 可以避免不必要bug
            // return;                     
        }
        else if (current == playerTail) {          // 要刪除的itemNode剛好在最尾端
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            playerTail = previous;
            delete current;
            current = 0;
            // return;
        }
        else {                                   // 其餘情況, list中有欲刪除的itemNode, 
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            delete current;
            current = 0;
            // return;
        }
        return;
    }

    int TimeTick_1s()
    {
        lock_guard<mutex> mLock(gMutexPlayerList);
        playerNode* current = playerHead;
        while (current != 0) {   // Traversal
            if (current->Time) { // 倒數時間大於0
                current->Time = current->Time - 1000;
            }
            if (current->Time < 0) {
                current->Time = 0;
            }

            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //

        return 1;
    }

};
player_linked_list PlayerAidList;
player_linked_list SkillDelayPlayerList;

struct targetNode
{
    int Aid;
    int Type;
    float CorrdX;
    float CorrdY;
    float CorrdToX;
    float CorrdToY;
    float Speed;
    int IsMove;
    int TryCount;
    int YouHit;
    int MonsterId;
    char Name[24];
    targetNode* next;
};

class target_linked_list
{
private:
    targetNode* targetHead, * targetTail;
public:
    target_linked_list()
    {
        targetHead = NULL;
        targetTail = NULL;
    }

    void add_target_node(int Aid, int Type, float CorrdX, float CorrdY, float Speed, int MonsterId, char Name[24])
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* tmp = new targetNode;
        tmp->Aid = Aid;
        tmp->Type = Type;
        tmp->CorrdX = CorrdX;
        tmp->CorrdY = CorrdY;
        tmp->CorrdToX = 0;
        tmp->CorrdToY = 0;
        tmp->IsMove = 0;
        tmp->YouHit = 0;
        tmp->MonsterId = MonsterId;
        if (Name != NULL) { // 不為空字串
            strcpy(tmp->Name, Name);
        }
        if (Speed > 0) {
            tmp->Speed = Speed;
        }
        else {
            tmp->Speed = 0.150;
        }
        tmp->TryCount = 0;
        tmp->next = NULL;
        //printf("Add target node: AID:%d \n", tmp->Aid);
        if (targetHead == NULL)
        {
            targetHead = tmp;
            targetTail = tmp;
        }
        else
        {
            targetTail->next = tmp;
            targetTail = targetTail->next;
        }
    }

    void delete_target_node(int Aid)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead,
            * previous = 0;
        while (current != 0 && current->Aid != Aid) {   // Traversal
            previous = current;                       // 如果current指向NULL
            current = current->next;                  // 或是current->data == x
        }                                             // 即結束while loop

        if (current == 0) {                           // list沒有要刪的itemNode, 或是list為empty
            //std::cout << "There is no " << itemId << " in list.\n";
            // return;
        }
        else if (current == targetHead) {          // 要刪除的itemNode剛好在list的開頭
            targetHead = current->next;            // 把itemHead移到下一個itemNode
            delete current;                      // 如果list只有一個itemNode, 那麼itemHead就會指向NULL
            current = 0;                         // 當指標被delete後, 將其指向NULL, 可以避免不必要bug
            // return;                     
        }
        else if (current == targetTail) {          // 要刪除的itemNode剛好在最尾端
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            targetTail = previous;
            delete current;
            current = 0;
            // return;
        }
        else {                                   // 其餘情況, list中有欲刪除的itemNode, 
            previous->next = current->next;      // 而且itemNode不為itemHead, 此時previous不為NULL
            delete current;
            current = 0;
            // return;
        }
        return;
    }

    void clear_target_node()
    {
        lock_guard<mutex> mLock(gMutexTarget);
        while (targetHead != 0) {            // Traversal
            targetNode* current = targetHead;
            targetHead = targetHead->next;
            delete current;
            current = 0;
        }
    }

    int search_target(int Aid)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        while (current != 0 && current->Aid != Aid) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        return 1;
    }

    int search_target_name (int Aid, char Name[24])
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        while (current != 0 && current->Aid != Aid) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        strcpy(Name, current->Name);

        return 1;
    }

    int update_target_to_you_hit(int Aid)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        while (current != 0 && current->Aid != Aid) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        current->YouHit = 1;
        return 1;
    }

    int search_target_you_hit(int Aid, int Type)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        while (current != 0 && (current->Aid != Aid || current->YouHit != 1 || current->Type != Type)) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        if (current->Type == 0) { // 人類
            return current->Aid;
        }
        return current->MonsterId;
    }

    int update_target_Corrd(int Aid, float CorrdX, float CorrdY)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        while (current != 0 && current->Aid != Aid) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        current->CorrdX = CorrdX;
        current->CorrdY = CorrdY;
        return 1;
    }

    int update_target_MoveXY(int Aid, float CorrdX, float CorrdY, float CorrdToX, float CorrdToY, int IsMove, float Speed)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        while (current != 0 && current->Aid != Aid) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        current->CorrdX = CorrdX;
        current->CorrdY = CorrdY;

        current->CorrdToX = CorrdToX;
        current->CorrdToY = CorrdToY;
        current->IsMove = IsMove;

        if (Speed > 0) {
            current->Speed = Speed;
        }

        return 1;
    }

    // 更新100毫秒後物件的座標
    int update_XY_by_Speed_100ms()
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        float MoveDist;
        float TotalDist;
        float MoveRemainDist; // 移動完後剩下距離
        while (current != 0) {   // Traversal
            if (current->IsMove) { // 物件正在移動
                if (current->CorrdX > 0 && current->CorrdY > 0 && current->CorrdToX > 0 && current->CorrdToY > 0 && current->Speed > 0) {

                    // 計算移動距離
                    MoveDist = 100 / current->Speed;
                    // 計算總距離
                    TotalDist = ((current->CorrdToX - current->CorrdX) * (current->CorrdToX - current->CorrdX)) +
                        ((current->CorrdToY - current->CorrdY) * (current->CorrdToY - current->CorrdY));
                    TotalDist = sqrt(TotalDist); // 開根號
                    // 計算斜率
                    MoveRemainDist = TotalDist - MoveDist;

                    //// x, y 各個相減差
                    //float XDev = current->CorrdToX - current->CorrdX;
                    //float YDev = current->CorrdToY - current->CorrdY;

                    //// 斜線
                    // 計算移動後的x y 座標 (分點座標公式)
                    current->CorrdX = MoveDist * current->CorrdToX + MoveRemainDist * current->CorrdX;
                    current->CorrdX = current->CorrdX / TotalDist;
                    current->CorrdY = MoveDist * current->CorrdToY + MoveRemainDist * current->CorrdY;
                    current->CorrdY = current->CorrdY / TotalDist;
                    //printf("%d 移動中 (%f, %f) MoveDist: %f TotalDist: %f \n", current->Aid, current->CorrdX, current->CorrdY, MoveDist, TotalDist);

                    // 如果已經移動超過點上, 停止移動
                    // 計算新的總距離
                    float TotalDistAfter = ((current->CorrdToX - current->CorrdX) * (current->CorrdToX - current->CorrdX)) +
                        ((current->CorrdToY - current->CorrdY) * (current->CorrdToY - current->CorrdY));
                    TotalDistAfter = sqrt(TotalDistAfter); // 開根號
                    if (TotalDistAfter < MoveDist || TotalDistAfter > TotalDist) {
                        //printf("到達! %d 移動中 (%f, %f)\n", current->Aid, current->CorrdX, current->CorrdY);
                        current->CorrdX = current->CorrdToX;
                        current->CorrdY = current->CorrdToY;
                        current->CorrdToX = 0;
                        current->CorrdToY = 0;
                        current->IsMove = 0;
                    }
                }
            }


            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的itemNode, 或是list為empty
            //
            // Not find item
            //
            return 0;
        }
        //
        // Find item, return 1
        //

        return 1;
    }


    int search_target_Type(int Type, int *Aid, int* CorrdX, int* CorrdY)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        int MinTargetDist = 30 * 30; // 最短的目標距離
        int Find = 0;
        while (current != 0) {         // Traversal
            if (current->Type == Type) { // 符合Type
                int TargetDist = ((abs(*PLAYER_CORRD_X - current->CorrdX) * abs(*PLAYER_CORRD_X - current->CorrdX)) + (abs(*PLAYER_CORRD_Y - current->CorrdY) * abs(*PLAYER_CORRD_Y - current->CorrdY)));
                if (TargetDist < MinTargetDist) { // 找到距離更近的目標
                    MinTargetDist = TargetDist; // 更新最短目標
                    
                    *Aid = current->Aid;
                    *CorrdX = current->CorrdX;
                    *CorrdY = current->CorrdY;
                    Find = 1;
                }
            }
            current = current->next;            // 或是current->data == x
        }                                       // 即結束while loop

        if (!Find) {                     // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        
        //
        // Find AID, return 1
        //
        return 1;
    }

    int search_target_Type_Los(int Type, int* Aid, int* CorrdX, int* CorrdY, int* MoveX, int* MoveY)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        int MinTargetDist = 30; // 最短的目標距離
        int Find = 0;
        while (current != 0) {         // Traversal
            if (current->Type == Type) { // 符合Type
                int LosDist = CalculateTwoPointLength(*PLAYER_CORRD_X, *PLAYER_CORRD_Y, current->CorrdX, current->CorrdY, MoveX, MoveY, 0, 7);
                if (LosDist < MinTargetDist) { // 找到距離更近的目標
                    MinTargetDist = LosDist; // 更新最短目標

                    *Aid = current->Aid;
                    *CorrdX = current->CorrdX;
                    *CorrdY = current->CorrdY;
                    Find = 1;
                }
            }
            current = current->next;            // 或是current->data == x
        }                                       // 即結束while loop

        if (!Find) {                     // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }

        //
        // Find AID, return 1
        //
        return 1;
    }

    int search_target_Type_ItemPickUp (int* Dist)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        int MinTargetDist = gAutoItemTakeDist * gAutoItemTakeDist; // 最短撿取物品距離
        while (current != 0) {         // Traversal
            int TargetDist = ((abs(*PLAYER_CORRD_X - current->CorrdX) * abs(*PLAYER_CORRD_X - current->CorrdX)) + (abs(*PLAYER_CORRD_Y - current->CorrdY) * abs(*PLAYER_CORRD_Y - current->CorrdY)));
            
            // 需要強制撿取的重要物品
            if (IsExceptionPickUpItemOrNot(current->Type) == 2) {
                if (gAutoItemTakeSwitch && gTotalSwitch && !gStorageStatus) {

                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || CloakingTryNum < 3 || *MAP_NAME == LOGIN_RSW || !current->Aid || !current->Type) {

                    }
                    else {
                        // 有找到物品, 回傳物品AID
                        *Dist = TargetDist * TargetDist;
                        gAutoTeleLosTryCountCurrent = 0;
                        return current->Aid;
                    }
                }
            }
            
            else if (TargetDist < MinTargetDist && current->TryCount < gAutoItemTakeTryCount) { // 符合撿取範圍 而且 嘗試撿取次數不超過兩次

                if (gAutoGreedSwitch && gAutoGreedScanCode && gTotalSwitch && !gStorageStatus && !IsExceptionPickUpItemOrNot(current->Type)) {
                    //gDetectGreedTriggerFlag = 1;
                    debug("自動貪婪");
                    current->TryCount++; // 嘗試撿取次數+1
                    *Dist = TargetDist * TargetDist;
                    return -1;
                }
                else if (gAutoItemTakeSwitch && gTotalSwitch && !gStorageStatus && !IsExceptionPickUpItemOrNot(current->Type)) {

                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || CloakingTryNum < 3 || *MAP_NAME == LOGIN_RSW || !current->Aid || !current->Type) {

                    }
                    else {
                        // 有找到物品, 回傳物品AID
                        current->TryCount++; // 嘗試撿取次數+1
                        *Dist = TargetDist * TargetDist;
                        gAutoTeleLosTryCountCurrent = 0;
                        return current->Aid;
                    }
                }
            }
            //previous = current;                     // 如果current指向NULL
            current = current->next;                  // 或是current->data == x
        }                                             // 即結束while loop

        *Dist = 0;
        return 0; // 沒找到物品
    }


    // 尋找目標數量
    int search_target_Type_Amount (int Type)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        int TargetAmount = 0; // 目標數量
        int Find = 0;
        //printf("怪物數量01");
        while (current != 0) {         // Traversal
            if (current->Type == Type) { // 符合Type
                int TargetDist = ((abs(*PLAYER_CORRD_X - current->CorrdX) * abs(*PLAYER_CORRD_X - current->CorrdX)) + (abs(*PLAYER_CORRD_Y - current->CorrdY) * abs(*PLAYER_CORRD_Y - current->CorrdY)));
                if (TargetDist <= gMoveDist * gMoveDist && !PlayerAidList.search_player(current->Aid)) { // 有在設定偵測範圍內
                    //printf("怪物數量02");
                    Find = 1;
                    TargetAmount++;
                }
            }
            current = current->next;            // 或是current->data == x
        }                                       // 即結束while loop

        if (!Find) {                     // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            //printf("怪物數量03");
            return 0;
        }

        //
        // TargetAmount
        //
        printf("怪物數量: %d\n", TargetAmount);
        return TargetAmount;
    }

    int search_target_Aid(int Aid, int* CorrdX, int* CorrdY)
    {
        lock_guard<mutex> mLock(gMutexTarget);
        targetNode* current = targetHead;
        while (current != 0 && current->Aid != Aid) {         // Traversal
            current = current->next;                          // 或是current->data == x
        }                                                     // 即結束while loop

        if (current == 0) {                                   // list沒有要刪的targetNode, 或是list為empty
            //
            // Not find AID
            //
            return 0;
        }
        //
        // Find AID, return 1
        //
        *CorrdX = current->CorrdX;
        *CorrdY = current->CorrdY;
        return 1;
    }

};

linked_list PacketDataList;
//linked_list2 PacketCompleteDataList;
skill_linked_list SkillDataList;
item_linked_list ItemDataList;
item_linked_list StorageDataList;
item_linked_list ItemReportList;
//player_linked_list PlayerAidList;
target_linked_list TargetAidList;
target_linked_list TargetAidExpReportList;
target_linked_list ItemAidList;

//void sendDataToKore(char* buffer, int len, e_PacketType type) {
//    // Is Kore running?
//    bool isAlive = koreClientIsAlive;
//
//    if (isAlive)
//    {
//        char* newbuf = (char*)malloc(len + 3);
//        unsigned short sLen = (unsigned short)len;
//        if (type == e_PacketType::RECEIVED) {
//            memcpy(newbuf, "R", 1);
//        }
//        else {
//            memcpy(newbuf, "S", 1);
//        }
//        memcpy(newbuf + 1, &sLen, 2);
//        memcpy(newbuf + 3, buffer, len);
//        xkoreSendBuf.append(newbuf, len + 3);
//        free(newbuf);
//    }
//}

SYSTEMTIME OnTime64toSystemTime(__time64_t& itime)
{
    struct tm* temptm = _localtime64(&itime);
    SYSTEMTIME st = { 
        1900 + temptm->tm_year,
        1 + temptm->tm_mon,
        temptm->tm_wday,
        temptm->tm_mday,
        temptm->tm_hour,
        temptm->tm_min,
        temptm->tm_sec,
        0 
    };
    return st;
}

void GetModifyDateTime(const wstring& strFilename, SYSTEMTIME& stLocal)
{
    struct _stat64i32 statbuf;
    _wstat64i32(strFilename.c_str(), &statbuf);
    stLocal = OnTime64toSystemTime(statbuf.st_mtime);
}

BOOL PrintBuffer(char* buffer) {

    int PrintValueCount = 0;
    while (PrintValueCount < 10) {
        PrintValueCount++;
        int newLineCount = 0;
        //Sleep(1000);
        debug("Called MyRecv ...");
        for (int Count = 0; Count < 100; Count++) {
            printf("%02hhX ", buffer[Count]);
            newLineCount++;
            if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}

char gSrc[1000];
int gLen;
int gSrcModify = 0;
BOOL AutoCdClinetFunction() {
    Sleep(100);
    while (1) {

        if (gSrc[0]) {
            // 算出長度
            int Length = 0;
            while (gSrc[Length] || gSrc[Length + 1] || gSrc[Length + 2] || gSrc[Length + 3] || gSrc[Length + 4] || gSrc[Length + 5]
                && Length < 20) {
                Length++;
            }

            printf("RoSendHook len: %d\n", Length);
            // 封包名稱
            int packetName = (gSrc[0] & 0xFF) + ((gSrc[1] & 0xFF) << 8); // 封包名稱

            //int newLineCount = 0;
            //for (int Count = 0; Count < Length; Count++) {
            //    printf("%02hhX ", gSrc[Count]);
            //    // 清空gSrc[Count]
            //    gSrc[Count] = 0;

            //    newLineCount++;
            //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
            //}
            //printf("\n");

            //清除
            gSrc[0] = 0;
        }
        Sleep(1);
    }

    return 1;
}

void RoSendHook(char* src, int len1, char* dest, int len2, int IsTrue) {

    //if (len1 > 2 && len1 < 200) {

   /*     printf("RoSendHook len: %d\n", len1);
        int newLineCount = 0;
        for (int Count = 0; Count < len1; Count++) {
            printf("%02hhX ", src[Count]);
            newLineCount++;
            if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        }
        printf("\n");*/

    //}

    //if (!gSrcModify && !gLen) {
        memcpy(gSrc, src, len1);
        //memcpy(gSrc, src, len1);
        //gLen = len1;
    //}

        //int* CDCLIENT_FUNCTION = reinterpret_cast <int*>(0x00D6E040);
        //int CDCLIENT_FUNCTION2 = *CDCLIENT_FUNCTION + 5;

        //__asm {
        //    //pushad

        //    mov eax, CDCLIENT_FUNCTION2
        //    jmp eax

        //    //popad
        //}
        OriginalRoSend(src, len1, dest, len2, IsTrue);
    //return 1;//OriginalRoSend(src, len1, dest, len2, IsTrue);
}


void* __cdecl Mine_Memcpy(void* dest, const char* src, size_t count) {
    
    if (dest == PACKET_RECV_ADDRESS) {
        if (count > 0 && *AidAddress && *MAP_NAME != LOGIN_RSW) {
            //if (gTotalSwitch) {
                PacketAnalyze2((char*)src, count);
            //}
            //PacketDataList.add_node((char*)src, count);
            PacketAnalyze((char*)src, count);


           //printf("Recv \n");
           //for (int Count = 0; Count < count; Count++) {
           //    printf("%02hhX ", (char*)src[Count]);
           //    if (Count % 16 == 0 && Count != 0) printf("\n");
           //}
           //printf("\n");
        }

        //printf("成功hook到memcpy dest: %8X\n", dest);
        //printf("成功hook到memcpy count: %d\n", count);
    }

    return memcpy_get(dest, src, count);
}

int gRunOnce = 0;
//  int (WINAPI* OriginalRecv)
int WINAPI HookedRecv(SOCKET socket, char* buffer, int len, int flags) {
    //debug("Called MyRecv ...");
    int ret_len = OriginalRecv(socket, buffer, len, flags);
    if (ret_len != SOCKET_ERROR && ret_len > 0 && socket != s_server) {

        roServer = socket;

        //if (ret_len <= 2048 && *AidAddress && *MAP_NAME != LOGIN_RSW) {
        //}
    }
    return ret_len;
}



// int (WINAPI* OriginalRecvFrom)
int WINAPI HookedRecvFrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen) {
    return  OriginalRecvFrom(s, buf, len, flags, from, fromlen);
}

// int (WINAPI* OriginalSend)
int WINAPI HookedSend(SOCKET s, const char* buffer, int len, int flags) {

#if PACKET_MODE
    int PacketValue = ((buffer[2 - OriginalSendOffset]) & 0xFF) + (((buffer[3 - OriginalSendOffset]) & 0xFF) << 8);

    //
    // 偵測智能切裝
    //
    if (len < 15 && gTotalSwitch) {

        int SkillIdTmp = ((buffer[6 - OriginalSendOffset]) & 0xFF) + (((buffer[7 - OriginalSendOffset]) & 0xFF) << 8);
        int AidTmp = ((buffer[8 - OriginalSendOffset]) & 0xFF) + (((buffer[9 - OriginalSendOffset]) & 0xFF) << 8)
            + (((buffer[10 - OriginalSendOffset]) & 0xFF) << 16) + (((buffer[11 - OriginalSendOffset]) & 0xFF) << 24);

        if (PacketValue == 0x0438 || PacketValue == 0x0AF4) {

            // 偵測智能切裝
            int Index = 0;
            if (SkillIdTmp == gAutoCastEquipSkill[2] && gAutoCastEquipSwitch[2] && !gCastEquipTrigger[2]) {
                Index = 2;
                AutoCastEquipStartFunction(Index, 0);
                gCastEquipTrigger[Index] = 1;
                printf("偵測使用特定技能切裝%d\n", Index);
            }
            else if (SkillIdTmp == gAutoCastEquipSkill[3] && gAutoCastEquipSwitch[3] && !gCastEquipTrigger[3]) {
                Index = 3;
                AutoCastEquipStartFunction(Index, 0);
                gCastEquipTrigger[Index] = 1;
                printf("偵測使用特定技能切裝%d\n", Index);

            }
            else if (SkillIdTmp == gAutoCastEquipSkill[4] && gAutoCastEquipSwitch[4] && !gCastEquipTrigger[4]) {
                Index = 4;
                AutoCastEquipStartFunction(Index, 0);
                gCastEquipTrigger[Index] = 1;
                printf("偵測使用特定技能切裝%d\n", Index);

            }
            else if (SkillIdTmp == gAutoCastEquipSkill[5] && gAutoCastEquipSwitch[5] && !gCastEquipTrigger[5]) {
                Index = 5;
                AutoCastEquipStartFunction(Index, 0);
                gCastEquipTrigger[Index] = 1;
                printf("偵測使用特定技能切裝%d\n", Index);

            }
            else if (SkillIdTmp == gAutoCastEquipSkill[6] && gAutoCastEquipSwitch[6] && !gCastEquipTrigger[6]) {
                Index = 6;
                AutoCastEquipStartFunction(Index, 0);
                gCastEquipTrigger[Index] = 1;
                printf("偵測使用特定技能切裝%d\n", Index);
            }
            else if (SkillIdTmp == gAutoCastEquipSkill[0] && gAutoCastEquipSwitch[0] && !gCastEquipTrigger[0]) {
                Index = 0;
                gCastEquipTrigger[Index] = 1;
                printf("偵測使用特定技能切裝%d\n", Index);
            }
            else if (SkillIdTmp == gAutoCastEquipSkill[1] && gAutoCastEquipSwitch[1] && !gCastEquipTrigger[1]) {
                Index = 0;
                gCastEquipTrigger[Index] = 1;
                printf("偵測使用特定技能切裝%d\n", Index);
            }

            // 偵測羅剎
            if (SkillIdTmp == 2343 && gSmartBloodSwitch) {
                printf("偵測使用羅剎\n");
                //
                // 自動壓血
                //
                if (((*HPIndexValue * 100 / *MaxHPTableValue) > gSmartBloodHp) && !gDetactSpecialAttack && !gSmartBloodAttack) { // 生命小於百分比和未被攻擊時
                    printf("壓血\n");
                    SendEquipCommand(gSmartBloodEquip02, 1, 1, 5, 0, 0);
                    SendEquipCommand(gSmartBloodEquip04, 1, 1, 5, 0, 0);

                    SendEquipCommand(gSmartBloodEquip01, 1, 1, 5, 0, 0);
                    SendEquipCommand(gSmartBloodEquip03, 1, 1, 5, 0, 0);

                    gSmartBloodAttack = 1;
                    gSmartBloodAttackIdleCounter = 1000;

                }

            }
        }
    }

#endif

    int ret;
    ret = OriginalSend(s, buffer, len, flags);

    return ret;
}

// int (WINAPI* OriginalSendTo)
int WINAPI HookedSendTo(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen) {
    return OriginalSendTo(s, buf, len, flags, to, tolen);
}

// int (WINAPI* OriginalConnect)
int WINAPI HookedConnect(SOCKET s, const struct sockaddr* name, int namelen) {
    return OriginalConnect(s, name, namelen);
}

// int (WINAPI* OriginalSelect)
int WINAPI HookedSelect(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timeval* timeout) {
    return OriginalSelect(nfds, readfds, writefds, exceptfds, timeout);
}

// int (WINAPI* OriginalWSARecv)
int WINAPI HookedWSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    return OriginalWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
}

// int (WINAPI* OriginalWSARecvFrom)
int WINAPI HookedWSARecvFrom(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, struct sockaddr* lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    return OriginalWSARecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);
}

// int (WINAPI* OriginalWSASend)
int WINAPI HookedWSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    return OriginalWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

// int (WINAPI* OriginalWSASendTo)
int WINAPI HookedWSASendTo(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const sockaddr* lpTo, int iToLen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    return OriginalWSASendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iToLen, lpOverlapped, lpCompletionRoutine);
}

// int (WINAPI* OriginalWSAAsyncSelect)
int WINAPI HookedWSAAsyncSelect(SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent) {
    return OriginalWSAAsyncSelect(s, hWnd, wMsg, lEvent);
}

int JobFilterFunction(int JobInput) {
    //
    // 過濾自鎖職業
    //
    if (gJobFilterSwitch) {
    
        for (int i = 0; i < JOB_NUM; i++) {
            if (JobInput == gJobFilterList[i] && gJobFilterList[i]) {
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

void AutoEquipDetectFunction(int Skill) {
    //
    // 切裝需求
    //
    int Index = 0;
    if ((Skill == gAutoCastEquipSkill[2] || gAutoCastEquipSkill[2] == -100) && gAutoCastEquipSwitch[2] && !gCastEquipTrigger[2]) {
        Index = 2;
        AutoCastEquipStartFunction(Index, 1);
        gCastEquipTrigger[Index] = 1;
        printf("偵測使用特定技能切裝%d\n", Index);
    }
    else if ((Skill == gAutoCastEquipSkill[3] || gAutoCastEquipSkill[3] == -100) && gAutoCastEquipSwitch[3] && !gCastEquipTrigger[3]) {
        Index = 3;
        AutoCastEquipStartFunction(Index, 1);
        gCastEquipTrigger[Index] = 1;
        printf("偵測使用特定技能切裝%d\n", Index);

    }
    else if ((Skill == gAutoCastEquipSkill[4] || gAutoCastEquipSkill[4] == -100) && gAutoCastEquipSwitch[4] && !gCastEquipTrigger[4]) {
        Index = 4;
        AutoCastEquipStartFunction(Index, 1);
        gCastEquipTrigger[Index] = 1;
        printf("偵測使用特定技能切裝%d\n", Index);

    }
    else if ((Skill == gAutoCastEquipSkill[5] || gAutoCastEquipSkill[5] == -100) && gAutoCastEquipSwitch[5] && !gCastEquipTrigger[5]) {
        Index = 5;
        AutoCastEquipStartFunction(Index, 1);
        gCastEquipTrigger[Index] = 1;
        printf("偵測使用特定技能切裝%d\n", Index);

    }
    else if ((Skill == gAutoCastEquipSkill[6] || gAutoCastEquipSkill[6] == -100) && gAutoCastEquipSwitch[6] && !gCastEquipTrigger[6]) {
        Index = 6;
        AutoCastEquipStartFunction(Index, 1);
        gCastEquipTrigger[Index] = 1;
        printf("偵測使用特定技能切裝%d\n", Index);
    }
    else if ((Skill == gAutoCastEquipSkill[0] || gAutoCastEquipSkill[0] == -100) && gAutoCastEquipSwitch[0] && !gCastEquipTrigger[0]) {
        Index = 0;
        gCastEquipTrigger[Index] = 1;
        printf("偵測使用特定技能切裝%d\n", Index);
    }
    else if ((Skill == gAutoCastEquipSkill[1] || gAutoCastEquipSkill[1] == -100) && gAutoCastEquipSwitch[1] && !gCastEquipTrigger[1]) {
        Index = 1;
        gCastEquipTrigger[Index] = 1;
        printf("偵測使用特定技能切裝%d\n", Index);
    }

    // 偵測羅剎
    if (!PACKET_MODE && !gSmartBloodTrigger && Skill == 2343 && gSmartBloodSwitch) {
        printf("偵測使用羅剎\n");
        gSmartBloodTrigger = 1;
        //SmartBloodFunction();
    }

    // 偵測羅剎
    if (PACKET_MODE && Skill == 2343 && gSmartBloodSwitch) {
        printf("偵測使用羅剎\n");
        //
        // 自動壓血
        //
        if (((*HPIndexValue * 100 / *MaxHPTableValue) > gSmartBloodHp) && !gDetactSpecialAttack && !gSmartBloodAttack) { // 生命小於百分比和未被攻擊時
            printf("壓血\n");
            SendEquipCommand(gSmartBloodEquip02, 1, 1, 5, 0, 0);
            SendEquipCommand(gSmartBloodEquip04, 1, 1, 5, 0, 0);

            SendEquipCommand(gSmartBloodEquip01, 1, 1, 5, 0, 0);
            SendEquipCommand(gSmartBloodEquip03, 1, 1, 5, 0, 0);

            gSmartBloodAttack = 1;
            gSmartBloodAttackIdleCounter = 1000;

        }

    }


}

void SendAttackWeaponPackageFunction(int TargetAid, int IsWalk)
{
#if PACKET_MODE
    char NearHandAttackBuffer[7] = { 0x37, 0x04, 0x00, 0x00, 0x00, 0x00, 0x07 };

    NearHandAttackBuffer[2] = (TargetAid & 0xFF);
    NearHandAttackBuffer[3] = ((TargetAid & 0xFF00) >> 8);
    NearHandAttackBuffer[4] = ((TargetAid & 0xFF0000) >> 16);
    NearHandAttackBuffer[5] = ((TargetAid & 0xFF000000) >> 24);

    OriginalSend(roServer, NearHandAttackBuffer, 7, 0);

#else
    MEMORY_BASIC_INFORMATION mbi;

    int* ATTACK_ADDRESS_1 = NULL;
    int* ATTACK_ADDRESS_2 = NULL;
    int* ATTACK_ADDRESS_TRIGGER = NULL;
    int* ATTACK_ADDRESS_AID = NULL;

    int* ATTACK_ADDRESS_STOP_ATTACK = NULL;

    // 填上5, 觸發普攻
    if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_1 = (int*)(*WINDOWS_LOCK_1 + 0xCC);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_2 = (int*)(*ATTACK_ADDRESS_1 + 0x2C);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_TRIGGER = (int*)(*ATTACK_ADDRESS_2 + 0x4E8);
        }
    }

    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_AID = (int*)(*ATTACK_ADDRESS_2 + 0x4FC);
        }
    }

    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_STOP_ATTACK = (int*)(*ATTACK_ADDRESS_2 + 0x4F0);
        }
    }

    // 觸發
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_TRIGGER, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_TRIGGER = 5;
        }
    }

    // 施展對象
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_AID, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_AID = TargetAid;
        }
    }

    //printf("測試: 普攻: ATTACK_ADDRESS_TRIGGER: %X, ATTACK_ADDRESS_AID: %X \n", ATTACK_ADDRESS_TRIGGER, ATTACK_ADDRESS_AID);

    if (IsWalk) {
        Sleep(5);
        // 觸發
        if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_STOP_ATTACK, &mbi, sizeof(mbi))) {
            if (mbi.Protect == PAGE_READWRITE) {
                *ATTACK_ADDRESS_STOP_ATTACK = 2;
            }
        }
    }
#endif

    gAutoTeleIdleCounter = 0;
}

void SendAttackPackageFunction(int SkillLv, int SkillId, int TargetAid)
{
#if PACKET_MODE
    char SkillBuffer[10] = { 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
    // 
    // SKILL LV
    //
    SkillBuffer[2] = SkillLv & 0xFF;
    SkillBuffer[3] = (SkillLv & 0xFF00) >> 8;
    //
    // SKILL ID
    //
    SkillBuffer[4] = SkillId & 0xFF;
    SkillBuffer[5] = (SkillId & 0xFF00) >> 8;
    //
    // AID
    //
    SkillBuffer[6] = (TargetAid & 0xFF);
    SkillBuffer[7] = ((TargetAid & 0xFF00) >> 8);
    SkillBuffer[8] = ((TargetAid & 0xFF0000) >> 16);
    SkillBuffer[9] = ((TargetAid & 0xFF000000) >> 24);

    AutoEquipDetectFunction(SkillId); // 切裝需求
    OriginalSend(roServer, SkillBuffer, 10, 0);

#else
    MEMORY_BASIC_INFORMATION mbi;

    int* ATTACK_ADDRESS_1 = NULL;
    int* ATTACK_ADDRESS_2 = NULL;
    int* ATTACK_ADDRESS_TRIGGER = NULL;
    int* ATTACK_ADDRESS_LEVEL = NULL;
    int* ATTACK_ADDRESS_SKILLID = NULL;
    int* ATTACK_ADDRESS_AID = NULL;

    //printf("印出封包測試: ");
    //for (int Index = 0; Index < 9; Index++) {
    //    printf("%02X ", NearSkillBuffer[2 + Index] & 0xFF);
    //}
    //printf("\n");

    //// 先修改封包位置的值
    //for (int Index = 0; Index < 9; Index++) {
    //    *(ATTACK_PACKAGE_REGION + Index) = NearSkillBuffer[2 + Index];
    //}

    // 填上3, 觸發技能
    if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_1 = (int*)(*WINDOWS_LOCK_1 + 0xCC);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_2 = (int*)(*ATTACK_ADDRESS_1 + 0x2C);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_TRIGGER = (int*)(*ATTACK_ADDRESS_2 + 0x4E8);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_LEVEL = (int*)(*ATTACK_ADDRESS_2 + 0x510);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_SKILLID = (int*)(*ATTACK_ADDRESS_2 + 0x50C);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_AID = (int*)(*ATTACK_ADDRESS_2 + 0x4FC);
        }
    }

    //
    // 切裝需求
    //
    AutoEquipDetectFunction(SkillId);

    // 觸發
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_TRIGGER, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_TRIGGER = 3;
        }
    }
    // 技能等級
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_LEVEL, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_LEVEL = SkillLv;
        }
    }
    // 技能ID
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_SKILLID, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_SKILLID = SkillId;
        }
    }
    // 施展對象
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_AID, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_AID = TargetAid;
        }
    }
#endif
    gAutoTeleIdleCounter = 0;
}

void SendAttackGroundPackageFunction(int SkillLv, int SkillId, int X, int Y)
{
#if PACKET_MODE
    char SkillGroundBuffer[11] = { 0xF4, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // SKILL LV
    SkillGroundBuffer[2] = SkillLv & 0xFF;
    SkillGroundBuffer[3] = (SkillLv & 0xFF00) >> 8;
    // SKILL ID
    SkillGroundBuffer[4] = SkillId & 0xFF;
    SkillGroundBuffer[5] = (SkillId & 0xFF00) >> 8;
    // x, y
    SkillGroundBuffer[6] = ((X) & 0xFF);
    SkillGroundBuffer[7] = (((X) & 0xFF00) >> 8);
    SkillGroundBuffer[8] = ((Y) & 0xFF);
    SkillGroundBuffer[9] = (((Y) & 0xFF00) >> 8);
    
    AutoEquipDetectFunction(SkillId); // 切裝需求
    OriginalSend(roServer, SkillGroundBuffer, 11, 0);

#else


    MEMORY_BASIC_INFORMATION mbi;

    int* ATTACK_ADDRESS_1 = NULL;
    int* ATTACK_ADDRESS_2 = NULL;
    int* ATTACK_ADDRESS_TRIGGER = NULL;
    int* ATTACK_ADDRESS_LEVEL = NULL;
    int* ATTACK_ADDRESS_SKILLID = NULL;
    int* ATTACK_ADDRESS_X = NULL;
    int* ATTACK_ADDRESS_Y = NULL;

    //printf("印出封包測試: ");
    //for (int Index = 0; Index < 10; Index++) {
    //    printf("%02X ", NearSkillGroundBuffer[2 + Index] & 0xFF);
    //}
    //printf("\n");

    //// 先修改封包位置的值
    //for (int Index = 0; Index < 9; Index++) {
    //    *(ATTACK_PACKAGE_REGION + Index) = NearSkillBuffer[2 + Index];
    //}

    // 填上3, 觸發技能
    if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_1 = (int*)(*WINDOWS_LOCK_1 + 0xCC);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_2 = (int*)(*ATTACK_ADDRESS_1 + 0x2C);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_TRIGGER = (int*)(*ATTACK_ADDRESS_2 + 0x4E8);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_LEVEL = (int*)(*ATTACK_ADDRESS_2 + 0x510);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_SKILLID = (int*)(*ATTACK_ADDRESS_2 + 0x50C);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_X = (int*)(*ATTACK_ADDRESS_2 + 0x514);
        }
    } 
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_Y = (int*)(*ATTACK_ADDRESS_2 + 0x518);
        }
    }

    //
    // 切裝需求
    //
    AutoEquipDetectFunction(SkillId);

    // 判斷是走路
    if (SkillId == 0 && SkillLv == 0) {
        // 先清掉trigger
        if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_TRIGGER, &mbi, sizeof(mbi))) {
            if (mbi.Protect == PAGE_READWRITE) {
                *ATTACK_ADDRESS_TRIGGER = 0;
                Sleep(10);
            }
        }
    }

    // 觸發
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_TRIGGER, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_TRIGGER = 4;
        }
    }
    // 技能等級
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_LEVEL, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_LEVEL = SkillLv;
        }
    }
    // 技能ID
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_SKILLID, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_SKILLID = SkillId;
        }
    }
    // 施展座標
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_X, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_X = X;
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_Y, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_Y = Y;
        }
    }
#endif
    gAutoTeleIdleCounter = 0;
}

void MoveFunction(int MoveX, int MoveY)
{
#if PACKET_MODE
    char MoveBuffer[5] = { 0x5F, 0x03, 0x00, 0x00, 0x00 };

    MoveBuffer[2] = ((((MoveX >> 6) << 4) & 0xF0) + ((MoveX >> 2) & 0xF)) & 0xFF;
    MoveBuffer[3] = ((((MoveX & 0x3) << 6) & 0xC0) + ((MoveY & 0x300) >> 4) + ((MoveY >> 4) & 0x0F)) & 0xFF;
    MoveBuffer[4] = ((MoveY << 4) & 0xF0);
    OriginalSend(roServer, MoveBuffer, 5, 0);
#else
    SendAttackGroundPackageFunction(0, 0, MoveX, MoveY);
    Sleep(50);
    SendAttackGroundPackageFunction(0, 0, MoveX, MoveY);

#endif

    gAutoTeleIdleCounter = 0;
}

void TelePortFunction(void) {
    AutoEquipDetectFunction(26); // 瞬移切裝需求
    SendMessage(gHwnd, WM_KEYDOWN, gAutoTeleScanCode, 0);
    if (gAutoTeleEnter) {
        Sleep(200);
        SendMessage(gHwnd, WM_KEYDOWN, 13, 0);
    }
    else if (gAutoTeleSpace) {
        Sleep(200);
        SendMessage(gHwnd, WM_KEYDOWN, 32, 0);
        Sleep(200);
        SendMessage(gHwnd, WM_KEYDOWN, 32, 0);
        Sleep(200);
        SendMessage(gHwnd, WM_KEYDOWN, 32, 0);
    }
}

void SendItemTakePackageFunction(int ItemAid)
{
    MEMORY_BASIC_INFORMATION mbi;

    int* ATTACK_ADDRESS_1 = NULL;
    int* ATTACK_ADDRESS_2 = NULL;
    int* ATTACK_ADDRESS_TRIGGER = NULL;
    int* ATTACK_ADDRESS_ITEM_AID = NULL;


    // 填上2, 觸發撿物品
    if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_1 = (int*)(*WINDOWS_LOCK_1 + 0xCC);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_2 = (int*)(*ATTACK_ADDRESS_1 + 0x2C);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_TRIGGER = (int*)(*ATTACK_ADDRESS_2 + 0x4E8);
        }
    }
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            ATTACK_ADDRESS_ITEM_AID = (int*)(*ATTACK_ADDRESS_2 + 0x4FC);
        }
    }

    // 觸發
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_TRIGGER, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_TRIGGER = 2;
        }
    }
    // 施展對象
    if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_ITEM_AID, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            *ATTACK_ADDRESS_ITEM_AID = ItemAid;
        }
    }
}

/* Init Function. Here we call the necessary functions */
void init()
{
    debugInit();
    debug("Hooking WS2_32 Functions...");
    HookWs2Functions();
    debug("WS2_32 Functions Hooked...");
    debug("Creating Main thread...");

}

/* Hook the WS2_32.dll functions */
void HookWs2Functions()
{
    // disable libary call
    DisableThreadLibraryCalls(hModule);

    // detour stuff
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    //We attach our hooked function the the original 
    /* HOOK CUSTOM FUNCTION*/

    // WS2_32.dll functions 
    DetourAttach(&(PVOID&)OriginalRecv, HookedRecv);
    DetourAttach(&(PVOID&)OriginalRecvFrom, HookedRecvFrom);
    DetourAttach(&(PVOID&)OriginalSend, HookedSend);
    DetourAttach(&(PVOID&)OriginalSendTo, HookedSendTo);
    DetourAttach(&(PVOID&)OriginalConnect, HookedConnect);
    DetourAttach(&(PVOID&)OriginalSelect, HookedSelect);
    DetourAttach(&(PVOID&)OriginalWSARecv, HookedWSARecv);
    DetourAttach(&(PVOID&)OriginalWSARecvFrom, HookedWSARecvFrom);
    DetourAttach(&(PVOID&)OriginalWSASend, HookedWSASend);
    DetourAttach(&(PVOID&)OriginalWSASendTo, HookedWSASendTo);
    DetourAttach(&(PVOID&)OriginalWSAAsyncSelect, HookedWSAAsyncSelect);

    HINSTANCE hDLL = LoadLibrary("vcruntime140.dll");
    printf("hDLL = %8X\n", hDLL);

    //int* MEMCPY_ADDRESS = reinterpret_cast <int*>(0x0D6E55C);
    //memcpy_get = (void* (__cdecl*)(void* dest, const void* src, size_t count)) (* MEMCPY_ADDRESS);

    memcpy_get = (void* (__cdecl*)(void*, const void*, size_t)) GetProcAddress(hDLL, "memcpy");
    printf("memcpy_get = %8X\n", memcpy_get);
    DetourAttach(&(PVOID&)memcpy_get, Mine_Memcpy);

    // Hook 傳送封包點
    //int* CDCLIENT_FUNCTION = reinterpret_cast <int*>(0x00D6E040);
    //OriginalRoSend = (void(__cdecl*)(char* src, int len1, char* dest, int len2, int IsTrue)) (*CDCLIENT_FUNCTION);
    //printf("OriginalRoSend = %8X\n", OriginalRoSend);
    //DetourAttach(&(PVOID&)OriginalRoSend, RoSendHook);

    DetourTransactionCommit();
    //MessageBox(NULL, L"HookWs2Functions.", L"HookWs2Functions", MB_OK);
}

void HotKeySendItem(int ItemId, int Delay) {
#if !PACKET_MODE
    int HotKeyBackup;
    *AUTO_HOTKEY_LOCATION_1 = ItemId;
    HotKeyBackup = *AUTO_HOTKEY_LOCATION_2;
    HotKeyBackup = HotKeyBackup & 0xFFFF0000;
    *AUTO_HOTKEY_LOCATION_2 = HotKeyBackup;

    HotKeyBackup = *AUTO_HOTKEY_LOCATION_3;
    HotKeyBackup = HotKeyBackup & 0x00FFFF00;
    *AUTO_HOTKEY_LOCATION_3 = HotKeyBackup;
    
    SendMessage(gHwnd, WM_KEYDOWN, 190, 0);
    Sleep(Delay);
#endif
    return;
}

BOOL TestFunc() {
    MEMORY_BASIC_INFORMATION mbi;

    int CURRENT_MAP_NAME1 = LOGIN_RSW; // login.rsw
    int CURRENT_MAP_NAME2 = 0;
    int CURRENT_MAP_NAME3 = 0;
    int CURRENT_MAP_NAME4 = 0;

    int MapChangeFlag2 = 0;

    int findRoTitleFlag = 0;
    //DWORD proc_id = 0;
    //HANDLE hProcess = NULL;

    int* WINDOWS_LOCK_2 = NULL;
    int* WINDOWS_LOCK_3 = NULL;
    int LockWindowFlag = 0;

    int mFirstTimeFlag = 1;
   
    int* CorrdX = new int;
    int* CorrdY = new int;

    while (*AidAddress == 0) {
        Sleep(1000);
    }

    findRoTitleFlag = GetConsoleHwnd();
    while (!findRoTitleFlag) {
        Sleep(1000);
        findRoTitleFlag = GetConsoleHwnd();
    }
    //FindTotalRoCharName();
    //MessageBox(NULL, L"找到RO視窗.", L"找到RO視窗.", MB_OK);

    //proc_id = GetCurrentProcessId();
    //hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_id);
    //if (!hProcess) {
    //    MessageBox(NULL, L"錯誤, 請先關閉所有RO視窗, 再切換使用者開啟RO, 然後再切回此使用者.", NULL, MB_ICONEXCLAMATION);
    //    exit(0);
    //}
    //CloseHandle(hProcess);

    while (1) {
        if (*AidAddress != 0) {
            ////
            //// 如果為選角畫面, 清除所有物品資料
            ////
            //if (*MAP_NAME == LOGIN_RSW) {
            //    ItemDataList.clear_item_node();
            //    StorageDataList.clear_item_node();
            //    PlayerAidList.clear_player_node();
            //    TargetAidList.clear_target_node();
            //    ItemAidList.clear_target_node();
            //    //PickUpItemException(); // 讀取檢物品清單
            //}

            //
            // 如果地圖轉換, 啟動無線狂蟻
            //
            if ((CURRENT_MAP_NAME1 != *MAP_NAME || CURRENT_MAP_NAME2 != *(MAP_NAME + 1)
                || CURRENT_MAP_NAME3 != *(MAP_NAME + 2) || CURRENT_MAP_NAME4 != *(MAP_NAME + 3))
                || mFirstTimeFlag) {

                //
                // 改變身後搖
                //
                if (SwitchOnFlag == 1 && gTotalSwitch) {
                    if (*MAP_NAME != LOGIN_RSW) {
                        //
                        // 先判斷是否為FFFF
                        //
                        if (*MONSTER_REGION_1 == 0xFFFFFFFF) {
                            if (*MONSTER_REGION_1 != MonsterValue) {
                                *MONSTER_REGION_1 = MonsterValue;
                            }
                        }
                    }
                }

                CURRENT_MAP_NAME1 = *MAP_NAME;
                CURRENT_MAP_NAME2 = *(MAP_NAME + 1);
                CURRENT_MAP_NAME3 = *(MAP_NAME + 2);
                CURRENT_MAP_NAME4 = *(MAP_NAME + 3);
                mFirstTimeFlag = 0;
                CloakingCounter = 0;
                CloakingTryNum = 0;
                MapChangeFlag = 1;
                MapChangeFlag2 = 1;
                gPauseDrink = 0;
                gStorageStatus = 0;
#if !PrivateServerOrNot
                *HOMUN_INTIMACY = 0;
#endif
                gAutoTeleIdleCounter = 0;

                TargetAidExpReportList.clear_target_node();
                TargetAidList.clear_target_node();
                ItemAidList.clear_target_node();



                //IsPvpMap();
                LoadDrinkSetting();
                FindTotalRoCharName();
            }
            //
            // 視窗鎖定
            //
            if (gLockWindowSwitch && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
                if (MapChangeFlag2 == 1) {
                    Sleep(1);
                    MapChangeFlag2 = 0;
                }
                if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
                    if (mbi.Protect == PAGE_READWRITE) {
                        WINDOWS_LOCK_2 = (int*)(*WINDOWS_LOCK_1 + 0xD0);
                    }
                }
                if (VirtualQuery((LPCVOID)WINDOWS_LOCK_2, &mbi, sizeof(mbi))) {
                    if (mbi.Protect == PAGE_READWRITE) {
                        WINDOWS_LOCK_3 = (int*)(*WINDOWS_LOCK_2 + 0x4E);
                    }
                }
                if (VirtualQuery((LPCVOID)WINDOWS_LOCK_3, &mbi, sizeof(mbi))) {
                    if (mbi.Protect == PAGE_READWRITE) {
                        *WINDOWS_LOCK_3 = gLockWindowValue;
                    }
                }

            }

            //
            // 改變身後搖
            //
            if (SwitchOnFlag == 1 && gTotalSwitch) {
                if (*MAP_NAME != LOGIN_RSW) {
                    if (MapChangeFlag2 == 1) {
                        Sleep(1);
                        MapChangeFlag2 = 0;
                    }
                    //
                    // 先判斷是否為FFFF
                    //
                    if (*MONSTER_REGION_1 == 0xFFFFFFFF) {
                        if (*MONSTER_REGION_1 != MonsterValue) {
                            *MONSTER_REGION_1 = MonsterValue;
                        }
                    }
                }
            }
            else {
                if (*MAP_NAME != LOGIN_RSW) {
                    //
                    // 先判斷是否為FFFF
                    //
                    if (*MONSTER_REGION_1 == MonsterValue) {
                        if (*MONSTER_REGION_1 != 0xFFFFFFFF) {
                            *MONSTER_REGION_1 = 0xFFFFFFFF;
                        }
                    }
                }
            }

            
        }

        if (IsRoFocus() && GetKeyState(35) & 0x8000 && gTotalSwitch && !PrivateServerNormal) {
            printf("偵測到一鍵吃料按鍵按下\n");
            gAutoAddBufferItem = 1;
        }

        //
        // 自動吃料理
        //
        if ((gAutoAddBufferItem || gAutoBufferLoop) && gTotalSwitch) {
            for (int AutoBufferItemCount = 0; AutoBufferItemCount < 24; AutoBufferItemCount++) {
                if (!CheckDebuffState(gAutoBufferStateID[AutoBufferItemCount])
                    && gAutoBufferItemID[AutoBufferItemCount] != 0    // ItemID為0代表不吃料
                    && !gStorageStatus                                // 非開倉狀態
                    && (CloakingTryNum >= 3 || !gAutoBufferLoop)      // 非自動吃料時快速吃料
                    //&& !MapLoadedFlag                               // 過圖後必須經過一段時間
                    ) {
                    int Id = ItemDataList.search_item(gAutoBufferItemID[AutoBufferItemCount]);

                    if (Id && Id != NON_ITEM_AMOUNT) { // 料理
                        Sleep(100);
                        if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState()) {
                            break;
                        }
                        if (gAutoBufferItemID[AutoBufferItemCount] == 100098) { // 活力激發
                            if (CheckDebuffState(271) ||
                                CheckDebuffState(273) ||
                                CheckDebuffState(272) ||
                                CheckDebuffState(275) ||
                                CheckDebuffState(274) ||
                                CheckDebuffState(276)) {
                                continue;
                            }
                        }

                        //
                        // >> Sent packet: 00A7  [Item Use] [8 bytes]   2021.10.11 02:16:12
                        // 0 > A7 00 41 00 BF A1 B8 00
                        // CD > 0a 00 a7 00 64 00 39 25 ac 00
                        ItemBuffer[ItemBufferOffset] = Id & 0xFF;
                        ItemBuffer[ItemBufferOffset + 1] = (Id >> 8) & 0xFF;
                        ItemBuffer[ItemBufferOffset + 2] = *AidAddress & 0xFF;
                        ItemBuffer[ItemBufferOffset + 3] = (*AidAddress & 0xFF00) >> 8;
                        ItemBuffer[ItemBufferOffset + 4] = (*AidAddress & 0xFF0000) >> 16;
                        ItemBuffer[ItemBufferOffset + 5] = (*AidAddress & 0xFF000000) >> 24;
                        printf("自動吃編號:%d的料理 \n", AutoBufferItemCount);
                        //printf("ItemBuffer: ");
                        //for (int i = 0; i < 10; i++) {
                        //    printf("%2x ", ItemBuffer[i]);
                        //
                        //}
                        //printf("\n");

#if PACKET_MODE
                        OriginalSend(roServer, ItemBuffer, 10 - OriginalSendOffset, 0);
#else
                        // 封包失效
                        HotKeySendItem(gAutoBufferItemID[AutoBufferItemCount], 50);
#endif


                        //Sleep(100);
                        gAutoTeleIdleCounter = 0;
                    }

                }
            }
            for (int AutoBufferItemCount = 24; AutoBufferItemCount < 40; AutoBufferItemCount++) {
                if (gAutoBufferItemSwitch[AutoBufferItemCount]        // Item 開關必須打開 (新增)
                    && !CheckDebuffState(gAutoBufferStateID[AutoBufferItemCount])
                    && gAutoBufferItemID[AutoBufferItemCount] != 0    // ItemID為0代表不吃料
                    && !gStorageStatus                                // 非開倉狀態
                    && (CloakingTryNum >= 3 || !gAutoBufferLoop)      // 非自動吃料時快速吃料
                    //&& !MapLoadedFlag                               // 過圖後必須經過一段時間
                    ) {
                    int Id = ItemDataList.search_item(gAutoBufferItemID[AutoBufferItemCount]);

                    if (Id && Id != NON_ITEM_AMOUNT) { // 料理
                        Sleep(100);
                        if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState()) {
                            break;
                        }
                        if (gAutoBufferItemID[AutoBufferItemCount] == 100098) { // 活力激發
                            if (CheckDebuffState(271) ||
                                CheckDebuffState(273) ||
                                CheckDebuffState(272) ||
                                CheckDebuffState(275) ||
                                CheckDebuffState(274) ||
                                CheckDebuffState(276)) {
                                continue;
                            }
                        }

                        //
                        // >> Sent packet: 00A7  [Item Use] [8 bytes]   2021.10.11 02:16:12
                        // 0 > A7 00 41 00 BF A1 B8 00
                        // CD > 0a 00 a7 00 64 00 39 25 ac 00
                        ItemBuffer[ItemBufferOffset] = Id & 0xFF;
                        ItemBuffer[ItemBufferOffset + 1] = (Id >> 8) & 0xFF;
                        ItemBuffer[ItemBufferOffset + 2] = *AidAddress & 0xFF;
                        ItemBuffer[ItemBufferOffset + 3] = (*AidAddress & 0xFF00) >> 8;
                        ItemBuffer[ItemBufferOffset + 4] = (*AidAddress & 0xFF0000) >> 16;
                        ItemBuffer[ItemBufferOffset + 5] = (*AidAddress & 0xFF000000) >> 24;
                        printf("自動吃編號:%d的料理 \n", AutoBufferItemCount);
                        //printf("ItemBuffer: ");
                        //for (int i = 0; i < 10; i++) {
                        //    printf("%2x ", ItemBuffer[i]);
                        //
                        //}
                        //printf("\n");

#if PACKET_MODE
                        OriginalSend(roServer, ItemBuffer, 10 - OriginalSendOffset, 0);
#else
                        // 封包失效
                        HotKeySendItem(gAutoBufferItemID[AutoBufferItemCount], 50);
#endif

                        //Sleep(100);
                        gAutoTeleIdleCounter = 0;
                    }

                }
            }

            gAutoAddBufferItem = 0;

            if (gHotKeyAutoBufferSwitch) {
                AutoBufferFunction(1);
            }

        }

        if (CloakingTryNum >= 3) {

            if (CloakingTryNum == 3) {
                Sleep(300);
                LoadDrinkSetting();
                ItemDataList.write_all_equip_data_in_setting();
                SkillDataList.write_all_skill_data_in_setting();
                FindTotalRoCharName();
                gAutoTeleIdleCounter = 0;
                CloakingTryNum++;
            }

            if (MapChangeFlag == 1) {
                MapChangeFlag = 0;
                MapChangeFlag2 = 0;
            }

            // 掛機死亡斷線
            if (*HPIndexValue <= 1 && gNearAttackMonster && gNearAttackSwitch && gNearAttackDie && gTotalSwitch) {
                exit(0);
            }

            
#if !PrivateServerOrNot
            if (!PrivateServerOrNot && gHomunIntimacySwitch && gHomunIntimacyAuto && CheckDebuffState(0x517) && gTotalSwitch) { // 招喚生命體狀態
                //printf("偵測自動吃生命體親密度\n");
                int HomunIntimacyOrg = *HOMUN_INTIMACY;
                int CountHomunIntimacy = 0;
                while (*HOMUN_INTIMACY < gHomunIntimacyValue && *HOMUN_INTIMACY > 0 && CountHomunIntimacy < 100 && CheckDebuffState(0x517)) {
//int Id = ItemDataList.search_item(100371); // 生命體營養品的ID
//if (Id && Id != NON_ITEM_AMOUNT) { // 料理
                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || !CheckDebuffState(0x517) || !gHomunIntimacySwitch || !gTotalSwitch) {
                        break;
                    }

                    //
                    // >> Sent packet: 00A7  [Item Use] [8 bytes]   2021.10.11 02:16:12
                    // 0 > A7 00 41 00 BF A1 B8 00
                    //
                    //ItemBuffer[ItemBufferOffset] = Id & 0xFF;
                    //ItemBuffer[ItemBufferOffset + 1] = (Id >> 8) & 0xFF;
                    //ItemBuffer[ItemBufferOffset + 2] = *AidAddress & 0xFF;
                    //ItemBuffer[ItemBufferOffset + 3] = (*AidAddress & 0xFF00) >> 8;
                    //ItemBuffer[ItemBufferOffset + 4] = (*AidAddress & 0xFF0000) >> 16;
                    //ItemBuffer[ItemBufferOffset + 5] = (*AidAddress & 0xFF000000) >> 24;
                    printf("自動吃生命體營養品\n");

                    SendMessage(gHwnd, WM_KEYDOWN, gHomunIntimacyAuto, 0);
                    // }
                    //} else printf("身上沒有生命體營養品\n");
                    Sleep(100);
                    //if (HomunIntimacyOrg == *HOMUN_INTIMACY) {
                    //    printf("發現沒招換生命體 停止吃生命體營養品\n");
                    //    break;
                    //}
                    //HomunIntimacyOrg = *HOMUN_INTIMACY;
                    CountHomunIntimacy++;
                }
            }
#endif
        }

        //
        // 例外排除, 某些狀態不應該反擊, 對同盟不該反擊
        //
        if (gDetectHitTriggerFlag && gTotalSwitch) {
            //
            // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
            //
            if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438) || !IsPvpMap()) {
                gDetectHitTriggerFlag = 0;
            }

            for (int DetectHitCount = 1; DetectHitCount <= 3; DetectHitCount++) {
                int X = 0;
                int Y = 0;
                int DetectHitAid = 0;

                int Switch = 0;
                int SkillId = 0;
                int IsSupport = 0;
                int IsGround = 0;
                int SkillLv = 0;
                int SkillCount = 0;
                int Delay = 0;

                if (gDetectHitTriggerFlag == DetectHitCount) { // Trigger
                    if (gDetectHitTriggerFlag == 1) {
                        Switch = gDetectSpecialSkillSwitch1;
                        SkillId = gDetectSpecialModify1;
                        IsSupport = gDetectSpecialSupport1;
                        IsGround = gDetectSpecialGround1;
                        SkillLv = gDetectSpecialModSkillLv1;
                        SkillCount = gDetectSpecialModSkillCount1;
                        Delay = gDetectSpecialDelay1;
                    }
                    else if (gDetectHitTriggerFlag == 2) {
                        Switch = gDetectSpecialSkillSwitch2;
                        SkillId = gDetectSpecialModify2;
                        IsSupport = gDetectSpecialSupport2;
                        IsGround = gDetectSpecialGround2;
                        SkillLv = gDetectSpecialModSkillLv2;
                        SkillCount = gDetectSpecialModSkillCount2;
                        Delay = gDetectSpecialDelay2;
                    }
                    else if (gDetectHitTriggerFlag == 3) {
                        Switch = gDetectSpecialSkillSwitch3;
                        SkillId = gDetectSpecialModify3;
                        IsSupport = gDetectSpecialSupport3;
                        IsGround = gDetectSpecialGround3;
                        SkillLv = gDetectSpecialModSkillLv3;
                        SkillCount = gDetectSpecialModSkillCount3;
                        Delay = gDetectSpecialDelay3;
                    }

                    if (!Switch || !gTotalSwitch) { // 開關沒開
                        continue;
                    }

                    debug("使用反擊技能");

                    // 存取目標AID
                    DetectHitAid = gDetectHitSkillBuffer;
                    //
                    // Myself AID
                    //
                    if (SkillId == 2006 || SkillId == 2317 || SkillId == 2330) {
                        gDetectHitSkillBuffer = *AidAddress;
                    }
                    // 是否對自己使用
                    if (IsSupport) {
                        gDetectHitSkillBuffer = *AidAddress;
                    }
                    // 是否是地板技能
                    if (IsGround) {

                        // 尋找目標座標
                        // 有找到目標在清單中 將x,y更新為目標所在地
                        int FindTarget = TargetAidList.search_target_Aid(DetectHitAid, CorrdX, CorrdY);
                        if (FindTarget && !IsSupport) {
                            // x, y
                            X = *CorrdX;
                            Y = *CorrdY;
                        }
                        else { // 沒找到目標在清單, 將x,y更新為本身座標
                            // x, y
                            X = *PLAYER_CORRD_X;
                            Y = *PLAYER_CORRD_Y;
                        }
                    }

                    for (int SkillUseCount = 0; SkillUseCount < SkillCount; SkillUseCount++) {
                        if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1) {
                            break;
                        }
                        //
                        // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                        //
                        if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                            break;
                        }

                        // 指定技能
                        if (!IsGround) {

                            SendAttackPackageFunction(SkillLv, SkillId, gDetectHitSkillBuffer);
                        }
                        // 地板技能
                        else {

                            SendAttackGroundPackageFunction(SkillLv, SkillId, X, Y);
                        }

                        gAutoTeleIdleCounter = 0;
                        Sleep(Delay);
                    }
                    gAutoTeleIdleCounter = 0;
                    ClearTriggerFlag();

                }
            }
        }
 
#if !PrivateServerOrNot
        //
        // 自動存倉
        //
        if (!MapChangeFlag && *WeightIndexValue > 0 && *MaxWeightTableValue > 0) {
            if ((((((*WeightIndexValue * 100) / *MaxWeightTableValue) >= gAutoStorageWeight) && gAutoStorageSwitch)  // 負重自動存倉
                || (gHotKeyStorageSwitch && IsRoFocus() && GetKeyState(gHotKeyStorageScanCode) & 0x8000))                           // 熱鍵存倉
                && gTotalSwitch) {                                                                                   // 總開關必須打開
                printf("自動存倉\n");//

                int Id = ItemDataList.search_item(14844); // 隨地倉庫(轉蛋)的ID
                //if (!Id) {
                //    Id = ItemDataList.search_item(23177); // 隨地倉庫(活動)的ID
                //}
                if (!Id) {
                    Id = ItemDataList.search_item(12211); // 隨地倉庫的ID
                }
                if (Id && Id != NON_ITEM_AMOUNT) { // 有找到物品
                    printf("找到隨倉\n");
                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1) {
                    }
                    else {
                        //
                        // >> Sent packet: 00A7  [Item Use] [8 bytes]   2021.10.11 02:16:12
                        // 0 > A7 00 41 00 BF A1 B8 00
                        //
                        char ItemBuffer[10] = { 0x0A, 0x00, 0xA7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                        char StorgeBuffer[38] = { 0x26, 0x00, 0x3B, 0x02, 0x03, 0x00, 0x31, 0x6B,
                                                  0x5A, 0x06, 0x51, 0xB5, 0x26, 0xF8, 0xF3, 0xB0,
                                                  0x15, 0xC7, 0xBC, 0xC7, 0x93, 0x30, 0xEC, 0x62,
                                                  0xE5, 0x39, 0xBB, 0x6B, 0xBC, 0x81, 0x1A, 0x60,
                                                  0xC0, 0x6F, 0xAC, 0xCB, 0x7E, 0xC8 };

                        char CloseStorageBuffer[4] = { 0x04, 0x00, 0xF7, 0x00 };

                        ItemBuffer[4] = Id & 0xFF;
                        ItemBuffer[5] = (Id >> 8) & 0xFF;
                        ItemBuffer[6] = *AidAddress & 0xFF;
                        ItemBuffer[7] = (*AidAddress & 0xFF00) >> 8;
                        ItemBuffer[8] = (*AidAddress & 0xFF0000) >> 16;
                        ItemBuffer[9] = (*AidAddress & 0xFF000000) >> 24;
                        printf("使用隨倉\n");
                        gStorageStatus = 1;

                        OriginalSend(roServer, ItemBuffer, 10 - OriginalSendOffset, 0);
                        Sleep(1000);
                        SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // Enter
                        Sleep(1000);
                        StorageDataList.clear_item_node();
                        printf("倉庫密碼1111\n");
                        OriginalSend(roServer, StorgeBuffer, 38 - OriginalSendOffset, 0);
                        Sleep(1000);
                        SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // Enter

                        Sleep(3000);

                        if (*AidAddress == 24467843 || *AidAddress == 11661673 || *AidAddress == 24859488 || *AidAddress == 24491298) {
                            gStorageSuccess = 1; // WA for storage process
                        }
                        if (gStorageSuccess) {
                            gStorageSuccess = 0;
                            StorageProcessFunction();
                            Sleep(1000);
                            OriginalSend(roServer, CloseStorageBuffer, 4 - OriginalSendOffset, 0);
                            printf("倉庫使用完畢\n");
                            StorageDataList.clear_item_node();

                            // 如果存完倉還是負重 關閉遊戲
                            if (((((*WeightIndexValue * 100) / *MaxWeightTableValue) >= gAutoStorageWeight) && gAutoStorageSwitch)  // 負重自動存倉
                                && gTotalSwitch) {
                                exit(0);
                            }

                        }
                        else {
                            printf("開倉失敗\n");
                            StorageDataList.clear_item_node();
                        }
                        
                        Sleep(2000);
                        gAutoTeleIdleCounter = 0;
                    }
                }
                else {
                    printf("身上沒有隨倉\n");
                }

                gStorageStatus = 0;
                ClearTriggerFlag();
                Sleep(100);
            }
        }
#endif

        if (gAutoTelePointTrigger) {
            TelePortFunction();
        }

#if !PrivateServerOrNot
        //
        // 自動做水
        //
        if ((IsRoFocus() && GetKeyState(gAutoPharmacyTriggerScanCode) & 0x8000)
            && gTotalSwitch
            && gAutoPharmacySwitch && gAutoPharmacyScanCode && gAutoPharmacyItemNo && gAutoPharmacyCount) {
            printf("自動做水\n");//

            if (gAutoPharmacyType == 0) {
                //
                // >> Sent packet: 00A7  [Item Use] [8 bytes]   2021.10.11 02:16:12
                // 0 > A7 00 41 00 BF A1 B8 00
                // 營養品  SendBuffer[20] = { 0x14, 0x00, 0x8E, 0x01, 0x13, 0x88, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                // 鹽酸瓶  SendBuffer[20] = { 0x14, 0x00, 0x8E, 0x01, 0xE0, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                char SendBuffer[20] = { 0x14, 0x00, 0x8E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                // 物品編號  營養品: 100371
                SendBuffer[4] = gAutoPharmacyItemNo & 0xFF;
                SendBuffer[5] = (gAutoPharmacyItemNo >> 8) & 0xFF;
                SendBuffer[6] = (gAutoPharmacyItemNo >> 16) & 0xFF;
                SendBuffer[7] = (gAutoPharmacyItemNo >> 24) & 0xFF;
                for (int count = 0; count < gAutoPharmacyCount; count++) {
                    if (!gTotalSwitch || !gAutoPharmacySwitch) {
                        break;
                    }
                    SendMessage(gHwnd, WM_KEYDOWN, gAutoPharmacyScanCode, 0); // F9
                    Sleep(gAutoPharmacyDelay);

                    //OriginalSend(roServer, SendBuffer, 20 - OriginalSendOffset, 0);
                    for (int DownArrowCount = 0; DownArrowCount < gAutoPharmacyItemNo; DownArrowCount++) {
                        SendMessage(gHwnd, WM_KEYDOWN, 40, 0); // Down Arrow	

                        Sleep(80);
                    }
                    SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // Enter
                    Sleep(80);
                    SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // Enter

                    Sleep(gAutoPharmacyDelay);
                }
            }
            else if (gAutoPharmacyType == 1) {
                //
                // 專門配藥
                char SendBuffer[10] = { 0x0A, 0x00, 0x5B, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00};
                // 物品編號  營養品: 100371
                printf("專門配藥:\n");
                SendBuffer[6] = gAutoPharmacyItemNo & 0xFF;
                SendBuffer[7] = (gAutoPharmacyItemNo >> 8) & 0xFF;
                SendBuffer[8] = (gAutoPharmacyItemNo >> 16) & 0xFF;
                SendBuffer[9] = (gAutoPharmacyItemNo >> 24) & 0xFF;
                for (int count = 0; count < gAutoPharmacyCount; count++) {
                    if (!gTotalSwitch || !gAutoPharmacySwitch) {
                        break;
                    }
                    SendMessage(gHwnd, WM_KEYDOWN, gAutoPharmacyScanCode, 0); // F9
                    Sleep(gAutoPharmacyDelay);

                    //OriginalSend(roServer, SendBuffer, 10 - OriginalSendOffset, 0);
                    for (int DownArrowCount = 0; DownArrowCount < gAutoPharmacyItemNo; DownArrowCount++) {
                        SendMessage(gHwnd, WM_KEYDOWN, 40, 0); // Down Arrow	

                        Sleep(80);
                    }

                    Sleep(gAutoPharmacyDelay);
                    printf("專門配藥 製作次數: %d\n", count+1);
                }
                SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // Enter
                Sleep(80);
                SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // Enter
                // 避免斷線
                Sleep(gAutoPharmacyDelay);
                //SendMessage(gHwnd, WM_KEYDOWN, gAutoPharmacyScanCode, 0); // F9
            
            }

            Sleep(100);
        }
#endif
        Sleep(3);
    }

    return EXIT_SUCCESS;
}

int CheckPetHungryState(void)
{
#if !PrivateServerOrNot
    for (int CheckCount = 0; CheckCount < 3; CheckCount++) {
        Sleep(1);
        if (*PET_HUNGRY >= 49) {
            return 0;
        }
    }
#endif
    return 1;
}

int CheckState(void)
{
    int Index = 0;

    while (*(STATUS_INDEX + Index) != 0xFFFFFFFF) {
        //printf("%d:\n", Index);
        if (*(STATUS_INDEX + Index) == 637) { // 無補槌
            return 0;
        }
        if (*(STATUS_INDEX + Index) == 422) { // 人孔
            return 0;
        }
        if (*(STATUS_INDEX + Index) == 107) { // 狂怒之槍
            return 0;
        }
        if (*(STATUS_INDEX + Index) == 428) { // 血腥慾望
            return 0;
        }
        if (*(STATUS_INDEX + Index) == 437) { // 冷凍
            return 0;
        }
        Index++;
    }
    return 1;
}

int CheckDebuffState(int DebuffValue)
{
    int Index = 0;

    for (int CheckCount = 0; CheckCount < 3; CheckCount++) {
        Index = 0;
        Sleep(1);
        while (*(STATUS_INDEX + Index) != 0xFFFFFFFF) {
            //printf("%d:\n", Index);
            if (*(STATUS_INDEX + Index) == DebuffValue) {
                return 1;
            }
            Index++;
        }
    }
    return 0;
}

BOOL LoadDrinkSetting() {

    //printf("讀取設定檔案!!!\n");

    //while (1) {
        //讀取ini
        CString SettingBuffer;
        int SettingBufferInt = 64;
        
        char pathCpy[50] = ".\\data\\setting\\";
        char pathOrg[50] = ".\\data\\setting.txt";
        LPCTSTR CommonPath = _T(".\\data\\setting.txt");
        LPCTSTR charPath = _T(".\\data\\charNameList.txt");

        //#define CharNameLen 24
        char CharNameStr[CharNameLen] = { "" };
        //WCHAR CharNameStr[MAX_PATH];
#if !PrivateServerOrNot
        //
        // Read setting
        //
        GetPrivateProfileString(_T("COMMON"), _T("MODIFY_MAP_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 格子圖
        SettingBufferInt = _ttoi(SettingBuffer);
        gModifyMapSwitch = SettingBufferInt;
#endif

        //while (*CharName == 0xFFFFFFFF || !*CharName || !*AidAddress) {
        //    Sleep(100);
        //
        //}

        if (*CharName != 0xFFFFFFFF && *CharName && *AidAddress) {

            gCharNameStr[0] = (*CharName & 0xFF);
            gCharNameStr[1] = ((*CharName & 0xFF00) >> 8);
            gCharNameStr[2] = ((*CharName & 0xFF0000) >> 16);
            gCharNameStr[3] = ((*CharName & 0xFF000000) >> 24);
            gCharNameStr[4] = (*(CharName + 1) & 0xFF);
            gCharNameStr[5] = ((*(CharName + 1) & 0xFF00) >> 8);
            gCharNameStr[6] = ((*(CharName + 1) & 0xFF0000) >> 16);
            gCharNameStr[7] = ((*(CharName + 1) & 0xFF000000) >> 24);
            gCharNameStr[8] = (*(CharName + 2) & 0xFF);
            gCharNameStr[9] = ((*(CharName + 2) & 0xFF00) >> 8);
            gCharNameStr[10] = ((*(CharName + 2) & 0xFF0000) >> 16);
            gCharNameStr[11] = ((*(CharName + 2) & 0xFF000000) >> 24);
            gCharNameStr[12] = (*(CharName + 3) & 0xFF);
            gCharNameStr[13] = ((*(CharName + 3) & 0xFF00) >> 8);
            gCharNameStr[14] = ((*(CharName + 3) & 0xFF0000) >> 16);
            gCharNameStr[15] = ((*(CharName + 3) & 0xFF000000) >> 24);
            gCharNameStr[16] = (*(CharName + 4) & 0xFF);
            gCharNameStr[17] = ((*(CharName + 4) & 0xFF00) >> 8);
            gCharNameStr[18] = ((*(CharName + 4) & 0xFF0000) >> 16);
            gCharNameStr[19] = ((*(CharName + 4) & 0xFF000000) >> 24);
            gCharNameStr[20] = (*(CharName + 5) & 0xFF);
            gCharNameStr[21] = ((*(CharName + 5) & 0xFF00) >> 8);
            gCharNameStr[22] = ((*(CharName + 5) & 0xFF0000) >> 16);
            gCharNameStr[23] = ((*(CharName + 5) & 0xFF000000) >> 24);
            
            strcat(pathCpy, gCharNameStr);
            strcpy_s(path, pathCpy);
            //cout << "文字:" << path << endl;

            GetPrivateProfileString(gCharNameStr, _T("TEST_TEXT"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") {
                ::WritePrivateProfileString(gCharNameStr, _T("TEST_TEXT"), _T("1"), path);
            }
            GetPrivateProfileString(gCharNameStr, _T("TEST_TEXT"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") {
                // 還是失敗, 改讀取setting
                strcpy_s(path, pathOrg);
            }

            //
            // 進階版限定功能
            //
#if !PrivateServerNormal

            GetPrivateProfileString(gCharNameStr, _T("AUTO_FIND_TARGET_SMART_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_FIND_TARGET_SMART_SWITCH"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SKILL_DELAY"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SKILL_DELAY"), _T("200"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_DELAY_1"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_DELAY_1"), _T("200"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_DELAY_2"), _T("200"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_DELAY_3"), _T("200"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_DELAY_1"), _T("200"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_DELAY_2"), _T("200"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_DELAY_3"), _T("200"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_DELAY"), _T("200"), path);
            }
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_DELAY"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_DELAY"), _T("10"), path);
            }


            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_04"), _T(""), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_04"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_05"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_04"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_05"), _T(""), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_01"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_02"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_03"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("MOVE_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("MOVE_DELAY"), _T("1000"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("MOVE_DIST"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("MOVE_DIST"), _T("30"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_01"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_02"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_03"), _T(""), path);
            }
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_POINT"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_TELE_POINT"), _T("0"), path);
            }
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CURE"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CURE"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") {
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_SWITCH"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_SMART"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") {
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_SMART"), _T("500"), path);
            }
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_DIST"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") {
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_DIST"), _T("4"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("DIAMONDDUST_EQUIP_01"), _T("-1"), SettingBuffer.GetBuffer(100), 100, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("DIAMONDDUST_EQUIP_01"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("DIAMONDDUST_EQUIP_02"), _T(""), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_3"), _T("-1"), SettingBuffer.GetBuffer(100), 100, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_SWITCH"), _T("0"), path);
                //::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_SCAN_CODE_1"), _T("0"), path);
                //::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_SCAN_CODE_2"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_HP"), _T("50"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_1"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_2"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_3"), _T(""), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_4"), _T(""), path);
            }

            //GetPrivateProfileString(gCharNameStr, _T("SMART_CLOAK_SCAN_CODE"), _T("-1"), SettingBuffer.GetBuffer(100), 100, path);
            //if (SettingBuffer == "-1") { // 建立新的ini
            //    ::WritePrivateProfileString(gCharNameStr, _T("SMART_CLOAK_SWITCH"), _T("0"), path);
            //    ::WritePrivateProfileString(gCharNameStr, _T("SMART_CLOAK_DELAY"), _T("1000"), path);
            //    ::WritePrivateProfileString(gCharNameStr, _T("SMART_CLOAK_EQUIP"), _T(""), path);
            //    //::WritePrivateProfileString(gCharNameStr, _T("SMART_CLOAK_SCAN_CODE"), _T("0"), path);
            //}

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_04"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_04"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_03"), _T("0"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03"), _T("0"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_05"), _T("0"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_03"), _T("0"), path);

            }
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_05"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_08"), _T("0"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08"), _T("0"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_05"), _T("0"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_08"), _T("0"), path);
            }


            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_06"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_08"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_08"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_08"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_08"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_08"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_06"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_07"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_08"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_05"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_05"), _T("0"), path);
            }
            
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_LOS"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_TELE_LOS"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_TELE_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_TELE_SCAN_CODE"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_TELE_IDLE"), _T("500"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_GREED_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path); // 自動貪婪
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_GREED_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_GREED_SCAN_CODE"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("CATCH_GUILD_ID_MODE"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path); // 抓取公會ID
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CATCH_GUILD_ID_MODE"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CATCH_GUILD_ID_AID"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_AID_MODE"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_AID_MODE"), _T("0"), path); // 自動攻擊aid模式
            }

            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_01"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_02"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_03"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_04"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_05"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_06"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_07"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_08"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_09"), _T("0"), CommonPath); // 自動攻擊aid模式
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_10"), _T("0"), CommonPath); // 自動攻擊aid模式
            }

            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_GROUND"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_GROUND"), _T("0"), path); // 自動打怪開關
            }
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER"), _T("0"), path); // 自動打怪開關
            }

            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SUPPORT"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SWITCH"), _T("0"), path); // 自動輔助
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SKILL"), _T("0"), path); // 自動輔助
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SKILV"), _T("0"), path); // 自動輔助
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_DIST"), _T("0"), path); // 自動輔助
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_AID_ONLY"), _T("0"), path); // 自動輔助
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SUPPORT"), _T("0"), path); // 自動輔助
            }

            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_DELAY"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_DELAY"), _T("200"), path); // 自動輔助
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_SWITCH"), _T("0"), path); // 自動補氣彈開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_SCAN_CODE"), _T("0"), path); // 自動補氣彈按鍵
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_VALUE"), _T("4"), path); // 自動補氣彈數量
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_DRINK_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_DRINK_SWITCH"), _T("0"), path); // 智能喝水開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CURE_DELAY"), _T("1000"), path); // 自動解冰和狂續延遲
            }
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_DELAY"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_DELAY"), _T("1000"), path); // 自動解冰和狂續延遲
            }

            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            if (SettingBuffer == "-1") { // 建立新的ini

                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_01"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_02"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_03"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_04"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_05"), _T("0"), CommonPath); // 自動輔助
            }
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_06"), _T("-1"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            if (SettingBuffer == "-1") { // 建立新的ini

                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_06"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_07"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_08"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_09"), _T("0"), CommonPath); // 自動輔助
                ::WritePrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_10"), _T("0"), CommonPath); // 自動輔助
            }


            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_03"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_03"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_04"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_05"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_06"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_07"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_03"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_04"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_05"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_06"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_07"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_01"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_02"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_03"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_04"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_05"), _T(""), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_03"), _T("100"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_04"), _T("100"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_05"), _T("100"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_06"), _T("100"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_07"), _T("100"), path); // 施放技能後切裝
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_02"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_01"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_02"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_01"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_02"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_01"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_02"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_03"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_04"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_05"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_01"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_02"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_03"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_04"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_05"), _T("0"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_01"), _T("100"), path); // 施放技能後切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_02"), _T("100"), path); // 施放技能後切裝
            }

            // 一鍵切裝設定
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SWITCH_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SWITCH_01"), _T("0"), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SWITCH_02"), _T("0"), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SCAN_CODE_01"), _T("0"), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SCAN_CODE_02"), _T("0"), path); // 一鍵切裝設定

                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_01"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_02"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_03"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_04"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_05"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_06"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_07"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_08"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_09"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_10"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_11"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_12"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_13"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_14"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_15"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_16"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_17"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_18"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_19"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_20"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_01"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_02"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_03"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_04"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_05"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_06"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_07"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_08"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_09"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_10"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_11"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_12"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_13"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_14"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_15"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_16"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_17"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_18"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_19"), _T(""), path); // 一鍵切裝設定
                ::WritePrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_20"), _T(""), path); // 一鍵切裝設定
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_04"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_05"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_06"), _T("0"), path); // 智能被揍切裝
            
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_05"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_06"), _T("0"), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_01"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_02"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_03"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_04"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_05"), _T(""), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_01"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_02"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_03"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_04"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_05"), _T(""), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_01"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_02"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_03"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_04"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_05"), _T(""), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_01"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_02"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_03"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_04"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_05"), _T(""), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_01"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_02"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_03"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_04"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_05"), _T(""), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_01"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_02"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_03"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_04"), _T(""), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_05"), _T(""), path); // 智能被揍切裝
            }


            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_03"), _T("0"), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_03"), _T("0"), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_03"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_05"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_03"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_05"), _T("0"), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_03"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_05"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_03"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_05"), _T("0"), path); // 智能被揍切裝

                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_03"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_05"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_01"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_02"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_03"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_04"), _T("0"), path); // 智能被揍切裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_05"), _T("0"), path); // 智能被揍切裝
            }

            // 自動接招新增
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SUPPROT_1"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SUPPROT_1"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_GROUND_1"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_FALLEN_1"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SUPPROT_2"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_GROUND_2"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_FALLEN_2"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SUPPROT_3"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_GROUND_3"), _T("0"), path); // 自動接招
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_FALLEN_3"), _T("0"), path); // 自動接招

                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SUPPORT_1"), _T("0"), path); // 自動反擊
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_GROUND_1"), _T("0"), path); // 自動反擊
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SUPPORT_2"), _T("0"), path); // 自動反擊
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_GROUND_2"), _T("0"), path); // 自動反擊
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SUPPORT_3"), _T("0"), path); // 自動反擊
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_GROUND_3"), _T("0"), path); // 自動反擊

            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_ENTER"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_TELE_ENTER"), _T("1"), path); // 自動瞬移是否按entet
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_SPACE"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_TELE_SPACE"), _T("0"), path); // 自動瞬移是否按space
            }

            GetPrivateProfileString(gCharNameStr, _T("BODY_RELOCATION"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("BODY_RELOCATION"), _T("0"), path); // 躬身代替走路
            }

            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SCREEN_MONSTER_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ARRACK_DIE"), _T("0"), path); // 死亡斷線
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SCREEN_MONSTER"), _T("5"), path); // 死亡斷線
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SCREEN_MONSTER_SWITCH"), _T("0"), path); // 死亡斷線
            }

            GetPrivateProfileString(gCharNameStr, _T("CATCH_MODE_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CATCH_MODE_SWITCH"), _T("0"), path); // 擷取模式開關
            }

            GetPrivateProfileString(gCharNameStr, _T("HOT_KEY_AUTO_BUFFER"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("HOT_KEY_AUTO_BUFFER"), _T("0"), path); // 熱鍵自動放招
            }

            GetPrivateProfileString(gCharNameStr, _T("CPU_SPEED_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CPU_SPEED_SWITCH"), _T("0"), path); // CPU SPEED
            }
            GetPrivateProfileString(gCharNameStr, _T("CPU_SPEED_CORE"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CPU_SPEED_CORE"), _T("0"), path); // CPU SPEED
            }
#endif


            //
            // 正服限定功能
            //
#if !PrivateServerOrNot

            // 自動做水
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_TRIGGER_SCAN_CODE"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_SCAN_CODE"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_TYPE"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_ITEM_NO"), _T("100371"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_COUNT"), _T("100"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_DELAY"), _T("300"), path);
            }

            // 自動傳陣
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_SWITCH_2"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_SWITCH_2"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_SWITCH_3"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_X_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_X_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_X_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_Y_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_Y_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_Y_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_DELAY"), _T("3000"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_01"), _T(".gat"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_02"), _T(".gat"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_03"), _T(".gat"), path);
            }
            GetPrivateProfileString(gCharNameStr, _T("AUTO_STORAGE_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_STORAGE_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_STORAGE_WEIGHT"), _T("70"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("HOT_KEY_STORAGE_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("HOT_KEY_STORAGE_SCAN_CODE"), _T("0"), path);
            }

            // 紀錄廣播訊息
            GetPrivateProfileString(gCharNameStr, _T("CATCH_BROAD_CAST_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CATCH_BROAD_CAST_SWITCH"), _T("0"), path);
            }
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_1"), _T("-1"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_SWITCH"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_1"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_2"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_3"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_4"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_5"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_6"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_7"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_8"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_9"), _T("0"), CommonPath);
                ::WritePrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_10"), _T("0"), CommonPath);
            }
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_X"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_SWITCH"), _T("0"), path); // 自動餵食寵物
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK"), _T("0"), path); // 自動餵食寵物
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2"), _T("0"), path); // 自動餵食寵物
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_X"), _T("0"), path); // 自動餵食寵物
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_Y"), _T("0"), path); // 自動餵食寵物
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2_X"), _T("0"), path); // 自動餵食寵物
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2_Y"), _T("0"), path); // 自動餵食寵物

            }
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_SWITCH"), _T("0"), path); // 集結後放招隊長
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_STATE"), _T("0"), path); // 集結後放招隊長
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_SCAN_CODE"), _T("0"), path); // 集結後放招隊長
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_DELAY"), _T("30000"), path); // 集結後放招隊長
            }

            // 集結獲得狀態終止暫停
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_01"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_02"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_05"), _T("0"), path);

                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_01"), _T("78"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_02"), _T("1201"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_03"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_04"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_05"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_1"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_1"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_1"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_1"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_2"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_2"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_2"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_2"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_3"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_3"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_3"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_3"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_4"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_4"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_4"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_4"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_5"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_5"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_5"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_5"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_6"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_6"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_6"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_6"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_7"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_7"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_7"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_7"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_8"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_8"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_8"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_8"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_9"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_9"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_9"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_9"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_10"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_10"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_10"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_10"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_11"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_11"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_11"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_11"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_12"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_12"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_12"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_12"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_13"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_13"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_13"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_13"), _T("500"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_14"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_14"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_LV_14"), _T("1"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_14"), _T("500"), path); // 集結後放招

                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_1"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_2"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_3"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_4"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_5"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_6"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_7"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_8"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_9"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_10"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_11"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_12"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_13"), _T("0"), path); // 集結後放招
                ::WritePrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_14"), _T("0"), path); // 集結後放招
            }


            GetPrivateProfileString(gCharNameStr, _T("LEADER_ATTACK_STOP_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("LEADER_AID"), _T("0"), path); // 隊長AID
                ::WritePrivateProfileString(gCharNameStr, _T("LEADER_ATTACK_STOP_SWITCH"), _T("0"), path); // 隊長AID
            }
            GetPrivateProfileString(gCharNameStr, _T("LEADER_DELAY"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("LEADER_DELAY"), _T("10000"), path); // 隊長AID
                ::WritePrivateProfileString(gCharNameStr, _T("LEADER_DIST"), _T("4"), path); // 隊長AID
                ::WritePrivateProfileString(gCharNameStr, _T("LEADER_AP_GIVE"), _T("0"), path); // 能量轉換
            }

            GetPrivateProfileString(gCharNameStr, _T("SMART_REF_T_POTION_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_REF_T_POTION_SWITCH"), _T("0"), path); // 黃金藥水
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_REF_T_POTION_SCAN_CODE"), _T("0"), path); // 黃金藥水
            }

            GetPrivateProfileString(gCharNameStr, _T("SMART_GHOSTRING_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_GHOSTRING_SWITCH"), _T("0"), path); // 幽波捲
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_GHOSTRING_SCAN_CODE"), _T("0"), path); // 幽波捲
            }

            // 自動洗AP
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_AP_SWITCH"), _T("0"), path); // 自動洗AP
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_AP_SKILL"), _T("0"), path); // 自動洗AP
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_AP_LV"), _T("1"), path); // 自動洗AP
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_AP_GROUND"), _T("0"), path); // 自動洗AP
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_AP_IDLE"), _T("2000"), path); // 自動洗AP
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_AP_VALUE"), _T("200"), path); // 自動洗AP
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_AP_DELAY"), _T("200"), path); // 自動洗AP
            }

            // AP自動施放
            GetPrivateProfileString(gCharNameStr, _T("AP_AUTO_SKILL_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AP_AUTO_SKILL_SWITCH"), _T("0"), path); // AP自動施放
                ::WritePrivateProfileString(gCharNameStr, _T("AP_AUTO_SKILL_SCANCODE"), _T("0"), path); // AP自動施放
                ::WritePrivateProfileString(gCharNameStr, _T("AP_AUTO_SKILL_VALUE"), _T("200"), path); // AP自動施放
            }
#endif
            
            //
            // 皆有的功能
            //
            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SWITCH"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SCAN_CODE"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_MACRO_DELAY"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SWITCH_2"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SWITCH_2"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SCAN_CODE_2"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_MACRO_DELAY_2"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("PC_ACTION_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("PC_ACTION_SWITCH"), _T("0"), path); // 人物後搖
            }



            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_1"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_1"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_1"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_1"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_1"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_2"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_2"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_2"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_2"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_3"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_3"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_3"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_3"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_4"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_4"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_4"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_4"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_5"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_5"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_5"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_5"), _T("0"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_1"), _T("1000"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_2"), _T("1000"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_3"), _T("1000"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_4"), _T("1000"), path); // 自動技能狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_5"), _T("1000"), path); // 自動技能狀態
            }


            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_01"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_01"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_02"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_03"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_04"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_05"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_06"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_07"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_08"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_09"), _T("0"), path); // 自動狀態鍵盤碼
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_01"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_02"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_03"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_04"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_05"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_06"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_07"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_08"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_09"), _T("0"), path); // 自動狀態
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_01"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_02"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_03"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_04"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_05"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_06"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_07"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_08"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_09"), _T("0"), path); // 自動狀態開關
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_01"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_02"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_03"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_04"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_05"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_06"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_07"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_08"), _T("1000"), path); // 自動狀態延遲
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_09"), _T("1000"), path); // 自動狀態延遲
            }

            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_03"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_03"), _T("0"), path); // 負面狀態03
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_03"), _T("0"), path); // 負面狀態03
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_04"), _T("0"), path); // 負面狀態04
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_04"), _T("0"), path); // 負面狀態04
            }

            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_1"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_1"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_2"), _T("0"), path);
            }
            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_3"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_3"), _T("0"), path);
            }

            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_4"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_4"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_5"), _T("0"), path);
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_6"), _T("0"), path);
            }


            GetPrivateProfileString(gCharNameStr, _T("TOTAL_SWITCH"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("TOTAL_SWITCH"), _T("0"), path); // 內掛總開關
                ::WritePrivateProfileString(gCharNameStr, _T("SMART_CASTING_AUTO"), _T("0"), path); // 智能施法

                ::WritePrivateProfileString(gCharNameStr, _T("LOCK_WINDOW_SWITCH"), _T("0"), path); // 固定視窗開關
                ::WritePrivateProfileString(gCharNameStr, _T("LOCK_WINDOW_VALUE"), _T("17463"), path); // 固定視窗值
                ::WritePrivateProfileString(gCharNameStr, _T("THANSFORM_SWITCH"), _T("0"), path); // 開啟秒七變身
                ::WritePrivateProfileString(gCharNameStr, _T("MONSTER_VALUE"), _T("3455"), path); // 秒七變身之魔物編號
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_SWITCH"), _T("0"), path); // 是否要喝水的開關
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_HP_VALUE_1"), _T("0"), path); // HP補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_HP_SCAN_CODE_1"), _T("0"), path); // HP補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_HP_DELAY_1"), _T("50"), path); // HP補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_HP_VALUE_2"), _T("0"), path); // HP2補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_HP_SCAN_CODE_2"), _T("0"), path); // HP2補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_HP_DELAY_2"), _T("50"), path); // HP2補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_SP_VALUE"), _T("0"), path); // SP補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_SP_SCAN_CODE"), _T("0"), path); // SP補品
                ::WritePrivateProfileString(gCharNameStr, _T("DRINK_SP_DELAY"), _T("50"), path); // SP補品

                ::WritePrivateProfileString(gCharNameStr, _T("CAN_SEE"), _T("0"), path); // 是否看見偽裝
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_STAND"), _T("0"), path); // 否自動起立
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_01"), _T("0"), path); // 負面狀態01
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_01"), _T("0"), path); // 負面狀態01
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_02"), _T("0"), path); // 負面狀態02
                ::WritePrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_02"), _T("0"), path); // 負面狀態02
                ::WritePrivateProfileString(gCharNameStr, _T("DIAMONDDUST_REMOVE"), _T("0"), path); // 自動解冷凍
                ::WritePrivateProfileString(gCharNameStr, _T("DIAMONDDUST_SCAN_CODE_01"), _T("0"), path); // 自動解冷凍
                ::WritePrivateProfileString(gCharNameStr, _T("DIAMONDDUST_SCAN_CODE_02"), _T("0"), path); // 自動解冷凍
                ::WritePrivateProfileString(gCharNameStr, _T("ILLUSION_REMOVE"), _T("0"), path); // 自動解幻覺


#if !PrivateServerNormal
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SWITCH"), _T("0"), path); // 敵人接近自動攻擊設定
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SKILL"), _T("0"), path); // 敵人接近自動攻擊設定
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SKILV"), _T("0"), path); // 敵人接近自動攻擊設定
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_COUNT"), _T("0"), path); // 敵人接近自動攻擊設定
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SUPPORT"), _T("0"), path); // 敵人接近自動攻擊設定
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_DIST"), _T("0"), path); // 敵人接近自動攻擊設定
                ::WritePrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_HOMUN"), _T("0"), path); // 敵人接近自動攻擊設定

                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SWITCH_1"), _T("0"), path); // 自動接招01
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SKILL_ID_1"), _T("0"), path); // 自動接招01
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MODIFY_1"), _T("0"), path); // 自動接招01
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_LV_1"), _T("1"), path); // 自動接招01
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_COUNT_1"), _T("1"), path); // 自動接招01
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SWITCH_2"), _T("0"), path); // 自動接招02
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SKILL_ID_2"), _T("0"), path); // 自動接招02
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MODIFY_2"), _T("0"), path); // 自動接招02
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_LV_2"), _T("1"), path); // 自動接招02
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_COUNT_2"), _T("1"), path); // 自動接招02
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SWITCH_3"), _T("0"), path); // 自動接招03
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SKILL_ID_3"), _T("0"), path); // 自動接招03
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MODIFY_3"), _T("0"), path); // 自動接招03
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_LV_3"), _T("1"), path); // 自動接招03
                ::WritePrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_COUNT_3"), _T("1"), path); // 自動接招03
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_SWITCH_1"), _T("0"), path); // 自動反擊01
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_ID_1"), _T("0"), path); // 自動反擊01
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MODIFY_1"), _T("0"), path); // 自動反擊01
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_LV_1"), _T("1"), path); // 自動反擊01
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_COUNT_1"), _T("1"), path); // 自動反擊01
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_SWITCH_2"), _T("0"), path); // 自動反擊02
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_ID_2"), _T("0"), path); // 自動反擊02
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MODIFY_2"), _T("0"), path); // 自動反擊02
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_LV_2"), _T("1"), path); // 自動反擊02
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_COUNT_2"), _T("1"), path); // 自動反擊02
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_SWITCH_3"), _T("0"), path); // 自動反擊03
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_ID_3"), _T("0"), path); // 自動反擊03
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MODIFY_3"), _T("0"), path); // 自動反擊03
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_LV_3"), _T("1"), path); // 自動反擊03
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_COUNT_3"), _T("1"), path); // 自動反擊03
                ::WritePrivateProfileString(gCharNameStr, _T("DETECT_SUPPORT_SKILL_SWITCH_1"), _T("0"), path); // 自動反擊輔助01
                ::WritePrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_LOOP"), _T("0"), path); // 是否自動補料理

#endif

#if !PrivateServerOrNot
                ::WritePrivateProfileString(gCharNameStr, _T("HOMUN_INTIMACY_SWITCH"), _T("0"), path); // 生命體親密度
                ::WritePrivateProfileString(gCharNameStr, _T("HOMUN_INTIMACY_AUTO"), _T("36"), path); // 生命體親密度按鍵
#endif
            }

#if !PrivateServerOrNot
            GetPrivateProfileString(gCharNameStr, _T("HOMUN_INTIMACY_VALUE"), _T("-1"), SettingBuffer.GetBuffer(32), 32, path);
            if (SettingBuffer == "-1") { // 建立新的ini
                ::WritePrivateProfileString(gCharNameStr, _T("HOMUN_INTIMACY_VALUE"), _T("501"), path);
            }
#endif

            //
            // Read setting
            //

            //
            // 進階版功能
            //
#if !PrivateServerNormal
            GetPrivateProfileString(gCharNameStr, _T("EXP_REPORT_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 經驗值紀錄
            SettingBufferInt = _ttoi(SettingBuffer);
            gExpReportSwitch = SettingBufferInt;
            if (!gExpReportSwitch) {
                ClearExpReport();
                ItemReportList.clear_item_node();
            }

            GetPrivateProfileString(gCharNameStr, _T("EXP_REPORT_CLEAR"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 經驗值紀錄
            SettingBufferInt = _ttoi(SettingBuffer);
            gExpReportClear = SettingBufferInt;
            if (gExpReportClear) {
                ClearExpReport();
                ItemReportList.clear_item_node();
                ::WritePrivateProfileString(gCharNameStr, _T("EXP_REPORT_CLEAR"), _T("0"), path); // 將清除觸發清0
            }

            GetPrivateProfileString(gCharNameStr, _T("EXP_REPORT_OBTAIN"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 經驗值紀錄
            SettingBufferInt = _ttoi(SettingBuffer);
            gExpReportObtain = SettingBufferInt;
            if (gExpReportObtain) {
                ExpReportGetFunction();
                ::WritePrivateProfileString(gCharNameStr, _T("EXP_REPORT_OBTAIN"), _T("0"), path); // 將清除觸發清0                
            }




            GetPrivateProfileString(gCharNameStr, _T("AUTO_FIND_TARGET_SMART_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能找怪開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoFindTargetSmartSwitch = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ELEME_MASTER_SUPER_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 元素破壞開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gElemeMasterSuperSwitch = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_IGNORANCE_SUPER_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 無知衰弱接替開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gIgnoranceSuperSwitch = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SKILL_DELAY"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 所有技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSkillDelay = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_DELAY_1"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitDelay1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_DELAY_2"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitDelay2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_DELAY_3"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitDelay3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_DELAY_1"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialDelay1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_DELAY_2"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialDelay2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_DELAY_3"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialDelay3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_DELAY"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackDelay = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_DELAY"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 技能的延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackMonsterDelay = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_STOP_WHEN_STATE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 此狀態不攻擊(PVP限定)
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackStopWhenState = SettingBufferInt;


            GetPrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動補氣彈開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSpiritSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動補氣彈按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSpiritScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_VALUE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動補氣彈數量
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSpiritSpiritValue = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_DRINK_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能喝水開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartDrinkSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CURE_DELAY"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path); // 自動解冰和狂續延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCureDelay = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SPIRIT_DELAY"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path); // 自動解冰和狂續延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSpiritDelay = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SWITCH_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSwitch1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SKILL_ID_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSkillID1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MODIFY_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModify1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_LV_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModSkillLv1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SUPPROT_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSupport1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_GROUND_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitGround1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_FALLEN_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitFallen1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_COUNT_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招01
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModSkillCount1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SWITCH_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSwitch2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SKILL_ID_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSkillID2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MODIFY_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModify2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_LV_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModSkillLv2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_COUNT_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModSkillCount2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SUPPROT_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSupport2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_GROUND_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitGround2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_FALLEN_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招02
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitFallen2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SWITCH_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSwitch3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SKILL_ID_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSkillID3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MODIFY_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModify3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_LV_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModSkillLv3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_MOD_SKILL_COUNT_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitModSkillCount3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_SUPPROT_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitSupport3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_GROUND_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitGround3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ATTACK_HIT_FALLEN_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動接招03
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackHitFallen3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_SWITCH_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSkillSwitch1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_ID_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSkillId1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MODIFY_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModify1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_LV_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModSkillLv1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_COUNT_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModSkillCount1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SUPPORT_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSupport1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_GROUND_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialGround1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_SWITCH_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSkillSwitch2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_ID_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSkillId2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MODIFY_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModify2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_LV_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModSkillLv2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_COUNT_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModSkillCount2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SUPPORT_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSupport2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_GROUND_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialGround2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_SWITCH_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊03
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSkillSwitch3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SKILL_ID_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊03
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSkillId3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MODIFY_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊03
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModify3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_LV_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊03
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModSkillLv3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_MOD_SKILL_COUNT_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊03
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialModSkillCount3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_SUPPORT_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊03
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialSupport3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DETECT_SPECIAL_GROUND_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動反擊03
            SettingBufferInt = _ttoi(SettingBuffer);
            gDetectSpecialGround3 = SettingBufferInt;


            GetPrivateProfileString(gCharNameStr, _T("CATCH_GUILD_ID_MODE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動抓取公會ID開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gCatchGuildIdMode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CATCH_GUILD_ID_AID"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動抓取之人的aid
            SettingBufferInt = _ttoi(SettingBuffer);
            gCatchGuildIdAid = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CURE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動點穴快開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCure = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 是否針對特定怪物
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackMonsterSpecialSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_01"), _T(""), gNearAttackMonsterSpecial01, 32, path); // 是否針對特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_02"), _T(""), gNearAttackMonsterSpecial02, 32, path); // 是否針對特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_03"), _T(""), gNearAttackMonsterSpecial03, 32, path); // 是否針對特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_04"), _T(""), gNearAttackMonsterSpecial04, 32, path); // 是否針對特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_SPECIAL_05"), _T(""), gNearAttackMonsterSpecial05, 32, path); // 是否針對特定怪物

            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 是否忽略特定怪物
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackMonsterIgnoreSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_01"), _T(""), gNearAttackMonsterIgnore01, 32, path); // 是否忽略特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_02"), _T(""), gNearAttackMonsterIgnore02, 32, path); // 是否忽略特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_03"), _T(""), gNearAttackMonsterIgnore03, 32, path); // 是否忽略特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_04"), _T(""), gNearAttackMonsterIgnore04, 32, path); // 是否忽略特定怪物
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER_IGNORE_05"), _T(""), gNearAttackMonsterIgnore05, 32, path); // 是否忽略特定怪物

            GetPrivateProfileString(gCharNameStr, _T("MOVE_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 是否自動移動打怪開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gMoveSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("MOVE_RAND_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 是否地圖隨機移動開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gMoveRandSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("MOVE_DELAY"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動移動延遲
            SettingBufferInt = _ttoi(SettingBuffer);
            gMoveDelay = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("MOVE_DIST"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動移動偵測距離
            SettingBufferInt = _ttoi(SettingBuffer);
            gMoveDist = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_GREED_SWITCH"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動貪婪開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoGreedSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_GREED_SCAN_CODE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動貪婪按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoGreedScanCode = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_SWITCH"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動撿物品開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoItemTakeSwitch = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_SMART"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動聰明撿物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoItemTakeSmart = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_DIST"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動撿物距離
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoItemTakeDist = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_FORCE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動撿物距離
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoItemTakeForce = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ITEM_TAKE_TRY_COUNT"), _T("3"), SettingBuffer.GetBuffer(32), 32, path); // 自動撿物距離
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoItemTakeTryCount = SettingBufferInt;
            

            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTeleSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_LOS"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTeleLOS = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_LOS_TRY_COUNT"), _T("2"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTeleLosTryCount = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_POINT"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 瞬移迴避傳點
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTelePoint = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTeleScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_ENTER"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移是否按ENTER
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTeleEnter = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_SPACE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移是否按ENTER
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTeleSpace = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("BODY_RELOCATION"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 躬身代替走路
            SettingBufferInt = _ttoi(SettingBuffer);
            gBodyRelocation = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_TELE_IDLE"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 自動瞬移閒置時間
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoTeleIdle = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSwitch[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSwitch[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSwitch[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSwitch[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSwitch[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSwitch[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SWITCH_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSwitch[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSkill[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSkill[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSkill[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSkill[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSkill[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSkill[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SKILL_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipSkill[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode01[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode01[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode01[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode01[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_01_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode01[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode02[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode02[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode02[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode02[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_SCAN_CODE_START_02_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipScanCode02[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_01"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipDelay[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_02"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipDelay[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_03"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipDelay[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_04"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipDelay[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_05"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipDelay[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_06"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipDelay[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_DELAY_07"), _T("100"), SettingBuffer.GetBuffer(32), 32, path); // 施放技能後切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoCastEquipDelay[6] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart03[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart03[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart03[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart03[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_03_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart03[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd03[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd03[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd03[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd03[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_03_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd03[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart04[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart04[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart04[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart04[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_04_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart04[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd04[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd04[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd04[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd04[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_04_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd04[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart05[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart05[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart05[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart05[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_05_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart05[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd05[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd05[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd05[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd05[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_05_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd05[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart06[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart06[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart06[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart06[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_06_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart06[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd06[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd06[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd06[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd06[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_06_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd06[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart07[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart07[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart07[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart07[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_START_07_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemStart07[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd07[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd07[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd07[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd07[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_CAST_EQUIP_ITEM_END_07_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 施放技能後切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoCastEquipitemEnd07[4]);


            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSwitch[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSwitch[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSwitch[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSwitch[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSwitch[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SWITCH_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSwitch[5] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSkill[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSkill[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSkill[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSkill[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSkill[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SKILL_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipSkill[5] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_DELAY_01"), _T("2000"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipDelay[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_DELAY_02"), _T("2000"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipDelay[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_DELAY_03"), _T("2000"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipDelay[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_DELAY_04"), _T("2000"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipDelay[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_DELAY_05"), _T("2000"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipDelay[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_DELAY_06"), _T("2000"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipDelay[5] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart01[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart01[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart01[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart01[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_01_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart01[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd01[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd01[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd01[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd01[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_01_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd01[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart02[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart02[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart02[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart02[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_02_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart02[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd02[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd02[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd02[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd02[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_02_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd02[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart03[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart03[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart03[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart03[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_START_03_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeStart03[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd03[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd03[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd03[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd03[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_SCAN_CODE_END_03_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能被揍切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSmartEquipScanCodeEnd03[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart04[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart04[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart04[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart04[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_04_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart04[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd04[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd04[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd04[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd04[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_04_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd04[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart05[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart05[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart05[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart05[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_05_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart05[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd05[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd05[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd05[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd05[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_05_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd05[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart06[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart06[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart06[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart06[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_START_06_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemStart06[4]);

            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd06[0]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd06[1]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_03"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd06[2]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_04"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd06[3]);
            GetPrivateProfileString(gCharNameStr, _T("AUTO_SMART_EQUIP_ITEM_END_06_05"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能被揍切裝
            ConvertEquipSetting(&SettingBuffer, &gAutoSmartEquipItemEnd06[4]);

            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SWITCH_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 一鍵切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gOneKeyEquipSwitch01 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SWITCH_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 一鍵切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gOneKeyEquipSwitch02 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SCAN_CODE_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 一鍵切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gOneKeyEquipScanCode01 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_SCAN_CODE_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 一鍵切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gOneKeyEquipScanCode02 = SettingBufferInt;

            int equipIndex = 0;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_01"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_02"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_03"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_04"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_05"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_06"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_07"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_08"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_09"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_10"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_11"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_12"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_13"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_14"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_15"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_16"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_17"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_18"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_19"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_01_20"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem01[equipIndex]);
            
            equipIndex = 0;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_01"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_02"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_03"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_04"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_05"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_06"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_07"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_08"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_09"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_10"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_11"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_12"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_13"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_14"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_15"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_16"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_17"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_18"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_19"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);
            equipIndex++;
            GetPrivateProfileString(gCharNameStr, _T("ONE_KEY_EQUIP_ITEM_02_20"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 一鍵切裝
            ConvertEquipSetting(&SettingBuffer, &gOneKeyEquipItem02[equipIndex]);


            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SWITCH_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipSwitch[7] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode[7] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode01[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode01[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode01[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode01[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_01_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode01[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode02[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode02[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode02[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode02[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_02_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode02[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode03[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode03[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode03[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode03[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_03_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode03[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode04[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode04[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode04[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode04[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_04_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode04[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode05[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode05[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode05[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode05[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_05_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode05[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode06[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode06[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode06[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode06[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_06_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode06[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode07[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode07[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode07[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode07[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_07_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode07[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode08[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode08[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode08[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode08[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_SCAN_CODE_08_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipScanCode08[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_ATTACKEQUIP_DELAY_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 攻擊特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoAttackEquipDelay[7] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SWITCH_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSwitch[7] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SCAN_CODE_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipScanCode[7] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_BACK_SCAN_CODE_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipBackScanCode[7] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILL_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkill[7] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_SKILV_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipSkilv[7] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_01"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_02"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_03"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_04"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_05"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[4] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_06"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[5] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_07"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[6] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_EQUIP_DELAY_08"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 特殊切裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoEquipDelay[7] = SettingBufferInt;


            GetPrivateProfileString(gCharNameStr, _T("LEADER_AID"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 隊長AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gLeaderSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("LEADER_DIST"), _T("4"), SettingBuffer.GetBuffer(32), 32, path); // 隊長AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gLeaderDist = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("LEADER_DELAY"), _T("4000"), SettingBuffer.GetBuffer(32), 32, path); // 隊長AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gLeaderDelay = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("LEADER_AP_GIVE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 隊長AID 能量轉換
            SettingBufferInt = _ttoi(SettingBuffer);
            gLeaderApGive = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("LEADER_ATTACK_STOP_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 隊長AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gLeaderAttackStopSwitch = SettingBufferInt;

            // 過傳點暫停使用自瞄
            GetPrivateProfileString(gCharNameStr, _T("STOP_AUTO_ATTACK_IDLE"), _T("5000"), SettingBuffer.GetBuffer(32), 32, path); // 過傳點暫停使用自瞄
            SettingBufferInt = _ttoi(SettingBuffer);
            gStopAutoAttackIdle = SettingBufferInt;

            // 擷取模式開關
            GetPrivateProfileString(gCharNameStr, _T("CATCH_MODE_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 擷取模式開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gCatchModeSwitch = SettingBufferInt;

            // 城戰顯傷開關
            GetPrivateProfileString(gCharNameStr, _T("GVG_DAMAGE_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 擷取模式開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gGvgDamageSwitch = SettingBufferInt;

            // 熱鍵補狀態開關
            GetPrivateProfileString(gCharNameStr, _T("HOT_KEY_AUTO_BUFFER"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 擷取模式開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gHotKeyAutoBufferSwitch = SettingBufferInt;

            // CPU SPEED開關
            GetPrivateProfileString(gCharNameStr, _T("CPU_SPEED_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // CPU SPEED開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gCpuSpeedSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CPU_SPEED_CORE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // CPU SPEED開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gCpuSpeedCore = SettingBufferInt;


            // 自鎖職業
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterSwitch = SettingBufferInt;

            int JobFilterNum = 0;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_10"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_11"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_12"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_13"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_14"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_15"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_16"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_17"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;

            JobFilterNum++;
            GetPrivateProfileString(gCharNameStr, _T("JOB_FILTER_LIST_18"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自鎖職業
            SettingBufferInt = _ttoi(SettingBuffer);
            gJobFilterList[JobFilterNum] = SettingBufferInt;


            // 集結後放招隊長
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_SWITCH"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLeaderSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_STATE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLeaderState = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_SCAN_CODE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLeaderScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LEADER_DELAY"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLeaderDelay = SettingBufferInt;

            // 集結獲得狀態終止暫停
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopSwitch[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopSwitch[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopSwitch[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopSwitch[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_SWITCH_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopSwitch[4] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopBuffer[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopBuffer[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopBuffer[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopBuffer[3] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_GET_BUFF_STOP_BUFF_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioGetBufferStopBuffer[4] = SettingBufferInt;


            // 集結後放招
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioTotalSwitch = SettingBufferInt;

            int ConvenioNum = 0;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_4"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_4"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_4"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_4"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_5"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_5"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_5"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_5"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_6"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_6"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_6"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_6"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_6"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_7"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_7"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_7"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_7"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_7"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_8"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_8"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_8"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_8"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_8"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_9"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_9"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_9"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_9"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_9"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_10"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_10"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_10"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_10"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_10"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_11"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_11"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_11"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_11"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_11"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_12"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_12"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_12"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_12"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_12"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_13"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_13"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_13"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_13"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_13"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            ConvenioNum++;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SWITCH_14"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSwitch[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_SKILL_14"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioSkill[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_LV_14"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioLv[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_DELAY_14"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioDelay[ConvenioNum] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("CONVENIO_JOB_14"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gConvenioJob[ConvenioNum] = SettingBufferInt;

            // 自動洗AP
            GetPrivateProfileString(gCharNameStr, _T("AP_AUTO_SKILL_SWITCH"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // AP自動施放
            SettingBufferInt = _ttoi(SettingBuffer);
            gApAutoSkillSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AP_AUTO_SKILL_SCANCODE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // AP自動施放
            SettingBufferInt = _ttoi(SettingBuffer);
            gApAutoSkillScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AP_AUTO_SKILL_VALUE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // AP自動施放
            SettingBufferInt = _ttoi(SettingBuffer);
            gApAutoSkillValue = SettingBufferInt;

            // 自動洗AP
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_SWITCH"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoApSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_SKILL"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoApSkill = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_LV"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoApLv = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_GROUND"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoApGround = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_IDLE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoApIdle = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_VALUE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoApValue = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_AP_DELAY"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 自動洗AP
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoApDelay = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_FOLLOW_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動跟隨設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoFollowSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_FOLLOW_AID"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動跟隨設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoFollowAid = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_FOLLOW_DIST"), _T("5"), SettingBuffer.GetBuffer(32), 32, path); // 自動跟隨設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoFollowDist = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SKILL"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportSkill = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SKILV"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportSkilv = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_DIST"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportDist = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_DELAY"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportDelay = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_AID_ONLY"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidOnly = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_SUPPORT_SUPPORT"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportSupport = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[0] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[1] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[2] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[3] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[4] = SettingBufferInt;

            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[5] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[6] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[7] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[8] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_SUPPORT_AID_10"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 自動輔助設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSupportAidTable[9] = SettingBufferInt;

            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[0] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[1] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[2] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[3] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[4] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[5] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[6] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[7] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[8] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_ENEMY_AID_10"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearEnemyAidTable[9] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_AID_MODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 針對敵人AID
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackAidMode = SettingBufferInt;


            GetPrivateProfileString(gCharNameStr, _T("NEAR_ARRACK_DIE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackDie = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SCREEN_MONSTER_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackScreenMonsterSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SCREEN_MONSTER"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackScreenMonster = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SKILL"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackSkill = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SKILV"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackSkilv = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_COUNT"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackCount = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_SUPPORT"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackSupport = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_GROUND"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackGround = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_DIST"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackDist = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_HOMUN"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gAttackPvpDot = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("NEAR_ATTACK_MONSTER"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 敵人接近自動攻擊設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearAttackMonster = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("NEAR_SKILL_PVP_STOP_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 技能成功後暫停
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSkillPvpStopSwitch = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("NEAR_SKILL_PVP_STOP_TIME"), _T("10000"), SettingBuffer.GetBuffer(32), 32, path); // 技能成功後暫停
            SettingBufferInt = _ttoi(SettingBuffer);
            gNearSkillPvpStopTime = SettingBufferInt;
#endif



            //
            // 正服限定功能
            //
#if !PrivateServerOrNot
            GetPrivateProfileString(gCharNameStr, _T("HOMUN_INTIMACY_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 生命體親密度
            SettingBufferInt = _ttoi(SettingBuffer);
            gHomunIntimacySwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("HOMUN_INTIMACY_VALUE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 生命體親密度
            SettingBufferInt = _ttoi(SettingBuffer);
            gHomunIntimacyValue = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("HOMUN_INTIMACY_AUTO"), _T("36"), SettingBuffer.GetBuffer(32), 32, path); // 生命體親密度按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gHomunIntimacyAuto = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_STORAGE_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動倉庫開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoStorageSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_STORAGE_WEIGHT"), _T("70"), SettingBuffer.GetBuffer(32), 32, path); // 自動倉庫負重設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoStorageWeight = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("HOT_KEY_STORAGE_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 熱鍵倉庫開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gHotKeyStorageSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("HOT_KEY_STORAGE_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 熱鍵倉庫按鍵設定
            SettingBufferInt = _ttoi(SettingBuffer);
            gHotKeyStorageScanCode = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("SMART_REF_T_POTION_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動黃金藥水開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartRefTPotionSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_REF_T_POTION_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動黃金藥水按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartRefTPotionScanCode = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("SMART_GHOSTRING_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動幽波捲開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartGhostringSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_GHOSTRING_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動幽波捲按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartGhostringScanCode = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動餵食寵物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPetFeedSwitch = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動餵食寵物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPetFeedLock = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動餵食寵物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPetFeedLock2 = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_X"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動餵食寵物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPetFeedLock_X = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_Y"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動餵食寵物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPetFeedLock_Y = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2_X"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動餵食寵物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPetFeedLock2_X = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2_Y"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動餵食寵物
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPetFeedLock2_Y = SettingBufferInt;
            

            // 自動傳陣
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpSwitch[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_SWITCH_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpSwitch[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_SWITCH_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpSwitch[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_X_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpCorrdX[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_X_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpCorrdX[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_X_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpCorrdX[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_Y_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpCorrdY[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_Y_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpCorrdY[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_CORRD_Y_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpCorrdY[2] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_DELAY"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpDelay = SettingBufferInt;

            //GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_01"), _T(""), gAutoWarpMap[0], 32, path); // 自動傳陣
            //GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_02"), _T(""), gAutoWarpMap[1], 32, path); // 自動傳陣
            //GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_03"), _T(""), gAutoWarpMap[2], 32, path); // 自動傳陣

            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣 下點次數
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpMap[0] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣 下點次數
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpMap[1] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_WARP_MAP_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動傳陣 下點次數
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoWarpMap[2] = SettingBufferInt;

            // 自動做水
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動做水
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPharmacySwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_TRIGGER_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動做水
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPharmacyTriggerScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動做水
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPharmacyScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_TYPE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動做水
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPharmacyType = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_ITEM_NO"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動做水
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPharmacyItemNo = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_COUNT"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動做水
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPharmacyCount = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_PHARMACY_DELAY"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動做水
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoPharmacyDelay = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("CATCH_BROAD_CAST_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 開啟記錄廣播訊息
            SettingBufferInt = _ttoi(SettingBuffer);
            gCatchBroadCastSwitch = SettingBufferInt;

            
#endif

            
            //
            // 皆有的功能
            //
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_SWITCH"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeSwitch = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[0] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[1] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_3"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[2] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_4"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[3] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_5"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[4] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_6"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[5] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_7"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[6] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_8"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[7] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_9"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[8] = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("GM_MODE_AID_10"), _T("1"), SettingBuffer.GetBuffer(32), 32, CommonPath); // GM模式
            SettingBufferInt = _ttoi(SettingBuffer);
            gGmModeAid[9] = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("PC_ACTION_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 後搖開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gPcActionSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("TOTAL_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 內掛總開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gTotalSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_AUTO"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能施法
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCastingAuto = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能施法按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCastingScanCode1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能施法按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCastingScanCode2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能施法按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCastingScanCode3 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能施法按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCastingScanCode4 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能施法按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCastingScanCode5 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_CASTING_SCAN_CODE_6"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能施法按鍵
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCastingScanCode6 = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("LOCK_WINDOW_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 固定視窗開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gLockWindowSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("LOCK_WINDOW_VALUE"), _T("17463"), SettingBuffer.GetBuffer(32), 32, path); // 固定視窗值
            SettingBufferInt = _ttoi(SettingBuffer);
            gLockWindowValue = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("THANSFORM_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 開啟秒七變身
            SettingBufferInt = _ttoi(SettingBuffer);
            SwitchOnFlag = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("MONSTER_VALUE"), _T("3455"), SettingBuffer.GetBuffer(32), 32, path); // 秒七變身之魔物編號
            SettingBufferInt = _ttoi(SettingBuffer);
            MonsterValue = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 是否要喝水的開關
            SettingBufferInt = _ttoi(SettingBuffer);
            gDrinkSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_HP_VALUE_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // HP補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoHpDrinkValue1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_HP_SCAN_CODE_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // HP補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoHpScanCode1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_HP_DELAY_1"), _T("50"), SettingBuffer.GetBuffer(32), 32, path); // HP補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gDrinkHpDelay1 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_HP_VALUE_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // HP2補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoHpDrinkValue2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_HP_SCAN_CODE_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // HP2補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoHpScanCode2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_HP_DELAY_2"), _T("50"), SettingBuffer.GetBuffer(32), 32, path); // HP2補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gDrinkHpDelay2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_SP_VALUE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // SP補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSpDrinkValue = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_SP_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // SP補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoSpScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DRINK_SP_DELAY"), _T("50"), SettingBuffer.GetBuffer(32), 32, path); // SP補品
            SettingBufferInt = _ttoi(SettingBuffer);
            gDrinkSpDelay = SettingBufferInt;
            
            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動巨集
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoMacroSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SCAN_CODE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動巨集
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoMacroScanCode = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_DELAY"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動巨集
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoMacroDelay = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SWITCH_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動巨集
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoMacroSwitch2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_SCAN_CODE_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動巨集
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoMacroScanCode2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_MACRO_DELAY_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動巨集
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoMacroDelay2 = SettingBufferInt;


            GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_SWITCH"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能壓血
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartBloodSwitch = SettingBufferInt;
            //GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_SCAN_CODE_1"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能壓血
            //SettingBufferInt = _ttoi(SettingBuffer);
            //gSmartBloodScanCode1 = SettingBufferInt;
            //GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_SCAN_CODE_2"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能壓血
            //SettingBufferInt = _ttoi(SettingBuffer);
            //gSmartBloodScanCode2 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_HP"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能壓血
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartBloodHp = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_1"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 智能壓血
            ConvertEquipSetting(&SettingBuffer, &gSmartBloodEquip01);
            GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_2"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 智能壓血
            ConvertEquipSetting(&SettingBuffer, &gSmartBloodEquip02);
            GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_3"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 智能壓血
            ConvertEquipSetting(&SettingBuffer, &gSmartBloodEquip03);
            GetPrivateProfileString(gCharNameStr, _T("SMART_BLOOD_EQUIP_4"), _T("0"), SettingBuffer.GetBuffer(100), 100, path); // 智能壓血
            ConvertEquipSetting(&SettingBuffer, &gSmartBloodEquip04);


            GetPrivateProfileString(gCharNameStr, _T("SMART_CLOAK_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 智能偽裝切披肩
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCloakSwitch = SettingBufferInt;

            GetPrivateProfileString(gCharNameStr, _T("SMART_CLOAK_EQUIP"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 智能偽裝切披肩
            ConvertEquipSetting(&SettingBuffer, &gSmartCloakEquip);

            GetPrivateProfileString(gCharNameStr, _T("SMART_CLOAK_DELAY"), _T("200"), SettingBuffer.GetBuffer(32), 32, path); // 智能偽裝切披肩
            SettingBufferInt = _ttoi(SettingBuffer);
            gSmartCloakDelay = SettingBufferInt;

            //GetPrivateProfileString(gCharNameStr, _T("SMART_CLOAK_SCAN_CODE"), _T("1"), SettingBuffer.GetBuffer(32), 32, path); // 智能偽裝切披肩
            //SettingBufferInt = _ttoi(SettingBuffer);
            //gSmartCloakScanCode = SettingBufferInt;

            


            GetPrivateProfileString(gCharNameStr, _T("CAN_SEE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 是否看見偽裝
            SettingBufferInt = _ttoi(SettingBuffer);
            gCanSee = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_STAND"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 否自動起立
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoStandSwitch = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuff01 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態01
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuffScanCode01 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuff02 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuffScanCode02 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuff03 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuffScanCode03 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuff04 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DEBUFF_SCAN_CODE_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 負面狀態02
            SettingBufferInt = _ttoi(SettingBuffer);
            gDebuffScanCode04 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DIAMONDDUST_REMOVE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動解冷凍
            SettingBufferInt = _ttoi(SettingBuffer);
            gDiamonddustRemove = SettingBufferInt;
            //GetPrivateProfileString(gCharNameStr, _T("DIAMONDDUST_SCAN_CODE_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動解冷凍
            //SettingBufferInt = _ttoi(SettingBuffer);
            //gDiamonddustScanCode01 = SettingBufferInt;
            //GetPrivateProfileString(gCharNameStr, _T("DIAMONDDUST_SCAN_CODE_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動解冷凍
            //SettingBufferInt = _ttoi(SettingBuffer);
            //gDiamonddustScanCode02 = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("DIAMONDDUST_EQUIP_01"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 自動解冷凍
            ConvertEquipSetting(&SettingBuffer, &gDiamonddustEquip01);
            GetPrivateProfileString(gCharNameStr, _T("DIAMONDDUST_EQUIP_02"), _T(""), SettingBuffer.GetBuffer(100), 100, path); // 自動解冷凍
            ConvertEquipSetting(&SettingBuffer, &gDiamonddustEquip02);
            GetPrivateProfileString(gCharNameStr, _T("ILLUSION_REMOVE"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 自動解幻覺
            SettingBufferInt = _ttoi(SettingBuffer);
            gIllusionRemove = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_LOOP"), _T("0"), SettingBuffer.GetBuffer(32), 32, path); // 是否自動補料理
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferLoop = SettingBufferInt;

            //
            // 讀取自動狀態清單
            //
            int j = 0;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_01"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_02"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_03"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_04"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_05"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_06"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_07"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_08"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_KEY_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferKey[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBuffer[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_SWITCH_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFF_DELAY_09"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferDelay[j] = SettingBufferInt;
            j++;

            //
            // 讀取自動狀態清單
            //
            j = 0;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillName[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillState[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillLv[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_1"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillName[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillState[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillLv[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_2"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillName[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillState[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillLv[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_3"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillName[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillState[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_4"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillLv[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillDelay[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_NAME_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillName[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_STATE_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillState[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_SWITCH_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_LV_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillLv[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("AUTO_BUFFER_SKILL_DELAY_5"), _T("1000"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferSkillDelay[j] = SettingBufferInt;
            j++;

            //
            // 表情放招
            //
            j = 0;

            GetPrivateProfileString(gCharNameStr, _T("EMO_SWITCH_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_SKILL_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSkill[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_MODE_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoMode[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_TYPE_1"), _T("3"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoType[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_IS_SELF_1"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoIsSelf[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("EMO_SWITCH_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_SKILL_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSkill[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_MODE_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoMode[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_TYPE_2"), _T("3"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoType[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_IS_SELF_2"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoIsSelf[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("EMO_SWITCH_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_SKILL_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSkill[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_MODE_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoMode[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_TYPE_3"), _T("3"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoType[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_IS_SELF_3"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoIsSelf[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("EMO_SWITCH_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_SKILL_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSkill[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_MODE_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoMode[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_TYPE_4"), _T("3"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoType[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_IS_SELF_4"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoIsSelf[j] = SettingBufferInt;
            j++;

            GetPrivateProfileString(gCharNameStr, _T("EMO_SWITCH_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSwitch[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_SKILL_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoSkill[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_MODE_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoMode[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_TYPE_5"), _T("3"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoType[j] = SettingBufferInt;
            GetPrivateProfileString(gCharNameStr, _T("EMO_IS_SELF_5"), _T("0"), SettingBuffer.GetBuffer(32), 32, path);
            SettingBufferInt = _ttoi(SettingBuffer);
            gEmoIsSelf[j] = SettingBufferInt;

            //
            // 同盟公會清單
            //
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_GUID_ID_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gGuidId01 = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_GUID_ID_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gGuidId02 = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_GUID_ID_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gGuidId03 = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_GUID_ID_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gGuidId04 = SettingBufferInt;
            GetPrivateProfileString(_T("COMMON"), _T("NEAR_GUID_ID_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gGuidId05 = SettingBufferInt;

            //
            // 讀取自動料理清單
            //
            int i = 0;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_01"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_02"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_03"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_04"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_05"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_06"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_07"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_08"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_09"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_10"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_10"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;

            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_11"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_11"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_12"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_12"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_13"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_13"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_14"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_14"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_15"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_15"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_16"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_16"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_17"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_17"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_18"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_18"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_19"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_19"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_20"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_20"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;

            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_21"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_21"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_22"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_22"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_23"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_23"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_24"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_24"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_25"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_25"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_25_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_26"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_26"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_26_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_27"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_27"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_27_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_28"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_28"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_28_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_29"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_29"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_29_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_30"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_30"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_30_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;

            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_31"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_31"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_31_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_32"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_32"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_32_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_33"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_33"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_33_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_34"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_34"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_34_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_35"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_35"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_35_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_36"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_36"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_36_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_37"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_37"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_37_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_38"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_38"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_38_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_39"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_39"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_39_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_ITEM_ID_40"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_STATE_ID_40"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferStateID[i] = SettingBufferInt;
            GetPrivateProfileString(_T("AUTO_BUFFER"), _T("AUTO_BUFFER_40_SWITCH"), _T("0"), SettingBuffer.GetBuffer(32), 32, CommonPath);
            SettingBufferInt = _ttoi(SettingBuffer);
            gAutoBufferItemSwitch[i] = SettingBufferInt;
            i++;


            /*
            //讀取到的資料存進inilastday，如果找不到就顯示"no find"；path 可以替代為路徑
            //寫入ini
            wprintf(L"ROSupport_01: %s\n", (LPCWSTR)SettingBuffer);

            SettingBufferInt = _ttoi(SettingBuffer);

            printf("int: %d\n", SettingBufferInt);

            if (SettingBuffer == "2035") {
                cout << "1他是2035.\n";
            }

            //::WritePrivateProfileString(_T("ROSupport_01"), _T("MONSTER_VALUE"), _T("2035"), path);*/
            SettingBuffer.ReleaseBuffer(); //清除緩衝區

        //}
        //Sleep(1000);
    }

    return EXIT_SUCCESS;
}


BOOL SmartBloodFunction() {

	if (PACKET_MODE) {
		return 1;
	}
	
    //MEMORY_BASIC_INFORMATION mbi;
    //int* SKILL_ID = 0;
    //int SkillId = 0;
    //int PlayerAidAddress = 0;
    //int* PlayerAidValue = 0;
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gSmartBloodSwitch) {
            //
            // 自動壓血
            //
            if (((*HPIndexValue * 100 / *MaxHPTableValue) > gSmartBloodHp) && !gDetactSpecialAttack && !gSmartBloodAttack) { // 生命小於百分比和未被攻擊時
                //if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
                //    if (mbi.Protect == PAGE_READWRITE) {
                //        SKILL_ID = (int*)(*WINDOWS_LOCK_1 + 4);
                //    }
                //}
                //if (VirtualQuery((LPCVOID)SKILL_ID, &mbi, sizeof(mbi))) {
                //    if (mbi.Protect == PAGE_READWRITE) {
                //        SkillId = *SKILL_ID;
                //    }
                //}

                //// 掃描玩家AID
                //// Read Player AID
                ////
                //PlayerAidAddress = *WINDOWS_LOCK_1 + 236;
                //PlayerAidValue = reinterpret_cast <int*>(PlayerAidAddress);

                //printf("SkillId: %d, PlayerAidValue: %d\n", SkillId, *PlayerAidValue);

                //if (SkillId == 2343 && *PlayerAidValue) { // 當技能圈圈為羅剎時 2343 且滑鼠有指到敵人

                Sleep(1); // 避免lag
                if (((*HPIndexValue * 100 / *MaxHPTableValue) > gSmartBloodHp) && gSmartBloodTrigger) {
                    printf("壓血\n");
                    SendEquipCommand(gSmartBloodEquip02, 0, 0, 5, 0, 1);
                    SendEquipCommand(gSmartBloodEquip04, 0, 0, 5, 0, 1);

                    SendEquipCommand(gSmartBloodEquip01, 0, 0, 5, 0, 1);
                    SendEquipCommand(gSmartBloodEquip03, 0, 0, 5, 0, 1);

                    //SendMessage(gHwnd, WM_KEYDOWN, gSmartBloodScanCode2, 0);
                    //Sleep(80);

                    //SendMessage(gHwnd, WM_KEYDOWN, gSmartBloodScanCode1, 0);
                    //Sleep(80);

                    //SendMessage(gHwnd, WM_KEYDOWN, gSmartBloodScanCode1, 0);
                    //Sleep(80);
                    gSmartBloodAttack = 1;
                    gSmartBloodAttackIdleCounter = 1000;
                }

            }
            gSmartBloodTrigger = 0;
        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL AutoCastEquipFunction1() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1 && gCastEquipTrigger[0] && gAutoCastEquipSwitch[0]) {
                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoCastEquipScanCode01[ScanCodeNum]) {
                            Sleep(gAutoCastEquipDelay[0]);
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoCastEquipScanCode01[ScanCodeNum], 0);
                        }
                    }
                }

                gCastEquipTrigger[0] = 0;
            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}

BOOL AutoCastEquipFunction2() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1 && gCastEquipTrigger[1] && gAutoCastEquipSwitch[1]) {

                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoCastEquipScanCode02[ScanCodeNum]) {
                            Sleep(gAutoCastEquipDelay[1]);
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoCastEquipScanCode02[ScanCodeNum], 0);
                        }
                    }
                }

                gCastEquipTrigger[1] = 0;
            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}


BOOL AutoCastEquipStartFunction(int Index, int CheckEquipSuccess) {

#if PACKET_MODE
    int SleepTime = 5;
#else
    int SleepTime = 80;
#endif

    if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
        //
        // 自動智能切裝
        //
        if (*HPIndexValue > 1 && gAutoCastEquipSwitch[Index]) {

            for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {

                switch (Index) {
                case 2: 
                    SendEquipCommand(gAutoCastEquipitemStart03[ScanCodeNum], 0, 0, 80, 0, CheckEquipSuccess);
                    break;

                case 3:
                    SendEquipCommand(gAutoCastEquipitemStart04[ScanCodeNum], 0, 0, 80, 0, CheckEquipSuccess);
                    break;

                case 4:
                    SendEquipCommand(gAutoCastEquipitemStart05[ScanCodeNum], 0, 0, 80, 0, CheckEquipSuccess);
                    break;

                case 5:
                    SendEquipCommand(gAutoCastEquipitemStart06[ScanCodeNum], 0, 0, 80, 0, CheckEquipSuccess);
                    break;

                case 6:
                    SendEquipCommand(gAutoCastEquipitemStart07[ScanCodeNum], 0, 0, 80, 0, CheckEquipSuccess);
                    break;

                default:
                    break;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

BOOL AutoCastEquipEndFunction() {

#if PACKET_MODE
    int SleepTime = 10;
#else
    int SleepTime = 80;
#endif

    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1) {

                if (gCastEquipTrigger[2] && gAutoCastEquipSwitch[2]) {
                    Sleep(gAutoCastEquipDelay[2]);
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        SendEquipCommand(gAutoCastEquipitemEnd03[ScanCodeNum], 0, 1, 80, 0, 1);
                    }
                }
                else if (gCastEquipTrigger[3] && gAutoCastEquipSwitch[3]) {
                    Sleep(gAutoCastEquipDelay[3]);
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        SendEquipCommand(gAutoCastEquipitemEnd04[ScanCodeNum], 0, 1, 80, 0, 1);
                    }
                }
                else if (gCastEquipTrigger[4] && gAutoCastEquipSwitch[4]) {
                    Sleep(gAutoCastEquipDelay[4]);
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        SendEquipCommand(gAutoCastEquipitemEnd05[ScanCodeNum], 0, 1, 80, 0, 1);
                    }
                }
                else if (gCastEquipTrigger[5] && gAutoCastEquipSwitch[5]) {
                    Sleep(gAutoCastEquipDelay[5]);
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        SendEquipCommand(gAutoCastEquipitemEnd06[ScanCodeNum], 0, 1, 80, 0, 1);
                    }
                }
                else if (gCastEquipTrigger[6] && gAutoCastEquipSwitch[6]) {
                    Sleep(gAutoCastEquipDelay[6]);
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        SendEquipCommand(gAutoCastEquipitemEnd07[ScanCodeNum], 0, 1, 80, 0, 1);
                    }
                }
                
            }
        }

        gCastEquipTrigger[2] = 0;
        gCastEquipTrigger[3] = 0;
        gCastEquipTrigger[4] = 0;
        gCastEquipTrigger[5] = 0;
        gCastEquipTrigger[6] = 0;
        Sleep(10);
    }
    return EXIT_SUCCESS;
}



BOOL AutoSmartEquipFunction1() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1 && gSmartEquipTrigger[0] && gAutoSmartEquipSwitch[0]) {

                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoSmartEquipScanCodeStart01[ScanCodeNum]) {
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoSmartEquipScanCodeStart01[ScanCodeNum], 0);
                            Sleep(140);
                        }
                    }
                }
                // 等待不被此技能打到時 切回
                while (gSmartEquipTrigger[0]) {
                    Sleep(10);
                }
                // 切回
                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoSmartEquipScanCodeEnd01[ScanCodeNum]) {
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoSmartEquipScanCodeEnd01[ScanCodeNum], 0);
                            Sleep(140);
                        }
                    }
                }
            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}

BOOL AutoSmartEquipFunction2() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1 && gSmartEquipTrigger[1] && gAutoSmartEquipSwitch[1]) {

                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoSmartEquipScanCodeStart02[ScanCodeNum]) {
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoSmartEquipScanCodeStart02[ScanCodeNum], 0);
                            Sleep(140);
                        }
                    }
                }
                // 等待不被此技能打到時 切回
                while (gSmartEquipTrigger[1]) {
                    Sleep(10);
                }
                // 切回
                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoSmartEquipScanCodeEnd02[ScanCodeNum]) {
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoSmartEquipScanCodeEnd02[ScanCodeNum], 0);
                            Sleep(140);
                        }
                    }
                }
            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}

BOOL AutoSmartEquipFunction3() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1 && gSmartEquipTrigger[2] && gAutoSmartEquipSwitch[2]) {

                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoSmartEquipScanCodeStart03[ScanCodeNum]) {
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoSmartEquipScanCodeStart03[ScanCodeNum], 0);
                            Sleep(140);
                        }
                    }
                }
                // 等待不被此技能打到時 切回
                while (gSmartEquipTrigger[2]) {
                    Sleep(10);
                }
                // 切回
                for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                    for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                        if (gAutoSmartEquipScanCodeEnd03[ScanCodeNum]) {
                            SendMessage(gHwnd, WM_KEYDOWN, gAutoSmartEquipScanCodeEnd03[ScanCodeNum], 0);
                            Sleep(140);
                        }
                    }
                }
            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}

BOOL AutoSmartEquipFunction4() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1) {

                if (gSmartEquipTrigger[3] && gAutoSmartEquipSwitch[3]) { // AUTO_SMART_EQUIP_SWITCH_04
                    for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                        for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                            SendEquipCommand(gAutoSmartEquipItemStart04[ScanCodeNum], 0, 0, 20, 0, 1);
                        }
                        //Sleep(200);
                    }
                    // 等待不被此技能打到時 切回
                    while (gSmartEquipTrigger[3]) {
                        Sleep(10);
                    }
                    // 切回
                    for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                        for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                            SendEquipCommand(gAutoSmartEquipItemEnd04[ScanCodeNum], 0, 0, 20, 0, 1);
                        }
                        //Sleep(200);
                    }
                }
            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}

BOOL AutoSmartEquipFunction5() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1) {

                if (gSmartEquipTrigger[4] && gAutoSmartEquipSwitch[4]) { // AUTO_SMART_EQUIP_SWITCH_05
                    for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                        for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                            SendEquipCommand(gAutoSmartEquipItemStart05[ScanCodeNum], 0, 0, 20, 0, 1);
                        }
                        //Sleep(200);
                    }
                    // 等待不被此技能打到時 切回
                    while (gSmartEquipTrigger[4]) {
                        Sleep(10);
                    }
                    // 切回
                    for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                        for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                            SendEquipCommand(gAutoSmartEquipItemEnd05[ScanCodeNum], 0, 0, 20, 0, 1);
                        }
                        //Sleep(200);
                    }
                }

            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}

BOOL AutoSmartEquipFunction6() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // 自動智能切裝
            //
            if (*HPIndexValue > 1) {

                if (gSmartEquipTrigger[5] && gAutoSmartEquipSwitch[5]) { // AUTO_SMART_EQUIP_SWITCH_06
                    for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                        for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                            SendEquipCommand(gAutoSmartEquipItemStart06[ScanCodeNum], 0, 0, 20, 0, 1);
                        }
                        //Sleep(200);
                    }
                    // 等待不被此技能打到時 切回
                    while (gSmartEquipTrigger[5]) {
                        Sleep(10);
                    }
                    // 切回
                    for (int EquipCount = 0; EquipCount < 1; EquipCount++) {
                        for (int ScanCodeNum = 0; ScanCodeNum < 5; ScanCodeNum++) {
                            SendEquipCommand(gAutoSmartEquipItemEnd06[ScanCodeNum], 0, 0, 20, 0, 1);
                        }
                        //Sleep(200);
                    }
                }

            }
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}


BOOL AutoHpDrink() {
    //InterceptionContext context = interception_create_context();
    //InterceptionKeyStroke keyStroke[2];
    int Rerty = 0;
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gAutoHpDrinkValue1 && gAutoHpScanCode1 && !IsPvpForbiddenMap()) {
            //if (gDelayAutoDrink == 1) {
            //    Sleep(2000);
            //    gDelayAutoDrink = 0;
            //}
            if (gDrinkSwitch == 1) {
                //printf("gAutoHpDrinkValue1: %d\n", gAutoHpDrinkValue1);
                //printf("HPIndexValue: %d\n", *HPIndexValue * 100 / *MaxHPTableValue);
                if (((*HPIndexValue * 100 / *MaxHPTableValue < gAutoHpDrinkValue1)   // 生命百分比條件
                    && ((*HPIndexValue * 100 / *MaxHPTableValue >= gAutoHpDrinkValue2) || !gAutoHpDrinkValue2) // HP2條件不能達成
                    && *HPIndexValue > 1                                            // HP大於1才吃水
                    //&& gHPItemNumber != -1 && gHPItemAmount > 0                   // 當HP物品數量大於1
                    //&& gStorageStatus != 1                                        // 當不是開倉狀態
                    && gPauseDrink != 1                                             // 不連續吃超過30次
                    ) || gDetactSpecialAttack && gAutoSmartDrinkSwitch && (*HPIndexValue * 100 / *MaxHPTableValue < 95)) { // 強制喝水
                    if (CheckState() == 1) {
                        printf("吃HP水!\n");
                        Rerty++;
                        SendMessage(gHwnd, WM_KEYDOWN, gAutoHpScanCode1, 0);
                        Sleep(gDrinkHpDelay1);
                        if (Rerty > RETYR_NUM) {
                            //gPauseDrink = 1;
                        }
                    }
                }
                else {
                    Rerty = 0;
                }
            }
        }
        Sleep(5);
    }
    return EXIT_SUCCESS;
}

BOOL AutoHpDrink2() {
    //InterceptionContext context = interception_create_context();
    //InterceptionKeyStroke keyStroke[2];
    int Rerty = 0;
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gAutoHpDrinkValue2 && gAutoHpScanCode2 && !IsPvpForbiddenMap()) {
            //if (gDelayAutoDrink == 1) {
            //    Sleep(2000);
            //    gDelayAutoDrink = 0;
            //}
            if (gDrinkSwitch == 1) {
                if (((*HPIndexValue * 100 / *MaxHPTableValue < gAutoHpDrinkValue2)  // 當目前生命小於85%
                    && *HPIndexValue > 1                                          // HP大於1才吃水
                    //&& gHPItemNumber2 != -1 && gHPItemAmount2 > 0                   // 當HP物品數量大於1
                    //&& gStorageStatus != 1
                    && gPauseDrink != 1                                             // 不連續吃超過30次
                    ) || gDetactSpecialAttack && gAutoSmartDrinkSwitch && (*HPIndexValue * 100 / *MaxHPTableValue < 95)) { // 強制喝水
                    if (CheckState() == 1) {
                        printf("吃HP2水!\n");
                        Rerty++;
                        SendMessage(gHwnd, WM_KEYDOWN, gAutoHpScanCode2, 0);
                        Sleep(gDrinkHpDelay2);
                        if (Rerty > RETYR_NUM) {
                            //gPauseDrink = 1;
                        }
                    }
                }
                else {
                    Rerty = 0;
                }
            }
        }
        Sleep(5);
    }
    return EXIT_SUCCESS;
}

BOOL AutoSpDrink() {
    /*   InterceptionContext context = interception_create_context();
       InterceptionKeyStroke keyStroke[2];*/
    int Rerty = 0;
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gAutoSpDrinkValue && gAutoSpScanCode && !IsPvpForbiddenMap()) {
            //if (gDelayAutoDrink == 1) {
            //    Sleep(2000);
            //    gDelayAutoDrink = 0;
            //}
            if (gDrinkSwitch == 1) {
                //
                // 吃SP補品
                //
                if ((*SPIndexValue * 100 / *MaxSPTableValue < gAutoSpDrinkValue)    // 當目前生命小於85%
                    //&& gSPItemNumber != -1 && gSPItemAmount > 0                     // 當SP物品數量大於1
                    //&& gStorageStatus != 1
                    && gPauseDrink != 1                                             // 不連續吃超過30次
                    ) {
                    if (CheckState() && !CheckDebuffState(88)) { // 阿修羅霸凰拳-禁止恢復SP 不喝水
                        printf("吃SP水!\n");
                        Rerty++;
                        SendMessage(gHwnd, WM_KEYDOWN, gAutoSpScanCode, 0);
                        Sleep(gDrinkSpDelay);
                        if (Rerty > RETYR_NUM) {
                            //gPauseDrink = 1;
                        }
                    }
                }
                else {
                    Rerty = 0;
                }
            }
        }
        Sleep(5);
    }
    return EXIT_SUCCESS;
}

int IsHandCusor(void) {
    MEMORY_BASIC_INFORMATION mbi;
    int* SMART_CASEING = 0;
    int SMART_CASEING_STATUS = 0;
    if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            SMART_CASEING = (int*)(*WINDOWS_LOCK_1 + 80);
        }
    }
    if (VirtualQuery((LPCVOID)SMART_CASEING, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            SMART_CASEING_STATUS = *SMART_CASEING;
        }
    }

    // 游標必須是手指
    if (SMART_CASEING_STATUS == 2) { // 手指: 2
        return 1;
    }
    return 0;
}

int PetLockXY(void) {
#if !PrivateServerOrNot
    while (1) {

        if (gAutoPetFeedLock && IsHandCusor()) {

            //
            // int轉化成字串
            //
            char AutoPetFeedLock_X_Text[20];
            char AutoPetFeedLock_Y_Text[20];
            sprintf(AutoPetFeedLock_X_Text, "%d", *CURSOR_CORRD_X);
            sprintf(AutoPetFeedLock_Y_Text, "%d", *CURSOR_CORRD_Y);

            ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK"), _T("0"), path); // 自動餵食寵物

            ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_X"), AutoPetFeedLock_X_Text, path); // 自動餵食寵物
            ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_Y"), AutoPetFeedLock_Y_Text, path); // 自動餵食寵物

            // 紀錄當前的X Y座標
            gAutoPetFeedLock = 0;
            gAutoPetFeedLock_X = *CURSOR_CORRD_X;
            gAutoPetFeedLock_Y = *CURSOR_CORRD_Y;
        }

        if (gAutoPetFeedLock2 && IsHandCusor()) {

            //
            // int轉化成字串
            //
            char AutoPetFeedLock2_X_Text[20];
            char AutoPetFeedLock2_Y_Text[20];
            sprintf(AutoPetFeedLock2_X_Text, "%d", *CURSOR_CORRD_X);
            sprintf(AutoPetFeedLock2_Y_Text, "%d", *CURSOR_CORRD_Y);

            ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2"), _T("0"), path); // 自動餵食寵物

            ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2_X"), AutoPetFeedLock2_X_Text, path); // 自動餵食寵物
            ::WritePrivateProfileString(gCharNameStr, _T("AUTO_PET_FEED_LOCK_2_Y"), AutoPetFeedLock2_Y_Text, path); // 自動餵食寵物

            // 紀錄當前的X Y座標
            gAutoPetFeedLock2 = 0;
            gAutoPetFeedLock2_X = *CURSOR_CORRD_X;
            gAutoPetFeedLock2_Y = *CURSOR_CORRD_Y;

        }
        Sleep(100);
    }
#endif
    return EXIT_SUCCESS;
}

BOOL SkillSendByKeboard (int Aid, int IsMyself, int ScanCode, int CorrdX, int CorrdY) {
#if 0
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gNearSupportSwitch && gTotalSwitch) {
            // 滑鼠游標指到右邊
            MEMORY_BASIC_INFORMATION mbi;
            int* TargetAidValue = NULL;
            int pixel = 0;
            int CurrentGreenX = 0;

            int RealCorrdX = 0;
            int RealCorrdY = 0;

            int ResolutionX = *RESOLUTION_CORRD_X / 2;
            int ResolutionY = *RESOLUTION_CORRD_Y / 2;

            /* 產生 [min , max] 的整數亂數 */
            int x = rand() % (25 - -10 + 1) -10;

            // 滑鼠移到右邊
            *CURSOR_CORRD_X = ResolutionX + 64;
            *CURSOR_CORRD_Y = ResolutionY + x;
            Sleep(100);

            // 查看鼠標是否有目標AID
            if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    TargetAidValue = (int*)(*WINDOWS_LOCK_1 + 0xEC);
                    if (VirtualQuery((LPCVOID)TargetAidValue, &mbi, sizeof(mbi))) {
                        if (gNearSupportAidOnly) {
                            if ((IsSpecialAid2(*TargetAidValue))) {
                                printf("符合目標AID1 %d, 使用輔助\n", *TargetAidValue);
                                SendMessage(gHwnd, WM_KEYDOWN, 112, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);

                                Sleep(gNearSupportDelay);

                                *TargetAidValue = 0;
                            }
                        }
                        else if (PlayerAidList.search_player(*TargetAidValue) || (IsSpecialAid2(*TargetAidValue))) {
                            printf("符合目標AID2 %d, 使用輔助\n", *TargetAidValue);
                            SendMessage(gHwnd, WM_KEYDOWN, 112, 0);
                            Sleep(10);
                            SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                            Sleep(10);
                            SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);

                            Sleep(gNearSupportDelay);

                            *TargetAidValue = 0;
                        }
                    }
                }
            }
            

        }
        Sleep(100);
    
    }
//#else
    MEMORY_BASIC_INFORMATION mbi;
    int* TargetAidValue = NULL;
    int pixel = 0;
    int CurrentGreenX = 0;

    int RealCorrdX = 0;
    int RealCorrdY = 0;

    int ResolutionX = *RESOLUTION_CORRD_X / 2;
    int ResolutionY = *RESOLUTION_CORRD_Y / 2;

    //// 先算出一格多少像素
    //// 鼠標置中
    //*CURSOR_CORRD_X = ResolutionX;
    //*CURSOR_CORRD_Y = ResolutionY;
    //Sleep(1);

    //CurrentGreenX = *REAL_CORRD_X;

    //while (*REAL_CORRD_X == CurrentGreenX && pixel < 1000) {
    //    Sleep(1);

    //    *CURSOR_CORRD_X = ResolutionX + pixel;
    //    pixel++;
    //}

    //pixel = pixel * 2;

    printf("Pixel: %d\n", pixel);
    pixel = 60;

    // 計算目標座標
    RealCorrdX = CorrdX  - *PLAYER_CORRD_X;
    RealCorrdY = *PLAYER_CORRD_Y - CorrdY;

    *CURSOR_CORRD_X = (*RESOLUTION_CORRD_X / 2) + (RealCorrdX * pixel);
    *CURSOR_CORRD_Y = (*RESOLUTION_CORRD_Y / 2) + (RealCorrdY * pixel);

    Sleep(500);

    // 查看鼠標是否有目標AID
    if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_READWRITE) {
            TargetAidValue = (int*)(*WINDOWS_LOCK_1 + 0xEC);
            if (VirtualQuery((LPCVOID)TargetAidValue, &mbi, sizeof(mbi))) {
                // 是否為目標AID
                if (*TargetAidValue == Aid) {
                    SendMessage(gHwnd, WM_KEYDOWN, ScanCode, 0);
                    Sleep(10);
                    SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                    Sleep(10);
                    SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);

                    return EXIT_SUCCESS;
                }
            }
        }
    }

    //for (int Y = 0; Y <= *RESOLUTION_CORRD_Y; Y++) {
    //    for (int X = 0; X <= *RESOLUTION_CORRD_X; X++) {
    //        // 移動游標位置
    //        *CURSOR_CORRD_X = X;
    //        *CURSOR_CORRD_Y = Y;

    //        // 查看鼠標是否有目標AID
    //        if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
    //            if (mbi.Protect == PAGE_READWRITE) {
    //                TargetAidValue = (int*)(*WINDOWS_LOCK_1 + 0xEC);
    //                if (VirtualQuery((LPCVOID)TargetAidValue, &mbi, sizeof(mbi))) {
    //                    // 是否為目標AID
    //                    if (*TargetAidValue == Aid) {
    //                        SendMessage(gHwnd, WM_KEYDOWN, ScanCode, 0);
    //                        Sleep(10);
    //                        SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
    //                        Sleep(10);
    //                        SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);

    //                        return EXIT_SUCCESS;
    //                    }
    //                }
    //            }
    //        }
    //        
    //    }
    //}
#endif

    return EXIT_SUCCESS;
}

BOOL AutoDetectStatusDo() {
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
            //
            // Check sit and do stand
            //
            // >> Sent packet: 0437  [Character Move] [7 bytes]   2021.10.12 22:28:58
            //    0 > 37 04 00 00 00 00 03
            //
            if (gAutoStandSwitch) {
                if (CheckDebuffState(622)) {
                    //char StandBuffer[9] = { 0x09, 0x00, 0x37, 0x04, 0x00, 0x00, 0x00, 0x00, 0x03 };
                    printf("站起來\n");
                    Sleep(100);
  
                    SendMessage(gHwnd, WM_KEYDOWN, 45, 0);
                    Sleep(100);
                }
            }

            //
            // Check Debuff01
            //
            if (gDebuff01) {
                if (CheckDebuffState(gDebuff01)) {
                    printf("解除負面狀態01\n");
                    SendMessage(gHwnd, WM_KEYDOWN, gDebuffScanCode01, 0);
                    Sleep(200);
                }
            }

            //
            // Check Debuff02
            //
            if (gDebuff02) {
                if (CheckDebuffState(gDebuff02)) {
                    printf("解除負面狀態02\n");
                    SendMessage(gHwnd, WM_KEYDOWN, gDebuffScanCode02, 0);
                    Sleep(200);
                }
            }

            //
            // Check Debuff03
            //
            if (gDebuff03) {
                if (CheckDebuffState(gDebuff03)) {
                    printf("解除負面狀態03\n");
                    SendMessage(gHwnd, WM_KEYDOWN, gDebuffScanCode03, 0);
                    Sleep(200);
                }
            }

            //
            // Check Debuff04
            //
            if (gDebuff04) {
                if (CheckDebuffState(gDebuff04)) {
                    printf("解除負面狀態04\n");
                    SendMessage(gHwnd, WM_KEYDOWN, gDebuffScanCode04, 0);
                    Sleep(200);
                }
            }

            //
            // 自動解鑽石狀態
            //
            if (gDiamonddustRemove) {
                while (CheckDebuffState(437) && *SPIndexValue > 0) {
                    printf("嘗試切裝解除冷凍\n");

                    SendEquipCommand(gDiamonddustEquip02, 1, 1, 80, 0, 1);
                    SendEquipCommand(gDiamonddustEquip01, 1, 1, 80, 0, 1);

                    //SendMessage(gHwnd, WM_KEYDOWN, gDiamonddustScanCode02, 0);
                    //Sleep(50);
                    //
                    //SendMessage(gHwnd, WM_KEYDOWN, gDiamonddustScanCode01, 0);
                    //Sleep(50);
                    //
                    //SendMessage(gHwnd, WM_KEYDOWN, gDiamonddustScanCode01, 0);
                    //Sleep(50);
                }
            }

            //
            // 偵測自動吃黃金藥水
            //
            if (gSmartRefTPotionSwitch && gSmartRefTPotionScanCode) { // 開關必須開
                if ((gDetactYouAreCast || gDetactSpecialAttack) && IsTeMap()){
                    // 被揍時或是在放招
                    if (!CheckDebuffState(1169)) { // 黃金抵抗藥水狀態
                        printf("吃黃金抵抗藥水\n");
                        SendMessage(gHwnd, WM_KEYDOWN, gSmartRefTPotionScanCode, 0);
                        Sleep(100);
                    }
                }
            
            }

            //
            // 偵測自動吃幽波捲
            //
            if (gSmartGhostringSwitch && gSmartGhostringScanCode) { // 開關必須開
                if ((gDetactYouAreCastGhostring || gDetactSpecialAttack) && IsPvpMap()) {
                    // 被揍時或是在放招
                    if (!CheckDebuffState(302)) { // 幽波捲狀態
                        printf("吃幽波捲\n");
                        SendMessage(gHwnd, WM_KEYDOWN, gSmartGhostringScanCode, 0);
                        Sleep(100);
                    }
                }

            }
#if !PrivateServerOrNot
            //
            // 解除幻覺
            //
            if (CheckDebuffState(0x22) && gIllusionRemove) {
                MEMORY_BASIC_INFORMATION mbi;
                int* WINDOWS_LOCK_2 = NULL;
                int* ILLUSION_LOCK_1 = NULL;
                int* ILLUSION_LOCK_2 = NULL;
                int* ILLUSION_LOCK_3 = NULL;

                if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
                    if (mbi.Protect == PAGE_READWRITE) {
                        WINDOWS_LOCK_2 = (int*)(*WINDOWS_LOCK_1 + 0xD0);
                    }
                }
                if (VirtualQuery((LPCVOID)WINDOWS_LOCK_2, &mbi, sizeof(mbi))) {
                    if (mbi.Protect == PAGE_READWRITE) {
                        ILLUSION_LOCK_1 = (int*)(*WINDOWS_LOCK_2 + 0x8C);
                        ILLUSION_LOCK_2 = (int*)(*WINDOWS_LOCK_2 + 0x90);
                        ILLUSION_LOCK_3 = (int*)(*WINDOWS_LOCK_2 + 0x94);

                        *ILLUSION_DOPING_REGION = 0; // 清掉0x2710
                        *ILLUSION_LOCK_1 = 0;
                        *ILLUSION_LOCK_2 = 0x3F800000;
                        *ILLUSION_LOCK_3 = 0;
                    }
                }

            }
#endif
        }
        Sleep(10);
    }
    return EXIT_SUCCESS;
}

BOOL AutoBufferFunction(int CallByFunction) {
#if !PrivateServerOrNot
    char SkillBuffer[12] = { 0x0c, 0x00, 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
    char NearSkillBufferGround[13] = { 0x0D, 0x00, 0xF4, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#else
    char SkillBuffer[10] = { 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
    char NearSkillBufferGround[11] = { 0xF4, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#endif
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {
#if !PrivateServerOrNot
            // 集結隊長自動放招
            if (!CheckDebuffState(gConvenioLeaderState)
                && gConvenioLeaderSwitch                             // 開關必須是開的
                && gConvenioTotalSwitch                              // 開關必須是開的
                && !gStorageStatus                                   // 非開倉狀態
                && CloakingTryNum >= 3                               // 過圖後必須經過一段時間
                && gTotalSwitch
                && !gConvenioTrigger) {

                Sleep(100);
                
                printf("自動唱集結 \n");
                SendMessage(gHwnd, WM_KEYDOWN, gConvenioLeaderScanCode, 0);
                Sleep(200);
                SendMessage(gHwnd, WM_KEYDOWN, gConvenioLeaderScanCode, 0);
                Sleep(200);
                SendMessage(gHwnd, WM_KEYDOWN, gConvenioLeaderScanCode, 0);
                Sleep(200);
                SendMessage(gHwnd, WM_KEYDOWN, gConvenioLeaderScanCode, 0);
                Sleep(200);
                gAutoTeleIdleCounter = 0;
            }

            //
            // 自動AP技能
            //
            if (gApAutoSkillSwitch                               // 開關必須是開的
                && !gStorageStatus                               // 非開倉狀態
                && CloakingTryNum >= 2                           // 過圖後必須經過一段時間
                && gTotalSwitch
                && gCurrentApValue >= gApAutoSkillValue) {
                Sleep(1000);
                if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1) {

                }
                else if (gCurrentApValue >= gApAutoSkillValue) {
                    SendMessage(gHwnd, WM_KEYDOWN, gApAutoSkillScanCode, 0);;
                }
            }
#endif
            if(!CallByFunction && (gHotKeyAutoBufferSwitch || gNearAttackMonster)) { // 熱鍵模式不自動補 與自動打怪模式
                Sleep(10);
                continue;
            }

            //
            // 自動狀態
            //
            for (int AutoBufferCount = 0; AutoBufferCount < 9; AutoBufferCount++) {
                if (!CheckDebuffState(gAutoBuffer[AutoBufferCount])
                    && gAutoBufferSwitch[AutoBufferCount]                             // 開關必須是開的
                    && !gStorageStatus                                                // 非開倉狀態
                    && CloakingTryNum >= 3                                            // 過圖後必須經過一段時間
                    && gTotalSwitch
                    && gAutoMonsterTeleIdleCounter == 0) { // 被集結不要補

                    Sleep(50);

                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1) {
                        break;
                    }
                    printf("自動狀態編號:%d \n", AutoBufferCount);
                    SendMessage(gHwnd, WM_KEYDOWN, gAutoBufferKey[AutoBufferCount], 0);

                    Sleep(gAutoBufferDelay[AutoBufferCount]);

                    gAutoTeleIdleCounter = 0;
                }
            }

            //
            // 自動技能狀態
            //
            for (int AutoBufferCount = 0; AutoBufferCount < 5; AutoBufferCount++) {
                if (!CheckDebuffState(gAutoBufferSkillState[AutoBufferCount])
                    && gAutoBufferSkillSwitch[AutoBufferCount]        // 開關必須是開的
                    && !gStorageStatus                                                    // 非開倉狀態
                    && CloakingTryNum >= 3                                                // 過圖後必須經過一段時間
                    && gTotalSwitch
                    && gAutoMonsterTeleIdleCounter == 0) { // 被集結不要補

                    Sleep(50);
                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1) {
                        break;
                    }
                    printf("自動技能狀態編號:%d \n", AutoBufferCount);

                    SendSkillCastFunction(gAutoBufferSkillName[AutoBufferCount], gAutoBufferSkillLv[AutoBufferCount], *AidAddress);
                    //SendMessage(gHwnd, WM_KEYDOWN, gAutoBufferKey[AutoBufferCount], 0);

                    Sleep(gAutoBufferSkillDelay[AutoBufferCount]);
                    gAutoTeleIdleCounter = 0;
                }
            }



        }
        Sleep(10);

        if (CallByFunction) {
            //printf("自動狀態CallByFunction返回 \n");
            return EXIT_SUCCESS;
        }
    }
    return EXIT_SUCCESS;
}

//
// 偽裝出來切回披肩
//
BOOL AutoSmartCloakFunction() {
    
    gAutoSmartCloakFlag = 1;
    while (1) {
        //if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gAutoSmartCloakFlag) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch) {

            // 進入偽裝狀態, gAutoSmartCloakFlag為1
            if (CheckDebuffState(5)) {
                gAutoSmartCloakFlag = 1;
            }

            if (!CheckDebuffState(5)
                && gSmartCloakSwitch                             // 開關必須是開的
                && !gStorageStatus                               // 非開倉狀態
                && CloakingTryNum >= 1                           // 過圖後必須經過一段時間
                && *HPIndexValue > 1                             // 血要大於1
                && gTotalSwitch
                && gAutoSmartCloakFlag) {

                for (int SmartCloakCount = 0; SmartCloakCount < gSmartCloakDelay; SmartCloakCount++) {
                    Sleep(1);

                    if (CheckDebuffState(5)) { // 等的途中又進入偽裝狀態
                        SmartCloakCount = 0;
                        gAutoSmartCloakFlag = 1;
                    }
                }

                if (!CheckDebuffState(5)) {
                    //SendEquipCommand(gSmartCloakEquip, 0, 0, 10, 0);
                    while (SkillDataList.IsSkillIdCanUse(135)) { // 還有偽裝狀態
                        //SendMessage(gHwnd, WM_KEYDOWN, gSmartCloakScanCode, 0);
                        SendEquipCommand(gSmartCloakEquip, 0, 0, 10, 0, 1);
                        Sleep(100);
                    }
                    gAutoSmartCloakFlag = 0;
                }
            }
            
        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL CloakingFunction() {

    int* CloakingMemoryRegionValue = NULL;
    int CloakingValue1 = 184; // 0xB8
    int CloakingValue2 = 0xF423F;
    MEMORY_BASIC_INFORMATION mbi;

    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW) {
            
            if (CloakingCounter < 500) {
                CloakingCounter++;
            }

            //
            // 無限狂蟻
            //
            if (CloakingCounter >= 500 && CloakingTryNum < 3) {

                CloakingCounter = 0;
                //LoadDrinkSetting();
                if (*MAP_NAME != LOGIN_RSW) {
                    //
                    // Find the ecx value for cloaking region.
                    //
                    Sleep(50);
                    CloakingMemoryRegionValue = (int*)(*CLOAKING_MEMORY_REGION);
                    if (VirtualQuery((LPCVOID)CloakingMemoryRegionValue, &mbi, sizeof(mbi))) {
                        if (mbi.Protect == PAGE_READWRITE && gCanSee && gTotalSwitch) {
                            *CloakingMemoryRegionValue = CloakingValue1;
                            CloakingMemoryRegionValue += 4;
                            *CloakingMemoryRegionValue = CloakingValue2;
                        }
                    }

                }

                CloakingTryNum++;
            }

        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

// 確認此AID是否在隊伍裡面
int CheckTheAidIsInParty(int Aid) {

    for (int i = 0; i < gPartyInfoLen; i++) {
        if (Aid == gPartyInfoAid[i]) {
            return 1;
        }
    }
    return 0;
}

// 確認此AID是否在隊伍裡面
int CheckTheAidIsInParty2(int Aid, int* X, int* Y, char* Map) {

    for (int i = 0; i < gPartyInfoLen; i++) {
        if (Aid == gPartyInfoAid[i] && gPartyInfoOnline[i]) {

            *X = gPartyInfoX[i];
            *Y = gPartyInfoY[i];
            strcpy(Map, gPartyInfoMap[i]);

            return 1;
        }
    }

    return 0;
}

//BOOL DetectKeyboardDevice() {
//    InterceptionContext context_detect = interception_create_context();
//    InterceptionKeyStroke stroke;
//
//    interception_set_filter(context_detect, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
//    cout << "開始尋找keyboard device..." << endl;
//    while (interception_receive(context_detect, device = interception_wait(context_detect), (InterceptionStroke*)&stroke, 1) > 0)
//    {
//        interception_send(context_detect, device, (InterceptionStroke*)&stroke, 1);
//
//        if (device > 0) {
//            cout << "找到Keyboard device:" << device << endl;
//            break;
//        }
//    }
//    cout << "離開: device:" << device << endl;
//    interception_destroy_context(context_detect);
//
//    return EXIT_SUCCESS;
//}

BOOL GetPackageBuffer() {
    Sleep(100);
    while (1) {
        Sleep(1);
        //if (*AidAddress && *MAP_NAME != LOGIN_RSW) {
        int result = 0;

        result = PacketDataList.pass_data_buffer();
        while (!result) {
            Sleep(1);
            result = PacketDataList.pass_data_buffer();
        }

        PacketDataList.delete_head_node();
        //}
    }

    return EXIT_SUCCESS;
}

VOID DetactSpecialAttackFunction(VOID) {

    Sleep(1000);
    gDetactSpecialAttack = 1;
    gDetactSpecialAttackIdleCounter = 1000;

    return;
}

int itemAddedStart = 0;

BOOL PacketAnalyze2(char* dataBufferDeliver, int len) {
    // 封包名稱
    int packetName = (dataBufferDeliver[0] & 0xFF) + ((dataBufferDeliver[1] & 0xFF) << 8); // 封包名稱

    // 如果封包名稱剛好等於長度, 代表是CD解密封包 先忽略
    if (packetName == len) {

        return 1;
    }

    //
    // 09FD - actor_display (actor moved) [98 bytes]
    //
    if (packetName == ActorMoved) { // actor moved

        int AidTmp = ((dataBufferDeliver[5]) & 0xFF) + (((dataBufferDeliver[6]) & 0xFF) << 8)
            + (((dataBufferDeliver[7]) & 0xFF) << 16) + (((dataBufferDeliver[8]) & 0xFF) << 24);
        // 座標
        int enemyXFrom = ((((dataBufferDeliver[67]) & 0xFF) << 12) + (((dataBufferDeliver[68]) & 0xFF) << 4) + (((dataBufferDeliver[69]) & 0xFF) >> 4)) >> 10;
        int enemyYFrom = ((((dataBufferDeliver[67]) & 0xFF) << 12) + (((dataBufferDeliver[68]) & 0xFF) << 4) + (((dataBufferDeliver[69]) & 0xFF) >> 4)) & 0x3FF;

        //
        // 發現隊長集結, 停止瞬移10秒
        //
        if (gTotalSwitch && gLeaderSwitch) {
            if (AidTmp == gLeaderAid && gAutoMonsterTeleIdleCounter < 200 && gLeaderIdleCounter > 0) {
                // 必須距離9格內
                if (((abs(*PLAYER_CORRD_X - enemyXFrom) * abs(*PLAYER_CORRD_X - enemyXFrom)) + (abs(*PLAYER_CORRD_Y - enemyYFrom) * abs(*PLAYER_CORRD_Y - enemyYFrom))) <= (gLeaderDist * gLeaderDist)) {
                    gAutoMonsterTeleIdleCounter = gLeaderDelay;
                    if (gLeaderApGive) {
                        //Sleep(1000);
                        SendSkillCastFunction(5368, 5, gLeaderAid);
                    }
                    // 被集結自動放招
                    if (!gConvenioTrigger && gConvenioTotalSwitch) {
                        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ConvenioFunction), NULL, 0, 0);
                    }
                }

            }
        }

    }

    //
    // 09FF - actor_display (actor exists) [92 bytes]
    //
    if (packetName == ActorExists) { // actor exists

        int AidTmp = ((dataBufferDeliver[5]) & 0xFF) + (((dataBufferDeliver[6]) & 0xFF) << 8)
            + (((dataBufferDeliver[7]) & 0xFF) << 16) + (((dataBufferDeliver[8]) & 0xFF) << 24);
        int enemyXFrom = ((((dataBufferDeliver[63]) & 0xFF) << 12) + (((dataBufferDeliver[64]) & 0xFF) << 4) + (((dataBufferDeliver[65]) & 0xFF) >> 4)) >> 10;
        int enemyYFrom = ((((dataBufferDeliver[63]) & 0xFF) << 12) + (((dataBufferDeliver[64]) & 0xFF) << 4) + (((dataBufferDeliver[65]) & 0xFF) >> 4)) & 0x3FF;


        //
        // 發現隊長集結, 停止瞬移10秒
        //
        if (gTotalSwitch && gLeaderSwitch) {
            if (AidTmp == gLeaderAid && gAutoMonsterTeleIdleCounter < 200 && gLeaderIdleCounter > 0) {
                // 必須距離9格內
                if (((abs(*PLAYER_CORRD_X - enemyXFrom) * abs(*PLAYER_CORRD_X - enemyXFrom)) + (abs(*PLAYER_CORRD_Y - enemyYFrom) * abs(*PLAYER_CORRD_Y - enemyYFrom))) <= (gLeaderDist * gLeaderDist)) {
                    gAutoMonsterTeleIdleCounter = gLeaderDelay;
                    if (gLeaderApGive) {
                        //Sleep(1000);
                        SendSkillCastFunction(5368, 5, gLeaderAid);
                    }
                    // 被集結自動放招
                    if (!gConvenioTrigger && gConvenioTotalSwitch) {
                        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ConvenioFunction), NULL, 0, 0);
                    }
                }

            }
        }

    }

    //
    // 07FB - Skill Cast [25 bytes]
    //
    if (packetName == SkillCast) { // 偵測到有人想對你施展特定技能
        int SourceAid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8) +
            (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int TargetAid = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8) +
            (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int TargetSkillId = ((dataBufferDeliver[14]) & 0xFF) + (((dataBufferDeliver[15]) & 0xFF) << 8);

        if (SourceAid != *AidAddress) {
            // 假設是魔凍 必須智能切裝 2209#魔力凍結#
            if (TargetSkillId == 2209) {
                for (int SkillCounter = 0; SkillCounter < 6; SkillCounter++) {
                    if (TargetSkillId == gAutoSmartEquipSkill[SkillCounter]) {
                        gSmartEquipTrigger[SkillCounter] = 1;
                        gSmartEquipTriggerIdleCounter[SkillCounter] = gAutoSmartEquipDelay[SkillCounter];
                    }
                }
            }
        }
    }

    //
    // ['equip_item', 'a2 V v C', [qw(ID type viewID success)]], #11
    //
    if (packetName == EquipItem) { // 偵測到穿裝

        int ItemId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int Type = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
            (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
        int ViewId = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8);
        int Success = ((dataBufferDeliver[10]) & 0xFF);

        if (ItemId && Success >= 0) {
            printf("0999 - equip_item [%d bytes] 穿上裝備:%d. type: %d viewID: %d 成功:%d\n", len, ItemId, Type, ViewId, Success);

            if (Success == 0 && Type) { // 成功
                ItemDataList.update_item_isEquip(ItemId, 1);
            }
            else if (Success == 2) { // 裝備損壞
                //ItemDataList.update_item_isEquip(ItemId, 2);
            }
            else {
                //ItemDataList.update_item_isEquip(ItemId, 0);
                //ItemDataList.update_item_failCount_byId(0, ItemId);
            }
        }
    }


    //
    // '099A' => ['unequip_item', 'a2 V C', [qw(ID type success)]],#9
    //
    if (packetName == UnequipItem) { // 偵測到脫裝

        // 座標
        int ItemId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int Type = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
            (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
        int Success = ((dataBufferDeliver[8]) & 0xFF);
        if (ItemId && Success >= 0) {
            printf("099A - unequip_item [%d bytes]", len);
            printf(" 卸下裝備:%d. type: %d 成功:%d\n", ItemId, Type, Success);

            if (Success == 0 && Type) { // 成功
                ItemDataList.update_item_isEquip(ItemId, 0);
            }
            else {
                ItemDataList.update_item_isEquip(ItemId, 0);
            }
        }
    }

    return EXIT_SUCCESS;
}

BOOL PacketAnalyze(char* dataBufferDeliver, int len) {

    int packetLen = 0;

    // 封包名稱
    int packetName = (dataBufferDeliver[0] & 0xFF) + ((dataBufferDeliver[1] & 0xFF) << 8); // 封包名稱

    // 如果封包名稱剛好等於長度, 代表是CD解密封包 先忽略
    if (packetName == len) {
    
        return 1;
    }

    //printf("未加密封包: %04X\n", packetName);

    //printf("Recv: %04X len: %d\n", packetName, len);
    //int newLineCount = 0;
    //for (int Count = 0; Count < len; Count++) {
    //    printf("%02hhX ", dataBufferDeliver[Count]);
    //    newLineCount++;
    //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
    //}
    //printf("\n");
    // 
    //
    // 抓人物AID
    if (packetName == 0x0283) {
        int Aid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        printf("account_id: %d\n", Aid);

    }

    //
    // 偵測地圖轉換  0AC7 - map_changed [8 bytes]
    // 	'0AC7' => ['map_changed', 'Z16 v2 a4 v a128', [qw(map x y IP port url)]], # 156
    if (packetName == 0x0AC7) {
        printf("map_changed\n");
        //ClearExpReport();
    }

    //
    // 偵測地圖轉換  099B - map_property3
    //
    if (packetName == 0x099B && gGvgDamageSwitch) {
        printf("map_property3\n");

        int newLineCount = 0;
        for (int Count = 0; Count < len; Count++) {
            printf("%02hhX ", dataBufferDeliver[Count]);
            newLineCount++;
            if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        }
        printf("\n");

        int flag = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);

        if (flag == 3) {
            printf("成功改到\n");
            dataBufferDeliver[2] = 1;
            dataBufferDeliver[3] = 0;
            dataBufferDeliver[4] = 0x31;
            dataBufferDeliver[5] = 6;
            dataBufferDeliver[6] = 0;
            dataBufferDeliver[7] = 0;
        }

    }


    //
    // 偵測地圖轉換  0091 - Map Property [8 bytes]
    //0x0ACB
    if (packetName == MapProperty) {
        printf ("Map Property [8 bytes]\n");
        //MapLoadedFlag = 1;
        gAutoTeleIdleCounter = 0;
        gAutoTeleLosTryCountCurrent = 0;
        TargetAidList.clear_target_node();
        TargetAidExpReportList.clear_target_node();
        ItemAidList.clear_target_node();
        PlayerAidList.clear_player_node();
        SkillDelayPlayerList.clear_player_node();

        // 看見傳點瞬移
        gAutoTelePointTrigger = 0;

        // 集結計時器 增加秒數
        gLeaderIdleCounter = 1500;

        // 過傳點暫停使用自瞄
        gStopAutoAttackIdleCounter = gStopAutoAttackIdle;

        if (gNearAttackScreenMonsterSwitch) {
            gNearAttackScreenMonsterTrigger = 1;
        }
        else {
            gNearAttackScreenMonsterTrigger = 1;
        }

        // 瞬移後技能指針歸0
        gCurrentSkillCount = 0;
        gCurrentSkillPvpCount = ElemeMasterSuper;
        gCurrentIgnoranceSkillCount = 0;
    }

    //
    // 技能列表抓取
    // '0B32' => ['skills_list']
    // $skill_info = {
    //    len = > 15,
    //        types = > 'v V v3 C v',
    //        keys = > [qw(ID targetType lv sp range up lv2)],
    // };
    //0x0B32
    if (packetName == SkillList) {
        int packetLen = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8);
        if (packetLen > 4) {
            printf("%04Xd - Skill List [%d byte]\n", packetName, len);
            SkillDataList.clear_skill_node(); // 先清除所有技能資訊
            int PrintLineCount = 0;
 #if !PrivateServerOrNot
            for (int skillCount = 4; skillCount < packetLen; skillCount += 15) {
                // 2E 00 01 00 00 00 07 00 0C 00 13 00 01 07 00
                int Id = ((dataBufferDeliver[skillCount]) & 0xFF) + (((dataBufferDeliver[skillCount + 1]) & 0xFF) << 8);
                int targetType = ((dataBufferDeliver[skillCount + 2]) & 0xFF) + (((dataBufferDeliver[skillCount + 3]) & 0xFF) << 8)
                    + (((dataBufferDeliver[skillCount + 4]) & 0xFF) << 16) + (((dataBufferDeliver[skillCount + 5]) & 0xFF) << 24);
                int Lv = ((dataBufferDeliver[skillCount + 6]) & 0xFF) + (((dataBufferDeliver[skillCount + 7]) & 0xFF) << 8);
                int Sp = ((dataBufferDeliver[skillCount + 8]) & 0xFF) + (((dataBufferDeliver[skillCount + 9]) & 0xFF) << 8);
                int Range = ((dataBufferDeliver[skillCount + 10]) & 0xFF) + (((dataBufferDeliver[skillCount + 11]) & 0xFF) << 8);
                int Upgradable = ((dataBufferDeliver[skillCount + 12]) & 0xFF);
                int Lv2 = ((dataBufferDeliver[skillCount + 13]) & 0xFF) + (((dataBufferDeliver[skillCount + 14]) & 0xFF) << 8);
                if (Id) {
                    //printf("技能: %4d 類型: %2d 等級: %2d 消耗SP: %3d 範圍: %2d 是否可升等: %2d LV2: %2d\n", Id, targetType, Lv, Sp, Range, Upgradable, Lv2);

                    // 新增技能到link list data
                    SkillDataList.delete_skill_node(Id);
                    if (Id && targetType) {
                        SkillDataList.add_skill_node(Id, targetType, Lv, Sp, Range, Upgradable, Lv2);
                    }
                    PrintLineCount++;
                }

            }
#else
            for (int skillCount = 4; skillCount < packetLen; skillCount += 37) {
                // 2E 00 01 00 00 00 07 00 0C 00 13 00 01 07 00
                int Id = ((dataBufferDeliver[skillCount]) & 0xFF) + (((dataBufferDeliver[skillCount + 1]) & 0xFF) << 8);
                int targetType = ((dataBufferDeliver[skillCount + 2]) & 0xFF) + (((dataBufferDeliver[skillCount + 3]) & 0xFF) << 8)
                    + (((dataBufferDeliver[skillCount + 4]) & 0xFF) << 16) + (((dataBufferDeliver[skillCount + 5]) & 0xFF) << 24);
                int Lv = ((dataBufferDeliver[skillCount + 6]) & 0xFF) + (((dataBufferDeliver[skillCount + 7]) & 0xFF) << 8);
                int Sp = ((dataBufferDeliver[skillCount + 8]) & 0xFF) + (((dataBufferDeliver[skillCount + 9]) & 0xFF) << 8);
                int Range = ((dataBufferDeliver[skillCount + 10]) & 0xFF) + (((dataBufferDeliver[skillCount + 11]) & 0xFF) << 8);
                //int Upgradable = ((dataBufferDeliver[skillCount + 12]) & 0xFF);
                //int Lv2 = ((dataBufferDeliver[skillCount + 13]) & 0xFF) + (((dataBufferDeliver[skillCount + 14]) & 0xFF) << 8);
                if (Id) {
                    printf("技能: %4d 類型: %2d 等級: %2d 消耗SP: %3d 範圍: %2d\n", Id, targetType, Lv, Sp, Range);

                    // 新增技能到link list data
                    SkillDataList.delete_skill_node(Id);
                    if (Id && targetType) {
                        SkillDataList.add_skill_node(Id, targetType, Lv, Sp, Range, 0, 0);
                    }
                    PrintLineCount++;
                }

            }
#endif
            // 寫入技能資訊到setting
            //SkillDataList.write_all_skill_data_in_setting();
        }
    }

    //
    // 技能新增
    // '0B31' => ['skill_add', 'v V v3 C v', [qw(skillID target lv sp range upgradable lv2)]], #17
    // Recv: 0B31 len: 17
    // 31 0B 87 00 04 00 00 00 03 00 0F 00 01 00 00 00 00
    // 
    if (packetName == 0x0B31) {

        int Id = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int targetType = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[8]) & 0xFF) << 8)
            + (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
        int Lv = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8);
        int Sp = ((dataBufferDeliver[10]) & 0xFF) + (((dataBufferDeliver[11]) & 0xFF) << 8);
        int Range = ((dataBufferDeliver[12]) & 0xFF) + (((dataBufferDeliver[13]) & 0xFF) << 8);
        int Upgradable = ((dataBufferDeliver[14]) & 0xFF);
        int Lv2 = ((dataBufferDeliver[15]) & 0xFF) + (((dataBufferDeliver[16]) & 0xFF) << 8);
        if (Id) {
            printf("0x0B31 - Skill Add [%d byte]\n", packetName, packetLen);
            printf("新增技能: %4d 類型: %2d 等級: %2d 消耗SP: %3d 範圍: %2d 是否可升等: %2d LV2: %2d\n", Id, targetType, Lv, Sp, Range, Upgradable, Lv2);

            // 新增技能到link list data
            SkillDataList.delete_skill_node(Id);
            if (Id && targetType) {
                SkillDataList.add_skill_node(Id, targetType, Lv, Sp, Range, Upgradable, Lv2);
            }
        }

    }

    //
    // 技能刪除
    // '0441' => ['skill_delete', 'v', [qw(skillID)]],
    // 
    if (packetName == 0x0441) {

        int Id = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        if (Id) {
            printf("0x0441 - Skill Delete [%d byte]\n", packetName, packetLen);
            printf("刪除技能: %4d \n", Id);

            // 刪除技能到link list data
            SkillDataList.delete_skill_node(Id);
        }

    }


    //
    // 表情
    // Recv: 00C0
    // C0 00 39 25 AC 00 02
    // '00C0' = > ['emoticon', 'a4 C', [qw(ID type)]],
    if (packetName == 0x00C0) {
        int Aid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int Type = ((dataBufferDeliver[6]) & 0xFF);
        
        printf("玩家 %d 放表情: %d\n", Aid, Type);

        int IsTrigger = 0;

        if (Aid != *AidAddress) {
            for (int EmoCounter = 0; EmoCounter < 5; EmoCounter++) {
                if (gEmoSwitch[EmoCounter] && gEmoType[EmoCounter] == Type) { // 開關 和 做對表情
                    switch (gEmoMode[EmoCounter]) { // 模式

                    case 1: // 放隊友跟公會
                        if (PlayerAidList.search_player(Aid)) {
                            IsTrigger = 1;
                            break;
                        }
                    case 0: // 只放隊友
                        for (int i = 0; i < gPartyInfoLen; i++) {
                            if (gPartyInfoOnline[i] && gPartyInfoAid[i] == Aid) { // 找到組隊隊友
                                IsTrigger = 1;
                                break;
                            }
                        }

                        break;

                    case 2: // 只放好友名單
                        if (IsSpecialAid2(Aid)) {
                            IsTrigger = 1;
                            break;
                        }
                        break;

                    default:
                        break;
                    }

                    if (IsTrigger) {
                        int SkillLvCatch = SkillDataList.IsSkillIdCanUse(gEmoSkill[EmoCounter]);
                        int SkillRange = SkillDataList.SearchSkillRange(gEmoSkill[EmoCounter]);
                        if (SkillLvCatch && SkillRange) {
                            int* CorrdX = new int;
                            int* CorrdY = new int;
                            int DIST = 0;
                            if (TargetAidExpReportList.search_target_Aid(Aid, CorrdX, CorrdY)) {
                                DIST = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                            }

                            if (DIST <= (SkillRange * SkillRange)) {
                                if (gEmoIsSelf[EmoCounter]) {
                                    SendSkillCastFunction(gEmoSkill[EmoCounter], SkillLvCatch, *AidAddress);
                                }
                                else {
                                    SendSkillCastFunction(gEmoSkill[EmoCounter], SkillLvCatch, Aid);
                                }
                            }
                        }
                    }
                }
            }
        }

    }

    //
    // 組隊訊息處理
    //
    if (packetName == PartyJoin) {
// '0A43' = > ['party_join', 'a4 V v4 C Z24 Z24 Z16 C2', [qw(ID role jobID lv x y type name user map item_pickup item_share)]],
        int Aid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int Role = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8)
            + (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int JobId = ((dataBufferDeliver[10]) & 0xFF) + (((dataBufferDeliver[11]) & 0xFF) << 8);
        int Lv = ((dataBufferDeliver[12]) & 0xFF) + (((dataBufferDeliver[13]) & 0xFF) << 8);
        int X = ((dataBufferDeliver[14]) & 0xFF) + (((dataBufferDeliver[15]) & 0xFF) << 8);
        int Y = ((dataBufferDeliver[16]) & 0xFF) + (((dataBufferDeliver[17]) & 0xFF) << 8);
        int Type = ((dataBufferDeliver[18]) & 0xFF) + (((dataBufferDeliver[19]) & 0xFF) << 8)
            + (((dataBufferDeliver[20]) & 0xFF) << 16) + (((dataBufferDeliver[21]) & 0xFF) << 24);

        //
        // 抓取角色ID
        // 
        char CharNameStr[CharNameLen] = { "" };
        int CharLen = 24;
        if (CharLen) {
            for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                CharNameStr[CharCount] = dataBufferDeliver[43 + CharCount];
            }
            //printf(" %s ", CharNameStr);
        }

        char MapNameStr[16] = { "" };
        int MapLen = 16;
        if (MapLen) {
            for (int MapCount = 0; MapCount < MapLen && 67 + MapCount < len; MapCount++) {
                
                MapNameStr[MapCount] = dataBufferDeliver[67 + MapCount];
            }
        }

        printf("Party Join, %d Role: %d, JobId: %d, Lv: %d, (%d, %d) %s %s\n", Aid, Role, JobId, Lv, X, Y, CharNameStr, MapNameStr);

        int FindInParty = 0;
        for (int i = 0; i < 12; i++) {
            if (gPartyInfoAid[i] == Aid) {
                FindInParty = 1;
                gPartyInfoJob[i] = JobId;
                gPartyInfoOnline[i] = 1;
                gPartyInfoX[i] = X;
                gPartyInfoY[i] = Y;
                strcpy(gPartyInfoMap[i], MapNameStr);

            }
        }
        if (!FindInParty && gPartyInfoLen < 12) {
            gPartyInfoAid[gPartyInfoLen] = Aid;
            gPartyInfoJob[gPartyInfoLen] = JobId;
            gPartyInfoOnline[gPartyInfoLen] = 1;
            gPartyInfoX[gPartyInfoLen] = X;
            gPartyInfoY[gPartyInfoLen] = Y;
            strcpy(gPartyInfoMap[gPartyInfoLen], MapNameStr);
            gPartyInfoLen++;
        }

        //int newLineCount = 0;
        //printf("%04X - Party Join [%d byte]\n", PartyJoin, len);
        //for (int Count = 0; Count < len; Count++) {
        //    printf("%02hhX ", dataBufferDeliver[Count]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

        if (Role == 0) { // 隊長
            gLeaderAid = Aid;
        }

    }

    //
    // 組隊訊息處理
    //
    if (packetName == 0x0107) {
        // '0107' => ['party_location', 'a4 v2', [qw(ID x y)]],
        int Aid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int X = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8);
        int Y = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8);

        //printf("Party Location %d (%d, %d)\n", Aid, X, Y);

        int FindInParty = 0;
        for (int i = 0; i < 12; i++) {
            if (gPartyInfoAid[i] == Aid) {
                gPartyInfoX[i] = X;
                gPartyInfoY[i] = Y;
            }
        }

        //int newLineCount = 0;
        //printf("%04X - Party Join [%d byte]\n", PartyJoin, len);
        //for (int Count = 0; Count < len; Count++) {
        //    printf("%02hhX ", dataBufferDeliver[Count]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

    }

    if (packetName == PartyLeader) {
//'07FC' = > ['party_leader', 'V2', [qw(old new)]], 
        int AidOld = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int AidNew = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8)
            + (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);

        printf("PartyLeader, %d %d \n", AidOld, AidNew);

        int newLineCount = 0;
        //printf("%04X - PartyLeader [%d byte]\n", PartyLeader, len);
        //for (int Count = 0; Count < len; Count++) {
        //    printf("%02hhX ", dataBufferDeliver[Count]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

        gLeaderAid = AidNew;

    }

    if (packetName == PartyLeave) {
// '0105' => ['party_leave', 'a4 Z24 C', [qw(ID name result)]],
        int Aid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);

        //
        // 抓取角色ID
        // 
        char CharNameStr[CharNameLen] = { "" };
        int CharLen = 24;
        if (CharLen) {
            for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                CharNameStr[CharCount] = dataBufferDeliver[6 + CharCount];
            }
            //printf(" %s ", CharNameStr);
        }

        printf("PartyLeave, %d %s \n", Aid, CharNameStr);

        int newLineCount = 0;
        printf("%04X - PartyLeave [%d byte]\n", PartyLeave, len);
        for (int Count = 0; Count < len; Count++) {
            printf("%02hhX ", dataBufferDeliver[Count]);
            newLineCount++;
            if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        }
        printf("\n");

        for (int i = 0; i < 12; i++) {
            // 如果是自己退組 清除所有組隊資訊
            if (Aid == *AidAddress) { // 清除所有組隊資訊
                for (int j = 0; j < 12; j++) {
                    gPartyInfoAid[j] = 0;
                    gPartyInfoJob[j] = 0;
                    gPartyInfoOnline[j] = 0;
                    gPartyInfoLen = 0;
                    gPartyInfoX[j] = 0;
                    gPartyInfoY[j] = 0;
                    strcpy(gPartyInfoMap[j], "");
                }
                break;
            }
            else if (Aid == gPartyInfoAid[i]) {
                gPartyInfoAid[i] = 0;
                gPartyInfoJob[i] = 0;
                gPartyInfoOnline[i] = 0;
                gPartyInfoX[i] = 0;
                gPartyInfoY[i] = 0;
                strcpy(gPartyInfoMap[i], "");
            }
        }

    }


    if (packetName == PartyUsersInfo && len > 50) {
//'00FB' => ['party_users_info', 'v Z24 a*', [qw(len party_name playerInfo)]],
        //
        // 清空組隊資料
        // 
        for (int i = 0; i < 12; i++) {
            gPartyInfoAid[i] = 0;
            gPartyInfoJob[i] = 0;
            gPartyInfoOnline[i] = 0;
            gPartyInfoLen = 0;
            gPartyInfoX[i] = 0;
            gPartyInfoY[i] = 0;
            strcpy(gPartyInfoMap[i], "");
        }
        //
        // 抓取角色ID
        // 
        char CharNameStr[CharNameLen] = { "" };
        int CharLen = 24;
        if (CharLen) {
            for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                CharNameStr[CharCount] = dataBufferDeliver[4 + CharCount];
            }
        }
        int newLineCount = 0;
        printf("%04X - PartyUsersInfo [%d byte]\n", PartyUsersInfo, len);
        //for (int Count = 0; Count < len; Count++) {
        //    printf("%02hhX ", dataBufferDeliver[Count]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

#if !PrivateServerOrNot
        printf("PartyUsersInfo, %s \n", CharNameStr);
        gPartyInfoLen = (len - 28) / 50; // 隊伍人數數量
        for (int PartyInfoCount = 0; PartyInfoCount < gPartyInfoLen; PartyInfoCount++) {
            int InfoCount = 28 + (PartyInfoCount * 50); // 計算基準點

            int Aid = ((dataBufferDeliver[InfoCount]) & 0xFF) + (((dataBufferDeliver[InfoCount + 1]) & 0xFF) << 8)
                + (((dataBufferDeliver[InfoCount + 2]) & 0xFF) << 16) + (((dataBufferDeliver[InfoCount + 3]) & 0xFF) << 24);
            int JobId = ((dataBufferDeliver[InfoCount + 46]) & 0xFF) + (((dataBufferDeliver[InfoCount + 47]) & 0xFF) << 8);
            int Lv = ((dataBufferDeliver[InfoCount + 48]) & 0xFF) + (((dataBufferDeliver[InfoCount + 49]) & 0xFF) << 8);

            int Admin = ((dataBufferDeliver[InfoCount + 44]) & 0xFF);
            int Online = ((dataBufferDeliver[InfoCount + 45]) & 0xFF); // 是否在線上, 0 = 在線上

            char CharNameStr[CharNameLen] = { "" };
            int CharLen = 24;
            if (CharLen) {
                for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                    CharNameStr[CharCount] = dataBufferDeliver[InfoCount + 4 + CharCount];
                }
            }

            char MapNameStr[16] = { "" };
            int MapLen = 16;
            if (MapLen) {
                for (int MapCount = 0; MapCount < MapLen && InfoCount + 28 + MapCount < len; MapCount++) {
                    if (!dataBufferDeliver[InfoCount + 28 + MapCount] && !dataBufferDeliver[InfoCount + 28 + MapCount + 1]) {
                        break;
                    }
                    MapNameStr[MapCount] = dataBufferDeliver[InfoCount + 28 + MapCount];
                }
            }

            printf("%d: %d %s Job: %d Lv: %d %s Admin: %d, 是否在線上: %d\n", PartyInfoCount, Aid, CharNameStr, JobId, Lv, MapNameStr, Admin, Online);

            if (PartyInfoCount >= 12) {
                break;
            }

            gPartyInfoAid[PartyInfoCount] = Aid;
            gPartyInfoJob[PartyInfoCount] = JobId;
            strcpy(gPartyInfoMap[PartyInfoCount], MapNameStr);
            if (Online) {
                gPartyInfoOnline[PartyInfoCount] = 0;
            }
            else {
                gPartyInfoOnline[PartyInfoCount] = 1;
            }

            if (Admin == 0) {
                gLeaderAid = Aid;
            }

        }
#else
        printf("PartyUsersInfo, %s \n", CharNameStr);
        gPartyInfoLen = (len - 28) / 54; // 隊伍人數數量
        for (int PartyInfoCount = 0; PartyInfoCount < gPartyInfoLen; PartyInfoCount++) {
            int InfoCount = 28 + (PartyInfoCount * 54); // 計算基準點

            int Aid = ((dataBufferDeliver[InfoCount]) & 0xFF) + (((dataBufferDeliver[InfoCount + 1]) & 0xFF) << 8)
                + (((dataBufferDeliver[InfoCount + 2]) & 0xFF) << 16) + (((dataBufferDeliver[InfoCount + 3]) & 0xFF) << 24);
            int JobId = ((dataBufferDeliver[InfoCount + 50]) & 0xFF) + (((dataBufferDeliver[InfoCount + 51]) & 0xFF) << 8);
            int Lv = ((dataBufferDeliver[InfoCount + 52]) & 0xFF) + (((dataBufferDeliver[InfoCount + 53]) & 0xFF) << 8);

            int Admin = ((dataBufferDeliver[InfoCount + 48]) & 0xFF);
            int Online = ((dataBufferDeliver[InfoCount + 49]) & 0xFF); // 是否在線上, 0 = 在線上

            char CharNameStr[CharNameLen] = { "" };
            int CharLen = 24;
            if (CharLen) {
                for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                    CharNameStr[CharCount] = dataBufferDeliver[InfoCount + 8 + CharCount];
                }
            }

            char MapNameStr[16] = { "" };
            int MapLen = 16;
            if (MapLen) {
                for (int MapCount = 0; MapCount < MapLen && InfoCount + 32 + MapCount < len; MapCount++) {
                    if (!dataBufferDeliver[InfoCount + 32 + MapCount] && !dataBufferDeliver[InfoCount + 32 + MapCount + 1]) {
                        break;
                    }
                    MapNameStr[MapCount] = dataBufferDeliver[InfoCount + 32 + MapCount];
                }
            }

            printf("%d: %d %s Job: %d Lv: %d %s Admin: %d, 是否在線上: %d\n", PartyInfoCount, Aid, CharNameStr, JobId, Lv, MapNameStr, Admin, Online);

            gPartyInfoAid[PartyInfoCount] = Aid;
            gPartyInfoJob[PartyInfoCount] = JobId;
            strcpy(gPartyInfoMap[PartyInfoCount], MapNameStr);
            if (Online) {
                gPartyInfoOnline[PartyInfoCount] = 0;
            }
            else {
                gPartyInfoOnline[PartyInfoCount] = 1;
            }

            if (Admin == 0) {
                gLeaderAid = Aid;
            }

        }
#endif
    }
    
    if (packetName == GuildAlliesEnemyList) {
//'014C' = > ['guild_allies_enemy_list'], #  - 1

        printf("GuildAlliesEnemyList\n");

        // 清空抓取同盟資料
        for (int i = 0; i < 6; i++) {
            gGuildIdCatch[i] = 0;
        }

        int GuildInfoLen = (len - 4) / 32; // 總列表數量
        for (int GuildInfoCount = 0; GuildInfoCount < GuildInfoLen; GuildInfoCount++) {
            int InfoCount = 4 + (GuildInfoCount * 32); // 計算基準點

            int Enemy = ((dataBufferDeliver[InfoCount]) & 0xFF) + (((dataBufferDeliver[InfoCount + 1]) & 0xFF) << 8)
                + (((dataBufferDeliver[InfoCount + 2]) & 0xFF) << 16) + (((dataBufferDeliver[InfoCount + 3]) & 0xFF) << 24);
            int GuildId = ((dataBufferDeliver[InfoCount + 4]) & 0xFF) + (((dataBufferDeliver[InfoCount + 5]) & 0xFF) << 8)
                + (((dataBufferDeliver[InfoCount + 6]) & 0xFF) << 16) + (((dataBufferDeliver[InfoCount + 7]) & 0xFF) << 24);

            char CharNameStr[CharNameLen] = { "" };
            int CharLen = 24;
            if (CharLen) {
                for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                    CharNameStr[CharCount] = dataBufferDeliver[InfoCount + 8 + CharCount];
                }
            }

            printf("%d: %d %d %s\n", GuildInfoCount, Enemy, GuildId, CharNameStr);

            if (!Enemy) { // 同盟
                gGuildIdCatch[GuildInfoCount] = GuildId;
            }
        }

    }
    // 
    // Item List Start
    //
    if (packetName == ItemListStart) {
        packetLen = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8);
        int StorageType = (dataBufferDeliver[4] & 0xFF);
        printf("0B08 - Item List Start [%d byte], type: %d\n", packetLen, StorageType);
#if !PrivateServerOrNot
        //
        // packetLen為五代表示掃身上物品
        //
        if (StorageType == 0) {
            itemAddedStart = 1;
            ItemDataList.clear_item_node();
            //ofstream fout(".\\data\\debug.txt", ios::app);   // Open the file
            //fout << "清!" << endl;
            //fout.close();
        }
        else if (StorageType == 2) {
            itemAddedStart = 2; // Storage
            StorageDataList.clear_item_node();
        }
        else {
            itemAddedStart = 0;
        }
        //if (itemAddedStart) {
        //    printf("0B08 - Item List Start [%d byte]\n", packetLen);
        //}
#else
        if (packetLen == ItemListStartLen && !dataBufferDeliver[4]) { // 私服6
            itemAddedStart = 1;
            ItemDataList.clear_item_node();
            printf("0B08 - Item List Start [%d byte]\n", packetLen);

            for (int itemCount = 0; itemCount < packetLen; itemCount++) {
                printf("%02hhX ", dataBufferDeliver[itemCount]);
            }
            printf("\n");
                    
        }
        else {
            itemAddedStart = 0;
        }
#endif
    }

    //
    // Item List Stackable
    //
    if (packetName == ItemListStackable && itemAddedStart) {
        int packetLen = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8);
        if (packetLen > 4) {
            printf("0B09 - Item List Stackable [%d byte]\n", packetLen);
                    
            int PrintLineCount = 0;
            for (int itemCount = 4; itemCount < packetLen - 1; itemCount += 34) {
                int IdTmp = ((dataBufferDeliver[itemCount + 1]) & 0xFF) + (((dataBufferDeliver[itemCount + 2]) & 0xFF) << 8);
                int ItemIdTmp = ((dataBufferDeliver[itemCount + 3]) & 0xFF) + (((dataBufferDeliver[itemCount + 4]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 5]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 6]) & 0xFF) << 24);
                int ItemAmountTmp = ((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8);
                //if (!ItemDataList.search_item_Id(IdTmp) && ItemIdTmp && IdTmp) {
                if (ItemAmountTmp >= 0 && ItemIdTmp && IdTmp) {
                    if (itemAddedStart == 1) { // 身上物品類
                        printf("%d: 0B09 - Item List Stackable %d 數量: %d\n", IdTmp, ItemIdTmp, ItemAmountTmp);
                        ItemDataList.add_item_node(
                            0, //((dataBufferDeliver[itemCount]) & 0xFF),
                            IdTmp,
                            ItemIdTmp,
                            ItemAmountTmp,
                            0, 0, 0, 0, 0, 0, 0
                        );
                    }
                    else if (itemAddedStart == 2) { // 倉庫類
                        printf("%d: 0B09 - Item List Stackable (倉庫) %d 數量: %d\n", IdTmp, ItemIdTmp, ItemAmountTmp);
                        if (gAutoStorageSwitch || gHotKeyStorageSwitch) {
                            StorageDataList.add_item_node(
                                0, //((dataBufferDeliver[itemCount]) & 0xFF),
                                IdTmp,
                                ItemIdTmp,
                                ItemAmountTmp,
                                0, 0, 0, 0, 0, 0, 0
                            );
                        }
                    }
                    PrintLineCount++;
                }
                //printf("%02hhX ", dataBufferDeliver[itemCount]);
                //if (itemCount % 16 == 0 && itemCount != 0) printf("\n");
            }
        }
    }
    //
    // 0B0A - Item List Non-Stackable (Equips)
    //
    if (packetName == ItemListNonStackable && itemAddedStart) {
        int packetLen = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8);
        if (packetLen > 4) {
            printf("0B39 - Item List Non-Stackable (Equips) [%d byte]\n", packetLen);

            int PrintLineCount = 0;   

            //for (int itemCount = 4; itemCount < packetLen - 1; itemCount += 67) {
            //    int IdTmp = ((dataBufferDeliver[itemCount + 1]) & 0xFF) + (((dataBufferDeliver[itemCount + 2]) & 0xFF) << 8);
            //    int ItemIdTmp = ((dataBufferDeliver[itemCount + 3]) & 0xFF) + (((dataBufferDeliver[itemCount + 4]) & 0xFF) << 8)
            //        + (((dataBufferDeliver[itemCount + 5]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 6]) & 0xFF) << 24);
            //    int EquipType = ((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8)
            //        + (((dataBufferDeliver[itemCount + 10]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 11]) & 0xFF) << 24);
            //    int IsEquipType = ((dataBufferDeliver[itemCount + 12]) & 0xFF) + (((dataBufferDeliver[itemCount + 13]) & 0xFF) << 8)
            //        + (((dataBufferDeliver[itemCount + 14]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 15]) & 0xFF) << 24);
            //    int Refined = ((dataBufferDeliver[itemCount + 16]) & 0xFF);
            //    int Card01 = ((dataBufferDeliver[itemCount + 17]) & 0xFF) + (((dataBufferDeliver[itemCount + 18]) & 0xFF) << 8)
            //        + (((dataBufferDeliver[itemCount + 19]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 20]) & 0xFF) << 24);
            //    int Card02 = ((dataBufferDeliver[itemCount + 21]) & 0xFF) + (((dataBufferDeliver[itemCount + 22]) & 0xFF) << 8)
            //        + (((dataBufferDeliver[itemCount + 23]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 24]) & 0xFF) << 24);
            //    int Card03 = ((dataBufferDeliver[itemCount + 25]) & 0xFF) + (((dataBufferDeliver[itemCount + 26]) & 0xFF) << 8)
            //        + (((dataBufferDeliver[itemCount + 27]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 28]) & 0xFF) << 24);
            //    int Card04 = ((dataBufferDeliver[itemCount + 29]) & 0xFF) + (((dataBufferDeliver[itemCount + 30]) & 0xFF) << 8)
            //        + (((dataBufferDeliver[itemCount + 31]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 32]) & 0xFF) << 24);

            for (int itemCount = 4; itemCount < packetLen - 1; itemCount += 68) {
                int IdTmp = ((dataBufferDeliver[itemCount + 1]) & 0xFF) + (((dataBufferDeliver[itemCount + 2]) & 0xFF) << 8);
                int ItemIdTmp = ((dataBufferDeliver[itemCount + 3]) & 0xFF) + (((dataBufferDeliver[itemCount + 4]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 5]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 6]) & 0xFF) << 24);
                int EquipType = ((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 10]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 11]) & 0xFF) << 24);
                int IsEquipType = ((dataBufferDeliver[itemCount + 12]) & 0xFF) + (((dataBufferDeliver[itemCount + 13]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 14]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 15]) & 0xFF) << 24);
                int Refined = ((dataBufferDeliver[itemCount + 66]) & 0xFF);
                int Card01 = ((dataBufferDeliver[itemCount + 16]) & 0xFF) + (((dataBufferDeliver[itemCount + 17]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 18]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 19]) & 0xFF) << 24);
                int Card02 = ((dataBufferDeliver[itemCount + 20]) & 0xFF) + (((dataBufferDeliver[itemCount + 21]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 22]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 23]) & 0xFF) << 24);
                int Card03 = ((dataBufferDeliver[itemCount + 24]) & 0xFF) + (((dataBufferDeliver[itemCount + 25]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 26]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 27]) & 0xFF) << 24);
                int Card04 = ((dataBufferDeliver[itemCount + 28]) & 0xFF) + (((dataBufferDeliver[itemCount + 29]) & 0xFF) << 8)
                    + (((dataBufferDeliver[itemCount + 30]) & 0xFF) << 16) + (((dataBufferDeliver[itemCount + 31]) & 0xFF) << 24);

                if (itemAddedStart == 1 && ItemIdTmp && IdTmp) { // 身上物品類
                    printf("%d: 0B39 - Item List Non-Stackable (Equips) 部位:%d (%d) +%d %d [%d, %d, %d, %d]\n", PrintLineCount, EquipType, IdTmp, Refined, ItemIdTmp, Card01, Card02, Card03, Card04);

                    int newLineCount = 0;
                    for (int printCount = itemCount; printCount < itemCount + 68; printCount++) {
                        printf("%02hhX ", dataBufferDeliver[printCount]);
                        newLineCount++;
                        if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
                    }
                    printf("\n");

                    if (IsEquipType) {
                        ItemDataList.add_item_node(
                            1, //((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8),
                            IdTmp,
                            ItemIdTmp,
                            1, //((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8)
                            Refined,
                            1,
                            EquipType,
                            Card01,
                            Card02,
                            Card03,
                            Card04
                        );
                    }
                    else {
                        ItemDataList.add_item_node(
                            1, //((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8),
                            IdTmp,
                            ItemIdTmp,
                            1, //((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8)
                            Refined,
                            0,
                            EquipType,
                            Card01,
                            Card02,
                            Card03,
                            Card04
                        );
                    }

                }
                else if (itemAddedStart == 2 && ItemIdTmp && IdTmp) { // 倉庫類
                    printf("%d: 0B39 - Item List Non-Stackable (Equips) 部位:%d (%d) (倉庫) +%d %d [%d, %d, %d, %d]\n", PrintLineCount, EquipType, IdTmp, Refined, ItemIdTmp, Card01, Card02, Card03, Card04);
                    if (gAutoStorageSwitch || gHotKeyStorageSwitch) {
                        StorageDataList.add_item_node(
                            1, //((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8),
                            IdTmp,
                            ItemIdTmp,
                            1, //((dataBufferDeliver[itemCount + 8]) & 0xFF) + (((dataBufferDeliver[itemCount + 9]) & 0xFF) << 8)
                            Refined,
                            0,
                            EquipType,
                            Card01,
                            Card02,
                            Card03,
                            Card04
                        );
                    }
                }


                PrintLineCount++;
                //printf("\n");
                //int newLineCount = 0;
                //for (int printCount = itemCount; printCount < itemCount + 67; printCount++) {
                //    printf("%02hhX ", dataBufferDeliver[printCount]);
                //    newLineCount++;
                //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
                //}
                //printf("\n");
            }

            //// WA 還解決不了
            //if (itemAddedStart == 2) {
            //    printf("0B0B - Item List End [4 bytes] (倉庫)\n");
            //    gStorageSuccess = 1;
            //    itemAddedStart = 0;
            //}
        }
    }
    //
    // 0B0B - Item List End
    //
    if (packetName == ItemListEnd && itemAddedStart) {
        //printf("0B0B - Item List End [4 bytes]\n");
        int packetLen = 4;

        if (PacketMatchOrNot(dataBufferDeliver, 2, ItemListEndSize)) {
            printf("0B0B - Item List End [4 bytes]\n");

            // 寫入背包裝備到設定
            //ItemDataList.write_all_equip_data_in_setting();

            if (itemAddedStart == 2) {
                gStorageSuccess = 1;
            }
            itemAddedStart = 0;
        }
        else {
            printf("0B0B - Item List End [4 bytes] (倉庫)\n");

            // 寫入背包裝備到設定
            //ItemDataList.write_all_equip_data_in_setting();

            if (itemAddedStart == 2) {
                gStorageSuccess = 1;
            }
            itemAddedStart = 0;
        }
    }

    //
    // 01D0 (Monk spirits) [8 bytes] 
    //
    if (packetName == MonkSpirits) { // Monk spirits
        packetLen = 8;
        //
        // 判斷是你得失修羅氣彈
        //
        int AidTmp = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int SpiritsValue = (dataBufferDeliver[6]) & 0xFF + (((dataBufferDeliver[7]) & 0xFF) << 8);

        if (AidTmp >= 0 && AidTmp == *AidAddress) {
            gCharSpirits = SpiritsValue;
            //printf("你的氣彈數為: %d\n", gCharSpirits);
        }
    }

    //
    // '00B0' => ['stat_info', 'v V', [qw(type val)]], # 8 
    //
    if (packetName == StatInfo) { // stat_info
        packetLen = 8;
        //
        // 狀態列表
        //
        int Type = (dataBufferDeliver[2]) & 0xFF + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int Value = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8)
            + (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);

        //printf("%04X - stat_info type: 0x%02X, value: 0x%04X\n", packetName, Type, Value);

        if (Type == 0xE8) { // AP量
            gCurrentApValue = Value;
            //printf("目前AP: %d\n", gCurrentApValue);
        }

        //else {
        //    printf("00B0- type:%d, value: %d\n", Type, Value);
        //}
    }

    //
    // '00B0' => '0ACC' => ['exp', 'a4 V2 v2', [qw(ID val val2 type flag)]], 18#
    //
    if (packetName == 0x0ACC) { // exp

        int Aid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int Value1 = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8)
            + (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int Value2 = ((dataBufferDeliver[10]) & 0xFF) + (((dataBufferDeliver[11]) & 0xFF) << 8)
            + (((dataBufferDeliver[12]) & 0xFF) << 16) + (((dataBufferDeliver[13]) & 0xFF) << 24);
        int Type = (dataBufferDeliver[14]) & 0xFF + (((dataBufferDeliver[15]) & 0xFF) << 8);
        int Flag = (dataBufferDeliver[16]) & 0xFF + (((dataBufferDeliver[17]) & 0xFF) << 8);


        if (Type == 0x1 && gExpReportSwitch && Value1 && Aid == *AidAddress) { // 獲取經驗值
            gExpTotalObtainBaseValue += Value1;
            //printf("獲取經驗值: %d\n", Value1);
        }

        if (Type == 0x2 && gExpReportSwitch && Value1 && Aid == *AidAddress) { // 獲取職業經驗值
            gExpTotalObtainJobValue += Value1;
            //printf("獲取職業經驗值: %d\n", Value1);
        }

    }

    //
    // 0080 - Actor Lost (Died, Disappeared, Disconnected) [7 bytes] 
    //
    if (packetName == ActorLost) { // Actor Lost
        packetLen = 7;

        ////
        //// 判斷是你死亡
        ////
        //if (IsYourAidOrNot(dataBufferDeliver, 2) && (dataBufferDeliver[6] & 0xFF) == 1) {
        //    printf("0080 - Actor Lost (Died, Disappeared, Disconnected) [7 bytes]\n");
        //    MapChangeFlag = 1;
        //}
        int AidTmp = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int Type = (dataBufferDeliver[6]) & 0xFF;
        //# type:
        //#     0 = out of sight
        //#     1 = died
        //#     2 = logged out
        //#     3 = teleport
        //#     4 = trickdead

        if (AidTmp >= 0 && Type == 1) {
            //printf("0080 - Actor Lost (Died, Disappeared, Disconnected) [7 bytes] %d (%d) Died\n", AidTmp, Type);
            //printf("目標 %d 已死亡 (%d)\n", AidTmp, Type);
#if !PrivateServerOrNot
            // 判斷是我們死亡
            if (AidTmp == *AidAddress && gDieNoteAid) {
                CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(DieNoteFunction), NULL, 0, 0);
            }
#endif
            // 魔物報告
            if (gExpReportSwitch) {
                int MonsterId = TargetAidExpReportList.search_target_you_hit(AidTmp, 5);
                if (MonsterId) {
                    if (!ItemReportList.search_item(MonsterId)) { // 如果魔物沒有在報告裡
                        ItemReportList.add_item_node(
                            99, // 魔物type
                            1,
                            MonsterId,
                            1,
                            0, 0, 0, 0, 0, 0, 0
                        );
                    }
                    else {
                        int MonsterAmountOrg = ItemReportList.search_itemAmount(MonsterId);
                        MonsterAmountOrg++;
                        ItemReportList.update_item_amount(MonsterId, MonsterAmountOrg);
                    }
                
                }
            }

        }

        //
        // 更新target列表
        //
        if (TargetAidList.search_target(AidTmp)) {
            TargetAidList.delete_target_node(AidTmp);
        }
        if (TargetAidExpReportList.search_target(AidTmp)) {
            TargetAidExpReportList.delete_target_node(AidTmp);
        }
    }

    //
    // '09CB' => ['skill_used_no_damage', 'v V a4 a4 C', [qw(skillID amount targetID sourceID success)]],
    //  Recv: 09CB len: 17
    if (packetName == SkillUsedNoDamage) { // 偵測技能使用失敗訊息

        int SkillId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int SkillLv = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
            (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
        int TargetAid = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8) +
            (((dataBufferDeliver[10]) & 0xFF) << 16) + (((dataBufferDeliver[11]) & 0xFF) << 24);
        int SourceAid = ((dataBufferDeliver[12]) & 0xFF) + (((dataBufferDeliver[13]) & 0xFF) << 8) +
            (((dataBufferDeliver[14]) & 0xFF) << 16) + (((dataBufferDeliver[15]) & 0xFF) << 24);
        int Success = ((dataBufferDeliver[16]) & 0xFF);

        if (SkillId) {
            //printf("%d 對 %d 使用技能 %d 等級 %d, Success: %d\n", SourceAid, TargetAid, SkillId, SkillLv, Success);
            if (gSkillIdAck == SkillId && SourceAid == *AidAddress && Success == 1) {
                printf("01: 技能%d ACK \n", gSkillIdAck);
                gSkillAck = 1;
            }

            // 魔凍成功就趕快切回來 2209#魔力凍結#
            if (SkillId == 2209) {
                for (int SkillCounter = 0; SkillCounter < 6; SkillCounter++) {
                    if (SkillId == gAutoSmartEquipSkill[SkillCounter]) {
                        gSmartEquipTrigger[SkillCounter] = 0;
                        gSmartEquipTriggerIdleCounter[SkillCounter] = 0;
                    }
                }
            }

            // 無知衰弱技能接替
            if (gIgnoranceSuperSwitch && gNearSkillPvpStopSwitch) {
                if ((SkillId == 2294 || SkillId == 2297) && Success == 1 && TargetAid != *AidAddress) {
                    SkillDelayPlayerList.delete_player_node_Id(TargetAid, SkillId);
                    SkillDelayPlayerList.add_player_node_2(TargetAid, gNearSkillPvpStopTime, SkillId);
                    printf("新增無視秒數 %d, skill: %d\n", TargetAid, SkillId);
                }
            }
            else if (SkillId == gNearAttackSkill && Success == 1 && gNearSkillPvpStopSwitch && !gIgnoranceSuperSwitch && TargetAid != *AidAddress) {
                SkillDelayPlayerList.delete_player_node_Id(TargetAid, SkillId);
                SkillDelayPlayerList.add_player_node_2(TargetAid, gNearSkillPvpStopTime, SkillId);
                printf("新增無視秒數 %d, skill: %d\n", TargetAid, SkillId);
            }
            if (SourceAid == *AidAddress && gIgnoranceSuperSwitch) {
                if (SkillId == 2294) { // 2294#面具-無知#
                    gCurrentIgnoranceSkillCount = 1;
                }
                else if (SkillId == 2297) { // 2297#面具-衰弱#  
                    gCurrentIgnoranceSkillCount = 0;
                }
            }
        }

    }

    //
    // '0110' => ['skill_use_failed', 'v V C2', [qw(skillID btype fail type)]],
    //
    if (packetName == SkillUseFailed) { // 偵測技能使用失敗訊息

        int SkillId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int Btype = ((dataBufferDeliver[4]) & 0xFF);
        int Type = ((dataBufferDeliver[13]) & 0xFF);
        
        //0 = > T('Basic'),
        //1 = > T('Insufficient SP'),
        //2 = > T('Insufficient HP'),
        //3 = > T('No Memo'),
        //4 = > T('Mid-Delay'),
        //5 = > T('No Zeny'),
        //6 = > T('Wrong Weapon Type'),
        //7 = > T('Red Gem Needed'),
        //8 = > T('Blue Gem Needed'),
        //9 = > TF('%s Overweight', '90%'),
        //10 = > T('Requirement'),
        //11 = > T('Failed to use in Target'),
        //12 = > T('Maximum Ancilla exceed'),
        //13 = > T('Need this within the Holy water'),
        //14 = > T('Missing Ancilla'),
        //19 = > T('Full Amulet'),
        //24 = > T('[Purchase Street Stall License] need 1'),
        //29 = > TF('Must have at least %s of base XP', '1%'),
        //30 = > T('Insufficient SP'),
        //33 = > T('Failed to use Madogear'),
        //34 = > T('Kunai is Required'),
        //37 = > T('Canon ball is Required'),
        //43 = > T('Failed to use Guillotine Poison'),
        //50 = > T('Failed to use Madogear'),
        //71 = > T('Missing Required Item'), # (item name) required x amount
        //72 = > T('Equipment is required'),
        //73 = > T('Combo Skill Failed'),
        //76 = > T('Too many HP'),
        //77 = > T('Need Royal Guard Branding'),
        //78 = > T('Required Equiped Weapon Class'),
        //83 = > T('Location not allowed to create chatroom/market'),
        //84 = > T('Need more bullet'),


        if (Btype == 0) {
            printf("%04X: 你使用技能失敗: %d fail: %d\n", SkillUseFailed, SkillId, Type);
            if (Type == 4) {
                // 延遲問題造成失敗 回傳Ack
                if (gNearAttackSkill == SkillId) {
                    gNearAttackAck = 1;
                    gNearAttackAckTotal = 0;
                }
            }
        }

        //0110: 你使用技能失敗: 2515 fail: 1024
        //10 01 D3 09 00 00 00 00 A8 F2 0A 05 00 04
        //

        //int newLineCount = 0;
        //for (int Count = 0; Count < len; Count++) {
        //    printf("%02hhX ", dataBufferDeliver[Count]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

    }

    //
    // 01DE - Skill Damage [33 bytes]
    //
    //	'01DE' => ['skill_use', 'v a4 a4 V4 v2 C', [qw(skillID sourceID targetID tick src_speed dst_speed damage level option type)]],
    if (packetName == SkillDamage) { // 偵測到有人想對你施展特定範圍技能
        packetLen = 33;
        int TargetSkillId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int SourceAid = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
            (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
        int TargetAid = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8) +
            (((dataBufferDeliver[10]) & 0xFF) << 16) + (((dataBufferDeliver[11]) & 0xFF) << 24);
        int Damage = ((dataBufferDeliver[24]) & 0xFF) + (((dataBufferDeliver[25]) & 0xFF) << 8) +
            (((dataBufferDeliver[26]) & 0xFF) << 16) + (((dataBufferDeliver[27]) & 0xFF) << 24);
        
        if (SourceAid == *AidAddress) {
            printf("你對目標 %d 使用技能 %d 造成傷害 %d\n", TargetAid, TargetSkillId, Damage);
            if (gNearAttackSkill == TargetSkillId && TargetAid != *AidAddress && Damage > 0) {
                gNearAttackAck = 1;
                gNearAttackAckTotal = 0;
            }

            gDetactYouAreCastGhostring = 1;
            gDetactYouAreCastGhostringIdleCounter = 1000;
            if (TargetAid != *AidAddress) {
                // 偵測你對人放招
                gDetactYouAreCast = 1;
                gDetactYouAreCastIdleCounter = 1000;
                // 魔物報告
                if (gExpReportSwitch) {
                    // 標記魔物被你攻擊過
                    TargetAidExpReportList.update_target_to_you_hit(TargetAid);
                }
            }
        }
        else {
            //
            // 防止手殘沒吃到水
            //
            if (TargetAid == *AidAddress) {
                //printf("目標 %d 對你 使用技能 %d 造成傷害 %d\n", SourceAid, TargetSkillId, Damage);
                if (PVP && gDetactSpecialAttackIdleCounter == 0 && gAutoSmartDrinkSwitch) {
                    CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(DetactSpecialAttackFunction), NULL, 0, 0);
                }
                else if (gDetactSpecialAttackIdleCounter == 0 && gAutoSmartDrinkSwitch) {
                    gDetactSpecialAttack = 1;
                    gDetactSpecialAttackIdleCounter = 1000;
                }

                // 智能被揍切裝
                for (int SkillCounter = 0; SkillCounter < 6; SkillCounter++) {
                    if (TargetSkillId == gAutoSmartEquipSkill[SkillCounter]) {
                        gSmartEquipTrigger[SkillCounter] = 1;
                        gSmartEquipTriggerIdleCounter[SkillCounter] = gAutoSmartEquipDelay[SkillCounter];
                    }
                }

                gDieNoteAid = SourceAid;
                gDieNoteDamage = Damage;
                gDieNoteSkill = TargetSkillId;
            }
            else {
                //printf("目標 %d 對 %d 使用技能 %d 造成傷害 %d\n", SourceAid, TargetAid, TargetSkillId, Damage);
            }
        }

        //int newLineCount = 0;
        //for (int Count = 0; Count < len; Count++) {
        //    printf("%02hhX ", dataBufferDeliver[Count]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

        //
        // 判斷是對你使用
        //
        if (TargetAid == *AidAddress) {
            if (gDetectSpecialSkillSwitch1) {
                if (TargetSkillId == gDetectSpecialSkillId1) {
                    gDetectHitSkillBuffer = SourceAid;
                    gDetectHitTriggerFlag = 1;
                }
            }
            if (gDetectSpecialSkillSwitch2) {
                if (TargetSkillId == gDetectSpecialSkillId2) {
                    gDetectHitSkillBuffer = SourceAid;
                    gDetectHitTriggerFlag = 2;
                }
            }
            if (gDetectSpecialSkillSwitch3) {
                if (TargetSkillId == gDetectSpecialSkillId3) {
                    gDetectHitSkillBuffer = SourceAid;
                    gDetectHitTriggerFlag = 3;
                }
            }
        }

        //
        // 判斷是你使用技能
        //
        if (SourceAid == *AidAddress && TargetAid != *AidAddress) {
            if (gAttackHitSwitch1 && !gAttackHitTriggerFlag1) {
                if (TargetSkillId == gAttackHitSkillID1 && !gAttackHitGround1) {
                    gAttackHitSkillBuffer = TargetAid;
                    gAttackHitTriggerFlag1 = 1;
                }
                //else if (TargetSkillId == gAttackHitSkillID1 && gAttackHitGround1) { // 地板技能
                //    int* CorrdX = new int;
                //    int* CorrdY = new int;

                //    // 取得目標的X ,Y座標
                //    int IsFind = TargetAidList.search_target_Aid(TargetAid, CorrdX, CorrdY);

                //    if (IsFind) {
                //        gAttackHitGroundCoordX1 = *CorrdX;
                //        gAttackHitGroundCoordY1 = *CorrdY;
                //        gAttackHitTriggerFlag1 = 1;
                //    }
                //    delete CorrdX;
                //    delete CorrdY;
                //}
            }
            if (gAttackHitSwitch2 && !gAttackHitTriggerFlag2) {
                if (TargetSkillId == gAttackHitSkillID2 && !gAttackHitGround2) {
                    gAttackHitSkillBuffer = TargetAid;
                    gAttackHitTriggerFlag2 = 2;
                }
                //else if (TargetSkillId == gAttackHitSkillID2 && gAttackHitGround2) { // 地板技能
                //    int* CorrdX = new int;
                //    int* CorrdY = new int;

                //    // 取得目標的X ,Y座標
                //    int IsFind = TargetAidList.search_target_Aid(TargetAid, CorrdX, CorrdY);

                //    if (IsFind) {
                //        gAttackHitGroundCoordX2 = *CorrdX;
                //        gAttackHitGroundCoordY2 = *CorrdY;
                //        gAttackHitTriggerFlag2 = 2;
                //    }
                //    delete CorrdX;
                //    delete CorrdY;
                //}
            }
            if (gAttackHitSwitch3 && !gAttackHitTriggerFlag3) {
                if (TargetSkillId == gAttackHitSkillID3 && !gAttackHitGround3) {
                    gAttackHitSkillBuffer = TargetAid;
                    gAttackHitTriggerFlag3 = 3;
                }
                //else if (TargetSkillId == gAttackHitSkillID3 && gAttackHitGround3) { // 地板技能
                //    int* CorrdX = new int;
                //    int* CorrdY = new int;

                //    // 取得目標的X ,Y座標
                //    int IsFind = TargetAidList.search_target_Aid(TargetAid, CorrdX, CorrdY);

                //    if (IsFind) {
                //        gAttackHitGroundCoordX3 = *CorrdX;
                //        gAttackHitGroundCoordY3 = *CorrdY;
                //        gAttackHitTriggerFlag3 = 3;
                //    }
                //    delete CorrdX;
                //    delete CorrdY;
                //}
            }
        }
    }

    //
    // 0087 - character_moves
    //
    //		'0087' => ['character_moves', 'a4 a6', [qw(move_start_time coords)]], # 12
    if (packetName == 0x0087) { // 本腳色移動
        //  設置AP閒置時間
        gAutoApIdleCounter = gAutoApIdle;
    }

    ////
    //// '09CA' => ['area_spell_multiple3', 'v a*', [qw(len spellInfo)]], # -1
    ////
    //if (packetName == 0x09CA) { // area_spell_multiple3

    //    int SourceAid = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
    //        (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
    //    int TargetAid = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8) +
    //        (((dataBufferDeliver[10]) & 0xFF) << 16) + (((dataBufferDeliver[11]) & 0xFF) << 24);

    //}


    //
    // 07FB - Skill Cast [25 bytes]
    //
    if (packetName == SkillCast) { // 偵測到有人想對你施展特定技能
        packetLen = 25;
                
        int SourceAid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8) +
            (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int TargetAid = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8) +
            (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int TargetSkillId = ((dataBufferDeliver[14]) & 0xFF) + (((dataBufferDeliver[15]) & 0xFF) << 8);
        int enemyX = ((dataBufferDeliver[10]) & 0xFF) + (((dataBufferDeliver[11]) & 0xFF) << 8);
        int enemyY = ((dataBufferDeliver[12]) & 0xFF) + (((dataBufferDeliver[13]) & 0xFF) << 8);

        //
        // 反擊-判斷敵方使用自我技能
        //
        if (SourceAid == TargetAid && SourceAid != *AidAddress && 
            TargetSkillId == 2209 ||  // 魔力凍結
            TargetSkillId == 10013 || // 緊急呼叫
            TargetSkillId == 243 ||   //243#生命體召喚#
            TargetSkillId == 311      // 洛奇的悲鳴
            ) {
            //int* CorrdX = new int;
            //int* CorrdY = new int;
            //// 取得目標的X ,Y座標
            //int IsFind = TargetAidList.search_target_Aid(SourceAid, CorrdX, CorrdY);
            //if (IsFind) {
            //    gAttackHitGroundCoordX1 = *CorrdX;
            //    gAttackHitGroundCoordY1 = *CorrdY;
            //    gAttackHitTriggerFlag1 = 1;
            //}
            //delete CorrdX;
            //delete CorrdY;
            if (gNearAttackAidMode && !IsEnemyAid2(SourceAid)) {
                // 開啟只攻擊特定AID的模式, 非AID列表就直接跳過
            }
            else if (!gNearAttackAidMode && PlayerAidList.search_player(SourceAid)) {
                // 沒有開啟AID模式, 但目標為盟友
            }
            else if (CheckTheAidIsInParty(SourceAid)) {
                // 如果玩家在隊伍裡面, 忽略
            }
            else {
                if (gDetectSpecialSkillSwitch1) {
                    if (TargetSkillId == gDetectSpecialSkillId1) {
                        debug("有人對你使用特定技能 01");
                        gDetectHitSkillBuffer = SourceAid;
                        gDetectHitTriggerFlag = 1;
                    }
                }
                if (gDetectSpecialSkillSwitch2) {
                    if (TargetSkillId == gDetectSpecialSkillId2) {
                        debug("有人對你使用特定技能 02");
                        gDetectHitSkillBuffer = SourceAid;
                        gDetectHitTriggerFlag = 2;
                    }
                }
                if (gDetectSpecialSkillSwitch3) {
                    if (TargetSkillId == gDetectSpecialSkillId3) {
                        debug("有人對你使用特定技能 03");
                        gDetectHitSkillBuffer = SourceAid;
                        gDetectHitTriggerFlag = 3;
                    }
                }
            }

        }

        if (SourceAid == *AidAddress) {
            printf("你對目標 %d 使用技能 %d (%d, %d)\n", TargetAid, TargetSkillId, enemyX, enemyY);
            if (TargetSkillId != 26) { // 除了 瞬移技能外, 歸零閒置時間
                gAutoTeleIdleCounter = 0;
            }

            if (TargetSkillId != gAutoApSkill) { // 除了 集AP技能外, 設置AP閒置時間
                gAutoApIdleCounter = gAutoApIdle;
            }

            // 紀錄技能ID
            if (gCatchModeSwitch) {

                char SkillIdText[20] = { "" };
                sprintf(SkillIdText, "%d", TargetSkillId); // 轉化數字變成文字

                ::WritePrivateProfileString(gCharNameStr, _T("最近一次使用技能"), SkillIdText, _T(".\\data\\charNameList.txt"));
            }

            // 壓血羅剎
            if (TargetSkillId == 2343 && gSmartBloodSwitch) {
                gSmartBloodTrigger = 1;
            }

            // 集結隊長放招
            if (TargetSkillId == 5073 && !gConvenioTrigger && gConvenioTotalSwitch && gConvenioLeaderSwitch) {
                CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ConvenioFunction), NULL, 0, 0);
            }


            if (gNearAttackSkill == TargetSkillId) {
                if (TargetAid != *AidAddress) {
                    //gNearAttackAck = 1;
                    //gNearAttackAckTotal = 0;
                }
                /*else if (gNearAttackSupport) {
                    gNearAttackAck = 1;
                }*/
                        
                //gNearAttackAckTotal = 0;
            }
            if (gSkillIdAck == TargetSkillId) {
                printf("02: 技能%d ACK \n", gSkillIdAck);
                gSkillAck = 1;
            }
                    
            gDetactYouAreCastGhostring = 1;
            gDetactYouAreCastGhostringIdleCounter = 1000;
            if (TargetAid != *AidAddress) {
                // 偵測你對人放招
                gDetactYouAreCast = 1;
                gDetactYouAreCastIdleCounter = 1000;
            }
        }
        else {
            if (TargetAid == *AidAddress) {
                //printf("07FB - Skill Cast [25 bytes] %d 對你使用技能: %d (%d, %d)\n", SourceAid, TargetSkillId, enemyX, enemyY);
            }
            else {
                //printf("07FB - Skill Cast [25 bytes] %d 對 %d 使用技能: %d (%d, %d)\n", SourceAid, TargetAid, TargetSkillId, enemyX, enemyY);
            }

            //
            // 防止手殘沒吃到水
            //
            if (TargetAid == *AidAddress && (TargetSkillId == 5033 || TargetSkillId == 5026 || TargetSkillId == 2213 || TargetSkillId == 2447)) {

                if (PVP && gDetactSpecialAttackIdleCounter == 0 && gAutoSmartDrinkSwitch) {
                    CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(DetactSpecialAttackFunction), NULL, 0, 0);
                }
                else if (gDetactSpecialAttackIdleCounter == 0 && gAutoSmartDrinkSwitch) {
                    gDetactSpecialAttack = 1;
                    gDetactSpecialAttackIdleCounter = 1000;
                }
            }

            // 智能被揍切裝
            if (TargetAid == *AidAddress) {
                for (int SkillCounter = 0; SkillCounter < 6; SkillCounter++) {
                    if (TargetSkillId == gAutoSmartEquipSkill[SkillCounter]) {
                        gSmartEquipTrigger[SkillCounter] = 1;
                        gSmartEquipTriggerIdleCounter[SkillCounter] = gAutoSmartEquipDelay[SkillCounter];
                    }
                }
            }

            //// 假設是魔凍 必須智能切裝 2209#魔力凍結#
            //if (TargetSkillId == 2209) {
            //    for (int SkillCounter = 0; SkillCounter < 6; SkillCounter++) {
            //        if (TargetSkillId == gAutoSmartEquipSkill[SkillCounter]) {
            //            gSmartEquipTrigger[SkillCounter] = 1;
            //            gSmartEquipTriggerIdleCounter[SkillCounter] = 2000;
            //        }
            //    }
            //}
        }
        //
        //
        // 判斷是對你使用
        //
        if (TargetAid == *AidAddress) {
            if (gDetectSpecialSkillSwitch1) {
                if (TargetSkillId == gDetectSpecialSkillId1) {
                    debug("有人對你使用特定技能 01");
                    gDetectHitSkillBuffer = SourceAid;
                    gDetectHitTriggerFlag = 1;
                }
            }
            if (gDetectSpecialSkillSwitch2) {
                if (TargetSkillId == gDetectSpecialSkillId2) {
                    debug("有人對你使用特定技能 02");
                    gDetectHitSkillBuffer = SourceAid;
                    gDetectHitTriggerFlag = 2;
                }
            }
            if (gDetectSpecialSkillSwitch3) {
                if (TargetSkillId == gDetectSpecialSkillId3) {
                    debug("有人對你使用特定技能 03");
                    gDetectHitSkillBuffer = SourceAid;
                    gDetectHitTriggerFlag = 3;
                }
            }

        }
        //
        // 是否是弓身
        //
        else if (TargetSkillId == 264) { // 264 = 弓身技能ID
            // 你本人使用弓身
            if (SourceAid == *AidAddress) {
                // 快速弓身放招測試
                if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW
                    && *HPIndexValue > 1
                    && CloakingTryNum >= 3
                    && CheckState()
                    && !gStorageStatus
                    && gNearAttackSwitch
                    && !gNearAttackMonster
                    && IsPvpMap() // PVP地圖
                    ) {
                    CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AttackToPlayerFunction), NULL, 0, 0);
                }
            }
            //
            // 非你本人使用躬身
            //
            else if (gNearAttackSwitch && gTotalSwitch && IsPvpMap()) {
                     
                //
                // 職業必須是修羅類型
                //
                if (JobFilterFunction(15) || JobFilterFunction(4016) || JobFilterFunction(4077) || JobFilterFunction(4262)) {

                    if (TargetAidList.search_target(SourceAid)) {
                        TargetAidList.update_target_MoveXY(SourceAid, enemyX, enemyY, 0, 0, 0, -1);
                    }
                    else {
                        TargetAidList.add_target_node(SourceAid, 0, enemyX, enemyY, -1, 0, NULL);
                        // 更新移動方向為靜止
                        TargetAidList.update_target_MoveXY(SourceAid, enemyX, enemyY, 0, 0, 0, -1);
                    }
                }

            }
        }

        //
        // 判斷是你使用技能
        //
        if (SourceAid == *AidAddress) {
            if (gAttackHitSwitch1 && !gAttackHitTriggerFlag1) {
                if (TargetSkillId == gAttackHitSkillID1 && !gAttackHitGround1) {
                    gAttackHitSkillBuffer = TargetAid;
                    gAttackHitTriggerFlag1 = 1;
                }
                else if (TargetSkillId == gAttackHitSkillID1 && gAttackHitGround1 && enemyX && enemyY) { // 地板技能
                    gAttackHitGroundCoordX1 = enemyX;
                    gAttackHitGroundCoordY1 = enemyY;
                    gAttackHitTriggerFlag1 = 1;
                }
            }
            if (gAttackHitSwitch2 && !gAttackHitTriggerFlag2) {
                if (TargetSkillId == gAttackHitSkillID2 && !gAttackHitGround2) {
                    gAttackHitSkillBuffer = TargetAid;
                    gAttackHitTriggerFlag2 = 2;
                }
                else if (TargetSkillId == gAttackHitSkillID2 && gAttackHitGround2 && enemyX && enemyY) { // 地板技能
                    gAttackHitGroundCoordX2 = enemyX;
                    gAttackHitGroundCoordY2 = enemyY;
                    gAttackHitTriggerFlag2 = 2;
                }
            }
            if (gAttackHitSwitch3 && !gAttackHitTriggerFlag3) {
                if (TargetSkillId == gAttackHitSkillID3 && !gAttackHitGround3) {
                    gAttackHitSkillBuffer = TargetAid;
                    gAttackHitTriggerFlag3 = 3;
                }
                else if (TargetSkillId == gAttackHitSkillID3 && gAttackHitGround3 && enemyX && enemyY) { // 地板技能
                    gAttackHitGroundCoordX3 = enemyX;
                    gAttackHitGroundCoordY3 = enemyY;
                    gAttackHitTriggerFlag3 = 3;
                }
            }
        }
    }

    //
    // 0A30 - Actor Get Info (Name) [106 bytes]
    //
    if (packetName == ActorGetInfo) { // 偵測到公會成員

        //printf("Actor Get Info (Name) [106 bytes] %d", len);

        int AidTmp = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);

        if (AidTmp) {
            //
            // 抓取角色ID
            // 
            char CharNameStr[CharNameLen] = { "" };
            int CharLen = 24;
            if (CharLen) {
                for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                    CharNameStr[CharCount] = dataBufferDeliver[6 + CharCount];
                }
                //printf(" %s ", CharNameStr);
            }

            //
            // 抓取組隊ID
            // 
            char PartyNameStr[CharNameLen] = { "" };
            int PartyLen = 24;
            if (PartyLen) {
                for (int PartyCount = 0; PartyCount < PartyLen; PartyCount++) {
                    PartyNameStr[PartyCount] = dataBufferDeliver[30 + PartyCount];
                }
                //printf(" 組隊: %s ", PartyNameStr);
            }

            //
            // 抓取公會ID
            // 
            char GuildNameStr[CharNameLen] = { "" };
            int GuildLen = 24;
            if (GuildLen) {
                for (int GuildCount = 0; GuildCount < GuildLen; GuildCount++) {
                    GuildNameStr[GuildCount] = dataBufferDeliver[54 + GuildCount];
                }
                //printf(" 公會: %s ", GuildNameStr);
            }
            //printf("\n");

            //
            // 紀錄公會名稱
            //
            if (gCatchGuildIdMode && strcmp(CharNameStr, "") && strcmp(GuildNameStr, "")) {
                ::WritePrivateProfileString(CharNameStr, _T("公會名稱"), GuildNameStr, _T(".\\data\\CatchAidList.txt")); // 公會名稱
            }
            if (gCatchGuildIdMode && strcmp(CharNameStr, "") && strcmp(PartyNameStr, "")) {
                ::WritePrivateProfileString(CharNameStr, _T("組隊名稱"), PartyNameStr, _T(".\\data\\CatchAidList.txt")); // 組隊名稱
            }
        }
    }

    //
    // 0088 - Actor Position [10 bytes]
    //
    if (packetName == ActorPosition) { // 座標更新
        packetLen = 10;

        int AidTmp = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int enemyX = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8);
        int enemyY = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8);

        //printf("0088 - Actor Position [10 bytes] %d (%d, %d)\n", AidTmp, enemyX, enemyY);

        //
        // 敵人移動
        //
        //
        // 更新到target列表
        //
        if (AidTmp > 0 && enemyX > 0 && enemyY > 0 && enemyX < 3000 && enemyY < 3000) {
            if (TargetAidList.search_target(AidTmp)) {
                //TargetAidList.update_target_Corrd(AidTmp, enemyX, enemyY);
                // 更新移動方向為靜止
                TargetAidList.update_target_MoveXY(AidTmp, enemyX, enemyY, 0, 0, 0, -1);
            }

            if (TargetAidExpReportList.search_target(AidTmp)) {
                //TargetAidExpReportList.update_target_Corrd(AidTmp, enemyX, enemyY);
                // 更新移動方向為靜止
                TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyX, enemyY, 0, 0, 0, -1);
            }
        }
    }

    //
    // 08D2 - High Jump [10 bytes]
    //
    if (packetName == HighJump) { // 座標更新
        packetLen = 10;

        int AidTmp = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8)
            + (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int enemyX = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8);
        int enemyY = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8);

        //printf("08D2 - High Jump [10 bytes] %d (%d, %d)\n", AidTmp, enemyX, enemyY);

        //
        // 敵人移動
        //
        //
        // 更新到target列表
        //
        if (AidTmp > 0 && enemyX > 0 && enemyY > 0 && enemyX < 3000 && enemyY < 3000) {
            if (gNearAttackSwitch && !gNearAttackMonster) {
                // 自動鎖定打玩家, 不更新位移座標怕危險
            }
            else if (TargetAidList.search_target(AidTmp)) {
                //TargetAidList.update_target_Corrd(AidTmp, enemyX, enemyY);
                // 更新移動方向為靜止
                TargetAidList.update_target_MoveXY(AidTmp, enemyX, enemyY, 0, 0, 0, -1);
            }

            if (TargetAidExpReportList.search_target(AidTmp)) {
                //TargetAidExpReportList.update_target_Corrd(AidTmp, enemyX, enemyY);
                // 更新移動方向為靜止
                TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyX, enemyY, 0, 0, 0, -1);
            }
        }
    }

    //
    // 09FD - actor_display (actor moved) [98 bytes]
    //
    if (packetName == ActorMoved) { // actor moved
// 		'09FD' => ['actor_moved', 'v C a4 a4 v3 V v2 V2 v V v6 a4 a2 v V C2 a6 C2 v2 V2 C v Z*', 
//        [qw(len object_type ID charID walk_speed opt1 opt2 option type hair_style weapon shield lowhead tick tophead midhead hair_color clothes_color head_dir costume guildID emblemID manner opt3 stance sex coords xSize ySize lv font maxHP HP isBoss opt4 name)]],

        packetLen = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8);
        if (packetLen < 90 || packetLen > 110) {
            packetLen = 90;
        }

        //
        // 判斷是公會成員 且不是自己
        //
        int AidTmp = ((dataBufferDeliver[5]) & 0xFF) + (((dataBufferDeliver[6]) & 0xFF) << 8)
            + (((dataBufferDeliver[7]) & 0xFF) << 16) + (((dataBufferDeliver[8]) & 0xFF) << 24);
        int ObjectType = dataBufferDeliver[4] & 0xFF;
        int WalkSpeed = ((dataBufferDeliver[13]) & 0xFF) + (((dataBufferDeliver[14]) & 0xFF) << 8);
        int Job = ((dataBufferDeliver[23]) & 0xFF) + (((dataBufferDeliver[24]) & 0xFF) << 8);
        int MonsterIdTmp = ((dataBufferDeliver[23]) & 0xFF) + (((dataBufferDeliver[24]) & 0xFF) << 8); // 怪物ID
        // 座標
        int enemyXFrom = ((((dataBufferDeliver[67]) & 0xFF) << 12) + (((dataBufferDeliver[68]) & 0xFF) << 4) + (((dataBufferDeliver[69]) & 0xFF) >> 4)) >> 10;
        int enemyYFrom = ((((dataBufferDeliver[67]) & 0xFF) << 12) + (((dataBufferDeliver[68]) & 0xFF) << 4) + (((dataBufferDeliver[69]) & 0xFF) >> 4)) & 0x3FF;
        int enemyXTo = (((dataBufferDeliver[69]) & 0x0F) << 16) + (((dataBufferDeliver[70]) & 0xFF) << 8) + ((dataBufferDeliver[71]) & 0xFF) >> 10;
        int enemyYTo = (((dataBufferDeliver[69]) & 0x0F) << 16) + (((dataBufferDeliver[70]) & 0xFF) << 8) + ((dataBufferDeliver[71]) & 0xFF) & 0x3FF;

        // 格數花的時間 豪秒數 = WalkSpeed * 距離

        //
        // 抓取角色ID
        // 
        char CharNameStr[CharNameLen] = { "" };
        int CharLen = packetLen - 90;
        if (CharLen) {
            for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                CharNameStr[CharCount] = dataBufferDeliver[90 + CharCount];
            }
        }
        char CharNameStr2[CharNameLen] = { "" };
        int CharLen2 = packetLen - 90 - 1;
        if (CharLen2) {
            for (int CharCount = 0; CharCount < CharLen2; CharCount++) {
                CharNameStr2[CharCount] = dataBufferDeliver[90 + CharCount];
            }
        }

        if (!CharLen || !CharLen2) { // 當怪物名子為空的
            CharLen = MonsterReadFunction(CharNameStr, MonsterIdTmp);
            CharLen2 = MonsterReadFunction(CharNameStr2, MonsterIdTmp);
        }

        if (TargetAidExpReportList.search_target(AidTmp)) {
            TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo, 1, WalkSpeed);
        }
        else {
            TargetAidExpReportList.add_target_node(AidTmp, ObjectType, enemyXFrom, enemyYFrom, WalkSpeed, MonsterIdTmp, CharNameStr);
            // 更新移動方向
            TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo, 1, WalkSpeed);
        }

        //printf("09FD - actor_display (actor moved) [%d bytes] (%d) %s (%d, %d) -> (%d, %d) %d (%d) WalkSpeed: %d\n",
        //    packetLen, ObjectType, CharNameStr, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo, MonsterIdTmp, Job, WalkSpeed);

        //int newLineCount = 0;
        //for (int printCount = 0; printCount < len; printCount++) {
        //    printf("%02hhX ", dataBufferDeliver[printCount]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

        if (gAutoTelePoint && MonsterIdTmp == 1956 && gTotalSwitch) {
            gAutoTelePointTrigger = 1;
            printf(" 遇到夜勝魔!!\n");
        }
        //
        // 抓取忽略怪物AID
        // 
        if (gNearAttackMonsterIgnoreSwitch
            && CheckIgnoreMonsterFunction(CharNameStr, CharNameStr2)) {
            if (!PlayerAidList.search_player(AidTmp)) {
                //printf("%s 加入忽略怪物清單\n", CharNameStr);
                PlayerAidList.add_player_node(AidTmp);
            }
        }
        // 針對怪物名單
        if (gNearAttackMonsterSpecialSwitch
            && !CheckIgnoreMonsterFunction2(CharNameStr, CharNameStr2)) {
            if (!PlayerAidList.search_player(AidTmp)) {
                //printf("%s 加入忽略怪物清單\n", CharNameStr);
                PlayerAidList.add_player_node(AidTmp);
            }
        }
        //
        // 判斷是否是GM
        //
        IsGmAid(dataBufferDeliver, 5);
        //
        // 更新到target列表
        //
        if (AidTmp && enemyXTo > 0 && enemyYTo > 0 && enemyXTo < 3000 && enemyYTo < 3000 && (CharLen > 0 || CharLen2 > 0)) {
            if (gNearAttackSwitch && !gNearAttackMonster && !JobFilterFunction(Job)) {
                // 自鎖玩家的職業過慮
            }
            else {
                if (TargetAidList.search_target(AidTmp)) {
                    TargetAidList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo, 1, WalkSpeed);
                }
                else {
                    TargetAidList.add_target_node(AidTmp, ObjectType, enemyXFrom, enemyYFrom, WalkSpeed, MonsterIdTmp, CharNameStr);
                    // 更新移動方向
                    TargetAidList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo, 1, WalkSpeed);
                }
            }
        }

        ////
        //// 發現隊長集結, 停止瞬移10秒
        ////
        //if (gTotalSwitch && gLeaderSwitch) {
        //    if (AidTmp == gLeaderAid && gAutoMonsterTeleIdleCounter < 200 && gLeaderIdleCounter > 0) {
        //        // 必須距離9格內
        //        if (((abs(*PLAYER_CORRD_X - enemyXFrom) * abs(*PLAYER_CORRD_X - enemyXFrom)) + (abs(*PLAYER_CORRD_Y - enemyYFrom) * abs(*PLAYER_CORRD_Y - enemyYFrom))) <= (gLeaderDist * gLeaderDist)) {
        //            gAutoMonsterTeleIdleCounter = gLeaderDelay;
        //            if (gLeaderApGive) {
        //                //Sleep(1000);
        //                SendSkillCastFunction(5368, 5, gLeaderAid);
        //            }
        //            // 被集結自動放招
        //            if (!gConvenioTrigger && gConvenioTotalSwitch) {
        //                CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ConvenioFunction), NULL, 0, 0);
        //            }
        //        }

        //    }
        //}

        if (AidTmp == *AidAddress) {
        }
        else if (IsGuidMember(dataBufferDeliver, 53)) {
            printf ("發現公會盟友 %s\n", CharNameStr);
            if (!PlayerAidList.search_player(AidTmp)) {
                PlayerAidList.add_player_node(AidTmp);
            }
        }
    }

    //
    // 0086 - character_moves (actor moved) [16 bytes]
    //
    if (packetName == CharacterMoves && PrivateServerOrNot) { // 私服角色移動
        packetLen = 16;
        
        int AidTemp = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8) + ((dataBufferDeliver[4] & 0xFF) << 16) + ((dataBufferDeliver[5] & 0xFF) << 24);
        int enemyXFrom = ((((dataBufferDeliver[6]) & 0xFF) << 12) + (((dataBufferDeliver[7]) & 0xFF) << 4) + (((dataBufferDeliver[8]) & 0xFF) >> 4)) >> 10;
        int enemyYFrom = ((((dataBufferDeliver[6]) & 0xFF) << 12) + (((dataBufferDeliver[7]) & 0xFF) << 4) + (((dataBufferDeliver[8]) & 0xFF) >> 4)) & 0x3FF;
        int enemyXTo = (((dataBufferDeliver[8]) & 0x0F) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 8) + ((dataBufferDeliver[10]) & 0xFF) >> 10;
        int enemyYTo = (((dataBufferDeliver[8]) & 0x0F) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 8) + ((dataBufferDeliver[10]) & 0xFF) & 0x3FF;
        //printf("0086 - character_moves (actor moved) [16 bytes] %d (%d, %d) -> (%d, %d)\n", AidTemp, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo);

        //
        // 更新到target列表
        //
        if (AidTemp > 0 && enemyXTo > 0 && enemyYTo > 0 && enemyXTo < 3000 && enemyYTo < 3000) {
            if (TargetAidList.search_target(AidTemp)) {
                //TargetAidList.update_target_Corrd(AidTemp, enemyXTo, enemyYTo);
                // 更新移動方向
                TargetAidList.update_target_MoveXY(AidTemp, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo, 1, -1);
            }

            if (TargetAidExpReportList.search_target(AidTemp)) {
                //TargetAidExpReportList.update_target_Corrd(AidTemp, enemyXTo, enemyYTo);
                // 更新移動方向
                TargetAidExpReportList.update_target_MoveXY(AidTemp, enemyXFrom, enemyYFrom, enemyXTo, enemyYTo, 1, -1);
            }
        }

        if (AidTemp == *AidAddress) {
        }
    }

    //
    // 09FE - Actor Connected [91 bytes]
    //
    if (packetName == ActorConnected) { // Actor Connected
        packetLen = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8);

        if (packetLen < 83 || packetLen > 200) {
            packetLen = 83;
        }

        int AidTmp = ((dataBufferDeliver[5]) & 0xFF) + (((dataBufferDeliver[6]) & 0xFF) << 8)
            + (((dataBufferDeliver[7]) & 0xFF) << 16) + (((dataBufferDeliver[8]) & 0xFF) << 24);
        int ObjectType = dataBufferDeliver[4] & 0xFF;
        int enemyXFrom = ((((dataBufferDeliver[63]) & 0xFF) << 12) + (((dataBufferDeliver[64]) & 0xFF) << 4) + (((dataBufferDeliver[65]) & 0xFF) >> 4)) >> 10;
        int enemyYFrom = ((((dataBufferDeliver[63]) & 0xFF) << 12) + (((dataBufferDeliver[64]) & 0xFF) << 4) + (((dataBufferDeliver[65]) & 0xFF) >> 4)) & 0x3FF;
        int Job = ((dataBufferDeliver[23]) & 0xFF) + (((dataBufferDeliver[24]) & 0xFF) << 8);
        int MonsterIdTmp = ((dataBufferDeliver[23]) & 0xFF) + (((dataBufferDeliver[24]) & 0xFF) << 8);
        int WalkSpeed = ((dataBufferDeliver[13]) & 0xFF) + (((dataBufferDeliver[14]) & 0xFF) << 8);
        //
        // 判斷是公會成員 且不是自己
        //
        IsGmAid(dataBufferDeliver, 5);
        
        // 抓取角色ID
        // 
        char CharNameStr[CharNameLen] = { "" };
        int CharLen = packetLen - 83;
        if (CharLen) {
            for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                CharNameStr[CharCount] = dataBufferDeliver[83 + CharCount];
            }
            //printf(" %s ", CharNameStr);
        }
        char CharNameStr2[CharNameLen] = { "" };
        int CharLen2 = packetLen - 83 - 1;
        if (CharLen2) {
            for (int CharCount = 0; CharCount < CharLen2; CharCount++) {
                CharNameStr2[CharCount] = dataBufferDeliver[83 + CharCount];
            }
        }
        if (!CharLen || !CharLen2) { // 當怪物名子為空的
            CharLen = MonsterReadFunction(CharNameStr, MonsterIdTmp);
            CharLen2 = MonsterReadFunction(CharNameStr2, MonsterIdTmp);
            //printf(" %s ", CharNameStr);
        }

        // 座標
        //printf("(%d, %d) (%d)\n", enemyXFrom, enemyYFrom, Job);

        // 怪物ID
        //printf("09FE - Actor Connected [%d bytes] %s (%d, %d) (%d)\n", packetLen, CharNameStr, enemyXFrom, enemyYFrom, Job);

        if (TargetAidExpReportList.search_target(AidTmp)) {
            TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, WalkSpeed);
        }
        else {
            TargetAidExpReportList.add_target_node(AidTmp, ObjectType, enemyXFrom, enemyYFrom, WalkSpeed, MonsterIdTmp, CharNameStr);
            // 更新移動方向為靜止
            TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, -1);
        }

        //
        // 自動抓取公會ID
        //
        if (gTotalSwitch && gCatchGuildIdMode && gCatchGuildIdAid) {
            if ((dataBufferDeliver[5] & 0xFF) == (gCatchGuildIdAid & 0xFF) && (dataBufferDeliver[6] & 0xFF) == ((gCatchGuildIdAid & 0xFF00) >> 8)
                && (dataBufferDeliver[7] & 0xFF) == ((gCatchGuildIdAid & 0xFF0000) >> 16) && (dataBufferDeliver[8] & 0xFF) == ((gCatchGuildIdAid & 0xFF000000) >> 24)) {
                //debug("抓到欲抓的角色之公會ID");
                gCatchGuildId = ((dataBufferDeliver[49]) & 0xFF) + (((dataBufferDeliver[50]) & 0xFF) << 8)
                    + (((dataBufferDeliver[51]) & 0xFF) << 16) + (((dataBufferDeliver[52]) & 0xFF) << 24);
            }
        }
        //
        // 抓取忽略怪物AID
        // 
        if (gNearAttackMonsterIgnoreSwitch
            && CheckIgnoreMonsterFunction(CharNameStr, CharNameStr2)) {
            //printf("%s 加入忽略怪物清單\n", CharNameStr);
            if (!PlayerAidList.search_player(AidTmp)) {
                PlayerAidList.add_player_node(AidTmp);
            }
        }
        if (gNearAttackMonsterSpecialSwitch
            && !CheckIgnoreMonsterFunction2(CharNameStr, CharNameStr2)) {
            if (!PlayerAidList.search_player(AidTmp)) {
                //printf("%s 加入忽略怪物清單\n", CharNameStr);
                PlayerAidList.add_player_node(AidTmp);
            }
        }

        if (IsYourAidOrNot(dataBufferDeliver, 5)) {
        }
        else if (IsGuidMember(dataBufferDeliver, 49)) {
            printf("發現公會盟友 %s\n", CharNameStr);
            if (!PlayerAidList.search_player(AidTmp)) {
                PlayerAidList.add_player_node(AidTmp);
            }
        }

        //
        // 更新到target列表
        //
        if (AidTmp && enemyXFrom > 0 && enemyYFrom > 0 && enemyXFrom < 3000 && enemyYFrom < 3000) {
            if (gNearAttackSwitch && !gNearAttackMonster && !JobFilterFunction(Job)) {
                // 自鎖玩家的職業過慮
            }
            else {
                if (TargetAidList.search_target(AidTmp)) {
                    TargetAidList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, WalkSpeed);
                }
                else {
                    TargetAidList.add_target_node(AidTmp, ObjectType, enemyXFrom, enemyYFrom, WalkSpeed, MonsterIdTmp, CharNameStr);
                    // 更新移動方向為靜止
                    TargetAidList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, -1);
                }
            }
        }
    }

    //
    // 09FF - actor_display (actor exists) [92 bytes]
    //
    if (packetName == ActorExists) { // actor exists
        packetLen = (dataBufferDeliver[2] & 0xFF) + ((dataBufferDeliver[3] & 0xFF) << 8);
        if (packetLen < 84 || packetLen > 200) {
            packetLen = 84;
        }

        int MonsterIdTmp = ((dataBufferDeliver[23]) & 0xFF) + (((dataBufferDeliver[24]) & 0xFF) << 8);
        int AidTmp = ((dataBufferDeliver[5]) & 0xFF) + (((dataBufferDeliver[6]) & 0xFF) << 8)
            + (((dataBufferDeliver[7]) & 0xFF) << 16) + (((dataBufferDeliver[8]) & 0xFF) << 24);
        int ObjectType = dataBufferDeliver[4] & 0xFF;
        int enemyXFrom = ((((dataBufferDeliver[63]) & 0xFF) << 12) + (((dataBufferDeliver[64]) & 0xFF) << 4) + (((dataBufferDeliver[65]) & 0xFF) >> 4)) >> 10;
        int enemyYFrom = ((((dataBufferDeliver[63]) & 0xFF) << 12) + (((dataBufferDeliver[64]) & 0xFF) << 4) + (((dataBufferDeliver[65]) & 0xFF) >> 4)) & 0x3FF;
        int NpcType = dataBufferDeliver[23] & 0xFF;
        int Job = ((dataBufferDeliver[23]) & 0xFF) + (((dataBufferDeliver[24]) & 0xFF) << 8);
        int WalkSpeed = ((dataBufferDeliver[13]) & 0xFF) + (((dataBufferDeliver[14]) & 0xFF) << 8);
        int IsDie = dataBufferDeliver[68] & 0xFF;

        int Weapon = ((dataBufferDeliver[27]) & 0xFF) + (((dataBufferDeliver[28]) & 0xFF) << 8)
            + (((dataBufferDeliver[29]) & 0xFF) << 16) + (((dataBufferDeliver[30]) & 0xFF) << 24);
        int Shield = ((dataBufferDeliver[31]) & 0xFF) + (((dataBufferDeliver[32]) & 0xFF) << 8)
            + (((dataBufferDeliver[33]) & 0xFF) << 16) + (((dataBufferDeliver[34]) & 0xFF) << 24);

        //
        // 判斷是公會成員 且不是自己
        //
        IsGmAid(dataBufferDeliver, 5);

        //
        // 抓取角色ID
        // 
        char CharNameStr[CharNameLen] = { "" };
        int CharLen = packetLen - 84;
        if (CharLen) {
            for (int CharCount = 0; CharCount < CharLen; CharCount++) {
                CharNameStr[CharCount] = dataBufferDeliver[84 + CharCount];
            }
            //printf(" %d %s ", AidTmp, CharNameStr);
        }
        char CharNameStr2[CharNameLen] = { "" };
        int CharLen2 = packetLen - 84 - 1;
        if (CharLen2) {
            for (int CharCount = 0; CharCount < CharLen2; CharCount++) {
                CharNameStr2[CharCount] = dataBufferDeliver[84 + CharCount];
            }
        }
        // 座標
        //printf("(%d, %d), %d, (%d)\n", enemyXFrom, enemyYFrom, MonsterIdTmp, Job);
        
        //printf("09FF - actor_display (actor exists) [%d bytes] (%d) %d %s (%d, %d), %d, (%d) 特殊狀態: %d\n",
        //    packetLen, ObjectType, AidTmp, CharNameStr, enemyXFrom, enemyYFrom, MonsterIdTmp, Job, IsDie);

        //int newLineCount = 0;
        //for (int printCount = 0; printCount < len; printCount++) {
        //    printf("%02hhX ", dataBufferDeliver[printCount]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

        if (TargetAidExpReportList.search_target(AidTmp)) {
            TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, WalkSpeed);
        }
        else {
            TargetAidExpReportList.add_target_node(AidTmp, ObjectType, enemyXFrom, enemyYFrom, WalkSpeed, MonsterIdTmp, CharNameStr);
            // 更新移動方向為靜止
            TargetAidExpReportList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, -1);
        }

        // 登記怪物ID列表到monsters
        if (ObjectType == 5 && strcmp(CharNameStr, "")) { // 必須是怪物
            MonsterGeneratorFunction(CharNameStr, MonsterIdTmp);
        }

        //
        // 自動抓取公會ID
        //
        if (gTotalSwitch && gCatchGuildIdMode && gCatchGuildIdAid) {
            if ((dataBufferDeliver[5] & 0xFF) == (gCatchGuildIdAid & 0xFF) && (dataBufferDeliver[6] & 0xFF) == ((gCatchGuildIdAid & 0xFF00) >> 8)
                && (dataBufferDeliver[7] & 0xFF) == ((gCatchGuildIdAid & 0xFF0000) >> 16) && (dataBufferDeliver[8] & 0xFF) == ((gCatchGuildIdAid & 0xFF000000) >> 24)) {
                //debug("抓到欲抓的角色之公會ID");
                gCatchGuildId = ((dataBufferDeliver[49]) & 0xFF) + (((dataBufferDeliver[50]) & 0xFF) << 8)
                    + (((dataBufferDeliver[51]) & 0xFF) << 16) + (((dataBufferDeliver[52]) & 0xFF) << 24);
            }
        }
        //
        // 自動抓取公會ID列表
        //
        if (gCatchGuildIdMode && (dataBufferDeliver[4] & 0xFF) == 0) {
            CatchAidList(
                dataBufferDeliver,
                CharNameStr,
                // Aid
                AidTmp,
                // Guild Id
                ((dataBufferDeliver[49]) & 0xFF) + (((dataBufferDeliver[50]) & 0xFF) << 8)
                + (((dataBufferDeliver[51]) & 0xFF) << 16) + (((dataBufferDeliver[52]) & 0xFF) << 24),
                Job,
                Weapon,
                Shield
            );
        }
        ////
        //// 發現隊長集結, 停止瞬移10秒
        ////
        //if (gTotalSwitch && gLeaderSwitch) {
        //    if (AidTmp == gLeaderAid) {
        //        // 必須距離9格內
        //        if (((abs(*PLAYER_CORRD_X - enemyXFrom) * abs(*PLAYER_CORRD_X - enemyXFrom)) + (abs(*PLAYER_CORRD_Y - enemyYFrom) * abs(*PLAYER_CORRD_Y - enemyYFrom))) <= (gLeaderDist * gLeaderDist)) {
        //            gAutoMonsterTeleIdleCounter = gLeaderDelay;
        //            if (gLeaderApGive) {
        //                //Sleep(1000);
        //                SendSkillCastFunction(5368, 5, gLeaderAid);
        //            }
        //            // 被集結自動放招
        //            if (!gConvenioTrigger && gConvenioTotalSwitch) {
        //                CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ConvenioFunction), NULL, 0, 0);
        //            }
        //        }

        //    }
        //}

        //
        // 更新到target列表
        //
        if (AidTmp && enemyXFrom > 0 && enemyYFrom > 0 && enemyXFrom < 3000 && enemyYFrom < 3000) {
            if (gNearAttackSwitch && !gNearAttackMonster && !JobFilterFunction(Job)) {
                // 自鎖玩家的職業過慮
            }
            else if (IsDie != 1) {
                if (TargetAidList.search_target(AidTmp)) {
                    TargetAidList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, WalkSpeed);
                }
                else {
                    TargetAidList.add_target_node(AidTmp, ObjectType, enemyXFrom, enemyYFrom, WalkSpeed, MonsterIdTmp, CharNameStr);
                    // 更新移動方向為靜止
                    TargetAidList.update_target_MoveXY(AidTmp, enemyXFrom, enemyYFrom, 0, 0, 0, -1);
                }
            }
        }

        // 掛寶偷竊模式遇到夜聖魔就飛

        if (gAutoTelePoint && MonsterIdTmp == 1956 && gTotalSwitch) {
            gAutoTelePointTrigger = 1;
            printf(" 遇到夜勝魔!!\n");
        }

        //
        // 看見傳點, 瞬移
        //
        if (gAutoTeleSwitch && gTotalSwitch && ObjectType == 6 && NpcType == 45 && gAutoTelePoint) {
            debug("看見傳點, 瞬移!!!!!!!!!!!!!!!!!!!!");
            gAutoTelePointTrigger = 1;
        }

        //
        // 抓取忽略怪物AID
        // 
        if (gNearAttackMonsterIgnoreSwitch
            && CheckIgnoreMonsterFunction(CharNameStr, CharNameStr2)) {
            //printf("%s 加入忽略怪物清單\n", CharNameStr);
            if (!PlayerAidList.search_player(AidTmp)) {
                PlayerAidList.add_player_node(AidTmp);
            }
        }
        if (gNearAttackMonsterSpecialSwitch
            && !CheckIgnoreMonsterFunction2(CharNameStr, CharNameStr2)) {
            if (!PlayerAidList.search_player(AidTmp)) {
                //printf("%s 加入忽略怪物清單\n", CharNameStr);
                PlayerAidList.add_player_node(AidTmp);
            }
        }

        if (IsYourAidOrNot(dataBufferDeliver, 5)) {
        }
        else if (IsGuidMember(dataBufferDeliver, 49)) {
            printf("發現公會盟友 %s\n", CharNameStr);
            if (!PlayerAidList.search_player(AidTmp)) {
                PlayerAidList.add_player_node(AidTmp);
            }
        }
    }

    //
    // 0ADD - item_appeared [24 bytes]
    //
    if (packetName == ItemAppeared) { // 偵測到物品

        packetLen = 24;

        // 座標
        int ItemId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8) +
            (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int ItemNameId = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8) +
            (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int enemyXFrom = ((dataBufferDeliver[13]) & 0xFF) + (((dataBufferDeliver[14]) & 0xFF) << 8);
        int enemyYFrom = ((dataBufferDeliver[15]) & 0xFF) + (((dataBufferDeliver[16]) & 0xFF) << 8);

        //printf("0ADD - item_appeared [%d bytes] 物品標號:%d. 物品名稱: %d (%d, %d)\n", packetLen, ItemId, ItemNameId, enemyXFrom, enemyYFrom);

        if (gAutoItemTakeSwitch || gAutoGreedSwitch) {
            if (ItemAidList.search_target(ItemId)) {
                ItemAidList.delete_target_node(ItemId);
            }
            ItemAidList.add_target_node(ItemId, ItemNameId, enemyXFrom, enemyYFrom, -1, 0, NULL);
        }
    }

    //
    // 009D - item_exists [19 bytes]
    //
    if (packetName == ItemExists) { // 偵測到物品

        packetLen = 19;

        // 座標
        int ItemId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8) +
            (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int ItemNameId = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8) +
            (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int enemyXFrom = ((dataBufferDeliver[11]) & 0xFF) + (((dataBufferDeliver[12]) & 0xFF) << 8);
        int enemyYFrom = ((dataBufferDeliver[13]) & 0xFF) + (((dataBufferDeliver[14]) & 0xFF) << 8);

        //printf("009D - item_exists [%d bytes] (%d.) Nmae: %d (%d, %d)\n", packetLen, ItemId, ItemNameId, enemyXFrom, enemyYFrom);

        if (gAutoItemTakeSwitch || gAutoGreedSwitch) {
            if (ItemAidList.search_target(ItemId)) {
                ItemAidList.delete_target_node(ItemId);
            }
            ItemAidList.add_target_node(ItemId, ItemNameId, enemyXFrom, enemyYFrom, -1, 0, NULL);
        }

        //
        // 自動貪婪
        //
        if (((abs(*PLAYER_CORRD_X - enemyXFrom) * abs(*PLAYER_CORRD_X - enemyXFrom)) + (abs(*PLAYER_CORRD_Y - enemyYFrom) * abs(*PLAYER_CORRD_Y - enemyYFrom))) <= (5 * 5) && !IsExceptionPickUpItemOrNot(ItemNameId)) {
            //gDetectGreedTriggerFlag = 1;
        }
    }

    //
    // 00A1 - item_disappeared [4 bytes]
    //
    if (packetName == 0xA1) { // 偵測到物品

        packetLen = 4;

        // 座標
        int ItemId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8) +
            (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);

        //printf("00A1 - item_disappeared [4 bytes] (%d.)\n", ItemId);

        if (gAutoItemTakeSwitch || gAutoGreedSwitch) {
            if (ItemAidList.search_target(ItemId)) {
                ItemAidList.delete_target_node(ItemId);
            }
        }
    }


    //
    // 02E1 - ['actor_action', 'a4 a4 a4 V3 v C V', [qw(sourceID targetID tick src_speed dst_speed damage div type dual_wield_damage)]] [33 bytes]
    //
    // E1 02 46 07 00 00 39 25 AC 00 57 92 88 21 40 02
    // 00 00 90 01 00 00 00 00 00 00 01 00 00 00 00 00
    // 00
    if (packetName == ActorAction) { // 偵測到怪物攻擊

        //int newLineCount = 0;
        //for (int printCount = 0; printCount < len; printCount++) {
        //    printf("%02hhX ", dataBufferDeliver[printCount]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

        int SourceAid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8) +
            (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int TargetAid = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8) +
            (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int Damage = ((dataBufferDeliver[22]) & 0xFF) + (((dataBufferDeliver[23]) & 0xFF) << 8) +
            (((dataBufferDeliver[24]) & 0xFF) << 16) + (((dataBufferDeliver[25]) & 0xFF) << 24);
        int Div = ((dataBufferDeliver[26]) & 0xFF) + (((dataBufferDeliver[27]) & 0xFF) << 8);
        int Type = ((dataBufferDeliver[28]) & 0xFF) + (((dataBufferDeliver[29]) & 0xFF) << 8);
        //printf("02E1 - actor_action [%d bytes] %d 攻擊--> %d, 傷害: %d, Div: %d, Type: %d\n", len, SourceAid, TargetAid, Damage, Div, Type);

        if (SourceAid == *AidAddress) {
            gNearAttackAck = 1;
        }
                
        if (SourceAid == *AidAddress && Damage > 0) {
            gNearAttackAck = 1;
        }

        if (TargetAid == *AidAddress && Damage > 0){
            gDieNoteAid = SourceAid;
            gDieNoteDamage = Damage;
        }
    }

    //
    // '08C8' => ['actor_action', 'a4 a4 a4 V3 x v C V', [qw(sourceID targetID tick src_speed dst_speed damage div type dual_wield_damage)]],
    //
    if (packetName == 0x08C8) { // 偵測到怪物攻擊

        //int newLineCount = 0;
        //for (int printCount = 0; printCount < len; printCount++) {
        //    printf("%02hhX ", dataBufferDeliver[printCount]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

        int SourceAid = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8) +
            (((dataBufferDeliver[4]) & 0xFF) << 16) + (((dataBufferDeliver[5]) & 0xFF) << 24);
        int TargetAid = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8) +
            (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int Damage = ((dataBufferDeliver[22]) & 0xFF) + (((dataBufferDeliver[23]) & 0xFF) << 8) +
            (((dataBufferDeliver[24]) & 0xFF) << 16) + (((dataBufferDeliver[25]) & 0xFF) << 24);
        int Div = ((dataBufferDeliver[26]) & 0xFF) + (((dataBufferDeliver[27]) & 0xFF) << 8);
        int Type = ((dataBufferDeliver[28]) & 0xFF) + (((dataBufferDeliver[29]) & 0xFF) << 8);
        //printf("08C8 - actor_action [%d bytes] %d 攻擊--> %d, 傷害: %d, Div: %d, Type: %d\n", len, SourceAid, TargetAid, Damage, Div, Type);

        if (SourceAid == *AidAddress && Damage > 0) {
            gNearAttackAck = 1;

            // 魔物報告
            if (gExpReportSwitch) {
                // 標記魔物被你攻擊過
                TargetAidExpReportList.update_target_to_you_hit(TargetAid);
            }
        }

        if (TargetAid == *AidAddress && Damage > 0) {
            gDieNoteAid = SourceAid;
            gDieNoteDamage = Damage;
        }

    }

    //
    // 更新補品數量
    // 01C8 - Inventory Used [15 bytes]
    //
    if (packetName == InventoryUsed) {

        int TargetAid = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8) +
            (((dataBufferDeliver[10]) & 0xFF) << 16) + (((dataBufferDeliver[11]) & 0xFF) << 24);
        int TargetItemId = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
            (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);

        int IdTmp = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int ItemAmountTmp = ((dataBufferDeliver[12]) & 0xFF) + (((dataBufferDeliver[13]) & 0xFF) << 8);

        int IsIIA = 1;
        if (TargetAid <= 0 || TargetItemId <= 0 || IdTmp <= 0 || ItemAmountTmp < 0) {
            IsIIA = 0;
        }
        if (IsIIA) {

            //printf("01C8 - Inventory Used [15 bytes] %d 使用物品: %d\n", TargetAid, TargetItemId);

            //
            // 判斷是你本人使用補品
            //
            if (TargetAid == *AidAddress && ItemDataList.search_item(TargetItemId)) {
                if (ItemAmountTmp) {
                    // 數量大於0
                    ItemDataList.delete_item_node_Id(IdTmp);
                    ItemDataList.add_item_node(
                        0,
                        IdTmp,
                        TargetItemId,
                        ItemAmountTmp,
                        0, 0, 0, 0, 0, 0, 0
                    );
                }
                // 數量少於0
                else {
                    ItemDataList.delete_item_node_Id(IdTmp);
                }

                //printf("01C8 - Inventory Used [15 bytes] (ID: %d, 總數量: %d) \n", IdTmp, ItemAmountTmp);

                // 物品使用報告
                if (TargetAid == *AidAddress && gExpReportSwitch) {
                    if (!ItemReportList.search_item(TargetItemId)) { // 如果消耗品沒有在報告裡
                        ItemReportList.add_item_node(
                            0,
                            1,
                            TargetItemId,
                            -1,
                            0, 0, 0, 0, 0, 0, 0
                        );
                    }
                    else {
                        int ItemAmountOrg = ItemReportList.search_itemAmount(TargetItemId);
                        ItemAmountOrg--;
                        ItemReportList.update_item_amount(TargetItemId, ItemAmountOrg);
                    }
                }
            }

            
        }

    }
    //
    // 更新補品數量
    // 0B41 - Inventory Item Added [70 bytes]
    //
    if (packetName == InventoryItemAdded) {
        packetLen = 70;

        int IsIIA = 1;
        //for (int dataBufferDeliverCount = 32; dataBufferDeliverCount < 69; dataBufferDeliverCount++) {
        //    if (dataBufferDeliver[dataBufferDeliverCount] & 0xFF != 0) {
        //        IsIIA = 0;
        //        break;
        //    }
        //}
        // 物品編號必須大於0
        int ItemIdTmp = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8)
            + (((dataBufferDeliver[8]) & 0xFF) << 16) + (((dataBufferDeliver[9]) & 0xFF) << 24);
        int IdTmp = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int ItemAmountTmp = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8);
        int ItemAmountTmpBackUp = ItemAmountTmp;
        int ItemAmountBackUp = ItemDataList.search_itemAmount(ItemIdTmp);
        int EquipType = ((dataBufferDeliver[28]) & 0xFF) + (((dataBufferDeliver[29]) & 0xFF) << 8)
            + (((dataBufferDeliver[30]) & 0xFF) << 16) + (((dataBufferDeliver[31]) & 0xFF) << 24);
        if (ItemIdTmp <= 0 || IdTmp <= 0 || ItemAmountTmp < 0) {
            IsIIA = 0;
        }
        if (IsIIA) {
            if (EquipType) {

                int Refined = ((dataBufferDeliver[68]) & 0xFF);
                int Card01 = ((dataBufferDeliver[12]) & 0xFF) + (((dataBufferDeliver[13]) & 0xFF) << 8)
                    + (((dataBufferDeliver[14]) & 0xFF) << 16) + (((dataBufferDeliver[15]) & 0xFF) << 24);
                int Card02 = ((dataBufferDeliver[16]) & 0xFF) + (((dataBufferDeliver[17]) & 0xFF) << 8)
                    + (((dataBufferDeliver[18]) & 0xFF) << 16) + (((dataBufferDeliver[19]) & 0xFF) << 24);
                int Card03 = ((dataBufferDeliver[20]) & 0xFF) + (((dataBufferDeliver[21]) & 0xFF) << 8)
                    + (((dataBufferDeliver[22]) & 0xFF) << 16) + (((dataBufferDeliver[23]) & 0xFF) << 24);
                int Card04 = ((dataBufferDeliver[24]) & 0xFF) + (((dataBufferDeliver[25]) & 0xFF) << 8)
                    + (((dataBufferDeliver[26]) & 0xFF) << 16) + (((dataBufferDeliver[27]) & 0xFF) << 24);

                //printf("%04X - Inventory Item Added [%d bytes] (裝備) 部位:%d (%d) +%d %d [%d, %d, %d, %d]\n", 
                //    InventoryItemAdded, packetLen, EquipType, IdTmp, Refined, ItemIdTmp, Card01, Card02, Card03, Card04);

                ItemDataList.delete_item_node_Id(IdTmp);
                //printf("(ID: %d) \n", IdTmp);

                ItemDataList.add_item_node(
                    1,
                    IdTmp,
                    ItemIdTmp,
                    1,
                    Refined,
                    0,
                    EquipType,
                    Card01,
                    Card02,
                    Card03,
                    Card04
                );

                //int newLineCount = 0;
                //for (int printCount = 0; printCount < len; printCount++) {
                //    printf("%02hhX ", dataBufferDeliver[printCount]);
                //    newLineCount++;
                //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
                //}
                //printf("\n");

                // 物品報告
                if (gExpReportSwitch) {
                    if (!ItemReportList.search_item(ItemIdTmp)) { // 如果物品沒有在報告裡
                        ItemReportList.add_item_node(
                            0,
                            1,
                            ItemIdTmp,
                            1,
                            0, 0, 0, 0, 0, 0, 0
                        );
                    }
                    else {
                        int ItemAmountOrg = ItemReportList.search_itemAmount(ItemIdTmp);
                        ItemAmountOrg++;
                        ItemReportList.update_item_amount(ItemIdTmp, ItemAmountOrg);
                    }
                }

                gSteal = 1;
            }
            else {
                //printf("0A37 - Inventory Item Added [%d bytes] (物品) %d ", packetLen, ItemIdTmp);
                if (ItemDataList.search_item_Id(IdTmp)) { // 補品的ID
                    //printf("身上有此物品 ");

                    //ItemDataList.delete_item_node(IdTmp);
                    ItemAmountTmp += ItemAmountBackUp;
                }
                //
                // 身上無此物品ID 新增
                //
                else {
                    //printf("身上無此物品 ");

                    //ItemDataList.delete_item_node(IdTmp);
                }

                //if (ItemIdTmp == 11742) {
                gSteal = 1;
                //}

                //printf("(ID: %d, 總數量: %d) \n", IdTmp, ItemAmountTmp);

                ItemDataList.delete_item_node_Id(IdTmp);
                ItemDataList.add_item_node(
                    0,
                    IdTmp,
                    ItemIdTmp,
                    ItemAmountTmp,
                    0, 0, 0, 0, 0, 0, 0
                );

                // 物品報告
                if (gExpReportSwitch) {
                    if (!ItemReportList.search_item(ItemIdTmp)) { // 如果物品沒有在報告裡
                        ItemReportList.add_item_node(
                            0,
                            1,
                            ItemIdTmp,
                            ItemAmountTmpBackUp,
                            0, 0, 0, 0, 0, 0, 0
                        );
                    }
                    else {
                        int ItemAmountOrg = ItemReportList.search_itemAmount(ItemIdTmp);
                        ItemAmountOrg += ItemAmountTmpBackUp;
                        ItemReportList.update_item_amount(ItemIdTmp, ItemAmountOrg);
                    }
                }
            }
        }
    }


    //
    // 更新補品數量
    // 07FA - Inventory Item RemoveD
    //
    if (packetName == InventoryItemRemoveD) {

        int Reason = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int Id = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8);
        int Amount = ((dataBufferDeliver[6]) & 0xFF) + (((dataBufferDeliver[7]) & 0xFF) << 8);

        //printf("Inventory Item RemoveD %d (%d) 減少: %d\n", Reason, Id, Amount);

        int ItemIdTmp = ItemDataList.search_item_Id(Id);
        if (ItemIdTmp) { // 尋找物品
            int itemAmount = ItemDataList.update_item_amount_add_delete_byId(Id, -Amount);
            // 如果數量小於1, 刪除物品節點
            if (itemAmount < 1) {
                ItemDataList.delete_item_node_Id(Id);
                //printf("物品 %d 從背包中移除\n",Id);
            }

            // 物品報告
            if (gExpReportSwitch) {
                if (!ItemReportList.search_item(ItemIdTmp)) { // 如果物品沒有在報告裡
                    ItemReportList.add_item_node(
                        0,
                        1,
                        ItemIdTmp,
                        -Amount,
                        0, 0, 0, 0, 0, 0, 0
                    );
                }
                else {
                    int ItemAmountOrg = ItemReportList.search_itemAmount(ItemIdTmp);
                    ItemAmountOrg -= Amount;
                    ItemReportList.update_item_amount(ItemIdTmp, ItemAmountOrg);
                }
            }
        }

        //int newLineCount = 0;
        //for (int printCount = 0; printCount < len; printCount++) {
        //    printf("%02hhX ", dataBufferDeliver[printCount]);
        //    newLineCount++;
        //    if (newLineCount % 16 == 0 && newLineCount != 0) printf("\n");
        //}
        //printf("\n");

    }
    //
    // 更新補品數量
    // 00AF - Inventory Removed
    //
    if (packetName == InventoryRemoved) {

        int Id = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
        int Amount = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8);

        //printf("Inventory Removed (%d) 減少: %d\n", Id, Amount);

        int ItemIdTmp = ItemDataList.search_item_Id(Id);
        if (ItemIdTmp) { // 尋找物品
            int itemAmount = ItemDataList.update_item_amount_add_delete_byId(Id, -Amount);
            // 如果數量小於1, 刪除物品節點
            if (itemAmount < 1) {
                ItemDataList.delete_item_node_Id(Id);
                //printf("物品 %d 從背包中移除\n", Id);
            }

            // 物品報告
            if (gExpReportSwitch) {
                if (!ItemReportList.search_item(ItemIdTmp)) { // 如果物品沒有在報告裡
                    ItemReportList.add_item_node(
                        0,
                        1,
                        ItemIdTmp,
                        -Amount,
                        0, 0, 0, 0, 0, 0, 0
                    );
                }
                else {
                    int ItemAmountOrg = ItemReportList.search_itemAmount(ItemIdTmp);
                    ItemAmountOrg -= Amount;
                    ItemReportList.update_item_amount(ItemIdTmp, ItemAmountOrg);
                }
            }
        }
    }

    ////
    //// ['equip_item', 'a2 V v C', [qw(ID type viewID success)]], #11
    ////
    //if (packetName == EquipItem) { // 偵測到穿裝

    //    packetLen = 11;

    //    int ItemId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
    //    int Type = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
    //        (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
    //    int ViewId = ((dataBufferDeliver[8]) & 0xFF) + (((dataBufferDeliver[9]) & 0xFF) << 8);
    //    int Success = ((dataBufferDeliver[10]) & 0xFF);

    //    if (ItemId && Success >= 0) {
    //        printf("0999 - equip_item [%d bytes]", packetLen);
    //        printf(" 穿上裝備:%d. type: %d viewID: %d 成功:%d\n", ItemId, Type, ViewId, Success);

    //        if (Success == 0 && Type) { // 成功
    //            ItemDataList.update_item_isEquip(ItemId, 1);
    //        }
    //        else {
    //            //ItemDataList.update_item_isEquip(ItemId, 0);
    //            ItemDataList.update_item_failCount_byId(0, ItemId);
    //        }
    //    }
    //}


    ////
    //// '099A' => ['unequip_item', 'a2 V C', [qw(ID type success)]],#9
    ////
    //if (packetName == UnequipItem) { // 偵測到脫裝

    //    packetLen = 9;

    //    // 座標
    //    int ItemId = ((dataBufferDeliver[2]) & 0xFF) + (((dataBufferDeliver[3]) & 0xFF) << 8);
    //    int Type = ((dataBufferDeliver[4]) & 0xFF) + (((dataBufferDeliver[5]) & 0xFF) << 8) +
    //        (((dataBufferDeliver[6]) & 0xFF) << 16) + (((dataBufferDeliver[7]) & 0xFF) << 24);
    //    int Success = ((dataBufferDeliver[8]) & 0xFF);
    //    if (ItemId && Success >= 0) {
    //        printf("099A - unequip_item [%d bytes]", packetLen);
    //        printf(" 卸下裝備:%d. type: %d 成功:%d\n", ItemId, Type, Success);

    //        if (Success == 0 && Type) { // 成功
    //            ItemDataList.update_item_isEquip(ItemId, 0);
    //        }
    //        else {
    //            ItemDataList.update_item_isEquip(ItemId, 0);
    //        }
    //    }
    //}

#if !PrivateServerOrNot
    //
    // '009A' => ['system_chat', 'v a*', [qw(len message)]]
    //
    if (packetName == SystemChat && 
        dataBufferDeliver[4] == 'm' && dataBufferDeliver[5] == 'i' && dataBufferDeliver[6] == 'c' && dataBufferDeliver[7] == 'c'
        && gCatchBroadCastSwitch) { // 廣播開關

        printf("廣播訊息: ");
        char BroadCastStr[48] = ".\\data\\log\\BroadCast_";
        char txt[5] = ".txt";
        strcat(BroadCastStr, gCharNameStr);
        strcat(BroadCastStr, txt);
        ofstream fout(BroadCastStr, ios::app);   // Open the file

        GetLocalTime(&sys);
        fout << sys.wYear << "/" << sys.wMonth << "/" << sys.wDay << " " << sys.wHour << ":" << sys.wMinute << ":" << sys.wSecond << " ";

        int PrintSystemChat = 0;
        string BroadCastMessage;
        int BroadCastCount = 0;
        for (int PrintCount = 4; PrintCount < len; PrintCount++) { // e7cdff
            if (dataBufferDeliver[PrintCount] == 'e' && dataBufferDeliver[PrintCount + 1] == '7' && dataBufferDeliver[PrintCount + 2] == 'c'
                && dataBufferDeliver[PrintCount + 3] == 'd' && dataBufferDeliver[PrintCount + 4] == 'f' && dataBufferDeliver[PrintCount + 5] == 'f') {
                PrintSystemChat = 1;
                PrintCount += 6;
            }

            if (PrintSystemChat) {
                BroadCastMessage += dataBufferDeliver[PrintCount];
                BroadCastCount++;
                //printf("%c", dataBufferDeliver[PrintCount]);
                //fout << dataBufferDeliver[PrintCount];
            }
        }
        if (PrintSystemChat) {
            // 轉換物品連結
            //printf("\n改前:%s\n", BroadCastMessage);
            solveMessage(&BroadCastMessage);
            cout << BroadCastMessage;
            //printf("%s", BroadCastMessage);
            fout << BroadCastMessage;
        }

        printf("\n");
        fout << endl;
        fout.close();
    }
#endif



    EndForLoop:


    return EXIT_SUCCESS;
}

int PacketMatchOrNot(char* buffer, int Count, int Packet) {
    if ((buffer[Count] & 0xFF) == (Packet & 0xFF) && (buffer[Count + 1] & 0xFF) == ((Packet & 0xFF00) >> 8)) {
        return 1;
    }
    return 0;
}

int IsYourAidOrNot(char* buffer, int Count) {
    if ((buffer[Count] & 0xFF) == (*AidAddress & 0xFF) && (buffer[Count + 1] & 0xFF) == ((*AidAddress & 0xFF00) >> 8)
        && (buffer[Count + 2] & 0xFF) == ((*AidAddress & 0xFF0000) >> 16) && (buffer[Count + 3] & 0xFF) == ((*AidAddress & 0xFF000000) >> 24) ) {
        return 1;
    }

    return 0;
}

int IsGuidMember(char* buffer, int Count) {
    int GuildId = ((buffer[Count]) & 0xFF) + (((buffer[Count + 1]) & 0xFF) << 8) +
        (((buffer[Count + 2]) & 0xFF) << 16) + (((buffer[Count + 3]) & 0xFF) << 24);

    if (GuildId) {

        if (GuildId == gGuidId01 ||
            GuildId == gGuidId02 ||
            GuildId == gGuidId03 ||
            GuildId == gGuidId04 ||
            GuildId == gGuidId05 ) {
            return 1;
        }
#if !PrivateServerOrNot
        if (*GUILD_CURRENT && *GUILD_CURRENT == GuildId) {
            return 1;
        }
#endif
        for (int i = 0; i < 6; i++) {
            if (gGuildIdCatch[i] && gGuildIdCatch[i] == GuildId) {
                return 1;
            }
        }
    
    }

    return 0;
}

int IsPvpForbiddenMap(void) {

    if (!PVP_FORBIDDEN) {
        return 0;
    }

    if (*AidAddress == 12100031) {
        return 0;
    }

    char CURRENT_MAP_NAME[16] = { "" };

    int charCompareCount = 0;
    int caclcStringLen = 0;

    CURRENT_MAP_NAME[0] = (*MAP_NAME & 0xFF);
    CURRENT_MAP_NAME[1] = ((*MAP_NAME & 0xFF00) >> 8);
    CURRENT_MAP_NAME[2] = ((*MAP_NAME & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[3] = ((*MAP_NAME & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[4] = (*(MAP_NAME + 1) & 0xFF);
    CURRENT_MAP_NAME[5] = ((*(MAP_NAME + 1) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[6] = ((*(MAP_NAME + 1) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[7] = ((*(MAP_NAME + 1) & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[8] = (*(MAP_NAME + 2) & 0xFF);
    CURRENT_MAP_NAME[9] = ((*(MAP_NAME + 2) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[10] = ((*(MAP_NAME + 2) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[11] = ((*(MAP_NAME + 2) & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[12] = (*(MAP_NAME + 3) & 0xFF);
    CURRENT_MAP_NAME[13] = ((*(MAP_NAME + 3) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[14] = ((*(MAP_NAME + 3) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[15] = ((*(MAP_NAME + 3) & 0xFF000000) >> 24);



    for (int charCount = 0; charCount < PVP_FORBIDDEN_MAP_COUNT; charCount++) {

        // 計算PVP圖的字串長度 遇到.就停
        for (caclcStringLen = 0; caclcStringLen < 16; caclcStringLen++) {
            if (gPvpForbiddenMapDataTable[charCount][caclcStringLen] == '.') {
                //printf("caclcStringLen: %d\n", caclcStringLen);
                break;
            }
        }
        // 比較PVP地圖的字串
        for (charCompareCount = 0; charCompareCount < caclcStringLen; charCompareCount++) {
            if (gPvpForbiddenMapDataTable[charCount][charCompareCount] != CURRENT_MAP_NAME[charCompareCount]) {
                break;
            }
        }
        //printf("charCompareCount: %d\n", charCompareCount);
        if (charCompareCount == caclcStringLen) {
            //printf("此為PVP地圖\n");
            return 1;
        }
    }
    //printf("非PVP地圖\n");
    return PvpOption;

}


int IsPvpMap(void) {
    char CURRENT_MAP_NAME[16] = { "" };

    int charCompareCount = 0;
    int caclcStringLen = 0;

    if (gNearAttackMonster) {
        return 1;
    }

    CURRENT_MAP_NAME[0] = (*MAP_NAME & 0xFF);
    CURRENT_MAP_NAME[1] = ((*MAP_NAME & 0xFF00) >> 8);
    CURRENT_MAP_NAME[2] = ((*MAP_NAME & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[3] = ((*MAP_NAME & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[4] = (*(MAP_NAME + 1) & 0xFF);
    CURRENT_MAP_NAME[5] = ((*(MAP_NAME + 1) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[6] = ((*(MAP_NAME + 1) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[7] = ((*(MAP_NAME + 1) & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[8] = (*(MAP_NAME + 2) & 0xFF);
    CURRENT_MAP_NAME[9] = ((*(MAP_NAME + 2) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[10] = ((*(MAP_NAME + 2) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[11] = ((*(MAP_NAME + 2) & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[12] = (*(MAP_NAME + 3) & 0xFF);
    CURRENT_MAP_NAME[13] = ((*(MAP_NAME + 3) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[14] = ((*(MAP_NAME + 3) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[15] = ((*(MAP_NAME + 3) & 0xFF000000) >> 24);

    for (int charCount = 0; charCount < PVP_MAP_COUNT; charCount++) {

        // 計算PVP圖的字串長度 遇到.就停
        for (caclcStringLen = 0; caclcStringLen < 16; caclcStringLen++) {
            if (gPvpMapDataTable[charCount][caclcStringLen] == '.') {
                //printf("caclcStringLen: %d\n", caclcStringLen);
                break;
            }
        }
        // 比較PVP地圖的字串
        for (charCompareCount = 0; charCompareCount < caclcStringLen; charCompareCount++) {
            if (gPvpMapDataTable[charCount][charCompareCount] != CURRENT_MAP_NAME[charCompareCount]) {
                break;
            }
        }
        //printf("charCompareCount: %d\n", charCompareCount);
        if (charCompareCount == caclcStringLen) {
            //printf("此為PVP地圖\n");
            return 1;
        }
    }
    //printf("非PVP地圖\n");
    return PvpOption;

}

int IsTeMap(void) {
    char CURRENT_MAP_NAME[16] = { "" };

    int charCompareCount = 0;
    int caclcStringLen = 0;

    CURRENT_MAP_NAME[0] = (*MAP_NAME & 0xFF);
    CURRENT_MAP_NAME[1] = ((*MAP_NAME & 0xFF00) >> 8);
    CURRENT_MAP_NAME[2] = ((*MAP_NAME & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[3] = ((*MAP_NAME & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[4] = (*(MAP_NAME + 1) & 0xFF);
    CURRENT_MAP_NAME[5] = ((*(MAP_NAME + 1) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[6] = ((*(MAP_NAME + 1) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[7] = ((*(MAP_NAME + 1) & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[8] = (*(MAP_NAME + 2) & 0xFF);
    CURRENT_MAP_NAME[9] = ((*(MAP_NAME + 2) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[10] = ((*(MAP_NAME + 2) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[11] = ((*(MAP_NAME + 2) & 0xFF000000) >> 24);
    CURRENT_MAP_NAME[12] = (*(MAP_NAME + 3) & 0xFF);
    CURRENT_MAP_NAME[13] = ((*(MAP_NAME + 3) & 0xFF00) >> 8);
    CURRENT_MAP_NAME[14] = ((*(MAP_NAME + 3) & 0xFF0000) >> 16);
    CURRENT_MAP_NAME[15] = ((*(MAP_NAME + 3) & 0xFF000000) >> 24);

    for (int charCount = 0; charCount < TE_MAP_COUNT; charCount++) {

        // 計算PVP圖的字串長度 遇到.就停
        for (caclcStringLen = 0; caclcStringLen < 16; caclcStringLen++) {
            if (gTeMapDataTable[charCount][caclcStringLen] == '.') {
                //printf("caclcStringLen: %d\n", caclcStringLen);
                break;
            }
        }
        // 比較PVP地圖的字串
        for (charCompareCount = 0; charCompareCount < caclcStringLen; charCompareCount++) {
            if (gTeMapDataTable[charCount][charCompareCount] != CURRENT_MAP_NAME[charCompareCount]) {
                break;
            }
        }
        //printf("charCompareCount: %d\n", charCompareCount);
        if (charCompareCount == caclcStringLen) {
            //printf("此為TE地圖\n");
            return 1;
        }
    }
    //printf("非TE地圖\n");
    return 0;
}

int IsGmAid(char* buffer, int Count) {
#if !PrivateServerOrNot
    #define GM_LIST_COUNT 12
    int GmAidTable[GM_LIST_COUNT]{
        23769077,
        23769078,
        23769079,
        23769080,
        23769081,
        24414416,
        24414417,
        24414418,
        23769076,
        23769082,
        23769083,
        23769085
    };
    for (int compareGmCount = 0; compareGmCount < GM_LIST_COUNT; compareGmCount++) {
        if ((buffer[Count] & 0xFF) == (GmAidTable[compareGmCount] & 0xFF) && (buffer[Count + 1] & 0xFF) == ((GmAidTable[compareGmCount] & 0xFF00) >> 8)
            && (buffer[Count + 2] & 0xFF) == ((GmAidTable[compareGmCount] & 0xFF0000) >> 16) && (buffer[Count + 3] & 0xFF) == ((GmAidTable[compareGmCount] & 0xFF000000) >> 24)) {
            debug("發現GM");

            //讀取ini
            CString SettingBuffer;
            int SettingBufferInt = 64;
            //LPCTSTR path = _T(".\\data\\setting.txt");
            //LPCTSTR charPath = _T(".\\data\\charNameList.txt");

            ::WritePrivateProfileString(gCharNameStr, _T("TOTAL_SWITCH"), _T("0"), path); // 內掛總開關
            gTotalSwitch = 0;
            MessageBox(NULL, "發現GM.", NULL, MB_ICONEXCLAMATION);
            //exit(0);
            return 1;
        }
    }
    //debug ("沒有發現GM");

    return 0;
#else
    return 0;
#endif
}

int IsSpecialAid(char* buffer, int Count) {

    for (int compareGmCount = 0; compareGmCount < 10; compareGmCount++) {
        if ((buffer[Count] & 0xFF) == (gNearSupportAidTable[compareGmCount] & 0xFF) && (buffer[Count + 1] & 0xFF) == ((gNearSupportAidTable[compareGmCount] & 0xFF00) >> 8)
            && (buffer[Count + 2] & 0xFF) == ((gNearSupportAidTable[compareGmCount] & 0xFF0000) >> 16) && (buffer[Count + 3] & 0xFF) == ((gNearSupportAidTable[compareGmCount] & 0xFF000000) >> 24)) {
            return 1;
        }
    }

    return 0;
}

int IsSpecialAid2(int Aid) {

    for (int compareGmCount = 0; compareGmCount < 10; compareGmCount++) {
        if (Aid == gNearSupportAidTable[compareGmCount] && Aid) {
            return 1;
        }
    }

    return 0;
}

int IsEnemyAid(char* buffer, int Count) {

    for (int compareGmCount = 0; compareGmCount < 10; compareGmCount++) {
        if ((buffer[Count] & 0xFF) == (gNearEnemyAidTable[compareGmCount] & 0xFF) && (buffer[Count + 1] & 0xFF) == ((gNearEnemyAidTable[compareGmCount] & 0xFF00) >> 8)
            && (buffer[Count + 2] & 0xFF) == ((gNearEnemyAidTable[compareGmCount] & 0xFF0000) >> 16) && (buffer[Count + 3] & 0xFF) == ((gNearEnemyAidTable[compareGmCount] & 0xFF000000) >> 24)) {
            return 1;
        }
    }

    return 0;
}

int IsEnemyAid2(int Aid) {

    for (int compareGmCount = 0; compareGmCount < 10; compareGmCount++) {
        if (Aid == gNearEnemyAidTable[compareGmCount] && Aid) {
            return 1;
        }
    }

    return 0;
}

// 計算物件移動座標
BOOL CalculateObjectXYFunction() {

    while (1) {
        TargetAidList.update_XY_by_Speed_100ms();

        TargetAidExpReportList.update_XY_by_Speed_100ms();

        Sleep(100);
    }
    return EXIT_SUCCESS;
}

BOOL AutoCastEquipFunction() {
    MEMORY_BASIC_INFORMATION mbi;
    int* SMART_CASEING = 0;
    int SMART_CASEING_STATUS = 0;

    while (1) {
        int SkillIdTmp = 0;
        int* SKILL_ID = 0;
        int SkillId = 0;
        if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
            if (mbi.Protect == PAGE_READWRITE) {
                SKILL_ID = (int*)(*WINDOWS_LOCK_1 + 0x400);
            }
        }
        if (VirtualQuery((LPCVOID)SKILL_ID, &mbi, sizeof(mbi))) {
            if (mbi.Protect == PAGE_READWRITE) {
                SkillIdTmp = *SKILL_ID;
                //printf("SkillIdTmp: %d\n", SkillIdTmp);
            }
        }

        // 偵測智能切裝
        int Index = 0;
        if ((SkillIdTmp == gAutoCastEquipSkill[2] || gAutoCastEquipSkill[2] == -100) && gAutoCastEquipSwitch[2] && !gCastEquipTrigger[2]) {
            Index = 2;
            AutoCastEquipStartFunction(Index, 1);
            gCastEquipTrigger[Index] = 1;
            printf("偵測使用特定技能切裝%d\n", Index);
        }
        else if ((SkillIdTmp == gAutoCastEquipSkill[3] || gAutoCastEquipSkill[3] == -100) && gAutoCastEquipSwitch[3] && !gCastEquipTrigger[3]) {
            Index = 3;
            AutoCastEquipStartFunction(Index, 1);
            gCastEquipTrigger[Index] = 1;
            printf("偵測使用特定技能切裝%d\n", Index);

        }
        else if ((SkillIdTmp == gAutoCastEquipSkill[4] || gAutoCastEquipSkill[4] == -100) && gAutoCastEquipSwitch[4] && !gCastEquipTrigger[4]) {
            Index = 4;
            AutoCastEquipStartFunction(Index, 1);
            gCastEquipTrigger[Index] = 1;
            printf("偵測使用特定技能切裝%d\n", Index);

        }
        else if ((SkillIdTmp == gAutoCastEquipSkill[5] || gAutoCastEquipSkill[5] == -100) && gAutoCastEquipSwitch[5] && !gCastEquipTrigger[5]) {
            Index = 5;
            AutoCastEquipStartFunction(Index, 1);
            gCastEquipTrigger[Index] = 1;
            printf("偵測使用特定技能切裝%d\n", Index);

        }
        else if ((SkillIdTmp == gAutoCastEquipSkill[6] || gAutoCastEquipSkill[6] == -100) && gAutoCastEquipSwitch[6] && !gCastEquipTrigger[6]) {
            Index = 6;
            AutoCastEquipStartFunction(Index, 1);
            gCastEquipTrigger[Index] = 1;
            printf("偵測使用特定技能切裝%d\n", Index);
        }
        else if ((SkillIdTmp == gAutoCastEquipSkill[0] || gAutoCastEquipSkill[0] == -100) && gAutoCastEquipSwitch[0] && !gCastEquipTrigger[0]) {
            Index = 0;
            gCastEquipTrigger[Index] = 1;
            printf("偵測使用特定技能切裝%d\n", Index);
        }
        else if ((SkillIdTmp == gAutoCastEquipSkill[1] || gAutoCastEquipSkill[1] == -100) && gAutoCastEquipSwitch[1] && !gCastEquipTrigger[1]) {
            Index = 1;
            gCastEquipTrigger[Index] = 1;
            printf("偵測使用特定技能切裝%d\n", Index);
        }

        // 偵測羅剎
        if (!gSmartBloodTrigger && SkillIdTmp == 2343 && gSmartBloodSwitch) {
            printf("偵測使用羅剎\n");
            gSmartBloodTrigger = 1;
            //SmartBloodFunction();
        }


        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL SmartCastingSkill() {
    MEMORY_BASIC_INFORMATION mbi;
    int* SMART_CASEING = 0;
    int SMART_CASEING_STATUS = 0;
    while (1) {
        if (gSmartCastingAuto && gTotalSwitch) {
            if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    SMART_CASEING = (int*)(*WINDOWS_LOCK_1 + 80);
                }
            }
            if (VirtualQuery((LPCVOID)SMART_CASEING, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    SMART_CASEING_STATUS = *SMART_CASEING;
                }
            }
            if (SMART_CASEING_STATUS == 10 || SMART_CASEING_STATUS == 11) { // 當技能圈圈的時候 10/11  刀圖: 5
                printf("偵測使用智能師法\n");
                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                Sleep(10);
                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
            }
        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL SmartCastingSkillKeyboardSupport() {
    MEMORY_BASIC_INFORMATION mbi;
    int* SMART_CASEING = 0;
    int SMART_CASEING_STATUS = 0;
    while (1) {
        if (!gSmartCastingAuto && gTotalSwitch && IsRoFocus() &&
            (GetKeyState(gSmartCastingScanCode1) & 0x8000 || GetKeyState(gSmartCastingScanCode2) & 0x8000 || 
             GetKeyState(gSmartCastingScanCode3) & 0x8000 || GetKeyState(gSmartCastingScanCode4) & 0x8000 || 
             GetKeyState(gSmartCastingScanCode5) & 0x8000 || GetKeyState(gSmartCastingScanCode6) & 0x8000))
        {
            if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    SMART_CASEING = (int*)(*WINDOWS_LOCK_1 + 80);
                }
            }
            if (VirtualQuery((LPCVOID)SMART_CASEING, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    SMART_CASEING_STATUS = *SMART_CASEING;
                }
            }
            if (SMART_CASEING_STATUS == 10 || SMART_CASEING_STATUS == 11) { // 當技能圈圈的時候 10/11  刀圖: 5
                printf("偵測使用智能師法\n");
                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                Sleep(10);
                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
            }
        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL SmartCastingSkillKeyboard() {
    int* SMART_CASEING = 0;
    int SMART_CASEING_STATUS = 0;
    MEMORY_BASIC_INFORMATION mbi;
    //int ScanCode = 118;
    while (1) {
        if (gTotalSwitch) {
            if (gSmartCastingScanCode1) {
                if (IsRoFocus() && GetKeyState(gSmartCastingScanCode1)) {
                    //printf("偵測到按按鍵: %d\n", ScanCode);
                    //Sleep(100);
                    // 確定按住按鍵
                    if (IsRoFocus() && GetKeyState(gSmartCastingScanCode1) & 0x8000 && gTotalSwitch) {
                        printf("確定有下壓按鍵: %d\n", gSmartCastingScanCode1);
                        while (1) {
                            SendMessage(gHwnd, WM_KEYDOWN, gSmartCastingScanCode1, 0);
                            Sleep(50);

                            if (!(GetKeyState(gSmartCastingScanCode1) & 0x8000)) {
                                break;
                            }
                        }
                    }
                }
            }

            if (gSmartCastingScanCode2) {
                if (IsRoFocus() && GetKeyState(gSmartCastingScanCode2)) {
                    //printf("偵測到按按鍵: %d\n", ScanCode);
                    //Sleep(100);
                    // 確定按住按鍵
                    if (IsRoFocus() && GetKeyState(gSmartCastingScanCode2) & 0x8000 && gTotalSwitch) {
                        printf("確定有下壓按鍵: %d\n", gSmartCastingScanCode2);
                        while (1) {
                            SendMessage(gHwnd, WM_KEYDOWN, gSmartCastingScanCode2, 0);
                            Sleep(50);

                            if (!(GetKeyState(gSmartCastingScanCode2) & 0x8000)) {
                                break;
                            }
                        }
                    }
                }
            }

            if (gSmartCastingScanCode3) {
                if (IsRoFocus() && GetKeyState(gSmartCastingScanCode3)) {
                    //printf("偵測到按按鍵: %d\n", ScanCode);
                    //Sleep(100);
                    // 確定按住按鍵
                    if (IsRoFocus() && GetKeyState(gSmartCastingScanCode3) & 0x8000 && gTotalSwitch) {
                        printf("確定有下壓按鍵: %d\n", gSmartCastingScanCode3);
                        while (1) {
                            SendMessage(gHwnd, WM_KEYDOWN, gSmartCastingScanCode3, 0);
                            Sleep(50);

                            if (!(GetKeyState(gSmartCastingScanCode3) & 0x8000)) {
                                break;
                            }
                        }
                    }
                }
            }

            if (gSmartCastingScanCode4) {
                if (IsRoFocus() && GetKeyState(gSmartCastingScanCode4)) {
                    //printf("偵測到按按鍵: %d\n", ScanCode);
                    //Sleep(100);
                    // 確定按住按鍵
                    if (IsRoFocus() && GetKeyState(gSmartCastingScanCode4) & 0x8000 && gTotalSwitch) {
                        printf("確定有下壓按鍵: %d\n", gSmartCastingScanCode4);
                        while (1) {
                            SendMessage(gHwnd, WM_KEYDOWN, gSmartCastingScanCode4, 0);
                            Sleep(50);

                            if (!(GetKeyState(gSmartCastingScanCode4) & 0x8000)) {
                                break;
                            }
                        }
                    }
                }
            }

            if (gSmartCastingScanCode5) {
                if (IsRoFocus() && GetKeyState(gSmartCastingScanCode5)) {
                    //printf("偵測到按按鍵: %d\n", ScanCode);
                    //Sleep(100);
                    // 確定按住按鍵
                    if (IsRoFocus() && GetKeyState(gSmartCastingScanCode5) & 0x8000 && gTotalSwitch) {
                        printf("確定有下壓按鍵: %d\n", gSmartCastingScanCode5);
                        while (1) {
                            SendMessage(gHwnd, WM_KEYDOWN, gSmartCastingScanCode5, 0);
                            Sleep(50);

                            if (!(GetKeyState(gSmartCastingScanCode5) & 0x8000)) {
                                break;
                            }
                        }
                    }
                }
            }

            if (gSmartCastingScanCode6) {
                if (IsRoFocus() && GetKeyState(gSmartCastingScanCode6)) {
                    //printf("偵測到按按鍵: %d\n", ScanCode);
                    //Sleep(100);
                    // 確定按住按鍵
                    if (IsRoFocus() && GetKeyState(gSmartCastingScanCode6) & 0x8000 && gTotalSwitch) {
                        printf("確定有下壓按鍵: %d\n", gSmartCastingScanCode6);
                        while (1) {
                            SendMessage(gHwnd, WM_KEYDOWN, gSmartCastingScanCode6, 0);
                            Sleep(50);

                            if (!(GetKeyState(gSmartCastingScanCode6) & 0x8000)) {
                                break;
                            }
                        }
                    }
                }
            }
        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL ModifyMap() {

    int* GmModeMemoryRegionValue = NULL;
    MEMORY_BASIC_INFORMATION mbi;

    int RunOnceGmBackUp = 0;
    DWORD dummy;
#if !PrivateServerOrNot
    //
    // 改權限
    //
    //VirtualProtect(
    //    (LPVOID)EMB_FILE_PATH_INDEX_1,
    //    (sizeof(int) * 4),
    //    PAGE_EXECUTE_READWRITE,
    //    &dummy
    //);
    //VirtualProtect(
    //    (LPVOID)EMB_FILE_PATH_INDEX_2,
    //    (sizeof(int) * 4),
    //    PAGE_EXECUTE_READWRITE,
    //    &dummy
    //);

    // 測試改善跳窗
    if (gModifyMapSwitch) {
        printf("改地圖\n");
        *(DAT_FILE_PATH_INDEX + 1) = 0x00005C32;

        //*EMB_FILE_PATH_INDEX_1 = 0x61746164;
        //*(EMB_FILE_PATH_INDEX_1 + 1) = 0x626D455C;
        //*(EMB_FILE_PATH_INDEX_1 + 2) = 0x005C656C;
        //
        //*EMB_FILE_PATH_INDEX_2 = 0x6C626D45;
        //*(EMB_FILE_PATH_INDEX_2 + 1) = 0x6C005C65;
        //*(EMB_FILE_PATH_INDEX_2 + 2) = 0x005C6D65;
    }

    while (1) {
        if (1) {
            if (gModifyMapSwitch) {
                *(DAT_FILE_PATH_INDEX + 1) = 0x00005C32;

                //*EMB_FILE_PATH_INDEX_1 = 0x61746164;
                //*(EMB_FILE_PATH_INDEX_1 + 1) = 0x626D455C;
                //*(EMB_FILE_PATH_INDEX_1 + 2) = 0x005C656C;
                //
                //*EMB_FILE_PATH_INDEX_2 = 0x6C626D45;
                //*(EMB_FILE_PATH_INDEX_2 + 1) = 0x6C005C65;
                //*(EMB_FILE_PATH_INDEX_2 + 2) = 0x005C6D65;
            }
            else {
                *(DAT_FILE_PATH_INDEX + 1) = 0x0000005C;

                //*EMB_FILE_PATH_INDEX_1 = 0x706D745F;
                //*(EMB_FILE_PATH_INDEX_1 + 1) = 0x6C626D45;
                //*(EMB_FILE_PATH_INDEX_1 + 2) = 0x005C6D65;
                //
                //*EMB_FILE_PATH_INDEX_2 = 0x706D745F;
                //*(EMB_FILE_PATH_INDEX_2 + 1) = 0x6C626D45;
                //*(EMB_FILE_PATH_INDEX_2 + 2) = 0x005C6D65;
            }
        }

        //
        // GM模式
        //
        if (*AidAddress) {
            //
            // 備份GM名單
            //
            if (!RunOnceGmBackUp) {
                printf("GM備份名單:\n");
                for (int i = 0; i < 10; i++) {
                    GmModeMemoryRegionValue = (int*)(*GM_AID_ADDRESS + 0x4 + (4 * i));
                    if (VirtualQuery((LPCVOID)GmModeMemoryRegionValue, &mbi, sizeof(mbi))) {
                        if (mbi.Protect == PAGE_READWRITE) {
                            gGmModeBackUpValue[i] = *GmModeMemoryRegionValue;
                            printf("%d\n", gGmModeBackUpValue[i]);
                        }
                    }
                }
                RunOnceGmBackUp = 1;
            }

            for (int i = 0; i < 10; i++) {
                GmModeMemoryRegionValue = (int*)(*GM_AID_ADDRESS + 0x4 + (4 * i));
                if (VirtualQuery((LPCVOID)GmModeMemoryRegionValue, &mbi, sizeof(mbi))) {
                    if (mbi.Protect == PAGE_READWRITE) {
                        // 開
                        if (gGmModeSwitch && gGmModeAid[i]) {
                            *GmModeMemoryRegionValue = gGmModeAid[i];
                        }
                        // 關
                        else if (!gGmModeSwitch) {
                            *GmModeMemoryRegionValue = gGmModeBackUpValue[i];
                        }
                    }
                }
            }
        }

        Sleep(1000);

        //// 測試
        //HWND ForgeHwnd = GetForegroundWindow();
        //printf("ForgeHwnd: %d, gHwnd: %d\n", ForgeHwnd, gHwnd);

    }
#else
    while (1) {
        //
        // GM模式
        //
        if (*AidAddress) {
            //
            // 備份GM名單
            //
            if (!RunOnceGmBackUp) {
                printf("GM備份名單:\n");
                for (int i = 0; i < 10; i++) {
                    GmModeMemoryRegionValue = (int*)(*GM_AID_ADDRESS + (4 * i));
                    if (VirtualQuery((LPCVOID)GmModeMemoryRegionValue, &mbi, sizeof(mbi))) {
                        if (mbi.Protect == PAGE_READWRITE) {
                            gGmModeBackUpValue[i] = *GmModeMemoryRegionValue;
                            printf("%d\n", gGmModeBackUpValue[i]);
                        }
                    }
                }
                RunOnceGmBackUp = 1;
            }

            for (int i = 0; i < 10; i++) {
                GmModeMemoryRegionValue = (int*)(*GM_AID_ADDRESS + (4 * i));
                if (VirtualQuery((LPCVOID)GmModeMemoryRegionValue, &mbi, sizeof(mbi))) {
                    if (mbi.Protect == PAGE_READWRITE) {
                        // 開
                        if (gGmModeSwitch && gGmModeAid[i]) {
                            *GmModeMemoryRegionValue = gGmModeAid[i];
                        }
                        // 關
                        else if (!gGmModeSwitch) {
                            *GmModeMemoryRegionValue = gGmModeBackUpValue[i];
                        }
                    }
                }
            }
        }
        Sleep(1000);
    }
#endif
    return EXIT_SUCCESS;
}

int IsRoFocus(void) {
    HWND ForgeHwnd = GetForegroundWindow();
    //printf("ForgeHwnd: %d, gHwnd: %d\n", ForgeHwnd, gHwnd);
    if (gHwnd == ForgeHwnd) {
        return 1;
    }

    return 0;
}

BOOL CpuSpeedFunction() {
#if !PrivateServerOrNot
    int CpuCoreBackUp = 0;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    printf("now system cpu num is %d\n", sysInfo.dwNumberOfProcessors);
    gCpuSpeedCoreTotal = sysInfo.dwNumberOfProcessors;

    LPCTSTR charPath = _T(".\\data\\setting.txt");
    char CpuSpeedCoreTotalText[20];
    sprintf(CpuSpeedCoreTotalText, "%d", gCpuSpeedCoreTotal); // 轉化數字變成文字
    ::WritePrivateProfileString("COMMON", "CPU_SPEED_CORE_TOTAL", CpuSpeedCoreTotalText, charPath);

    while (1) {

        if (*AidAddress) {
            if (gCpuSpeedSwitch) {
                // 將RO指派CPU0 GetCurrentThread
                int Success = 0;
                // 計算core核心值
                int CpuSpeedCoreValue = 1;
                for (int i = 0; i < gCpuSpeedCore; i++) {
                    CpuSpeedCoreValue *= 2;
                }
                if (CpuCoreBackUp == 0) {
                    Success = SetThreadAffinityMask(GetCurrentThread(), CpuSpeedCoreValue);
                    CpuCoreBackUp = Success;

                    printf("將RO指派CPU0, Error code: %d, Success: %d.\n", GetLastError(), Success);
                }
            }
            else {
                if (CpuCoreBackUp) {
                    int Success = 0;
                    Success = SetThreadAffinityMask(GetCurrentThread(), CpuCoreBackUp);
                    CpuCoreBackUp = 0;

                    printf("將RO指派CPU還原, Error code: %d, Success: %d.\n", GetLastError(), Success);
                }

            }
        }

        Sleep(1000);
    }
#endif
    return EXIT_SUCCESS;
}

#if !PrivateServerOrNot
BOOL AutoWarpFunction() {
   
    while (1) {
        
        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW && CloakingTryNum >= 2 && !gStorageStatus) {
            
            for (int i = 0; i < 3; i++) {
                //if (gAutoWarpSwitch[i] && gAutoWarpCorrdX[i] && gAutoWarpCorrdY[i] && strcmp((const char*)gAutoWarpMap[i], "")) {
                if (gAutoWarpSwitch[i] && gAutoWarpCorrdX[i] && gAutoWarpCorrdY[i]) {
                    char WarpSkillBuffer[13] = { 0x0D, 0x00, 0xF4, 0x0A, 0x04, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

                    char WarpSkillBuffer2[22] = { 0x16, 0x00, 0x1B, 0x01, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                    // x, y
                    WarpSkillBuffer[AidSkillBufferOffset] = (gAutoWarpCorrdX[i] & 0xFF);
                    WarpSkillBuffer[AidSkillBufferOffset + 1] = ((gAutoWarpCorrdX[i] & 0xFF00) >> 8);
                    WarpSkillBuffer[AidSkillBufferOffset + 2] = (gAutoWarpCorrdY[i] & 0xFF);
                    WarpSkillBuffer[AidSkillBufferOffset + 3] = ((gAutoWarpCorrdY[i] & 0xFF00) >> 8);

                    SendAttackGroundPackageFunction(4, 0x1B, gAutoWarpCorrdX[i], gAutoWarpCorrdY[i]);

                    Sleep(2000);

                    //// WARP
                    //WarpSkillBuffer2[AidSkillBufferOffset - 2]  = (int)gAutoWarpMap[i][0];
                    //WarpSkillBuffer2[AidSkillBufferOffset - 1]  = (int)gAutoWarpMap[i][1];
                    //WarpSkillBuffer2[AidSkillBufferOffset - 0]  = (int)gAutoWarpMap[i][2];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 1]  = (int)gAutoWarpMap[i][3];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 2]  = (int)gAutoWarpMap[i][4];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 3]  = (int)gAutoWarpMap[i][5];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 4]  = (int)gAutoWarpMap[i][6];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 5]  = (int)gAutoWarpMap[i][7];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 6]  = (int)gAutoWarpMap[i][8];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 7]  = (int)gAutoWarpMap[i][9];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 8]  = (int)gAutoWarpMap[i][10];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 9]  = (int)gAutoWarpMap[i][11];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 10] = (int)gAutoWarpMap[i][12];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 11] = (int)gAutoWarpMap[i][13];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 12] = (int)gAutoWarpMap[i][14];
                    //WarpSkillBuffer2[AidSkillBufferOffset + 13] = (int)gAutoWarpMap[i][15];

                    //printf("測試數字: ");
                    //for (int j = 0; j < 16; j++) {
                    //    printf("%x ", (int)gAutoWarpMap[i][j]);
                    //}
                    //printf("\n");

                    //OriginalSend(roServer, WarpSkillBuffer2, 22 - OriginalSendOffset, 0);
                    for (int DownArrowCount = 0; DownArrowCount < gAutoWarpMap[i]; DownArrowCount++) {
                        SendMessage(gHwnd, WM_KEYDOWN, 40, 0); // Down Arrow	

                        Sleep(80);
                    }
                    SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // Enter

                    Sleep(gAutoWarpDelay);
                }
                
            }
            

        }

        Sleep(1000);
    }

    return EXIT_SUCCESS;
}
#endif

BOOL PcActionModifyFunction() {
    int PcActionValue1 = 0x90909090;
    int PcActionValue2 = 0xC7909090;

    int PcActionValueOrg1 = 0x009D86C6;
    int PcActionValueOrg2 = 0xC7000000;
    DWORD dummy;

    while (1) {
        //
        // 改人物動作後搖
        //
        if (gPcActionSwitch && gTotalSwitch) {

            MEMORY_BASIC_INFORMATION mbi;
            int* WINDOWS_LOCK_2 = NULL;
            int* WINDOWS_LOCK_3 = NULL;
            int* PCACTION_LOCK = NULL;

            if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    WINDOWS_LOCK_2 = (int*)(*WINDOWS_LOCK_1 + 0xCC);
                }
            }
            if (VirtualQuery((LPCVOID)WINDOWS_LOCK_2, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    WINDOWS_LOCK_3 = (int*)(*WINDOWS_LOCK_2 + 0x2C);
                }
            }
            if (VirtualQuery((LPCVOID)WINDOWS_LOCK_3, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    PCACTION_LOCK = (int*)(*WINDOWS_LOCK_3 + 0x9D);
                }
            }
            if (VirtualQuery((LPCVOID)PCACTION_LOCK, &mbi, sizeof(mbi))) {
                if (mbi.Protect == PAGE_READWRITE) {
                    //if (*PCACTION_LOCK & 0x1 == 0) {
                        *PCACTION_LOCK = *PCACTION_LOCK | 0x1;
                    //}
                }
            }
            //printf("PCACTION_LOCK = %x\n", (*WINDOWS_LOCK_3 + 0x9D));
            //printf("*PCACTION_LOCK = %x\n", *PCACTION_LOCK);

        //    if (*PC_ACTION_REGION_1 == PcActionValueOrg1) {
        //        //
        //        // 改權限
        //        //
        //        VirtualProtect(
        //            (LPVOID)PC_ACTION_REGION_1,
        //            (sizeof(int) * 2),
        //            PAGE_EXECUTE_READWRITE,
        //            &dummy
        //        );
        //        *PC_ACTION_REGION_1 = PcActionValue1;
        //        *PC_ACTION_REGION_2 = PcActionValue2;
        //        //
        //        // 將權限還原
        //        //
        //        VirtualProtect(
        //            (LPVOID)PC_ACTION_REGION_1,
        //            (sizeof(int) * 2),
        //            PAGE_EXECUTE_READ,
        //            &dummy
        //        );
        //    }
        //}
        //else {
        //    if (*PC_ACTION_REGION_1 == PcActionValue1) {
        //        //
        //        // 改權限
        //        //
        //        VirtualProtect(
        //            (LPVOID)PC_ACTION_REGION_1,
        //            (sizeof(int) * 2),
        //            PAGE_EXECUTE_READWRITE,
        //            &dummy
        //        );
        //        *PC_ACTION_REGION_1 = PcActionValueOrg1;
        //        *PC_ACTION_REGION_2 = PcActionValueOrg2;
        //        //
        //        // 將權限還原
        //        //
        //        VirtualProtect(
        //            (LPVOID)PC_ACTION_REGION_1,
        //            (sizeof(int) * 2),
        //            PAGE_EXECUTE_READ,
        //            &dummy
        //        );
        //    }
        //
        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}

void StorageProcessFunction(void) {

    vector<ItemControl> ItemControls;   // A place to store the list of students
    ItemControl Item;                  // A place to store data of one student
    int ItemTotalValue = 0;

    ifstream fin(".\\data\\storage.txt");   // Open the file

    while (fin >> Item.ItemId >> Item.Miminum >> Item.AutoStore >> Item.AutoSell) {
        //cout << Item.ItemId << " " << Item.Miminum <<
        //    " " << Item.AutoStore << " " << Item.AutoSell << endl;
        ItemTotalValue++;

        // 放東西進倉庫
        if (Item.ItemId && Item.AutoStore) {
            while (ItemDataList.search_item(Item.ItemId)) {
                int Id = ItemDataList.search_item(Item.ItemId); // 掃描要存倉的物品ID
                int ItemAmount = ItemDataList.search_itemAmount(Item.ItemId); // 掃描要存倉的物品數量
                if (Id && Id != NON_ITEM_AMOUNT && ItemAmount > 0 && ItemAmount != NON_ITEM_AMOUNT) { // 有找到物品

                    //
                    // >> '0364' => ['storage_item_add', 'a2 V', [qw(ID amount)]]
                    //
                    // >> '0365' => ['storage_item_remove', 'a2 V', [qw(ID amount)]],
                    //
                    char StorageItemAdd[10] = { 0x0A, 0x00, 0x64, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

                    StorageItemAdd[4] = Id & 0xFF;
                    StorageItemAdd[5] = (Id >> 8) & 0xFF;
                    // Amount
                    StorageItemAdd[6] = ItemAmount & 0xFF;
                    StorageItemAdd[7] = (ItemAmount & 0xFF00) >> 8;
                    StorageItemAdd[8] = (ItemAmount & 0xFF0000) >> 16;
                    StorageItemAdd[9] = (ItemAmount & 0xFF000000) >> 24;

                    Sleep(800);
                    OriginalSend(roServer, StorageItemAdd, 10 - OriginalSendOffset, 0);
                    printf("物品: %d ID: %d 數量: %d 已加入倉庫\n", Item.ItemId, Id, ItemAmount);

                    // 刪除物品節點
                    //ItemDataList.delete_item_node(StorageItemList[StorageItemCount]);
                    //ItemDataList.delete_item_node_Id(Id);
                }
            }
        }

        // 從倉庫取出物品
        if (Item.ItemId && Item.Miminum) {
            int ItemAmount = ItemDataList.search_itemAmount(Item.ItemId); // 掃描要存倉的物品數量
            int StorageItemAmount = StorageDataList.search_itemAmount(Item.ItemId); // 該物品倉庫的數量
            int GetItemAmount = Item.Miminum - ItemAmount; // 要拿的物品數量
            int StorageId = StorageDataList.search_item(Item.ItemId); // 該物品在倉庫的ID

            //printf("物品: %d ID: %d 數量: %d 已從倉庫取出\n", Item.ItemId, StorageId, GetItemAmount);
            if (StorageId > 0 && GetItemAmount > 0 && StorageItemAmount > 0) { // 符合拿取條件
                if (StorageItemAmount < GetItemAmount) {
                    GetItemAmount = StorageItemAmount;
                }
                //
                // >> '0364' => ['storage_item_add', 'a2 V', [qw(ID amount)]]
                //
                // >> '0365' => ['storage_item_remove', 'a2 V', [qw(ID amount)]],
                //
                char StorageItemRemove[10] = { 0x0A, 0x00, 0x65, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

                StorageItemRemove[4] = StorageId & 0xFF;
                StorageItemRemove[5] = (StorageId >> 8) & 0xFF;
                // Amount
                StorageItemRemove[6] = GetItemAmount & 0xFF;
                StorageItemRemove[7] = (GetItemAmount & 0xFF00) >> 8;
                StorageItemRemove[8] = (GetItemAmount & 0xFF0000) >> 16;
                StorageItemRemove[9] = (GetItemAmount & 0xFF000000) >> 24;

                Sleep(500);
                OriginalSend(roServer, StorageItemRemove, 10 - OriginalSendOffset, 0);
                printf("物品: %d ID: %d 數量: %d 已從倉庫取出\n", Item.ItemId, StorageId, GetItemAmount);

                // 刪除物品節點
                //StorageDataList.delete_item_node_Id(StorageId);
            }

        }

        ItemControls.push_back(Item);
    }
    fin.close();

    //for (int StorageItemCount = 0; StorageItemCount < StorageItemValue; StorageItemCount++) {
    //    while (ItemDataList.search_item(StorageItemList[StorageItemCount])) {
    //        int Id = ItemDataList.search_item(StorageItemList[StorageItemCount]); // 掃描要存倉的物品ID
    //        int ItemAmount = ItemDataList.search_itemAmount(StorageItemList[StorageItemCount]); // 掃描要存倉的物品數量
    //        if (Id && Id != NON_ITEM_AMOUNT && ItemAmount > 0 && ItemAmount != NON_ITEM_AMOUNT) { // 有找到物品
    //
    //            //
    //            // >> '0364' => ['storage_item_add', 'a2 V', [qw(ID amount)]]
    //            //
    //            char StorageItemAdd[10] = { 0x0A, 0x00, 0x64, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //
    //            StorageItemAdd[4] = Id & 0xFF;
    //            StorageItemAdd[5] = (Id >> 8) & 0xFF;
    //            // Amount
    //            StorageItemAdd[6] = ItemAmount & 0xFF;
    //            StorageItemAdd[7] = (ItemAmount & 0xFF00) >> 8;
    //            StorageItemAdd[8] = (ItemAmount & 0xFF0000) >> 16;
    //            StorageItemAdd[9] = (ItemAmount & 0xFF000000) >> 24;
    //
    //            Sleep(500);
    //            OriginalSend(roServer, StorageItemAdd, 10 - OriginalSendOffset, 0);
    //            printf("物品: %d ID: %d 數量: %d 已加入倉庫\n", StorageItemList[StorageItemCount], Id, ItemAmount);
    //
    //            // 刪除物品節點
    //            //ItemDataList.delete_item_node(StorageItemList[StorageItemCount]);
    //            ItemDataList.delete_item_node_Id(Id);
    //        }
    //    }
    //}
}

int MonsterGeneratorFunction(char* MonsterStr, int MonsterId) {

#if !PrivateServerOrNot
    if (!gExpReportSwitch) {
        return 0;
    }
#endif

    //printf("MonsterGeneratorFunction01\n");

    if (!strcmp (MonsterStr, "") || !strcmp(MonsterStr, " ") || !strcmp(MonsterStr, "  ")) {
        // 空字串不處理
        return 0;
    }
    //printf("MonsterGeneratorFunction02\n");
    ifstream fin(".\\data\\monsters.txt");    // Open the file

    size_t pos = 0;
    size_t pos2 = 0;
    string space_delimiter = " ";
    string newline_delimiter = "\0";
    vector<string> lines;
    string line;

    while (getline(fin, line)) {
        pos = line.find(space_delimiter);
        int MonsterIdRead = stoi(line.substr(0, pos));
        //printf("%d ", MonsterIdRead);

        pos2 = line.length();
        string MonsterStrRead = line.substr(pos + 1, pos2);
        //cout << MonsterStrRead << endl;

        if (MonsterIdRead == MonsterId) {
            fin.close();
            return 0;
        }

        lines.push_back(line);
    }
    fin.close();
    //printf("MonsterGeneratorFunction03\n");
    ofstream fout(".\\data\\monsters.txt", ios::app);   // Open the file

    fout << MonsterId << " " << MonsterStr << endl;

    fout.close();

    return 1;
}

int MonsterReadFunction(char* MonsterStr, int MonsterId) {

#if !PrivateServerOrNot
    return 0;
#endif

    //vector<MonsterControl> MonsterControls;   // A place to store the list of students
    //MonsterControl Monster;                   // A place to store data of one student
    //
    //ifstream fin(".\\data\\monsters.txt");    // Open the file
    //
    //while (fin >> Monster.MonsterId >> Monster.MonsterStr) {
    //
    //    if (Monster.MonsterId == MonsterId) {
    //        strcpy(MonsterStr, Monster.MonsterStr);
    //        fin.close();
    //        return 1;
    //    }
    //    MonsterControls.push_back(Monster);
    //}
    //fin.close();

    //printf("MonsterReadFunction01\n");
    ifstream fin(".\\data\\monsters.txt");    // Open the file

    size_t pos = 0;
    size_t pos2 = 0;
    string space_delimiter = " ";
    string newline_delimiter = "\0";
    vector<string> lines;
    string line;

    while (getline(fin, line)) {
        pos = line.find(space_delimiter);
        int MonsterIdRead = stoi(line.substr(0, pos));
        //printf("%d ", MonsterIdRead);

        pos2 = line.length();
        string MonsterStrRead = line.substr(pos + 1, pos2);
        //cout << MonsterStrRead << endl;

        if (MonsterIdRead == MonsterId) {
            strcpy(MonsterStr, MonsterStrRead.data());
            fin.close();
            return 1;
        }

        lines.push_back(line);
    }
    fin.close();
    //printf("MonsterReadFunction02\n");

    return 0;
}


BOOL TimeCounter() {
    int TimerCounter = 0;
    gAutoTeleIdleCounter = 0;
    while (1) {
        
        if ((gAutoTeleSwitch || gMoveRandSwitch) && gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW) {
            gAutoTeleIdleCounter++;
        }

        // 怪物計時器
        if (gAutoMonsterTeleIdleCounter > 0) {
            gAutoMonsterTeleIdleCounter--;
        }

        // 致命傷害計時器
        if (gDetactSpecialAttackIdleCounter > 0) {
            gDetactSpecialAttackIdleCounter--;
        }
        else {
            gDetactSpecialAttack = 0;
        }

        // 羅剎壓血 避免重複計時器
        if (gSmartBloodAttackIdleCounter > 0) {
            gSmartBloodAttackIdleCounter--;
        }
        else {
            gSmartBloodAttack = 0;
        }

        //// 智能偽裝計時器
        //if (gAutoSmartCloakIdleCounter > 0) {
        //    gAutoSmartCloakIdleCounter--;
        //}
        //else {
        //    gAutoSmartCloakFlag = 1;
        //}

        // 集結計時器
        if (gLeaderIdleCounter > 0) {
            gLeaderIdleCounter--;
        }
        else {
            gLeaderIdleCounter = 0;
        }

        // 過傳點暫停使用自瞄
        if (gStopAutoAttackIdleCounter > 0) {
            gStopAutoAttackIdleCounter--;
        }
        else {
            gStopAutoAttackIdleCounter = 0;
        }

        // 自動洗AP計時器
        if (gAutoApIdleCounter > 0) {
            gAutoApIdleCounter--;
        }
        else {
            gAutoApIdleCounter = 0;
        }

        // 偵測你放招計時器
        if (gDetactYouAreCastIdleCounter > 0) {
            gDetactYouAreCastIdleCounter--;
        }
        else {
            gDetactYouAreCast = 0;
        }

        // 偵測你放招計時器(幽波)
        if (gDetactYouAreCastGhostringIdleCounter > 0) {
            gDetactYouAreCastGhostringIdleCounter--;
        }
        else {
            gDetactYouAreCastGhostring = 0;
        }

        // 智能切裝計時器
        for (int SkillCounter = 0; SkillCounter < 6; SkillCounter++) {
            if (gSmartEquipTriggerIdleCounter[SkillCounter] > 0) {
                gSmartEquipTriggerIdleCounter[SkillCounter]--;
            }
            else {
                gSmartEquipTrigger[SkillCounter] = 0;
            }
        }

        TimerCounter++;
        if (TimerCounter > 10000) {
            if (gAutoTeleSwitch && gTotalSwitch && !gStorageStatus && gAutoTeleEnter && gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW) {
                SendMessage(gHwnd, WM_KEYDOWN, 13, 0); // 開啟瞬移模式 每10秒按一次enter避免卡住
            } 
            else if (gAutoTeleSwitch && gTotalSwitch && !gStorageStatus && gAutoTeleSpace && gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW) {
                SendMessage(gHwnd, WM_KEYDOWN, 32, 0); // 開啟瞬移模式 每10秒按一次space避免卡住
            }
            TimerCounter = 0;
        }

        if (!gTotalSwitch || *MAP_NAME == LOGIN_RSW) {
            ClearAllTriggerFlag();
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL TimeCounter2() {
    while (1) {

        SkillDelayPlayerList.TimeTick_1s();

        Sleep(1000);
    }
    return EXIT_SUCCESS;
}



void ClearTriggerFlag(void) {
    gAttackHitTriggerFlag1 = 0;
    gAttackHitTriggerFlag2 = 0;
    gAttackHitTriggerFlag3 = 0;
    gDetectHitTriggerFlag = 0;
    gDetectGreedTriggerFlag = 0;

    //gNearAttackScreenMonsterTrigger = 0;
}

void ClearAllTriggerFlag(void) {
    gAttackHitTriggerFlag1 = 0;
    gAttackHitTriggerFlag2 = 0;
    gAttackHitTriggerFlag3 = 0;
    gDetectHitTriggerFlag = 0;
    gDetectGreedTriggerFlag = 0;

    gTeleportTrigger = 0;
    //gNearAttackScreenMonsterTrigger = 0;
    gAutoFindTargetAndAttackTrigger = 0;
    gSmartBloodTrigger = 0;
    for (int i = 0; i < 6; i++) gSmartEquipTrigger[i] = 0;
    for (int i = 0; i < 6; i++) gSmartEquipTriggerIdleCounter[i] = 0;
    for (int i = 0; i < 7; i++) gCastEquipTrigger[i] = 0;
}

BOOL SpecialEquip() {
    while (1) {

        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW) {
            //
            // 特殊切裝
            //
            for (int SpecialEquipCount = 0; SpecialEquipCount < 8; SpecialEquipCount++) {
                if ((IsRoFocus() && GetKeyState(gAutoEquipScanCode[SpecialEquipCount]) & 0x8000)  // 掃描的號碼
                    && gAutoEquipSwitch[SpecialEquipCount]         // 開關必須是開的
                    && !gStorageStatus                             // 非開倉狀態
                    && CloakingTryNum >= 1                         // 過圖後必須經過一段時間
                    && gTotalSwitch) {                             // 總開關
                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState()) {
                        break;
                    }
                    printf("特殊切裝編號:%d \n", SpecialEquipCount);

                    Sleep(gAutoEquipDelay[SpecialEquipCount]);

                    SendAttackPackageFunction(gAutoEquipSkilv[SpecialEquipCount], gAutoEquipSkill[SpecialEquipCount], *AidAddress);

                    ////
                    //// 測試CD加密
                    ////
                    //printf("測試CD加密\n");

                    //DWORD EDI_ADDRESS = 0x0019349C;

                    //DWORD PUSH_ADDRESS = 0x190A84;

                    //DWORD CDCLIENT_ADDRESS = 0xD6E040;


                    //MEMORY_BASIC_INFORMATION mbi;
                    //DWORD* DECODE_BASE_ADDRESS_1 = reinterpret_cast <DWORD*>(0x00FFD738);

                    //DWORD* DECODE_BASE_ADDRESS_2 = NULL;
                    //DWORD DECODE_BASE_ADDRESS_3 = NULL;

                    //if (VirtualQuery((LPCVOID)DECODE_BASE_ADDRESS_1, &mbi, sizeof(mbi))) {
                    //    if (mbi.Protect == PAGE_READWRITE) {
                    //        DECODE_BASE_ADDRESS_2 = (DWORD*)(*DECODE_BASE_ADDRESS_1 + 8);
                    //    }
                    //}
                    //if (VirtualQuery((LPCVOID)DECODE_BASE_ADDRESS_2, &mbi, sizeof(mbi))) {
                    //    if (mbi.Protect == PAGE_READWRITE) {
                    //        DECODE_BASE_ADDRESS_3 = (*DECODE_BASE_ADDRESS_2);
                    //    }
                    //}

                    //DECODE_BASE_ADDRESS_3 += 2;

                    //printf("DECODE_BASE_ADDRESS_3: %8X\n", DECODE_BASE_ADDRESS_3);
                    //
                    //__asm {
                    //    pushad

                    //    mov eax, DECODE_BASE_ADDRESS_3
                    //    mov dword ptr[eax], 0

                    //    push 1;

                    //    push PUSH_ADDRESS;

                    //    push DECODE_BASE_ADDRESS_3

                    //    push 10

                    //    push EDI_ADDRESS

                    //    mov eax, CDCLIENT_ADDRESS
                    //    //call dword ptr[eax]

                    //    popad
                    //}



                    ////
                    //// 測試走路
                    ////
                    //printf("測試走路\n");

                    //DWORD WALK_ADDRESS = 0x00DF2F4C;

                    //DWORD WALK_ADDRESS2 = 0x006E00F1;
                    //DWORD WALK_ADDRESS3 = 0x0F05EDC;

                    //DWORD WALK_ADDRESS4 = 0x1;

                    //__asm {
                    //    pushad

                    //    push 0
                    //    push 0
                    //    push 0

                    //    push 01
                    //    push 0x18

                    //    mov eax, WALK_ADDRESS

                    //    call dword ptr[eax]

                    //    popad
                    //}



                    //int* WALK_ADDRESS_TRIGGER_REGION = reinterpret_cast <int*>(0x00EE7A4C);

                    //int* WALK_ADDRESS_X_REGION = reinterpret_cast <int*>(0x0019F70C);
                    //int* WALK_ADDRESS_Y_REGION = reinterpret_cast <int*>(0x0019F710);

                    //*WALK_ADDRESS_TRIGGER_REGION = 2;
                    ////*WALK_ADDRESS_X_REGION = *CURSOR_CORRD_X + 1;
                    ////*WALK_ADDRESS_Y_REGION = *CURSOR_CORRD_Y + 1;
                    //Sleep(200);
                    //*WALK_ADDRESS_TRIGGER_REGION = 0;



                    //DWORD WALK_ADDRESS = 0xDF2F4C;
                    //DWORD x = 0x94;
                    //DWORD y = 0xBA;
                    //DWORD CALL_VALUE_REGION_00 = 0x193448;

                    //DWORD EBP_ADDRESS = 0x0019F62C;

                    //__asm {
                    //    pushad

                    //    push ebp              ;儲存當前ebp　　
                    //    //mov ebp, EBP_ADDRESS  ;EBP設為當前堆疊指標

                    //    mov eax, CALL_VALUE_REGION_00
                    //    mov byte ptr[eax], 0x5F

                    //    push 0
                    //    push 0

                    //    push y
                    //    push x

                    //    push 0x8A

                    //    mov eax, WALK_ADDRESS

                    //    call dword ptr[eax]

                    //    pop ebp  ;取出當前ebp　
   
                    //    popad
                    //}


                //    DWORD EBP_ADDRESS = 0x19F5B4;
                //    DWORD MOVE_PACKET_VALUE = 0x0000035F;

                //    DWORD VALUE_0C168 = 0x0000C168;
                //    DWORD VALUE_0C16A = 0x0000C16A;
                //    DWORD VALUE_0C16C = 0x0000C16C;

                //    DWORD CODE_REGION_01 = EBP_ADDRESS - VALUE_0C168;
                //    DWORD CODE_REGION_02 = EBP_ADDRESS - VALUE_0C16A;
                //    DWORD CODE_REGION_03 = EBP_ADDRESS - VALUE_0C16C;

                //    DWORD CALL_VALUE_REGION_03 = 0x00A99EF0;

                //    DWORD CALL_VALUE_REGION_EAXECX = 0x0125AB68;
                //    DWORD CALL_VALUE_REGION_ESI = 0x8A;
                //    DWORD CALL_VALUE_REGION_EBP = 0x0019F5B4;
                //    DWORD CALL_VALUE_REGION_ESP = 0x00190A34;

                //    // 變數 EBX, EDX, EDI

                //    int* CALL_VALUE_REGION_EBX_PTR = reinterpret_cast <int*>(0x0125A958);
                //    DWORD CALL_VALUE_REGION_EBX = (DWORD) *CALL_VALUE_REGION_EBX_PTR;

                //    DWORD CALL_VALUE_REGION_EDI = (DWORD) *WINDOWS_LOCK_1;
                //    DWORD CALL_VALUE_REGION_00 = 0x193448;

                //    __asm {
                //        pushad

                //    /*    mov eax, CALL_VALUE_REGION_EAXECX
                //        mov ebx, CALL_VALUE_REGION_EBX
                //        mov ecx, CALL_VALUE_REGION_EAXECX

                //        mov esi, CALL_VALUE_REGION_ESI
                //        mov edi, CALL_VALUE_REGION_EDI

                //        mov ebp, CALL_VALUE_REGION_EBP*/
                //       // mov esp, CALL_VALUE_REGION_ESP

                //        mov eax, CALL_VALUE_REGION_00
                //        mov byte ptr[eax], 0x5F

                //        mov eax, CALL_VALUE_REGION_EAXECX
                //        mov ecx, eax
                //        mov ebx, CALL_VALUE_REGION_EBX
                //        mov esi, CALL_VALUE_REGION_ESI
                //        mov edi, CALL_VALUE_REGION_EDI

                //        call CALL_VALUE_REGION_03

                //        //mov eax, CALL_VALUE_REGION_EAXECX
                //        //call CALL_VALUE_REGION_03

                ///*       
                //        mov ebp, EBP_ADDRESS
                //        lea eax, dword ptr[CODE_REGION_03]
                //        push eax
                //        push MOVE_PACKET_VALUE

                //        mov byte ptr[CODE_REGION_01], dl

                //        call CALL_VALUE_REGION_01
                //        mov ecx, eax
                //        call CALL_VALUE_REGION_02
                //        push eax
                //        call CALL_VALUE_REGION_01
                //        mov ecx, eax
                //        call CALL_VALUE_REGION_03
                //*/
                //        popad
                //    }

                    if (gAutoEquipBackScanCode[SpecialEquipCount]) {
                        Sleep(gAutoEquipDelay[SpecialEquipCount]);
                        SendMessage(gHwnd, WM_KEYDOWN, gAutoEquipBackScanCode[SpecialEquipCount], 0);
                    }

                    while (1) {
                        Sleep(10);
                        if (!(GetKeyState(gAutoEquipScanCode[SpecialEquipCount]) & 0x8000)) {
                            break;
                        }
                    }
                }
            }


            //
            // 攻擊特殊切裝
            //
            for (int SpecialEquipCount = 0; SpecialEquipCount < 8; SpecialEquipCount++) {
                if ((IsRoFocus() && GetKeyState(gAutoAttackEquipScanCode[SpecialEquipCount]) & 0x8000)  // 掃描的號碼
                    && gAutoAttackEquipSwitch[SpecialEquipCount]         // 開關必須是開的
                    && !gStorageStatus                                   // 非開倉狀態
                    && CloakingTryNum >= 1                               // 過圖後必須經過一段時間
                    && gTotalSwitch) {                                   // 總開關
                    //if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState()) {
                    //    break;
                    //}
                    printf("攻擊特殊切裝編號:%d \n", SpecialEquipCount);

                    for (int i = 0; i < 5; i++) {
                        if (SpecialEquipCount == 0 && gAutoAttackEquipScanCode01[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode01[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode01[i], 0);
                            }
                        }

                        if (SpecialEquipCount == 1 && gAutoAttackEquipScanCode02[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode02[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode02[i], 0);
                            }
                        }

                        if (SpecialEquipCount == 2 && gAutoAttackEquipScanCode03[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode03[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode03[i], 0);
                            }
                        }
                        if (SpecialEquipCount == 3 && gAutoAttackEquipScanCode04[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode04[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode04[i], 0);
                            }
                        }

                        if (SpecialEquipCount == 4 && gAutoAttackEquipScanCode05[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode05[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode05[i], 0);
                            }
                        }

                        if (SpecialEquipCount == 5 && gAutoAttackEquipScanCode06[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode06[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode06[i], 0);
                            }
                        }

                        if (SpecialEquipCount == 6 && gAutoAttackEquipScanCode07[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode07[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode07[i], 0);
                            }
                        }
                        if (SpecialEquipCount == 7 && gAutoAttackEquipScanCode08[i]) {
                            Sleep(gAutoAttackEquipDelay[SpecialEquipCount]);
                            if (gAutoAttackEquipScanCode08[i] == 20000) {
                                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                                Sleep(10);
                                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            }
                            else {
                                SendMessage(gHwnd, WM_KEYDOWN, gAutoAttackEquipScanCode08[i], 0);
                            }
                        }
                    }

                    while (1) {
                        Sleep(10);
                        if (!(GetKeyState(gAutoAttackEquipScanCode[SpecialEquipCount]) & 0x8000)) {
                            break;
                        }
                    }
                }
            }
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL PetFeedFunction() {
#if !PrivateServerOrNot
    while (1) {
        if (*AidAddress && *MAP_NAME != LOGIN_RSW) {
            if (gAutoPetFeedSwitch) { // 寵物餵食
                if (CheckPetHungryState()) {

                    char PetBuffer[5] = { 0x05, 0x00, 0xA1, 0x01, 0x01 };
                    printf("自動餵食寵物\n");
                    printf("寵物飢餓度: %d\n", *PET_HUNGRY);
                    //OriginalSend(roServer, PetBuffer, 5 - OriginalSendOffset, 0);
                    //Sleep(5000);
                    //

                    int PetSuccessTry = 0;
                    *CURSOR_CORRD_X = gAutoPetFeedLock_X;
                    *CURSOR_CORRD_Y = gAutoPetFeedLock_Y;
                    Sleep(200);
                    if (!IsHandCusor()) {
                        Sleep(1000);
                        *CURSOR_CORRD_X = gAutoPetFeedLock_X;
                        *CURSOR_CORRD_Y = gAutoPetFeedLock_Y;
                    }
                    if (IsHandCusor()) {
                        // 按兩下左鍵
                        SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                        Sleep(50);
                        SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                        Sleep(500);

                        SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                        Sleep(50);
                        SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);

                        Sleep(2000);

                        *CURSOR_CORRD_X = gAutoPetFeedLock2_X;
                        *CURSOR_CORRD_Y = gAutoPetFeedLock2_Y;
                        Sleep(200);
                        if (!IsHandCusor()) {
                            Sleep(1000);
                            *CURSOR_CORRD_X = gAutoPetFeedLock2_X;
                            *CURSOR_CORRD_Y = gAutoPetFeedLock2_Y;
                        }
                        if (IsHandCusor()) {
                            // 按一下左鍵
                            SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                            Sleep(50);
                            SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
                            Sleep(500);
                            *PET_HUNGRY += 50;
                            PetSuccessTry = 1;
                        }
                    }

                    // 餵食失敗
                    if (!PetSuccessTry) {

                        //MessageBox(NULL, "錯誤, 你的寵物設定有誤.", NULL, MB_ICONEXCLAMATION);

                        for (int i = 0; i < 10; i++) {
                            PlaySound(TEXT(".\\data\\sound.wav"), NULL, SND_FILENAME | SND_ASYNC);
                            Sleep(1000);
                        }
                        Sleep(5000);

                        *PET_HUNGRY += 50;
                    }

                    //system("start data\\sound.mp3");

                    if (*PET_HUNGRY <= 10) {
                        //太過飢餓 鼠標又指不到
                        Sleep(150000);
                        if (*PET_HUNGRY <= 10) {
                            printf("遊戲關閉\n");
                            exit(0);
                        }
                    }
                    
                }
            }
        }

        Sleep(1);
    }
#endif
    return EXIT_SUCCESS;
}

BOOL AutoCureFunction() {
    #define SR_GENTLETOUCH_CURE 2345
    #define SR_GENTLETOUCH_CURE_LV 2345
    while (1) {

        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW && CloakingTryNum >= 1 && !gStorageStatus
            && !CheckDebuffState(4) && !CheckDebuffState(470) && !CheckDebuffState(417) && !CheckDebuffState(411) || CheckDebuffState(438)) {
            //
            // 自動點穴快
            //
            if ((CheckDebuffState(876) || CheckDebuffState(875)) && gCharSpirits > 0 && gAutoCure) { // 冰凍和石化
                
                Sleep(10);
                Sleep(gAutoCureDelay);
                if ((CheckDebuffState(876) || CheckDebuffState(875)) && gCharSpirits > 0 && gAutoCure) { // 冰凍和石化
          
                    SendAttackPackageFunction(SR_GENTLETOUCH_CURE_LV, SR_GENTLETOUCH_CURE, *AidAddress);
                }
            }

            //
            // 自動補氣彈
            //
            if (gCharSpirits <= gAutoSpiritSpiritValue && gAutoSpiritSwitch) { // 補氣彈
                AutoEquipDetectFunction(401); // 狂蓄氣
                Sleep(gAutoSpiritDelay);
                if (gCharSpirits <= gAutoSpiritSpiritValue) { // 再檢查一次
                    SendMessage(gHwnd, WM_KEYDOWN, gAutoSpiritScanCode, 0);
                }

            }
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL AutoApFunction() {
#if !PrivateServerOrNot
    char SkillBuffer[12] = { 0x0c, 0x00, 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
    char NearSkillBufferGround[13] = { 0x0D, 0x00, 0xF4, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#else
    char SkillBuffer[10] = { 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
    char NearSkillBufferGround[11] = { 0xF4, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#endif
    while (1) {
        if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gAutoApSwitch && gAutoApSkill && gAutoApLv
            && !gStorageStatus && CloakingTryNum >= 3 && !gConvenioTrigger && *HPIndexValue > 1 && CheckState() && !MapLoadedFlag) {
            //
            // 自動補AP
            //

            // 如果有設定集結隊長
            // 當狀態條件不符合 略過集AP
            if (!CheckDebuffState(gConvenioLeaderState) && gConvenioLeaderSwitch && gConvenioTotalSwitch) {
                continue;
            }

            if (gCurrentApValue < gAutoApValue && gAutoApIdleCounter == 0) {

                Sleep(gAutoApDelay);

                if (*AidAddress != 0 && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gAutoApSwitch && gAutoApSkill && gAutoApLv
                    && !gStorageStatus && CloakingTryNum >= 3 && !gConvenioTrigger && *HPIndexValue > 1 && CheckState()) {
                    if (!gAutoApGround) {
                        SendSkillCastFunction(gAutoApSkill, gAutoApLv, *AidAddress);
                    }
                    else {
                        SendSkillGroundFunction(gAutoApSkill, gAutoApLv, *PLAYER_CORRD_X, *PLAYER_CORRD_Y);
                    }
                }
                
            }

        }
        Sleep(1);
    }
    return EXIT_SUCCESS;
}


BOOL AutoAttackHitFunction1() {
    int AttackHitAid = 0;
    int* CorrdX = new int;
    int* CorrdY = new int;
    while (1) {
        if (gAttackHitTriggerFlag1 == 1 && gAttackHitSwitch1 == 1 && gTotalSwitch) {
            //
            // char gAttackHitSkillBuffer[10] = { 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
            // 38 04 --> SKILL USE
            // 03 00 
            // F6 08 --> 無知ID
            debug("使用接技技能01");
            
            // 存取AID
            AttackHitAid = gAttackHitSkillBuffer;
            //
            // Myself AID
            //
            if (gAttackHitModify1 == 2006 || gAttackHitModify1 == 2317 || gAttackHitModify1 == 2330 || gAttackHitModify1 == 2329) {
                gAttackHitSkillBuffer = *AidAddress;
            }
            // 是否對自己使用
            if (gAttackHitSupport1) {
                gAttackHitSkillBuffer = *AidAddress;
            }
            // 是否是地板技能
            if (gAttackHitGround1) {

                //// 尋找目標座標
                //// 有找到目標在清單中 將x,y更新為目標所在地
                //if (TargetAidList.search_target_Aid(AttackHitAid, CorrdX, CorrdY) && !PlayerAidList.search_player(AttackHitAid) && !gAttackHitSupport1) {
                //    // x, y
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((*CorrdX) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((*CorrdX) & 0xFF00) >> 8);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((*CorrdY) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((*CorrdY) & 0xFF00) >> 8);
                //}
                //else { // 沒找到目標在清單, 將x,y更新為本身座標
                //    // x, y
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((*PLAYER_CORRD_X) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((*PLAYER_CORRD_X) & 0xFF00) >> 8);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((*PLAYER_CORRD_Y) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((*PLAYER_CORRD_Y) & 0xFF00) >> 8);
                //}
                // x, y
                //gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((gAttackHitGroundCoordX1) & 0xFF);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((gAttackHitGroundCoordX1) & 0xFF00) >> 8);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((gAttackHitGroundCoordY1) & 0xFF);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((gAttackHitGroundCoordY1) & 0xFF00) >> 8);
                
            }
            int Aid = gAttackHitSkillBuffer;

            for (int SkillUseCount = 0; SkillUseCount < gAttackHitModSkillCount1; SkillUseCount++) {

                Sleep(gAttackHitDelay1);

                int FindTarget = 1;
                if (AttackHitAid != *AidAddress && !(gAttackHitGroundCoordX1 && gAttackHitGroundCoordY1 && gAttackHitGround1)) {
                    FindTarget = TargetAidList.search_target(AttackHitAid);
                }

                if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || !FindTarget) {
                    break;
                }
                //
                // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                //
                if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                    break;
                }

                // 指定技能
                if (!gAttackHitGround1) {
                    
                    SendAttackPackageFunction(gAttackHitModSkillLv1, gAttackHitModify1, Aid);
                } 
                // 地板技能
                else if (gAttackHitGround1) {
                   
                    SendAttackGroundPackageFunction(gAttackHitModSkillLv1, gAttackHitModify1, gAttackHitGroundCoordX1, gAttackHitGroundCoordY1);
                }
                
                gAutoTeleIdleCounter = 0;
                
            }
            // 大纏後接技
            if (gAttackHitModify1 == 2329 && gAttackHitFallen1) {
                for (int SkillUseCount = 0; SkillUseCount < 5; SkillUseCount++) {

                    Sleep(gAttackHitDelay1);

                    int FindTarget = 1;
                    if (AttackHitAid != *AidAddress) {
                        FindTarget = TargetAidList.search_target(AttackHitAid);
                    }

                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || !FindTarget) {
                        break;
                    }
                    //
                    // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                    //
                    if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                        break;
                    }
                    if (gAttackHitFallen1 == 1) { // 虎泡
                        SendSkillCastFunction(2330, 10, *AidAddress);
                    }
                    else if (gAttackHitFallen1 == 2) { // 羅剎
                        SendSkillCastFunction(2343, 10, *AidAddress);
                    }
                    gAutoTeleIdleCounter = 0;
                    
                }
            }

            Sleep(300);
            gAutoTeleIdleCounter = 0;
            gAttackHitTriggerFlag1 = 0;
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL AutoAttackHitFunction2() {
    int AttackHitAid = 0;
    int* CorrdX = new int;
    int* CorrdY = new int;
    while (1) {
        if (gAttackHitTriggerFlag2 == 2 && gAttackHitSwitch2 == 1 && gTotalSwitch) {
            //
            // char gAttackHitSkillBuffer[10] = { 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
            // 38 04 --> SKILL USE
            // 03 00 
            // F6 08 --> 無知ID
            debug("使用接技技能02");

            // 存取AID
            AttackHitAid = gAttackHitSkillBuffer;
            //
            // Myself AID
            //
            if (gAttackHitModify2 == 2006 || gAttackHitModify2 == 2317 || gAttackHitModify2 == 2330 || gAttackHitModify2 == 2329) {
                gAttackHitSkillBuffer = *AidAddress;
            }
            // 是否對自己使用
            if (gAttackHitSupport2) {
                gAttackHitSkillBuffer = *AidAddress;
            }
            // 是否是地板技能
            if (gAttackHitGround2) {
                //// 尋找目標座標
                //// 有找到目標在清單中 將x,y更新為目標所在地
                //if (TargetAidList.search_target_Aid(AttackHitAid, CorrdX, CorrdY) && !PlayerAidList.search_player(AttackHitAid) && !gAttackHitSupport2) {
                //    // x, y
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((*CorrdX) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((*CorrdX) & 0xFF00) >> 8);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((*CorrdY) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((*CorrdY) & 0xFF00) >> 8);
                //}
                //else { // 沒找到目標在清單, 將x,y更新為本身座標
                //    // x, y
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((*PLAYER_CORRD_X) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((*PLAYER_CORRD_X) & 0xFF00) >> 8);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((*PLAYER_CORRD_Y) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((*PLAYER_CORRD_Y) & 0xFF00) >> 8);
                //}
                //gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((gAttackHitGroundCoordX2) & 0xFF);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((gAttackHitGroundCoordX2) & 0xFF00) >> 8);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((gAttackHitGroundCoordY2) & 0xFF);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((gAttackHitGroundCoordY2) & 0xFF00) >> 8);
            }
            int Aid = gAttackHitSkillBuffer;
            for (int SkillUseCount = 0; SkillUseCount < gAttackHitModSkillCount2; SkillUseCount++) {

                Sleep(gAttackHitDelay2);
                
                int FindTarget = 1;
                if (AttackHitAid != *AidAddress && !(gAttackHitGroundCoordX2 && gAttackHitGroundCoordY2 && gAttackHitGround2)) {
                    FindTarget = TargetAidList.search_target(AttackHitAid);
                }

                if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || !FindTarget) {
                    break;
                }
                //
                // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                //
                if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                    break;
                }

                // 指定技能
                if (!gAttackHitGround2) {
                    SendAttackPackageFunction(gAttackHitModSkillLv2, gAttackHitModify2, Aid);
                   
                }
                // 地板技能
                else if (gAttackHitGround2) {
                 
                    SendAttackGroundPackageFunction(gAttackHitModSkillLv2, gAttackHitModify2, gAttackHitGroundCoordX2, gAttackHitGroundCoordY2);
                }

                gAutoTeleIdleCounter = 0;
                
            }
            // 大纏後接技
            if (gAttackHitModify2 == 2329 && gAttackHitFallen2) {
                for (int SkillUseCount = 0; SkillUseCount < 5; SkillUseCount++) {

                    Sleep(gAttackHitDelay2);

                    int FindTarget = 1;
                    if (AttackHitAid != *AidAddress) {
                        FindTarget = TargetAidList.search_target(AttackHitAid);
                    }

                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || !FindTarget) {
                        break;
                    }
                    //
                    // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                    //
                    if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                        break;
                    }
                    if (gAttackHitFallen2 == 1) { // 虎泡
                        SendSkillCastFunction(2330, 10, *AidAddress);
                    }
                    else if (gAttackHitFallen2 == 2) { // 羅剎
                        SendSkillCastFunction(2343, 10, *AidAddress);
                    }
                    gAutoTeleIdleCounter = 0;
                }
            }

            Sleep(300);
            gAutoTeleIdleCounter = 0;
            gAttackHitTriggerFlag2 = 0;
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}


BOOL AutoAttackHitFunction3() {
    int AttackHitAid = 0;
    int* CorrdX = new int;
    int* CorrdY = new int;

    while (1) {
        if (gAttackHitTriggerFlag3 == 3 && gAttackHitSwitch3 == 1 && gTotalSwitch) {
            //
            // char gAttackHitSkillBuffer[10] = { 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
            // 38 04 --> SKILL USE
            // 03 00 
            // F6 08 --> 無知ID
            debug("使用接技技能03");

            // 存取AID
            AttackHitAid = gAttackHitSkillBuffer;
            //
            // Myself AID
            //
            if (gAttackHitModify3 == 2006 || gAttackHitModify3 == 2317 || gAttackHitModify3 == 2330 || gAttackHitModify3 == 2329) {
                gAttackHitSkillBuffer = *AidAddress;
            }
            // 是否對自己使用
            if (gAttackHitSupport3) {
                gAttackHitSkillBuffer = *AidAddress;
            }
            // 是否是地板技能
            if (gAttackHitGround3) {
                //// 尋找目標座標
                //// 有找到目標在清單中 將x,y更新為目標所在地
                //if (TargetAidList.search_target_Aid(AttackHitAid, CorrdX, CorrdY) && !PlayerAidList.search_player(AttackHitAid) && !gAttackHitSupport3) {
                //    // x, y
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((*CorrdX) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((*CorrdX) & 0xFF00) >> 8);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((*CorrdY) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((*CorrdY) & 0xFF00) >> 8);
                //}
                //else { // 沒找到目標在清單, 將x,y更新為本身座標
                //    // x, y
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((*PLAYER_CORRD_X) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((*PLAYER_CORRD_X) & 0xFF00) >> 8);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((*PLAYER_CORRD_Y) & 0xFF);
                //    gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((*PLAYER_CORRD_Y) & 0xFF00) >> 8);
                //}
                //gAttackHitSkillBufferGround[AidSkillBufferOffset] = ((gAttackHitGroundCoordX3) & 0xFF);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 1] = (((gAttackHitGroundCoordX3) & 0xFF00) >> 8);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 2] = ((gAttackHitGroundCoordY3) & 0xFF);
                //gAttackHitSkillBufferGround[AidSkillBufferOffset + 3] = (((gAttackHitGroundCoordY3) & 0xFF00) >> 8);
            }
            int Aid = gAttackHitSkillBuffer;
            for (int SkillUseCount = 0; SkillUseCount < gAttackHitModSkillCount3; SkillUseCount++) {

                Sleep(gAttackHitDelay3);
                
                int FindTarget = 1;
                if (AttackHitAid != *AidAddress && !(gAttackHitGroundCoordX3 && gAttackHitGroundCoordY3 && gAttackHitGround3)) {
                    FindTarget = TargetAidList.search_target(AttackHitAid);
                }

                if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || !FindTarget) {
                    break;
                }
                //
                // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                //
                if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                    break;
                }
                
                // 指定技能
                if (!gAttackHitGround3) {
                    SendAttackPackageFunction(gAttackHitModSkillLv3, gAttackHitModify3, Aid);
                    
                }
                // 地板技能
                else if (gAttackHitGround3) {
                   
                    SendAttackGroundPackageFunction(gAttackHitModSkillLv3, gAttackHitModify3, gAttackHitGroundCoordX3, gAttackHitGroundCoordY3);
                }

                gAutoTeleIdleCounter = 0;
                
            }
            // 大纏後接技
            if (gAttackHitModify3 == 2329 && gAttackHitFallen3) {
                for (int SkillUseCount = 0; SkillUseCount < 5; SkillUseCount++) {

                    Sleep(gAttackHitDelay3);

                    int FindTarget = 1;
                    if (AttackHitAid != *AidAddress) {
                        FindTarget = TargetAidList.search_target(AttackHitAid);
                    }

                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || !FindTarget) {
                        break;
                    }
                    //
                    // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                    //
                    if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                        break;
                    }
                    if (gAttackHitFallen3 == 1) { // 虎泡
                        SendSkillCastFunction(2330, 10, *AidAddress);
                    }
                    else if (gAttackHitFallen3 == 2) { // 羅剎
                        SendSkillCastFunction(2343, 10, *AidAddress);
                    }
                    gAutoTeleIdleCounter = 0;
                }
            }

            Sleep(300);
            gAutoTeleIdleCounter = 0;
            gAttackHitTriggerFlag3 = 0;
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}


BOOL AutoMacroFunction() {

    while (1) {

        if (gTotalSwitch && gAutoMacroSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW && CloakingTryNum >= 1) {
            if (gAutoMacroScanCode != 8888) {
                SendMessage(gHwnd, WM_KEYDOWN, gAutoMacroScanCode, 0);
                Sleep(gAutoMacroDelay);
            }
            else { // 左鍵
                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                Sleep(50);
                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
            }
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

BOOL AutoMacroFunction2() {
    while (1) {

        if (gTotalSwitch && gAutoMacroSwitch2 && *AidAddress && *MAP_NAME != LOGIN_RSW && CloakingTryNum >= 1) {
            if (gAutoMacroScanCode2 != 8888) {
                SendMessage(gHwnd, WM_KEYDOWN, gAutoMacroScanCode2, 0);
                Sleep(gAutoMacroDelay2);
            }
            else { // 左鍵
                SendMessage(gHwnd, WM_LBUTTONDOWN, 0, 0);
                Sleep(50);
                SendMessage(gHwnd, WM_LBUTTONUP, 0, 0);
            }
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

void SendEquipCommand(equipDataStruct EquipItem, int UpdateIsEquip, int ForceEquip, int delay, int typeOption, int CheckEquipSuccess) {
    int* isEquip = new int;
    int* equipType = new int;
    int* Id = new int;
    int Status = 0;

    if (PACKET_MODE) {
        CheckEquipSuccess = 0;
    }

    if (CheckEquipSuccess && !PACKET_MODE) {
        delay = 10;
    }

#if !PrivateServerOrNot
    char EquipBuffer[10] = { 0x0A, 0x00, 0x98, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int EquipBufferOffset = 4;
#else
    char EquipBuffer[8] = { 0x98, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int EquipBufferOffset = 2;
#endif

    if (EquipItem.itemId) { // 有設定裝備
                            // 尋找此裝備的身上ID, 以及查看是否已經有裝備在身上
        *isEquip = 0;
        *equipType = 0;
        *Id = 0;
        Status = ItemDataList.search_equip_data(
            EquipItem.refineView,
            EquipItem.itemId,
            EquipItem.slot1,
            EquipItem.slot2,
            EquipItem.slot3,
            EquipItem.slot4,
            isEquip,
            equipType,
            Id
        );

        if (Status) {
            // printf("有找到\n");
        }

        // 達成穿裝條件
        int TryEquipCount = 0;
        while ((*isEquip == 0 || ForceEquip) && *equipType && *Id) {
            if (CheckDebuffState(418) && (*equipType == 2 || *equipType == 32 || *equipType == 34)) { // 衰弱
                ItemDataList.update_item_isEquip(*Id, 0);
                return;
            }
            if (CheckDebuffState(421) && (*equipType == 8 || *equipType == 128 || *equipType == 136)) { // 脫裝飾品
                ItemDataList.update_item_isEquip(*Id, 0);
                return;
            }
            printf("達成條件, 切裝 %d type: %d, IsEquip: %d\n", EquipItem.itemId, *equipType, *isEquip);
            // Id
            EquipBuffer[EquipBufferOffset] = *Id & 0xFF;
            EquipBuffer[EquipBufferOffset + 1] = (*Id >> 8) & 0xFF;

            if (typeOption) {
                *equipType = typeOption;
            }

            // equipType
            EquipBuffer[EquipBufferOffset + 2] = *equipType & 0xFF;
            EquipBuffer[EquipBufferOffset + 3] = (*equipType & 0xFF00) >> 8;
            EquipBuffer[EquipBufferOffset + 4] = (*equipType & 0xFF0000) >> 16;
            EquipBuffer[EquipBufferOffset + 5] = (*equipType & 0xFF000000) >> 24;
#if PACKET_MODE
            OriginalSend(roServer, EquipBuffer, 10 - OriginalSendOffset, 0);
#else
            HotKeySendItem(EquipItem.itemId, delay);
#endif

            //Sleep(delay);
            TryEquipCount++;

            if (UpdateIsEquip) {
                ItemDataList.update_item_isEquip(*Id, 1);
            }

            if (TryEquipCount > 15) {
                break;
            }

            Status = ItemDataList.search_equip_data(
                EquipItem.refineView,
                EquipItem.itemId,
                EquipItem.slot1,
                EquipItem.slot2,
                EquipItem.slot3,
                EquipItem.slot4,
                isEquip,
                equipType,
                Id);
            ForceEquip = 0;

            if (!CheckEquipSuccess) {
                break;
            }
        }

    }

    delete isEquip;
    delete equipType;
    delete Id;
}


BOOL OneKeyEquipFunction() {
    int* isEquip = new int;
    int* equipType = new int;
    int* Id = new int;

    while (1) {

        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW && CloakingTryNum >= 2 && !gStorageStatus) {

            if (IsRoFocus() && gOneKeyEquipSwitch01 && GetKeyState(gOneKeyEquipScanCode01) & 0x8000) {
                printf("偵測到一鍵切裝01\n");
                
                for (int TotalRun = 0; TotalRun < 2; TotalRun++) { // 總共檢查2輪

                    int AccessorFlag = 0; // 飾品判斷flag
                    for (int Index = 0; Index < 20; Index++) { // 20件裝備
                        // 切裝
   
                        // 檢查裝備是否是左右飾品
                        ItemDataList.search_equip_data(
                            gOneKeyEquipItem01[Index].refineView,
                            gOneKeyEquipItem01[Index].itemId,
                            gOneKeyEquipItem01[Index].slot1,
                            gOneKeyEquipItem01[Index].slot2,
                            gOneKeyEquipItem01[Index].slot3,
                            gOneKeyEquipItem01[Index].slot4,
                            isEquip,
                            equipType,
                            Id
                        );
                        if (*equipType == 136 && !AccessorFlag) {
                            AccessorFlag = 1;
                            SendEquipCommand(gOneKeyEquipItem01[Index], 0, 0, 80, 0, TotalRun);
                        }
                        else if (*equipType == 136 && AccessorFlag) {
                            SendEquipCommand(gOneKeyEquipItem01[Index], 0, 0, 80, 128, TotalRun);
                            AccessorFlag = 0;
                        }
                        else {
                            SendEquipCommand(gOneKeyEquipItem01[Index], 0, 0, 80, 0, TotalRun);
                        }

                    }
                    Sleep(200);
                }
                // 防止下壓太久
                while (IsRoFocus() && GetKeyState(gOneKeyEquipScanCode01) & 0x8000) {
                    Sleep(10);
                }
            
            }

            if (IsRoFocus() && gOneKeyEquipSwitch02 && GetKeyState(gOneKeyEquipScanCode02) & 0x8000) {
                printf("偵測到一鍵切裝02\n");

                for (int TotalRun = 0; TotalRun < 2; TotalRun++) { // 總共檢查2輪
                    int AccessorFlag = 0; // 飾品判斷flag
                    for (int Index = 0; Index < 20; Index++) { // 20件裝備
                        // 切裝
    
                        // 檢查裝備是否是左右飾品
                        ItemDataList.search_equip_data(
                            gOneKeyEquipItem02[Index].refineView,
                            gOneKeyEquipItem02[Index].itemId,
                            gOneKeyEquipItem02[Index].slot1,
                            gOneKeyEquipItem02[Index].slot2,
                            gOneKeyEquipItem02[Index].slot3,
                            gOneKeyEquipItem02[Index].slot4,
                            isEquip,
                            equipType,
                            Id
                        );
                        if (*equipType == 136 && !AccessorFlag) {
                            AccessorFlag = 1;
                            SendEquipCommand(gOneKeyEquipItem02[Index], 0, 0, 80, 0, TotalRun);
                        }
                        else if (*equipType == 136 && AccessorFlag) {
                            SendEquipCommand(gOneKeyEquipItem02[Index], 0, 0, 80, 128, TotalRun);
                            AccessorFlag = 0;
                        }
                        else {
                            SendEquipCommand(gOneKeyEquipItem02[Index], 0, 0, 80, 0, TotalRun);
                        }
                  
                    }
                    Sleep(200);
                }
                // 防止下壓太久
                while (IsRoFocus() && GetKeyState(gOneKeyEquipScanCode02) & 0x8000) {
                    Sleep(10);
                }

            }


        }

        Sleep(10);
    }

    return EXIT_SUCCESS;
}


BOOL CheckSpecialMonsterFunction (char CharNameStr[CharNameLen], char CharNameStr2[CharNameLen]) {

    if (!strcmp(CharNameStr, gNearAttackMonsterSpecial01)
     || !strcmp(CharNameStr, gNearAttackMonsterSpecial02)
     || !strcmp(CharNameStr, gNearAttackMonsterSpecial03)
     || !strcmp(CharNameStr, gNearAttackMonsterSpecial04)
     || !strcmp(CharNameStr, gNearAttackMonsterSpecial05)
     || !strcmp(CharNameStr2, gNearAttackMonsterSpecial01)
     || !strcmp(CharNameStr2, gNearAttackMonsterSpecial02)
     || !strcmp(CharNameStr2, gNearAttackMonsterSpecial03)
     || !strcmp(CharNameStr2, gNearAttackMonsterSpecial04)
     || !strcmp(CharNameStr2, gNearAttackMonsterSpecial05)) {
        return 1;
    }
    return 0;
}

BOOL CheckIgnoreMonsterFunction(char CharNameStr[CharNameLen], char CharNameStr2[CharNameLen]) {

    if (!strcmp(CharNameStr, gNearAttackMonsterIgnore01)
        || !strcmp(CharNameStr, gNearAttackMonsterIgnore02)
        || !strcmp(CharNameStr, gNearAttackMonsterIgnore03)
        || !strcmp(CharNameStr, gNearAttackMonsterIgnore04)
        || !strcmp(CharNameStr, gNearAttackMonsterIgnore05)
        || !strcmp(CharNameStr2, gNearAttackMonsterIgnore01)
        || !strcmp(CharNameStr2, gNearAttackMonsterIgnore02)
        || !strcmp(CharNameStr2, gNearAttackMonsterIgnore03)
        || !strcmp(CharNameStr2, gNearAttackMonsterIgnore04)
        || !strcmp(CharNameStr2, gNearAttackMonsterIgnore05)) {
        return 1;
    }
    return 0;
}

BOOL CheckIgnoreMonsterFunction2(char CharNameStr[CharNameLen], char CharNameStr2[CharNameLen]) {

    if (!strcmp(CharNameStr, gNearAttackMonsterSpecial01)
        || !strcmp(CharNameStr, gNearAttackMonsterSpecial02)
        || !strcmp(CharNameStr, gNearAttackMonsterSpecial03)
        || !strcmp(CharNameStr, gNearAttackMonsterSpecial04)
        || !strcmp(CharNameStr, gNearAttackMonsterSpecial05)
        || !strcmp(CharNameStr2, gNearAttackMonsterSpecial01)
        || !strcmp(CharNameStr2, gNearAttackMonsterSpecial02)
        || !strcmp(CharNameStr2, gNearAttackMonsterSpecial03)
        || !strcmp(CharNameStr2, gNearAttackMonsterSpecial04)
        || !strcmp(CharNameStr2, gNearAttackMonsterSpecial05)) {
        return 1;
    }
    return 0;
}
//gItemExceptionAllSetting

VOID PickUpItemException(VOID) {
    vector<PickUpItemControl> PickUpItemControls;   // A place to store the list of students
    PickUpItemControl PickUpItem;                  // A place to store data of one student
    
    int ItemExceptionCount = 0;

    ifstream fin(".\\data\\pickupitems.txt");   // Open the file
    ifstream finline(".\\data\\pickupitems.txt");   // Open the file
    gItemExceptionLine = 0;
    string tmps;
    while (getline(finline, tmps)) {
        gItemExceptionLine++;//一行一行讀，每讀一行就加一
    }
    finline.close();
    printf("抓取排除物品行數: %d\n", gItemExceptionLine);

    //delete [] gItemException;
    gItemException = new PickUpItemControl[gItemExceptionLine];
    int FirstLine = 1;
    while (fin >> PickUpItem.ItemId >> PickUpItem.PickUp) {
        if (FirstLine) {
            FirstLine = 0;
            gItemExceptionAllSetting = PickUpItem.PickUp;
        }
        else { // 新增物品
            gItemException[ItemExceptionCount].ItemId = PickUpItem.ItemId;
            gItemException[ItemExceptionCount].PickUp = PickUpItem.PickUp;
            printf("新增物品: %d %d\n", gItemException[ItemExceptionCount].ItemId, gItemException[ItemExceptionCount].PickUp);
            ItemExceptionCount++;
        }
        PickUpItemControls.push_back(PickUpItem);
    }
    fin.close();
}

int IsExceptionPickUpItemOrNot (int ItemNameId) {
    for (int Count = 0; Count < gItemExceptionLine; Count++) {
        if (gItemException[Count].ItemId == ItemNameId && gItemException[Count].PickUp == 0) {
            return 1;
        }
        if (gItemException[Count].ItemId == ItemNameId && gItemException[Count].PickUp == 2) {
            return 2;
        }
        if (gItemException[Count].ItemId == ItemNameId && gItemException[Count].PickUp != 0) {
            return 0;
        }
    }

    if (gItemExceptionAllSetting == 0) {
        return 1;
    }

    return 0;
}

VOID ReadPacketFile(VOID) {
    vector<RecvpacketControl> PacketControls;   // A place to store the list of students
    RecvpacketControl PacketControlValue;                  // A place to store data of one student

    int PacketControlCount = 0;

    ifstream fin(".\\data\\recvpackets.txt");   // Open the file
    ifstream finline(".\\data\\recvpackets.txt");   // Open the file
    gPcaketControlLine = 0;
    string tmps;
    while (getline(finline, tmps)) {
        gPcaketControlLine++;//一行一行讀，每讀一行就加一
    }
    finline.close();
    printf("總封包行數: %d\n", gPcaketControlLine);

    gPcaketControl = new RecvpacketControl[gPcaketControlLine];
    while (fin >> hex >> PacketControlValue.PacketName >> dec >> PacketControlValue.PacketLen1 >> PacketControlValue.PacketLen2 >> PacketControlValue.PacketLen3) {

        gPcaketControl[PacketControlCount].PacketName = PacketControlValue.PacketName;
        gPcaketControl[PacketControlCount].PacketLen1 = PacketControlValue.PacketLen1;
        printf("新增封包: %d %d\n", gPcaketControl[PacketControlCount].PacketName, gPcaketControl[PacketControlCount].PacketLen1);
        PacketControlCount++;

        PacketControls.push_back(PacketControlValue);
    }
    fin.close();
}

int SearchRcvPacketName(int PacketName, int* PacketLen) {
    //vector<RecvpacketControl> RecvpacketControls;   // A place to store the list of students
    //RecvpacketControl Recvpacket;                   // A place to store data of one student
    //ifstream fin(".\\data\\recvpackets.txt");    // Open the file

    //while (fin >> hex >> Recvpacket.PacketName >> dec >> Recvpacket.PacketLen1 >> Recvpacket.PacketLen2 >> Recvpacket.PacketLen3) {

    //    if (Recvpacket.PacketName == PacketName) {
    //        *PacketLen = Recvpacket.PacketLen1;
    //        fin.close();
    //        return 1;
    //    }
    //    RecvpacketControls.push_back(Recvpacket);
    //}
    //fin.close();
    int PacketControlCount = 0;
    while (PacketControlCount <= gPcaketControlLine) {
        if (gPcaketControl[PacketControlCount].PacketName == PacketName) {
            *PacketLen = gPcaketControl[PacketControlCount].PacketLen1;
            return 1;
        }
        PacketControlCount++;
    }

    return 0;
}

BOOL ConvenioFunction(void) {

#if !PrivateServerOrNot
    char SkillBuffer[12] = { 0x0c, 0x00, 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
#else
    char SkillBuffer[10] = { 0x38, 0x04, 0x03, 0x00, 0xF6, 0x08, 0x00, 0x00, 0xAC, 0x00 };
#endif
    gConvenioTrigger = 1;

    // 集結隊長需等1秒
    if (gConvenioLeaderSwitch) {
        Sleep(1000);
    }
    else {
        Sleep(1500);
    }

    for (int ConvenioIndex = 0; ConvenioIndex < 14; ConvenioIndex++) {
        //
        // 被集結自動放招
        //
        if (gConvenioSwitch[ConvenioIndex] && gConvenioSkill[ConvenioIndex] && gConvenioLv[ConvenioIndex]) {
            if (gConvenioJob[ConvenioIndex] == 0) { // 對自己施放
                for (int SkillUse = 0; SkillUse < 15; SkillUse++) {
                    gSkillIdAck = gConvenioSkill[ConvenioIndex];
                    gSkillAck = 0;
                    SendSkillCastFunction(gConvenioSkill[ConvenioIndex], gConvenioLv[ConvenioIndex], *AidAddress);
                    Sleep(gConvenioDelay[ConvenioIndex]);
                    if (gSkillAck) { // 成功
                        break;
                    }
                }
            }
            else if (gConvenioJob[ConvenioIndex] != 9999) { // 看設定職業
                for (int i = 0; i < gPartyInfoLen; i++) {
                    if (gPartyInfoJob[i] == gConvenioJob[ConvenioIndex] && gPartyInfoOnline[i]) {
                        for (int SkillUse = 0; SkillUse < 15; SkillUse++) {
                            gSkillIdAck = gConvenioSkill[ConvenioIndex];
                            gSkillAck = 0;
                            SendSkillCastFunction(gConvenioSkill[ConvenioIndex], gConvenioLv[ConvenioIndex], gPartyInfoAid[i]);
                            Sleep(gConvenioDelay[ConvenioIndex]);
                            if (gSkillAck) { // 成功
                                break;
                            }
                        }
                    }
                }
            }
            else { // 對組隊內所有人施放
                for (int i = 0; i < gPartyInfoLen; i++) {
                    if (gPartyInfoOnline[i]) {
                        for (int SkillUse = 0; SkillUse < 15; SkillUse++) {
                            gSkillIdAck = gConvenioSkill[ConvenioIndex];
                            gSkillAck = 0;
                            SendSkillCastFunction(gConvenioSkill[ConvenioIndex], gConvenioLv[ConvenioIndex], gPartyInfoAid[i]);
                            Sleep(gConvenioDelay[ConvenioIndex]);
                            if (gSkillAck) { // 成功
                                break;
                            }
                        }
                    }
                }
            }
            

        }

    }

    // 轉AP給隊長
    if (gLeaderApGive) {
        while (gAutoMonsterTeleIdleCounter > 0) {
            Sleep(500);
            SendSkillCastFunction(5368, 5, gLeaderAid);
        }
    }

    if (gConvenioLeaderSwitch) {
        Sleep(gConvenioLeaderDelay);
    }

    gConvenioTrigger = 0;

    return EXIT_SUCCESS;
}

BOOL DieNoteFunction(void) {

    // 建立死亡筆記本
    char DieNoteStr[48] = ".\\data\\log\\DieLog_";
    char txt[5] = ".txt";

    char Name[24] = "";

    strcat(DieNoteStr, gCharNameStr);
    strcat(DieNoteStr, txt);
    ofstream fout(DieNoteStr, ios::app);   // Open the file

    GetLocalTime(&sys);
    fout << sys.wYear << "/" << sys.wMonth << "/" << sys.wDay << " " << sys.wHour << ":" << sys.wMinute << ":" << sys.wSecond << " ";

    int FindName = TargetAidExpReportList.search_target_name(gDieNoteAid, Name);
    if (FindName) {
        fout << "你被 " << Name << " 殺死了, 傷害: " << gDieNoteDamage;
    }
    else {
        fout << "你被 " << gDieNoteAid << " 殺死了, 傷害: " << gDieNoteDamage;
    }

    if (gDieNoteSkill) {
        char SkillName[100] = "";
        int FindSkill = FindSkillName(gDieNoteSkill, SkillName);
        if (FindSkill) {
            fout << ", 技能: " << SkillName;
        }
        else {
            fout << ", 技能: " << gDieNoteSkill;
        }
    }

    fout << endl;
    fout.close();

    gDieNoteAid = 0;
    gDieNoteDamage = 0;
    gDieNoteSkill = 0;

    return EXIT_SUCCESS;
}

BOOL NearAttackScreenMonsterFunction(void) {

    // 如果有開螢幕怪物幾隻判定, 多停200毫秒去抓怪物數量
    if (gNearAttackScreenMonsterSwitch && gAutoTeleSwitch) {
        Sleep(300);
        Sleep(gNearAttackMonsterDelay);
    }
    else {
        Sleep(gNearAttackMonsterDelay);
        gNearAttackScreenMonsterTrigger = 0;
        return 0;
    }
    
    gNearAttackScreenMonsterTrigger = 0;
    gAutoTeleIdleCounter = 0;

    int TargetAmount = TargetAidList.search_target_Type_Amount(5);
    if (TargetAmount < gNearAttackScreenMonster && gAutoMonsterTeleIdleCounter == 0) {
        Sleep(gAutoTeleIdle);
        TargetAmount = TargetAidList.search_target_Type_Amount(5);
        if (TargetAmount < gNearAttackScreenMonster && gAutoMonsterTeleIdleCounter == 0) {
            printf("觸發瞬移\n");
            //TargetAidList.clear_target_node();
            gAutoTeleIdleCounter = 0;
            gAutoFindTargetAndAttackTrigger = 1;

            ClearTriggerFlag();
//#if !PrivateServerOrNot
//            PlayerAidList.clear_player_node();
//#endif
            for (int SkillUseCount = 0; SkillUseCount < 1; SkillUseCount++) {
                //SendMessage(gHwnd, WM_KEYDOWN, gAutoTeleScanCode, 0);
                //if (gAutoTeleEnter) {
                //    Sleep(200);
                //    SendMessage(gHwnd, WM_KEYDOWN, 13, 0);
                //} else if (gAutoTeleSpace) {
                //    Sleep(200);
                //    SendMessage(gHwnd, WM_KEYDOWN, 32, 0);
                //}
                gAutoTelePointTrigger = 1;
            }

            gNearAttackScreenMonsterTrigger = 0;
            gAutoTeleIdleCounter = 0;

            return 1;
        }
    }

    gNearAttackScreenMonsterTrigger = 0;
    gAutoTeleIdleCounter = 0;

    return 0;
}

// 自動打怪演算法
BOOL AutoFindTargetAndAttack(void) {
    int *TargetAid = new int;
    int *CorrdX = new int;
    int *CorrdY = new int;
    int* MoveXByLos = new int;
    int* MoveYByLos = new int;
    int* TargetDist = new int;

    int MoveX;
    int MoveY;
    int RandMapMoveX = 0;
    int RandMapMoveY = 0;

    float LosDist = 0;

    while (1) {

        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW
            && *HPIndexValue > 1
            && CloakingTryNum >= 3
            && CheckState()
            && !gStorageStatus
            && gNearAttackMonster
            //&& gAutoFindTargetSmartSwitch
            && !gIsTeleport
            && gNearAttackSwitch
            && !gTeleportTrigger
            && !gAutoTelePointTrigger
        ) {
            // 自動補狀態
            // 
            AutoBufferFunction(1);

            // 檢查集結時, 是否已經獲得需要的狀態
            if (gAutoMonsterTeleIdleCounter > 0 && (gLeaderDelay - gAutoMonsterTeleIdleCounter) > 3000) {
                int CanGoFlag = 0;
                for (int i = 0; i < 5; i++) {
                    if (gConvenioGetBufferStopSwitch[i]) {
                        if (CheckDebuffState(gConvenioGetBufferStopBuffer[i])) {
                            CanGoFlag = 1;
                        }
                        else {
                            CanGoFlag = 0;
                            break;
                        }
                    }

                }
                if (CanGoFlag) {
                    gAutoMonsterTeleIdleCounter = 0;
                }
            }

            //
            // 自動找怪
            //
            *TargetAid = 0;
            LosDist = 0;
            int FindTargetFlag = 0;
            int CanAttack = 1;
            *MoveXByLos = 0;
            *MoveYByLos = 0;
            if (gHeight > 0 && gWidth > 0) {
                FindTargetFlag = TargetAidList.search_target_Type_Los(5, TargetAid, CorrdX, CorrdY, MoveXByLos, MoveYByLos);
            }
            else {
                FindTargetFlag = TargetAidList.search_target_Type(5, TargetAid, CorrdX, CorrdY);
            }

            if (FindTargetFlag) { // 計算目標障礙物距離
                LosDist = CalculateTwoPointLength(*PLAYER_CORRD_X, *PLAYER_CORRD_Y, *CorrdX, *CorrdY, MoveXByLos, MoveYByLos, 0, 7);
                // 檢查目標與我們中間是否有障礙物
                CanAttack = CheckLos(*PLAYER_CORRD_X, *PLAYER_CORRD_Y, *CorrdX, *CorrdY);
            }

            if (*TargetAid && PlayerAidList.search_player(*TargetAid)) { // 目標在白名單 刪除
                TargetAidList.delete_target_node(*TargetAid);
            }
            else if (*TargetAid && LosDist > 30) { // 目標障礙物距離超過30
                TargetAidList.delete_target_node(*TargetAid);
            }
            else if (*TargetAid && !PlayerAidList.search_player(*TargetAid)) {
                gAutoTeleIdleCounter = 0; // 計時器歸零
                gAutoFindTargetAndAttackTrigger = 1;
                // 計算距離
                int Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                float DistFloat = sqrt(Dist);
                if (!gAutoMonsterTeleIdleCounter) {
                    printf("找到怪!! AID: %d, 座標 (%d, %d), 距離: %3.2f\n", *TargetAid, *CorrdX, *CorrdY, DistFloat);
                }

                if (gNearAttackScreenMonsterTrigger && gAutoMonsterTeleIdleCounter == 0) {
                    if (NearAttackScreenMonsterFunction()) { // 因為怪物不足觸發瞬移的話, 跳過下面打怪
                        continue;
                    }
                }

                //if (gTeleportTrigger || gNearAttackScreenMonsterTrigger) {
                //    continue;
                //}

                if (Dist <= (gNearAttackDist * gNearAttackDist) && CanAttack) {
                   

                    if (gAutoMonsterTeleIdleCounter > 0 && gLeaderAttackStopSwitch) { // 當被集結且開關開起來, 就不打怪
                    
                    }
                    else {
                        printf("攻擊!!\n");
                        gAutoTeleLosTryCountCurrent = 0;
                        // 在距離內, 攻擊目標
                        // 
                        
                        // 自動壓血
                        if (gNearAttackSkill == 2343 && gSmartBloodSwitch) {
                            gSmartBloodTrigger = 1;
                        }
                        if (gAutoFindTargetSmartSwitch) { // 偷竊
                            // 原招式
                            
                            if (!gNearAttackGround) { // 非地板技能
                                if (!gNearAttackSupport) {
                                    SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *TargetAid);
                                }
                                else {
                                    SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *AidAddress);
                                }
                            }
                            Sleep(gNearAttackDelay);

                            if (gNearAttackAck == 0) {

                                // 再丟一次
                                if (!gNearAttackGround) { // 非地板技能
                                    if (!gNearAttackSupport) {
                                        SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *TargetAid);
                                    }
                                    else {
                                        SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *AidAddress);
                                    }
                                }
                                Sleep(gNearAttackDelay);

                                if (gNearAttackAck == 0) { // 偷竊前的招式失敗
                                    TargetAidList.delete_target_node(*TargetAid);
                                    continue;
                                }
                            }

                            gSteal = 0;
                            int i = 0;
                            for (i = 0; i < 10; i++) {

                                SendSkillCastFunction(50, 10, *TargetAid);
                                Sleep(300);
                                if (gSteal || !TargetAidList.search_target(*TargetAid)) {
                                    gSteal = 0;
                                    break;
                                }

                            }
                            if (i == 10) { // 偷竊偷到10次還沒中
                                //TargetAidList.delete_target_node(*TargetAid);
                            }
                        }

                        for (int SkillUseCount = 0; SkillUseCount < gNearAttackCount; SkillUseCount++) {

                            if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || gAutoTelePointTrigger) {
                                break;
                            }
                            if (!TargetAidList.search_target(*TargetAid)) {
                                if (!gAutoItemTakeSwitch) {
                                    break;
                                }
                                // 如果有開撿物品, 停一下把物品撿完
                                else if (gAutoMonsterTeleIdleCounter == 0) {
                                    while (1) {
                                        *TargetDist = 0;
                                        int TackItemAid = ItemAidList.search_target_Type_ItemPickUp(TargetDist);

                                        if (gAutoTelePointTrigger) { // 遇到瞬移情況
                                            break;
                                        }

                                        if (TackItemAid == -1) { // 貪婪
                                            SendMessage(gHwnd, WM_KEYDOWN, gAutoGreedScanCode, 0);
                                            for (int SleepCount = 0; SleepCount < gAutoItemTakeSmart; SleepCount++) {
                                                Sleep(1);
                                                gAutoTeleIdleCounter = 0;
                                            }
                                        }
                                        else if (TackItemAid > 0) { // 撿取物品

                                            printf("自動撿物品 %d \n", TackItemAid);

                                            gAutoTeleIdleCounter = 0;
                                            SendItemTakePackageFunction(TackItemAid);

                                            for (int SleepCount = 0; SleepCount < gAutoItemTakeSmart; SleepCount++) {
                                                Sleep(1);
                                                gAutoTeleIdleCounter = 0;
                                            }
                                           
                                            //if (*TargetDist < 3 * 3) {
                                            //    Sleep(gAutoItemTakeSmart);
                                            //}
                                            //else if (*TargetDist < 7 * 7) {
                                            //    Sleep(1000);
                                            //}
                                            //else {
                                            //    Sleep(2000);
                                            //}
                                        }
                                        else { // 沒掃到物品, 跳出while
                                            break;
                                        }
                                    }
                                    break;
                                }
                                break;
                            }


                            if (gElemeMasterSuperSwitch) { // 元素破壞專用連打

                                int SkillLvCatch = SkillDataList.IsSkillIdCanUse(gCurrentSkillTable[gCurrentSkillCount]);
                                if (SkillLvCatch) {
                                    SendSkillGroundFunction(gCurrentSkillTable[gCurrentSkillCount], SkillLvCatch, *CorrdX, *CorrdY);
                                }

                                // 技能指針往下
                                gCurrentSkillCount++;
                                if (gCurrentSkillCount >= ElemeMasterSuper) {
                                    gCurrentSkillCount = 0;
                                }
                                
                            }
                            else if (gNearAttackSkill == -100) { // 普通攻擊

                                SendAttackWeaponPackageFunction(*TargetAid, 0);
                                gNearAttackAck = 0;
                                //Sleep(1000);
                            }
                            else if (!gNearAttackGround) { // 非地板招

                                if (!gNearAttackSupport) {
                                    SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *TargetAid);
                                }
                                else {
                                    SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *AidAddress);
                                }
                            }
                            else { // 地板招

                                SendSkillGroundFunction(gNearAttackSkill, gNearAttackSkilv, *CorrdX, *CorrdY);
                            }

                            Sleep(gNearAttackDelay);

                            //
                            // 重新確定距離
                            // 取得目標的X ,Y座標
                            //
                            int IsFind = TargetAidList.search_target_Aid(*TargetAid, CorrdX, CorrdY);
                            if (IsFind) {
                                Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                                if (Dist > (gNearAttackDist * gNearAttackDist)) {
                                    break;
                                }
                            }
                            else {
                                break;
                            }

                        }

                        //gAutoMonsterTeleIdleCounter = 1000;
                        if (gMoveRandSwitch && PrivateServerOrNot && gNearAttackCount < 30) {
                            if (gNearAttackAck == 0 && gNearAttackAckTotal > 8) {
                                TargetAidList.delete_target_node(*TargetAid);
                            }
                        }
                        else if (gNearAttackAck == 0 && gNearAttackAckTotal > 2 && gNearAttackCount < 30) {
                            TargetAidList.delete_target_node(*TargetAid);
                        }
                    }
                }
                else if ( gMoveSwitch && *MoveXByLos && *MoveYByLos && Dist <= (gMoveDist * gMoveDist) && gAutoMonsterTeleIdleCounter == 0) {
                    
                    int PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                    int PlayerCorrdYBackUp = *PLAYER_CORRD_Y;
                    MoveX = *MoveXByLos;
                    MoveY = *MoveYByLos;

                    printf("嘗試規劃移動!! ( %d, %d)\n", MoveX, MoveY);

                    if (gBodyRelocation && CheckDebuffState(410)) {
                        SendSkillGroundFunction(264, 1, MoveX, MoveY);
                    }
                    else {
                        MoveFunction(MoveX, MoveY);

                    }
                    Sleep(gMoveDelay);

                    if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y && TargetAidList.search_target(*TargetAid)) { // 移動失敗

                        if (!gBodyRelocation) {
                            if (!gMoveRandSwitch && PrivateServerOrNot && gNearAttackCount < 30) {
                                TargetAidList.delete_target_node(*TargetAid);
                            }
                        }
                        else if (gBodyRelocation && gAutoTeleLosTryCountCurrent > 4 && gNearAttackCount < 30) {
                            TargetAidList.delete_target_node(*TargetAid);
                        }
                        printf("規劃移動失敗!\n");

                        gAutoTeleLosTryCountCurrent++;

                        if (gAutoTeleSwitch && gAutoTeleLOS && gAutoMonsterTeleIdleCounter == 0 && gAutoTeleLosTryCountCurrent >= gAutoTeleLosTryCount) {
                            gAutoTeleLosTryCountCurrent = 0;

                            TelePortFunction();
                        }
                    }
                    else { // 移動成功
                        gAutoTeleLosTryCountCurrent = 0;
                    }
                }
                else if (gMoveSwitch && Dist <= (13 * 13)
                    && Dist <= (gMoveDist * gMoveDist)
                    && gAutoMonsterTeleIdleCounter == 0) {
                    // 在距離外, 往目標前進 小移動
                    // 最多走10格
                    printf("嘗試走過去!!\n");
                    int PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                    int PlayerCorrdYBackUp = *PLAYER_CORRD_Y;
                    MoveX = *CorrdX;
                    MoveY = *CorrdY;

                    if (gBodyRelocation && CheckDebuffState(410)) {
                        SendSkillGroundFunction(264, 1, MoveX, MoveY);
                    }
                    else {
                        MoveFunction(*CorrdX, *CorrdY);
                    }
                    Sleep(gMoveDelay);

                    if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y && TargetAidList.search_target(*TargetAid)) { // 移動失敗
                        if (!gMoveRandSwitch && PrivateServerOrNot && gNearAttackCount < 30) {
                            TargetAidList.delete_target_node(*TargetAid);
                        }
                        printf("小移動失敗!\n");

                        gAutoTeleLosTryCountCurrent++;

                        if (gAutoTeleSwitch && gAutoTeleLOS && gAutoMonsterTeleIdleCounter == 0 && gAutoTeleLosTryCountCurrent >= gAutoTeleLosTryCount) {
                            gAutoTeleLosTryCountCurrent = 0;
                            TelePortFunction();
                        }
                    }
                    else { // 移動成功
                        gAutoTeleLosTryCountCurrent = 0;
                    }
                }
                else if (gMoveSwitch && Dist <= (gMoveDist * gMoveDist)
                    && gAutoMonsterTeleIdleCounter == 0) {
                    // 在距離外, 往目標前進 大移動
                    // 最多走10格
                    printf("嘗試走過去!!\n");
                    int PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                    int PlayerCorrdYBackUp = *PLAYER_CORRD_Y;
                    MoveX = (*PLAYER_CORRD_X + ((*CorrdX - *PLAYER_CORRD_X) / 2));
                    MoveY = (*PLAYER_CORRD_Y + ((*CorrdY - *PLAYER_CORRD_Y) / 2));

                    if (gBodyRelocation && CheckDebuffState(410)) {
                        SendSkillGroundFunction(264, 1, MoveX, MoveY);
                    }
                    else {
                        MoveFunction(*CorrdX, *CorrdY);
                    }

                    Sleep(gMoveDelay);

                    if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y && TargetAidList.search_target(*TargetAid)) { // 移動失敗
                        if (!gMoveRandSwitch && PrivateServerOrNot && gNearAttackCount < 30) {
                            TargetAidList.delete_target_node(*TargetAid);
                        }
                        printf("大移動失敗!\n");

                        gAutoTeleLosTryCountCurrent++;

                        if (gAutoTeleSwitch && gAutoTeleLOS && gAutoMonsterTeleIdleCounter == 0 && gAutoTeleLosTryCountCurrent >= gAutoTeleLosTryCount) {
                            gAutoTeleLosTryCountCurrent = 0;
                            TelePortFunction();
                        }
                    }
                    else { // 移動成功
                        gAutoTeleLosTryCountCurrent = 0;
                    }
                }
                else if (gAutoTeleSwitch && gAutoMonsterTeleIdleCounter == 0) {
                    // 都太遠
                    printf("怪物距離太遠 飛走!!\n");
                    //gTeleportTrigger = 1;
                    TargetAidList.delete_target_node(*TargetAid);
                    //TargetAidList.update_target_Corrd(*TargetAid, 999999, 999999);
                    //gAutoMonsterTeleIdleCounter = 0;
                    //gAutoTeleIdleCounter = 100000;
                }
                gAutoTeleIdleCounter = 0; // 計時器歸零
                gAutoFindTargetAndAttackTrigger = 0;
                //gAutoMonsterTeleIdleCounter = 100;
            }
        }

        // 瞬移判定
        if ((gTeleportTrigger && gAutoMonsterTeleIdleCounter == 0) || (gAutoTeleSwitch && gTotalSwitch && gAutoTeleIdleCounter > gAutoTeleIdle
            && gAutoMonsterTeleIdleCounter == 0)) {
            debug("自動瞬移找怪");

            gTeleportTrigger = 0;
            ClearTriggerFlag();
            gIsTeleport = 1;
            for (int SkillUseCount = 0; SkillUseCount < 1; SkillUseCount++) {
                //if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1 || (*HPIndexValue * 100 / *MaxHPTableValue < 30)) { // 生命小於30%不瞬移
                if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1) {
                    break;
                }
                //
                // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                //
                if (gAutoMonsterTeleIdleCounter || CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438) || gAutoTeleIdleCounter < gAutoTeleIdle) {
                    break;
                }
                TelePortFunction();
                Sleep(300);
            }
            gAutoTeleIdleCounter = 0;
            gIsTeleport = 0;

        }

        // 自動地圖隨機移動
        if (gMoveSwitch && gMoveRandSwitch && !gAutoTeleSwitch && gTotalSwitch && gAutoTeleIdleCounter > gAutoTeleIdle && gAutoMonsterTeleIdleCounter == 0) {

            if (gHeight > 0 && gWidth > 0) { // 必須loading到地圖

                if (!RandMapMoveX || !RandMapMoveY) {
                    /* 指定亂數範圍 */
                    int min = 1;
                    int max = gWidth - 1;
                    RandMapMoveX = rand() % (max - min + 1) + min;
                    max = gHeight - 1;
                    RandMapMoveY = rand() % (max - min + 1) + min;
                }

                CalculateTwoPointLength(*PLAYER_CORRD_X, *PLAYER_CORRD_Y, RandMapMoveX, RandMapMoveY, MoveXByLos, MoveYByLos, 1, 7);

                if (*MoveXByLos && *MoveYByLos) {

                    printf("地圖隨機移動!! ( %d, %d) ( %d, %d)\n", RandMapMoveX, RandMapMoveY, *MoveXByLos, *MoveYByLos);

                    int PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                    int PlayerCorrdYBackUp = *PLAYER_CORRD_Y;

                    MoveFunction(*MoveXByLos, *MoveYByLos);

                    Sleep(gMoveDelay);

                    if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y) {
                        // 移動失敗, 重新找地圖座標移動
                        RandMapMoveX = 0;
                        RandMapMoveY = 0;
                    }

                }
                else {
                    // 路徑有誤
                    RandMapMoveX = 0;
                    RandMapMoveY = 0;
                }

            }
            else { // 沒有讀取到地圖檔
                /* 指定亂數範圍 */

                if (!RandMapMoveX || !RandMapMoveY) {
                    int min = - 7;
                    int max = + 7;
                    RandMapMoveX = rand() % (max - min + 1) + min;
                    RandMapMoveY = rand() % (max - min + 1) + min;

                    if (RandMapMoveX > 0 && RandMapMoveX < 7) {
                        RandMapMoveX = 7;
                    }
                    else if (RandMapMoveX < 0 && RandMapMoveX > -7) {
                        RandMapMoveX = -7;
                    }

                    if (RandMapMoveY > 0 && RandMapMoveY < 7) {
                        RandMapMoveY = 7;
                    }
                    else if (RandMapMoveY < 0 && RandMapMoveY > -7) {
                        RandMapMoveY = -7;
                    }
                }

                int PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                int PlayerCorrdYBackUp = *PLAYER_CORRD_Y;

                MoveFunction(*PLAYER_CORRD_X + RandMapMoveX, *PLAYER_CORRD_Y + RandMapMoveY);

                Sleep(gMoveDelay);

                if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y) {
                    // 移動失敗, 重新找地圖座標移動
                    RandMapMoveX = 0;
                    RandMapMoveY = 0;
                }
            }
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

// 自動攻擊玩家判定function
BOOL AttackToPlayerFunction(void) {
    int* TargetAid = new int;
    int* CorrdX = new int;
    int* CorrdY = new int;
    //
    // 自動找敵人玩家
    //
    *TargetAid = 0;
    TargetAidList.search_target_Type(0, TargetAid, CorrdX, CorrdY); // 找玩家

    if (*TargetAid) {
        if (gNearAttackAidMode && !IsEnemyAid2(*TargetAid)) {
            // 開啟只攻擊特定AID的模式, 非AID列表就直接跳過
            TargetAidList.delete_target_node(*TargetAid);
        }
        else if (!gNearAttackAidMode && PlayerAidList.search_player(*TargetAid)) {
            // 沒有開啟AID模式, 但目標為盟友
            TargetAidList.delete_target_node(*TargetAid);
        }
        else if (CheckTheAidIsInParty(*TargetAid)) {
            // 如果玩家在隊伍裡面, 忽略
            TargetAidList.delete_target_node(*TargetAid);
        }
        else {
            printf("弓身: 找到敵人玩家!! AID: %d, 座標 (%d, %d)\n", *TargetAid, *CorrdX, *CorrdY);
            // 計算距離
            int Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
            if (Dist <= (gNearAttackDist * gNearAttackDist)) {
                printf("攻擊敵人玩家!\n");

                for (int SkillUseCount = 0; SkillUseCount < gNearAttackCount; SkillUseCount++) {
                    if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || !TargetAidList.search_target(*TargetAid)) {
                        break;
                    }

                    if (gNearSkillPvpStopSwitch && gIgnoranceSuperSwitch && !SkillDelayPlayerList.search_player_2(*TargetAid, gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount])) {
                        // 先交替指針
                        if (gCurrentIgnoranceSkillCount == 0) {
                            gCurrentIgnoranceSkillCount = 1;
                        }
                        else {
                            gCurrentIgnoranceSkillCount = 0;
                        }

                        if (!SkillDelayPlayerList.search_player_2(*TargetAid, gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount])) {
                            // 再交替指針
                            if (gCurrentIgnoranceSkillCount == 0) {
                                gCurrentIgnoranceSkillCount = 1;
                            }
                            else {
                                gCurrentIgnoranceSkillCount = 0;
                            }
                            break;
                        }
                    }
                    else if (!gIgnoranceSuperSwitch && gNearSkillPvpStopSwitch && !SkillDelayPlayerList.search_player_2(*TargetAid, gNearAttackSkill)) {
                        break;
                    }

                    //
                    // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                    //
                    if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                        break;
                    }

                    // 此狀態不攻擊(PVP限定)
                    if (gNearAttackStopWhenState && CheckDebuffState(gNearAttackStopWhenState)) {
                        break;
                    }

                    if (gIgnoranceSuperSwitch) { // 無知衰弱連打
                        int SkillLvCatch = SkillDataList.IsSkillIdCanUse(gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount]);
                        if (SkillLvCatch) {
                            SendSkillCastFunction(gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount], SkillLvCatch, *TargetAid);
                        }
                    }
                    else {

                        if (gNearAttackSkill == -100) { // 普通攻擊

                            SendAttackWeaponPackageFunction(*TargetAid, 0);
                            break;
                        }

                        // 指定技能
                        if (!gNearAttackGround) {

                            if (!gNearAttackSupport) {
                                SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *TargetAid);
                            }
                            else {
                                SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *AidAddress);
                            }
                        }
                        // 地板技能
                        else if (gNearAttackGround) {

                            SendSkillGroundFunction(gNearAttackSkill, gNearAttackSkilv, *CorrdX, *CorrdY);
                        }
                    }
                    Sleep(gNearAttackDelay);

                    //
                    // 重新確定距離
                    // 取得目標的X ,Y座標
                    //
                    int IsFind = TargetAidList.search_target_Aid(*TargetAid, CorrdX, CorrdY);
                    if (IsFind) {
                        Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                        if (Dist > (gNearAttackDist * gNearAttackDist)) {
                            break;
                        }
                    }
                    else {
                        break;
                    }
                }
                if (gAttackPvpDot == 0) { // 連點模式 - 關
                    TargetAidList.delete_target_node(*TargetAid);
                }
                else if (gAttackPvpDot == 1 && gNearAttackAck == 0 && gNearAttackAckTotal > 2) { // 連點 - 普通模式
                    TargetAidList.delete_target_node(*TargetAid);
                }
                else if (gAttackPvpDot == 2) { // 連點 - 無情模式
                    // 直到目標消失
                }
                
            }
        }
    }
    delete TargetAid;
    delete CorrdX;
    delete CorrdY;

    return EXIT_SUCCESS;
}


// 自動鎖定打玩家 演算法
BOOL AutoAttackToPlayerFunction(void) {
    int* TargetAid = new int;
    int* CorrdX = new int;
    int* CorrdY = new int;
    while (1) {

        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW
            && *HPIndexValue > 1
            && CloakingTryNum >= 3
            && !gStopAutoAttackIdleCounter // 過傳點暫停
            && CheckState()
            && !gStorageStatus
            && gNearAttackSwitch
            && !gNearAttackMonster
            && IsPvpMap() // PVP地圖
            ) {
            //
            // 自動找敵人玩家
            //
            *TargetAid = 0;
            TargetAidList.search_target_Type(0, TargetAid, CorrdX, CorrdY); // 找玩家

            if (*TargetAid) {
                if (gNearAttackAidMode && !IsEnemyAid2(*TargetAid)) {
                    // 開啟只攻擊特定AID的模式, 非AID列表就直接跳過
                    TargetAidList.delete_target_node(*TargetAid);
                    continue;
                }
                if (!gNearAttackAidMode && PlayerAidList.search_player(*TargetAid)) {
                    // 沒有開啟AID模式, 但目標為盟友
                    TargetAidList.delete_target_node(*TargetAid);
                    continue;
                }
                if (CheckTheAidIsInParty(*TargetAid)) {
                    // 如果玩家在隊伍裡面, 忽略
                    TargetAidList.delete_target_node(*TargetAid);
                    continue;
                }

                //printf("找到敵人玩家!! AID: %d, 座標 (%d, %d)\n", *TargetAid, *CorrdX, *CorrdY);
                // 計算距離
                int Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                if (Dist <= (gNearAttackDist * gNearAttackDist)) {
                    printf("攻擊敵人玩家!\n");

                    for (int SkillUseCount = 0; SkillUseCount < gNearAttackCount; SkillUseCount++) {
                        if(*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || !TargetAidList.search_target(*TargetAid) || gStopAutoAttackIdleCounter) {
                            break;
                        }

                        if (gNearSkillPvpStopSwitch && gIgnoranceSuperSwitch && !SkillDelayPlayerList.search_player_2(*TargetAid, gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount])) {
                            // 先交替指針
                            if (gCurrentIgnoranceSkillCount == 0) {
                                gCurrentIgnoranceSkillCount = 1;
                            }
                            else {
                                gCurrentIgnoranceSkillCount = 0;
                            }
                            
                            if (!SkillDelayPlayerList.search_player_2(*TargetAid, gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount])) {
                                // 再交替指針
                                if (gCurrentIgnoranceSkillCount == 0) {
                                    gCurrentIgnoranceSkillCount = 1;
                                }
                                else {
                                    gCurrentIgnoranceSkillCount = 0;
                                }
                                break;
                            }
                        }
                        else if (!gIgnoranceSuperSwitch && gNearSkillPvpStopSwitch && !SkillDelayPlayerList.search_player_2(*TargetAid, gNearAttackSkill)) {
                            break;
                        }

                        //
                        // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                        //
                        if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                            break;
                        }
                        
                        // 此狀態不攻擊(PVP限定)
                        if (gNearAttackStopWhenState && CheckDebuffState(gNearAttackStopWhenState)) {
                            break;
                        }

                        /*if (SkillUseCount == 1) {
                            Sleep(50);
                        }*/

                        if (gElemeMasterSuperSwitch) { // 元素破壞專用連打 PVP

                            int SkillLvCatch = SkillDataList.IsSkillIdCanUse(gCurrentSkillTable[gCurrentSkillPvpCount]);
                            if (SkillLvCatch) {
                                SendSkillGroundFunction(gCurrentSkillTable[gCurrentSkillPvpCount], SkillLvCatch, *CorrdX, *CorrdY);
                            }

                            // 技能指針往下
                            gCurrentSkillPvpCount++;
                            if (gCurrentSkillPvpCount >= ElemeMasterSuperPvp) {
                                gCurrentSkillPvpCount = ElemeMasterSuper;
                            }

                        }
                        else if (gIgnoranceSuperSwitch) { // 無知衰弱連打
                            int SkillLvCatch = SkillDataList.IsSkillIdCanUse(gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount]);
                            if (SkillLvCatch) {
                                SendSkillCastFunction(gCurrentIgnoranceSkillTable[gCurrentIgnoranceSkillCount], SkillLvCatch, *TargetAid);
                            }
                        }
                        else {
                            if (gNearAttackSkill == -100) { // 普通攻擊

                                SendAttackWeaponPackageFunction(*TargetAid, 0);
                                break;
                            }

                            // 指定技能
                            if (!gNearAttackGround) {

                                if (!gNearAttackSupport) {
                                    SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *TargetAid);
                                }
                                else {
                                    SendSkillCastFunction(gNearAttackSkill, gNearAttackSkilv, *AidAddress);
                                }
                            }
                            // 地板技能
                            else if (gNearAttackGround) {

                                SendSkillGroundFunction(gNearAttackSkill, gNearAttackSkilv, *CorrdX, *CorrdY);
                            }
                        }
                        Sleep(gNearAttackDelay);

                        // 技能是SRB.4.4 吃營養品
                        if (gNearAttackSkill == 8012 && !PrivateServerOrNot) { // 8012#S.B.R.44#
                            if (gHomunIntimacySwitch && gHomunIntimacyAuto && CheckDebuffState(0x517)) { // 招喚生命體狀態
                                SendMessage(gHwnd, WM_KEYDOWN, gHomunIntimacyAuto, 0);
                            }
                        }

                        //
                        // 重新確定距離
                        // 取得目標的X ,Y座標
                        //
                        int IsFind = TargetAidList.search_target_Aid(*TargetAid, CorrdX, CorrdY);
                        if (IsFind) {
                            Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                            if (Dist > (gNearAttackDist * gNearAttackDist)) {
                                break;
                            }
                        }
                        else {
                            break;
                        }

                    }
                    if (gAttackPvpDot == 0) { // 連點模式 - 關
                        TargetAidList.delete_target_node(*TargetAid);
                    }
                    else if (gAttackPvpDot == 1 && gNearAttackAck == 0 && gNearAttackAckTotal > 2) { // 連點 - 普通模式
                        TargetAidList.delete_target_node(*TargetAid);
                    }
                    else if (gAttackPvpDot == 2) { // 連點 - 無情模式
                        // 直到目標消失
                    }
                    
                }
                else {
                    // 敵人距離太遠
                    //printf("敵人距離太遠!!\n");

                    //TargetAidList.delete_target_node(*TargetAid);

                }
            }
        }

        Sleep(1);
    }
    return EXIT_SUCCESS;
}

// 自動幫隊友輔助 演算法
BOOL AutoSupportToFriendFunction(void) {
    int* TargetAid = new int;
    int* CorrdX = new int;
    int* CorrdY = new int;
    int* MoveXByLos = new int;
    int* MoveYByLos = new int;

    int PlayerCorrdXBackUp = 0;
    int PlayerCorrdYBackUp = 0;
    while (1) {

        // 跟隨模式
        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW
            && *HPIndexValue > 1
            && CloakingTryNum >= 3
            && CheckState()
            && !gStorageStatus
            && gAutoFollowSwitch
            ) {
            //
            // 自動找跟隨目標
            //
            *TargetAid = 0;
            int IsFind = 0;
            if (gAutoFollowAid) {
                IsFind = TargetAidExpReportList.search_target_Aid(gAutoFollowAid, CorrdX, CorrdY); // 找玩家
            }
            else { // 跟隨隊長
                IsFind = TargetAidExpReportList.search_target_Aid(gLeaderAid, CorrdX, CorrdY); // 找玩家
            }

            if (IsFind) {

                // 計算距離
                int Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                if (Dist > (gAutoFollowDist * gAutoFollowDist)) {
                    if (gAutoFollowAid) {
                        printf("跟隨 %d!\n", gAutoFollowAid);
                    }
                    else {
                        printf("跟隨 %d!\n", gLeaderAid);
                    }

                    //MoveX = *MoveXByLos;
                    //MoveY = *MoveYByLos;
                    int MoveX = *CorrdX;
                    int MoveY = *CorrdY;

                    printf("嘗試跟隨移動!! ( %d, %d)\n", MoveX, MoveY);

                    PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                    PlayerCorrdYBackUp = *PLAYER_CORRD_Y;

                    MoveFunction(MoveX, MoveY);
                    
                    Sleep(500);

                    if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y && gHeight > 0 && gWidth > 0) { // 移動失敗

                        PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                        PlayerCorrdYBackUp = *PLAYER_CORRD_Y;

                        CalculateTwoPointLength(*PLAYER_CORRD_X, *PLAYER_CORRD_Y, *CorrdX, *CorrdY, MoveXByLos, MoveYByLos, 1, 5);
                        printf("移動失敗!! 嘗試跟隨移動!! ( %d, %d) ( %d, %d)\n", *CorrdX, *CorrdY, *MoveXByLos, *MoveYByLos);

                        MoveFunction(*MoveXByLos, *MoveYByLos);

                        Sleep(500);
                        if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y && gHeight > 0 && gWidth > 0) { // 移動失敗2
                            /* 指定亂數範圍 */
                            int min = -2;
                            int max = 2;
                            /* 產生 [min , max] 的整數亂數 */
                            int xRand = rand() % (max - min + 1) + min;
                            int yRand = rand() % (max - min + 1) + min;

                            //int TryCount = 0;
                            //while (gMapData[*PLAYER_CORRD_Y + yRand][*PLAYER_CORRD_X + xRand] != 0 && TryCount < 20) {
                            //    xRand = rand() % (max - min + 1) + min;
                            //    yRand = rand() % (max - min + 1) + min;

                            //    TryCount++;
                            //}
                            printf("隨機 嘗試跟隨移動!! ( %d, %d) ( %d, %d)\n", *CorrdX, *CorrdY, *PLAYER_CORRD_X + xRand, *PLAYER_CORRD_Y + yRand);

                            MoveFunction(*PLAYER_CORRD_X + xRand, *PLAYER_CORRD_Y + yRand);

                            Sleep(500);
                        }
                    }

                }
            }
            else if (gHeight > 0 && gWidth > 0) { // 從隊伍中尋找

                int IsFindInParty = 0;
                char FollowMap[16] = { "" };
                if (gAutoFollowAid) {
                    IsFindInParty = CheckTheAidIsInParty2(gAutoFollowAid, CorrdX, CorrdY, FollowMap); // 找玩家
                }
                else { // 跟隨隊長
                    IsFindInParty = CheckTheAidIsInParty2(gLeaderAid, CorrdX, CorrdY, FollowMap); // 找玩家
                }
                // 與你現在地圖一樣
                if (IsFindInParty) {

                    char CURRENT_MAP_NAME[16] = { "" }; // 定義地圖字串
                    GetCurrentMapName(CURRENT_MAP_NAME);

                    char delimiter[2] = ".";

                    strtok(CURRENT_MAP_NAME, delimiter);
                    strtok(FollowMap, delimiter);

                    if (!strcmp(CURRENT_MAP_NAME, FollowMap) && *CorrdX > 0 && *CorrdY > 0) { // 與你現在地圖一樣

                        CalculateTwoPointLength(*PLAYER_CORRD_X, *PLAYER_CORRD_Y, *CorrdX, *CorrdY, MoveXByLos, MoveYByLos, 1, 7);

                       

                        printf("嘗試跟隨移動!! ( %d, %d) ( %d, %d)\n", *CorrdX, *CorrdY, *MoveXByLos, *MoveYByLos);

                        PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                        PlayerCorrdYBackUp = *PLAYER_CORRD_Y;

                        MoveFunction(*MoveXByLos, *MoveYByLos);

                        Sleep(500);

                        if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y) { // 移動失敗
                            
                            PlayerCorrdXBackUp = *PLAYER_CORRD_X;
                            PlayerCorrdYBackUp = *PLAYER_CORRD_Y;

                            CalculateTwoPointLength(*PLAYER_CORRD_X, *PLAYER_CORRD_Y, *CorrdX, *CorrdY, MoveXByLos, MoveYByLos, 1, 2);
                            printf("移動失敗!! ");
                            printf("嘗試跟隨移動!! ( %d, %d) ( %d, %d)\n", *CorrdX, *CorrdY, *MoveXByLos, *MoveYByLos);

                            MoveFunction(*MoveXByLos, *MoveYByLos);

                            Sleep(500);
                            if (PlayerCorrdXBackUp == *PLAYER_CORRD_X && PlayerCorrdYBackUp == *PLAYER_CORRD_Y) { // 移動失敗2
                               /* 指定亂數範圍 */
                               int min = -5;
                               int max = 5;
                               /* 產生 [min , max] 的整數亂數 */
                               int xRand = rand() % (max - min + 1) + min;
                               int yRand = rand() % (max - min + 1) + min;

                               //int TryCount = 0;
                               //while (gMapData[*PLAYER_CORRD_Y + yRand][*PLAYER_CORRD_X + xRand] != 0 && TryCount < 20) {
                               //    xRand = rand() % (max - min + 1) + min;
                               //    yRand = rand() % (max - min + 1) + min;

                               //    TryCount++;
                               //}

                               MoveFunction(*PLAYER_CORRD_X + xRand, *PLAYER_CORRD_Y + yRand);

                               Sleep(500);
                            }
                        }

                    }
                }


                
            }
        }

        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW
            && *HPIndexValue > 1
            && CloakingTryNum >= 3
            && CheckState()
            && !gStorageStatus
            && gNearSupportSwitch
            ) {
            //
            // 自動找找盟友
            //
            *TargetAid = 0;
            TargetAidExpReportList.search_target_Type(0, TargetAid, CorrdX, CorrdY); // 找玩家

            if (*TargetAid) {
                if (gNearSupportAidOnly && !IsSpecialAid2(*TargetAid)) {
                    // 開啟只輔助特定AID的模式, 非AID列表就直接跳過
                    TargetAidExpReportList.delete_target_node(*TargetAid);
                    continue;
                }
                if (!gNearSupportAidOnly && !PlayerAidList.search_player(*TargetAid)) {
                    // 沒有開啟AID模式, 但目標物非盟友
                    TargetAidExpReportList.delete_target_node(*TargetAid);
                    continue;
                }

                //printf("找到盟友!! AID: %d, 座標 (%d, %d)\n", *TargetAid, *CorrdX, *CorrdY);
                // 計算距離
                int Dist = ((abs(*PLAYER_CORRD_X - *CorrdX) * abs(*PLAYER_CORRD_X - *CorrdX)) + (abs(*PLAYER_CORRD_Y - *CorrdY) * abs(*PLAYER_CORRD_Y - *CorrdY)));
                if (Dist <= (gNearSupportDist * gNearSupportDist) && SkillDelayPlayerList.search_player_2(*TargetAid, 0)) {
                    printf("輔助盟友!\n");

                    for (int SkillUseCount = 0; SkillUseCount < 5; SkillUseCount++) {
                        if (*HPIndexValue == 0 || *HPIndexValue == 1 || !CheckState() || MapChangeFlag == 1) {
                            break;
                        }
                        //
                        // 隱匿狀態 尖叫狀態 無知狀態 被咒縛 憂鬱 不使用技能
                        //
                        if (CheckDebuffState(4) || CheckDebuffState(470) || CheckDebuffState(417) || CheckDebuffState(411) || CheckDebuffState(438)) {
                            break;
                        }
                        gSkillIdAck = gNearSupportSkill;
                        gSkillAck = 0;
                        if (gNearSupportSupport) {
                            SendAttackPackageFunction(gNearSupportSkilv, gNearSupportSkill, *AidAddress);
                        }
                        else {
                            SendAttackPackageFunction(gNearSupportSkilv, gNearSupportSkill, *TargetAid);
                        }
                        Sleep(1000);
                        if (gSkillAck) { // 成功
                            break;
                        }
                    }
                    //TargetAidExpReportList.delete_target_node(*TargetAid);

                    SkillDelayPlayerList.delete_player_node_Id(*TargetAid, 0);
                    SkillDelayPlayerList.add_player_node_2(*TargetAid, gNearSupportDelay, 0);
                }
                else {
                    // 盟友距離太遠
                    //printf("盟友距離太遠!!\n");
                    
                    //TargetAidExpReportList.delete_target_node(*TargetAid);
                    
                }
            }
        }

        

        Sleep(1);
    }
    return EXIT_SUCCESS;
}


int SendSkillCastFunction(int SkillId, int SkillLv, int Target) {

    gNearAttackAck = 0;
    gNearAttackAckTotal++;


    if (gNearAttackSupport || SkillId == 2006 || SkillId == 2317 || SkillId == 2330 || SkillId == 4 || SkillId == 2334) {
        SendAttackPackageFunction(SkillLv, SkillId, *AidAddress);
    }
    else {
        SendAttackPackageFunction(SkillLv, SkillId, Target);
    }

    return 1;
}

int SendSkillGroundFunction(int SkillId, int SkillLv, int CorrdX, int CorrdY) {

    //SendAttackGroundPackageFunction(SkillLv, SkillId, (*PLAYER_CORRD_X + ((CorrdX - *PLAYER_CORRD_X) / 2)), (*PLAYER_CORRD_Y + ((CorrdY - *PLAYER_CORRD_Y) / 2)));

    SendAttackGroundPackageFunction(SkillLv, SkillId, CorrdX, CorrdY);


    gNearAttackAck = 0;
    gNearAttackAckTotal++;
    return 1;
}

wstring charToWstring(const char* szIn)
{
    int length = MultiByteToWideChar(CP_ACP, 0, szIn, -1, NULL, 0);
    WCHAR* buf = new WCHAR[length + 1];
    ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, szIn, -1, buf, length);
    std::wstring strRet(buf);
    delete[] buf;
    return strRet;
}

// 循環確認檔案日期是否有改變, 若有改變就讀取setting
BOOL LoadDrinkSettingFunction() {

    // 紀錄當前setting檔案時間
    SYSTEMTIME stLocalSetting;
    SYSTEMTIME stLocalSetting2;

    // 紀錄讀取的檔案時間
    SYSTEMTIME stLocalSettingRead;
    SYSTEMTIME stLocalSettingRead2;

    // 一開始先讀取當前檔案時間
    GetModifyDateTime(charToWstring(path), stLocalSetting);
    GetModifyDateTime(L".\\data\\setting.txt", stLocalSetting2);

    while (1) {
        Sleep(1000);

        // 讀取檔案時間
        GetModifyDateTime(charToWstring(path), stLocalSettingRead);
        GetModifyDateTime(L".\\data\\setting.txt", stLocalSettingRead2);

        if (memcmp(&stLocalSetting, &stLocalSettingRead, sizeof(SYSTEMTIME))) {
            // loading 5 秒
            for (int TimeCount = 0; TimeCount < 5; TimeCount++) {
                LoadDrinkSetting();
                Sleep(1000);
            }
            memcpy(&stLocalSetting, &stLocalSettingRead, sizeof(SYSTEMTIME));
        }

        if (memcmp(&stLocalSetting2, &stLocalSettingRead2, sizeof(SYSTEMTIME))) {
            // loading 5 秒
            for (int TimeCount = 0; TimeCount < 5; TimeCount++) {
                LoadDrinkSetting();
                Sleep(1000);
            }
            memcpy(&stLocalSetting2, &stLocalSettingRead2, sizeof(SYSTEMTIME));
        }


    }

}


BOOL PickItemFunction(void) {
    int* TargetDist = new int;

    while (1) {

        if (gTotalSwitch && *AidAddress && *MAP_NAME != LOGIN_RSW
            && *HPIndexValue > 1
            && CloakingTryNum >= 3
            && CheckState()
            && !gStorageStatus
            && !gIsTeleport
            && !gTeleportTrigger
            && (gAutoItemTakeSwitch || gAutoGreedSwitch)
            && gAutoMonsterTeleIdleCounter == 0
            ) {
            //
            // 自動找尋物品
            //
            while (1) {
                
                *TargetDist = 0;
                int TackItemAid = ItemAidList.search_target_Type_ItemPickUp(TargetDist);

                if (gAutoTelePointTrigger) { // 遇到瞬移情況
                    break;
                }
                else if (TackItemAid == -1) { // 貪婪
                    SendMessage(gHwnd, WM_KEYDOWN, gAutoGreedScanCode, 0);
                    for (int SleepCount = 0; SleepCount < gAutoItemTakeSmart; SleepCount++) {
                        Sleep(1);
                        if (gAutoItemTakeForce) {
                            gAutoTeleIdleCounter = 0;
                        }
                    }
                }
                else if (TackItemAid > 0) { // 撿取物品

                    printf("自動撿物品 %d\n", TackItemAid);

                    gAutoTeleIdleCounter = 0;
                    SendItemTakePackageFunction(TackItemAid);

                    for (int SleepCount = 0; SleepCount < gAutoItemTakeSmart; SleepCount++) {
                        Sleep(1);
                        if (gAutoItemTakeForce) {
                            gAutoTeleIdleCounter = 0;
                        }
                    }
                    /*if (*TargetDist < 3 * 3) {
                        Sleep(gAutoItemTakeSmart);
                    }
                    else if (*TargetDist < 7 * 7) {
                        Sleep(1000);
                    }
                    else {
                        Sleep(2000);
                    }*/
                }
                else { // 沒掃到物品, 跳出while
                    break;
                }
            }
            
        }

        Sleep(100);
    }
    return EXIT_SUCCESS;
}

BOOL ClearCurrentCharNameFunction(void) {

    int ClearCount = 0;

    while (1) {
        if (*AidAddress != 0) {
            //
            // 如果為選角畫面, 清除所有物品資料
            //
            if (*MAP_NAME == LOGIN_RSW) {
                SkillDataList.clear_skill_node();
                ItemDataList.clear_item_node();
                StorageDataList.clear_item_node();
                PlayerAidList.clear_player_node();
                SkillDelayPlayerList.clear_player_node();
                TargetAidList.clear_target_node();
                TargetAidExpReportList.clear_target_node();
                ItemAidList.clear_target_node();
                gDieNoteAid = 0;
                gDieNoteDamage = 0;
                gDieNoteSkill = 0;
                gExpReportTime = 0;
                gExpTotalObtainBaseValue = 0;
                gExpTotalObtainJobValue = 0;
                ClearExpReport();

                //PickUpItemException(); // 讀取檢物品清單
                if (ClearCount < 20) {
                    ClearCount++;
                    ClearCharNameFunction();
                }
            }
            else {
                ClearCount = 0;
            }

        }
        Sleep(100);
    }

    return EXIT_SUCCESS;
}

BOOL CatchCharNameAndAidFunction(void) {

    while (1) {
        if (*AidAddress && *MAP_NAME != LOGIN_RSW) {
            if (gCatchModeSwitch) {
                printf("偵測到擷取腳色資料\n");
                CatchCharNameAndAid();
            }
        }
        Sleep(300);
    }

    return EXIT_SUCCESS;
}

BOOL ExpReportFunction(void) {
    gExpReportTime = 0;
    gExpTotalObtainBaseValue = 0;
    gExpTotalObtainJobValue = 0;
    while (1) {
        if (*AidAddress && *MAP_NAME != LOGIN_RSW) {
            if (gExpReportSwitch) {
                gExpReportTime++;

            }

            if (gExpReportTime > 86400) {
                gExpReportTime = 0;
            }
        }

        Sleep(1000);
    }

    return EXIT_SUCCESS;
}

string addCommas(__int64 num) {
    stringstream ss;
    ss << num;
    string str = ss.str();
    int n = str.length();
    for (int i = n - 3; i > 0; i -= 3) {
        str.insert(i, ",");
    }
    return str;
}

void ExpReportGetFunction(void) {

    char ExpReportStr[48] = ".\\data\\log\\ExpReport_";
    char txt[5] = ".txt";
    strcat(ExpReportStr, gCharNameStr);
    strcat(ExpReportStr, txt);

    printf("case 0\n");

    ofstream fout(ExpReportStr, ios::trunc);   // 檔案如果存在就清空
    printf("case 1\n");
    if (!fout.fail() && *MaxExpValue > 0 && *MaxJobValue > 0 && gExpReportTime > 0) {

        int Minute = 0;
        int Hour = 0;
        int Second = gExpReportTime;
        Hour = Second / 3600;
        Second = Second - (Hour * 3600);
        Minute = Second / 60;
        Second = Second - (Minute * 60);
        
        __int64 ExpExceptValue = 0;
        __int64 JobExceptValue = 0;
        __int64 ExpExceptHourValue = 0;
        __int64 JobExceptHourValue = 0;
        printf("case 2\n");
        GetLocalTime(&sys);
        fout << sys.wYear << "/" << sys.wMonth << "/" << sys.wDay << " " << sys.wHour << ":" << sys.wMinute << ":" << sys.wSecond << " " << endl;
        fout << "------------經驗值報告------------" << endl;
        fout << "已經過時間: " << Hour << " 小時 " << Minute << " 分鐘 " << Second << "  秒" << endl;
        printf("case 3\n");
        ExpExceptValue = gExpTotalObtainBaseValue * 100 / *MaxExpValue;
        JobExceptValue = gExpTotalObtainJobValue * 100 / *MaxJobValue;
        fout << "獲得的Base經驗值: " << addCommas(gExpTotalObtainBaseValue) << " (" << ExpExceptValue << "%)" << endl;
        fout << "獲得的Job經驗值: " << addCommas(gExpTotalObtainJobValue) << " (" << JobExceptValue << "%)" << endl;
        printf("case 4\n");
        ExpExceptHourValue = gExpTotalObtainBaseValue * 3600 / gExpReportTime;
        JobExceptHourValue = gExpTotalObtainJobValue * 3600 / gExpReportTime;
        ExpExceptValue = ExpExceptHourValue * 100 / *MaxExpValue;
        JobExceptValue = JobExceptHourValue * 100 / *MaxJobValue;
        fout << "預估每小時Base經驗值: " << addCommas(ExpExceptHourValue) << " (" << ExpExceptValue << "%)" << endl;
        fout << "預估每小時Job經驗值: " << addCommas(JobExceptHourValue) << " (" << JobExceptValue << "%)" << endl;
        printf("case 5\n");
        fout << "------------獲取物品報告------------" << endl;
        fout.close();
        printf("case 6\n");
        ItemReportList.exp_report_all_item_data_byType(0);
        printf("case 7\n");

        ItemReportList.exp_report_all_item_data_byType(99);
        printf("case 9\n");
    }
    else {
        fout.close();
    }

    return;
}

void GetCurrentMapName(char* MapName) {
    MapName[0] = (*MAP_NAME & 0xFF);
    MapName[1] = ((*MAP_NAME & 0xFF00) >> 8);
    MapName[2] = ((*MAP_NAME & 0xFF0000) >> 16);
    MapName[3] = ((*MAP_NAME & 0xFF000000) >> 24);
    MapName[4] = (*(MAP_NAME + 1) & 0xFF);
    MapName[5] = ((*(MAP_NAME + 1) & 0xFF00) >> 8);
    MapName[6] = ((*(MAP_NAME + 1) & 0xFF0000) >> 16);
    MapName[7] = ((*(MAP_NAME + 1) & 0xFF000000) >> 24);
    MapName[8] = (*(MAP_NAME + 2) & 0xFF);
    MapName[9] = ((*(MAP_NAME + 2) & 0xFF00) >> 8);
    MapName[10] = ((*(MAP_NAME + 2) & 0xFF0000) >> 16);
    MapName[11] = ((*(MAP_NAME + 2) & 0xFF000000) >> 24);
    MapName[12] = (*(MAP_NAME + 3) & 0xFF);
    MapName[13] = ((*(MAP_NAME + 3) & 0xFF00) >> 8);
    MapName[14] = ((*(MAP_NAME + 3) & 0xFF0000) >> 16);
    MapName[15] = ((*(MAP_NAME + 3) & 0xFF000000) >> 24);
}


BOOL LoadingMapGatFunction(void) {

    char LOADING_MAP_NAME[16] = { "" }; // 當前載入地圖名稱
    while (1) {
        //if (*AidAddress && *MAP_NAME != LOGIN_RSW && gTotalSwitch && gNearAttackMonster) {
        if (*AidAddress && *MAP_NAME != LOGIN_RSW) {
            char CURRENT_MAP_NAME[16] = { "" }; // 定義地圖字串
            GetCurrentMapName(CURRENT_MAP_NAME);
            
            if (strcmp(CURRENT_MAP_NAME, LOADING_MAP_NAME)) { // 當 載入地圖 不等於 當前地圖時

                gMutexMapData.lock();
                strcpy(LOADING_MAP_NAME, CURRENT_MAP_NAME);
                //
                // loading 地圖
                //
                // 釋放記憶體
                if (gMapData != NULL && gWidth && gHeight) {
                    for (int i = 0; i < gHeight; i++) {
                        if (gMapData[i] != NULL) {
                            delete[] gMapData[i];
                        }
                    }
                    if (gMapData != NULL) {
                        delete[] gMapData;
                    }

                    printf("釋放記憶體\n");
                }
                char LoadingMapByFileStr[16] = { "" }; // 用來過濾.rsw
                strncpy(LoadingMapByFileStr, LOADING_MAP_NAME, strlen(LOADING_MAP_NAME) - 4); // 將過濾掉.rsw的地圖名稱值給LoadingMapByFileStr
                char MapFilePathStr[48] = ".\\data2\\";
                char gat[5] = ".gat";
                strcat(MapFilePathStr, LoadingMapByFileStr);
                strcat(MapFilePathStr, gat);
                // MapFilePathStr為地圖檔路徑
                printf("MapFilePathStr: %s\n", MapFilePathStr);

                ifstream fin(MapFilePathStr, ios::binary);

                int Index = 0;
                gWidth = 0;
                gHeight = 0;

                if (!fin.fail()) {

                    while (fin)
                    {
                        char ch;
                        fin.get(ch);  //使用in的成員函式get()讀取一個字元

                        // 取得寬
                        if (Index == 6) {
                            gWidth = ch;
                            gWidth &= 0xFF;
                        }
                        if (Index == 7) {
                            int tmp = ch;
                            tmp &= 0xFF;
                            gWidth = gWidth + (tmp * 0x100);
                        }

                        // 取得高
                        if (Index == 10) {
                            gHeight = ch;
                            gHeight &= 0xFF;
                        }
                        if (Index == 11) {
                            int tmp = ch;
                            tmp &= 0xFF;
                            gHeight = gHeight + (tmp * 0x100);

                        }

                        if (Index == 0x1E && gWidth && gHeight) { // 已讀取好長寬
                            printf("寬 高 %d, %d\n", gWidth, gHeight);
                            //// 動態分配陣列
                            //delete gMapData; // 釋放記憶體
                            //
                            //gMapData = new char[gWidth * gHeight + 1];
                            //if (gMapData == NULL) {
                            //    printf("記憶體配置失敗\n");
                            //    break;
                            //}

                            // Alloc 記憶體陣列
                            gMapData = new char* [gHeight];
                            if (gMapData == NULL) {
                                printf("記憶體配置失敗01\n");
                                gWidth = 0;
                                gHeight = 0;
                                break;
                            }
                            int AllocFail = 0;
                            for (int i = 0; i < gHeight; i++) {
                                gMapData[i] = new char[gWidth];
                                if (gMapData[i] == NULL) {
                                    printf("記憶體配置失敗02\n");
                                    gWidth = 0;
                                    gHeight = 0;
                                    AllocFail = 1;
                                    break;
                                }
                            }
                            if (AllocFail) {

                                break;
                            }

                            //int MapDataIndex = 0;
                            int MapDataWidthIndex = 0;
                            int MapDataHeightIndex = 0;
                            while (fin) {

                                if (MapDataHeightIndex >= gHeight) {
                                    break;
                                }

                                gMapData[MapDataHeightIndex][MapDataWidthIndex] = ch;
                                //printf("(%d, %d): 讀取type: %d\n", MapDataHeightIndex, MapDataWidthIndex, (ch & 0xFF));

                                MapDataWidthIndex++;
                                if (MapDataWidthIndex >= gWidth) {
                                    MapDataWidthIndex = 0;
                                    MapDataHeightIndex++;
                                }
                                for (int i = 0; i < 20; i++) {
                                    fin.get(ch);
                                }
                            }
                        }


                        Index++;
                        //printf("%02X ", ch);
                    }
                    printf("Loading地圖完畢!! %d, %d\n", gWidth, gHeight);

                    fin.close();

                    //printf("287,233測試: %d\n", gMapData[233][287]);
                    //printf("288,233測試: %d\n", gMapData[233][288]);
                }

                gMutexMapData.unlock();
            } 
        }

        Sleep(10);
    }

    return EXIT_SUCCESS;
}



BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    int timeoutUse = 0;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        LoadDrinkSetting();
        //CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(DetectKeyboardDevice), NULL, 0, 0);

        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadDrinkSettingFunction), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CloakingFunction), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(TestFunc), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoHpDrink), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoHpDrink2), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSpDrink), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoDetectStatusDo), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoBufferFunction), 0, 0, 0);
#if !PACKET_MODE
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoCastEquipFunction), NULL, 0, 0);
#endif
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SmartCastingSkill), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SmartCastingSkillKeyboardSupport), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SmartCastingSkillKeyboard), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(PcActionModifyFunction), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(TimeCounter), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(TimeCounter2), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SpecialEquip), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoMacroFunction), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoMacroFunction2), NULL, 0, 0);

        //CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(GetPackageBuffer), NULL, 0, 0);

        //#if PrivateServerOrNot
        //ReadPacketFile();
        //#endif
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ClearCurrentCharNameFunction), NULL, 0, 0);
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CatchCharNameAndAidFunction), NULL, 0, 0);
        if (!PrivateServerNormal) {
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ExpReportFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoCureFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoAttackHitFunction1), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoAttackHitFunction2), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoAttackHitFunction3), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoFindTargetAndAttack), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoAttackToPlayerFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSupportToFriendFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoCastEquipFunction1), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoCastEquipFunction2), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoCastEquipEndFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSmartEquipFunction1), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSmartEquipFunction2), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSmartEquipFunction3), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSmartEquipFunction4), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSmartEquipFunction5), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSmartEquipFunction6), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SmartBloodFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(OneKeyEquipFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(PickItemFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoSmartCloakFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CalculateObjectXYFunction), NULL, 0, 0);

            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadingMapGatFunction), NULL, 0, 0);

            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ModifyMap), NULL, 0, 0);
        }

#if !PrivateServerOrNot
			
			CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(PetFeedFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoWarpFunction), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoApFunction), NULL, 0, 0);
            //CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CpuSpeedFunction), NULL, 0, 0);

            //CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SkillSendByKeboard), NULL, 0, 0);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(PetLockXY), NULL, 0, 0);
#endif
        init();

        // 測試CD
        //CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoCdClinetFunction), NULL, 0, 0);

        if (!CheckSerialNumberList()) {
            MessageBox(NULL, "錯誤, 你的電腦序號未被加入到列表, 請聯繫管理員.", NULL, MB_ICONEXCLAMATION);
            exit(0);
            gPassFlag = 0;
        }
        else {
            gPassFlag = 1;
        }

        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(ConnectServer), NULL, 0, 0);

        GetLocalTime(&sys);
        printf("%4d/%02d/%02d %02d:%02d:%02d.%03d 星期%1d\n", 
            sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, sys.wDayOfWeek);

        if (sys.wYear > TIMEOUT_YEAR && sys.wDay > TIMEOUT_DAY) {
            timeoutUse = 1;
        }
        if (sys.wMonth > TIMEOUT_MONTH && sys.wDay > TIMEOUT_DAY) {
            timeoutUse = 1;
        }
        if ((sys.wMonth - TIMEOUT_MONTH) >= 2) {
            timeoutUse = 1;
        }

        //if (timeoutUse && PrivateServerOrNot) {
        if (timeoutUse && PayMode) {
            MessageBox(NULL, "錯誤, 你的序號已到期, 請聯繫管理員.", NULL, MB_ICONEXCLAMATION);
            exit(0);
        }

        PickUpItemException();

        atexit(ClearCharNameFunction);

        //MessageBox(NULL, "卡住.", "卡住", MB_OK);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

typedef struct
{
    HWND hwndWindow;
    DWORD dwProcessID;
} EnumWindowsArg;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    EnumWindowsArg* pArg = (EnumWindowsArg*)lParam;
    DWORD dwProcessID = 0;
    
    ::GetWindowThreadProcessId(hwnd, &dwProcessID);
    if (dwProcessID == pArg->dwProcessID)
    {
        pArg->hwndWindow = hwnd;
        
        return FALSE;
    }

    return TRUE;
}

HWND GetWindowHwndByPID(DWORD dwProcessID)
{
    HWND hwndRet = NULL;
    EnumWindowsArg ewa;
    ewa.dwProcessID = dwProcessID;
    ewa.hwndWindow = NULL;
    EnumWindows(EnumWindowsProc, (LPARAM)&ewa);
    if (ewa.hwndWindow)
    {
        hwndRet = ewa.hwndWindow;
    }
    return hwndRet;
}

int GetConsoleHwnd(void)
{
    DWORD proc_id_current = 0;
    int findFlag = 0;

    //MessageBox(NULL, L"成功", L"錯誤", MB_OK);

    proc_id_current = GetCurrentProcessId();
    gHwnd = GetWindowHwndByPID(proc_id_current);

    printf("gHwnd: %08X\n", gHwnd);

    if (gHwnd) {
        findFlag = 1;
    }

    //hwndCount = 0;
    //do {
    //    hwnd[hwndCount] = FindWindow(NULL, gRoWindowsName.data()); // 尋找RO windows視窗
    //    if (!hwnd[hwndCount]) {
    //        if (hwndCount == 0) {
    //            return 0;
    //        }
    //        MaxHwndCount = hwndCount;
    //        break;
    //    }
    //
    //    GetWindowThreadProcessId(hwnd[hwndCount], &proc_id2[hwndCount]);
    //    SetWindowText(hwnd[hwndCount], gRoWindowsName1.data());
    //    Sleep(40);
    //
    //    if (proc_id_current == proc_id2[hwndCount]) {
    //        findFlag = 1;
    //        gHwnd = hwnd[hwndCount];
    //    }
    //    hwndCount++;
    //
    //} while (hwndCount < 50);
    ////
    //// 把標題改回來
    ////
    //while (hwndCount != 0) {
    //    hwnd[hwndCount] = FindWindow(NULL, gRoWindowsName1.data());
    //    SetWindowText(hwnd[hwndCount], gRoWindowsName.data());
    //    hwndCount--;
    //}

    return findFlag;
}

void ClearCharNameFunction (void)
{
    //puts("Exit function.");
    // 清除所有ID相關的charname

    //
    // 寫資料到 charNameList.txt
    //
    char CurrentCharName[32] = "";
    LPCTSTR charPath = _T(".\\data\\charNameList.txt");

    // 當角色ID還沒裝到時, 先跳出
    if (!strcmp(gCharNameStr, "")) {
        return;
    }

    char CharName[11] = { "CHAR_NAME0" };
    char CharName2[12] = { "CHAR_NAME10" };
    for (int i = 0; i < 10; i++) {
        CharName[9] = i + 48;
        GetPrivateProfileString(_T("COMMON"), CharName, _T(""), CurrentCharName, 32, charPath);
        if (!strcmp(CurrentCharName, gCharNameStr)) { // 當CHAR_NAME 為角色ID時, 清空
            ::WritePrivateProfileString(_T("COMMON"), CharName, "", charPath); // CHAR_NAME0
        }
    }
    for (int i = 0; i < 10; i++) {
        CharName2[10] = i + 48;

        GetPrivateProfileString(_T("COMMON"), CharName2, _T(""), CurrentCharName, 32, charPath);
        if (!strcmp(CurrentCharName, gCharNameStr)) { // 當CHAR_NAME10 為角色ID時, 清空
            ::WritePrivateProfileString(_T("COMMON"), CharName2, "", charPath); // CHAR_NAME10
        }
    }
}

void ClearExpReport(void) {
    gExpTotalObtainBaseValue = 0;
    gExpTotalObtainJobValue = 0;
    gExpReportTime = 0;
}

int FindTotalRoCharName(void)
{
    //
    // 寫資料到 charNameList.txt
    //
    char CurrentCharName[32] = "";
    LPCTSTR charPath = _T(".\\data\\charNameList.txt");

    // 當角色ID還沒裝到時, 先跳出
    if (!strcmp(gCharNameStr, "")) { 
        return 0;
    }

    // 當為登入畫面
    if (*MAP_NAME == LOGIN_RSW) {
        return 0;
    }

    // 清除相關角色ID
    ClearCharNameFunction();

    char CharName[11] = { "CHAR_NAME0" };
    char CharName2[12] = { "CHAR_NAME10" };
    for (int i = 0; i < 10; i++) {
        CharName[9] = i + 48;
        GetPrivateProfileString(_T("COMMON"), CharName, _T(""), CurrentCharName, 32, charPath);
        if (!strcmp(CurrentCharName, "")) { // 當CHAR_NAME 為空的, 填寫角色資料
            ::WritePrivateProfileString(_T("COMMON"), CharName, gCharNameStr, charPath); // CHAR_NAME0
            return 1;
        }
    }
    for (int i = 0; i < 10; i++) {
        CharName2[10] = i + 48;

        GetPrivateProfileString(_T("COMMON"), CharName2, _T(""), CurrentCharName, 32, charPath);
        if (!strcmp(CurrentCharName, "")) { // 當CHAR_NAME10 為空的, 填寫角色資料
            ::WritePrivateProfileString(_T("COMMON"), CharName2, gCharNameStr, charPath); // CHAR_NAME10
            return 1;
        }
    }

    return 0;
}


int CatchCharNameAndAid(void)
{
    LPCTSTR charPath = _T(".\\data\\charNameList.txt");
    char AidText[20];
    int PlayerAidAddress = 0;
    int* PlayerAidValue = 0;
    char PlayerAidText[20];
    char PlayerGuildIdText[20];

    DWORD proc_id_current = GetCurrentProcessId();


    // 掃描玩家AID
    // Read Player AID
    //
    PlayerAidAddress = *WINDOWS_LOCK_1 + 236;
    PlayerAidValue = reinterpret_cast <int*>(PlayerAidAddress);

    //
    // int轉化成字串
    //
    sprintf(AidText, "%d", *AidAddress);
    sprintf(PlayerAidText, "%d", *PlayerAidValue);

    //
    // 寫資料到 charNameList.txt
    //

    ::WritePrivateProfileString(gCharNameStr, _T("角色名稱"), gCharNameStr, charPath); // CHAR_NAME0
    ::WritePrivateProfileString(gCharNameStr, _T("角色AID"), AidText, charPath); // CHAR_NAME0

    if (*PlayerAidValue) {
        ::WritePrivateProfileString(gCharNameStr, _T("滑鼠指標的角色AID"), PlayerAidText, charPath); // CHAR_NAME0
    }


    // 清空狀態序列
    char IndexTextClear[20];
    for (int i = 0; i < 50; i++) {

        sprintf(IndexTextClear, "%d", i); // 轉化數字變成文字
        ::WritePrivateProfileString(gCharNameStr, IndexTextClear, _T(""), charPath);
    }
    // 印出目前身上所有狀態
    char StateText[20];
    char IndexText[20];
    int Index = 0;
    int CheckStatusCount = 0;
    while (CheckStatusCount < 1) {
        while (*(STATUS_INDEX + Index) != 0xFFFFFFFF) {
            sprintf(StateText, "%d", *(STATUS_INDEX + Index)); // 轉化數字變成文字
            sprintf(IndexText, "%d", Index); // 轉化數字變成文字
            ::WritePrivateProfileString(gCharNameStr, IndexText, StateText, charPath);
            Index++;
            //Sleep(10);
        }

        Index = 0;
        CheckStatusCount++;
        //Sleep(50);
    }

    // 印出所有裝備
    ItemDataList.print_all_item_data_byType(1);
    // 印出所有堆疊物品
    ItemDataList.print_all_item_data_byType(0);

    return 1;
}

int CatchAidList(char* buffer, char* CharNameStr, int AidValue, int GuildId, int Job, int Weapon, int Shield)
{
    LPCTSTR charPath = _T(".\\data\\CatchAidList.txt");
    char PlayerAidText[20];
    char PlayerGuildIdText[20];
    char PlayerJobIdText[30];

    char PlayerWeaponText[100];
    char PlayeShieldText[100];

    //
    // int轉化成字串
    //
    sprintf(PlayerAidText, "%d", AidValue);
    sprintf(PlayerGuildIdText, "%d", GuildId);
    sprintf(PlayerJobIdText, "%d", Job);

    sprintf(PlayerWeaponText, "%d", Weapon);
    sprintf(PlayeShieldText, "%d", Shield);
#if !PrivateServerOrNot
    FindItemName(Weapon, PlayerWeaponText);
    FindItemName(Shield, PlayeShieldText);
    FindJobName(Job, PlayerJobIdText);
#endif
    //
    // 寫資料到 charNameList.txt
    //

    // 角色AID
    if (strcmp(CharNameStr, "")) {
        ::WritePrivateProfileString(CharNameStr, _T("角色編號"), PlayerAidText, charPath); // AID
        ::WritePrivateProfileString(CharNameStr, _T("公會編號"), PlayerGuildIdText, charPath); // Guild ID

        ::WritePrivateProfileString(CharNameStr, _T("職業編號"), PlayerJobIdText, charPath); // Job

        ::WritePrivateProfileString(CharNameStr, _T("裝備武器"), PlayerWeaponText, charPath); // Weapon
        ::WritePrivateProfileString(CharNameStr, _T("裝備盾牌"), PlayeShieldText, charPath); // Shield
    }
    
    return 1;
}

BOOL ConnectServer() {
    
ReConnect:
    int FirstTime = 0;

    //定義長度變量
    int send_len = 0;
    int recv_len = 0;
    //定義發送緩衝區和接受緩衝區
    char send_buf[100];
    char recv_buf[100];
    //定義服務端套接字，接受請求套接字
    
    //服務端地址客戶端地址
    SOCKADDR_IN server_addr;
    initialization();
    //填充服務端信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = inet_addr("49.158.24.247"); // 1.34.66.85
    server_addr.sin_port = htons(1234);
    //創建套接字
    s_server = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        //cout << "服務器連接失敗01" << endl;
        //server_addr.sin_addr.S_un.S_addr = inet_addr("60.250.30.118"); // 60.250.30.118
        //if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        //    cout << "服務器連接失敗02" << endl;
            if (!gPassFlag) {
                //MessageBox(NULL, "伺服器連接失敗", NULL, MB_ICONEXCLAMATION);
                exit(0);
            }
            gIsConnectFlag = 0;
            Sleep(3000);
            goto ReConnect;
            //return 1;
        //}
    }

    cout << "服務器連接成功！" << endl;
    gIsConnectFlag = 1;

    //發送,接收數據

    cout << "發送訊息!" << endl;

    if (!FirstTime) {
        // 發送序號
        send_buf[0] = 2;
        strcpy(&send_buf[1], serialNumberTrue);
        send_len = send(s_server, send_buf, 100, 0);
        if (send_len < 0) {
            //cout << "發送失敗！" << endl;
            gIsConnectFlag = 0;
            //break;
        }

        recv_len = recv(s_server, recv_buf, 100, 0);
        if (recv_len < 0) {
            //cout << "接收失敗！" << endl;
            gIsConnectFlag = 0;
        }
        else {
            cout << "服務端訊息:" << recv_buf << endl;
        }
        if (recv_buf[0] == 48) {
            cout << "驗證失敗" << endl;
            //MessageBox(NULL, "錯誤, 你的電腦序號未被加入到列表, 請聯繫管理員.", NULL, MB_ICONEXCLAMATION);
            exit(0);
        }
        else if (recv_buf[0] == 49) {
            gPassFlag = 1; // 序號通過
        }

        FirstTime = 1;
    }

    while (1) {
        if (CloakingTryNum == 4 && *MAP_NAME != LOGIN_RSW) {
            Sleep(3000);
            if (CloakingTryNum != 4 || *MAP_NAME == LOGIN_RSW) {
                continue;
            }
            CloakingTryNum++;
            char CURRENT_MAP_NAME[16] = { "" };
            CURRENT_MAP_NAME[0] = (*MAP_NAME & 0xFF);
            CURRENT_MAP_NAME[1] = ((*MAP_NAME & 0xFF00) >> 8);
            CURRENT_MAP_NAME[2] = ((*MAP_NAME & 0xFF0000) >> 16);
            CURRENT_MAP_NAME[3] = ((*MAP_NAME & 0xFF000000) >> 24);
            CURRENT_MAP_NAME[4] = (*(MAP_NAME + 1) & 0xFF);
            CURRENT_MAP_NAME[5] = ((*(MAP_NAME + 1) & 0xFF00) >> 8);
            CURRENT_MAP_NAME[6] = ((*(MAP_NAME + 1) & 0xFF0000) >> 16);
            CURRENT_MAP_NAME[7] = ((*(MAP_NAME + 1) & 0xFF000000) >> 24);
            CURRENT_MAP_NAME[8] = (*(MAP_NAME + 2) & 0xFF);
            CURRENT_MAP_NAME[9] = ((*(MAP_NAME + 2) & 0xFF00) >> 8);
            CURRENT_MAP_NAME[10] = ((*(MAP_NAME + 2) & 0xFF0000) >> 16);
            CURRENT_MAP_NAME[11] = ((*(MAP_NAME + 2) & 0xFF000000) >> 24);
            CURRENT_MAP_NAME[12] = (*(MAP_NAME + 3) & 0xFF);
            CURRENT_MAP_NAME[13] = ((*(MAP_NAME + 3) & 0xFF00) >> 8);
            CURRENT_MAP_NAME[14] = ((*(MAP_NAME + 3) & 0xFF0000) >> 16);
            CURRENT_MAP_NAME[15] = ((*(MAP_NAME + 3) & 0xFF000000) >> 24);


            //MEMORY_BASIC_INFORMATION mbi;

            //int* ATTACK_ADDRESS_1 = NULL;
            //int* ATTACK_ADDRESS_2 = NULL;
            //int* ATTACK_ADDRESS_TRIGGER = NULL;
            //int* ATTACK_ADDRESS_AID = NULL;

            //int* ATTACK_ADDRESS_STOP_ATTACK = NULL;

            //// 填上5, 觸發普攻
            //if (VirtualQuery((LPCVOID)WINDOWS_LOCK_1, &mbi, sizeof(mbi))) {
            //    if (mbi.Protect == PAGE_READWRITE) {
            //        ATTACK_ADDRESS_1 = (int*)(*WINDOWS_LOCK_1 + 0xCC);
            //    }
            //}
            //if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_1, &mbi, sizeof(mbi))) {
            //    if (mbi.Protect == PAGE_READWRITE) {
            //        ATTACK_ADDRESS_2 = (int*)(*ATTACK_ADDRESS_1 + 0x2C);
            //    }
            //}
            //if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
            //    if (mbi.Protect == PAGE_READWRITE) {
            //        ATTACK_ADDRESS_TRIGGER = (int*)(*ATTACK_ADDRESS_2 + 0x4E8);
            //    }
            //}

            //if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
            //    if (mbi.Protect == PAGE_READWRITE) {
            //        ATTACK_ADDRESS_AID = (int*)(*ATTACK_ADDRESS_2 + 0x4FC);
            //    }
            //}

            //if (VirtualQuery((LPCVOID)ATTACK_ADDRESS_2, &mbi, sizeof(mbi))) {
            //    if (mbi.Protect == PAGE_READWRITE) {
            //        ATTACK_ADDRESS_STOP_ATTACK = (int*)(*ATTACK_ADDRESS_2 + 0x4F0);
            //    }
            //}

            //printf("測試: 普攻: ATTACK_ADDRESS_TRIGGER: %X, ATTACK_ADDRESS_AID: %X \n", ATTACK_ADDRESS_TRIGGER, ATTACK_ADDRESS_AID);

            // 發送角色名稱
            send_buf[0] = 1;
            strcpy(&send_buf[1], gCharNameStr);
            send_len = send(s_server, send_buf, 100, 0);
            if (send_len < 0) {
                //cout << "發送失敗！" << endl;
                gIsConnectFlag = 0;
                //break;
            }

            // 發送地圖
            send_buf[0] = 3;
            strcpy(&send_buf[1], CURRENT_MAP_NAME);
            send_len = send(s_server, send_buf, 100, 0);
            if (send_len < 0) {
                //cout << "發送失敗！" << endl;
                gIsConnectFlag = 0;
                //break;
            }

            // 發送最大HP
            send_buf[0] = 4;
            // int轉化成字串
            char MaxHPText[20];
            sprintf(MaxHPText, "%d", *MaxHPTableValue);
            strcpy(&send_buf[1], MaxHPText);
            send_len = send(s_server, send_buf, 100, 0);
            if (send_len < 0) {
                //cout << "發送失敗！" << endl;
                gIsConnectFlag = 0;
                //break;
            }

#if !PrivateServerOrNot
            // 發送身上金錢
            send_buf[0] = 6;
            // int轉化成字串
            char MoneyText[20];
            sprintf(MoneyText, "%d", *MoneyIndexValue);
            strcpy(&send_buf[1], MoneyText);
            send_len = send(s_server, send_buf, 100, 0);
            if (send_len < 0) {
                //cout << "發送失敗！" << endl;
                gIsConnectFlag = 0;
                //break;
            }
#endif

            // 發送最大SP
            send_buf[0] = 5;
            // int轉化成字串
            char MaxSPText[20];
            sprintf(MaxSPText, "%d", *MaxSPTableValue);
            strcpy(&send_buf[1], MaxSPText);
            send_len = send(s_server, send_buf, 100, 0);
            if (send_len < 0) {
                //cout << "發送失敗！" << endl;
                gIsConnectFlag = 0;
                //break;
            }



        }

        if (!gIsConnectFlag) {
            //goto ReConnect;
            return 1;
        }
        Sleep(100);
    }

	return 1;
}
void initialization() {
    //初始化套接字庫
    WORD w_req = MAKEWORD(2, 2);//版本號
    WSADATA wsadata;
    int err;
    err = WSAStartup(w_req, &wsadata);
    if (err != 0) {
        cout << "初始化失敗！" << endl;
    }
    else {
        cout << "初始化成功！" << endl;
    }
    //檢查版本號
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
        cout << "版本不符！" << endl;
        WSACleanup();
    }
    else {
        cout << "版本正確！" << endl;
    }
}

int ConvertEquipSetting (CString *SettingBuffer, equipDataStruct *EquipItem) {
    char PartingText[10] = "";
    int PartingIndex = 0;
    int SettingBufferInt = 0;
    int passingDataIndex = 0; // 總共有六個

    int StringLen = strlen(*SettingBuffer);

    // 清空資料
    if (StringLen < 6) {
        EquipItem->refineView = 0;
        EquipItem->itemId = 0;
        EquipItem->slot1 = 0;
        EquipItem->slot2 = 0;
        EquipItem->slot3 = 0;
        EquipItem->slot4 = 0;
    }
    for (int k = 0; k < StringLen; k++) {
        if (SettingBuffer->GetBuffer(100)[k] != ',') {
            PartingText[PartingIndex] = SettingBuffer->GetBuffer(100)[k];
            PartingIndex++;
        }
        else if (passingDataIndex == 0) { // 精煉值
            // 將PartingText轉換成數字
            SettingBufferInt = _ttoi(PartingText);
            EquipItem->refineView = SettingBufferInt;
            PartingIndex = 0; // 歸零
            memset(PartingText, 0, strlen(PartingText)); // 清空字串
            passingDataIndex++;
        }
        else if (passingDataIndex == 1) { // 裝備ID
            // 將PartingText轉換成數字
            SettingBufferInt = _ttoi(PartingText);
            EquipItem->itemId = SettingBufferInt;
            PartingIndex = 0; // 歸零
            memset(PartingText, 0, strlen(PartingText)); // 清空字串
            passingDataIndex++;
        }
        else if (passingDataIndex == 2) { // 洞1
           // 將PartingText轉換成數字
            SettingBufferInt = _ttoi(PartingText);
            EquipItem->slot1 = SettingBufferInt;
            PartingIndex = 0; // 歸零
            memset(PartingText, 0, strlen(PartingText)); // 清空字串
            passingDataIndex++;
        }
        else if (passingDataIndex == 3) { // 洞2
           // 將PartingText轉換成數字
            SettingBufferInt = _ttoi(PartingText);
            EquipItem->slot2 = SettingBufferInt;
            PartingIndex = 0; // 歸零
            memset(PartingText, 0, strlen(PartingText)); // 清空字串
            passingDataIndex++;
        }
        else if (passingDataIndex == 4) { // 洞3
           // 將PartingText轉換成數字
            SettingBufferInt = _ttoi(PartingText);
            EquipItem->slot3 = SettingBufferInt;
            PartingIndex = 0; // 歸零
            memset(PartingText, 0, strlen(PartingText)); // 清空字串
            passingDataIndex++;
        }
        else if (passingDataIndex == 5) { // 洞4
           // 將PartingText轉換成數字
            SettingBufferInt = _ttoi(PartingText);
            EquipItem->slot4 = SettingBufferInt;
            PartingIndex = 0; // 歸零
            memset(PartingText, 0, strlen(PartingText)); // 清空字串
            passingDataIndex++;
            break;
        }
    }
    //printf("測試: +%d %d [%d, %d, %d, %d]\n", EquipItem->refineView, EquipItem->itemId,
    //    EquipItem->slot1, EquipItem->slot2, EquipItem->slot3, EquipItem->slot4);

}


//#define MAX 1000
queue<Point> mark;
void test(Point(**p));

bool work(Point& q, Point& e, char(**map), Point(**p), int Follow) {
    int x, y;

    // 左上
    if ((x = q.x - 1) >= 0 && (y = q.y + 1) < gHeight && map[y][x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {

        p[y][x].len = q.len + 1.41;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) { // 為終點座標
            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }
    // 左下
    if ((x = q.x - 1) >= 0 && (y = q.y - 1) >= 0 && map[y][x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {

        p[y][x].len = q.len + 1.41;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) { // 為終點座標
            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }
    // 右上
    if ((x = q.x + 1) < gWidth && (y = q.y + 1) < gHeight && map[y][x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {

        p[y][x].len = q.len + 1.41;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) { // 為終點座標
            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }
    // 右下
    if ((x = q.x + 1) < gWidth && (y = q.y - 1) >= 0 && map[y][x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {

        p[y][x].len = q.len + 1.41;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) { // 為終點座標
            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }

    // left
    if ((x = q.x - 1) >= 0 && map[y = q.y][x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {

        p[y][x].len = q.len + 1;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) { // 為終點座標
            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }
    // right
    if ((x = q.x + 1) < gWidth && map[y = q.y][x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {

        p[y][x].len = q.len + 1;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) {
            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }
    // down
    if ((y = q.y - 1) >= 0 && map[y][x = q.x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {
        p[y][x].len = q.len + 1;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) {
            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }
    // up
    if ((y = q.y + 1) < gHeight && map[y][x = q.x] != 1 && map[y][x] != 5 && p[y][x].len == 0) {
        p[y][x].len = q.len + 1;
        p[y][x].pre_x = q.x;
        p[y][x].pre_y = q.y;
        if (x == e.x && y == e.y) {

            return true;
        }
        else if (p[y][x].len > 30 && !Follow) { // 已經太遠 直接回傳
            return false;
        }
        else {
            mark.push(p[y][x]);
        }
    }

    return false;
}

void printPath(Point& end, Point(**p), int* MoveX, int* MoveY, int Step) {

    if (end.pre_x == 0 && end.pre_y == 0) {
        //printf("[%d][%d]\n", end.x, end.y);
        return;
    }
    else {
        //printf("[%d][%d] , %3.2f\n", end.x, end.y, end.len);

        if (end.len < Step + 1 && end.len >= Step) { // 一次移動Step格
            *MoveX = end.x;
            *MoveY = end.y;
        }

        printPath((p[end.pre_y][end.pre_x]), p, MoveX, MoveY, Step);
        //printf("[%d][%d]\n", end.x, end.y);
    }
}

void f(char(**map), Point& s, Point& e, bool& k, Point(**p), int Follow) {
    mark.push(s);
    int flag = false;

    while (mark.size() != 0 && !flag) { // 終止條件：找遍所有點都沒找到終點&找到終點
        flag = work(mark.front(), e, map, p, Follow);
        mark.pop();
        //test(p);
    }
    k = flag;
}

void test(Point(**p)) { 
    for (int i = 0; i != gHeight; i++) {
        for (int j = 0; j != gWidth; j++) {
            printf("%d ", p[i][j].len);
        }
        cout << endl;
    }
    cout << endl;
}

int CalculateTwoPointLength(int X1, int Y1, int X2, int Y2, int* MoveX, int* MoveY, int Follow, int Step)
{
    lock_guard<mutex> mLock(gMutexMapData);
    *MoveX = 0;
    *MoveY = 0;

    if (gWidth < 1 || gHeight < 1) {
        return 0;
    }

    mark = queue<Point>();

    Point s(X1, Y1);
    Point e(X2, Y2);

    bool k = false;

    // Alloc 記憶體陣列
    Point** p;
    int BackupWidth = gWidth;
    int BackupHeight = gHeight;

    p = new Point * [BackupHeight];
    for (int i = 0; i < BackupHeight; i++) {
        p[i] = new Point[BackupWidth];
    }

    p[s.y][s.x].len = 1;
    for (int i = 0; i < BackupHeight; i++) {
        for (int j = 0; j < BackupWidth; j++) {
            p[i][j].setPoint(j, i);
        }
    } // 初始化
    int LosDist = 999;
    f(gMapData, s, e, k, p, Follow);
    if (k) {
        //printf("MIN=%3.2f\n", p[e.y][e.x].len);
        LosDist = p[e.y][e.x].len;

        //printf("(%d, %d) --> (%d, %d) LosDist: %d\n", X1, Y1, X2, Y2, LosDist);

        *MoveX = e.x;
        *MoveY = e.y;
        printPath(p[e.y][e.x], p, MoveX, MoveY, Step);
    }
    else {
        //printf("ERROR\n"); // 不通
        *MoveX = 0;
        *MoveY = 0;
        LosDist = 999;
    }

    if (p != NULL && BackupWidth && BackupHeight) {
        for (int i = 0; i < BackupHeight; i++) {
            if (p[i] != NULL) {
                delete[] p[i];
            }
        }
        if (p != NULL) {
            delete[] p;
        }
    }

    return LosDist;
}


int CheckLos(int X0, int Y0, int X1, int Y1) {
    lock_guard<mutex> mLock(gMutexMapData);

    if (gWidth < 1 || gHeight < 1) { // 地圖檔沒抓到
        return 1;
    }

    // Simulate tracing a line to the location (modified Bresenham's algorithm)
    int steep;
    int posX = 1;
    int posY = 1;

    if (X1 - X0 < 0) {
        posX = -1;
    }
    if (Y1 - Y0 < 0) {
        posY = -1;
    }
    if (abs(Y0 - Y1) < abs(X0 - X1)) {
        steep = 0;
    }
    else {
        steep = 1;
    }
    if (steep == 1) {
        int Yt = Y0;
        Y0 = X0;
        X0 = Yt;

        Yt = Y1;
        Y1 = X1;
        X1 = Yt;
    }
    if (X0 > X1) {
        int Xt = X0;
        X0 = X1;
        X1 = Xt;

        int Yt = Y0;
        Y0 = Y1;
        Y1 = Yt;
    }
    int dX = X1 - X0;
    int dY = abs(Y1 - Y0);
    int E = 0;
    int dE;
    if (dX) {
        dE = dY / dX;
    }
    else {
        // Delta X is 0, it only occures when $from is equal to $to
        return 1;
    }
    int stepY;
    if (Y0 < Y1) {
        stepY = 1;
    }
    else {
        stepY = -1;
    }

    int Y = Y0;
    float Erate = 0.99;
    if ((posY == -1 && posX == 1) || (posY == 1 && posX == -1)) {
        Erate = 0.01;
    }
    for (int X = X0; X <= X1; X++) {
        E += dE;
        if (steep == 1) {
            if (gMapData[X][Y] == 1) { // 之後修改
                return 0;
            }
            if (gNearAttackSkill == 2307 && gMapData[X][Y] == 5) { //  2307#加農砲攻擊# 連灰色牆壁都要繞過
                return 0;
            }
        }
        else {
            if (gMapData[Y][X] == 1) {
                return 0;
            }
            if (gNearAttackSkill == 2307 && gMapData[Y][X] == 5) { //  2307#加農砲攻擊# 連灰色牆壁都要繞過
                return 0;
            }
        }
        if (E >= Erate) {
            Y += stepY;
            E -= 1;
        }
    }
    return 1;
}

int FindSkillName(int SkillId, char SkillName[100]) {
    ifstream fin(".\\data\\table\\skills.txt");    // Open the file

    size_t pos = 0;
    size_t pos2 = 0;
    string space_delimiter = "#";
    vector<string> lines;
    string line;

    while (getline(fin, line)) {
        pos = line.find(space_delimiter);
        if (pos == -1) {
            //printf("break\n");
            break;
        }
        if (pos > 0) {
            //printf("pos: %d\n", pos);
            int SkillIdRead = stoi(line.substr(0, pos));
            //printf("%d ", SkillIdRead);

            pos2 = line.length();
            string SkillStrRead = line.substr(pos + 1, pos2 - pos - 2);
            //cout << SkillStrRead << endl;

            int CheckLen = pos2 - pos - 2;
            if (SkillIdRead == SkillId && CheckLen < 100) {
                strcpy(SkillName, SkillStrRead.data());
                fin.close();
                return 1;
            }
        }

        lines.push_back(line);
    }

    fin.close();
    return 0;
}

int FindItemName(int ItemId, char *ItemName) {
    ifstream fin(".\\data\\table\\items.txt");    // Open the file

    size_t pos = 0;
    size_t pos2 = 0;
    string space_delimiter = "#";
    vector<string> lines;
    string line;

    while (getline(fin, line)) {
        pos = line.find(space_delimiter);
        if (pos == -1) {
            //printf("break\n");
            break;
        }
        if (pos > 0) {
            //printf("pos: %d\n", pos);
            int ItemIdRead = stoi(line.substr(0, pos));
            //printf("%d ", ItemIdRead);

            pos2 = line.length();
            string ItemStrRead = line.substr(pos + 1, pos2 - pos - 2);
            //cout << ItemStrRead << endl;

            int CheckLen = pos2 - pos - 2;
            if (ItemIdRead == ItemId && CheckLen < 100) {
                strcpy(ItemName, ItemStrRead.data());
                fin.close();
                return 1;
            }
        }

        lines.push_back(line);
    }

    fin.close();
    return 0;
}

int FindJobName(int JobId, char JobName[30]) {
    ifstream fin(".\\data\\table\\joblist.txt");    // Open the file

    size_t pos = 0;
    size_t pos2 = 0;
    string space_delimiter = "#";
    vector<string> lines;
    string line;

    while (getline(fin, line)) {
        pos = line.find(space_delimiter);
        //printf("pos: %d\n", pos);
        if (pos == -1) {
            //printf("break\n");
            break;
        }
        if (pos > 0) {
            int JobIdRead = stoi(line.substr(0, pos));
            //printf("%d ", JobIdRead);

            pos2 = line.length();
            string JobStrRead = line.substr(pos + 1, pos2 - pos - 2);
            //cout << JobStrRead << endl;

            int CheckLen = pos2 - pos - 2;
            if (JobIdRead == JobId && CheckLen < 30) {
                strcpy(JobName, JobStrRead.data());
                fin.close();
                return 1;
            }
        }
        lines.push_back(line);
    }

    fin.close();
    return 0;
}

// 計算幾次方
int powValue(int Base, int BaseBy, int Count) {
    int Value = Base;
    for (int i = 0; i < Count; i++) {
        Value *= BaseBy;
    }
    return Value;
}

int ConvertToInt(char ItemChar) {
    int ItemValue = 0;
    if (ItemChar >= '0' && ItemChar <= '9') {
        ItemValue = ItemChar - '0';
    }
    else if (ItemChar >= 'a' && ItemChar <= 'z') {
        ItemValue = ItemChar - 'a' + 10;
    }
    else if (ItemChar >= 'A' && ItemChar <= 'Z') {
        ItemValue = ItemChar - 'A' + 36;
    }

    return ItemValue;
}

int fromBase62 (const char *ItemChar, int IsRefine) {
    int Base10 = 0;
    int ItemCharLen = strlen(ItemChar);
    int Count = 0;
    for (int i = ItemCharLen - 1; i >= 0; i--) {
        int ItemValue = ConvertToInt(ItemChar[i]);
        //printf("%d: ItemValue: %d\n", i, ItemValue);
        Base10 += powValue(ItemValue, 62, ItemCharLen - i - 1);
        Count++;
        if (!IsRefine) {
            if (Count >= ItemCharLen - 6) {
                break;
            }

        }
    }

    return Base10;
}

// Solve plain message string that contains client's special functions
void solveMessage (string * BroadCastMessage)  {
    // Example:
    // <ITEML>.*</ITEML> to readable item name
    // Sell < ITEML>0000y1kk & 0g)00)00)00)00 + 05,00 - 01 + 0B,00 - 0m < / ITEML>500k
    // S > < ITEML>0000y1nC & 05)00)00 + 0h,00 - 0C + 0F,00 - 0h< / ITEML> < ITEML>000021jM & 01)00)00 + 0h,00 - 0N + 0B,00 - 0g< / ITEML> < ITEML>000021jM & 01)00)00 + 0f,00 - 02 + 0B,00 - 0e< / ITEML>
    
    //my($itemstr, $infostr) = ($1, $2);
    //my($loc, $showslots, $id) = $itemstr = ~/ ([\d\w]{5})(\d)([\d\w] + ) / ;
    //my($refine) = $infostr = ~/ % ([\d\w] + ) / ;
    //my($itemtype) = $infostr = ~/ &([\d\w] + ) / ;
    //my $item_info = {
    //    nameID = > fromBase62($id),
    //    upgrade = > fromBase62($refine),
    //};

    size_t pos = 0;
    size_t pos2 = 0;
    string item_delimiter = "<ITEML>";
    string item_delimiter2 = "<\/ITEML>";
    string item_delimiter3 = "&";
    string BroadCastMessageStr = *BroadCastMessage;

    pos = BroadCastMessageStr.find(item_delimiter);
    pos2 = BroadCastMessageStr.find(item_delimiter2);
    while (pos != -1 && pos2 != -1) {
        string ItemStrRead = BroadCastMessageStr.substr(pos + 7, pos2 - pos - 7);
        //printf("ItemStrRead: %s\n", ItemStrRead);
        size_t pos_percent = ItemStrRead.find("%");
        int FindRefine = 1;
        if (pos_percent == -1) {
            FindRefine = 0;
            pos_percent = ItemStrRead.find("&");
        }

        //printf("pos_and: %d\n", pos_percent);
        if (pos_percent != -1) {
            string ItemStrRead01 = ItemStrRead.substr(0, pos_percent);
            //cout << ItemStrRead01 << endl;
            int ItemIdValue = fromBase62(ItemStrRead01.data(), 0);
            //printf("ItemIdValue: %d\n", ItemIdValue);
            char ItemName[300] = "";
            FindItemName(ItemIdValue, ItemName);

            // 尋找插卡附魔資訊
            size_t pos_slot1 = ItemStrRead.find(")");
            size_t pos_slot2 = ItemStrRead.find(")", pos_slot1 + 1);
            size_t pos_slot3 = ItemStrRead.find(")", pos_slot2 + 1);
            size_t pos_slot4 = ItemStrRead.find(")", pos_slot3 + 1);
            size_t pos_length = ItemStrRead.length();

            //printf("\n%d %d %d %d %d\n", pos_slot1, pos_slot2, pos_slot3, pos_slot4, pos_length);

            string SlotStrRead01 = ItemStrRead.substr(pos_slot1 + 1, pos_slot2 - pos_slot1 - 1);
            string SlotStrRead02 = ItemStrRead.substr(pos_slot2 + 1, pos_slot3 - pos_slot2 - 1);
            string SlotStrRead03 = ItemStrRead.substr(pos_slot3 + 1, pos_slot4 - pos_slot3 - 1);
            string SlotStrRead04 = ItemStrRead.substr(pos_slot4 + 1, pos_length - pos_slot4 - 1);

            if (pos_slot1 && pos_slot2 && pos_slot3 && pos_slot4 && pos_length) {
                //cout << endl << SlotStrRead01 << " " << SlotStrRead02 << " " << SlotStrRead03 << " " << SlotStrRead04 << endl;
                if (SlotStrRead01 != "00") {
                    int SlotIdValue = fromBase62(SlotStrRead01.data(), 1);
                    char SlotName[100] = "";
                    FindItemName(SlotIdValue, SlotName);

                    strcat_s(ItemName, " ");
                    strcat_s(ItemName, SlotName);
                }
                if (SlotStrRead02 != "00") {
                    int SlotIdValue = fromBase62(SlotStrRead02.data(), 1);
                    char SlotName[100] = "";
                    FindItemName(SlotIdValue, SlotName);

                    strcat_s(ItemName, " ");
                    strcat_s(ItemName, SlotName);
                }
                if (SlotStrRead03 != "00") {
                    int SlotIdValue = fromBase62(SlotStrRead03.data(), 1);
                    char SlotName[100] = "";
                    FindItemName(SlotIdValue, SlotName);

                    strcat_s(ItemName, " ");
                    strcat_s(ItemName, SlotName);
                }
                if (SlotStrRead04 != "00") {
                    int SlotIdValue = fromBase62(SlotStrRead04.data(), 1);
                    char SlotName[100] = "";
                    FindItemName(SlotIdValue, SlotName);

                    strcat_s(ItemName, " ");
                    strcat_s(ItemName, SlotName);
                }
            }

            if (FindRefine) {
                // 有精煉 合併文字
                string RefineStrRead = ItemStrRead.substr(pos_percent + 1, 2);
                int RefineValue = fromBase62(RefineStrRead.data(), 1);
                //printf("RefineValue: %d\n", RefineValue);

                char refineText[300] = "+";
                char refineViewText[4] = "";
                char refineSpaceText[2] = " ";
                sprintf(refineViewText, "%d", RefineValue); // 轉化數字變成文字

                strcat_s(refineText, refineViewText);
                strcat_s(refineText, refineSpaceText);

                strcat_s(refineText, ItemName);
                strcpy_s(ItemName, refineText);
            }

            BroadCastMessageStr.replace(pos + 1, pos2 - pos + 8 - 2, ItemName);
        }

        pos = BroadCastMessageStr.find(item_delimiter);
        pos2 = BroadCastMessageStr.find(item_delimiter2);
    }
    *BroadCastMessage = BroadCastMessageStr;
    //strcpy(BroadCastMessage, BroadCastMessageStr.data());

    //if ($msg = ~/ <ITEML>([a - zA - Z0 - 9 % &(),+\ - *] *) < \ / ITEML > / ) {
    //    $msg = ~s / <ITEML>([a - zA - Z0 - 9 % &(),+\ - *] *) < \ / ITEML > / solveItemLink($1) / eg;
    //}

    return;
}
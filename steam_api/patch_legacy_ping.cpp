#include "stdafx.h"
#include "hooking.h"

#if 0
#pragma unmanaged
DWORD currentMaxPing = 0;
DWORD pingSearches = 0;
DWORD failedSearches = 0;

DWORD GetCurrentMaxPing() {
	return currentMaxPing;
}

// GetLocalizeString
typedef char* (__cdecl * GetLocalizeString_t)(const char* name);
GetLocalizeString_t GetLocalizeString = (GetLocalizeString_t)0x4F1700;

// LocalizedIntString
typedef char* (__cdecl * LocalizedIntString_t)(const char* string, int number);
LocalizedIntString_t LocalizedIntString = (LocalizedIntString_t)0x501090;

// local variables for PingDisplayHookFunc
typedef struct pdh_value_s {
	int type;
	char* string;
} pdh_value_t;

pdh_value_t* pdh_value;
int curPing;
char* searchString;

StompHook pingDisplayHook;
DWORD pingDisplayHookLoc = 0x62EE01;

void __declspec(naked) PingDisplayHookFunc() {
	__asm mov pdh_value, esi

    if ( GetCurrentMaxPing() && GetCurrentMaxPing() <= 150 )
    {
		curPing = GetCurrentMaxPing();
		searchString = GetLocalizeString("PATCH_SEARCHINGFORGAMES_NMS");
		pdh_value->type = 2;
		pdh_value->string = LocalizedIntString(searchString, curPing);
	} else {
		pdh_value->type = 2;
		pdh_value->string = GetLocalizeString("MENU_SEARCHING_FOR_LOBBY");
	}

	__asm pop edi
	__asm retn
}

// values for hpong result
DWORD* gameState = (DWORD*)0xB2C540;

typedef char* (__cdecl * Com_Sprintf_t)(const char* string, ...);
Com_Sprintf_t Com_Sprintf = (Com_Sprintf_t)0x4785B0;

typedef int (__cdecl * GetNumPartyPlayers_t)(void* party);
GetNumPartyPlayers_t GetNumPartyPlayers = (GetNumPartyPlayers_t)0x497330;

typedef int (__cdecl * GetMaxPartyPlayers_t)(void* party);
GetMaxPartyPlayers_t GetMaxPartyPlayers = (GetMaxPartyPlayers_t)0x4F5D60;

// local variables for hpong result
int curPlayers;
int maxPlayers;
int** currentParty;

char* __cdecl FormatHpongResult(const char* string, int num, char* v1, char* v2) {
	__asm mov eax, [esp + 40h] // 34 + 4 (call return) + 4 (arg 1) + some other 4
	__asm mov currentParty, eax

	curPlayers = GetNumPartyPlayers(*currentParty);
	maxPlayers = GetMaxPartyPlayers(*currentParty);

	return Com_Sprintf("%ihpong %s %s %d %d %d", num, v1, v2, (*gameState > 0), curPlayers, maxPlayers);
}

typedef void (__cdecl * Com_Printf_t)(int, const char*, ...);
Com_Printf_t Com_Printf_p = (Com_Printf_t)0x402500;

CallHook hpongResultHook;
DWORD hpongResultHookLoc = 0x5B44D7;

void __declspec(naked) HpongResultHookFunc() {
	__asm jmp FormatHpongResult
}

void RetryPingSearch() {
	if (!currentMaxPing) {
		currentMaxPing = 50; // ping_default_min->intValue
		pingSearches = -1;
	}

	pingSearches++;
	failedSearches++;

	Com_Printf_p(25, "Min Ping %d Search %d Failed Search Count %d\n", currentMaxPing, pingSearches, failedSearches);

	if (pingSearches > 2/* ping_searches_per->intValue */) {
		currentMaxPing += 10; // ping_increment->intValue
		pingSearches = 0;
	}
}

CallHook retryGameSearchHook;
DWORD retryGameSearchHookLoc = 0x4352FF;

void __declspec(naked) RetryGameSearchHookFunc() {
	RetryPingSearch();
	__asm jmp retryGameSearchHook.pOriginal
}

CallHook qosHook;
DWORD qosHookLoc = 0x4C024E;

int qhPingMS;
int qhTime;
int qhBias = 0;

void __declspec(naked) QosHookFunc() {
	__asm mov qhPingMS, ebx
	__asm mov qhTime, ebp

	qhPingMS += qhBias;

	if (qhPingMS < 1) { qhPingMS = 1; }

	if (qhPingMS > GetCurrentMaxPing()) {
		// this method is a bit lazy, but don't want to translate it to proper C
		// it sets the party slot to inactive
		__asm {
			mov eax, [edi + 0F0h]
			mov byte ptr [esi + eax], 0
		}
	}

	__asm mov ebx, qhPingMS
	__asm mov ebp, qhTime

	// and for security, we put ebx to the printf parameter
	__asm mov [esp + 0Ch], ebx // 4 + 8 (arg3)

	__asm jmp qosHook.pOriginal
}

void PatchLegacyPing() {
	pingDisplayHook.initialize("aaaaa", 5, (PBYTE)pingDisplayHookLoc);
	pingDisplayHook.installHook(PingDisplayHookFunc, true, false);

	hpongResultHook.initialize("aaaaa", (PBYTE)hpongResultHookLoc);
	hpongResultHook.installHook(HpongResultHookFunc, false);

	retryGameSearchHook.initialize("aaaaa", (PBYTE)retryGameSearchHookLoc);
	retryGameSearchHook.installHook(RetryGameSearchHookFunc, false);
}
#endif
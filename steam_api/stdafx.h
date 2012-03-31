#pragma once
#define _CRT_SECURE_NO_WARNINGS

#define DEDICATED 0
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>

#pragma unmanaged
#define NO_STEAM // to reduce header amount needed
#include "CCallback.h"
#include "ISteamClient008.h"
#include "ISteamContentServer002.h"
#include "ISteamUser012.h"
#include "ISteamFriends005.h"
#include "ISteamGameServer009.h"
#include "ISteamMasterServerUpdater001.h"
#include "ISteamMatchmaking007.h"
#include "ISteamNetworking003.h"
#include "ISteamRemoteStorage002.h"
#include "ISteamUtils005.h"
#include "ISteamUserStats006.h"
#include "ISteamApps003.h"
#pragma managed
//#include "steamworks.h"
//#include "steam_api.h"
#include <iostream>
#include <fstream>
//#include <boost/program_options.hpp>
//#include <boost/unordered_map.hpp>
#include <vector>

#pragma unmanaged
//#include "libtomcrypt/tomcrypt.h"

#include <dwmapi.h>

#include "mygui.h"
#include "mygui_directxplatform.h"
#pragma managed
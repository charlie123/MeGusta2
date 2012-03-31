#pragma once

using namespace System;

namespace SteamAPI {
	public ref class SteamAPIHookBase abstract {
	public:
		virtual bool SteamAPIInit(String^ server) { return true; }
		virtual int GetSteamID() { return 0; }

		virtual UInt64 MatchCreateLobby() { return 0; }
		virtual UInt64 MatchJoinLobby(UInt64 lobbyID) { return 0; }
		virtual void MatchSetLobbyData(UInt64 lobbyID, String^ key, String^ value) { }
		virtual String^ MatchGetLobbyData(UInt64 lobbyID, String^ key) { return ""; }
		virtual int MatchGetLobbyMembers(UInt64 lobbyID) { return 1; }
		virtual int MatchGetLobbyMemberByIndex(UInt64 lobbyID, int index) { return 0; }
		virtual int MatchGetLobbyOwner(UInt64 lobbyID) { return 0; }
		virtual void PerformUpdate() { return; }

		virtual void GUIInited() { return; }
		virtual void ShowInviteUI() { return; }

		virtual void G_RunFrame() { return; }
		virtual void G_Say(String^ name, String^ text) { return; }

		virtual void ToggleGlobalGUI(bool on) { return; }
	};
}

public ref class Custom {
public:
	static SteamAPI::SteamAPIHookBase^ Hook;
};
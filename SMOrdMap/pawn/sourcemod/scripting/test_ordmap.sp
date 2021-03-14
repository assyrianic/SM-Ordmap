#include <ordmap>
#include <sdkhooks>
#include <sdktools>


OrdMap g_players;
Handle g_HUD;

public void OnPluginStart() {
	g_players = new OrdMap();
	g_HUD = CreateHudSynchronizer();
	for( int i=MaxClients; i; i-- ) {
		if( IsClientInGame(i) ) {
			OnClientPutInServer(i);
		}
	}
}

public void OnClientPutInServer(int client) {
	g_players.SetCellByCellKey(client, GetClientUserId(client));
}

public void OnClientDisconnect(int client) {
	g_players.SetCellByCellKey(client, 0);
}

public void OnGameFrame() {
	char players_ingame[2048];
	SetHudTextParams(-1.0, 0.20, 0.11, 255, 255, 255, 255);
	
	int entries = g_players.Len;
	for( int i; i<entries; i++ ) {
		int userid;
		if( !g_players.GetCellByIndex(i, userid) || userid <= 0 )
			continue;
		
		int client = GetClientOfUserId(userid);
		if( client <= 0 )
			continue;
		
		Format(players_ingame, sizeof(players_ingame), "%s\n%N", players_ingame, client);
	}
	
	for( int i; i<entries; i++ ) {
		int userid;
		if( !g_players.GetCellByIndex(i, userid) || userid <= 0 )
			continue;
		
		int client = GetClientOfUserId(userid);
		if( client <= 0 )
			continue;
		
		ShowSyncHudText(client, g_HUD, "%s", players_ingame);
	}
}
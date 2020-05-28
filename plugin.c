/*
 * TeamSpeak 3 demo plugin
 *
 * Copyright (c) 2008-2016 TeamSpeak Systems GmbH
 */

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 24

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 512
#define SERVERINFO_BUFSIZE 512
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if (WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

 /* Unique name identifying this plugin */
const char* ts3plugin_name() {
	return "Information";
}

/* Plugin version */
const char* ts3plugin_version() {
	return "1.1";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
	return "Gamer92000";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
	return "Display you some informations about clients, servers and channels.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init() {
	return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
	/* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
	 * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
	 * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
	if (pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */


 /*
  * Implement the following three functions when the plugin should display a line in the server/channel/client info.
  * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
  */

  /* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "More informations";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
	char* name;
	char* uid;
	char* server_ip;
	int* icon_id;
	char* server_id;
	int* talkpower;
	int* clientDataBaseId;
	char str[124];
	int i;
	switch (type) {
	case PLUGIN_SERVER:
		if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &name) != ERROR_ok) { printf("Error getting virtual server name\n"); return; }
		if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_UNIQUE_IDENTIFIER, &uid) != ERROR_ok) { printf("Error getting virtual server uid\n"); return; }
		if (ts3Functions.getServerVariableAsInt(serverConnectionHandlerID, VIRTUALSERVER_ICON_ID, &icon_id) != ERROR_ok) { printf("Error getting virtual server icon id\n"); return; }
		if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_ID, &server_id) != ERROR_ok) { printf("Error getting virtual server id\n"); return; }
		if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_IP, &server_ip) != ERROR_ok) { printf("Error getting virtual server ip\n"); return; }
		*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  // Must be allocated in the plugin!
		snprintf(*data, INFODATA_BUFSIZE, "[B]Server Informations :[/B]\nName:  [color=#990000]%s[/color]\nUID: [color=#990000]%s[/color]\nIcon ID: [color=#990000]%d[/color]\nVirtualServer ID: [color=#990000]%s[/color]\nIP: [color=#990000]%s[/color]", name, uid, icon_id, server_id, server_ip);
		
		sprintf_s(str, 124, "\nName Length: [color=#990000]%d[/color]", strlen(name));
		strcat_s(*data, INFODATA_BUFSIZE, str);
		strcat_s(*data, INFODATA_BUFSIZE, "\nAdvanced Name Inspect:\n[color=#990000]");

		i = 0;
		while (name[i] != '\0') {
			char output[4];
			sprintf(output, "%02X ", name[i]);
			strcat_s(*data, INFODATA_BUFSIZE, output);
			i++;
		}
		strcat_s(*data, INFODATA_BUFSIZE, "[/color]");
		
		break;
	case PLUGIN_CHANNEL:
		if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_NAME, &name) != ERROR_ok) { printf("Error getting channel name\n"); return; }
		if (ts3Functions.getChannelVariableAsInt(serverConnectionHandlerID, id, CHANNEL_ICON_ID, &icon_id) != ERROR_ok) { printf("Error getting channel icon_id\n"); return; }
		*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  // Must be allocated in the plugin!
		snprintf(*data, INFODATA_BUFSIZE, "[B]Channel Informations :[/B]\nName: [color=#990000]%s[/color]\nID: [color=#990000]%d[/color]\nIcon ID: [color=#990000]%d[/color]", name, id, icon_id);
		
		sprintf_s(str, 124, "\nName Length: [color=#990000]%d[/color]", strlen(name));
		strcat_s(*data, INFODATA_BUFSIZE, str);
		strcat_s(*data, INFODATA_BUFSIZE, "\nAdvanced Name Inspect:\n[color=#990000]");

		i = 0;
		while (name[i] != '\0') {
			char output[4];
			sprintf(output, "%02X ", name[i]);
			strcat_s(*data, INFODATA_BUFSIZE, output);
			i++;
		}
		strcat_s(*data, INFODATA_BUFSIZE, "[/color]");

		break;
	case PLUGIN_CLIENT:
		if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME, &name) != ERROR_ok) { printf("Error getting client nickname\n"); return; }
		if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_UNIQUE_IDENTIFIER, &uid) != ERROR_ok) { printf("Error getting client uid\n"); return; }
		if (ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, (anyID)id, CLIENT_TALK_POWER, &talkpower) != ERROR_ok) { printf("Error getting client talkpower\n"); return; }
		if (ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, (anyID)id, CLIENT_DATABASE_ID, &clientDataBaseId) != ERROR_ok) { printf("Error getting client DataBaseId\n"); return; }
		if (ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, (anyID)id, CLIENT_ICON_ID, &icon_id) != ERROR_ok) { printf("Error getting client icon id\n"); return; }
		*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  // Must be allocated in the plugin!
		snprintf(*data, INFODATA_BUFSIZE, "[B]Client Informations :[/B]\nName:  [color=#990000]%s[/color]\nUID: [color=#990000]%s[/color]\nID: [color=#990000]%d[/color]\nDBID: [color=#990000]%d[/color]\nTalkPower: [color=#990000]%d[/color]\nIcon ID: [color=#990000]%d[/color]", name, uid, (anyID)id, clientDataBaseId, talkpower, icon_id);
		
		sprintf_s(str, 124, "\nName Length: [color=#990000]%d[/color]", strlen(name));
		strcat_s(*data, INFODATA_BUFSIZE, str);
		strcat_s(*data, INFODATA_BUFSIZE, "\nAdvanced Name Inspect:\n[color=#990000]");

		i = 0;
		while (name[i] != '\0') {
			char output[4];
			sprintf(output, "%02X ", name[i]);
			strcat_s(*data, INFODATA_BUFSIZE, output);
			i++;
		}
		strcat_s(*data, INFODATA_BUFSIZE, "[/color]");
		
		break;
	default:
		printf("Invalid item type: %d\n", type);
		data = NULL;  //Ignore
		return;
	}
	ts3Functions.freeMemory(name, uid, server_id, server_ip, icon_id, server_id);
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
	free(data);
}

/* Helper function to create a hotkey */
static struct PluginHotkey* createHotkey(const char* keyword, const char* description) {
	struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
	return hotkey;
}


/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

 /* Clientlib */

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
	/* Some example code following to show how to use the information query functions. */
	if (1) {
		if (newStatus == STATUS_CONNECTION_ESTABLISHED) {  /* connection established and we have client and channels available */
			char* s;
			char msg[1024];
			anyID myID;
			uint64* ids;
			size_t i;
			unsigned int error;

			/* Print clientlib version */
			if (ts3Functions.getClientLibVersion(&s) == ERROR_ok) {
				printf("PLUGIN: Client lib version: %s\n", s);
				ts3Functions.freeMemory(s);  /* Release string */
			}
			else {
				ts3Functions.logMessage("Error querying client lib version", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}

			/* Write plugin name and version to log */
			snprintf(msg, sizeof(msg), "Plugin %s, Version %s, Author: %s", ts3plugin_name(), ts3plugin_version(), ts3plugin_author());
			ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);

			/* Print virtual server name */
			if ((error = ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &s)) != ERROR_ok) {
				if (error != ERROR_not_connected) {  /* Don't spam error in this case (failed to connect) */
					ts3Functions.logMessage("Error querying server name", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				}
				return;
			}
			printf("PLUGIN: Server name: %s\n", s);
			ts3Functions.freeMemory(s);

			/* Print virtual server welcome message */
			if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &s) != ERROR_ok) {
				ts3Functions.logMessage("Error querying server welcome message", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}
			printf("PLUGIN: Server welcome message: %s\n", s);
			ts3Functions.freeMemory(s);  /* Release string */

			/* Print own client ID and nickname on this server */
			if (ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
				ts3Functions.logMessage("Error querying client ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}
			if (ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_NICKNAME, &s) != ERROR_ok) {
				ts3Functions.logMessage("Error querying client nickname", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}
			printf("PLUGIN: My client ID = %d, nickname = %s\n", myID, s);
			ts3Functions.freeMemory(s);

			/* Print list of all channels on this server */
			if (ts3Functions.getChannelList(serverConnectionHandlerID, &ids) != ERROR_ok) {
				ts3Functions.logMessage("Error getting channel list", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}
			printf("PLUGIN: Available channels:\n");
			for (i = 0; ids[i]; i++) {
				/* Query channel name */
				if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, ids[i], CHANNEL_NAME, &s) != ERROR_ok) {
					ts3Functions.logMessage("Error querying channel name", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
					return;
				}
				printf("PLUGIN: Channel ID = %llu, name = %s\n", (long long unsigned int)ids[i], s);
				ts3Functions.freeMemory(s);
			}
			ts3Functions.freeMemory(ids);  /* Release array */

			/* Print list of existing server connection handlers */
			printf("PLUGIN: Existing server connection handlers:\n");
			if (ts3Functions.getServerConnectionHandlerList(&ids) != ERROR_ok) {
				ts3Functions.logMessage("Error getting server list", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}
			for (i = 0; ids[i]; i++) {
				if ((error = ts3Functions.getServerVariableAsString(ids[i], VIRTUALSERVER_NAME, &s)) != ERROR_ok) {
					if (error != ERROR_not_connected) {  /* Don't spam error in this case (failed to connect) */
						ts3Functions.logMessage("Error querying server name", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
					}
					continue;
				}
				printf("- %llu - %s\n", (long long unsigned int)ids[i], s);
				ts3Functions.freeMemory(s);
			}
			ts3Functions.freeMemory(ids);
		}
	}
}

void ts3plugin_onNewChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID) {
}

void ts3plugin_onNewChannelCreatedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onDelChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onChannelMoveEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 newChannelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onUpdateChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onUpdateClientEvent(uint64 serverConnectionHandlerID, anyID clientID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
}

void ts3plugin_onClientMoveSubscriptionEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility) {
}

void ts3plugin_onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* timeoutMessage) {
}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
}

void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
}

void ts3plugin_onClientIDsEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, anyID clientID, const char* clientName) {
}

void ts3plugin_onClientIDsFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerEditedEvent(uint64 serverConnectionHandlerID, anyID editerID, const char* editerName, const char* editerUniqueIdentifier) {
}

void ts3plugin_onServerUpdatedEvent(uint64 serverConnectionHandlerID) {
}

int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage) {
	return 0;
}

void ts3plugin_onServerStopEvent(uint64 serverConnectionHandlerID, const char* shutdownMessage) {
}

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored) {
	return 0;  /* 0 = handle normally, 1 = client will ignore the text message */
}

void ts3plugin_onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID) {
}

void ts3plugin_onConnectionInfoEvent(uint64 serverConnectionHandlerID, anyID clientID) {
}

void ts3plugin_onServerConnectionInfoEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelSubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelSubscribeFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelUnsubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelUnsubscribeFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelDescriptionUpdateEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelPasswordChangedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onPlaybackShutdownCompleteEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onSoundDeviceListChangedEvent(const char* modeID, int playOrCap) {
}

void ts3plugin_onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels) {
}

void ts3plugin_onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
}

void ts3plugin_onEditMixedPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
}

void ts3plugin_onEditCapturedVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited) {
}

void ts3plugin_onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume) {
}

void ts3plugin_onCustom3dRolloffCalculationWaveEvent(uint64 serverConnectionHandlerID, uint64 waveHandle, float distance, float* volume) {
}

void ts3plugin_onUserLoggingMessageEvent(const char* logMessage, int logLevel, const char* logChannel, uint64 logID, const char* logTime, const char* completeLogString) {
}

/* Clientlib rare */

void ts3plugin_onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, uint64 time, const char* kickMessage) {
}

int ts3plugin_onClientPokeEvent(uint64 serverConnectionHandlerID, anyID fromClientID, const char* pokerName, const char* pokerUniqueIdentity, const char* message, int ffIgnored) {
}

void ts3plugin_onClientSelfVariableUpdateEvent(uint64 serverConnectionHandlerID, int flag, const char* oldValue, const char* newValue) {
}

void ts3plugin_onFileListEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path, const char* name, uint64 size, uint64 datetime, int type, uint64 incompletesize, const char* returnCode) {
}

void ts3plugin_onFileListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path) {
}

void ts3plugin_onFileInfoEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* name, uint64 size, uint64 datetime) {
}

void ts3plugin_onServerGroupListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, const char* name, int type, int iconID, int saveDB) {
}

void ts3plugin_onServerGroupListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
}

void ts3plugin_onServerGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onServerGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID) {
}

void ts3plugin_onServerGroupClientListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, uint64 clientDatabaseID, const char* clientNameIdentifier, const char* clientUniqueID) {
}

void ts3plugin_onChannelGroupListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, const char* name, int type, int iconID, int saveDB) {
}

void ts3plugin_onChannelGroupListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID) {
}

void ts3plugin_onChannelPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onClientPermListEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID) {
}

void ts3plugin_onChannelClientPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID) {
}

void ts3plugin_onClientChannelGroupChangedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, uint64 channelID, anyID clientID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

int ts3plugin_onServerPermissionErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, unsigned int failedPermissionID) {
	return 0;
}

void ts3plugin_onPermissionListGroupEndIDEvent(uint64 serverConnectionHandlerID, unsigned int groupEndID) {
}

void ts3plugin_onPermissionListEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, const char* permissionName, const char* permissionDescription) {
}

void ts3plugin_onPermissionListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onPermissionOverviewEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, uint64 channelID, int overviewType, uint64 overviewID1, uint64 overviewID2, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onPermissionOverviewFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

void ts3plugin_onClientNeededPermissionsEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, int permissionValue) {
}

void ts3plugin_onClientNeededPermissionsFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onFileTransferStatusEvent(anyID transferID, unsigned int status, const char* statusMessage, uint64 remotefileSize, uint64 serverConnectionHandlerID) {
}

void ts3plugin_onClientChatClosedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {
}

void ts3plugin_onClientChatComposingEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {
}

void ts3plugin_onServerLogEvent(uint64 serverConnectionHandlerID, const char* logMsg) {
}

void ts3plugin_onServerLogFinishedEvent(uint64 serverConnectionHandlerID, uint64 lastPos, uint64 fileSize) {
}

void ts3plugin_onMessageListEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, uint64 timestamp, int flagRead) {
}

void ts3plugin_onMessageGetEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, const char* message, uint64 timestamp) {
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
}

void ts3plugin_onClientNamefromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {
}

void ts3plugin_onClientNamefromDBIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {
}

void ts3plugin_onComplainListEvent(uint64 serverConnectionHandlerID, uint64 targetClientDatabaseID, const char* targetClientNickName, uint64 fromClientDatabaseID, const char* fromClientNickName, const char* complainReason, uint64 timestamp) {
}

void ts3plugin_onBanListEvent(uint64 serverConnectionHandlerID, uint64 banid, const char* ip, const char* name, const char* uid, uint64 creationTime, uint64 durationTime, const char* invokerName,
	uint64 invokercldbid, const char* invokeruid, const char* reason, int numberOfEnforcements, const char* lastNickName) {
}

void ts3plugin_onClientServerQueryLoginPasswordEvent(uint64 serverConnectionHandlerID, const char* loginPassword) {
}

void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* pluginName, const char* pluginCommand) {
}

void ts3plugin_onIncomingClientQueryEvent(uint64 serverConnectionHandlerID, const char* commandText) {
}

void ts3plugin_onServerTemporaryPasswordListEvent(uint64 serverConnectionHandlerID, const char* clientNickname, const char* uniqueClientIdentifier, const char* description, const char* password, uint64 timestampStart, uint64 timestampEnd, uint64 targetChannelID, const char* targetChannelPW) {
}

/* Client UI callbacks */

/*
 * Called from client when an avatar image has been downloaded to or deleted from cache.
 * This callback can be called spontaneously or in response to ts3Functions.getAvatar()
 */
void ts3plugin_onAvatarUpdated(uint64 serverConnectionHandlerID, anyID clientID, const char* avatarPath) {
}

/*
 * Called when a plugin menu item (see ts3plugin_initMenus) is triggered. Optional function, when not using plugin menus, do not implement this.
 *
 * Parameters:
 * - serverConnectionHandlerID: ID of the current server tab
 * - type: Type of the menu (PLUGIN_MENU_TYPE_CHANNEL, PLUGIN_MENU_TYPE_CLIENT or PLUGIN_MENU_TYPE_GLOBAL)
 * - menuItemID: Id used when creating the menu item
 * - selectedItemID: Channel or Client ID in the case of PLUGIN_MENU_TYPE_CHANNEL and PLUGIN_MENU_TYPE_CLIENT. 0 for PLUGIN_MENU_TYPE_GLOBAL.
 */
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
}

/* This function is called if a plugin hotkey was pressed. Omit if hotkeys are unused. */
void ts3plugin_onHotkeyEvent(const char* keyword) {
}

/* Called when recording a hotkey has finished after calling ts3Functions.requestHotkeyInputDialog */
void ts3plugin_onHotkeyRecordedEvent(const char* keyword, const char* key) {
}

/* Called when client custom nickname changed */
void ts3plugin_onClientDisplayNameChanged(uint64 serverConnectionHandlerID, anyID clientID, const char* displayName, const char* uniqueClientIdentifier) {
}

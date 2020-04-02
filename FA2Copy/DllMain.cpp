// FA2Copy DllMain.cpp
// Programed by SEC-SOME

// In fact I was about to program a MFC dll
// but then found it unnecessary XD
#include "stdafx.h"

#pragma region Global Variables
// Global Variables

logger g_logger; // logger

HWND g_FA2Wnd; // Final Alert 2 Window Handle
HMODULE g_hModule;// Final Alert 2 HModule, HInstance as well

HHOOK g_CallWndHook, g_GetMsgHook;
// CallWndHook: Catch messages
// GetMsgHook: For initializations

// Some special flags
BOOL g_GetMsgHooked;
BOOL g_TaskforcesRead;
BOOL g_TaskforceComboFlag;
BOOL g_TerrainTheaterFlag;
BOOL g_AllowHotKey;

ATOM g_CTRL_S, g_CTRL_O, g_CTRL_N; // Hot Keys
WNDPROC g_oldProc; // Save old Proc for Hot Keys

// Some Handle needs to be global for some reason XD
HWND g_TerrainWnd;
HWND g_SysTreeView;

// Store templates
std::vector<TeamTemplate> g_TeamTemplates;
std::vector<ScriptTemplate> g_ScriptTemplates;
std::vector<TerrainSort> g_TerrainSorts;

// Global strings for further use
std::string g_TerrainTheater;
std::string g_Path;

Ini g_ini; // Ini Config file

// Necessary items for FindWindow
struct FindWindowConfig {
	std::string DialogClass, MapWnd, IniWnd, HouseWnd, TriggerWnd, TagWnd, 
		TriggerGlobalWnd, TriggerEventWnd, TriggerActionWnd,
		TaskforceWnd, ScriptWnd, TeamWnd, AITriggerWnd, TerrainWnd,
		SaveWnd, LoadWnd, NewWnd1, NewWnd2, NewWnd3;
}g_FindWindowConfig;

// Necessary items for MessageBox
class MessageBoxConfig {
private:
	struct s_Captain {
		std::string Hint, Error, Warning;
	};
	struct s_Message {
		std::string HookFailed, UnHookFailed, IniNotExist, TerrainDisabled,
			TerrainMapUnloaded, TerrainMapUnknown, TriggerEventFull, TriggerActionFull,
			HouseWndNotFound, INISectionNotFound;
	};
public:
	MessageBoxConfig() {
		Message.HookFailed = "FA2Copy failed to hook!";
		Message.UnHookFailed = "FA2Copy failed to unhook!";
		Message.IniNotExist = "Cannot read FA2CopyData.ini!";
	}
	s_Captain Captain;
	s_Message Message;
}g_MessageBoxConfig;
#pragma endregion

#pragma region Function Definations
// Function Definations

// Set hooks and Unhook
BOOL StartHook();
BOOL EndHook();

// Hook Proc
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);


// Replace for Hotkeys
LRESULT CALLBACK HotkeyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Get Window
HWND GetWindowHandle();
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
BOOL CALLBACK EnumChildWindowsProc(HWND hwnd, LPARAM lParam);

// Dialog Proc
BOOL CALLBACK HouseDlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

// Functionality Functions
std::string GetPath();
void LoadTerrainGroups(std::string Theater);
void LoadTeamTemplates();
void LoadScriptTemplates();
void GetTreeViewHwnd();
void LoadINI();
void LoadFA2CopyConfig();
#pragma endregion

#pragma region Function Realizations
//Function Realizations

// Set hooks and Unhook
BOOL StartHook()
{
	g_CallWndHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hModule, GetCurrentThreadId());
	g_GetMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, GetModuleHandle(NULL), GetCurrentThreadId());
	return ((g_CallWndHook != NULL) && (g_GetMsgHook != NULL));
}
BOOL EndHook()
{
	return (UnhookWindowsHookEx(g_CallWndHook) && UnhookWindowsHookEx(g_GetMsgHook));
}

// Hook Proc
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		CWPSTRUCT* cwps = (CWPSTRUCT*)lParam;
		if (cwps->message == WM_COMMAND)
		{
			INT wmId = LOWORD(cwps->wParam);
			INT wmHi = HIWORD(cwps->wParam);
			switch (wmId)
			{

			//Used ID
			//9970 - 9999
			
			//House
			case 9973: {//Allies Manager
				HWND HouseWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.HouseWnd.c_str()
				);
				if (HouseWnd == NULL) {
					g_logger.Error("Cannot locate the House window.");
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.HouseWndNotFound.c_str(),
						g_MessageBoxConfig.Captain.Error.c_str(),
						MB_OK
					);
					break;
				}
				g_logger.Info("Successfully located the House window.");

				HWND ComboBox, Edit;
				ComboBox = GetDlgItem(HouseWnd, 1091);
				Edit = GetDlgItem(HouseWnd, 1099);

				int HouseCount = SendMessage(ComboBox, CB_GETCOUNT, NULL, NULL);
				int CurHouseIndex = SendMessage(ComboBox, CB_GETCURSEL, NULL, NULL);
				TCHAR* str;
				int strLen = GetWindowTextLength(ComboBox) + 1;
				str = new TCHAR[strLen];
				GetWindowText(ComboBox, str, strLen);
				std::string CurHouseStr = str;
				g_ini.Trim(CurHouseStr);

				std::unordered_map<std::string, bool> Houses;
				for (register int i = 0; i < HouseCount; ++i) {
					SendMessage(ComboBox, CB_SETCURSEL, i, NULL);
					strLen = GetWindowTextLength(ComboBox) + 1;
					delete[] str;
					str = new TCHAR[strLen];
					GetWindowText(ComboBox, str, strLen);
					std::string key = str;
					Houses[key] = false;
				}
				delete[] str;
				SendMessage(ComboBox, CB_SETCURSEL, HouseCount >= 0 ? CurHouseIndex : -1, NULL);
				TCHAR* AllieStr;
				strLen= GetWindowTextLength(Edit) + 1;
				AllieStr = new TCHAR[strLen];
				GetWindowText(Edit, AllieStr, strLen);
				std::string AlliedText = AllieStr;
				delete[] AllieStr;

				std::vector<std::string> AllieStrs = g_ini.Split(AlliedText, ',');
				for (register int i = 0, cnt = AllieStrs.size(); i < cnt; ++i) {
					g_ini.Trim(AllieStrs[i]);
					if (Houses.find(AllieStrs[i]) != Houses.end()) //House Exists
						Houses[AllieStrs[i]] = true;
				}
				
				Houses.erase(CurHouseStr);

				std::pair<std::unordered_map<std::string, bool>*, std::pair<std::string*, std::string*>*> Param;
				std::pair<std::string*, std::string*> StringParam = std::make_pair(&AlliedText, &CurHouseStr);
				
				Param.first = &Houses;
				Param.second = &StringParam;

				DialogBoxParam(g_hModule, MAKEINTRESOURCE(IDD_DIALOG1), HouseWnd, HouseDlgProc, (LPARAM)&Param);
				
				SetWindowText(Edit, AlliedText.c_str());
				
				break;
			}
			//Tree View Debug
			/*case 9974: {
				g_logger.Info("Tree View Debug");
				break;
			}*/
			//Terrain Group
			case 9975: {//Sub ComboBox
				switch (wmHi) {
				case CBN_SELCHANGE: {
					g_logger.Info("Terrain Sub SELCHANGE");
					HWND TerrainWnd1 = FindWindowEx(g_FA2Wnd, NULL, "AfxFrameOrView42s", NULL);
					HWND TerrainWnd2 = FindWindowEx(TerrainWnd1, NULL, "AfxMDIFrame42s", NULL);
					EnumChildWindows(TerrainWnd2, EnumChildWindowsProc, NULL);
					HWND& TerrainWnd = g_TerrainWnd;
					HWND ComboMain = GetDlgItem(TerrainWnd, 9983);
					HWND ComboSub = GetDlgItem(TerrainWnd, 9975);
					HWND ComboReal = GetDlgItem(TerrainWnd, 1366);
					int MainCount = SendMessage(ComboMain, CB_GETCOUNT, NULL, NULL);
					if (MainCount <= 0)	break;
					int MainIndex = SendMessage(ComboMain, CB_GETCURSEL, NULL, NULL);
					if (MainIndex < 0)	break;
					int SubCount = g_TerrainSorts[MainIndex].Count();
					if (SubCount <= 0)	break;
					int SubIndex = SendMessage(ComboSub, CB_GETCURSEL, NULL, NULL);
					if (SubIndex < 0)	break;
					std::string TerrainId = g_TerrainSorts[MainIndex][SubIndex]->second;
					int FindIndex = SendMessage(ComboReal, CB_FINDSTRING, -1, (LPARAM)TerrainId.c_str());
					if (FindIndex == CB_ERR)	FindIndex = 0;
					SendMessage(ComboReal, CB_SETCURSEL, FindIndex, NULL);
					SendMessage(TerrainWnd, WM_COMMAND, MAKEWPARAM(1366, CBN_SELCHANGE), (LPARAM)ComboReal);
					break;
				}
				default:
					break;
				}
				break;
			}
			case 9983: {//Main ComboBox
				switch (wmHi) {
				case CBN_SELCHANGE: {
					g_logger.Info("Terrain Main SELCHANGE");
					HWND TerrainWnd1 = FindWindowEx(g_FA2Wnd, NULL, "AfxFrameOrView42s", NULL);
					HWND TerrainWnd2 = FindWindowEx(TerrainWnd1, NULL, "AfxMDIFrame42s", NULL);
					EnumChildWindows(TerrainWnd2, EnumChildWindowsProc, NULL);
					HWND& TerrainWnd = g_TerrainWnd;
					HWND ComboMain = GetDlgItem(TerrainWnd, 9983);
					HWND ComboSub = GetDlgItem(TerrainWnd, 9975);

					int MainCount = SendMessage(ComboMain, CB_GETCOUNT, NULL, NULL);
					if (MainCount <= 0)	break;
					int MainIndex = SendMessage(ComboMain, CB_GETCURSEL, NULL, NULL);
					if (MainIndex < 0)	break;
					int SubCount = g_TerrainSorts[MainIndex].Count();
					SendMessage(ComboSub, CB_RESETCONTENT, NULL, NULL);
					SendMessage(ComboSub, CB_SETCURSEL, -1, NULL);
					if (SubCount <= 0)	break;
					for (register int i = 0; i < SubCount; ++i)
						SendMessage(ComboSub, CB_ADDSTRING, NULL, (LPARAM)g_TerrainSorts[MainIndex][i]->first.c_str());
					SendMessage(ComboSub, CB_SETCURSEL, 0, NULL);
					SendMessage(TerrainWnd, WM_COMMAND, MAKEWPARAM(9975, CBN_SELCHANGE), (LPARAM)ComboSub);
					break;
				}
				default:
					break;
				}
				break;
			}
			case 9984: {//Reload Button
				g_logger.Info("Reload Terrain Group");
				if (!g_TerrainTheaterFlag) {
					g_TerrainTheaterFlag = TRUE;
					SendMessage(g_FA2Wnd, WM_COMMAND, MAKEWPARAM(40040, BN_CLICKED), NULL);
					HWND MapWnd = FindWindow(
						g_FindWindowConfig.DialogClass.c_str(),
						g_FindWindowConfig.MapWnd.c_str()
					);
					HWND ComboTerrain = GetDlgItem(MapWnd, 1046);
					TCHAR* str;
					int strLen = GetWindowTextLength(ComboTerrain) + 1;
					str = new TCHAR[strLen];
					GetWindowText(ComboTerrain, str, strLen);
					g_TerrainTheater = str;
					SendMessage(MapWnd, WM_CLOSE, NULL, NULL);
					delete[] str;
					break;
				}
				HWND TerrainWnd1 = FindWindowEx(g_FA2Wnd, NULL, "AfxFrameOrView42s", NULL);
				HWND TerrainWnd2 = FindWindowEx(TerrainWnd1, NULL, "AfxMDIFrame42s", NULL);
				EnumChildWindows(TerrainWnd2, EnumChildWindowsProc, NULL);
				HWND& TerrainWnd = g_TerrainWnd;
				HWND ComboMain = GetDlgItem(TerrainWnd, 9983);
				HWND ComboSub = GetDlgItem(TerrainWnd, 9975);

				SendMessage(ComboMain, CB_RESETCONTENT, NULL, NULL);
				SendMessage(ComboSub, CB_RESETCONTENT, NULL, NULL);
				SendMessage(ComboMain, CB_SETCURSEL, -1, NULL);
				SendMessage(ComboSub, CB_SETCURSEL, -1, NULL);

				g_TerrainTheaterFlag = FALSE;
				std::string StandardTheater[7] = { "","TEMPERATE","SNOW","URBAN","NEWURBAN","LUNAR","DESERT" };
				if (g_TerrainTheater == StandardTheater[0]) {
					g_logger.Info("No map has been loaded while reading");
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.TerrainMapUnloaded.c_str(),
						g_MessageBoxConfig.Captain.Hint.c_str(),
						MB_OK
					);
					break;
				}

				BOOL Flag = FALSE;

				for (register int i = 1; i <= 6; ++i)
					if (g_TerrainTheater == StandardTheater[i]) {
						g_logger.Info("Start to load terrain groups: " + g_TerrainTheater);
						LoadTerrainGroups(StandardTheater[i]);
						Flag = TRUE;
						break;
					}
				if (!Flag)
				{
					g_logger.Error("Loaded unknown terrain threater");
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.TerrainMapUnknown.c_str(),
						g_MessageBoxConfig.Captain.Error.c_str(),
						MB_OK
					);
				}
				break;
			}
			//INI
			case 9971: {
				HWND INIWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.IniWnd.c_str()
				);
				HWND SearchEdit = GetDlgItem(INIWnd, 9972);
				HWND CheckBox = GetDlgItem(INIWnd, 9970);
				HWND ComboBox = GetDlgItem(INIWnd, 1036);
				int strLen = GetWindowTextLength(SearchEdit) + 1;
				TCHAR* str = new TCHAR[strLen];
				GetWindowText(SearchEdit, str, strLen);
				int FindStartIndex = (SendMessage(CheckBox, BM_GETCHECK, NULL, NULL) == BST_CHECKED)
					? -1
					: SendMessage(ComboBox, CB_GETCURSEL, NULL, NULL);
				int FindResult = SendMessage(
					ComboBox,
					CB_FINDSTRING, 
					FindStartIndex,
					(LPARAM)str
				);

				if (FindResult == CB_ERR || (FindResult < FindStartIndex && FindStartIndex >= 0))
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.INISectionNotFound.c_str(),
						g_MessageBoxConfig.Captain.Hint.c_str(),
						MB_OK
					);
				else {
					SendMessage(ComboBox, CB_SETCURSEL, FindResult, NULL);
					SendMessage(INIWnd, WM_COMMAND, MAKELPARAM(1036, CBN_SELCHANGE), (LPARAM)ComboBox);
				}
				delete[] str;
				break;
			}
			case 9982: {//Override the Add Button
				g_logger.Info("Add INI Section");
				HWND INIWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.IniWnd.c_str()
				);
				HWND BtnAdd = GetDlgItem(INIWnd, 1037);
				HWND ComboBox = GetDlgItem(INIWnd, 1036);

				//Remember Sections
				int SectionCount = SendMessage(ComboBox, CB_GETCOUNT, NULL, NULL);
				std::vector<TCHAR*> KeyDictionary(SectionCount);
				for (register int i = 0; i < SectionCount; ++i) {
					int strLen = SendMessage(ComboBox, CB_GETLBTEXTLEN, NULL, NULL) + 1;
					KeyDictionary[i] = new TCHAR[strLen];
					SendMessage(ComboBox, CB_GETLBTEXT, i, (LPARAM)KeyDictionary[i]);
				}

				g_logger.Info(std::to_string(SectionCount) + "INI section loaded");

				//New Section
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1037, NULL);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1037, NULL);

				register int i;
				for (i = 0; i < SectionCount; ++i) {
					SendMessage(ComboBox, CB_SETCURSEL, i, NULL);
					SendMessage(INIWnd, WM_COMMAND, MAKEWPARAM(1036, CBN_SELCHANGE), (LPARAM)ComboBox);
					TCHAR* str;
					int strLen = SendMessage(ComboBox, CB_GETLBTEXTLEN, i, NULL) + 1;
					str = new TCHAR[strLen];
					SendMessage(ComboBox, CB_GETLBTEXT, i, (LPARAM)str);
					if (strcmp(str, KeyDictionary[i]) != 0) {
						delete[] str;
						break;
					}
					delete[] str;
				}
				SendMessage(ComboBox, CB_SETCURSEL, i, NULL);
				SendMessage(INIWnd, WM_COMMAND, MAKEWPARAM(1036, CBN_SELCHANGE), (LPARAM)ComboBox);

				for (auto x : KeyDictionary)	delete[] x;

				break;
			}
			//Tag
			case 9970: {//Copy Tag
				HWND TagWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TagWnd.c_str()
				);

				HWND TagCombo = GetDlgItem(TagWnd, 1083);
				if (SendMessage(TagCombo, CB_GETCURSEL, NULL, NULL) == CB_ERR)	break;

				HWND BtnAdd = GetDlgItem(TagWnd, 1154);
				int ID[3] = { 1010,1156,1157 };
				HWND Control[3];
				TCHAR* CopyData[3];
				for (register int i = 0; i < 3; ++i) {
					Control[i] = GetDlgItem(TagWnd, ID[i]);
					int strLen = GetWindowTextLength(Control[i]) + 1;
					CopyData[i] = new TCHAR[strLen];
					GetWindowText(Control[i], CopyData[i], strLen);
				}
					
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1154, NULL);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1154, NULL);
				
				//SetWindowText(Control[0], strcat(CopyData[0], " Clone"));
				std::string NewName = CopyData[0];
				NewName += " Clone";
				SetWindowText(Control[0], NewName.c_str());

				for (register int i = 1; i < 3; ++i) {
					SetWindowText(Control[i], CopyData[i]);
					SendMessage(TagWnd, WM_COMMAND, MAKEWPARAM(ID[i], CBN_EDITCHANGE), (LPARAM)Control[i]);
				}
				for (register int i = 0; i < 3; ++i)	delete[] CopyData[i];

				break;
			}
			//Trigger
			case 9987: {
				g_logger.Info("Copy Repeat Type");
				HWND TriggerWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TriggerWnd.c_str()
				);
				HWND TriggerTabWnd = GetDlgItem(FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TriggerWnd.c_str()
				), 1393);
				HWND GlobalWnd = FindWindowEx(
					TriggerTabWnd,
					NULL,
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TriggerGlobalWnd.c_str()
				);
				HWND ComboBox = GetDlgItem(GlobalWnd, 1394);
				HWND BtnCopy = GetDlgItem(TriggerWnd, 1163);
				int CurType;
				TCHAR *str;
				int strLen = GetWindowTextLength(ComboBox) + 1;
				str = new TCHAR[strLen];
				GetWindowText(ComboBox, str, strLen);
				switch (str[0]) {
				case '0':
					CurType = 0;
					break;
				case '1':
					CurType = 1;
					break;
				case '2':
					CurType = 2;
					break;
				default:
					CurType = 0;
					break;
				}
				g_logger.Info("Current trigger repeat type: " + str[0]);
				SendMessage(BtnCopy, WM_LBUTTONDOWN, 1403, NULL);
				SendMessage(BtnCopy, WM_LBUTTONUP, 1403, NULL);
				SendMessage(ComboBox, CB_SETCURSEL, CurType, NULL);
				SendMessage(GlobalWnd, WM_COMMAND, MAKEWPARAM(1394, CBN_SELCHANGE), (LPARAM)ComboBox);
				delete[] str;
				break;
			}
			case 9988: {
				g_logger.Info("Add Event Member");
				HWND TriggerTabWnd = GetDlgItem(
					FindWindow(
						g_FindWindowConfig.DialogClass.c_str(),
						g_FindWindowConfig.TriggerWnd.c_str()
					),
					1393);
				HWND EventWnd = FindWindowEx(
					TriggerTabWnd,
					NULL,
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TriggerEventWnd.c_str()
				);
				HWND CurrEventHandle = GetDlgItem(EventWnd, 1167);
				HWND BtnAdd = GetDlgItem(EventWnd, 1396);
				int CurrEventCount = SendMessage(CurrEventHandle, CB_GETCOUNT, NULL, NULL);
				int CurIndex = SendMessage(CurrEventHandle, CB_GETCURSEL, NULL, NULL);
				int DesIndex;
				if (CurrEventCount <= 9)	DesIndex = CurrEventCount + 1;
				else
				{
					++CurIndex;
					DesIndex = CurrEventCount - 8;
				}
				if (CurrEventCount > 19) {
					g_logger.Warn("Current trigger's event is full");
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.TriggerEventFull.c_str(),
						g_MessageBoxConfig.Captain.Warning.c_str(),
						MB_OK
					);
				}
				else {
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1403, NULL);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1403, NULL);
					SendMessage(CurrEventHandle, CB_SETCURSEL, DesIndex, NULL);
					SendMessage(EventWnd, WM_COMMAND, MAKEWPARAM(1167, CBN_SELCHANGE), (LPARAM)CurrEventHandle);
				}
				break;
			}
			case 9989: {
				g_logger.Info("Add Action Member");
				HWND TriggerTabWnd = GetDlgItem(
					FindWindow(
						g_FindWindowConfig.DialogClass.c_str(),
						g_FindWindowConfig.TriggerWnd.c_str()
					),
					1393);
				HWND ActionWnd = FindWindowEx(
					TriggerTabWnd, 
					NULL, 
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TriggerActionWnd.c_str()
				);
				HWND CurrActionHandle = GetDlgItem(ActionWnd, 1170);
				HWND BtnAdd = GetDlgItem(ActionWnd, 1403);
				int CurrActionCount = SendMessage(CurrActionHandle, CB_GETCOUNT, NULL, NULL);
				int CurIndex = SendMessage(CurrActionHandle, CB_GETCURSEL, NULL, NULL);
				int DesIndex;
				if (CurrActionCount <= 9)	DesIndex = CurrActionCount + 1;
				else
				{
					++CurIndex;
					DesIndex = CurrActionCount - 8;
				}
				if (CurrActionCount > 19) {
					g_logger.Warn("Current trigger's action is full");
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.TriggerActionFull.c_str(),
						g_MessageBoxConfig.Captain.Warning.c_str(),
						MB_OK
					);
				}
				else {
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1403, NULL);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1403, NULL);
					SendMessage(CurrActionHandle, CB_SETCURSEL, DesIndex, NULL);
					SendMessage(ActionWnd, WM_COMMAND, MAKEWPARAM(1170, CBN_SELCHANGE), (LPARAM)CurrActionHandle);
				}
				break;
			}
			case 9990: {//Copy Event Member
				g_logger.Info("Copy Event Member");
				HWND TriggerTabWnd = GetDlgItem(
					FindWindow(
						g_FindWindowConfig.DialogClass.c_str(),
						g_FindWindowConfig.TriggerWnd.c_str()
					),
					1393);
				HWND EventWnd = FindWindowEx(
					TriggerTabWnd,
					NULL,
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TriggerEventWnd.c_str()
				);
				HWND CurrEventHandle = GetDlgItem(EventWnd, 1167);
				HWND EventType = GetDlgItem(EventWnd, 1175);
				HWND EventList = GetDlgItem(EventWnd, 1401);
				HWND EventPara = GetDlgItem(EventWnd, 1402);
				int CurrEventCount = SendMessage(CurrEventHandle, CB_GETCOUNT, NULL, NULL);
				if (CurrEventCount > 19) {
					g_logger.Warn("Current trigger's event is full");
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.TriggerEventFull.c_str(),
						g_MessageBoxConfig.Captain.Warning.c_str(),
						MB_OK
					);
					break;
				}
				int CurIndex = SendMessage(CurrEventHandle, CB_GETCURSEL, NULL, NULL);
				int DesIndex;
				if (CurrEventCount <= 9)	DesIndex = CurrEventCount + 1;
				else
				{
					++CurIndex;
					DesIndex = CurrEventCount - 8;
				}
				TCHAR *CurType;
				int curLen = GetWindowTextLength(EventType) + 1;
				CurType = new TCHAR[curLen];
				GetWindowText(EventType, CurType, curLen);
				int CurParaCount = SendMessage(EventList, LB_GETCOUNT, NULL, NULL);
				std::vector<TCHAR*> CurPara(CurParaCount);
				for (register int i = 0; i < CurParaCount; ++i) {
					SendMessage(EventList, LB_SETCURSEL, i, NULL);
					SendMessage(EventWnd, WM_COMMAND, MAKEWPARAM(1401, LBN_SELCHANGE), (LPARAM)EventList);
					int strLen = GetWindowTextLength(EventPara) + 1;
					CurPara[i] = new TCHAR[strLen];
					GetWindowText(EventPara, CurPara[i], strLen);
				}

				HWND BtnAdd = GetDlgItem(EventWnd, 1396);
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1396, NULL);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1396, NULL);
				SendMessage(CurrEventHandle, CB_SETCURSEL, DesIndex, NULL);
				SendMessage(EventWnd, WM_COMMAND, MAKEWPARAM(1167, CBN_SELCHANGE), (LPARAM)CurrEventHandle);

				SetWindowText(EventType, CurType);
				delete[] CurType;
				SendMessage(EventWnd, WM_COMMAND, MAKEWPARAM(1175, CBN_EDITCHANGE), (LPARAM)EventType);

				for (register int i = 0; i < CurParaCount; ++i) {
					SendMessage(EventList, LB_SETCURSEL, i, NULL);
					SendMessage(EventWnd, WM_COMMAND, MAKEWPARAM(1401, LBN_SELCHANGE), (LPARAM)EventList);
					SetWindowText(EventPara, CurPara[i]);
					delete[] CurPara[i];
					SendMessage(EventWnd, WM_COMMAND, MAKEWPARAM(1402, CBN_EDITCHANGE), (LPARAM)EventPara);
				}

				break;
			}
			case 9991: {//Copy Action Member
				g_logger.Info("Copy Action Member");
				HWND TriggerTabWnd = GetDlgItem(
					FindWindow(
						g_FindWindowConfig.DialogClass.c_str(),
						g_FindWindowConfig.TriggerWnd.c_str()
					),
					1393);
				HWND ActionWnd = FindWindowEx(
					TriggerTabWnd,
					NULL,
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TriggerActionWnd.c_str()
				);
				HWND CurrActionHandle = GetDlgItem(ActionWnd, 1170);
				HWND ActionType = GetDlgItem(ActionWnd, 1178);
				HWND ActionList = GetDlgItem(ActionWnd, 1401);
				HWND ActionPara = GetDlgItem(ActionWnd, 1402);
				int CurrEventCount = SendMessage(CurrActionHandle, CB_GETCOUNT, NULL, NULL);
				if (CurrEventCount > 19) {
					g_logger.Warn("Current trigger's action is full");
					MessageBox(
						NULL,
						g_MessageBoxConfig.Message.TriggerActionFull.c_str(),
						g_MessageBoxConfig.Captain.Warning.c_str(),
						MB_OK
					);
					break;
				}
				int CurIndex = SendMessage(CurrActionHandle, CB_GETCURSEL, NULL, NULL);
				int DesIndex;
				if (CurrEventCount <= 9)	DesIndex = CurrEventCount + 1;
				else
				{
					++CurIndex;
					DesIndex = CurrEventCount - 8;
				}
				TCHAR *CurType;
				int curLen = GetWindowTextLength(ActionType) + 1;
				CurType = new TCHAR[curLen];
				GetWindowText(ActionType, CurType, curLen);
				int CurParaCount = SendMessage(ActionList, LB_GETCOUNT, NULL, NULL);
				std::vector<TCHAR*> CurPara(CurParaCount);
				for (register int i = 0; i < CurParaCount; ++i) {
					SendMessage(ActionList, LB_SETCURSEL, i, NULL);
					SendMessage(ActionWnd, WM_COMMAND, MAKEWPARAM(1401, LBN_SELCHANGE), (LPARAM)ActionList);
					int strLen = GetWindowTextLength(ActionPara) + 1;
					CurPara[i] = new TCHAR[strLen];
					GetWindowText(ActionPara, CurPara[i], strLen);
				}

				HWND BtnAdd = GetDlgItem(ActionWnd, 1403);
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1403, NULL);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1403, NULL);
				SendMessage(CurrActionHandle, CB_SETCURSEL, DesIndex, NULL);
				SendMessage(ActionWnd, WM_COMMAND, MAKEWPARAM(1170, CBN_SELCHANGE), (LPARAM)CurrActionHandle);

				SetWindowText(ActionType, CurType);
				delete[] CurType;
				SendMessage(ActionWnd, WM_COMMAND, MAKEWPARAM(1178, CBN_EDITCHANGE), (LPARAM)ActionType);

				for (register int i = 0; i < CurParaCount; ++i) {
					SendMessage(ActionList, LB_SETCURSEL, i, NULL);
					SendMessage(ActionWnd, WM_COMMAND, MAKEWPARAM(1401, LBN_SELCHANGE), (LPARAM)ActionList);
					SetWindowText(ActionPara, CurPara[i]);
					delete[] CurPara[i];
					SendMessage(ActionWnd, WM_COMMAND, MAKEWPARAM(1402, CBN_EDITCHANGE), (LPARAM)ActionPara);
				}
				break;
			}
			//Taskforces
			case 9985: {//Override Combobox
				switch (wmHi) {
				case CBN_SETFOCUS: {
					g_logger.Info("Taskforce SETFOCUS");
					HWND TaskforceWnd = FindWindow(
						g_FindWindowConfig.DialogClass.c_str(),
						g_FindWindowConfig.TaskforceWnd.c_str()
					);
					HWND NewType = GetDlgItem(TaskforceWnd, 9985);
					HWND OldType = GetDlgItem(TaskforceWnd, 1149);
					if (g_TaskforcesRead) {
						TCHAR *cur;
						int curLen = GetWindowTextLength(OldType) + 1;
						cur = new TCHAR[curLen];
						GetWindowText(OldType, cur, curLen);
						SetWindowText(NewType, cur);
						SendMessage(TaskforceWnd, WM_COMMAND, MAKEWPARAM(9985, CBN_SELCHANGE), (LPARAM)NewType);
						delete[] cur;
						break;
					}
					g_TaskforcesRead = TRUE;
					int Count = SendMessage(OldType, CB_GETCOUNT, NULL, NULL);
					g_logger.Info(std::to_string(Count) + " Unit/Infantry types read");
					if (Count <= 0)	break;
					int CurSel = SendMessage(OldType, CB_GETCURSEL, NULL, NULL);
					SendMessage(NewType, CB_RESETCONTENT, NULL, NULL);
					SendMessage(NewType, CB_SETCURSEL, -1, NULL);
					for (register int i = 0; i < Count; ++i) {
						TCHAR* cur;
						int curLen = 4 * GetWindowTextLength(OldType) + 1;
						cur = new TCHAR[curLen];
						SendMessage(OldType, CB_SETCURSEL, i, NULL);
						GetWindowText(OldType, cur, curLen);
						SendMessage(NewType, CB_ADDSTRING, NULL, (LPARAM)cur);

						delete[] cur;
					}
					SendMessage(OldType, CB_SETCURSEL, CurSel, NULL);

					break;
				}
				case CBN_EDITCHANGE:
				case CBN_EDITUPDATE:
				case CBN_KILLFOCUS:{
					g_logger.Info("Taskforce Update");
					HWND TaskforceWnd = FindWindow(
						g_FindWindowConfig.DialogClass.c_str(),
						g_FindWindowConfig.TaskforceWnd.c_str()
					);
					HWND NewType = GetDlgItem(TaskforceWnd, 9985);
					HWND OldType = GetDlgItem(TaskforceWnd, 1149);
					TCHAR *t_Type;
					int tLen = GetWindowTextLength(NewType) + 1;
					t_Type = new TCHAR[tLen];
					GetWindowText(NewType, t_Type, tLen);
					//g_logger.Info("Set current taskforce member to " + (std::string)t_Type);
					int Index = SendMessage(OldType, CB_FINDSTRING, NULL, (LPARAM)t_Type);
					delete[] t_Type;
					SendMessage(OldType, CB_SETCURSEL, Index, NULL);
					g_TaskforceComboFlag = TRUE;
					SendMessage(TaskforceWnd, WM_COMMAND, MAKEWPARAM(1149, CBN_EDITCHANGE), (LPARAM)OldType);
					break;
				}
				default:
					break;
				}				
				break;
			}
			case 9986: {
				g_logger.Info("Reload Taskforces Units");
				HWND TaskforceWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TaskforceWnd.c_str()
				);
				HWND NewType = GetDlgItem(TaskforceWnd, 9985);
				HWND OldType = GetDlgItem(TaskforceWnd, 1149);
				int Count = SendMessage(OldType, CB_GETCOUNT, NULL, NULL);
				g_logger.Info(std::to_string(Count) + " Unit types read");
				if (Count <= 0)	break;
				int CurSel = SendMessage(OldType, CB_GETCURSEL, NULL, NULL);
				SendMessage(NewType, CB_RESETCONTENT, NULL, NULL);
				SendMessage(NewType, CB_SETCURSEL, -1, NULL);
				for (register int i = 0; i < Count; ++i) {
					TCHAR *cur;
					int curLen = 4 * GetWindowTextLength(OldType) + 1;
					cur = new TCHAR[curLen];
					SendMessage(OldType, CB_SETCURSEL, i, NULL);
					GetWindowText(OldType, cur, curLen);

					SendMessage(NewType, CB_ADDSTRING, NULL, (LPARAM)cur);
					delete[] cur;
				}
				g_TaskforcesRead = TRUE;
				break;
			}
			case 9995: {//Copy Taskforce Member
				g_logger.Info("Copy Taskforce Member");
				HWND TaskforceWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TaskforceWnd.c_str()
				);
				HWND BtnAdd = GetDlgItem(TaskforceWnd, 1146);
				HWND EditNum = GetDlgItem(TaskforceWnd, 1148);
				HWND ComboType = GetDlgItem(TaskforceWnd, 1149);
				HWND ListBox = GetDlgItem(TaskforceWnd, 1145);
				TCHAR *CurNum, *CurType;
				int numLen = GetWindowTextLength(EditNum) + 1;
				int typeLen = GetWindowTextLength(ComboType) + 1;
				CurNum = new TCHAR[numLen];
				CurType = new TCHAR[typeLen];
				GetWindowText(ComboType, CurType, typeLen);
				GetWindowText(EditNum, CurNum, numLen);
				g_logger.Info("Currect (Type,Number) :(" + (std::string)CurType + ',' + (std::string)CurNum);
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1146, 0);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1146, 0);
				int Count = SendMessage(ListBox, LB_GETCOUNT, NULL, NULL);
				SendMessage(ListBox, LB_SETCURSEL, Count - 1, NULL);
				SendMessage(TaskforceWnd, WM_COMMAND, MAKEWPARAM(1145, LBN_SELCHANGE), (LPARAM)ListBox);
				SetWindowText(ComboType, CurType);
				SendMessage(TaskforceWnd, WM_COMMAND, MAKEWPARAM(1149, CBN_SELCHANGE), (LPARAM)ComboType);
				SetWindowText(EditNum, CurNum);
				delete[] CurNum;
				delete[] CurType;
				break;
			}
			case 9998: {//Copy Taskforce
				g_logger.Info("Copy Taskforce");
				HWND TaskforceWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TaskforceWnd.c_str()
				);
				TCHAR* TaskforceName, * TaskforceGroup;
				HWND EditName = GetDlgItem(TaskforceWnd, 1010);
				HWND EditGroup = GetDlgItem(TaskforceWnd, 1122);
				HWND EditNum = GetDlgItem(TaskforceWnd, 1148);
				HWND ComboType = GetDlgItem(TaskforceWnd, 1149);
				HWND ListBox = GetDlgItem(TaskforceWnd, 1145);

				int nameLen = GetWindowTextLength(EditName) + 1;
				int groupLen = GetWindowTextLength(EditGroup) + 1;
				TaskforceName = new TCHAR[nameLen];
				TaskforceGroup = new TCHAR[groupLen];

				GetWindowText(EditName, TaskforceName, nameLen);
				GetWindowText(EditGroup, TaskforceGroup, groupLen);
				int ListBoxCount = SendMessage(ListBox, LB_GETCOUNT, 0, 0);
				std::vector<TCHAR*> UnitType(ListBoxCount), UnitNum(ListBoxCount);
				for (register int i = 0; i < ListBoxCount; ++i) {
					SendMessage(ListBox, LB_SETCURSEL, i, NULL);
					SendMessage(TaskforceWnd, WM_COMMAND, MAKEWPARAM(1145, LBN_SELCHANGE), (LPARAM)ListBox);
					int typeLen = GetWindowTextLength(ComboType) + 1;
					int numLen = GetWindowTextLength(EditNum) + 1;
					UnitType[i] = new TCHAR[typeLen];
					UnitNum[i] = new TCHAR[numLen];
					GetWindowText(ComboType, UnitType[i], typeLen);
					GetWindowText(EditNum, UnitNum[i], numLen);
				}
				g_logger.Info(std::to_string(ListBoxCount) + " taskforce members saved");

				//New Taskforce
				HWND BtnNew = GetDlgItem(TaskforceWnd, 1151);
				SendMessage(BtnNew, WM_LBUTTONDOWN, 1151, 0);
				SendMessage(BtnNew, WM_LBUTTONUP, 1151, 0);
				//SetWindowText(EditName, strcat(TaskforceName, " Clone"));
				std::string NewTaskforceName = TaskforceName;
				NewTaskforceName += " Clone";
				SetWindowText(EditName, NewTaskforceName.c_str());
				SetWindowText(EditGroup, TaskforceGroup);
				HWND BtnAdd = GetDlgItem(TaskforceWnd, 1146);
				for (register int i = 0; i < ListBoxCount; ++i) {
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1146, 0);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1146, 0);
					SendMessage(ListBox, LB_SETCURSEL, i, NULL);
					SendMessage(TaskforceWnd, WM_COMMAND, MAKEWPARAM(1145, LBN_SELCHANGE), (LPARAM)ListBox);
					SetWindowText(ComboType, UnitType[i]);
					SendMessage(TaskforceWnd, WM_COMMAND, MAKEWPARAM(1149, CBN_SELCHANGE), (LPARAM)ComboType);
					SetWindowText(EditNum, UnitNum[i]);
					delete[] UnitType[i];
					delete[] UnitNum[i];
				}
				delete[] TaskforceGroup;
				delete[] TaskforceName;
				break;
			}
			//Scripts
			case 9976: {//Override Add Script for Templates
				g_logger.Info("Add Script");
				HWND ScriptWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.ScriptWnd.c_str()
				);
				HWND ComboScriptTemplate = GetDlgItem(ScriptWnd, 9978);
				HWND CheckBox = GetDlgItem(ScriptWnd, 9993);
				HWND EditName = GetDlgItem(ScriptWnd, 1010);
				HWND ListBox = GetDlgItem(ScriptWnd, 1170);
				HWND ComboType = GetDlgItem(ScriptWnd, 1064);
				HWND ComboPara = GetDlgItem(ScriptWnd, 1196);
				int curTemplateComboCount = SendMessage(ComboScriptTemplate, CB_GETCOUNT, NULL, NULL);
				if (curTemplateComboCount <= 0) {
					HWND BtnLoad = GetDlgItem(ScriptWnd, 9977);
					SendMessage(BtnLoad, WM_LBUTTONDOWN, 9977, NULL);
					SendMessage(BtnLoad, WM_LBUTTONUP, 9977, NULL);
				}
				int curTemplateIndex = SendMessage(ComboScriptTemplate, CB_GETCURSEL, NULL, NULL);
				ScriptTemplate& curTemplate = g_ScriptTemplates[curTemplateIndex];
				g_logger.Info("Now using Script Template "+curTemplate[0]->first);

				HWND AllScriptCombo = GetDlgItem(ScriptWnd, 1193);
				int ScriptCount = SendMessage(AllScriptCombo, CB_GETCOUNT, 0, 0);
				std::vector<TCHAR*> ScriptDictionary(ScriptCount);
				for (register int i = 0; i < ScriptCount; ++i) {
					int strLen = SendMessage(AllScriptCombo, CB_GETLBTEXTLEN, NULL, NULL) + 1;
					ScriptDictionary[i] = new TCHAR[strLen];
					SendMessage(AllScriptCombo, CB_GETLBTEXT, i, (LPARAM)ScriptDictionary[i]);
				}

				HWND BtnNew = GetDlgItem(ScriptWnd, 1154);
				SendMessage(BtnNew, WM_LBUTTONDOWN, 1154, 0);
				SendMessage(BtnNew, WM_LBUTTONUP, 1154, 0);

				register int i;
				for (i = 0; i < ScriptCount; ++i) {
					TCHAR str[256];
					SendMessage(AllScriptCombo, CB_GETLBTEXT, i, (LPARAM)str);
					if (strcmp(str, ScriptDictionary[i]) != 0)	break;
				}

				SendMessage(AllScriptCombo, CB_SETCURSEL, i, NULL);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1193, CBN_SELCHANGE), (LPARAM)AllScriptCombo);

				HWND BtnAdd = GetDlgItem(ScriptWnd, 1173);
				int KeyCount = curTemplate.Count();
				SetWindowText(EditName, curTemplate[0]->second.c_str());

				for (register int i = 1; i <= KeyCount; ++i) {
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, NULL);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1173, NULL);
					SendMessage(ListBox, LB_SETCURSEL, i - 1, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					SendMessage(ComboType, CB_SETCURSEL, atoi(curTemplate[i]->first.c_str()), NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1064, CBN_SELCHANGE), (LPARAM)ComboType);
					if (curTemplate[i]->second == "EMPTY")	continue;
					SetWindowText(ComboPara, curTemplate[i]->second.c_str());
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1196, CBN_SELCHANGE), (LPARAM)ComboPara);
				}

				for (auto x : ScriptDictionary)	delete[] x;

				break;
			}
			case 9977: {//Load ScriptTemplates
				g_logger.Info("Load Script Templates");
				LoadScriptTemplates();
				HWND ScriptWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.ScriptWnd.c_str()
				);
				HWND ComboScriptTemplate = GetDlgItem(ScriptWnd, 9978);
				SendMessage(ComboScriptTemplate, CB_RESETCONTENT, NULL, NULL);
				int ScriptTemplateCount = g_ScriptTemplates.size();
				for (register int i = 0; i < ScriptTemplateCount; ++i)
					SendMessage(ComboScriptTemplate, CB_ADDSTRING, NULL, (LPARAM)(g_ScriptTemplates[i][0]->first.c_str()));
				SendMessage(ComboScriptTemplate, CB_SETCURSEL, 0, NULL);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(9978, CBN_SELCHANGE), (LPARAM)ComboScriptTemplate);
				break;
			}
			case 9992: {//Add Script Member (Override 1173)
				g_logger.Info("Add Script Member");
				HWND ScriptWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.ScriptWnd.c_str()
				);
				HWND CheckBox = GetDlgItem(ScriptWnd, 9993);
				HWND BtnAdd = GetDlgItem(ScriptWnd, 1173);
				HWND ListBox = GetDlgItem(ScriptWnd, 1170);
				HWND ComboType = GetDlgItem(ScriptWnd, 1064);
				HWND ComboPara = GetDlgItem(ScriptWnd, 1196);
				int IsChecked = SendMessage(CheckBox, BM_GETCHECK, NULL, NULL);
				int ScriptCount = SendMessage(ListBox, LB_GETCOUNT, 0, 0);
				int CurSelIndex = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
				if (IsChecked != BST_CHECKED) {
					g_logger.Info("Script Member - Insert Mode OFF");
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, NULL);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1173, NULL);
					SendMessage(ListBox, LB_SETCURSEL, ScriptCount, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					break;
				}
				if (ScriptCount == 0 || IsChecked != BST_CHECKED) {
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, NULL);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1173, NULL);
					SendMessage(ListBox, LB_SETCURSEL, 0, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					break;
				}
				g_logger.Info("Script Member - Insert Mode ON");
				std::vector<int> CurType(ScriptCount - CurSelIndex + 1);
				std::vector<TCHAR*> CurPara(ScriptCount - CurSelIndex + 1);
				for (register int i = CurSelIndex; i < ScriptCount; ++i) {
					SendMessage(ListBox, LB_SETCURSEL, i, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					CurType[i - CurSelIndex] = SendMessage(ComboType, CB_GETCURSEL, NULL, NULL);
					int strLen = GetWindowTextLength(ComboPara) + 1;
					CurPara[i - CurSelIndex] = new TCHAR[strLen];
					GetWindowText(ComboPara, CurPara[i - CurSelIndex], strLen);
				}
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, NULL);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1173, NULL);
				++ScriptCount;
				for (register int i = CurSelIndex + 1; i < ScriptCount; ++i) {
					SendMessage(ListBox, LB_SETCURSEL, i, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					SendMessage(ComboType, CB_SETCURSEL, CurType[i - CurSelIndex - 1], NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1064, CBN_SELCHANGE), (LPARAM)ComboType);
					SetWindowText(ComboPara, CurPara[i - CurSelIndex - 1]);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1196, CBN_SELCHANGE), (LPARAM)ComboPara);
				}
				SendMessage(ListBox, LB_SETCURSEL, CurSelIndex, NULL);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);

				SendMessage(ComboType, CB_SETCURSEL, 0, NULL);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1064, CBN_SELCHANGE), (LPARAM)ComboType);
				SetWindowText(ComboPara, "0");
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1196, CBN_SELCHANGE), (LPARAM)ComboPara);
				for (auto x : CurPara)	delete[] x;
				break;
			}
			case 9996: {//Copy Script Member
				g_logger.Info("Copy Script Member");
				HWND ScriptWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.ScriptWnd.c_str()
				);
				HWND CheckBox = GetDlgItem(ScriptWnd, 9993);
				HWND BtnAdd = GetDlgItem(ScriptWnd, 1173);
				HWND ListBox = GetDlgItem(ScriptWnd, 1170);
				HWND ComboType = GetDlgItem(ScriptWnd, 1064);
				HWND ComboPara = GetDlgItem(ScriptWnd, 1196);
				int IsChecked = SendMessage(CheckBox, BM_GETCHECK, NULL, NULL);
				int ScriptCount = SendMessage(ListBox, LB_GETCOUNT, 0, 0);
				int CurSelIndex = SendMessage(ListBox, LB_GETCURSEL, 0, 0);
				if (ScriptCount == 0) {
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, NULL);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1173, NULL);
					SendMessage(ListBox, LB_SETCURSEL, 0, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					break;
				}
				if (IsChecked != BST_CHECKED) {
					g_logger.Info("Script Member - Insert Mode OFF");
					int t_Type = SendMessage(ComboType, CB_GETCURSEL, NULL, NULL);
					TCHAR t_Para[256];
					GetWindowText(ComboPara, t_Para, 256);
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, NULL);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1173, NULL);
					SendMessage(ListBox, LB_SETCURSEL, ScriptCount, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					SendMessage(ComboType, CB_SETCURSEL, t_Type, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1064, CBN_SELCHANGE), (LPARAM)ComboType);
					SetWindowText(ComboPara, t_Para);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1196, CBN_SELCHANGE), (LPARAM)ComboPara);
					break;
				}
				int CopyType = SendMessage(ComboType, CB_GETCURSEL, NULL, NULL);
				TCHAR CopyPara[256];
				GetWindowText(ComboPara, CopyPara, 256);
				g_logger.Info("Script Member - Insert Mode ON");
				std::vector<int> CurType(ScriptCount - CurSelIndex + 1);
				std::vector<TCHAR*> CurPara(ScriptCount - CurSelIndex + 1);
				for (register int i = CurSelIndex; i < ScriptCount; ++i) {
					SendMessage(ListBox, LB_SETCURSEL, i, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					CurType[i - CurSelIndex] = SendMessage(ComboType, CB_GETCURSEL, NULL, NULL);
					int strLen = GetWindowTextLength(ComboPara) + 1;
					CurPara[i - CurSelIndex] = new TCHAR[strLen];
					GetWindowText(ComboPara, CurPara[i - CurSelIndex], strLen);
				}
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, NULL);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1173, NULL);
				++ScriptCount;
				for (register int i = CurSelIndex + 1; i < ScriptCount; ++i) {
					SendMessage(ListBox, LB_SETCURSEL, i, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					SendMessage(ComboType, CB_SETCURSEL, CurType[i - CurSelIndex - 1], NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1064, CBN_SELCHANGE), (LPARAM)ComboType);
					SetWindowText(ComboPara, CurPara[i - CurSelIndex - 1]);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1196, CBN_SELCHANGE), (LPARAM)ComboPara);
				}
				SendMessage(ListBox, LB_SETCURSEL, CurSelIndex, NULL);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);

				SendMessage(ComboType, CB_SETCURSEL, CopyType, NULL);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1064, CBN_SELCHANGE), (LPARAM)ComboType);
				SetWindowText(ComboPara, CopyPara);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1196, CBN_SELCHANGE), (LPARAM)ComboPara);
				for (auto x : CurPara)	delete[] x;
				break;
			}
			case 9999: {//Copy Script
				g_logger.Info("Copy Script");
				HWND ScriptWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.ScriptWnd.c_str()
				);
				HWND EditName = GetDlgItem(ScriptWnd, 1010);
				HWND ListBox = GetDlgItem(ScriptWnd, 1170);
				HWND ComboType = GetDlgItem(ScriptWnd, 1064);
				HWND ComboPara = GetDlgItem(ScriptWnd, 1196);
				TCHAR *ScriptName;
				int nameLen = GetWindowTextLength(EditName) + 1;
				ScriptName = new TCHAR[nameLen];
				GetWindowText(EditName, ScriptName, nameLen);
				int ListBoxCount = SendMessage(ListBox, LB_GETCOUNT, 0, 0);
				std::vector<TCHAR*> ScriptPara(ListBoxCount);
				std::vector<int> ScriptType(ListBoxCount);
				for (register int i = 0; i < ListBoxCount; ++i) {
					SendMessage(ListBox, LB_SETCURSEL, i, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					ScriptType[i] = SendMessage(ComboType, CB_GETCURSEL, NULL, NULL);
					int strLen = GetWindowTextLength(ComboPara) + 1;
					ScriptPara[i] = new TCHAR[strLen];
					GetWindowText(ComboPara, ScriptPara[i], strLen);
				}

				//Remember All
				HWND AllScriptCombo = GetDlgItem(ScriptWnd, 1193);
				int ScriptCount = SendMessage(AllScriptCombo, CB_GETCOUNT, 0, 0);
				std::vector<TCHAR*> ScriptDictionary(ScriptCount);
				for (register int i = 0; i < ScriptCount; ++i) {
					int strLen = SendMessage(AllScriptCombo, CB_GETLBTEXTLEN, i, NULL) + 1;
					ScriptDictionary[i] = new TCHAR[strLen];
					SendMessage(AllScriptCombo, CB_GETLBTEXT, i, (LPARAM)ScriptDictionary[i]);
				}

				g_logger.Info(std::to_string(ScriptCount) + " Script Members Stored");

				//New Script
				HWND BtnNew = GetDlgItem(ScriptWnd, 1154);
				SendMessage(BtnNew, WM_LBUTTONDOWN, 1154, 0);
				SendMessage(BtnNew, WM_LBUTTONUP, 1154, 0);

				//Find The New Script
				register int i;
				for (i = 0; i < ScriptCount; ++i) {
					TCHAR* str;
					int strLen = SendMessage(AllScriptCombo, CB_GETLBTEXTLEN, i, NULL) + 1;
					str = new TCHAR[strLen];
					SendMessage(AllScriptCombo, CB_GETLBTEXT, i, (LPARAM)str);
					if (strcmp(str, ScriptDictionary[i]) != 0) {
						delete[] str;
						break;
					}
					delete[] str;
				}
				SendMessage(AllScriptCombo, CB_SETCURSEL, i, NULL);
				SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1193, CBN_SELCHANGE), (LPARAM)AllScriptCombo);

				//Do Copy Works
				//SetWindowText(EditName, strcat(ScriptName, " Clone"));
				std::string NewName = ScriptName;
				NewName += " Clone";
				SetWindowText(EditName, NewName.c_str());

				HWND BtnAdd = GetDlgItem(ScriptWnd, 1173);
				for (register int j = 0; j < ListBoxCount; ++j) {
					SendMessage(BtnAdd, WM_LBUTTONDOWN, 1173, 0);
					SendMessage(BtnAdd, WM_LBUTTONUP, 1173, 0);
					SendMessage(ListBox, LB_SETCURSEL, j, NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1170, LBN_SELCHANGE), (LPARAM)ListBox);
					SendMessage(ComboType, CB_SETCURSEL, ScriptType[j], NULL);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1064, CBN_SELCHANGE), (LPARAM)ComboType);
					SetWindowText(ComboPara, ScriptPara[j]);
					SendMessage(ScriptWnd, WM_COMMAND, MAKEWPARAM(1196, CBN_SELCHANGE), (LPARAM)ComboPara);
				}

				for (auto x : ScriptPara)	delete[] x;
				for (auto x : ScriptDictionary) delete[] x;
				delete[] ScriptName;

				break;
			}
			//Teams
			case 9979: {//Override New Team for Templates
				g_logger.Info("New Team");
				HWND TeamWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TeamWnd.c_str()
				);
				HWND ComboTeamTemplate = GetDlgItem(TeamWnd, 9980);
				HWND EditName = GetDlgItem(TeamWnd, 1010),
					EditPriority = GetDlgItem(TeamWnd, 1011),
					EditMax = GetDlgItem(TeamWnd, 1012);
				HWND ComboGroup = GetDlgItem(TeamWnd, 1122),
					ComboExpLevel = GetDlgItem(TeamWnd, 1143),
					ComboTechLevel = GetDlgItem(TeamWnd, 1103),
					ComboMindControl = GetDlgItem(TeamWnd, 1140);
				int CheckID[19] = { 1113,1114,1115,1116,1117,1128,1129,1130,1131,1132,1119,1120,1127,1133,1134,1137,1136,1138,1139 };
				HWND Check[19];
				for (register int i = 0; i < 19; ++i)	Check[i] = GetDlgItem(TeamWnd, CheckID[i]);
				int curTemplateComboCount = SendMessage(ComboTeamTemplate, CB_GETCOUNT, NULL, NULL);
				if (curTemplateComboCount <= 0) {
					HWND BtnLoad = GetDlgItem(TeamWnd, 9981);
					SendMessage(BtnLoad, WM_LBUTTONDOWN, 9981, NULL);
					SendMessage(BtnLoad, WM_LBUTTONUP, 9981, NULL);
				}
				int curTemplateIndex = SendMessage(ComboTeamTemplate, CB_GETCURSEL, NULL, NULL);
				TeamTemplate& curTemplate = g_TeamTemplates[curTemplateIndex];
				g_logger.Info("Now using Team Template " + *curTemplate[0]);
				HWND BtnNew = GetDlgItem(TeamWnd, 1110);
				SendMessage(BtnNew, WM_LBUTTONDOWN, 1151, 0);
				SendMessage(BtnNew, WM_LBUTTONUP, 1151, 0);

				SetWindowText(EditName, curTemplate[1]->c_str());
				SetWindowText(EditPriority, curTemplate[3]->c_str());
				SetWindowText(EditMax, curTemplate[4]->c_str());

				SetWindowText(ComboExpLevel, curTemplate[2]->c_str());
				SetWindowText(ComboGroup, curTemplate[5]->c_str());
				SetWindowText(ComboTechLevel, curTemplate[6]->c_str());
				SetWindowText(ComboMindControl, curTemplate[7]->c_str());
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1143, CBN_SETFOCUS), (LPARAM)ComboExpLevel);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1122, CBN_SETFOCUS), (LPARAM)ComboGroup);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1140, CBN_SETFOCUS), (LPARAM)ComboMindControl);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1103, CBN_SETFOCUS), (LPARAM)ComboTechLevel);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1143, CBN_KILLFOCUS), (LPARAM)ComboExpLevel);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1122, CBN_KILLFOCUS), (LPARAM)ComboGroup);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1140, CBN_KILLFOCUS), (LPARAM)ComboMindControl);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1103, CBN_KILLFOCUS), (LPARAM)ComboTechLevel);

				for (register int i = 0; i < 19; ++i) {
					SendMessage(Check[i], BM_SETCHECK, (*curTemplate[i + 8] == "1") ? BST_CHECKED : BST_UNCHECKED, 0);
					SendMessage(Check[i], WM_LBUTTONDOWN, CheckID[i], NULL);
					SendMessage(Check[i], WM_LBUTTONUP, CheckID[i], NULL);
					SendMessage(Check[i], WM_LBUTTONDOWN, CheckID[i], NULL);
					SendMessage(Check[i], WM_LBUTTONUP, CheckID[i], NULL);
				}
				break;
			}
			case 9981: {//Load Templates
				g_logger.Info("Load Team Templates");
				LoadTeamTemplates();
				HWND TeamWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TeamWnd.c_str()
				);
				HWND ComboTeamTemplate = GetDlgItem(TeamWnd, 9980);
				SendMessage(ComboTeamTemplate, CB_RESETCONTENT, NULL, NULL);
				int TeamTemplateCount = g_TeamTemplates.size();
				for (register int i = 0; i < TeamTemplateCount; ++i)
					SendMessage(ComboTeamTemplate, CB_ADDSTRING, NULL, (LPARAM)(g_TeamTemplates[i][0]->c_str()));
				SendMessage(ComboTeamTemplate, CB_SETCURSEL, 0, NULL);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(9980, CBN_SELCHANGE), (LPARAM)ComboTeamTemplate);
				break;
			}
			case 9997: {//Copy Team
				g_logger.Info("Copy Team");
				HWND TeamWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.TeamWnd.c_str()
				);
				HWND EditName = GetDlgItem(TeamWnd, 1010),
					EditPriority = GetDlgItem(TeamWnd, 1011),
					EditMax = GetDlgItem(TeamWnd, 1012);
				HWND ComboGroup = GetDlgItem(TeamWnd, 1122),
					ComboExpLevel = GetDlgItem(TeamWnd, 1143),
					ComboWaypoint = GetDlgItem(TeamWnd, 1123),
					ComboTransportWaypoint = GetDlgItem(TeamWnd, 1126),
					ComboTechLevel = GetDlgItem(TeamWnd, 1103),
					ComboMindControl = GetDlgItem(TeamWnd, 1140),
					ComboHouse = GetDlgItem(TeamWnd, 1079),
					ComboScript = GetDlgItem(TeamWnd, 1124),
					ComboTaskforce = GetDlgItem(TeamWnd, 1125),
					ComboTag = GetDlgItem(TeamWnd, 1083);
				int CheckID[20] = { 1113,1114,1115,1116,1117,1128,1129,1130,1131,1132,1119,1120,1127,1133,1134,1135,1137,1136,1138,1139 };
				HWND Check[20];
				for (register int i = 0; i < 20; ++i)	Check[i] = GetDlgItem(TeamWnd, CheckID[i]);

				//Save Current Team
				TCHAR* CurrentTeamTextData[13];
				int strLen;
				strLen = GetWindowTextLength(EditName) + 1;
				CurrentTeamTextData[0] = new TCHAR[strLen];
				GetWindowText(EditName, CurrentTeamTextData[0], strLen);
				strLen = GetWindowTextLength(ComboExpLevel) + 1;
				CurrentTeamTextData[1] = new TCHAR[strLen];
				GetWindowText(ComboExpLevel, CurrentTeamTextData[1], 256);
				strLen = GetWindowTextLength(ComboHouse) + 1;
				CurrentTeamTextData[2] = new TCHAR[strLen];
				GetWindowText(ComboHouse, CurrentTeamTextData[2], 256);
				strLen = GetWindowTextLength(EditPriority) + 1;
				CurrentTeamTextData[3] = new TCHAR[strLen];
				GetWindowText(EditPriority, CurrentTeamTextData[3], 256);
				strLen = GetWindowTextLength(EditMax) + 1;
				CurrentTeamTextData[4] = new TCHAR[strLen];
				GetWindowText(EditMax, CurrentTeamTextData[4], 256);
				strLen = GetWindowTextLength(ComboGroup) + 1;
				CurrentTeamTextData[5] = new TCHAR[strLen];
				GetWindowText(ComboGroup, CurrentTeamTextData[5], 256);
				strLen = GetWindowTextLength(ComboWaypoint) + 1;
				CurrentTeamTextData[6] = new TCHAR[strLen];
				GetWindowText(ComboWaypoint, CurrentTeamTextData[6], 256);
				strLen = GetWindowTextLength(ComboTransportWaypoint) + 1;
				CurrentTeamTextData[7] = new TCHAR[strLen];
				GetWindowText(ComboTransportWaypoint, CurrentTeamTextData[7], 256);
				strLen = GetWindowTextLength(ComboTechLevel) + 1;
				CurrentTeamTextData[8] = new TCHAR[strLen];
				GetWindowText(ComboTechLevel, CurrentTeamTextData[8], 256);
				strLen = GetWindowTextLength(ComboMindControl) + 1;
				CurrentTeamTextData[9] = new TCHAR[strLen];
				GetWindowText(ComboMindControl, CurrentTeamTextData[9], 256);
				strLen = GetWindowTextLength(ComboScript) + 1;
				CurrentTeamTextData[10] = new TCHAR[strLen];
				GetWindowText(ComboScript, CurrentTeamTextData[10], 256);
				strLen = GetWindowTextLength(ComboTaskforce) + 1;
				CurrentTeamTextData[11] = new TCHAR[strLen];
				GetWindowText(ComboTaskforce, CurrentTeamTextData[11], 256);
				strLen = GetWindowTextLength(ComboTag) + 1;
				CurrentTeamTextData[12] = new TCHAR[strLen];
				GetWindowText(ComboTag, CurrentTeamTextData[12], 256);
				int IsChecked[20];
				for (register int i = 0; i < 20; ++i)	IsChecked[i] = SendMessage(Check[i], BM_GETCHECK, 0, 0);

				//New Team
				HWND BtnNew = GetDlgItem(TeamWnd, 1110);
				SendMessage(BtnNew, WM_LBUTTONDOWN, 1151, 0);
				SendMessage(BtnNew, WM_LBUTTONUP, 1151, 0);
				//SetWindowText(EditName, strcat(CurrentTeamTextData[0], " Clone"));
				std::string NewName = CurrentTeamTextData[0];
				NewName += " Clone";
				SetWindowText(EditName, NewName.c_str());
				SetWindowText(ComboExpLevel, CurrentTeamTextData[1]);
				SetWindowText(ComboHouse, CurrentTeamTextData[2]);
				SetWindowText(EditPriority, CurrentTeamTextData[3]);
				SetWindowText(EditMax, CurrentTeamTextData[4]);
				SetWindowText(ComboGroup, CurrentTeamTextData[5]);
				SetWindowText(ComboWaypoint, CurrentTeamTextData[6]);
				SetWindowText(ComboTransportWaypoint, CurrentTeamTextData[7]);
				SetWindowText(ComboTechLevel, CurrentTeamTextData[8]);
				SetWindowText(ComboMindControl, CurrentTeamTextData[9]);
				SetWindowText(ComboScript, CurrentTeamTextData[10]);
				SetWindowText(ComboTaskforce, CurrentTeamTextData[11]);
				SetWindowText(ComboTag, CurrentTeamTextData[12]);

				//Save ComboBox Changes
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1143, CBN_SETFOCUS), (LPARAM)ComboExpLevel);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1079, CBN_SETFOCUS), (LPARAM)ComboHouse);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1122, CBN_SETFOCUS), (LPARAM)ComboGroup);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1123, CBN_SETFOCUS), (LPARAM)ComboWaypoint);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1126, CBN_SETFOCUS), (LPARAM)ComboWaypoint);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1103, CBN_SETFOCUS), (LPARAM)ComboTechLevel);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1140, CBN_SETFOCUS), (LPARAM)ComboMindControl);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1124, CBN_SETFOCUS), (LPARAM)ComboScript);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1125, CBN_SETFOCUS), (LPARAM)ComboTaskforce);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1083, CBN_SETFOCUS), (LPARAM)ComboTag);

				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1143, CBN_KILLFOCUS), (LPARAM)ComboExpLevel);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1079, CBN_KILLFOCUS), (LPARAM)ComboHouse);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1122, CBN_KILLFOCUS), (LPARAM)ComboGroup);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1123, CBN_KILLFOCUS), (LPARAM)ComboWaypoint);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1126, CBN_KILLFOCUS), (LPARAM)ComboWaypoint);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1103, CBN_KILLFOCUS), (LPARAM)ComboTechLevel);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1140, CBN_KILLFOCUS), (LPARAM)ComboMindControl);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1124, CBN_KILLFOCUS), (LPARAM)ComboScript);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1125, CBN_KILLFOCUS), (LPARAM)ComboTaskforce);
				SendMessage(TeamWnd, WM_COMMAND, MAKEWPARAM(1083, CBN_KILLFOCUS), (LPARAM)ComboTag);

				for (register int i = 0; i < 20; ++i) {
					SendMessage(Check[i], BM_SETCHECK, IsChecked[i], 0);
					//Save Checkbox Changes
					SendMessage(Check[i], WM_LBUTTONDOWN, CheckID[i], NULL);
					SendMessage(Check[i], WM_LBUTTONUP, CheckID[i], NULL);
					SendMessage(Check[i], WM_LBUTTONDOWN, CheckID[i], NULL);
					SendMessage(Check[i], WM_LBUTTONUP, CheckID[i], NULL);
				}

				for (register int i = 0; i < 13; ++i)	delete[] CurrentTeamTextData[i];

				break;
			}
			//AI Triggers
			case 9994: {
				g_logger.Info("Copy AI Trigger");
				//Initialize Handles
				HWND AITriggerWnd = FindWindow(
					g_FindWindowConfig.DialogClass.c_str(),
					g_FindWindowConfig.AITriggerWnd.c_str()
				);
				HWND BtnAdd = GetDlgItem(AITriggerWnd, 1154);
				int CheckID[6] = { 1218,1424,1425,1426,1452,1453 };
				HWND CheckBox[6];
				for (register int i = 0; i < 6; ++i)
					CheckBox[i] = GetDlgItem(AITriggerWnd, CheckID[i]);
				int ComboID[7] = { 1163,1205,1204,1206,1456,1149,1449 };
				HWND ComboBox[7];
				for (register int i = 0; i < 7; ++i)
					ComboBox[i] = GetDlgItem(AITriggerWnd, ComboID[i]);
				int EditID[5] = { 1010,1213,1215,1216,1450 };
				HWND Edit[5];
				for (register int i = 0; i < 5; ++i)
					Edit[i] = GetDlgItem(AITriggerWnd, EditID[i]);

				//Memorize Things
				int CurCheck[6];
				for (register int i = 0; i < 6; ++i)
					CurCheck[i] = SendMessage(CheckBox[i], BM_GETCHECK, 0, 0);
				TCHAR* CurEdit[5];
				for (register int i = 0; i < 5; ++i) {
					int strLen = GetWindowTextLength(Edit[i]) + 1;
					CurEdit[i] = new TCHAR[strLen];
					GetWindowText(Edit[i], CurEdit[i], strLen);
				}
					
				int CurTriggerType = SendMessage(ComboBox[0], CB_GETCURSEL, 0, 0);
				int CurTriggerEvent = SendMessage(ComboBox[6], CB_GETCURSEL, 0, 0);
				int CurTriggerSide = SendMessage(ComboBox[4], CB_GETCURSEL, 0, 0);
				TCHAR *CurComboBox[5];
				for (register int i = 0; i < 5; ++i) {
					int strLen = GetWindowTextLength(ComboBox[i + 1]) + 1;
					CurComboBox[i] = new TCHAR[strLen];
					GetWindowText(ComboBox[i + 1], CurComboBox[i], strLen);
				}
					

				//New AI Trigger
				SendMessage(BtnAdd, WM_LBUTTONDOWN, 1154, NULL);
				SendMessage(BtnAdd, WM_LBUTTONUP, 1154, NULL);

				for (register int i = 0; i < 6; ++i) {
					SendMessage(CheckBox[i], BM_SETCHECK, CurCheck[i], 0);
					SendMessage(CheckBox[i], WM_LBUTTONDOWN, CheckID[i], NULL);
					SendMessage(CheckBox[i], WM_LBUTTONUP, CheckID[i], NULL);
					SendMessage(CheckBox[i], WM_LBUTTONDOWN, CheckID[i], NULL);
					SendMessage(CheckBox[i], WM_LBUTTONUP, CheckID[i], NULL);
				}


				//SetWindowText(Edit[0], strcat(CurEdit[0], " Clone"));
				std::string NewName = CurEdit[0];
				NewName += " Clone";
				SetWindowText(Edit[0], NewName.c_str());
				for (register int i = 1; i < 5; ++i)
					SetWindowText(Edit[i], CurEdit[i]);

				SendMessage(ComboBox[0], CB_SETCURSEL, CurTriggerType, 0);
				SendMessage(ComboBox[6], CB_SETCURSEL, CurTriggerEvent, 0);
				SendMessage(ComboBox[4], CB_SETCURSEL, CurTriggerSide, 0);
				for (register int i = 0; i < 5; ++i)
					SetWindowText(ComboBox[i + 1], CurComboBox[i]);

				for (register int i = 0; i < 7; ++i) {
					SendMessage(AITriggerWnd, WM_COMMAND, MAKEWPARAM(ComboID[i], CBN_SELCHANGE), (LPARAM)ComboBox[i]);
				}

				SendMessage(ComboBox[4], CB_SETCURSEL, CurTriggerSide, 0);
				SendMessage(AITriggerWnd, WM_COMMAND, MAKEWPARAM(ComboID[4], CBN_SELCHANGE), (LPARAM)ComboBox[4]);

				for (register int i = 0; i < 5; ++i) {
					delete[] CurEdit[i];
					delete[] CurComboBox[i];
				}
				break;
			}
			}
		}

	}
	return CallNextHookEx(g_CallWndHook, nCode, wParam, lParam);
}
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (!g_GetMsgHooked) {
		g_GetMsgHooked = TRUE;

		// For global initialization
		g_FA2Wnd = GetWindowHandle();
		g_Path = GetPath();
		//GetTreeViewHwnd();
		LoadINI();
		LoadFA2CopyConfig();
		
		g_CTRL_S = GlobalAddAtom("Ctrl+S");
		g_CTRL_O = GlobalAddAtom("Ctrl+O");
		g_CTRL_N = GlobalAddAtom("Ctrl+N");
		RegisterHotKey(g_FA2Wnd, g_CTRL_S, MOD_CONTROL, 0x53);
		RegisterHotKey(g_FA2Wnd, g_CTRL_O, MOD_CONTROL, 0x4f);
		RegisterHotKey(g_FA2Wnd, g_CTRL_N, MOD_CONTROL, 0x4e);
		int result = GetLastError();

		g_oldProc = (WNDPROC)SetWindowLong(g_FA2Wnd, GWL_WNDPROC, (LONG)HotkeyWndProc);
	}
	MSG curMsg = *(MSG*)lParam;
	switch (curMsg.message) {
	case WM_SETFOCUS:
		if ((HWND)(curMsg.wParam) == g_FA2Wnd)
			g_AllowHotKey = TRUE;
		break;
	case WM_KILLFOCUS:
		if ((HWND)(curMsg.wParam) == g_FA2Wnd)
			g_AllowHotKey = FALSE;
		break;
	default:
		break;
	}
	return CallNextHookEx(g_GetMsgHook, nCode, wParam, lParam);
}

// Replace for Hotkeys
LRESULT CALLBACK HotkeyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_HOTKEY && g_AllowHotKey == TRUE)
		if (wParam == g_CTRL_S) {
			g_logger.Info("Ctrl+S Hotkey pressed");
			HWND SaveWnd = FindWindow(
				g_FindWindowConfig.DialogClass.c_str(),
				g_FindWindowConfig.SaveWnd.c_str()
			);
			if(SaveWnd==NULL)
				SendMessage(g_FA2Wnd, WM_COMMAND, MAKEWPARAM(57603, BN_CLICKED), NULL);
		}
		else if (wParam == g_CTRL_O) {
			g_logger.Info("Ctrl+O Hotkey pressed");
			HWND LoadWnd = FindWindow(
				g_FindWindowConfig.DialogClass.c_str(),
				g_FindWindowConfig.LoadWnd.c_str()
			);
			if(LoadWnd==NULL)
				SendMessage(g_FA2Wnd, WM_COMMAND, MAKEWPARAM(40001, BN_CLICKED), NULL);
		}
		else if (wParam == g_CTRL_N) {
			g_logger.Info("Ctrl+N Hotkey pressed");
			HWND NewWnd1 = FindWindow(
				g_FindWindowConfig.DialogClass.c_str(),
				g_FindWindowConfig.NewWnd1.c_str()
			);
			HWND NewWnd2 = FindWindow(
				g_FindWindowConfig.DialogClass.c_str(),
				g_FindWindowConfig.NewWnd2.c_str()
			);
			HWND NewWnd3 = FindWindow(
				g_FindWindowConfig.DialogClass.c_str(),
				g_FindWindowConfig.NewWnd3.c_str()
			);
			if (NewWnd1 == NULL && NewWnd2 == NULL && NewWnd3 == NULL)
				SendMessage(g_FA2Wnd, WM_COMMAND, MAKEWPARAM(57600, BN_CLICKED), NULL);
		}
	
	return g_oldProc(hWnd, uMsg, wParam, lParam);
}

// Get Window
HWND GetWindowHandle()
{
	DWORD dwCurrentProcessId = GetCurrentProcessId();

	if (!EnumWindows(EnumWindowsProc, (LPARAM)&dwCurrentProcessId))
	{
		return (HWND)dwCurrentProcessId;
	}

	return NULL;
}
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwCurProcessId = *((DWORD*)lParam);
	DWORD dwProcessId = 0;

	GetWindowThreadProcessId(hwnd, &dwProcessId);
	if (dwProcessId == dwCurProcessId && GetParent(hwnd) == NULL)
	{
		*((HWND*)lParam) = hwnd;
		return FALSE;
	}

	return TRUE;
}
BOOL CALLBACK EnumChildWindowsProc(HWND hwnd, LPARAM lParam) {
	HWND Result = FindWindowEx(hwnd, NULL,
		g_FindWindowConfig.DialogClass.c_str(), g_FindWindowConfig.TerrainWnd.c_str());
	if (Result != NULL)	g_TerrainWnd = Result;
	return 1;
}

// Dialog Proc
BOOL CALLBACK HouseDlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
		case WM_INITDIALOG:{
			std::unordered_map<std::string, bool>* Houses = 
				((std::pair<std::unordered_map<std::string, bool>*, std::pair<std::string*, std::string*>*>*)lParam)->first;
			std::pair<std::string*, std::string*>* StringParam =
				((std::pair<std::unordered_map<std::string, bool>*, std::pair<std::string*, std::string*>*>*)lParam)->second;
			//I think using a pointer to pass information is much funnier than simply using a global variable XD
			HWND LBA, LBB, EDIT, EDIT2;
			LBA = GetDlgItem(hwnd, IDC_LIST2);//Allies ListBox
			LBB = GetDlgItem(hwnd, IDC_LIST3);//Enemy ListBox
			EDIT = GetDlgItem(hwnd, IDC_EDIT1);//Edit
			EDIT2 = GetDlgItem(hwnd, IDC_EDIT2);//String Address
			for (auto x : (*Houses)) {
				std::string str = x.first;
				if (x.second)
					SendMessage(LBA, LB_ADDSTRING, NULL, (LPARAM)str.c_str());
				else
					SendMessage(LBB, LB_ADDSTRING, NULL, (LPARAM)str.c_str());
			}

			SetWindowText(EDIT2, std::to_string((int)(StringParam->first)).c_str());
			std::string &EditShowStr = *(StringParam->second);
			g_logger.Info("Now processing house " + EditShowStr);
			SetWindowText(EDIT, EditShowStr.c_str());
			g_logger.Info("Successfully done initialization of allie editor dialog box.");
			return TRUE;
		}
		case WM_COMMAND:{
			switch (wParam)
			{
				case IDOK:{
					g_logger.Info("Allie editor confirmed.");
					HWND LBA = GetDlgItem(hwnd, IDC_LIST2);//Allies ListBox
					HWND EDIT = GetDlgItem(hwnd, IDC_EDIT1);
					HWND EDIT2 = GetDlgItem(hwnd, IDC_EDIT2);
					TCHAR str[256];
					GetWindowText(EDIT2, str, 11);
					std::string* retStr = (std::string*)(atoi(str));
					retStr->clear();
					int AllieCount = SendMessage(LBA, LB_GETCOUNT, NULL, NULL);
					for (register int i = 0; i < AllieCount; ++i) {
						int TextLen = SendMessage(LBA, LB_GETTEXTLEN, i, NULL);
						if (TextLen == LB_ERR)	break;
						TCHAR* _str = new TCHAR[TextLen + 1];
						SendMessage(LBA, LB_GETTEXT, i, (LPARAM)_str);
						(*retStr) += ((std::string)_str + ',');
					}
					GetWindowText(EDIT, str, 256);
					(*retStr) += (std::string)str;
					EndDialog(hwnd, NULL);
					return TRUE;
				}
				case IDCANCEL: {
					g_logger.Info("Allie editor cancelled.");
					EndDialog(hwnd, NULL);
					return TRUE;
				}
				case IDC_BUTTON1: {//Go Allies
					HWND LBA = GetDlgItem(hwnd, IDC_LIST2);//Allies ListBox
					HWND LBB = GetDlgItem(hwnd, IDC_LIST3);//Enemy ListBox
					int EnemyCount = SendMessage(LBB, LB_GETCOUNT, NULL, NULL);
					if (EnemyCount <= 0)	break;
					int EnemyCurSelIndex = SendMessage(LBB, LB_GETCURSEL, NULL, NULL);
					if (EnemyCurSelIndex < 0 || EnemyCurSelIndex >= EnemyCount)	break;
					int TextLen = SendMessage(LBB, LB_GETTEXTLEN, EnemyCurSelIndex, NULL);
					if (TextLen == LB_ERR)	break;
					TCHAR* str = new TCHAR[TextLen + 1];
					SendMessage(LBB, LB_GETTEXT, EnemyCurSelIndex, (LPARAM)str);
					SendMessage(LBB, LB_DELETESTRING, EnemyCurSelIndex, NULL);
					SendMessage(LBA, LB_ADDSTRING, NULL, (LPARAM)str);
					delete[] str;
					break;
				}
				case IDC_BUTTON2: {//Go Enemies
					HWND LBA = GetDlgItem(hwnd, IDC_LIST2);//Allies ListBox
					HWND LBB = GetDlgItem(hwnd, IDC_LIST3);//Enemy ListBox
					int AllieCount = SendMessage(LBA, LB_GETCOUNT, NULL, NULL);
					if (AllieCount <= 0)	break;
					int AllieCurSelIndex = SendMessage(LBA, LB_GETCURSEL, NULL, NULL);
					if (AllieCurSelIndex < 0 || AllieCurSelIndex >= AllieCount)	break;
					int TextLen = SendMessage(LBA, LB_GETTEXTLEN, AllieCurSelIndex, NULL);
					if (TextLen == LB_ERR)	break;
					TCHAR* str = new TCHAR[TextLen + 1];
					SendMessage(LBA, LB_GETTEXT, AllieCurSelIndex, (LPARAM)str);
					SendMessage(LBA, LB_DELETESTRING, AllieCurSelIndex, NULL);
					SendMessage(LBB, LB_ADDSTRING, NULL, (LPARAM)str);
					delete[] str;
					break;
				}
				default:
					break;
			}
			break;
		}
		case WM_CLOSE:{
			EndDialog(hwnd, NULL);
			return TRUE;
		}
	}
	return FALSE;
}

// Functionality Functions
std::string GetPath() {
	TCHAR* path = NULL;
	path = _getcwd(NULL, 0);
	std::string ret;
	if (path != NULL)
		ret = path;
	free(path);
	g_logger.Custom("", "Dll Path :" + ret, false);
	return ret;
}

// Templates
void LoadTerrainGroups(std::string Theater) {
	using std::string;
	string::iterator itr = Theater.begin();	++itr;
	transform(itr, Theater.end(), itr, tolower);
	string IniSectionName = "Terrain" + Theater;
	g_TeamTemplates.clear();
	Ini& ini = g_ini;
	if (!ini.Exist()) {
		MessageBox(
			NULL,
			g_MessageBoxConfig.Message.IniNotExist.c_str(),
			g_MessageBoxConfig.Captain.Error.c_str(),
			MB_OK
		);
		return;
	}
	string Enable = ini.Read(IniSectionName, "Enable");
	if (Enable != "yes") {
		MessageBox(
			NULL,
			g_MessageBoxConfig.Message.TerrainDisabled.c_str(),
			g_MessageBoxConfig.Captain.Hint.c_str(),
			MB_OK
		);
		return;
	}
	int Count = atoi(ini.Read(IniSectionName, "Counts").c_str());
	if (Count <= 0)	return;
	g_logger.Info(std::to_string(Count) + " Terrain Groups Loading");
	g_TerrainSorts.resize(Count);
	for (register int i = 0; i < Count; ++i) {
		string str = ini.Read(IniSectionName, std::to_string(i+1));
		TerrainSort terrainsort = ini.Split(str, ',');
		g_TerrainSorts[i] = terrainsort;
	}

	HWND TerrainWnd1 = FindWindowEx(g_FA2Wnd, NULL, "AfxFrameOrView42s", NULL);
	HWND TerrainWnd2 = FindWindowEx(TerrainWnd1, NULL, "AfxMDIFrame42s", NULL);
	EnumChildWindows(TerrainWnd2, EnumChildWindowsProc, NULL);
	
	HWND& TerrainWnd = g_TerrainWnd;
	HWND ComboMain = GetDlgItem(TerrainWnd, 9983);

	SendMessage(ComboMain, CB_RESETCONTENT, NULL, NULL);
	for (register int i = 0; i < Count; ++i)
		SendMessage(ComboMain, CB_ADDSTRING, NULL, (LPARAM)(g_TerrainSorts[i].GetName().c_str()));

	SendMessage(ComboMain, CB_SETCURSEL, 0, NULL);
	SendMessage(TerrainWnd, WM_COMMAND, MAKEWPARAM(9983, CBN_SELCHANGE), (LPARAM)ComboMain);
	return;
}
void LoadTeamTemplates() {
	Ini& ini = g_ini;
	g_TeamTemplates.clear();

	//Read Team Templates
	int TeamTemplatesCount = atoi(ini.Read("TeamTemplates", "Counts").c_str());
	if (ini.Exist() == FALSE) {
		MessageBox(
			NULL,
			g_MessageBoxConfig.Message.IniNotExist.c_str(),
			g_MessageBoxConfig.Captain.Error.c_str(),
			MB_OK
		);
		TeamTemplatesCount = 0;
	}
	if (TeamTemplatesCount < 0)	TeamTemplatesCount = 0;

	g_logger.Info(std::to_string(TeamTemplatesCount) + " Team Templates Loading");

	g_TeamTemplates.resize(TeamTemplatesCount + 1);
	*g_TeamTemplates[0][0] = ini.Read("TeamTemplates", "DefaultName");
	*g_TeamTemplates[0][1] = "New teamtype";
	*g_TeamTemplates[0][2] = "1";
	*g_TeamTemplates[0][3] = "5";
	*g_TeamTemplates[0][4] = "5";
	*g_TeamTemplates[0][5] = "-1";

	for (register int i = 1; i <= TeamTemplatesCount; ++i) {
		std::string curstr = ini.Read("TeamTemplates", std::to_string(i));
		TeamTemplate teamTemplate = ini.Split(curstr, ',');
		g_TeamTemplates[i] = teamTemplate;
	}

	return;
}
void LoadScriptTemplates() {
	Ini& ini = g_ini;
	g_ScriptTemplates.clear();

	//Read Team Templates
	int ScriptTemplatesCount = atoi(ini.Read("ScriptTemplates", "Counts").c_str());
	if (ini.Exist() == FALSE) {
		MessageBox(
			NULL,
			g_MessageBoxConfig.Message.IniNotExist.c_str(),
			g_MessageBoxConfig.Captain.Error.c_str(),
			MB_OK
		);
		ScriptTemplatesCount = 0;
	}
	if (ScriptTemplatesCount < 0)	ScriptTemplatesCount = 0;

	g_logger.Info(std::to_string(ScriptTemplatesCount) + " Script Templates Loading");

	g_ScriptTemplates.resize(ScriptTemplatesCount + 1);
	g_ScriptTemplates[0].Resize(1);
	g_ScriptTemplates[0][0]->first = ini.Read("ScriptTemplates", "DefaultName");
	g_ScriptTemplates[0][0]->second = "New script";

	for (register int i = 1; i <= ScriptTemplatesCount; ++i) {
		std::string curstr = ini.Read("ScriptTemplates", std::to_string(i));
		ScriptTemplate scriptTemplate = ini.Split(curstr, ',');
		g_ScriptTemplates[i] = scriptTemplate;
	}

	return;
}

void GetTreeViewHwnd() {
	g_logger.Info("Tree View Handle has gotten");
	HWND Hwnd1 = FindWindowEx(g_FA2Wnd, NULL, "AfxFrameOrView42s", NULL);
	HWND Hwnd2 = FindWindowEx(Hwnd1, NULL, "AfxMDIFrame42s", NULL);
	g_SysTreeView = FindWindowEx(Hwnd2, NULL, "SysTreeView32", NULL);
	return;
}
void LoadINI() {
	g_logger.Info("INI is loading...");
	std::string FA2CopyDataPath = g_Path;
	FA2CopyDataPath += "\\FA2CopyData.ini";
	g_ini = FA2CopyDataPath;
	if (!g_ini.Exist()) {
		MessageBox(
			NULL,
			g_MessageBoxConfig.Message.IniNotExist.c_str(),
			g_MessageBoxConfig.Captain.Error.c_str(),
			MB_OK
		);
		SendMessage(g_FA2Wnd, WM_CLOSE, NULL, NULL);
	}
}
void LoadFA2CopyConfig() {
	g_logger.Info("FA2Copy Config Loaded");

	//g_FindWindowConfig
	g_FindWindowConfig.AITriggerWnd = g_ini.Read("FindWindowConfig", "AITriggerWnd");
	g_FindWindowConfig.DialogClass = g_ini.Read("FindWindowConfig", "DialogClass");
	g_FindWindowConfig.MapWnd = g_ini.Read("FindWindowConfig", "MapWnd");
	g_FindWindowConfig.IniWnd = g_ini.Read("FindWindowConfig", "IniWnd");
	g_FindWindowConfig.HouseWnd = g_ini.Read("FindWindowConfig", "HouseWnd");
	g_FindWindowConfig.TriggerWnd = g_ini.Read("FindWindowConfig", "TriggerWnd");
	g_FindWindowConfig.TagWnd = g_ini.Read("FindWindowConfig", "TagWnd");
	g_FindWindowConfig.TriggerGlobalWnd = g_ini.Read("FindWindowConfig", "TriggerGlobalWnd");
	g_FindWindowConfig.TriggerEventWnd = g_ini.Read("FindWindowConfig", "TriggerEventWnd");
	g_FindWindowConfig.TriggerActionWnd = g_ini.Read("FindWindowConfig", "TriggerActionWnd");
	g_FindWindowConfig.TaskforceWnd = g_ini.Read("FindWindowConfig", "TaskforceWnd");
	g_FindWindowConfig.ScriptWnd = g_ini.Read("FindWindowConfig", "ScriptWnd");
	g_FindWindowConfig.TeamWnd = g_ini.Read("FindWindowConfig", "TeamWnd");
	g_FindWindowConfig.TerrainWnd = g_ini.Read("FindWindowConfig", "TerrainWnd");
	g_FindWindowConfig.SaveWnd = g_ini.Read("FindWindowConfig", "SaveWnd");
	g_FindWindowConfig.LoadWnd = g_ini.Read("FindWindowConfig", "LoadWnd");
	g_FindWindowConfig.NewWnd1 = g_ini.Read("FindWindowConfig", "NewWnd1");
	g_FindWindowConfig.NewWnd2 = g_ini.Read("FindWindowConfig", "NewWnd2");
	g_FindWindowConfig.NewWnd3 = g_ini.Read("FindWindowConfig", "NewWnd3");

	// g_MessageBoxConfig
	g_MessageBoxConfig.Captain.Error = g_ini.Read("MessageBoxCaptain", "Error");
	g_MessageBoxConfig.Captain.Warning = g_ini.Read("MessageBoxCaptain", "Warning");
	g_MessageBoxConfig.Captain.Hint = g_ini.Read("MessageBoxCaptain", "Hint");

	g_MessageBoxConfig.Message.HookFailed = g_ini.Read("MessageBoxMessage", "HookFailed");
	g_MessageBoxConfig.Message.UnHookFailed = g_ini.Read("MessageBoxMessage", "UnHookFailed");
	g_MessageBoxConfig.Message.IniNotExist = g_ini.Read("MessageBoxMessage", "IniNotExist");
	g_MessageBoxConfig.Message.TerrainDisabled = g_ini.Read("MessageBoxMessage", "TerrainDisabled");
	g_MessageBoxConfig.Message.TerrainMapUnloaded = g_ini.Read("MessageBoxMessage", "TerrainMapUnloaded");
	g_MessageBoxConfig.Message.TerrainMapUnknown = g_ini.Read("MessageBoxMessage", "TerrainMapUnknown");
	g_MessageBoxConfig.Message.TriggerEventFull = g_ini.Read("MessageBoxMessage", "TriggerEventFull");
	g_MessageBoxConfig.Message.TriggerActionFull = g_ini.Read("MessageBoxMessage", "TriggerActionFull");
	g_MessageBoxConfig.Message.HouseWndNotFound = g_ini.Read("MessageBoxMessage", "HouseWndNotFound");
	g_MessageBoxConfig.Message.INISectionNotFound = g_ini.Read("MessageBoxMessage", "INISectionNotFound");
	
}
#pragma endregion

#pragma region DllMain Function
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:{
		g_hModule = hModule;
		g_logger = (std::string)"FA2Copy.log";
		g_logger.Custom(
			"[Global]",
			"FA2Copy.dll is attaching...",
			true
		);
		if (!StartHook())
		{
			g_logger.Error("Failed to set hooks!");
			MessageBox(
				NULL,
				g_MessageBoxConfig.Message.HookFailed.c_str(),
				g_MessageBoxConfig.Captain.Error.c_str(),
				MB_OK
			);
			return FALSE;
		}
		break;
	}
	case DLL_PROCESS_DETACH:{
		g_logger.Custom(
			"[Global]",
			"FA2Copy.dll is detaching...",
			true
		);
		if (g_GetMsgHooked) {
			g_logger.Info("Unregistering hot keys...");
			UnregisterHotKey(g_FA2Wnd, g_CTRL_S);
			UnregisterHotKey(g_FA2Wnd, g_CTRL_O);
			UnregisterHotKey(g_FA2Wnd, g_CTRL_N);
		}
		if (EndHook() == FALSE)
		{
			g_logger.Error("Failed to release hooks!");
			MessageBox(
				NULL,
				g_MessageBoxConfig.Message.UnHookFailed.c_str(),
				g_MessageBoxConfig.Captain.Error.c_str(),
				MB_OK
			);
			return FALSE;
		}
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
#pragma endregion

#pragma region Export Function
__declspec(dllexport) void FA2CopyImportFunc()
{
	//Do nothing
}
#pragma endregion
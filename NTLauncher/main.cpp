#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"
#include "qq_ipc.h"

#pragma comment(lib, "MMMojoCall.lib")

#define IDC_CHECK_QQSCREENSHOT               1001
#define IDC_CHECK_WECHATOCR                  1002
#define IDC_CHECK_WECHATUTILITY              1003
#define IDC_CHECK_AUTORUN                    1004
#define IDC_CHECK_SCROLLVOLICE               1005
#define IDC_CHECK_RUNTIP                     1006
#define IDC_CHECK_RUNLAUNCHNTVIEWER          1007
#define IDC_CHECK_OPENNTLAUNCHERDEBUGCONSOLE 1008
#define IDC_CHECK_OPENNTVIEWERDEBUGCONSOLE   1009
#define IDC_CHECK_KILLXPLUGLIN               1010
#define IDC_CHECK_OCR                        1011
#define IDC_CHECK_AUTOEXITCLEAR              1012
#define IDC_EDIT_QQSCREENSHOT                1013
#define IDC_BUTTON_AUTOOBTAINQQSCREENSHOT    1014
#define IDC_EDIT_USRLIBPATH                  1015
#define IDC_BUTTON_TENCENTOCR                1016
#define IDC_BUTTON_WECHATXPLUGIN             1017
#define IDC_EDIT_WECHATOCR                   1018
#define IDC_EDIT_WECHATUTILITY               1019
#define IDC_EDIT_PYTHONDIR                   1020
#define IDC_BUTTON_AUTOOBTAINPYTHONDIR       1021
#define IDC_EDIT_PLUGINDIR                   1022
#define IDC_CHECK_PLUGIN                     1023
#define IDC_EDIT_HOTKEYCTRL                  1024
#define IDC_EDIT_HOTKEYALT                   1025
#define IDC_EDIT_HOTKEYA                     1026
#define IDC_CHECK_HOTKEY                     1027
#define IDC_BUTTON_CLEARCACHE                1028
#define IDC_BUTTON_OK                        1029
#define IDC_BUTTON_CANCEL                    1030
#define ID_NOTIFYICON_MENU_OPEN              40001
#define ID_NOTIFYICON_MENU_EXIT              40002
#define WM_APP_INIT                          (WM_APP + 0)
#define WM_NOTIFYICON                        (WM_USER + 1)

// 应用配置
typedef struct _APP_CONFIG
{
	// ExePath
	TCHAR QQScreenShot[MAX_PATH];
	TCHAR UsrLib[MAX_PATH];
	TCHAR WeChatOCR[MAX_PATH];
	TCHAR WeChatUtility[MAX_PATH];

	// General
	BOOL AutoExitClear;
	BOOL AutoNTVOpen;
	BOOL AutoRun;
	BOOL DebugConsole;
	BOOL EnableHotKey;
	BOOL EnableOCR;
	BOOL EnablePlugin;
	BOOL EnableScreenShot;
	BOOL EnableUtility;
	BOOL FisrtRun;
	TCHAR HotKey[96];
	BOOL KillXPlugin;
	BOOL RunTip;
	BOOL ScrollVol;

	// NTViewer
	BOOL DbgConsole;
	BOOL LaunchShow;

	// Plugin
	TCHAR CurOCR[MAX_PATH];
	TCHAR CurSearch[MAX_PATH];
	TCHAR CurSoutu[MAX_PATH];
	TCHAR CurTran[MAX_PATH];
	TCHAR PluginDir[MAX_PATH];
	TCHAR PythonDir[MAX_PATH];
} APP_CONFIG;

HINSTANCE hInst;
// 应用配置路径
TCHAR appConfigPath[MAX_PATH];
// 应用配置数据
APP_CONFIG appConfig;
// 通知图标数据
NOTIFYICONDATA nid;
// 用于 QQ 截图
qqimpl::qqipc::QQIpcParentWrapper qqIpcParentWrapper;
// 子进程 PID
int childProcessPID;

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 获取应用配置路径
void GetAppConfigPath(TCHAR appConfigPath[MAX_PATH]);
// 加载应用配置
void LoadAppConfig(TCHAR* iniFile, APP_CONFIG* cfg);
// 保存应用配置
void SaveAppConfig(TCHAR* iniFile, APP_CONFIG* cfg);
// 获取 parent-ipc-core-x64.dll 路径
void GetParentIpcCorePath(char* parentIpcCorePath);
// 回调函数
void __stdcall OnReceiveScreenShotMessage(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size);

// 程序入口
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// 加载应用配置
	GetAppConfigPath(appConfigPath);
	LoadAppConfig(appConfigPath, &appConfig);

	// 填充窗口类
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_NTLAUNCHER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"NTLauncher";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_NTLAUNCHER));

	// 注册窗口类
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, L"调用 RegisterClassEx 失败！", L"NTLauncher", NULL);
		return 1;
	}

	hInst = hInstance;

	// 创建窗口
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"NTLauncher", L"NTLauncher", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 870, 600, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, L"调用 CreateWindowEx 失败！", L"NTLauncher", NULL);
		return 1;
	}

	// 显示窗口
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 显示通知图标
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NTLAUNCHER));
	nid.guidItem = { 0x30549556, 0x75c2, 0x452a, { 0xaf, 0x3c, 0x12, 0x32, 0xb, 0x53, 0x4e, 0x9 } }; // {30549556-75C2-452A-AF3C-12320B534E09}
	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), L"NTLauncher");
	if (Shell_NotifyIcon(NIM_ADD, &nid) != TRUE)
	{
		MessageBox(NULL, L"调用 Shell_NotifyIcon 失败！", L"NTLauncher", MB_ICONERROR);
		return 1;
	}

	// 初始化 QQ IPC
	char parentIpcCorePath[MAX_PATH];
	GetParentIpcCorePath(parentIpcCorePath);
	bool initEnvSuccess = qqIpcParentWrapper.InitEnv(parentIpcCorePath);
	if (!initEnvSuccess)
	{
		const char* err = qqIpcParentWrapper.GetLastErrStr();
		int len = MultiByteToWideChar(CP_ACP, 0, err, -1, NULL, 0);
		std::wstring werr(len, 0);
		MultiByteToWideChar(CP_ACP, 0, err, -1, &werr[0], len);
		std::wstring msg = L"调用 InitEnv 失败！" + werr;
		MessageBox(NULL, msg.c_str(), L"NTLauncher", MB_ICONERROR);

		return 1;
	}
	qqIpcParentWrapper.InitLog(0, NULL);
	qqIpcParentWrapper.InitParentIpc();
	char qqScreenShotExePath[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, appConfig.QQScreenShot, -1, qqScreenShotExePath, MAX_PATH, NULL, NULL);
	childProcessPID = qqIpcParentWrapper.LaunchChildProcess(qqScreenShotExePath, OnReceiveScreenShotMessage, NULL, NULL, 0);
	qqIpcParentWrapper.ConnectedToChildProcess(childProcessPID);

	// 消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		CreateWindow(L"BUTTON", L"控制", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 10, 10, 830, 250, hWnd, NULL, hInst, NULL);

		CreateWindow(L"BUTTON", L"组件开关", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 20, 30, 540, 60, hWnd, NULL, hInst, NULL);
		CreateWindow(L"BUTTON", L"启用QQ截图", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 30, 50, 110, 30, hWnd, (HMENU)IDC_CHECK_QQSCREENSHOT, hInst, NULL);
		CreateWindow(L"BUTTON", L"启用WeChatOCR", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 150, 50, 140, 30, hWnd, (HMENU)IDC_CHECK_WECHATOCR, hInst, NULL);
		CreateWindow(L"BUTTON", L"启用WeChatUtility", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 300, 50, 150, 30, hWnd, (HMENU)IDC_CHECK_WECHATUTILITY, hInst, NULL);

		CreateWindow(L"BUTTON", L"NTLauncher行为", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 20, 90, 340, 60, hWnd, NULL, hInst, NULL);
		CreateWindow(L"BUTTON", L"开机自启", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 30, 110, 90, 30, hWnd, (HMENU)IDC_CHECK_AUTORUN, hInst, NULL);
		CreateWindow(L"BUTTON", L"滚轮控制音量", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 130, 110, 120, 30, hWnd, (HMENU)IDC_CHECK_SCROLLVOLICE, hInst, NULL);
		CreateWindow(L"BUTTON", L"启动提示", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 260, 110, 90, 30, hWnd, (HMENU)IDC_CHECK_RUNTIP, hInst, NULL);

		CreateWindow(L"BUTTON", L"NTViewer行为", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 370, 90, 190, 60, hWnd, NULL, hInst, NULL);
		CreateWindow(L"BUTTON", L"启动时显示NTViewer", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 380, 110, 170, 30, hWnd, (HMENU)IDC_CHECK_RUNLAUNCHNTVIEWER, hInst, NULL);

		CreateWindow(L"BUTTON", L"调试开关", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 570, 30, 260, 120, hWnd, NULL, hInst, NULL);
		CreateWindow(L"BUTTON", L"打开NTLauncher调试控制台窗口", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 580, 50, 240, 30, hWnd, (HMENU)IDC_CHECK_OPENNTLAUNCHERDEBUGCONSOLE, hInst, NULL);
		CreateWindow(L"BUTTON", L"打开NTViewer调试控制台窗口", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 580, 90, 220, 30, hWnd, (HMENU)IDC_CHECK_OPENNTVIEWERDEBUGCONSOLE, hInst, NULL);

		CreateWindow(L"BUTTON", L"XPlugin行为", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 20, 150, 400, 100, hWnd, NULL, hInst, NULL);
		CreateWindow(L"BUTTON", L"5分钟未使用WeChat组件后自动结束组件进程", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 30, 170, 330, 30, hWnd, (HMENU)IDC_CHECK_KILLXPLUGLIN, hInst, NULL);

		CreateWindow(L"BUTTON", L"截图行为", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 430, 150, 400, 100, hWnd, NULL, hInst, NULL);
		CreateWindow(L"BUTTON", L"截图完成后自动进行OCR与QRScan", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 450, 170, 260, 30, hWnd, (HMENU)IDC_CHECK_OCR, hInst, NULL);
		CreateWindow(L"BUTTON", L"退出时自动清空截图缓存", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 450, 210, 220, 30, hWnd, (HMENU)IDC_CHECK_AUTOEXITCLEAR, hInst, NULL);

		CreateWindow(L"BUTTON", L"程序路径", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 10, 260, 830, 180, hWnd, NULL, hInst, NULL);

		CreateWindow(L"STATIC", L"QQScreenShot.exe：", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_LEFT, 20, 280, 140, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 170, 280, 550, 30, hWnd, (HMENU)IDC_EDIT_QQSCREENSHOT, hInst, NULL);
		CreateWindow(L"BUTTON", L"自动获取", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 730, 280, 100, 30, hWnd, (HMENU)IDC_BUTTON_AUTOOBTAINQQSCREENSHOT, hInst, NULL);

		CreateWindow(L"STATIC", L"UsrLibPath：", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_LEFT, 20, 320, 140, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 170, 320, 420, 30, hWnd, (HMENU)IDC_EDIT_USRLIBPATH, hInst, NULL);
		CreateWindow(L"BUTTON", L"TencentOCR", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 600, 320, 100, 30, hWnd, (HMENU)IDC_BUTTON_TENCENTOCR, hInst, NULL);
		CreateWindow(L"BUTTON", L"WeChatXPlugin", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 710, 320, 120, 30, hWnd, (HMENU)IDC_BUTTON_WECHATXPLUGIN, hInst, NULL);

		CreateWindow(L"STATIC", L"WeChatOCR.exe：", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_LEFT, 20, 360, 140, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 170, 360, 660, 30, hWnd, (HMENU)IDC_EDIT_WECHATOCR, hInst, NULL);

		CreateWindow(L"STATIC", L"WeChatUtility.exe：", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_LEFT, 20, 400, 140, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 170, 400, 660, 30, hWnd, (HMENU)IDC_EDIT_WECHATUTILITY, hInst, NULL);

		CreateWindow(L"BUTTON", L"插件", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 10, 440, 470, 100, hWnd, NULL, hInst, NULL);

		CreateWindow(L"STATIC", L"Python目录：", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_LEFT, 20, 460, 90, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 120, 460, 240, 30, hWnd, (HMENU)IDC_EDIT_PYTHONDIR, hInst, NULL);
		CreateWindow(L"BUTTON", L"自动获取", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 370, 460, 100, 30, hWnd, (HMENU)IDC_BUTTON_AUTOOBTAINPYTHONDIR, hInst, NULL);

		CreateWindow(L"STATIC", L"插件目录：", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_LEFT, 20, 500, 90, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 120, 500, 260, 30, hWnd, (HMENU)IDC_EDIT_PLUGINDIR, hInst, NULL);
		CreateWindow(L"BUTTON", L"启用插件", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 390, 500, 80, 30, hWnd, (HMENU)IDC_CHECK_PLUGIN, hInst, NULL);

		CreateWindow(L"BUTTON", L"修改热键（CTRL\\SHIFT\\ALT\\A-Z\\F1-F24）", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 490, 440, 350, 60, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 500, 460, 60, 30, hWnd, (HMENU)IDC_EDIT_HOTKEYCTRL, hInst, NULL);
		CreateWindow(L"STATIC", L"+", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_CENTER, 565, 460, 10, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 580, 460, 60, 30, hWnd, (HMENU)IDC_EDIT_HOTKEYALT, hInst, NULL);
		CreateWindow(L"STATIC", L"+", WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | SS_CENTER, 645, 460, 10, 30, hWnd, NULL, hInst, NULL);
		CreateWindow(L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 660, 460, 60, 30, hWnd, (HMENU)IDC_EDIT_HOTKEYA, hInst, NULL);
		CreateWindow(L"BUTTON", L"启用热键", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 730, 460, 90, 30, hWnd, (HMENU)IDC_CHECK_HOTKEY, hInst, NULL);

		CreateWindow(L"BUTTON", L"清空截图缓存", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 490, 510, 100, 30, hWnd, (HMENU)IDC_BUTTON_CLEARCACHE, hInst, NULL);
		CreateWindow(L"BUTTON", L"确定", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 630, 510, 100, 30, hWnd, (HMENU)IDC_BUTTON_OK, hInst, NULL);
		CreateWindow(L"BUTTON", L"取消", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 740, 510, 100, 30, hWnd, (HMENU)IDC_BUTTON_CANCEL, hInst, NULL);

		// 发送应用初始化消息
		PostMessage(hWnd, WM_APP_INIT, 0, 0);
	}
	break;
	case WM_APP_INIT:
	{
		HWND hItem;

		hItem = GetDlgItem(hWnd, IDC_CHECK_QQSCREENSHOT);
		SendMessage(hItem, BM_SETCHECK, appConfig.EnableScreenShot ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_WECHATOCR);
		SendMessage(hItem, BM_SETCHECK, appConfig.EnableOCR ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_WECHATUTILITY);
		SendMessage(hItem, BM_SETCHECK, appConfig.EnableUtility ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_AUTORUN);
		SendMessage(hItem, BM_SETCHECK, appConfig.AutoRun ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_SCROLLVOLICE);
		SendMessage(hItem, BM_SETCHECK, appConfig.ScrollVol ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_RUNTIP);
		SendMessage(hItem, BM_SETCHECK, appConfig.RunTip ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_RUNLAUNCHNTVIEWER);
		SendMessage(hItem, BM_SETCHECK, appConfig.LaunchShow ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_OPENNTLAUNCHERDEBUGCONSOLE);
		SendMessage(hItem, BM_SETCHECK, appConfig.DebugConsole ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_OPENNTVIEWERDEBUGCONSOLE);
		SendMessage(hItem, BM_SETCHECK, appConfig.DbgConsole ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_KILLXPLUGLIN);
		SendMessage(hItem, BM_SETCHECK, appConfig.KillXPlugin ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_OCR);
		SendMessage(hItem, BM_SETCHECK, appConfig.AutoNTVOpen ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_CHECK_AUTOEXITCLEAR);
		SendMessage(hItem, BM_SETCHECK, appConfig.AutoExitClear ? BST_CHECKED : BST_UNCHECKED, 0);

		hItem = GetDlgItem(hWnd, IDC_EDIT_QQSCREENSHOT);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)appConfig.QQScreenShot);

		hItem = GetDlgItem(hWnd, IDC_EDIT_USRLIBPATH);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)appConfig.UsrLib);

		hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATOCR);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)appConfig.WeChatOCR);

		hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATUTILITY);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)appConfig.WeChatUtility);

		hItem = GetDlgItem(hWnd, IDC_EDIT_PYTHONDIR);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)appConfig.PythonDir);

		hItem = GetDlgItem(hWnd, IDC_EDIT_PLUGINDIR);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)appConfig.PluginDir);

		hItem = GetDlgItem(hWnd, IDC_CHECK_PLUGIN);
		SendMessage(hItem, BM_SETCHECK, appConfig.EnablePlugin ? BST_CHECKED : BST_UNCHECKED, 0);

		TCHAR temp[96];
		_tcsncpy_s(temp, appConfig.HotKey, _TRUNCATE);
		TCHAR* context = NULL;
		TCHAR* hotkey1 = _tcstok_s(temp, L"+", & context);
		TCHAR* hotkey2 = _tcstok_s(NULL, L"+", & context);
		TCHAR* hotkey3 = _tcstok_s(NULL, L"+", &context);

		hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYCTRL);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)hotkey1);

		hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYALT);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)hotkey2);

		hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYA);
		SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)hotkey3);

		hItem = GetDlgItem(hWnd, IDC_CHECK_HOTKEY);
		SendMessage(hItem, BM_SETCHECK, appConfig.EnableHotKey ? BST_CHECKED : BST_UNCHECKED, 0);
	}
	break;
	case WM_NOTIFYICON:
	{
		// 显示通知图标弹出菜单
		if (lParam == WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, ID_NOTIFYICON_MENU_OPEN, L"打开");
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hMenu, MF_STRING, ID_NOTIFYICON_MENU_EXIT, L"退出");
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
		}
		// 单击鼠标左键时发起截图
		if (lParam == WM_LBUTTONDOWN && appConfig.EnableScreenShot)
		{
			qqIpcParentWrapper.SendIpcMessage(childProcessPID, "screenShot", NULL, 0);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_OK:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				HWND hItem;

				hItem = GetDlgItem(hWnd, IDC_CHECK_QQSCREENSHOT);
				appConfig.EnableScreenShot = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_WECHATOCR);
				appConfig.EnableOCR = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_WECHATUTILITY);
				appConfig.EnableUtility = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_AUTORUN);
				appConfig.AutoRun = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_SCROLLVOLICE);
				appConfig.ScrollVol = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_RUNTIP);
				appConfig.RunTip = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_RUNLAUNCHNTVIEWER);
				appConfig.LaunchShow = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_OPENNTLAUNCHERDEBUGCONSOLE);
				appConfig.DebugConsole = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_OPENNTVIEWERDEBUGCONSOLE);
				appConfig.DbgConsole = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_KILLXPLUGLIN);
				appConfig.KillXPlugin = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_OCR);
				appConfig.AutoNTVOpen = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_CHECK_AUTOEXITCLEAR);
				appConfig.AutoExitClear = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				hItem = GetDlgItem(hWnd, IDC_EDIT_QQSCREENSHOT);
				SendMessage(hItem, WM_GETTEXT, MAX_PATH, (LPARAM)appConfig.QQScreenShot);

				hItem = GetDlgItem(hWnd, IDC_EDIT_USRLIBPATH);
				SendMessage(hItem, WM_GETTEXT, MAX_PATH, (LPARAM)appConfig.UsrLib);

				hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATOCR);
				SendMessage(hItem, WM_GETTEXT, MAX_PATH, (LPARAM)appConfig.WeChatOCR);

				hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATUTILITY);
				SendMessage(hItem, WM_GETTEXT, MAX_PATH, (LPARAM)appConfig.WeChatUtility);

				hItem = GetDlgItem(hWnd, IDC_EDIT_PYTHONDIR);
				SendMessage(hItem, WM_GETTEXT, MAX_PATH, (LPARAM)appConfig.PythonDir);

				hItem = GetDlgItem(hWnd, IDC_EDIT_PLUGINDIR);
				SendMessage(hItem, WM_GETTEXT, MAX_PATH, (LPARAM)appConfig.PluginDir);

				hItem = GetDlgItem(hWnd, IDC_CHECK_PLUGIN);
				appConfig.EnablePlugin = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				TCHAR hotkey1[32];
				TCHAR hotkey2[32];
				TCHAR hotkey3[32];
				TCHAR hotkey[96];

				hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYCTRL);
				SendMessage(hItem, WM_GETTEXT, 32, (LPARAM)hotkey1);

				hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYALT);
				SendMessage(hItem, WM_GETTEXT, 32, (LPARAM)hotkey2);

				hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYA);
				SendMessage(hItem, WM_GETTEXT, 32, (LPARAM)hotkey3);

				_stprintf_s(hotkey, L"%s+%s+%s", hotkey1, hotkey2, hotkey3);
				_tcscpy_s(appConfig.HotKey, 96, hotkey);

				hItem = GetDlgItem(hWnd, IDC_CHECK_HOTKEY);
				appConfig.EnableHotKey = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				// 保存应用配置
				SaveAppConfig(appConfigPath, &appConfig);
			}
		}
		break;
		case ID_NOTIFYICON_MENU_OPEN:
		{
			ShowWindow(hWnd, SW_SHOW);
			SetForegroundWindow(hWnd);
		}
		break;
		case ID_NOTIFYICON_MENU_EXIT:
		{
			PostMessage(hWnd, WM_DESTROY, wParam, lParam);
		}
		break;
		}
	}
	break;
	case WM_CLOSE:
		// 拦截关闭请求，改为隐藏窗口
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

void GetAppConfigPath(TCHAR appConfigPath[MAX_PATH])
{
	// 获取 exe 全路径
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);

	// 找到最后一个 '\'，截断为目录，拼接 config.ini
	LPTSTR p = _tcsrchr(szExePath, L'\\');
	// 仅保留"路径\"
	if (p) *(p + 1) = 0;

	// 添加文件名
	_tcscat_s(szExePath, MAX_PATH, L"config.ini");

	// 输出
	_tcscpy_s(appConfigPath, MAX_PATH, szExePath);
}

void LoadAppConfig(TCHAR* iniFile, APP_CONFIG* cfg)
{
	// ExePath
	GetPrivateProfileString(L"ExePath", L"QQScreenShot", L"", cfg->QQScreenShot, MAX_PATH, iniFile);
	GetPrivateProfileString(L"ExePath", L"UsrLib", L"", cfg->UsrLib, MAX_PATH, iniFile);
	GetPrivateProfileString(L"ExePath", L"WeChatOCR", L"", cfg->WeChatOCR, MAX_PATH, iniFile);
	GetPrivateProfileString(L"ExePath", L"WeChatUtility", L"", cfg->WeChatUtility, MAX_PATH, iniFile);

	// General
	cfg->AutoExitClear = GetPrivateProfileInt(L"General", L"AutoExitClear", 0, iniFile);
	cfg->AutoNTVOpen = GetPrivateProfileInt(L"General", L"AutoNTVOpen", 0, iniFile);
	cfg->AutoRun = GetPrivateProfileInt(L"General", L"AutoRun", 0, iniFile);
	cfg->DebugConsole = GetPrivateProfileInt(L"General", L"DebugConsole", 0, iniFile);
	cfg->EnableHotKey = GetPrivateProfileInt(L"General", L"EnableHotKey", 0, iniFile);
	cfg->EnableOCR = GetPrivateProfileInt(L"General", L"EnableOCR", 0, iniFile);
	cfg->EnablePlugin = GetPrivateProfileInt(L"General", L"EnablePlugin", 0, iniFile);
	cfg->EnableScreenShot = GetPrivateProfileInt(L"General", L"EnableScreenShot", 0, iniFile);
	cfg->EnableUtility = GetPrivateProfileInt(L"General", L"EnableUtility", 0, iniFile);
	cfg->FisrtRun = GetPrivateProfileInt(L"General", L"FisrtRun", 0, iniFile);
	GetPrivateProfileString(L"General", L"HotKey", L"", cfg->HotKey, 96, iniFile);
	cfg->KillXPlugin = GetPrivateProfileInt(L"General", L"KillXPlugin", 0, iniFile);
	cfg->RunTip = GetPrivateProfileInt(L"General", L"RunTip", 0, iniFile);
	cfg->ScrollVol = GetPrivateProfileInt(L"General", L"ScrollVol", 0, iniFile);

	// NTViewer
	cfg->DbgConsole = GetPrivateProfileInt(L"NTViewer", L"DbgConsole", 0, iniFile);
	cfg->LaunchShow = GetPrivateProfileInt(L"NTViewer", L"LaunchShow", 0, iniFile);

	// Plugin
	GetPrivateProfileString(L"Plugin", L"CurOCR", L"", cfg->CurOCR, MAX_PATH, iniFile);
	GetPrivateProfileString(L"Plugin", L"CurSearch", L"", cfg->CurSearch, MAX_PATH, iniFile);
	GetPrivateProfileString(L"Plugin", L"CurSoutu", L"", cfg->CurSoutu, MAX_PATH, iniFile);
	GetPrivateProfileString(L"Plugin", L"CurTran", L"", cfg->CurTran, MAX_PATH, iniFile);
	GetPrivateProfileString(L"Plugin", L"PluginDir", L"", cfg->PluginDir, MAX_PATH, iniFile);
	GetPrivateProfileString(L"Plugin", L"PythonDir", L"", cfg->PythonDir, MAX_PATH, iniFile);
}

void SaveAppConfig(TCHAR* iniFile, APP_CONFIG* cfg)
{
	// ExePath
	WritePrivateProfileString(L"ExePath", L"QQScreenShot", cfg->QQScreenShot, iniFile);
	WritePrivateProfileString(L"ExePath", L"UsrLib", cfg->UsrLib, iniFile);
	WritePrivateProfileString(L"ExePath", L"WeChatOCR", cfg->WeChatOCR, iniFile);
	WritePrivateProfileString(L"ExePath", L"WeChatUtility", cfg->WeChatUtility, iniFile);

	// General
	WritePrivateProfileString(L"General", L"AutoExitClear", cfg->AutoExitClear ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"AutoNTVOpen", cfg->AutoNTVOpen ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"AutoRun", cfg->AutoRun ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"DebugConsole", cfg->DebugConsole ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"EnableHotKey", cfg->EnableHotKey ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"EnableOCR", cfg->EnableOCR ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"EnablePlugin", cfg->EnablePlugin ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"EnableScreenShot", cfg->EnableScreenShot ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"EnableUtility", cfg->EnableUtility ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"FisrtRun", cfg->FisrtRun ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"HotKey", cfg->HotKey, iniFile);
	WritePrivateProfileString(L"General", L"KillXPlugin", cfg->KillXPlugin ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"RunTip", cfg->RunTip ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"General", L"ScrollVol", cfg->ScrollVol ? L"1" : L"0", iniFile);

	// NTViewer
	WritePrivateProfileString(L"NTViewer", L"DbgConsole", cfg->DbgConsole ? L"1" : L"0", iniFile);
	WritePrivateProfileString(L"NTViewer", L"LaunchShow", cfg->LaunchShow ? L"1" : L"0", iniFile);

	// Plugin
	WritePrivateProfileString(L"Plugin", L"CurOCR", cfg->CurOCR, iniFile);
	WritePrivateProfileString(L"Plugin", L"CurSearch", cfg->CurSearch, iniFile);
	WritePrivateProfileString(L"Plugin", L"CurSoutu", cfg->CurSoutu, iniFile);
	WritePrivateProfileString(L"Plugin", L"CurTran", cfg->CurTran, iniFile);
	WritePrivateProfileString(L"Plugin", L"PluginDir", cfg->PluginDir, iniFile);
	WritePrivateProfileString(L"Plugin", L"PythonDir", cfg->PythonDir, iniFile);
}

void GetParentIpcCorePath(char* parentIpcCorePath)
{
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);

	LPTSTR p = _tcsrchr(szExePath, L'\\');
	if (p) *(p + 1) = 0;

	_tcscat_s(szExePath, MAX_PATH, L"parent-ipc-core-x64.dll");

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szExePath, -1, parentIpcCorePath, MAX_PATH, NULL, NULL);
#else
	strncpy_s(parentIpcCorePath, MAX_PATH, szParentIpcCorePath, _TRUNCATE);
#endif
}

void __stdcall OnReceiveScreenShotMessage(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size)
{
	int len = MultiByteToWideChar(CP_ACP, 0, msg, -1, NULL, 0);
	std::wstring wmsg(len > 1 ? len - 1 : 0, 0);
	if (len > 1)
		MultiByteToWideChar(CP_ACP, 0, msg, -1, &wmsg[0], len - 1);

	int len1 = MultiByteToWideChar(CP_ACP, 0, addition_msg, -1, NULL, 0);
	std::wstring waddition_msg(len1 > 1 ? len1 - 1 : 0, 0);
	if (len1 > 1)
		MultiByteToWideChar(CP_ACP, 0, addition_msg, -1, &waddition_msg[0], len1 - 1);

	std::wstring text = L"IPC消息：" + wmsg + L"\n参数3：" + std::to_wstring(arg3) + L"\n附加信息：" + waddition_msg + L"\n附加消息大小：" + std::to_wstring(addition_msg_size);

	MessageBox(NULL, text.c_str(), L"NTLauncher", MB_ICONINFORMATION);
}

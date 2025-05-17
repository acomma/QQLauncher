#include <windows.h>
#include <tchar.h>
#include "resource.h"

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
#define WM_APP_INIT                          (WM_APP + 0)

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

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 获取应用配置路径
void GetAppConfigPath(TCHAR appConfigPath[MAX_PATH]);
// 加载应用配置
void LoadAppConfig(TCHAR* iniFile, APP_CONFIG* cfg);
// 保存应用配置
void SaveAppConfig(TCHAR* iniFile, APP_CONFIG* cfg);

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
	wcex.lpszClassName = _T("NTLauncher");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_NTLAUNCHER));

	// 注册窗口类
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("调用 RegisterClassEx 失败！"), _T("NTLauncher"), NULL);
		return 1;
	}

	hInst = hInstance;

	// 创建窗口
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("NTLauncher"), _T("NTLauncher"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 870, 600, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, _T("调用 CreateWindowEx 失败！"), _T("NTLauncher"), NULL);
		return 1;
	}

	// 显示窗口
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

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
		SetWindowText(hItem, appConfig.QQScreenShot);

		hItem = GetDlgItem(hWnd, IDC_EDIT_USRLIBPATH);
		SetWindowText(hItem, appConfig.UsrLib);

		hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATOCR);
		SetWindowText(hItem, appConfig.WeChatOCR);

		hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATUTILITY);
		SetWindowText(hItem, appConfig.WeChatUtility);
		
		hItem = GetDlgItem(hWnd, IDC_EDIT_PYTHONDIR);
		SetWindowText(hItem, appConfig.PythonDir);

		hItem = GetDlgItem(hWnd, IDC_EDIT_PLUGINDIR);
		SetWindowText(hItem, appConfig.PluginDir);

		hItem = GetDlgItem(hWnd, IDC_CHECK_PLUGIN);
		SendMessage(hItem, BM_SETCHECK, appConfig.EnablePlugin ? BST_CHECKED : BST_UNCHECKED, 0);

		TCHAR temp[96];
		_tcsncpy_s(temp, appConfig.HotKey, _TRUNCATE);
		TCHAR* context = NULL;
		TCHAR* hotkey1 = _tcstok_s(temp, TEXT("+"), &context);
		TCHAR* hotkey2 = _tcstok_s(NULL, TEXT("+"), &context);
		TCHAR* hotkey3 = _tcstok_s(NULL, TEXT("+"), &context);

		hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYCTRL);
		SetWindowText(hItem, hotkey1);

		hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYALT);
		SetWindowText(hItem, hotkey2);

		hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYA);
		SetWindowText(hItem, hotkey3);

		hItem = GetDlgItem(hWnd, IDC_CHECK_HOTKEY);
		SendMessage(hItem, BM_SETCHECK, appConfig.EnableHotKey ? BST_CHECKED : BST_UNCHECKED, 0);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_COMMAND: {
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
				GetWindowText(hItem, appConfig.QQScreenShot, MAX_PATH);

				hItem = GetDlgItem(hWnd, IDC_EDIT_USRLIBPATH);
				GetWindowText(hItem, appConfig.UsrLib, MAX_PATH);

				hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATOCR);
				GetWindowText(hItem, appConfig.WeChatOCR, MAX_PATH);

				hItem = GetDlgItem(hWnd, IDC_EDIT_WECHATUTILITY);
				GetWindowText(hItem, appConfig.WeChatUtility, MAX_PATH);

				hItem = GetDlgItem(hWnd, IDC_EDIT_PYTHONDIR);
				GetWindowText(hItem, appConfig.PythonDir, MAX_PATH);

				hItem = GetDlgItem(hWnd, IDC_EDIT_PLUGINDIR);
				GetWindowText(hItem, appConfig.PluginDir, MAX_PATH);

				hItem = GetDlgItem(hWnd, IDC_CHECK_PLUGIN);
				appConfig.EnablePlugin = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				TCHAR hotkey1[32];
				TCHAR hotkey2[32];
				TCHAR hotkey3[32];
				TCHAR hotkey[96];

				hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYCTRL);
				GetWindowText(hItem, hotkey1, 32);

				hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYALT);
				GetWindowText(hItem, hotkey2, 32);

				hItem = GetDlgItem(hWnd, IDC_EDIT_HOTKEYA);
				GetWindowText(hItem, hotkey3, 32);

				_stprintf_s(hotkey, _T("%s+%s+%s"), hotkey1, hotkey2, hotkey3);
				_tcscpy_s(appConfig.HotKey, 96, hotkey);

				hItem = GetDlgItem(hWnd, IDC_CHECK_HOTKEY);
				appConfig.EnableHotKey = (SendMessage(hItem, BM_GETCHECK, 0, 0) == BST_CHECKED);

				// 保存应用配置
				SaveAppConfig(appConfigPath, &appConfig);
			}
		}
		break;
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

void GetAppConfigPath(TCHAR cfgPath[MAX_PATH])
{
	// 获取 exe 全路径
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);

	// 找到最后一个 '\'，截断为目录，拼接 config.ini
	LPTSTR p = _tcsrchr(szExePath, _T('\\'));
	// 仅保留"路径\"
	if (p) *(p + 1) = 0;

	// 添加文件名
	_tcscat_s(szExePath, MAX_PATH, _T("config.ini"));

	// 输出
	_tcscpy_s(cfgPath, MAX_PATH, szExePath);
}

void LoadAppConfig(TCHAR* iniFile, APP_CONFIG* cfg)
{
	// ExePath
	GetPrivateProfileString(TEXT("ExePath"), TEXT("QQScreenShot"), TEXT(""), cfg->QQScreenShot, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("ExePath"), TEXT("UsrLib"), TEXT(""), cfg->UsrLib, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("ExePath"), TEXT("WeChatOCR"), TEXT(""), cfg->WeChatOCR, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("ExePath"), TEXT("WeChatUtility"), TEXT(""), cfg->WeChatUtility, MAX_PATH, iniFile);

	// General
	cfg->AutoExitClear = GetPrivateProfileInt(TEXT("General"), TEXT("AutoExitClear"), 0, iniFile);
	cfg->AutoNTVOpen = GetPrivateProfileInt(TEXT("General"), TEXT("AutoNTVOpen"), 0, iniFile);
	cfg->AutoRun = GetPrivateProfileInt(TEXT("General"), TEXT("AutoRun"), 0, iniFile);
	cfg->DebugConsole = GetPrivateProfileInt(TEXT("General"), TEXT("DebugConsole"), 0, iniFile);
	cfg->EnableHotKey = GetPrivateProfileInt(TEXT("General"), TEXT("EnableHotKey"), 0, iniFile);
	cfg->EnableOCR = GetPrivateProfileInt(TEXT("General"), TEXT("EnableOCR"), 0, iniFile);
	cfg->EnablePlugin = GetPrivateProfileInt(TEXT("General"), TEXT("EnablePlugin"), 0, iniFile);
	cfg->EnableScreenShot = GetPrivateProfileInt(TEXT("General"), TEXT("EnableScreenShot"), 0, iniFile);
	cfg->EnableUtility = GetPrivateProfileInt(TEXT("General"), TEXT("EnableUtility"), 0, iniFile);
	cfg->FisrtRun = GetPrivateProfileInt(TEXT("General"), TEXT("FisrtRun"), 0, iniFile);
	GetPrivateProfileString(TEXT("General"), TEXT("HotKey"), TEXT(""), cfg->HotKey, 96, iniFile);
	cfg->KillXPlugin = GetPrivateProfileInt(TEXT("General"), TEXT("KillXPlugin"), 0, iniFile);
	cfg->RunTip = GetPrivateProfileInt(TEXT("General"), TEXT("RunTip"), 0, iniFile);
	cfg->ScrollVol = GetPrivateProfileInt(TEXT("General"), TEXT("ScrollVol"), 0, iniFile);

	// NTViewer
	cfg->DbgConsole = GetPrivateProfileInt(TEXT("NTViewer"), TEXT("DbgConsole"), 0, iniFile);
	cfg->LaunchShow = GetPrivateProfileInt(TEXT("NTViewer"), TEXT("LaunchShow"), 0, iniFile);

	// Plugin
	GetPrivateProfileString(TEXT("Plugin"), TEXT("CurOCR"), TEXT(""), cfg->CurOCR, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("Plugin"), TEXT("CurSearch"), TEXT(""), cfg->CurSearch, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("Plugin"), TEXT("CurSoutu"), TEXT(""), cfg->CurSoutu, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("Plugin"), TEXT("CurTran"), TEXT(""), cfg->CurTran, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("Plugin"), TEXT("PluginDir"), TEXT(""), cfg->PluginDir, MAX_PATH, iniFile);
	GetPrivateProfileString(TEXT("Plugin"), TEXT("PythonDir"), TEXT(""), cfg->PythonDir, MAX_PATH, iniFile);
}

void SaveAppConfig(TCHAR* iniFile, APP_CONFIG* cfg)
{
	// ExePath
	WritePrivateProfileString(TEXT("ExePath"), TEXT("QQScreenShot"), cfg->QQScreenShot, iniFile);
	WritePrivateProfileString(TEXT("ExePath"), TEXT("UsrLib"), cfg->UsrLib, iniFile);
	WritePrivateProfileString(TEXT("ExePath"), TEXT("WeChatOCR"), cfg->WeChatOCR, iniFile);
	WritePrivateProfileString(TEXT("ExePath"), TEXT("WeChatUtility"), cfg->WeChatUtility, iniFile);

	// General
	WritePrivateProfileString(TEXT("General"), TEXT("AutoExitClear"), cfg->AutoExitClear ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("AutoNTVOpen"), cfg->AutoNTVOpen ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("AutoRun"), cfg->AutoRun ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("DebugConsole"), cfg->DebugConsole ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("EnableHotKey"), cfg->EnableHotKey ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("EnableOCR"), cfg->EnableOCR ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("EnablePlugin"), cfg->EnablePlugin ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("EnableScreenShot"), cfg->EnableScreenShot ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("EnableUtility"), cfg->EnableUtility ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("FisrtRun"), cfg->FisrtRun ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("HotKey"), cfg->HotKey, iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("KillXPlugin"), cfg->KillXPlugin ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("RunTip"), cfg->RunTip ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("General"), TEXT("ScrollVol"), cfg->ScrollVol ? TEXT("1") : TEXT("0"), iniFile);

	// NTViewer
	WritePrivateProfileString(TEXT("NTViewer"), TEXT("DbgConsole"), cfg->DbgConsole ? TEXT("1") : TEXT("0"), iniFile);
	WritePrivateProfileString(TEXT("NTViewer"), TEXT("LaunchShow"), cfg->LaunchShow ? TEXT("1") : TEXT("0"), iniFile);

	// Plugin
	WritePrivateProfileString(TEXT("Plugin"), TEXT("CurOCR"), cfg->CurOCR, iniFile);
	WritePrivateProfileString(TEXT("Plugin"), TEXT("CurSearch"), cfg->CurSearch, iniFile);
	WritePrivateProfileString(TEXT("Plugin"), TEXT("CurSoutu"), cfg->CurSoutu, iniFile);
	WritePrivateProfileString(TEXT("Plugin"), TEXT("CurTran"), cfg->CurTran, iniFile);
	WritePrivateProfileString(TEXT("Plugin"), TEXT("PluginDir"), cfg->PluginDir, iniFile);
	WritePrivateProfileString(TEXT("Plugin"), TEXT("PythonDir"), cfg->PythonDir, iniFile);
}

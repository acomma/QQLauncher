#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"
#include "qq_ipc.h"

#pragma comment(lib, "MMMojoCall.lib")

// ֪ͨͼ����Ϣ
#define WM_NOTIFYICON (WM_USER + 1)

// Ӧ������
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
// ֪ͨͼ������
NOTIFYICONDATA nid;
// ���öԻ����״̬
BOOL bSettingDialogOpen = FALSE;
// Ӧ������·��
TCHAR appConfigPath[MAX_PATH];
// Ӧ����������
APP_CONFIG appConfig;
// ���� QQ ��ͼ
qqimpl::qqipc::QQIpcParentWrapper qqIpcParentWrapper;
// �ص�����
void __stdcall OnReceiveScreenShotMessage(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size);
// �ӽ��� PID
int childProcessPID;

// ���ڹ���
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ���öԻ������
INT_PTR CALLBACK SettingDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// ��ȡӦ������·��
void GetAppConfigPath(TCHAR appConfigPath[MAX_PATH]);
// ����Ӧ������
void LoadAppConfig(TCHAR* iniFile, APP_CONFIG* cfg);
// ����Ӧ������
void SaveAppConfig(TCHAR* iniFile, APP_CONFIG* cfg);
// ��ȡ parent-ipc-core-x64.dll ·��
void GetParentIpcCorePath(char* parentIpcCorePath);

// �������
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// ��䴰����
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

	// ע�ᴰ����
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("���� RegisterClassEx ʧ�ܣ�"), _T("NTLauncher"), NULL);
		return 1;
	}

	// ����ʵ�����
	hInst = hInstance;

	// ��������
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("NTLauncher"), _T("NTLauncher"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 100, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, _T("���� CreateWindowEx ʧ�ܣ�"), _T("NTLauncher"), NULL);
		return 1;
	}

	// ��ʾ����
	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	// ����Ӧ������
	GetAppConfigPath(appConfigPath);
	LoadAppConfig(appConfigPath, &appConfig);

	// ��ʼ�� QQ IPC
	char parentIpcCorePath[MAX_PATH];
	GetParentIpcCorePath(parentIpcCorePath);
	bool initEnvSuccess = qqIpcParentWrapper.InitEnv(parentIpcCorePath);
	if (!initEnvSuccess)
	{
		const char* err = qqIpcParentWrapper.GetLastErrStr();
		int len = MultiByteToWideChar(CP_ACP, 0, err, -1, NULL, 0);
		std::wstring werr(len, 0);
		MultiByteToWideChar(CP_ACP, 0, err, -1, &werr[0], len);
		std::wstring msg = L"���� InitEnv ʧ�ܣ�" + werr;
		MessageBox(NULL, msg.c_str(), L"NTLauncher", NULL);

		return 1;
	}
	qqIpcParentWrapper.InitLog(0, NULL);
	qqIpcParentWrapper.InitParentIpc();
	char qqScreenShotExePath[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, appConfig.QQScreenShot, -1, qqScreenShotExePath, MAX_PATH, NULL, NULL);
	childProcessPID = qqIpcParentWrapper.LaunchChildProcess(qqScreenShotExePath, OnReceiveScreenShotMessage, NULL, NULL, 0);
	qqIpcParentWrapper.ConnectedToChildProcess(childProcessPID);

	// ��ʾ֪ͨͼ��
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NTLAUNCHER));
	nid.guidItem = { 0x30549556, 0x75c2, 0x452a, { 0xaf, 0x3c, 0x12, 0x32, 0xb, 0x53, 0x4e, 0x9 } }; // {30549556-75C2-452A-AF3C-12320B534E09}
	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), _T("NTLauncher"));
	if (Shell_NotifyIcon(NIM_ADD, &nid) != TRUE)
	{
		MessageBox(NULL, _T("���� Shell_NotifyIcon ʧ�ܣ�"), _T("NTLauncher"), NULL);
		return 1;
	}

	// ��Ϣѭ��
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
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_NOTIFYICON:
	{
		// ��ʾ֪ͨͼ�굯���˵�
		if (lParam == WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);

			HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_NOTIFYICON_MENU));
			HMENU hPopup = GetSubMenu(hMenu, 0);

			if (hPopup)
			{
				SetForegroundWindow(hWnd);
				TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
			}

			DestroyMenu(hMenu);
		}
		// ����������ʱ�����ͼ
		if (lParam == WM_LBUTTONDOWN)
		{
			qqIpcParentWrapper.SendIpcMessage(childProcessPID, "screenShot", NULL, 0);
		}
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_NOTIFYICON_MENU_EXIT:
		{
			// �˳�����
			PostMessage(hWnd, WM_DESTROY, wParam, lParam);
		}
		break;
		case ID_NOTIFYICON_MENU_SETTING:
		{
			// ȷ��ֻ��һ�����öԻ���
			if (!bSettingDialogOpen)
			{
				bSettingDialogOpen = TRUE;
				if (DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTING_DIALOG), hWnd, SettingDialogProc) == IDOK)
				{
					MessageBox(NULL, _T("IDOK"), _T("NTLauncher"), NULL);
				}
				else
				{
					MessageBox(NULL, _T("IDCANCEL"), _T("NTLauncher"), NULL);
				}
				bSettingDialogOpen = FALSE;
			}
			else
			{
				HWND hDialog = FindWindow(NULL, _T("����"));
				SetForegroundWindow(hDialog);
				// ���ߵ���
				//SetActiveWindow(hDialog);
			}
		}
		break;
		}
	}
	break;
	case WM_DESTROY:
		// �˳��ӽ���
		qqIpcParentWrapper.TerminateChildProcess(childProcessPID, 0, true);

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

INT_PTR CALLBACK SettingDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Ϊ�Ի����ֵ
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_QQSCREENSHOT, BM_SETCHECK, appConfig.EnableScreenShot ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_WECHATOCR, BM_SETCHECK, appConfig.EnableOCR ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_WECHATUTILITY, BM_SETCHECK, appConfig.EnableUtility ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_AUTORUN, BM_SETCHECK, appConfig.AutoRun ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_SCROLLVOLICE, BM_SETCHECK, appConfig.ScrollVol ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_RUNTIP, BM_SETCHECK, appConfig.RunTip ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_RUNLAUNCHNTVIEWER, BM_SETCHECK, appConfig.LaunchShow ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_OPENNTLAUNCHERDEBUGCONSOLE, BM_SETCHECK, appConfig.DebugConsole ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_OPENNTVIEWERDEBUGCONSOLE, BM_SETCHECK, appConfig.DbgConsole ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_KILLXPLUGLIN, BM_SETCHECK, appConfig.KillXPlugin ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_OCR, BM_SETCHECK, appConfig.AutoNTVOpen ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_AUTOEXITCLEAR, BM_SETCHECK, appConfig.AutoExitClear ? BST_CHECKED : BST_UNCHECKED, 0);

		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_QQSCREENSHOT, appConfig.QQScreenShot);
		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_USRLIBPATH, appConfig.UsrLib);
		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_WECHATOCR, appConfig.WeChatOCR);
		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_WECHATUTILITY, appConfig.WeChatUtility);

		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_PYTHONDIR, appConfig.PythonDir);
		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_PLUGINDIR, appConfig.PluginDir);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_PLUGIN, BM_SETCHECK, appConfig.EnablePlugin ? BST_CHECKED : BST_UNCHECKED, 0);

		TCHAR temp[96];
		_tcsncpy_s(temp, appConfig.HotKey, _TRUNCATE);
		TCHAR* context = NULL;
		TCHAR* hotkey1 = _tcstok_s(temp, TEXT("+"), &context);
		TCHAR* hotkey2 = _tcstok_s(NULL, TEXT("+"), &context);
		TCHAR* hotkey3 = _tcstok_s(NULL, TEXT("+"), &context);

		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_HOTKEYCTRL, hotkey1);
		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_HOTKEYALT, hotkey2);
		SetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_HOTKEYA, hotkey3);
		SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_HOTKEY, BM_SETCHECK, appConfig.EnableHotKey ? BST_CHECKED : BST_UNCHECKED, 0);

		// ����Ļ���½ǵ����Ի���
		RECT rcScreen, rcDialog;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
		GetWindowRect(hWnd, &rcDialog);
		int x = rcScreen.right - (rcDialog.right - rcDialog.left) - 10;
		int y = rcScreen.bottom - (rcDialog.bottom - rcDialog.top) - 10;
		SetWindowPos(hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// �ռ��Ի������ֵ
			appConfig.EnableScreenShot = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_QQSCREENSHOT, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.EnableOCR = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_WECHATOCR, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.EnableUtility = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_WECHATUTILITY, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.AutoRun = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_AUTORUN, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.ScrollVol = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_SCROLLVOLICE, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.RunTip = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_RUNTIP, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.LaunchShow = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_RUNLAUNCHNTVIEWER, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.DebugConsole = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_OPENNTLAUNCHERDEBUGCONSOLE, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.DbgConsole = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_OPENNTVIEWERDEBUGCONSOLE, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.KillXPlugin = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_KILLXPLUGLIN, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.AutoNTVOpen = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_OCR, BM_GETCHECK, 0, 0) == BST_CHECKED;
			appConfig.AutoExitClear = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_AUTOEXITCLEAR, BM_GETCHECK, 0, 0) == BST_CHECKED;

			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_QQSCREENSHOT, appConfig.QQScreenShot, MAX_PATH);
			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_USRLIBPATH, appConfig.UsrLib, MAX_PATH);
			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_WECHATOCR, appConfig.WeChatOCR, MAX_PATH);
			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_WECHATUTILITY, appConfig.WeChatUtility, MAX_PATH);

			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_PYTHONDIR, appConfig.PythonDir, MAX_PATH);
			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_PLUGINDIR, appConfig.PluginDir, MAX_PATH);
			appConfig.EnablePlugin = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_PLUGIN, BM_GETCHECK, 0, 0) == BST_CHECKED;

			TCHAR hotkey1[32];
			TCHAR hotkey2[32];
			TCHAR hotkey3[32];
			TCHAR hotkey[96];

			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_HOTKEYCTRL, hotkey1, 32);
			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_HOTKEYALT, hotkey2, 32);
			GetDlgItemText(hWnd, IDC_SETTING_DIALOG_EDIT_HOTKEYA, hotkey3, 32);
			_stprintf_s(hotkey, _T("%s+%s+%s"), hotkey1, hotkey2, hotkey3);
			_tcscpy_s(appConfig.HotKey, 96, hotkey);
			appConfig.EnableHotKey = SendDlgItemMessage(hWnd, IDC_SETTING_DIALOG_CHECK_HOTKEY, BM_GETCHECK, 0, 0) == BST_CHECKED;

			// ����Ӧ������
			SaveAppConfig(appConfigPath, &appConfig);

			EndDialog(hWnd, IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
	}
	return FALSE;
}

void GetAppConfigPath(TCHAR cfgPath[MAX_PATH])
{
	// ��ȡ exe ȫ·��
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);

	// �ҵ����һ�� '\'���ض�ΪĿ¼��ƴ�� config.ini
	LPTSTR p = _tcsrchr(szExePath, _T('\\'));
	// ������"·��\"
	if (p) *(p + 1) = 0;

	// ����ļ���
	_tcscat_s(szExePath, MAX_PATH, _T("config.ini"));

	// ���
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

void GetParentIpcCorePath(char* parentIpcCorePath)
{
	TCHAR szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);

	LPTSTR p = _tcsrchr(szExePath, _T('\\'));
	if (p) *(p + 1) = 0;

	_tcscat_s(szExePath, MAX_PATH, _T("parent-ipc-core-x64.dll"));

#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szExePath, -1, parentIpcCorePath, MAX_PATH, NULL, NULL);
#else
	strncpy_s(parentIpcCorePath, MAX_PATH, szParentIpcCorePath, _TRUNCATE);
#endif
}

void __stdcall OnReceiveScreenShotMessage(void* pArg, char* msg, int arg3, char* addition_msg, int addition_msg_size)
{
	MessageBoxA(NULL, msg, "", 0);
}

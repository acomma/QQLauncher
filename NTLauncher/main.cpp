#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

// 通知图标消息
#define WM_NOTIFYICON (WM_USER + 1)

HINSTANCE hInst;
// 通知图标数据
NOTIFYICONDATA nid;
// 设置对话框打开状态
BOOL bSettingDialogOpen = FALSE;

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 设置对话框过程
INT_PTR CALLBACK SettingDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// 程序入口
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
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

	// 保存实例句柄
	hInst = hInstance;

	// 创建窗口
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("NTLauncher"), _T("NTLauncher"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 100, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, _T("调用 CreateWindowEx 失败！"), _T("NTLauncher"), NULL);
		return 1;
	}

	// 显示窗口
	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	// 显示通知图标
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NTLAUNCHER));
	nid.guidItem = { 0x30549556, 0x75c2, 0x452a, { 0xaf, 0x3c, 0x12, 0x32, 0xb, 0x53, 0x4e, 0x9 } }; // {30549556-75C2-452A-AF3C-12320B534E09}
	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), _T("NTLauncher"));
	if (Shell_NotifyIcon(NIM_ADD, &nid) != TRUE)
	{
		MessageBox(NULL, _T("调用 Shell_NotifyIcon 失败！"), _T("NTLauncher"), NULL);
		return 1;
	}

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
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_NOTIFYICON:
	{
		// 显示通知图标弹出菜单
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
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_NOTIFYICON_MENU_EXIT:
		{
			// 退出程序
			PostMessage(hWnd, WM_DESTROY, wParam, lParam);
		}
		break;
		case ID_NOTIFYICON_MENU_SETTING:
		{
			// 确保只打开一个设置对话框
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
				HWND hDialog = FindWindow(NULL, _T("设置"));
				SetForegroundWindow(hDialog);
				// 或者调用
				//SetActiveWindow(hDialog);
			}
		}
		break;
		}
	}
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

INT_PTR CALLBACK SettingDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		// 在屏幕右下角弹出对话框
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
			EndDialog(hWnd, IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
	}
	return FALSE;
}

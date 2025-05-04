#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

// ֪ͨͼ����Ϣ
#define WM_NOTIFYICON (WM_USER + 1)

HINSTANCE hInst;
// ֪ͨͼ������
NOTIFYICONDATA nid;

// ���ڹ���
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

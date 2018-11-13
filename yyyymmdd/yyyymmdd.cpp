#include "stdafx.h"
#include "yyyymmdd.h"

#include <time.h>
#include <locale.h>
#include <Shlwapi.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <unordered_map>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")

#define MAX_LOADSTRING 100
#define WM_NOTIFYICON (WM_USER+100)
#define NOTIFY_UID 1

HINSTANCE g_hInst;
WCHAR szTitle[MAX_LOADSTRING] = L"YYYYMMDD";
LPCWSTR szWindowClass = L"{C06B20FA-1B4C-49C1-A84E-AC3E5F8470D5}";

static ATOM                MyRegisterClass(HINSTANCE hInstance);
static bool                InitInstance(HINSTANCE, int);
static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL AddNotifyIcon(HWND hWnd, unsigned int uID) {
	NOTIFYICONDATA nid = {};

	nid.cbSize = sizeof(nid);
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.hWnd = hWnd;
	nid.uID = uID;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SMALL));
	_tcscpy_s(nid.szTip, TEXT("YYYYMMDD"));

	return Shell_NotifyIcon(NIM_ADD, &nid);
}
void DeleteNotifyIcon(HWND hWnd, unsigned int uID) {
	NOTIFYICONDATA nid = {};

	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = uID;

	Shell_NotifyIcon(NIM_DELETE, &nid);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	//wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YYYYMMDD));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

enum HotkeyIds
{
	kHotkeyCtrlOemPlus = 0xB0B2,
	kHotkeyCtrlAltOemPlus = 0xB0B3,
	kHotkeyCtrlShiftOemPlus = 0xB0B6,
};

struct HotkeyInfo
{
	LPCWSTR format;
	UINT fsModifiers;
	UINT vk;
	LPCWSTR iniKey;
};

static const std::unordered_map<HotkeyIds, HotkeyInfo> g_registerdHotkeyIds =
{
	{ kHotkeyCtrlOemPlus, { L"%Y/%m/%d" ,MOD_CONTROL, VK_OEM_PLUS } },
	{ kHotkeyCtrlAltOemPlus, {L"%Y%m%d" ,MOD_CONTROL | MOD_ALT, VK_OEM_PLUS } },
	{ kHotkeyCtrlShiftOemPlus, { L"%Y-%m-%d",MOD_CONTROL | MOD_SHIFT, VK_OEM_PLUS } },
};

bool InitInstance(HINSTANCE hInstance, int /*nCmdShow*/)
{
	g_hInst = hInstance;


	HWND hWnd = CreateWindow(szWindowClass, szTitle, 0,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return false;
	}

	for (auto i : g_registerdHotkeyIds)
	{
		if (!RegisterHotKey(hWnd, i.first, i.second.fsModifiers, i.second.vk))
		{
			MessageBox(hWnd, L"Failed To RegisterHotKey.", szTitle, MB_ICONEXCLAMATION | MB_OK);
			return false;
		}
	}

	if (!AddNotifyIcon(hWnd, NOTIFY_UID))
	{
		return false;
	}

	return true;
}

static std::wstring getModuleName()
{
	WCHAR path[MAX_PATH] = {};
	GetModuleFileName(nullptr, path, _countof(path));
	return path;
}

void inputDateString(LPCWSTR keyName, LPCWSTR defaultValue, bool useGMT = false)
{
	wchar_t text[100] = {};
	time_t t = time(NULL);
	tm tm;

	useGMT ? gmtime_s(&tm, &t) : localtime_s(&tm, &t);

	WCHAR formatStr[100];
	GetPrivateProfileString(L"yyyymmdd", keyName, defaultValue, formatStr, _countof(formatStr), (getModuleName() + L".ini").c_str());
	wcsftime(text, _countof(text), formatStr, &tm);

	std::vector<INPUT> inputs;
	for (size_t i = 0; i < wcslen(text); i++)
	{
		INPUT input = { INPUT_KEYBOARD };
		input.ki.wScan = text[i];
		input.ki.dwFlags = KEYEVENTF_UNICODE; // key down
		inputs.push_back(input);

		input.ki.dwFlags |= KEYEVENTF_KEYUP;
		inputs.push_back(input);
	}

	UINT ret = SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
	if (ret == 0)
	{
		wprintf(L"SendInput Error: 0x%x\n", ::GetLastError());
	}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static UINT s_uTaskbarRestart;
	switch (message)
	{
	case WM_CREATE:
		s_uTaskbarRestart = RegisterWindowMessage(L"TaskbarCreated");
		break;
	case WM_HOTKEY:
	{
		auto found = g_registerdHotkeyIds.find((HotkeyIds)wParam);
		if (found != g_registerdHotkeyIds.end())
		{
			inputDateString(found->second.iniKey, found->second.format);
			break;
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	case WM_DESTROY:
		for (auto id : g_registerdHotkeyIds)
		{
			UnregisterHotKey(hWnd, id.first);
		}
		DeleteNotifyIcon(hWnd, NOTIFY_UID);
		PostQuitMessage(0);
		return 0;
	case WM_NOTIFYICON:
		switch (lParam) {
		case WM_RBUTTONDOWN:
			POINT pt;
			GetCursorPos(&pt);
			HMENU menu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU1));
			TrackPopupMenu(GetSubMenu(menu, 0), TPM_LEFTALIGN, pt.x, pt.y, NULL, hWnd, NULL);
			DestroyMenu(menu);
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_ROOT_EXIT:
			DestroyWindow(hWnd);
		default:
			break;
		}
		break;
	default:
		if (message == s_uTaskbarRestart)
			AddNotifyIcon(hWnd, NOTIFY_UID);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
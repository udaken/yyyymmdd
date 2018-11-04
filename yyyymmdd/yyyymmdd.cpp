// shortcut.cpp: アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "yyyymmdd.h"

#include <time.h>
#include <Shlwapi.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <tuple>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")

#if defined(_MSC_VER) & (_MSC_VER < 1600) & !defined(_WIN32LIB_NO_NULLPTR)
namespace std {

	static const                        // this is a const object
		class nullptr_t
	{
	public:
		template<typename T>             // convertible to any type
		operator T* () const           // of null non-member pointer...
		{
			return 0;
		}
		template<typename C, typename T> // or any type of null
		operator T C::* () const       // member pointer...
		{
			return 0;
		}
	private:
		void operator&() const { std::abort(); };    // whose address can't be taken
	} nullptr = {};              // and whose name is nullptr
}

static const std::nullptr_t& nullptr = std::nullptr;

#endif

#define MAX_LOADSTRING 100
#define WM_NOTIFYICON (WM_USER+100)
#define NOTIFY_UID 1

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING] = L"";                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING] = L"{C06B20FA-1B4C-49C1-A84E-AC3E5F8470D5}";            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
static ATOM                MyRegisterClass(HINSTANCE hInstance);
static BOOL                InitInstance(HINSTANCE, int);
static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL AddNotifyIcon(HWND hWnd, unsigned int uID) {
	NOTIFYICONDATA nid = {};

	nid.cbSize = sizeof(nid);
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.hWnd = hWnd;
	nid.uID = uID;
	nid.uCallbackMessage = WM_NOTIFYICON;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_YYYYMMDD));
	_tcscpy(nid.szTip, TEXT("YYYYMMDD"));

	return Shell_NotifyIcon(NIM_ADD, &nid);
}
void DeleteNotifyIcon(HWND hWnd, unsigned int uID) {
	NOTIFYICONDATA nid = {};

	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = uID;

	Shell_NotifyIcon(NIM_DELETE, &nid);
}
#if 1

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
#else
int main()
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	int nCmdShow = SW_SHOW;
#endif
	// TODO: ここにコードを挿入してください。

	// グローバル文字列を初期化しています。
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;
	// メイン メッセージ ループ:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YYYYMMDD));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

const int hotkeyId = 0xB0B0;

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int /*nCmdShow*/)
{
	hInst = hInstance; // グローバル変数にインスタンス処理を格納します。


	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	if (!RegisterHotKey(hWnd, hotkeyId, MOD_CONTROL | MOD_ALT, VK_OEM_PLUS))
	{
		return FALSE;
	}

	if (!AddNotifyIcon(hWnd, NOTIFY_UID))
	{
		return FALSE;
	}

	return TRUE;
}

DWORD GetWindowModuleFileNameEx(HWND hWnd,
	__out_ecount(nSize) LPWSTR lpFilename,
	__in DWORD nSize)
{
	DWORD pid;
	if (GetWindowThreadProcessId(hWnd, &pid))
	{
		HANDLE hProcess = OpenProcess(GENERIC_ALL, FALSE, pid);
		HMODULE hModules[256];
		DWORD need;

		EnumProcessModules(hProcess, hModules, sizeof(hModules), &need);
		DWORD ret = GetModuleFileNameEx(hProcess, hModules[0], lpFilename, nSize);
		CloseHandle(hProcess);
		return ret;
	}
	return 0;
}

std::vector<std::tuple<WORD, bool>> charToVk(LPCWSTR str)
{
	std::vector<std::tuple<WORD, bool>> v;
	for (size_t i = 0; i < wcslen(str); i++)
	{
		SHORT vkAndShift = VkKeyScan(str[i]);
		if (vkAndShift == -1)
			continue;
		BYTE vk = LOBYTE(vkAndShift);
		BYTE shift = HIBYTE(vkAndShift);
		if (shift & 1) v.emplace_back(VK_SHIFT, false);
		if (shift & 2) v.emplace_back(VK_CONTROL, false);
		if (shift & 4) v.emplace_back(VK_MENU, false);
		v.emplace_back(vk, false);
		v.emplace_back(vk, true);
		if (shift & 4) v.emplace_back(VK_MENU, true);
		if (shift & 2) v.emplace_back(VK_CONTROL, true);
		if (shift & 1) v.emplace_back(VK_SHIFT, true);
	}
	return v;
}

static std::wstring getModuleName()
{
	WCHAR path[MAX_PATH] = {};
	GetModuleFileName(nullptr, path, _countof(path));
	return path;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static UINT s_uTaskbarRestart;
	switch (message)
	{
	case WM_CREATE:
		s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
		break;
	case WM_HOTKEY:
		if (wParam == hotkeyId)
		{
			wchar_t text[100] = {};
			time_t t = time(NULL);
			tm tm;
			localtime_s(&tm, &t);
			WCHAR formatStr[100];
			GetPrivateProfileString(L"yyyymmdd", L"TIMEFORMAT", L"%Y%m%d", formatStr, _countof(formatStr), (getModuleName() + L".ini").c_str());
			wcsftime(text, _countof(text), formatStr, &tm);

			std::vector<INPUT> inputs;
#if 1
			for (size_t i = 0; i < wcslen(text); i++)
			{
				INPUT input = { INPUT_KEYBOARD };
				input.ki.wScan = text[i];
				input.ki.dwFlags = KEYEVENTF_UNICODE; // key down
				inputs.push_back(input);

				input.ki.dwFlags |= KEYEVENTF_KEYUP;
				inputs.push_back(input);
			}
#else
			auto v = charToVk(text);
			for (auto& i : v)
			{
				INPUT input = { INPUT_KEYBOARD };
				input.ki.wVk = std::get<0>(i);
				input.ki.wScan = MapVirtualKey(input.ki.wVk, 0);
				input.ki.dwFlags = KEYEVENTF_EXTENDEDKEY; // key down
				if (std::get<1>(i))
					input.ki.dwFlags |= KEYEVENTF_KEYUP;

				inputs.push_back(input);
			}
#endif
			UINT ret = SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
			if (ret == 0)
			{
				wprintf(L"SendInput Error: 0x%x\n", ::GetLastError());
			}
			break;
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	case WM_DESTROY:
		UnregisterHotKey(hWnd, hotkeyId);
		DeleteNotifyIcon(hWnd, NOTIFY_UID);
		PostQuitMessage(0);
		return 0;
	case WM_NOTIFYICON:
		switch (lParam) {
		case WM_RBUTTONDOWN:
			POINT pt;
			GetCursorPos(&pt);
			HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
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
		if(message == s_uTaskbarRestart)
			AddNotifyIcon(hWnd, NOTIFY_UID);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
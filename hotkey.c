#define _WIN32_WINNT 0x500

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <winuser.h>

#define WM_SYSTRAY (WM_USER + 6)

static void AddTrayIcon(HWND hwnd)
{
	NOTIFYICONDATA tnid;
	
#ifdef NIM_SETVERSION
	tnid.uVersion = 0;
	res = Shell_NotifyIcon(NIM_SETVERSION, &tnid);
#endif

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = 1;  /* unique within this systray use */
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uCallbackMessage = WM_SYSTRAY;
	tnid.hIcon = LoadIcon(NULL, IDI_QUESTION);
	strcpy(tnid.szTip, "Magical Hotkey Mystery Thingy");

	Shell_NotifyIcon(NIM_ADD, &tnid);
}

static void KillTrayIcon(HWND hwnd)
{
	NOTIFYICONDATA tnid;

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = 1;

	Shell_NotifyIcon(NIM_DELETE, &tnid);
}

static RECT dw_rect;
static RECT tb_rect;

static void updateGlobalVars()
{
	GetWindowRect(FindWindow("Shell_TrayWnd", NULL), &tb_rect);
	GetWindowRect(GetDesktopWindow(), &dw_rect);
}

static void myMoveWindow(HWND hwnd, int x, int y, int width, int height,
						 BOOL repaint)
{
	RECT r = {0,0,0,0};

	/* trick PuTTY into thinking it is resized manually */
	PostMessage(hwnd, WM_ENTERSIZEMOVE, 0, 0);
	MoveWindow(hwnd, x, y, width, height, repaint);
	PostMessage(hwnd, WM_SIZING, WMSZ_BOTTOM, (LPARAM) &r);
	PostMessage(hwnd, WM_EXITSIZEMOVE, 0, 0);
}

static void maxWindowVertically()
{
	RECT w;
	
	GetWindowRect(GetForegroundWindow(), &w);
	updateGlobalVars();
	myMoveWindow(GetForegroundWindow(), w.left, 0, w.right-w.left,
		dw_rect.bottom - (tb_rect.bottom-tb_rect.top), TRUE);
}

static void maxWindowHoriz()
{
	RECT w;
	
	GetWindowRect(GetForegroundWindow(), &w);
	updateGlobalVars();
	myMoveWindow(GetForegroundWindow(), 0, w.top,
		dw_rect.right-dw_rect.left, w.bottom-w.top, TRUE);
}

static void moveWindowLeft()
{
	RECT w;

	GetWindowRect(GetForegroundWindow(), &w);
	myMoveWindow(GetForegroundWindow(), 0, w.top,
		w.right-w.left, w.bottom-w.top, TRUE);
}

static void moveWindowRight()
{
	RECT w;

	GetWindowRect(GetForegroundWindow(), &w);
	updateGlobalVars();
	myMoveWindow(GetForegroundWindow(), dw_rect.right-(w.right-w.left),
		w.top, w.right-w.left, w.bottom-w.top, TRUE);
}

static void moveWindowUp()
{
	RECT w;

	GetWindowRect(GetForegroundWindow(), &w);
	updateGlobalVars();
	myMoveWindow(GetForegroundWindow(), w.left, 0, w.right-w.left,
		w.bottom-w.top, TRUE);
}

static void moveWindowDown()
{
	RECT w;

	GetWindowRect(GetForegroundWindow(), &w);
	updateGlobalVars();
	myMoveWindow(GetForegroundWindow(),
		w.left,
	
dw_rect.bottom-(w.bottom-w.top)-(tb_rect.bottom-tb_rect.top),
		w.right-w.left, w.bottom-w.top, TRUE);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
								WPARAM
wparam, LPARAM lparam)
{
	static UINT msgTaskbarCreated = 0;

	switch(message) {
		case WM_CREATE:
			msgTaskbarCreated =
RegisterWindowMessage(_T("TaskbarCreated"));
			break;
		default:
			if(message == msgTaskbarCreated) {
				/* Explorer crashed, so restore the tray
icon :-( */
				AddTrayIcon(hwnd);
			}
			break;

		case WM_SYSTRAY:
			if(lparam == WM_LBUTTONDBLCLK) {
				// Die.
				PostMessage(hwnd, WM_QUIT, 0, 0);
			}
			break;

		case WM_HOTKEY:
			switch(wparam) {
				case 0:
					keybd_event(VK_MEDIA_PLAY_PAUSE, 0,
0, 0);
					break;
				case 1:
					keybd_event(VK_VOLUME_UP, 0, 0, 0);
					break;
				case 2:
					keybd_event(VK_VOLUME_DOWN, 0, 0,
0);
					break;
				case 3:
					maxWindowVertically();
					break;
				case 4:
					moveWindowLeft();
					break;
				case 5:
					moveWindowRight();
					break;
				case 6:
					moveWindowUp();
					break;
				case 7:
					moveWindowDown();
					break;
				case 8:
					maxWindowHoriz();
					break;
			}
			break;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmdline, int show)
{
	bool already_running = FindWindow("KPHotkey", "Magic") ? true :
false;

	if(already_running) {
		MessageBox(NULL, "Hotkey Program Already Running",
"DENIED!!",
			MB_ICONERROR | MB_OK);
		return 1;
	}

	WNDCLASS wndclass;
    wndclass.style = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = inst;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "KPHotkey";

    RegisterClass(&wndclass);

	HWND hwnd = CreateWindow("KPHotkey", "Magic", 0,
		CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, NULL, NULL, inst, NULL);

	RegisterHotKey(hwnd, 0, MOD_WIN, VkKeyScan('s'));
	RegisterHotKey(hwnd, 1, MOD_WIN, VK_PRIOR);  // pgup
	RegisterHotKey(hwnd, 2, MOD_WIN, VK_NEXT);   // pgdn
	RegisterHotKey(hwnd, 3, MOD_WIN, VkKeyScan('z'));
	RegisterHotKey(hwnd, 4, MOD_WIN, VK_LEFT);
	RegisterHotKey(hwnd, 5, MOD_WIN, VK_RIGHT);
	RegisterHotKey(hwnd, 6, MOD_WIN, VK_UP);
	RegisterHotKey(hwnd, 7, MOD_WIN, VK_DOWN);
	RegisterHotKey(hwnd, 8, MOD_WIN, VkKeyScan('x'));

	AddTrayIcon(hwnd);

	// Message Loop!
	MSG msg;
	while( GetMessage(&msg, NULL, 0, 0) == 1) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    KillTrayIcon(hwnd);
	return (int) msg.wParam;
}

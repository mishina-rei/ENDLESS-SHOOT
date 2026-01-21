#include "Input.h"

//--- ?O???[?o?????
BYTE g_keyTable[256];
BYTE g_oldTable[256];
HWND g_hWnd;
bool g_isMouseLock = false;
float g_mouseDeltaX = 0;
float g_mouseDeltaY = 0;

HRESULT InitInput(HWND hWnd)
{
	g_hWnd = hWnd;
	// ???????????
	GetKeyboardState(g_keyTable);
	return S_OK;
}
void UninitInput()
{
}
void UpdateInput()
{
	// ?Â???????X?V
	memcpy_s(g_oldTable, sizeof(g_oldTable), g_keyTable, sizeof(g_keyTable));
	// ???????????
	GetKeyboardState(g_keyTable);

	// ???????????
	if (IsKeyTrigger(VK_ESCAPE)) {
		SetMouseLock(false);
	}
	if (IsKeyTrigger(VK_LBUTTON)) {
		SetMouseLock(true);
	}
	// ?????
	g_mouseDeltaX = 0;
	g_mouseDeltaY = 0;

	if (g_isMouseLock && g_hWnd)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(g_hWnd, &pt);

		RECT rc;
		GetClientRect(g_hWnd, &rc);
		int cx = (rc.right - rc.left) / 2;
		int cy = (rc.bottom - rc.top) / 2;

		g_mouseDeltaX = pt.x - cx;
		g_mouseDeltaY = pt.y - cy;

		// ????????????
		POINT center = { cx, cy };
		ClientToScreen(g_hWnd, &center);
		SetCursorPos(center.x, center.y);
	}
}

bool IsKeyPress(BYTE key)
{
	return g_keyTable[key] & 0x80;
}
bool IsKeyTrigger(BYTE key)
{
	return (g_keyTable[key] ^ g_oldTable[key]) & g_keyTable[key] & 0x80;
}
bool IsKeyRelease(BYTE key)
{
	return (g_keyTable[key] ^ g_oldTable[key]) & g_oldTable[key] & 0x80;
}
bool IsKeyRepeat(BYTE key)
{
	return false;
}

void SetMouseLock(bool lock)
{
	if (g_isMouseLock == lock) return;
	g_isMouseLock = lock;

	if (g_isMouseLock)
	{
		// ?????????????????????
		if (g_hWnd) {
			RECT rc;
			GetClientRect(g_hWnd, &rc);
			POINT center = { (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };
			ClientToScreen(g_hWnd, &center);
			SetCursorPos(center.x, center.y);
		}
		ShowCursor(FALSE);
	}
	else
	{
		ShowCursor(TRUE);
	}
}

long GetMouseDeltaX() { return g_mouseDeltaX; }
long GetMouseDeltaY() { return g_mouseDeltaY; }
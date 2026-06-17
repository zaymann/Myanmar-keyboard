// Myanmar Romanized IME for Windows — a single-executable, WaitZar-style helper.
//
// How it works:
//   * A low-level keyboard hook watches your typing system-wide.
//   * When Myanmar mode is ON, Latin letters are captured into a buffer and a
//     small popup shows the live Myanmar Unicode conversion.
//   * Press SPACE / ENTER / punctuation to commit: the Burmese text is typed
//     into whatever app has focus (via SendInput Unicode), then the separator.
//   * Toggle Myanmar mode with Ctrl+Alt+M (or the tray icon menu).
//
// No installer, no admin rights, no COM registration. Just run the .exe.
//
// Build (MinGW):  see build.bat   ->  g++ main.cpp -o MyanmarIME.exe ...
// Requires a Myanmar Unicode font (Windows 10+ ships "Myanmar Text").

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include "../core/romanizer.h"

// ---- globals ---------------------------------------------------------------
static HHOOK   g_hook    = nullptr;
static HWND    g_mainWnd = nullptr;   // hidden window (tray + hotkey + msg loop)
static HWND    g_popup   = nullptr;   // composing popup
static bool    g_enabled = false;     // Myanmar mode on/off
static std::string g_buffer;          // roman letters being composed
static NOTIFYICONDATA g_nid = {};

static const UINT WM_TRAY   = WM_APP + 1;
static const UINT HOTKEY_ID = 1;
static const wchar_t* POPUP_CLASS = L"MyanmarIMEPopup";
static const wchar_t* MAIN_CLASS  = L"MyanmarIMEMain";

// ---- utf helpers -----------------------------------------------------------
static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

// ---- inject Unicode text into the focused application ----------------------
static void SendUnicode(const std::wstring& text) {
    std::vector<INPUT> in;
    in.reserve(text.size() * 2);
    for (wchar_t ch : text) {
        INPUT down = {}; down.type = INPUT_KEYBOARD;
        down.ki.wScan = ch; down.ki.dwFlags = KEYEVENTF_UNICODE;
        INPUT up = down; up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        in.push_back(down); in.push_back(up);
    }
    if (!in.empty()) SendInput((UINT)in.size(), in.data(), sizeof(INPUT));
}

// ---- composing popup -------------------------------------------------------
static void HidePopup() { if (g_popup) ShowWindow(g_popup, SW_HIDE); }

static void UpdatePopup() {
    if (!g_popup) return;
    if (g_buffer.empty()) { HidePopup(); return; }

    std::string converted = myanmar::convert(g_buffer);
    std::wstring text = Utf8ToWide(g_buffer) + L"  →  " + Utf8ToWide(converted);

    // position near the caret of the foreground window, fallback to cursor.
    POINT pt; GUITHREADINFO gti = { sizeof(gti) };
    HWND fg = GetForegroundWindow();
    DWORD tid = GetWindowThreadProcessId(fg, nullptr);
    if (GetGUIThreadInfo(tid, &gti) && gti.hwndCaret) {
        pt.x = gti.rcCaret.left; pt.y = gti.rcCaret.bottom;
        ClientToScreen(gti.hwndCaret, &pt);
    } else {
        GetCursorPos(&pt); pt.y += 20;
    }

    SetWindowTextW(g_popup, text.c_str());
    HDC dc = GetDC(g_popup);
    SIZE sz; GetTextExtentPoint32W(dc, text.c_str(), (int)text.size(), &sz);
    ReleaseDC(g_popup, dc);
    int w = sz.cx + 24, h = sz.cy + 12;
    SetWindowPos(g_popup, HWND_TOPMOST, pt.x, pt.y, w, h,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

static LRESULT CALLBACK PopupProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_PAINT) {
        PAINTSTRUCT ps; HDC dc = BeginPaint(h, &ps);
        RECT rc; GetClientRect(h, &rc);
        FillRect(dc, &rc, (HBRUSH)(COLOR_INFOBK + 1));
        HFONT font = CreateFontW(20, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                 DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0,
                                 L"Myanmar Text");
        HFONT old = (HFONT)SelectObject(dc, font);
        SetBkMode(dc, TRANSPARENT);
        wchar_t buf[512]; int n = GetWindowTextW(h, buf, 512);
        rc.left += 12; rc.top += 6;
        DrawTextW(dc, buf, n, &rc, DT_LEFT | DT_TOP | DT_NOPREFIX);
        SelectObject(dc, old); DeleteObject(font);
        EndPaint(h, &ps);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// ---- commit / mode ---------------------------------------------------------
static void CommitBuffer() {
    if (g_buffer.empty()) return;
    std::wstring out = Utf8ToWide(myanmar::convert(g_buffer));
    g_buffer.clear();
    HidePopup();
    SendUnicode(out);
}

static void SetEnabled(bool on) {
    if (!on) CommitBuffer();
    g_enabled = on;
    g_nid.uFlags = NIF_TIP;
    lstrcpynW(g_nid.szTip, on ? L"Myanmar IME: ON (Ctrl+Alt+M)"
                              : L"Myanmar IME: off (Ctrl+Alt+M)", ARRAYSIZE(g_nid.szTip));
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

// ---- the keyboard hook -----------------------------------------------------
static bool IsRomanVk(DWORD vk, char& outCh) {
    if (vk >= 'A' && vk <= 'Z') {                 // letters (keep case for 'N')
        bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        bool caps  = (GetKeyState(VK_CAPITAL) & 1) != 0;
        bool upper = shift ^ caps;
        outCh = upper ? (char)vk : (char)('a' + (vk - 'A'));
        return true;
    }
    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    if (vk == VK_OEM_PERIOD) { outCh = '.'; return true; }          // .
    if (vk == VK_OEM_1 && shift) { outCh = ':'; return true; }      // : (shift+;)
    if (vk == VK_OEM_MINUS && shift) { outCh = '_'; return true; }  // _ stacker (shift+-)
    return false;
}

static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code != HC_ACTION)
        return CallNextHookEx(g_hook, code, wParam, lParam);

    KBDLLHOOKSTRUCT* k = (KBDLLHOOKSTRUCT*)lParam;
    // ignore our own injected keystrokes
    if (k->flags & LLKHF_INJECTED)
        return CallNextHookEx(g_hook, code, wParam, lParam);

    bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
    if (!g_enabled || !keyDown)
        return CallNextHookEx(g_hook, code, wParam, lParam);

    DWORD vk = k->vkCode;

    // backspace edits the buffer
    if (vk == VK_BACK) {
        if (!g_buffer.empty()) { g_buffer.pop_back(); UpdatePopup(); return 1; }
        return CallNextHookEx(g_hook, code, wParam, lParam);
    }

    char ch;
    if (IsRomanVk(vk, ch)) {
        g_buffer.push_back(ch);
        UpdatePopup();
        return 1; // swallow — the letter goes into our buffer instead
    }

    // space / enter -> commit, then emit the separator ourselves
    if (vk == VK_SPACE || vk == VK_RETURN) {
        if (!g_buffer.empty()) {
            std::wstring out = Utf8ToWide(myanmar::convert(g_buffer));
            g_buffer.clear(); HidePopup();
            out += (vk == VK_SPACE) ? L" " : L"\r\n";
            SendUnicode(out);
            return 1;
        }
        return CallNextHookEx(g_hook, code, wParam, lParam);
    }

    // any other key (punctuation, digits): commit buffer, let the key pass
    if (!g_buffer.empty()) CommitBuffer();
    return CallNextHookEx(g_hook, code, wParam, lParam);
}

// ---- tray ------------------------------------------------------------------
static void ShowTrayMenu() {
    POINT pt; GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING | (g_enabled ? MF_CHECKED : 0), 1,
                L"Myanmar mode (Ctrl+Alt+M)");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 2, L"Exit");
    SetForegroundWindow(g_mainWnd);
    int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                             pt.x, pt.y, 0, g_mainWnd, nullptr);
    DestroyMenu(menu);
    if (cmd == 1) SetEnabled(!g_enabled);
    else if (cmd == 2) DestroyWindow(g_mainWnd);
}

static LRESULT CALLBACK MainProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_HOTKEY:
        if (w == HOTKEY_ID) SetEnabled(!g_enabled);
        return 0;
    case WM_TRAY:
        if (l == WM_RBUTTONUP || l == WM_LBUTTONUP) ShowTrayMenu();
        return 0;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    // hidden main window
    WNDCLASSW wc = {}; wc.lpfnWndProc = MainProc; wc.hInstance = hInst;
    wc.lpszClassName = MAIN_CLASS; RegisterClassW(&wc);
    g_mainWnd = CreateWindowW(MAIN_CLASS, L"MyanmarIME", 0, 0, 0, 0, 0,
                              HWND_MESSAGE, nullptr, hInst, nullptr);

    // composing popup (never takes focus)
    WNDCLASSW pc = {}; pc.lpfnWndProc = PopupProc; pc.hInstance = hInst;
    pc.lpszClassName = POPUP_CLASS; pc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&pc);
    g_popup = CreateWindowExW(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
                              POPUP_CLASS, L"", WS_POPUP, 0, 0, 10, 10,
                              nullptr, nullptr, hInst, nullptr);

    // tray icon
    g_nid.cbSize = sizeof(g_nid); g_nid.hWnd = g_mainWnd; g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    lstrcpynW(g_nid.szTip, L"Myanmar IME: off (Ctrl+Alt+M)", ARRAYSIZE(g_nid.szTip));
    Shell_NotifyIcon(NIM_ADD, &g_nid);

    RegisterHotKey(g_mainWnd, HOTKEY_ID, MOD_CONTROL | MOD_ALT, 'M');

    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, hInst, 0);
    if (!g_hook) { MessageBoxW(nullptr, L"Failed to install keyboard hook.",
                               L"MyanmarIME", MB_ICONERROR); return 1; }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg); DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(g_hook);
    return 0;
}

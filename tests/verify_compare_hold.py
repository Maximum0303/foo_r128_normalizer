"""Exercise the actual subclass body with a native-button message contract mock.

This catches reliance on obsolete highlight notifications, reentrant state
updates and stuck comparison after cancellation. It does not emulate Windows
rendering or replace the Windows mouse/keyboard and playback checks.
"""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'source/dsp_r128_normalizer.cpp').read_text(encoding='utf-8-sig')
body = source.split('// BEGIN compare hold input repair\n', 1)[1].split(
    '// END compare hold input repair', 1)[0]
prefix = r'''
#include <cassert>
#include <cstdint>
#include <iostream>
using UINT = unsigned; using UINT_PTR = std::uintptr_t; using DWORD_PTR = std::uintptr_t;
using WPARAM = std::uintptr_t; using LPARAM = std::intptr_t; using LRESULT = std::intptr_t;
#define CALLBACK
constexpr int FALSE = 0;
enum { WM_NCDESTROY=1, WM_LBUTTONDOWN, WM_LBUTTONDBLCLK, WM_LBUTTONUP,
    WM_MOUSEMOVE, WM_KEYDOWN, WM_KEYUP, BM_SETSTATE, WM_CAPTURECHANGED,
    WM_CANCELMODE, WM_KILLFOCUS, WM_ENABLE, WM_COMMAND, BM_GETSTATE };
enum { IDC_ORIGINAL_COMPARE=42, BN_HILITE=2, BN_UNHILITE=3, BN_CLICKED=0, BST_PUSHED=4 };
struct window { bool valid=true, enabled=true, pushed=false, installed=true;
    DWORD_PTR ref=0; window* parent=nullptr; };
using HWND = window*;
using proc = LRESULT(*)(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK original_compare_button_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
HWND capture=nullptr;
int comparison=0, notifications=0;
bool matched=true;
HWND GetParent(HWND w) { return w->parent; }
bool IsWindow(HWND w) { return w && w->valid; }
bool IsWindowEnabled(HWND w) { return w->enabled; }
bool GetWindowSubclass(HWND w, proc, UINT_PTR, DWORD_PTR* data) {
    *data=w->ref; return w->installed;
}
bool SetWindowSubclass(HWND w, proc, UINT_PTR, DWORD_PTR data) { w->ref=data; return true; }
bool RemoveWindowSubclass(HWND w, proc, UINT_PTR) { w->installed=false; return true; }
WPARAM MAKEWPARAM(unsigned low, unsigned high) { return low | (high << 16); }
HWND GetCapture() { return capture; }
LRESULT SendMessageW(HWND,UINT,WPARAM,LPARAM);
bool ReleaseCapture() {
    auto old=capture; capture=nullptr;
    if (old) SendMessageW(old,WM_CAPTURECHANGED,0,0);
    return true;
}
LRESULT DefSubclassProc(HWND w, UINT message, WPARAM wp, LPARAM lp) {
    // A standard button sends BN_CLICKED, not BN_HILITE. State updates may
    // reenter the installed subclass through BM_SETSTATE.
    switch (message) {
    case BM_GETSTATE: return w->pushed ? BST_PUSHED : 0;
    case BM_SETSTATE: w->pushed=wp!=0; break;
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        capture=w; SendMessageW(w,BM_SETSTATE,1,0); break;
    case WM_MOUSEMOVE:
        if (capture==w) SendMessageW(w,BM_SETSTATE,lp>0,0);
        break;
    case WM_LBUTTONUP:
        SendMessageW(w,BM_SETSTATE,0,0); ReleaseCapture();
        comparison=0; break; // native BN_CLICKED handler
    case WM_KEYDOWN:
        if (wp==32) SendMessageW(w,BM_SETSTATE,1,0);
        break;
    case WM_KEYUP:
        if (wp==32) SendMessageW(w,BM_SETSTATE,0,0);
        break;
    case WM_NCDESTROY: w->valid=false; break;
    }
    return 0;
}
LRESULT SendMessageW(HWND w, UINT m, WPARAM wp, LPARAM lp) {
    if (m==WM_COMMAND) {
        ++notifications;
        comparison=(wp>>16)==BN_HILITE ? (matched ? 2 : 1) : 0;
        return 0;
    }
    if (w->installed) return original_compare_button_proc(w,m,wp,lp,1,w->ref);
    return DefSubclassProc(w,m,wp,lp);
}
'''
suffix = r'''
int main() {
    window parent; window button; button.parent=&parent;
    auto send=[&](UINT m,WPARAM wp=0,LPARAM lp=0) { SendMessageW(&button,m,wp,lp); };
    send(WM_LBUTTONDOWN); assert(comparison==2 && notifications==1);
    send(WM_MOUSEMOVE,0,1); assert(comparison==2 && notifications==1);
    send(WM_MOUSEMOVE,0,-1); assert(comparison==0 && notifications==2);
    send(WM_MOUSEMOVE,0,1); assert(comparison==2 && notifications==3);
    send(WM_LBUTTONUP); assert(comparison==0 && notifications==4);
    matched=false;
    send(WM_KEYDOWN,32); assert(comparison==1);
    int count=notifications; send(WM_KEYDOWN,32); assert(notifications==count);
    send(WM_KEYUP,32); assert(comparison==0);
    for (UINT cancel : {WM_CAPTURECHANGED, WM_CANCELMODE, WM_KILLFOCUS, WM_ENABLE}) {
        send(WM_LBUTTONDOWN); assert(comparison==1);
        send(cancel); assert(comparison==0 && !button.pushed && capture==nullptr);
        count=notifications;
        send(WM_MOUSEMOVE,0,1); assert(comparison==0 && notifications==count);
    }
    button.enabled=false;
    send(WM_LBUTTONDOWN); assert(comparison==0);
    send(WM_CANCELMODE); button.enabled=true;
    send(WM_LBUTTONDOWN); assert(comparison==1);
    send(WM_NCDESTROY); assert(comparison==0 && !button.installed);
    std::cout << "PASS: native push state, mouse/Space, drag, reentry, cancellation and destruction\n";
}
'''
with tempfile.TemporaryDirectory(prefix='r128-compare-hold-') as directory:
    cpp = Path(directory) / 'compare_hold_test.cpp'
    binary = Path(directory) / 'compare_hold_test'
    cpp.write_text(prefix + body + suffix)
    subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                    str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)

"""Exercise actual owner/menu functions with Win32 API contract mocks.

Checks option routing, localization, default/save separation and topmost
independence. Windows itself must still verify minimize/restore, persistence
and live playback. The mock does not claim to emulate those OS behaviors.
"""
from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
ui = (root / 'source/r128_monitor_ui.h').read_text()
baseline = subprocess.check_output(
    ['git', 'show', 'a85c1976dea5a07cc290688b7466f8a5a51bc349:source/r128_monitor_ui.h'],
    cwd=root).decode()

assert 'cfg_bool cfg_minimize_with_foobar2000(guid_minimize_with_foobar2000, true);' in ui
assert 'cfg_bool cfg_topmost(guid_topmost, false);' in ui
assert ui[ui.index('void refresh('):ui.index('void save_position(')] == \
    baseline[baseline.index('void refresh('):baseline.index('void save_position(')]
assert ui[ui.index('void capture_layout('):].replace('            apply_owner(wnd);\n', '') == \
    baseline[baseline.index('void capture_layout('):]
assert 'IsWindowVisible' not in ui and 'IsIconic(core_api::get_main_window())' not in ui
assert 'SetTimer(wnd, kTimer, 100, nullptr);' in ui
guid_values = re.findall(r'static const GUID \w+ =\s*(\{.*?\});', ui, re.S)
assert len(guid_values) == 5 and len(set(guid_values)) == 5
assert all(value in ui for value in re.findall(
    r'static const GUID \w+ =\s*(\{.*?\});', baseline, re.S))

body = ui[ui.index('void apply_topmost('):ui.index('void capture_layout(')]
prefix = r'''
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
using UINT=unsigned; using UINT_PTR=std::uintptr_t;
using LONG_PTR=std::intptr_t; using LPARAM=std::intptr_t;
struct win { bool valid=true; win* owner=nullptr; bool topmost=false; };
using HWND=win*;
struct item { UINT flags; UINT_PTR id; std::wstring text; };
struct menu { std::vector<item> items; };
using HMENU=menu*;
struct POINT { long x,y; }; struct RECT { long left,top,right,bottom; };
struct saved_bool { bool value; bool get() const { return value; }
    void operator=(bool next) { value=next; } };
saved_bool cfg_topmost{false}, cfg_minimize_with_foobar2000{true};
win main_window, top_marker, normal_marker;
HWND const HWND_TOPMOST=&top_marker, HWND_NOTOPMOST=&normal_marker;
namespace core_api { HWND get_main_window() { return &main_window; }
    int get_my_instance() { return 0; } }
enum { SWP_NOMOVE=1, SWP_NOSIZE=2, SWP_NOACTIVATE=4, GWLP_HWNDPARENT=8,
    MF_STRING=16, MF_CHECKED=32, MF_UNCHECKED=0, MF_SEPARATOR=64,
    TPM_RETURNCMD=128, TPM_RIGHTBUTTON=256, IDD_R128_GLOSSARY=400 };
constexpr int glossary_dialog_proc=123, kMonitorGlossaryIndex=30;
bool english=false, fail_menu=false;
unsigned choice=0, owner_writes=0, help_calls=0;
std::vector<item> last_menu;
const wchar_t* ui_text(const wchar_t* ja,const wchar_t* en) { return english?en:ja; }
bool IsWindow(HWND w) { return w && w->valid; }
LONG_PTR SetWindowLongPtrW(HWND w,int index,LONG_PTR p) {
    assert(index==GWLP_HWNDPARENT); ++owner_writes;
    auto previous=reinterpret_cast<LONG_PTR>(w->owner);
    w->owner=reinterpret_cast<HWND>(p); return previous;
}
bool SetWindowPos(HWND w,HWND z,int,int,int,int,UINT flags) {
    assert(flags==(SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE));
    w->topmost=z==HWND_TOPMOST; return true;
}
HMENU CreatePopupMenu() { return fail_menu?nullptr:new menu; }
bool AppendMenuW(HMENU m,UINT flags,UINT_PTR id,const wchar_t* text) {
    m->items.push_back({flags,id,text?text:L""}); return true;
}
unsigned LOWORD(LPARAM p) { return static_cast<unsigned>(p)&0xffff; }
unsigned HIWORD(LPARAM p) { return (static_cast<std::uintptr_t>(p)>>16)&0xffff; }
bool GetWindowRect(HWND,RECT* r) { *r={0,0,100,100}; return true; }
UINT TrackPopupMenu(HMENU m,UINT,int,int,int,HWND,void*) {
    last_menu=m->items; return choice;
}
bool DestroyMenu(HMENU m) { delete m; return true; }
int MAKEINTRESOURCEW(int id) { return id; }
int DialogBoxParamW(int,int id,HWND,int,LPARAM initial) {
    assert(id==IDD_R128_GLOSSARY && initial==kMonitorGlossaryIndex);
    ++help_calls; return 0;
}
'''
suffix = r'''
int main() {
    win monitor;
    apply_owner(&monitor); assert(monitor.owner==&main_window);
    auto count=owner_writes; apply_owner(nullptr); assert(owner_writes==count);
    win invalid; invalid.valid=false;
    apply_owner(&invalid); assert(owner_writes==count);
    for (bool language : {false,true}) {
        english=language;
        for (bool top : {false,true}) {
            cfg_topmost=top;
            cfg_minimize_with_foobar2000=true;
            apply_owner(&monitor); apply_topmost(&monitor);
            choice=0; show_context_menu(&monitor,-1);
            assert(last_menu.size()==4 && last_menu[0].id==1 &&
                last_menu[1].id==2 && last_menu[3].id==3);
            assert(last_menu[1].flags & MF_CHECKED);
            assert(last_menu[1].text==(language?L"Minimize with foobar2000":L"foobar2000と一緒に最小化"));
            choice=2; show_context_menu(&monitor,0);
            assert(!cfg_minimize_with_foobar2000.get() && monitor.owner==nullptr);
            assert(cfg_topmost.get()==top && monitor.topmost==top);
            choice=0; show_context_menu(&monitor,0);
            assert(!(last_menu[1].flags & MF_CHECKED));
            choice=1; show_context_menu(&monitor,0);
            assert(cfg_topmost.get()!=top && monitor.topmost!=top);
            assert(!cfg_minimize_with_foobar2000.get() && monitor.owner==nullptr);
            choice=2; show_context_menu(&monitor,0);
            assert(cfg_minimize_with_foobar2000.get() && monitor.owner==&main_window);
            assert(monitor.topmost!=top);
            choice=3; show_context_menu(&monitor,0);
            assert(cfg_minimize_with_foobar2000.get());
        }
    }
    assert(help_calls==4);
    auto saved_minimize=cfg_minimize_with_foobar2000.get();
    auto saved_top=cfg_topmost.get();
    fail_menu=true; choice=2; show_context_menu(&monitor,0);
    assert(cfg_minimize_with_foobar2000.get()==saved_minimize && cfg_topmost.get()==saved_top);
    std::cout << "PASS: actual owner/menu code, ON/OFF, topmost independence, JP/EN, cancel/help paths\n";
}
'''
with tempfile.TemporaryDirectory(prefix='r128-monitor-minimize-') as directory:
    cpp = Path(directory) / 'minimize.cpp'
    binary = Path(directory) / 'minimize'
    cpp.write_text(prefix + body + suffix)
    subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                    str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('PASS: default ON, distinct GUID, unchanged refresh/timer, close/open/startup lifecycle')

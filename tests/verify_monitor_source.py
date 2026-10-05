"""Static regression guard, not a Windows compilation or audio runtime test."""
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[1]
baseline = subprocess.check_output(
    ["git", "show", "v1.11.0:source/dsp_r128_normalizer.cpp"],
    cwd=root).decode("utf-8-sig").replace("\r\n", "\n")
current = (root / "source/dsp_r128_normalizer.cpp").read_text(
    encoding="utf-8-sig").replace("\r\n", "\n")

# Remove only explicit monitor instrumentation, then compare all original code.
restored = current.replace('#include "r128_monitor_state.h"\n', '')
restored = restored.replace('\n#include "r128_monitor_ui.h"\n', '\n')
start = restored.index('    dsp_r128_normalizer(const dsp_preset& preset, unsigned flags)')
end = restored.index('    explicit dsp_r128_normalizer(', start)
restored = restored[:start] + restored[end:]
restored = re.sub(r'^        m_monitor\.(?:begin_chunk|invalidate)\(\);\n',
                  '', restored, flags=re.M)
restored = restored.replace('            m_monitor.observe_output_peak(output_frame_true_peak);\n', '')
start = restored.index('        r128_monitor::snapshot monitor_value;')
end = restored.index('        m_monitor.publish(monitor_value);\n', start)
restored = restored[:start] + restored[end + len('        m_monitor.publish(monitor_value);\n'):]
restored = restored.replace('    r128_monitor::publisher m_monitor;\n', '')
start = restored.index('// Keep the legacy popup and preset behavior.')
end = restored.index('\nnamespace {', start)
restored = restored[:start] + 'static dsp_factory_t<dsp_r128_normalizer> g_dsp_r128_normalizer_factory;\n' + restored[end:]
restored = restored.replace('1.12.0\\r\\n', '1.11.0\\r\\n')
for language in ('JA', 'EN'):
    restored, count = re.subn(
        r',\n    // BEGIN R128 monitor glossary ' + language +
        r'\n.*?    // END R128 monitor glossary ' + language + r'\n',
        '\n', restored, flags=re.S)
    assert count == 1
restored, count = re.subn(
    r'\n// BEGIN R128 monitor glossary index\n.*?// END R128 monitor glossary index\n',
    '', restored, flags=re.S)
assert count == 1
restored, count = re.subn(
    r'        // BEGIN R128 monitor glossary selection\n.*?'
    r'        // END R128 monitor glossary selection\n\n', '', restored, flags=re.S)
assert count == 1
restored = restored.replace('    LPARAM initial_selection\n', '    LPARAM\n', 1)
restored = restored.replace('            LB_SETCURSEL,\n            selection,',
                            '            LB_SETCURSEL,\n            0,', 1)
for block, indent in [('repair', ''), ('install', '        '), ('deactivate', '    ')]:
    restored, count = re.subn(
        re.escape(indent + '// BEGIN compare hold input ' + block) + r'\n.*?' +
        re.escape(indent + '// END compare hold input ' + block) + r'\n' +
        ('\n' if block == 'repair' else ''), '', restored, flags=re.S)
    assert count == 1
assert restored.rstrip() == baseline.rstrip(), 'Unexpected change to original v1.11.0 implementation'
print('PASS: original CPP identical after removing named monitor hooks, help additions and version text')

resource = (root / 'source/resource.h').read_text(encoding='utf-8-sig')
ids = re.findall(r'^#define\s+(\w+)\s+(\d+)\s*$', resource, re.M)
assert len({name for name, _ in ids}) == len(ids), 'Duplicate resource name'
assert len({number for _, number in ids}) == len(ids), 'Duplicate resource value'
assert 'constexpr t_uint32 kPresetVersion = 7;' in current
ui = (root / 'source/r128_monitor_ui.h').read_text()
assert 'SetTimer(wnd, kTimer, 100, nullptr)' in ui
assert 'dsp_entry::flag_conversion' in current
assert 'dsp_entry::flag_playback' in current
assert 'std::mutex' not in (root / 'source/r128_monitor_state.h').read_text()
script = (root / 'scripts/build_v1120.ps1').read_text(encoding='utf-8-sig')
for name in ['stdafx.h', 'main.cpp', 'resource.h', 'foo_r128_normalizer.rc',
             'dsp_r128_normalizer.cpp', 'r128_monitor_state.h', 'r128_monitor_ui.h',
             'r128_monitor_meter.h']:
    assert (root / 'source' / name).is_file()
    assert f'"{name}"' in script
rc = (root / 'source/foo_r128_normalizer.rc').read_text(encoding='utf-8-sig')
monitor_rc = rc[rc.index('IDD_R128_MONITOR DIALOGEX'):rc.index('IDD_R128_AUTO_HISTORY DIALOGEX')]
assert monitor_rc.count('"msctls_progress32"') == 7
assert 'IDC_MONITOR_TOPMOST' not in monitor_rc
assert 'WS_EX_TOOLWINDOW' in monitor_rc and 'WS_THICKFRAME' not in monitor_rc
assert 'DIALOGEX 0, 0, 156, 163' in monitor_rc, 'Approved compact width changed'
assert 'FONT 9, "Yu Gothic UI", 400, 0, 0x80' in monitor_rc
rectangles = re.findall(r',\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)', monitor_rc)
assert len(rectangles) == 23
for values in rectangles:
    x, y, width, height = map(int, values)
    assert width > 0 and height > 0
    assert x + width <= 156 and y + height <= 163, 'Control outside compact window'
for index in range(7):
    label, number, bar = [tuple(map(int, values)) for values in rectangles[index*3:index*3+3]]
    assert label[0]+label[2] < number[0] and number[0]+number[2] < bar[0], 'Row overlap'
    assert (bar[0],bar[2],bar[3]) == (99,49,8), 'Sonic-aligned bar dimensions changed'
assert 'case WM_CONTEXTMENU:' in ui and 'Always on Top' in ui
assert 'L"Active"' in ui and 'L"Waiting"' in ui
assert 'L"一時停止中"' in ui
print('PASS: resource IDs, format v7, timer, playback flags, eight inputs, seven bars, context menu')

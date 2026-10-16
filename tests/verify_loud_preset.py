"""Exercise actual preset/history/storage code; not a Windows UI or audio test."""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BASE = 'd42f85e3c647ea34e11a59437507b882568cb20a'


def baseline_text(path):
    return subprocess.check_output(['git', 'show', BASE + ':' + path],
                                   cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')


def span(source, name, kind='function'):
    if kind == 'function':
        pattern = r'(?m)^[\w:<>*& ]+\b' + re.escape(name) + r'\s*\('
    else:
        pattern = r'(?m)^[^\n]*\b' + re.escape(name) + r'\b[^\n]*'
    for match in re.finditer(pattern, source):
        brace = source.find('{', match.start())
        semi = source.find(';', match.start())
        if semi >= 0 and semi < brace:
            continue  # A forward declaration, not the function body.
        masked = re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                        lambda m: ' ' * len(m.group()), source[brace:], flags=re.S)
        level = 0
        for index, char in enumerate(masked):
            if char == '{':
                level += 1
            elif char == '}':
                level -= 1
                if level == 0:
                    end = brace + index + 1
                    if kind != 'function':
                        assert source[end] == ';'
                        end += 1
                    return match.start(), end
    raise AssertionError('Definition not found: ' + name)


def definition(source, name, kind='function'):
    start, end = span(source, name, kind)
    return source[start:end]


def restore_loud_changes(current, baseline):
    """Only approved UI/profile regions differ; all DSP code outside them must match."""
    s = current.replace('1.14.0', '1.13.0')
    s = s.replace('std::wstring user_preset_display_name(const std::wstring& name);\n\n', '')
    for name in ['loud_profile', 'user_preset_name_matches_loud',
                 'user_preset_name_is_reserved_for_creation', 'user_preset_display_name']:
        start, end = span(s, name)
        s = s[:start] + s[end + 2:]
    s = s.replace('// Newly reserved names remain valid in existing storage and backup files.\n'
                  "// Keep user_preset_name_is_reserved's original fourteen-name validation there.\n", '')
    # The sole new dispatcher route must reuse the existing Apply/preset path.
    route = ('        case IDC_PROFILE_LOUD:\n'
             '            select_profile_in_dialog(wnd, context, loud_profile());\n'
             '            return TRUE;\n\n')
    assert s.count(route) == 1
    s = s.replace(route, '')
    for name in ['detect_recognized_profile', 'recognized_profile_name',
                 'update_profile_indicator', 'valid_recognized_profile_id',
                 'preset_name_dialog_proc', 'refresh_user_preset_combo',
                 'suggested_import_preset_name', 'english_context_help_description']:
        start, end = span(s, name)
        s = s[:start] + definition(baseline, name) + s[end:]
    for name in ['recognized_profile', 'kPrimaryUiText', 'kPresetTooltips',
                 'kGlossaryEntries', 'kGlossaryEntriesEnglish', 'kContextHelpEntries']:
        start, end = span(s, name, 'block')
        s = s[:start] + definition(baseline, name, 'block') + s[end:]
    assert s == baseline, 'Change outside approved preset/UI regions'
    return s


def run():
    source = (ROOT/'source/dsp_r128_normalizer.cpp').read_text(encoding='utf-8-sig')
    old = baseline_text('source/dsp_r128_normalizer.cpp')
    restore_loud_changes(source, old)
    protected = ['default_settings', 'standard_profile', 'streaming_profile',
                 'broadcast_profile', 'night_profile', 'modern_profile',
                 'adaptive_profile', 'three_band_profile', 'make_preset', 'parse_preset',
                 'user_preset_name_is_reserved', 'append_user_preset_record',
                 'save_user_presets', 'serialize_user_preset_file',
                 'parse_user_preset_record', 'parse_user_preset_file', 'ensure_user_presets_loaded']
    for name in protected:
        assert definition(source, name) == definition(old, name), name + ' changed'
    for name in ['r128_monitor_meter.h', 'r128_monitor_state.h', 'r128_monitor_ui.h', 'stdafx.h']:
        assert (ROOT/'source'/name).read_bytes() == subprocess.check_output(
            ['git', 'show', BASE+':source/'+name], cwd=ROOT), name
    old_ids = dict(re.findall(r'^#define\s+(\w+)\s+(\d+)\s*$',
                            baseline_text('source/resource.h'), re.M))
    ids = dict(re.findall(r'^#define\s+(\w+)\s+(\d+)\s*$',
                        (ROOT/'source/resource.h').read_text(encoding='utf-8-sig'), re.M))
    assert all(ids.get(name) == value for name, value in old_ids.items())
    assert set(ids) - set(old_ids) == {'IDC_PROFILE_LOUD'}
    rc = (ROOT/'source/foo_r128_normalizer.rc').read_text(encoding='utf-8-sig')
    rows = re.findall(r'PUSHBUTTON\s+"([^"]+)",\s*(IDC_PROFILE_\w+),\s*'
                      r'(\d+),\s*(\d+),\s*(\d+),\s*(\d+)', rc)
    assert len(rows) == 8
    for y, count in [(214, 5), (234, 3)]:
        row = sorted((int(x), int(w)) for _, _, x, yy, w, _ in rows if int(yy) == y)
        assert len(row) == count and all(x+w <= 506 for x,w in row)
        assert all(x+w < row[i+1][0] for i,(x,w) in enumerate(row[:-1]))
    for name in ['kGlossaryEntries', 'kGlossaryEntriesEnglish']:
        entries = definition(source, name, 'block')
        assert entries.count('\n    {') == definition(source,
            'kGlossaryEntriesEnglish' if name == 'kGlossaryEntries' else 'kGlossaryEntries',
            'block').count('\n    {')
        assert 'Loud -9' in entries or 'ラウド -9' in entries

    # SDK/Win32 adapters only: preset values, comparison, serialization and parsing
    # below are extracted verbatim from the product, rather than reimplemented.
    prefix = r'''
#include <algorithm>
#include <cassert>
#include <cmath>
#include <codecvt>
#include <cstdint>
#include <cwchar>
#include <cwctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <vector>
using t_size = std::size_t;
constexpr t_size kMaximumUserPresets = 100, kMaximumUserPresetNameCharacters = 64;
constexpr const char* kUserPresetFileMagic = "R128PRESET1";
constexpr int CSTR_EQUAL=2;
int CompareStringOrdinal(const wchar_t* a,int na,const wchar_t* b,int nb,bool) {
    if (na!=nb) return 1;
    for (int i=0;i<na;++i) if(std::towlower(a[i])!=std::towlower(b[i])) return 1;
    return CSTR_EQUAL;
}
constexpr bool TRUE=true;
constexpr int IDOK=1, IDCANCEL=2;
bool english=false;
bool ui_uses_english() { return english; }
const wchar_t* ui_text(const wchar_t* ja,const wchar_t* en) { return english?en:ja; }
std::string wide_to_utf8(const std::wstring& s) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(s);
}
std::wstring utf8_to_wide(const char* s) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(s);
}
struct config_string {
    std::string value;
    const char* get_ptr() const { return value.c_str(); }
    void operator=(const char* s) { value=s; }
} g_cfg_user_presets;
using HWND=void*; using WPARAM=std::uintptr_t; using LPARAM=std::intptr_t;
using LRESULT=std::intptr_t;
constexpr LRESULT CB_ERR=-1;
enum { CB_GETCURSEL=1,CB_GETITEMDATA,CB_RESETCONTENT,CB_ADDSTRING,CB_SETITEMDATA,CB_SETCURSEL };
std::vector<std::wstring> combo_labels;
std::vector<LPARAM> combo_indices;
int combo_selection=-1;
HWND GetDlgItem(HWND,int) { return reinterpret_cast<HWND>(1); }
LRESULT SendMessageW(HWND,int m,WPARAM w,LPARAM l) {
    switch(m) {
    case CB_GETCURSEL: return combo_selection;
    case CB_GETITEMDATA: return w<combo_indices.size()?combo_indices[w]:CB_ERR;
    case CB_RESETCONTENT: combo_labels.clear(); combo_indices.clear(); combo_selection=-1; return 0;
    case CB_ADDSTRING:
        combo_labels.emplace_back(reinterpret_cast<const wchar_t*>(l));
        combo_indices.push_back(-1); return combo_labels.size()-1;
    case CB_SETITEMDATA: combo_indices.at(w)=l; return 0;
    case CB_SETCURSEL: combo_selection=static_cast<int>(w); return combo_selection;
    }
    return CB_ERR;
}
LRESULT SendDlgItemMessageW(HWND h,int id,int m,WPARAM w,LPARAM l) {
    return SendMessageW(GetDlgItem(h,id),m,w,l);
}
void update_user_preset_button_state(HWND) {}
std::wstring indicator;
void SetDlgItemTextW(HWND,int,const wchar_t* value) { indicator=value; }
template<std::size_t N,typename... Args>
int swprintf_s(wchar_t (&buffer)[N],const wchar_t* format,Args... args) {
    // Windows wide printf uses %s for wchar_t strings; POSIX uses %ls.
    std::wstring translated;
    for(std::size_t i=0;format[i];++i) {
        if(format[i]=='%' && format[i+1]=='s') { translated+=L"%ls"; ++i; }
        else translated+=format[i];
    }
    return std::swprintf(buffer,N,translated.c_str(),args...);
}
'''
    prefix += '\n#include "' + str(ROOT/'source/resource.h') + '"\n'
    types = '\n'.join(definition(source, name, 'block') for name in
                      ['r128_settings', 'user_preset_entry', 'recognized_profile',
                       'localized_control_text', 'context_help_entry',
                       'kPrimaryUiText', 'kContextHelpEntries'])
    globals_ = '\nstd::vector<user_preset_entry> g_user_presets;\nbool g_user_presets_loaded=false;\n'
    names = ['default_settings', 'standard_profile', 'streaming_profile', 'loud_profile',
             'broadcast_profile', 'night_profile', 'modern_profile', 'adaptive_profile',
             'three_band_profile', 'settings_float_equal', 'settings_equal',
             'detect_recognized_profile', 'recognized_profile_name',
             'valid_recognized_profile_id', 'history_profile_to_text', 'hex_encode',
             'hex_digit_value', 'hex_decode', 'split_history_fields',
             'trim_user_preset_name', 'user_preset_name_is_valid', 'user_preset_names_equal',
             'user_preset_name_is_reserved', 'user_preset_name_matches_loud',
             'user_preset_name_is_reserved_for_creation', 'user_preset_display_name',
             'user_preset_settings_are_valid', 'append_user_preset_record',
             'save_user_presets', 'serialize_user_preset_file', 'parse_strict_float',
             'parse_strict_boolean', 'parse_user_preset_record', 'parse_user_preset_file',
             'ensure_user_presets_loaded', 'update_profile_indicator',
             'selected_user_preset_index', 'refresh_user_preset_combo',
             'find_context_help', 'english_control_title', 'english_context_help_description',
             'context_help_title', 'context_help_description']
    body = '\n\n'.join(definition(source, name) for name in names)
    predicate = re.search(
        r'if \((user_preset_name_is_reserved_for_creation\(trimmed\).*?)\) \{',
        source, re.S).group(1)
    body += ('\nbool actual_name_dialog_rejects(const std::wstring& trimmed, '
             'const std::wstring& initial, bool present=true) {\n'
             '    struct initial_context { std::wstring initial_name; } data{initial};\n'
             '    auto* context=present?&data:nullptr;\n'
             '    return ' + predicate + ';\n}\n')
    suffix = r'''
int main() {
    static_assert(static_cast<int>(recognized_profile::standard)==0);
    static_assert(static_cast<int>(recognized_profile::streaming)==1);
    static_assert(static_cast<int>(recognized_profile::broadcast)==2);
    static_assert(static_cast<int>(recognized_profile::night)==3);
    static_assert(static_cast<int>(recognized_profile::modern)==4);
    static_assert(static_cast<int>(recognized_profile::adaptive)==5);
    static_assert(static_cast<int>(recognized_profile::three_band)==6);
    static_assert(static_cast<int>(recognized_profile::custom)==7);
    static_assert(static_cast<int>(recognized_profile::loud)==8);
    auto loud=loud_profile(), power=streaming_profile();
    assert(loud.target_lufs==-9 && loud.true_peak_limit_dbtp==-1);
    assert(!loud.enable_modern_boost && !loud.enable_adaptive_master && !loud.enable_three_band_master);
    assert(loud.enable_peak_guard && loud.enable_silence_guard && loud.enable_gain_lock);
    auto only_target=loud; only_target.target_lufs=power.target_lufs;
    assert(settings_equal(only_target,power));
    assert(detect_recognized_profile(loud)==recognized_profile::loud);
    auto edited=loud; edited.max_boost_db+=1;
    assert(detect_recognized_profile(edited)==recognized_profile::custom);
    const r128_settings profiles[]={standard_profile(),streaming_profile(),broadcast_profile(),
        night_profile(),modern_profile(),adaptive_profile(),three_band_profile()};
    for(int i=0;i<7;++i) assert(static_cast<int>(detect_recognized_profile(profiles[i]))==i);
    assert(!valid_recognized_profile_id(-1) && !valid_recognized_profile_id(9));
    for (bool en : {false,true}) {
        english=en;
        assert(std::wstring(history_profile_to_text(7))==(en?L"Custom Settings":L"カスタム設定"));
        assert(std::wstring(history_profile_to_text(8))==(en?L"Loud -9":L"ラウド -9"));
        for (const auto& name : {std::wstring(L"ラウド -9"),std::wstring(L"Loud -9"),std::wstring(L"lOuD -9")}) {
            assert(!user_preset_name_is_reserved(name)); // storage/import compatibility
            assert(user_preset_name_is_reserved_for_creation(name));
            assert(user_preset_display_name(name)==name+(en?L" (User)":L"（ユーザー）"));
            user_preset_entry legacy{name, modern_profile()}; // not forced to built-in values
            g_user_presets={legacy}; save_user_presets();
            const auto old_storage=g_cfg_user_presets.value;
            const auto old_backup=serialize_user_preset_file(g_user_presets);
            g_user_presets_loaded=false; ensure_user_presets_loaded();
            assert(g_user_presets.size()==1 && g_user_presets[0].name==name);
            assert(settings_equal(g_user_presets[0].settings,legacy.settings));
            save_user_presets(); assert(g_cfg_user_presets.value==old_storage);
            std::vector<user_preset_entry> imported;
            assert(parse_user_preset_file(old_backup,imported) && imported.size()==1);
            assert(imported[0].name==name && settings_equal(imported[0].settings,legacy.settings));
            assert(serialize_user_preset_file(imported)==old_backup);
            auto broken=legacy; broken.settings.target_lufs=0;
            assert(!parse_user_preset_file(serialize_user_preset_file({broken}),imported));
            assert(g_user_presets[0].name==name); // parser does not mutate saved presets
        }
        assert(user_preset_display_name(L"My preset")==
            std::wstring(L"My preset")+(en?L" (User)":L"（ユーザー）"));
        assert(user_preset_display_name(L"ラウド -9")!=user_preset_display_name(L"ラウド -9（ユーザー）"));
        assert(actual_name_dialog_rejects(L"Loud -9",L""));
        assert(actual_name_dialog_rejects(L"ラウド -9",L""));
        assert(actual_name_dialog_rejects(L"Loud -9",L"other"));
        assert(!actual_name_dialog_rejects(L"Loud -9",L"Loud -9"));
        assert(!actual_name_dialog_rejects(L"loud -9",L"LOUD -9"));
        assert(actual_name_dialog_rejects(L"Loud -9",L"Loud -9",false));
        assert(actual_name_dialog_rejects(L"Natural -18",L"Natural -18"));
        assert(!actual_name_dialog_rejects(L"My preset",L""));
        const auto window=reinterpret_cast<HWND>(1);
        update_profile_indicator(window,loud,false);
        assert(indicator==(en?L"Current: Loud -9":L"現在: ラウド -9"));
        std::wstring user_name=L"ラウド -9";
        update_profile_indicator(window,loud,true,&user_name);
        assert(indicator.find(en?L" (User)":L"（ユーザー）")!=std::wstring::npos);
        const auto* help=find_context_help(IDC_PROFILE_LOUD);
        assert(help && std::wstring(context_help_title(*help))==(en?L"Loud -9":L"ラウド -9"));
        assert(std::wstring(context_help_description(*help)).find(L"-9 LUFS")!=std::wstring::npos);
        g_user_presets={{L"ラウド -9",loud},{L"ラウド -9（ユーザー）",power}};
        g_user_presets_loaded=true;
        refresh_user_preset_combo(window,L"ラウド -9（ユーザー）");
        assert(selected_user_preset_index(window)==1 && combo_labels.size()==2);
        assert(combo_labels[0]!=combo_labels[1]);
        assert(combo_labels[0]==user_preset_display_name(L"ラウド -9"));
        refresh_user_preset_combo(window,L"ラウド -9");
        assert(selected_user_preset_index(window)==0);
    }
    std::vector<user_preset_entry> imported;
    assert(!parse_user_preset_file(serialize_user_preset_file({{L"Power Boost -14",power}}),imported));
    assert(!parse_user_preset_file(serialize_user_preset_file({{L"Loud -9",loud},{L"LOUD -9",power}}),imported));
    assert(!parse_user_preset_file("BAD\n1\n",imported));
    assert(!parse_user_preset_file(serialize_user_preset_file({{L"My preset",loud}})+"extra\n",imported));
    std::cout << "PASS: actual 8 preset definitions, fixed old history IDs, JP/EN recognition, "
                 "legacy same-name storage/backup round trips and invalid-file rejection\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='r128-loud-preset-') as directory:
        cpp = Path(directory)/'preset.cpp'
        binary = Path(directory)/'preset'
        cpp.write_text(prefix+types+globals_+body+suffix)
        subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True)
    print('PASS: unchanged audio/config/GUIDs/old presets/monitor; eight unique buttons in two rows')


if __name__ == '__main__':
    run()

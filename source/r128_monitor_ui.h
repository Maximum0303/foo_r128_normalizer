#pragma once

#include "r128_monitor_meter.h"

// Included only at the end of dsp_r128_normalizer.cpp, on the UI side.
namespace r128_monitor {

static const GUID guid_open =
{0x6b40fe4d,0x9f8a,0x45c9,{0xb8,0x9f,0x62,0xa6,0x37,0xa5,0xe3,0x7b}};
static const GUID guid_visible =
{0xb25b9ef0,0x6316,0x4bbd,{0x8c,0x91,0x55,0x70,0x6d,0xad,0xfa,0x4e}};
static const GUID guid_topmost =
{0x9fb00c64,0x5a41,0x405d,{0x94,0x40,0xa3,0xbe,0xbd,0xf0,0x9a,0x91}};
static const GUID guid_position =
{0xc39050ca,0x89a1,0x49bb,{0xa9,0xb2,0x6a,0x6d,0xc7,0xbe,0xca,0xf9}};

struct saved_position { int version = 1; int x = 0; int y = 0; unsigned valid = 0; };
cfg_bool cfg_visible(guid_visible, false);
cfg_bool cfg_topmost(guid_topmost, false);
cfg_struct_t<saved_position> cfg_position(guid_position, saved_position{});
HWND window = nullptr;
bool shutting_down = false;
constexpr UINT_PTR kTimer = 7;

struct ui_context {
    fb2k::CCoreDarkModeHooks dark_mode;
    bool english = false;
    bool language_ready = false;
    bool ready = false;
    bool registered = false;
    UINT initial_dpi = 96;
    SIZE initial_client = {};
    std::vector<std::pair<HWND, RECT>> controls;
    HFONT scaled_font = nullptr;
    ULONGLONG last_chain_poll = 0;
    t_size chain_count = 0;
    bool chain_known = false;
    int transport_phase = -1;
};

UINT window_dpi(HWND wnd) {
    using get_dpi_fn = UINT (WINAPI*)(HWND);
    static auto get_dpi = reinterpret_cast<get_dpi_fn>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    if (get_dpi != nullptr) return std::max<UINT>(96, get_dpi(wnd));
    HDC dc = GetDC(wnd);
    const UINT dpi = dc != nullptr ? static_cast<UINT>(GetDeviceCaps(dc, LOGPIXELSX)) : 96;
    if (dc != nullptr) ReleaseDC(wnd, dc);
    return std::max<UINT>(96, dpi);
}

void text_if_changed(HWND wnd, int id, const wchar_t* value) {
    HWND target = id == 0 ? wnd : GetDlgItem(wnd, id);
    wchar_t previous[256] = {};
    GetWindowTextW(target, previous, static_cast<int>(std::size(previous)));
    if (std::wcscmp(previous, value) != 0) SetWindowTextW(target, value);
}

void apply_language(HWND wnd, ui_context& context) {
    const bool english = ui_uses_english();
    if (context.language_ready && context.english == english) return;
    context.english = english;
    context.language_ready = true;
    text_if_changed(wnd, 0, ui_text(L"R128 補正モニター", L"R128 Monitor"));
    const wchar_t* japanese[] = {L"入力 3秒", L"ゲイン",
        L"出力 3秒", L"True Peak", L"リミッター",
        L"自動保護", L"適応強度"};
    const wchar_t* english_labels[] = {L"Input 3 s", L"Gain",
        L"Output 3 s", L"True Peak", L"Limiter",
        L"Protect.", L"Adaptive"};
    for (int index = 0; index < 7; ++index) {
        text_if_changed(wnd, IDC_MONITOR_LABEL_INPUT + index,
            english ? english_labels[index] : japanese[index]);
    }
    text_if_changed(wnd, IDC_MONITOR_NOTE,
        ui_text(L"このDSPの出力／TPは近似", L"This DSP / TP approx."));
}

void set_bar(HWND wnd, int id, int position) {
    const HWND bar = GetDlgItem(wnd, id);
    if (SendMessageW(bar, PBM_GETPOS, 0, 0) != position)
        SendMessageW(bar, PBM_SETPOS, position, 0);
}

void clear_values(HWND wnd, const wchar_t* state) {
    for (int index = 0; index < 7; ++index) {
        text_if_changed(wnd, IDC_MONITOR_INPUT + index, L"—");
        set_bar(wnd, IDC_MONITOR_BAR_INPUT + index, 0);
    }
    text_if_changed(wnd, IDC_MONITOR_STATUS, state);
}

void set_number(HWND wnd, int id, double value, const wchar_t* unit,
    bool signed_value = false, bool measurement = false) {
    if (!std::isfinite(value) || (measurement && value <= -190.0)) {
        text_if_changed(wnd, id, L"—");
        return;
    }
    wchar_t buffer[80] = {};
    swprintf_s(buffer, signed_value ? L"%+.1f %s" : L"%.1f %s", value, unit);
    text_if_changed(wnd, id, buffer);
}

void refresh(HWND wnd, ui_context& context) {
    apply_language(wnd, context);
    if (!core_version_info_v2::get()->test_version(2, 1, 0, 0)) {
        clear_values(wnd, ui_text(L"foobar2000 2.1以降が必要",
            L"Requires foobar2000 2.1+"));
        return;
    }
    const auto now = GetTickCount64();
    if (!context.chain_known || now - context.last_chain_poll >= 500) {
        context.chain_known = false;
        try {
            dsp_chain_config_impl chain;
            dsp_config_manager::get()->get_core_settings(chain);
            context.chain_count = 0;
            for (t_size index = 0; index < chain.get_count(); ++index)
                if (chain.get_item(index).get_owner() == guid_r128_normalizer)
                    ++context.chain_count;
            context.chain_known = true;
        } catch (...) {
            // Never let an unavailable service escape through a Win32 callback.
        }
        context.last_chain_poll = now;
    }
    if (!context.chain_known) {
        clear_values(wnd, ui_text(L"DSP確認中", L"Checking DSP chain"));
        return;
    }
    if (context.chain_count == 0) {
        context.transport_phase = -1;
        clear_values(wnd, ui_text(L"DSP未登録", L"DSP not in playback chain"));
        return;
    }
    if (context.chain_count != 1) {
        context.transport_phase = -1;
        clear_values(wnd, ui_text(L"DSP複数登録", L"Multiple DSP instances in chain"));
        return;
    }
    auto playback = playback_control::get();
    if (!playback->is_playing()) {
        context.transport_phase = 0;
        clear_values(wnd, ui_text(L"待機中", L"Waiting"));
        return;
    }
    if (playback->is_paused()) {
        context.transport_phase = 2;
        clear_values(wnd, ui_text(L"一時停止中", L"Paused"));
        return;
    }
    const int phase = g_original_compare_request.load(std::memory_order_relaxed) != 0 ? 3 : 1;
    if (context.transport_phase != phase) {
        context.transport_phase = phase;
        display_epoch.fetch_add(1);
    }
    const auto epoch = display_epoch.load();
    snapshot value;
    telemetry_slot* selected = nullptr;
    unsigned active_count = 0;
    for (auto& slot : slots) {
        snapshot candidate;
        if (read_slot(slot, candidate) && candidate.epoch == epoch && now >= candidate.tick &&
            now - candidate.tick < 1500) {
            ++active_count;
            value = candidate;
            selected = &slot;
        }
    }
    if (active_count != 1) {
        clear_values(wnd, active_count > 1
            ? ui_text(L"DSP表示対象を確認中", L"Checking DSP source")
            : ui_text(L"測定待機／更新なし", L"Waiting for data"));
        return;
    }
    // Acknowledge the exact version read, never an unseen newer publication.
    selected->peak_ack.store(value.sequence);
    if (value.comparing || g_original_compare_request.load(std::memory_order_relaxed) != 0) {
        clear_values(wnd, ui_text(L"原音比較中", L"Comparing original"));
        set_number(wnd, IDC_MONITOR_INPUT, value.input, L"LUFS", false, true);
        set_bar(wnd, IDC_MONITOR_BAR_INPUT, meter_position(meter_kind::loudness, value.input));
        return;
    }
    set_number(wnd, IDC_MONITOR_INPUT, value.input, L"LUFS", false, true);
    set_number(wnd, IDC_MONITOR_OUTPUT, value.output, L"LUFS", false, true);
    set_number(wnd, IDC_MONITOR_GAIN, value.gain, L"dB", true);
    set_number(wnd, IDC_MONITOR_PEAK, value.peak, L"dBTP", false, true);
    set_number(wnd, IDC_MONITOR_LIMITER, value.limiter, L"dB");
    set_number(wnd, IDC_MONITOR_SAFETY, value.safety, L"dB");
    if (value.adaptive) set_number(wnd, IDC_MONITOR_STRENGTH, value.strength, L"%");
    else text_if_changed(wnd, IDC_MONITOR_STRENGTH, ui_text(L"オフ", L"OFF"));
    const double numbers[] = {value.input, value.gain, value.output, value.peak,
        value.limiter, value.safety, value.adaptive ? value.strength : 0.0};
    const meter_kind kinds[] = {meter_kind::loudness, meter_kind::gain,
        meter_kind::loudness, meter_kind::true_peak, meter_kind::reduction,
        meter_kind::reduction, meter_kind::strength};
    for (int index = 0; index < 7; ++index)
        set_bar(wnd, IDC_MONITOR_BAR_INPUT + index, meter_position(kinds[index], numbers[index]));
    text_if_changed(wnd, IDC_MONITOR_STATUS,
        value.input <= -190.0 || value.output <= -190.0
            ? ui_text(L"解析中／静音", L"Analyzing / silence")
            : ui_text(L"動作中", L"Active"));
}

void save_position(HWND wnd) {
    if (IsIconic(wnd)) return;
    RECT rect = {};
    if (GetWindowRect(wnd, &rect)) {
        saved_position position;
        position.x = rect.left;
        position.y = rect.top;
        position.valid = 1;
        cfg_position = position;
    }
}

void clamp_position(HWND wnd) {
    RECT rect = {};
    GetWindowRect(wnd, &rect);
    MONITORINFO info = {};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST), &info)) return;
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    // Keep the title bar in the work area even on unusually small displays.
    const int x = std::max<int>(info.rcWork.left,
        std::min<int>(rect.left, info.rcWork.right - width));
    const int y = std::max<int>(info.rcWork.top,
        std::min<int>(rect.top, info.rcWork.bottom - height));
    SetWindowPos(wnd, nullptr, x, y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void apply_topmost(HWND wnd) {
    const bool topmost = cfg_topmost.get();
    SetWindowPos(wnd, topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void show_context_menu(HWND wnd, LPARAM coordinates) {
    HMENU menu = CreatePopupMenu();
    if (menu == nullptr) return;
    AppendMenuW(menu, MF_STRING | (cfg_topmost.get() ? MF_CHECKED : MF_UNCHECKED),
        1, ui_text(L"常に手前に表示", L"Always on Top"));
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 2,
        ui_text(L"モニターの見方...", L"How to read the monitor..."));
    POINT point = {static_cast<short>(LOWORD(coordinates)),
        static_cast<short>(HIWORD(coordinates))};
    if (point.x == -1 && point.y == -1) {
        RECT rect = {};
        GetWindowRect(wnd, &rect);
        point = {rect.left + 16, rect.top + 16};
    }
    const UINT selected = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
        point.x, point.y, 0, wnd, nullptr);
    DestroyMenu(menu);
    if (selected == 1) {
        cfg_topmost = !cfg_topmost.get();
        apply_topmost(wnd);
    } else if (selected == 2) {
        DialogBoxParamW(core_api::get_my_instance(), MAKEINTRESOURCEW(IDD_R128_GLOSSARY),
            wnd, glossary_dialog_proc, static_cast<LPARAM>(kMonitorGlossaryIndex));
    }
}

void capture_layout(HWND wnd, ui_context& context) {
    context.initial_dpi = window_dpi(wnd);
    RECT client = {};
    GetClientRect(wnd, &client);
    context.initial_client = {client.right, client.bottom};
    for (HWND child = GetWindow(wnd, GW_CHILD); child != nullptr;
         child = GetWindow(child, GW_HWNDNEXT)) {
        RECT rect = {};
        GetWindowRect(child, &rect);
        MapWindowPoints(HWND_DESKTOP, wnd, reinterpret_cast<POINT*>(&rect), 2);
        context.controls.emplace_back(child, rect);
    }
}

void scale_layout(HWND wnd, ui_context& context, UINT dpi, const RECT* suggested) {
    if (dpi == 0 || context.initial_dpi == 0) return;
    const auto scaled = [&](int value) { return MulDiv(value, dpi, context.initial_dpi); };
    HFONT font = CreateFontW(-MulDiv(9, dpi, 72), 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Yu Gothic UI");
    if (font != nullptr) {
        SendMessageW(wnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), FALSE);
        for (const auto& child : context.controls)
            SendMessageW(child.first, WM_SETFONT, reinterpret_cast<WPARAM>(font), FALSE);
        if (context.scaled_font != nullptr) DeleteObject(context.scaled_font);
        context.scaled_font = font;
    }
    RECT size = {0, 0, scaled(context.initial_client.cx), scaled(context.initial_client.cy)};
    using adjust_fn = BOOL (WINAPI*)(LPRECT, DWORD, BOOL, DWORD, UINT);
    static auto adjust = reinterpret_cast<adjust_fn>(GetProcAddress(
        GetModuleHandleW(L"user32.dll"), "AdjustWindowRectExForDpi"));
    const DWORD style = static_cast<DWORD>(GetWindowLongPtrW(wnd, GWL_STYLE));
    const DWORD extended = static_cast<DWORD>(GetWindowLongPtrW(wnd, GWL_EXSTYLE));
    if (adjust != nullptr) adjust(&size, style, FALSE, extended, dpi);
    else AdjustWindowRectEx(&size, style, FALSE, extended);
    RECT current = {};
    GetWindowRect(wnd, &current);
    SetWindowPos(wnd, nullptr, suggested ? suggested->left : current.left,
        suggested ? suggested->top : current.top,
        size.right - size.left, size.bottom - size.top,
        SWP_NOZORDER | SWP_NOACTIVATE);
    for (const auto& child : context.controls) {
        const RECT& rect = child.second;
        SetWindowPos(child.first, nullptr, scaled(rect.left), scaled(rect.top),
            scaled(rect.right - rect.left), scaled(rect.bottom - rect.top),
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
    clamp_position(wnd);
    InvalidateRect(wnd, nullptr, TRUE);
}

INT_PTR CALLBACK dialog_proc(HWND wnd, UINT message, WPARAM wp, LPARAM lp) {
    auto* context = reinterpret_cast<ui_context*>(GetWindowLongPtrW(wnd, GWLP_USERDATA));
    try {
        switch (message) {
        case WM_INITDIALOG: {
            context = new ui_context();
            SetWindowLongPtrW(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(context));
            context->dark_mode.AddDialogWithControls(wnd);
            modeless_dialog_manager::g_add(wnd);
            context->registered = true;
            for (int index = 0; index < 7; ++index)
                SendDlgItemMessageW(wnd, IDC_MONITOR_BAR_INPUT + index,
                    PBM_SETRANGE32, 0, 1000);
            capture_layout(wnd, *context);
            const auto position = cfg_position.get();
            if (position.version == 1 && position.valid == 1 &&
                position.x >= -1000000 && position.x <= 1000000 &&
                position.y >= -1000000 && position.y <= 1000000) {
                SetWindowPos(wnd, nullptr, position.x, position.y, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            } else {
                // Match Sonic Refiner's first placement beside the main window.
                RECT owner = {};
                if (GetWindowRect(core_api::get_main_window(), &owner))
                    SetWindowPos(wnd, nullptr, owner.right + 8, owner.top, 0, 0,
                        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
            clamp_position(wnd);
            const UINT restored_dpi = window_dpi(wnd);
            if (restored_dpi != context->initial_dpi)
                scale_layout(wnd, *context, restored_dpi, nullptr);
            apply_topmost(wnd);
            context->ready = true;
            refresh(wnd, *context);
            SetTimer(wnd, kTimer, 100, nullptr);
            return TRUE;
        }
        case WM_DISPLAYCHANGE:
            if (context != nullptr && context->ready) clamp_position(wnd);
            return FALSE;
        case WM_TIMER:
            if (wp == kTimer && context != nullptr) {
                refresh(wnd, *context);
                return TRUE;
            }
            break;
        case WM_DPICHANGED:
            if (context != nullptr && context->ready)
                scale_layout(wnd, *context, HIWORD(wp), reinterpret_cast<const RECT*>(lp));
            return TRUE;
        case WM_EXITSIZEMOVE:
            clamp_position(wnd);
            save_position(wnd);
            return TRUE;
        case WM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
        case WM_SETTINGCHANGE:
            if (context != nullptr && context->ready) {
                context->language_ready = false;
                refresh(wnd, *context);
                clamp_position(wnd);
                InvalidateRect(wnd, nullptr, TRUE);
            }
            return FALSE;
        case WM_COMMAND:
            if (LOWORD(wp) == IDCANCEL || LOWORD(wp) == IDOK) {
                SendMessageW(wnd, WM_CLOSE, 0, 0);
                return TRUE;
            }
            break;
        case WM_CONTEXTMENU:
            show_context_menu(wnd, lp);
            return TRUE;
        case WM_CLOSE:
            save_position(wnd);
            if (!shutting_down) cfg_visible = false;
            DestroyWindow(wnd);
            return TRUE;
        case WM_DESTROY:
            display_requested.store(false);
            display_epoch.fetch_add(1);
            KillTimer(wnd, kTimer);
            if (context != nullptr && context->registered)
                modeless_dialog_manager::g_remove(wnd);
            return TRUE;
        case WM_NCDESTROY:
            SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);
            if (window == wnd) window = nullptr;
            if (context != nullptr) {
                if (context->scaled_font != nullptr) DeleteObject(context->scaled_font);
                delete context;
            }
            return FALSE;
        }
    } catch (...) {
        if (message == WM_INITDIALOG) return FALSE;
        if (message == WM_TIMER && IsWindow(wnd))
            clear_values(wnd, ui_text(L"表示データを確認中", L"Data unavailable"));
    }
    return FALSE;
}

void show(bool activate) {
    if (shutting_down) return;
    if (IsWindow(window)) {
        ShowWindow(window, IsIconic(window) ? SW_RESTORE : SW_SHOW);
        if (activate) SetForegroundWindow(window);
        return;
    }
    display_epoch.fetch_add(1);
    display_requested.store(true);
    window = CreateDialogParamW(core_api::get_my_instance(),
        MAKEINTRESOURCEW(IDD_R128_MONITOR), core_api::get_main_window(),
        dialog_proc, 0);
    if (window == nullptr) {
        display_requested.store(false);
        if (activate) MessageBoxW(core_api::get_main_window(),
            ui_text(L"モニターを作成できませんでした。", L"Could not create the monitor."),
            L"R128 Monitor", MB_OK | MB_ICONERROR);
        return;
    }
    auto* context = reinterpret_cast<ui_context*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (context == nullptr || !context->ready) {
        DestroyWindow(window);
        window = nullptr;
        if (activate) MessageBoxW(core_api::get_main_window(),
            ui_text(L"モニターを初期化できませんでした。", L"Could not initialize the monitor."),
            L"R128 Monitor", MB_OK | MB_ICONERROR);
        return;
    }
    cfg_visible = true;
    ShowWindow(window, activate ? SW_SHOW : SW_SHOWNOACTIVATE);
    if (activate) SetForegroundWindow(window);
}

class menu_commands : public mainmenu_commands {
public:
    t_uint32 get_command_count() override { return 1; }
    GUID get_command(t_uint32 index) override {
        if (index != 0) uBugCheck();
        return guid_open;
    }
    void get_name(t_uint32 index, pfc::string_base& out) override {
        if (index != 0) uBugCheck();
        out = pfc::stringcvt::string_utf8_from_wide(ui_text(
            L"R128 補正モニター", L"R128 Processing Monitor"));
    }
    bool get_description(t_uint32 index, pfc::string_base& out) override {
        if (index != 0) uBugCheck();
        out = pfc::stringcvt::string_utf8_from_wide(ui_text(
            L"独立したリアルタイム補正モニターを開きます。",
            L"Opens the independent real-time correction monitor."));
        return true;
    }
    GUID get_parent() override { return mainmenu_groups::playback; }
    void execute(t_uint32 index, service_ptr_t<service_base>) override {
        if (index != 0) uBugCheck();
        show(true);
    }
};
static mainmenu_commands_factory_t<menu_commands> menu_factory;

class lifecycle : public initquit {
public:
    void on_init() override {
        shutting_down = false;
        fb2k::inMainThread([] {
            if (!shutting_down && cfg_visible.get()) show(false);
        });
    }
    void on_quit() override {
        shutting_down = true;
        if (IsWindow(window)) {
            save_position(window);
            // Preserve cfg_visible, unlike an explicit user close.
            DestroyWindow(window);
        }
    }
};
static initquit_factory_t<lifecycle> lifecycle_factory;

} // namespace r128_monitor

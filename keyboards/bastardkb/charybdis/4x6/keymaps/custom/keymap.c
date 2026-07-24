/**
 * Copyright 2021 Charly Delay <charly@codesink.dev> (@0xcharly)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

#ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
#    include "timer.h"
#endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

enum charybdis_keymap_layers {
    LAYER_BASE = 0,
    LAYER_NAV,
    LAYER_POINTER,
    LAYER_MEDIA,
    LAYER_NUMBER,
    LAYER_SYMBOL,
    LAYER_FUNCTION,
    LAYER_LOWER,
    LAYER_RAISE,
};

/** \brief Automatically enable sniping-mode on the pointer layer. */
#define CHARYBDIS_AUTO_SNIPING_ON_LAYER LAYER_POINTER

#ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
static uint16_t auto_pointer_layer_timer = 0;

#    ifndef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS
#        define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS 1000
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS

#    ifndef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD
#        define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD 8
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD
#endif     // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

#define LOWER MO(LAYER_LOWER)
#define RAISE MO(LAYER_RAISE)
#define PT_M LT(LAYER_POINTER, KC_M)
#define PT_DOT LT(LAYER_POINTER, KC_DOT)
#define TH_L1 LT(LAYER_MEDIA, KC_ESC)
#define TH_L2 LT(LAYER_NAV, KC_SPC)
#define TH_L3 LT(LAYER_FUNCTION, KC_TAB)
#define TH_R1 LT(LAYER_SYMBOL, KC_ENT)
#define TH_R2 LT(LAYER_NUMBER, KC_BSPC)

#ifndef POINTING_DEVICE_ENABLE
#    define DRGSCRL KC_NO
#    define DPI_MOD KC_NO
#    define S_D_MOD KC_NO
#    define SNIPING KC_NO
#endif // !POINTING_DEVICE_ENABLE

// Home row mods — Graphite NRTS / HAEI, same physical fingers as before
// pinky GUI, ring Alt, middle Ctrl, index Shift
#define HM_N  MT(MOD_LGUI, KC_N)
#define HM_R  MT(MOD_LALT, KC_R)
#define HM_T  MT(MOD_LCTL, KC_T)
#define HM_S  MT(MOD_LSFT, KC_S)
#define HM_H  MT(MOD_RSFT, KC_H)
#define HM_A  MT(MOD_RCTL, KC_A)
#define HM_E  MT(MOD_RALT, KC_E)
#define HM_I  MT(MOD_RGUI, KC_I)

// Dedicated mods (same fingers as HRMs)
#define MOD_N  KC_LGUI
#define MOD_R  KC_LALT
#define MOD_T  KC_LCTL
#define MOD_S  KC_LSFT
#define MOD_H  KC_RSFT
#define MOD_A  KC_RCTL
#define MOD_E  KC_RALT
#define MOD_I  KC_RGUI

// copy, cut, paste undo, redo
#define M_REDO LCTL(KC_Y)
#define M_PASTE LCTL(KC_V)
#define M_COPY LCTL(KC_C)
#define M_CUT  LCTL(KC_X)
#define M_UNDO LCTL(KC_Z)

// Graphite nonstandard shift pairs
const key_override_t quote_underscore = ko_make_basic(MOD_MASK_SHIFT, KC_QUOT, KC_UNDS);
const key_override_t comma_question   = ko_make_basic(MOD_MASK_SHIFT, KC_COMM, KC_QUES);
const key_override_t minus_dquote     = ko_make_basic(MOD_MASK_SHIFT, KC_MINS, KC_DQUO);
const key_override_t slash_lt         = ko_make_basic(MOD_MASK_SHIFT, KC_SLSH, KC_LT);

const key_override_t *key_overrides[] = {
    &quote_underscore,
    &comma_question,
    &minus_dquote,
    &slash_lt,
};

// Set tap hold delay for each modifier individually
uint16_t get_flow_tap_term(uint16_t keycode, keyrecord_t* record, uint16_t prev_keycode) {
    if (is_flow_tap_key(keycode) && is_flow_tap_key(prev_keycode)) {
      switch (keycode) {
          case HM_S:
          case HM_H:
              return 0;
          default:
              return FLOW_TAP_TERM;  // use the global macro directly
      }
    }
    return 0;
}

// Set tapping term for each modifier individually
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case HM_N:
        case HM_I:   // pinky: slow, give more time
            return TAPPING_TERM + 45;
        case HM_R:
        case HM_E:    // ring
            return TAPPING_TERM + 20;
        case HM_S:
        case HM_H:    // index shifts: fast, can be shorter
            return TAPPING_TERM - 30;
        default:
            return TAPPING_TERM;
    }
}

// HOLD_ON_OTHER_KEY_PRESS for thumbs (layers) and index shifts only.
// Chordal Hold still forces same-hand shift rolls to tap.
// Other HRMs and pointer LTs stay off HoOKP so rolls remain taps.
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TH_L1:
        case TH_L2:
        case TH_L3:
        case TH_R1:
        case TH_R2:
        case HM_S: // left shift
        case HM_H: // right shift
            return true;
        default:
            return false;
    }
}

// Home-row mods use chordal hold (opposite hands); everything else bypasses it.
bool get_chordal_hold(uint16_t tap_hold_keycode, keyrecord_t* tap_hold_record,
                      uint16_t other_keycode, keyrecord_t* other_record) {
    switch (tap_hold_keycode) {
        case HM_N: case HM_R: case HM_T: case HM_S:
        case HM_H: case HM_A: case HM_E: case HM_I:
            return get_chordal_hold_default(tap_hold_record, other_record);
        default:
            return true;  // bypass chordal-hold for everything else
    }
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [LAYER_BASE] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_B,    KC_L,    KC_D,    KC_W,    KC_Z,    KC_QUOT,    KC_F,    KC_O,    KC_U,   KC_J,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    HM_N,    HM_R,    HM_T,    HM_S,    KC_G,       KC_Y,    HM_H,    HM_A,    HM_E,   HM_I,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_Q,    KC_X,    PT_M,    KC_C,    KC_V,       KC_K,    KC_P,  PT_DOT, KC_MINS, KC_SLSH,  KC_NO,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                        TH_L1, TH_L2, TH_L3,      TH_R1, TH_R2,
                                        LOWER,        RAISE,      KC_NO
  //      ╰─────────────────────────────────────────────────╯ ╰──────────────────────────────────────╯
  ),

  [LAYER_NAV] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      M_REDO,  M_PASTE, M_COPY,  M_CUT, M_UNDO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    MOD_N,   MOD_R,   MOD_T,   MOD_S,   KC_NO,      KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_DEL,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_HOME, KC_PGDN, KC_PGUP, KC_END, KC_INS, KC_NO,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                   KC_NO,  KC_TRNS,   KC_NO,     KC_ENT, KC_BSPC,
                                             KC_NO,   KC_NO,     KC_NO
  //                            ╰───────────────────────────╯ ╰──────────────────╯
  ),

  [LAYER_POINTER] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
        KC_NO,   KC_NO,   KC_NO,  KC_NO,   KC_NO,    KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO, QK_BOOT,  EE_CLR,  KC_NO,   KC_NO,    KC_NO,      M_REDO, M_PASTE,  M_COPY,   M_CUT,  M_UNDO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,   MOD_N,   MOD_R, DPI_RMOD, S_D_RMOD, KC_NO,      KC_NO,  S_D_MOD, DPI_MOD,   KC_NO,   KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,   KC_NO, SNIPING, KC_TRNS, DRGSCRL,   KC_NO,      KC_NO,  DRGSCRL, KC_TRNS, SNIPING,   KC_NO,  KC_NO,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                         KC_MS_BTN2, KC_MS_BTN1, KC_MS_BTN3,      KC_MS_BTN3,  KC_MS_BTN1,
                                          KC_NO,      KC_NO,      KC_NO
  //                ╰───────────────────────────────────────╯ ╰───────────────────────────────╯
  ),

  [LAYER_MEDIA] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      RGB_RMOD, RGB_VAD, RGB_VAI, RGB_MOD, KC_NO, KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,  KC_MUTE, KC_MPRV, KC_VOLU, KC_VOLD, KC_MNXT,      KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT, KC_MUTE, KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  KC_TRNS, KC_MPLY, KC_MSTP,      KC_MSTP, KC_MPLY,
                                           KC_NO,     KC_NO,      KC_NO
  //                            ╰───────────────────────────╯ ╰──────────────────╯
  ),

  [LAYER_NUMBER] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_LBRC,  KC_7,    KC_8,    KC_9, KC_RBRC,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_SCLN,  KC_4,    KC_5,    KC_6,  KC_EQL,      KC_NO,   MOD_H,   MOD_A,   MOD_E,  MOD_I,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,     KC_GRV,  KC_1,    KC_2,    KC_3, KC_BSLS,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                   KC_DOT,   KC_0,  KC_COMM,      KC_NO,  KC_TRNS,
                                           KC_NO,     KC_NO,      KC_NO
  //                            ╰───────────────────────────╯ ╰──────────────────╯
  ),

  [LAYER_SYMBOL] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_NO, S(KC_LBRC), S(KC_7), S(KC_8), S(KC_9), S(KC_RBRC), KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_NO, S(KC_SCLN), S(KC_4), S(KC_5), S(KC_6), S(KC_EQL),  KC_NO,   MOD_H,   MOD_A,   MOD_E,  MOD_I,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_NO, S(KC_GRV),  S(KC_1), S(KC_2), S(KC_3), S(KC_BSLS), KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                S(KC_9), S(KC_0),   KC_QUES,      KC_TRNS,  KC_NO,
                                           KC_NO,     KC_NO,      KC_NO
  //                            ╰───────────────────────────╯ ╰──────────────────╯
  ),

  [LAYER_FUNCTION] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,   KC_NO,  KC_NO,  KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,    KC_PSCR,   KC_F7,   KC_F8,   KC_F9,  KC_F12, KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    MOD_N,   MOD_R,   MOD_T,   MOD_S,   KC_NO,    KC_SCRL,   KC_F4,   KC_F5,   KC_F6,  KC_F11, KC_NO,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
        KC_NO,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,    KC_PAUS,   KC_F1,   KC_F2,   KC_F3,  KC_F10, KC_NO,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                    KC_NO, KC_NO,   KC_TRNS,     KC_NO,  KC_APP,
                                           KC_NO,     KC_NO,     KC_NO
  //                            ╰───────────────────────────╯ ╰──────────────────╯
  ),

  [LAYER_LOWER] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       KC_TILD, KC_EXLM,   KC_AT, KC_HASH,  KC_DLR, KC_PERC,    KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_UNDS,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       RGB_MOD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    KC_LBRC,   KC_P7,   KC_P8,   KC_P9, KC_RBRC, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       RGB_TOG,   MOD_N,   MOD_R,   MOD_T,   MOD_S, XXXXXXX,    KC_PPLS,   KC_P4,   KC_P5,   KC_P6, KC_PMNS, KC_PEQL,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
      RGB_RMOD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    KC_PAST,   KC_P1,   KC_P2,   KC_P3, KC_PSLS, KC_PDOT,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  XXXXXXX, XXXXXXX, _______,    XXXXXXX, _______,
                                           XXXXXXX, XXXXXXX,      KC_P0
  //                            ╰───────────────────────────╯ ╰──────────────────╯
  ),

  [LAYER_RAISE] = LAYOUT(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
        KC_F12,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,      KC_F6,   KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_MNXT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_VOLU,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_MPLY, KC_LEFT,   KC_UP, KC_DOWN, KC_RGHT, XXXXXXX,    XXXXXXX,   MOD_H,   MOD_A,   MOD_E,  MOD_I, KC_MUTE,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_MPRV, KC_HOME, KC_PGUP, KC_PGDN,  KC_END, XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_VOLD,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  _______, _______, XXXXXXX,    _______, XXXXXXX,
                                           _______, _______,    XXXXXXX
  //                            ╰───────────────────────────╯ ╰──────────────────╯
  ),
};
// clang-format on

#ifdef POINTING_DEVICE_ENABLE
#    ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (abs(mouse_report.x) > CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD || abs(mouse_report.y) > CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD) {
        if (auto_pointer_layer_timer == 0) {
            layer_on(LAYER_POINTER);
#        ifdef RGB_MATRIX_ENABLE
            rgb_matrix_mode_noeeprom(RGB_MATRIX_NONE);
            rgb_matrix_sethsv_noeeprom(HSV_GREEN);
#        endif // RGB_MATRIX_ENABLE
        }
        auto_pointer_layer_timer = timer_read();
    }
    return mouse_report;
}

void matrix_scan_user(void) {
    if (auto_pointer_layer_timer != 0 && TIMER_DIFF_16(timer_read(), auto_pointer_layer_timer) >= CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS) {
        auto_pointer_layer_timer = 0;
        layer_off(LAYER_POINTER);
#        ifdef RGB_MATRIX_ENABLE
        rgb_matrix_mode_noeeprom(RGB_MATRIX_DEFAULT_MODE);
#        endif // RGB_MATRIX_ENABLE
    }
}
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

#    ifdef CHARYBDIS_AUTO_SNIPING_ON_LAYER
layer_state_t layer_state_set_user(layer_state_t state) {
    charybdis_set_pointer_sniping_enabled(layer_state_cmp(state, CHARYBDIS_AUTO_SNIPING_ON_LAYER));
    return state;
}
#    endif // CHARYBDIS_AUTO_SNIPING_ON_LAYER
#endif     // POINTING_DEVICE_ENABLE

#ifdef RGB_MATRIX_ENABLE
// Forward-declare this helper function since it is defined in rgb_matrix.c.
void rgb_matrix_update_pwm_buffers(void);
#endif

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
#pragma once

#ifdef VIA_ENABLE
/* VIA configuration. */
#    define DYNAMIC_KEYMAP_LAYER_COUNT 9
#endif // VIA_ENABLE

#ifndef __arm__
/* Disable unused features. */
#    define NO_ACTION_ONESHOT
#endif // __arm__

/* Charybdis-specific features. */

#ifdef POINTING_DEVICE_ENABLE
// Automatically enable the pointer layer when moving the trackball.  See also:
// - `CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS`
// - `CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD`
// #define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
#endif // POINTING_DEVICE_ENABLE

#define BOTH_SHIFTS_TURNS_ON_CAPS_WORD

// for homerow mods
// see: https://docs.qmk.fm/tap_hold
#define CHORDAL_HOLD // only resolve as hold if next key is on opposite hand
#define PERMISSIVE_HOLD // overriden by hold on other key press
#define TAPPING_TERM 200 // register hold after X ms delay
#define TAPPING_TERM_PER_KEY
#define FLOW_TAP_TERM 100 // disables holds when key is pressed X ms after previous key
#define QUICK_TAP_TERM 120  // defaults to TAPPING_TERM, delay for double tap to repeat
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY // only apply to layer tap thumb keys
// #define HOLD_ON_OTHER_KEY_PRESS // Better to enable only for layer taps

// This is the default layout for the handwired/jotanck keyboard

#include QMK_KEYBOARD_H

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.

#define _QWERTY   0
#define _LOWER    1
#define _RAISE    2

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_QWERTY] = LAYOUT_ortho_2x4( 
    KC_H,  KC_J,    KC_K,    KC_L,
    LOWER, RAISE, KC_LSFT, KC_LALT
),

[_LOWER] = LAYOUT_ortho_2x4 ( 
    _______, _______, KC_UP, KC_ENT,
    _______, KC_LEFT, KC_DOWN, KC_RGHT
),

[_RAISE] = LAYOUT_ortho_2x4 ( 
    KC_LEFT, KC_DOWN, KC_UP, KC_RGHT,
    KC_ENT, _______, _______, _______
),

};

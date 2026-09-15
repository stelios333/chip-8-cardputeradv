#pragma once
#include <unordered_map>
#include <cstdint>

// Cardputer keymap copied from https://github.com/bmorcelli/Launcher/blob/main/lib/Keyboard/Keyboard.h

#define KEYPAD_ROWS 14
#define KEYPAD_COLS 4

#define KEY_SHIFT 0x80
#define KEY_LEFT_CTRL 0x80
#define KEY_LEFT_SHIFT 0x81
#define KEY_LEFT_ALT 0x82
#define KEY_FN 0xff
#define KEY_OPT 0x00
#define KEY_BACKSPACE 0x2a
#define KEY_TAB 0x2b
#define KEY_ENTER 0x28


typedef struct {
    const char value_first;
    const char value_second;
} KeyValue_t;

inline void mapRawKeyToPhysical(uint8_t event, uint8_t &row, uint8_t &col) {
    const uint8_t u = event % 10; // 1..8
    const uint8_t t = event / 10; // 0..6

    if (u >= 1 && u <= 8 && t <= 6) {
        const uint8_t u0 = u - 1;   // 0..7
        col = u0 & 0x03;            // bits [1:0] => 0..3
        row = (t << 1) | (u0 >> 2); // t*2 + bit2(u0) => 0..13
    } else {
        row = 0xFF; // invalid
        col = 0xFF;
    }
}

const KeyValue_t CARDPUTER_KEYMAP[KEYPAD_COLS][KEYPAD_ROWS] = {
    {{'`', '~'},
     {'1', '!'},
     {'2', '@'},
     {'3', '#'},
     {'4', '$'},
     {'5', '%'},
     {'6', '^'},
     {'7', '&'},
     {'8', '*'},
     {'9', '('},
     {'0', ')'},
     {'-', '_'},
     {'=', '+'},
     {KEY_BACKSPACE, KEY_BACKSPACE}},
    {{KEY_TAB, KEY_TAB},
     {'q', 'Q'},
     {'w', 'W'},
     {'e', 'E'},
     {'r', 'R'},
     {'t', 'T'},
     {'y', 'Y'},
     {'u', 'U'},
     {'i', 'I'},
     {'o', 'O'},
     {'p', 'P'},
     {'[', '{'},
     {']', '}'},
     {'\\', '|'}                   },
    {{KEY_FN, KEY_FN},
     {KEY_LEFT_SHIFT, KEY_LEFT_SHIFT},
     {'a', 'A'},
     {'s', 'S'},
     {'d', 'D'},
     {'f', 'F'},
     {'g', 'G'},
     {'h', 'H'},
     {'j', 'J'},
     {'k', 'K'},
     {'l', 'L'},
     {';', ':'},
     {'\'', '\"'},
     {KEY_ENTER, KEY_ENTER}        },
    {{KEY_LEFT_CTRL, KEY_LEFT_CTRL},
     {KEY_OPT, KEY_OPT},
     {KEY_LEFT_ALT, KEY_LEFT_ALT},
     {'z', 'Z'},
     {'x', 'X'},
     {'c', 'C'},
     {'v', 'V'},
     {'b', 'B'},
     {'n', 'N'},
     {'m', 'M'},
     {',', '<'},
     {'.', '>'},
     {'/', '?'},
     {' ', ' '}                    }
};
const std::unordered_map<char, uint8_t> CHIP8_KEYMAP = {
    {'2', 1},
    {'3', 2},
    {'4', 3},
    {'5', 0xc},
    {'w', 4},
    {'e', 5},
    {'r', 6},
    {'t', 0xd},
    {'a', 7},
    {'s', 8},
    {'d', 9},
    {'f', 0xe},
    {KEY_LEFT_ALT, 0xa},
    {'z', 0},
    {'x', 0xb},
    {'c', 0xf},
};
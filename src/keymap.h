#include <unordered_map>

const std::unordered_map<char, uint8_t> CHIP8_KEYMAP = {
    {'1', 1},
    {'2', 2},
    {'3', 3},
    {'A', 0xc},
    {'4', 4},
    {'5', 5},
    {'6', 6},
    {'B', 0xd},
    {'7', 7},
    {'8', 8},
    {'9', 9},
    {'C', 0xe},
    {'*', 0xa},
    {'0', 0},
    {'#', 0xb},
    {'D', 0xf},
};
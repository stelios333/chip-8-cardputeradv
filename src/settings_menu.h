#pragma once
#include <array>

struct MenuOption
{
    const char *name;
    int name_len;
    int default_value;
    int max_value;
    int min_value;
    int step;

    MenuOption(const char *name, int default_value = 0, int max_value = 1, int min_value = 0, int step = 1) : 
    name(name), name_len(strlen(name)), default_value(default_value), max_value(max_value), min_value(min_value), step(step) {};

    ~MenuOption() = default;
};

const std::array settings_arr = {MenuOption("Shift quirk"), MenuOption("Memory quirk"), MenuOption("Turbo"), 
                                MenuOption("Mute"), MenuOption("Test option", 0, 10)};
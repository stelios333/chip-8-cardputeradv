#pragma once
#include <array>


using OptionUmap = std::unordered_map<int, const char*>;
struct MenuOption
{
    const char *name;
    int name_len;
    int default_value;
    int max_value;
    int min_value;
    int step;
    const OptionUmap* const value_map;
    MenuOption(const char *name, int default_value = 0, int max_value = 1, int min_value = 0, int step = 1,
    const OptionUmap* const value_map = nullptr) : 
    name(name), name_len(strlen(name)), default_value(default_value), max_value(max_value), min_value(min_value), step(step), value_map(value_map) {};

    ~MenuOption() = default;
};

const OptionUmap scale_map = {
    {1, "1x"},
    {2, "2x"},
    {3, "3x"},
    {4, "fullscreen"}
};

const std::array settings_arr = {MenuOption("Shift quirk"), MenuOption("Memory quirk"), MenuOption("Turbo"), 
                                MenuOption("Audio", 1), MenuOption("Scale", 3, 4, 1, 1, &scale_map)};
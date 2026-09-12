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
    const OptionUmap* const value_names;
    MenuOption(const char *name, int default_value = 0, int max_value = 1, int min_value = 0, int step = 1,
    const OptionUmap* const value_names = nullptr) : 
    name(name), name_len(strlen(name)), default_value(default_value), max_value(max_value), min_value(min_value), step(step), value_names(value_names) {};

    ~MenuOption() = default;
};

const OptionUmap scale_map = {
    {0, "1x"},
    {1, "2x"}
};

const std::array settings_arr = {MenuOption("Shift quirk"), MenuOption("Memory quirk"), MenuOption("Turbo"), 
                                MenuOption("Audio", 1), MenuOption("Scale", 1, 1, 0, 1, &scale_map)};
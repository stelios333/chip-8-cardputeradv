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

const OptionUmap audio_vol_map = {
    {0x7F, "mute"},
    {0x8F, "15%"},
    {0x9F, "25%"},
    {0xAF, "50%"},
    {0xBF, "75%"},
    {0xCF, "100%"}
};

const OptionUmap audio_wf_map = {
    {1, "sine"},
    {2, "square"},
};

const std::array settings_arr = {MenuOption("Shift quirk"), MenuOption("Memory quirk"), MenuOption("Turbo"), 
                                MenuOption("Volume", 0xAF, 0xCF, 0x7F, 16, &audio_vol_map),
                                MenuOption("Audio waveform", 1, 2, 1, 1, &audio_wf_map),
                                MenuOption("Scale", 3, 4, 1, 1, &scale_map)};
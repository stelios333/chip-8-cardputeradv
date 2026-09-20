
# Chip 8 Emulator for Cardputer ADV

A port of my chip-8 emulator for the M5Stack Cardputer-adv.

# Pictures
![](https://raw.githubusercontent.com/Stelios333/chip-8-cardputeradv/master/pictures/space_invaders.jpg)
![](https://raw.githubusercontent.com/Stelios333/chip-8-cardputeradv/master/pictures/quirks.jpg)
![](https://raw.githubusercontent.com/Stelios333/chip-8-cardputeradv/master/pictures/settings_menu.jpg)

# Keybinds
**Menu-related**
 - `;` Up
 - `.` Down
 - `ok` Enter
 - `,` Decrease value
 - `/` Increase value
 - `s` Open settings
 - `esc` Close settings

**In-game**
 - `p` Pause
 - Chip-8 keypad:
![](https://raw.githubusercontent.com/Stelios333/chip-8-cardputeradv/master/pictures/keypad.jpg)

# Note

The audio part of the emulator is almost entirely written by an LLM. I feel ashamed for that as it was quite trivial to implement for other platforms and I was disappointed I couldn't figure it out for this one. Unfortunately I'm not familiar with the esp-idf i2s api nor how the i2c registers of the es8311 work. I think it is better though than no audio support at all.\

  

If you know better and think that the code needs to be improved don't hesitate to make a pull request :)

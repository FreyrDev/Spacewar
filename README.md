# Spacewar!
 A clone of the 1962 game "Spacewar!" using text based graphics (ASCII plus box-drawing characters).

## Running
 Compilation is handled by the makefile, `make` will compile and run, while `make c` or `make r` will do each separately. To compile manually, remember to link `-lncursesw` and `-lm`.

 When running the game, please fullscreen the terminal before entering the make command, it needs to be at least 168x51 characters or it won't display properly.

## Controls
 Due to limitations of ncurses, the controls are tap or toggle based rather than hold down. Engines are toggle on/off, while turning requires taps.
 Pause/unpause as well as selecting the menu option are performed using the <kbd>Enter</kbd> key.
 
 | Action | Player 1 | Player 2 |
 |:------:|:--------:|:--------:|
 | Toggle Engine | <kbd>w</kbd> | <kbd>↑</kbd> |
 | Rotate Left   | <kbd>a</kbd> | <kbd>←</kbd> |
 | Rotate Right  | <kbd>d</kbd> | <kbd>→</kbd> |
 | Fire Torpedo  | <kbd>s</kbd> | <kbd>↓</kbd> |

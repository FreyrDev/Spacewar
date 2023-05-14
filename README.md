# Spacewar!
 A clone of the 1962 game "Spacewar!" using text based graphics (ASCII plus box-drawing characters).

## Running
 Compilation requires `-lncursesw` and `-lm`, for example `gcc -o out/main src/main.c -lncursesw -lm && ./out/main` compiles it and the runs the compiled file from its output directory.

## Controls
 Due to limitations of ncurses, the controls are tap or toggle based rather than hold down. Engines are toggle on/off, while turning requires taps.
 
 | Action | Player 1 | Player 2 |
 |:------:|:--------:|:--------:|
 | Engine Thrust | <kbd>w</kbd> | <kbd>↑</kbd> |
 | Rotate Left   | <kbd>a</kbd> | <kbd>←</kbd> |
 | Rotate Right  | <kbd>d</kbd> | <kbd>→</kbd> |
 | Engine Thrust | <kbd>s</kbd> | <kbd>↓</kbd> |

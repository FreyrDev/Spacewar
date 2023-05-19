# Spacewar!
 A clone of the 1962 game "Spacewar!" using text based graphics (ASCII plus box-drawing characters).

## Running
 Compilation is handled by the makefile, `make` will compile and run, while `make c` or `make r` will do each separately.
 If you are compiling manually without the makefile, remember to link `-lncursesw` and `-lm`.

 When running the game, please fullscreen the terminal before entering the make command, it needs to be at least 168x51 characters or it won't display properly.

## Playing
 Due to limitations of ncurses, the controls are tap or toggle based rather than hold down. Engines are toggle on/off, while turning requires taps.
 
 Pausing, unpausing, and selecting an option in the menu are all performed using the <kbd>↵ Enter</kbd> key.
 
 |    Action     | Player 1 (`A`) | Player 2 (`T`) |
 |:-------------:|:--------------:|:--------------:|
 | Toggle Engine |  <kbd>w</kbd>  |  <kbd>↑</kbd>  |
 | Rotate Left   |  <kbd>a</kbd>  |  <kbd>←</kbd>  |
 | Rotate Right  |  <kbd>d</kbd>  |  <kbd>→</kbd>  |
 | Fire Torpedo  |  <kbd>s</kbd>  |  <kbd>↓</kbd>  |

 Players gain 250 points by hitting the enemy ship with a torpedo, and lose 50 if they die by colliding with a torpedo, black hole, or the other ship.
 A player wins if they reach 1000 points, or if their oponent reaching -1000.

#include "utils.h"

#define PHYSICS_SPEED 1.0
#define FRAMERATE 50

#define P1_Y 76.5
#define P1_X 25.5
#define P2_Y 26.5
#define P2_X 75.5

#define WIN_H 51
#define WIN_W 101
#define UI_SIZE 30

/* SETUP
 * Calls all required functions for ncurses setup so the screen displays correctly
 * and inputs are read correctly, as well as configuring the terminal colours
 */

void setup() {
  setlocale(LC_ALL, "");
  initscr();
  noecho();
  nodelay(stdscr, TRUE);
  cbreak();
  keypad(stdscr, TRUE);
  start_color();
  curs_set(0);
  refresh();

  init_color(COLOR_CYAN, 400, 850, 975);
  init_color(COLOR_GREEN, 250, 400, 150);
  init_color(COLOR_YELLOW, 125, 175, 100);
  init_color(COLOR_BLACK, 50, 50, 50);
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
}


void draw_ship(WINDOW *win, int y, int x, int player) {
  if (player == 1) {
    mvwprintw(win, y+0, x, "  /!\\  ");
    mvwprintw(win, y+1, x, " (   ) ");
    mvwprintw(win, y+2, x, " / θ \\");
    mvwprintw(win, y+3, x, "( ___ )");
    mvwprintw(win, y+4, x, "/_\\ /_\\");
  }
  else {
    mvwprintw(win, y+0, x, " __!__ ");
    mvwprintw(win, y+1, x, "(_   _)");
    mvwprintw(win, y+2, x, "  |θ|  ");
    mvwprintw(win, y+3, x, "  ( )  ");
    mvwprintw(win, y+4, x, "  /_\\ ");
  }
}


/* CREATE UI
 * Prints the static parts of the players' HUDs, so they don't have to be reprinted every frame
 * Also includes the player specific spaceship for the visuals section
 */

void create_ui(WINDOW *ui, int player) {
  mvwprintw(ui, 0, 0,
    "┌────────┤ PLAYER %d ├────────┐"
    "│ ┌──┐                  ┌──┐ │"
    "│ │()│  SCORE  [     ]  │()│ │"
    "│ └──┘                  └──┘ │"
    "└────────────────────────────┘"
    "┌┤ VISUALS ├┐ ┌──┤ ENGINE ├──┐"
    "│           │ │              │"
    "│           │ │              │"
    "│           │ │              │"
    "│           │ │      ╶╴      │"
    "│           │ └──────────────┘"
    "│           │ ┌┤ ENG. TEMP. ├┐"
    "│           │ │              │"
    "└───────────┘ └──────────────┘"
    "┌─┤ HEADING ├─┐ ┌┤ WEAPONRY ├┐"
    "│             │ │            │"
    "│        ---  │ │ T  T  ---  │"
    "│   •         │ │ |  |       │"
    "│        ---  │ │ |  |  ---  │"
    "│             │ │ I==I       │"
    "└─────────────┘ └────────────┘"
    "┌─────┤ STATUS READOUT ├─────┐"
    "│                            │"
    "│ CURRENT VELOCITY           │"
    "│ TRAVEL DIRECTION           │"
    "│                            │"
    "└────────────────────────────┘"
    , player);

  draw_ship(ui, 7, 3, player);
  
  wrefresh(ui);
}


// Automates resetting the players position when they are destroyed
void destroy(Player *player) {
  if (player->type == PLAYER1) {
    *player = new_player(PLAYER1, P1_Y, P1_X, NW, player->score-50);
  }
  else if (player->type == PLAYER2) {
    *player = new_player(PLAYER2, P2_Y, P2_X, SE, player->score-50);
  }
}


/* UPDATE PHYSICS
 * Steps all physics on each frame, with delta since last frame to correct for frametime differences
 */

void update_physics(GameState *game, int delta, int frame) {
  double d = (double)delta/33333333.3 * PHYSICS_SPEED;

  for (int i=0; i < 2; i++) {

    // Calculate the new positions of the colour trails
    if (frame%4 == 0) {
      shift_trails(&game->players[i].data);
      shift_trails(&game->bullets[i].data);
    }

    // Check for overheating and calculate temperature & engine acceleration
    if (game->players[i].temp > 100) {
      game->players[i].acc = false;
      game->players[i].temp = -100;
    }
    else if (game->players[i].temp < 0) {
      game->players[i].temp += d;
    }
    else if (!game->players[i].acc) {
      game->players[i].temp -= d/2;
      if (game->players[i].temp < 0) { game->players[i].temp = 0; }
    }
    else {
      game->players[i].data.vely += 0.005 * thrust_vector(game->players[i].dir, Y) * d;
      game->players[i].data.velx += 0.005 * thrust_vector(game->players[i].dir, X) * d;
      game->players[i].temp += d/2;
    }

    // Gravity calculations for the black hole
    double dy = game->players[i].data.y - game->bh.y;
    double dx = game->players[i].data.x - game->bh.x;
    double r2 = total_dist_squared(dy, dx);
    double r = sqrt(r2);
    double g = -2 / r2;
    double unity = dy / r;
    double unitx = dx / r;
    game->players[i].data.vely += g * unity * d;
    game->players[i].data.velx += g * unitx * d;

    if (r < 1) {
      destroy(&game->players[i]);
    }

    // Destroy players ships if they've crashed into each other
    dy = game->players[i].data.y - game->players[1-i].data.y;
    dx = game->players[i].data.x - game->players[1-i].data.x;
    r = sqrt(total_dist_squared(dy, dx));
    if (r < 2) {
      destroy(&game->players[i]);
      destroy(&game->players[1-i]);
    }

    // Cap players velocity at 1
    double velxy = total_vel(game->players[i].data);
    if (velxy > 1) {
      game->players[i].data.vely /= velxy;
      game->players[i].data.velx /= velxy;
    }

    // Update position with new velocity
    game->players[i].data.y += game->players[i].data.vely * d;
    game->players[i].data.x += game->players[i].data.velx * d;

    // Move ship to opposite side of screen if it goes off the edge
    if (game->players[i].data.y >= 2*WIN_H-2) { game->players[i].data.y -= 2*WIN_H-4; }
    else if (game->players[i].data.y <= 2) { game->players[i].data.y += 2*WIN_H-4; }
    if (game->players[i].data.x >= WIN_W-1) { game->players[i].data.x -= WIN_W-2; }
    else if (game->players[i].data.x <= 1) { game->players[i].data.x += WIN_W-2; }


    if (game->bullets[i].type == BULLET) {
      game->bullets[i].data.y += game->bullets[i].data.vely * d;
      game->bullets[i].data.x += game->bullets[i].data.velx * d;

      if (game->bullets[i].data.y >= 2*WIN_H-2) { game->bullets[i].data.y -= 2*WIN_H-4; }
      else if (game->bullets[i].data.y <= 2) { game->bullets[i].data.y += 2*WIN_H-4; }
      if (game->bullets[i].data.x >= WIN_W-1) { game->bullets[i].data.x -= WIN_W-2; }
      else if (game->bullets[i].data.x <= 1) { game->bullets[i].data.x += WIN_W-2; }

      // Check if a bullet has hit a ship and update positions & score
      for (int j=0; j<=1; j++) {
        double dy = game->bullets[i].data.y - game->players[j].data.y;
        double dx = game->bullets[i].data.x - game->players[j].data.x;
        double r = sqrt(total_dist_squared(dy, dx));
        if (r < 2) {
          destroy(&game->players[j]);
          game->bullets[i] = err_bullet();
          game->players[1-j].score += 250;
        }
      }

      // Destroy bullets if they've crashed into each other
      dy = game->bullets[i].data.y - game->bullets[1-i].data.y;
      dx = game->bullets[i].data.x - game->bullets[1-i].data.x;
      r = sqrt(total_dist_squared(dy, dx));
      if (r < 2) {
        game->bullets[i] = err_bullet();
        game->bullets[1-i] = err_bullet();
      }

      // Destroy bullet after certain amount of time
      game->bullets[i].fuse -= delta;
      if (game->bullets[i].fuse < 0) {
        game->bullets[i] = err_bullet();
      }
    }
  }
}


/* UPDATE SCREEN
 * Clears game screen and redraws new positions of all game objects
 * Updates dynamic parts of HUDs, including animations and status indicators
 */

void update_screen(WINDOW *win, WINDOW *ui1, WINDOW *ui2, GameState game) {
  WINDOW *ui[] = { ui1, ui2 };
  werase(win);

  for (int i=0; i < 2; i++) {
    wcolour(win, 3);
    mvwprintw(win, (game.players[i].data.y3)/2, game.players[i].data.x3, "%lc", charoftype(game.players[i].type));
    mvwprintw(win, (game.bullets[i].data.y3)/2, game.bullets[i].data.x3, "%lc", charoftype(game.bullets[i].type));
    wcolour(win, 2);
    mvwprintw(win, (game.players[i].data.y2)/2, game.players[i].data.x2, "%lc", charoftype(game.players[i].type));
    mvwprintw(win, (game.players[i].data.y1)/2, game.players[i].data.x1, "%lc", charoftype(game.players[i].type));
    mvwprintw(win, (game.bullets[i].data.y2)/2, game.bullets[i].data.x2, "%lc", charoftype(game.bullets[i].type));
    mvwprintw(win, (game.bullets[i].data.y1)/2, game.bullets[i].data.x1, "%lc", charoftype(game.bullets[i].type));
    wcolour(win, 1);
    mvwprintw(win, (game.players[i].data.y)/2, game.players[i].data.x, "%lc", charoftype(game.players[i].type));
    mvwprintw(win, (game.bullets[i].data.y)/2, game.bullets[i].data.x, "%lc", charoftype(game.bullets[i].type));
  }
  
  mvwprintw(win, game.bh.y / 2, game.bh.x, "%lc", charoftype(BLACKHOLE));

  box(win, 0, 0);
  mvwprintw(win, 0, 4, "┤ SPACEWAR! ├");

  wnoutrefresh(win);

  for (int i=0; i<2; i++) {
    mvwprintw(ui[i], 2, 16, "%05d", game.players[i].score);

    if (game.bullets[i].type == ERR) {
      mvwprintw(ui[i], 7, 6, "!");
      mvwprintw(ui[i], 16, 19, "╭╮");
      mvwprintw(ui[i], 17, 19, "├┤");
      mvwprintw(ui[i], 18, 19, "└┘");
      mvwprintw(ui[i], 17, 23, "READY");
    }
    else {
      if (game.bullets[i].fuse > INT_MAX/2) {
        mvwprintw(ui[i], 7, 6, ".");
        mvwprintw(ui[i], 16, 19, "  ");
        mvwprintw(ui[i], 17, 19, "  ");
        mvwprintw(ui[i], 18, 19, "  ");
        mvwprintw(ui[i], 17, 23, "     ");
      }
      else if (game.bullets[i].fuse > INT_MAX/4) {
        mvwprintw(ui[i], 7, 6, ".");
        mvwprintw(ui[i], 16, 19, "  ");
        mvwprintw(ui[i], 17, 19, "  ");
        mvwprintw(ui[i], 18, 19, "╭╮");
        mvwprintw(ui[i], 17, 23, "     ");
      }
      else {
        mvwprintw(ui[i], 7, 6, ".");
        mvwprintw(ui[i], 16, 19, "  ");
        mvwprintw(ui[i], 17, 19, "╭╮");
        mvwprintw(ui[i], 18, 19, "├┤");
        mvwprintw(ui[i], 17, 23, "     ");
      }
    }

    if (game.players[i].acc) {
      mvwprintw(ui[i], 7, 16, "MAIN ENGINES");
      mvwprintw(ui[i], 8, 16, " FULL POWER ");
      mvwprintw(ui[i], 9, 16, "! ! !╶╴! ! !");
    }
    else {
      mvwprintw(ui[i], 7, 16, "            ");
      mvwprintw(ui[i], 8, 16, "            ");
      mvwprintw(ui[i], 9, 16, "     ╶╴     ");
      mvwprintw(ui[i], 12,  4, "     ");
    }

    for (int j = 0; j < 14; j++) {
      if ((100/14)*j < game.players[i].temp) { mvwaddch(ui[i], 12, 15+j, '|'); }
      else { mvwaddch(ui[i], 12, 15+j, ' '); }
    }

    if (game.players[i].temp < 0) {
      mvwprintw(ui[i], 12, 15, " ! OVERHEAT ! ");
    }

    mvwprintw(ui[i], 16, 2, "· · ·");
    mvwprintw(ui[i], 17, 2, "· • ·");
    mvwprintw(ui[i], 18, 2, "· · ·");

    mvwprintw(ui[i], 17+round(thrust_vector(game.players[i].dir, Y)), 4+2*round(thrust_vector(game.players[i].dir, X)), "%lc", charofdir(game.players[i].dir));

    mvwprintw(ui[i], 17, 9, "%03d°", game.players[i].dir * 45);

    if (total_vel(game.players[i].data) > 0.995) {
      mvwprintw(ui[i], 23, 20, "100.000%%");
    } 
    else {
      mvwprintw(ui[i], 23, 20, "%07.3f%%", total_vel(game.players[i].data) * 100);
    }

    mvwprintw(ui[i], 24, 20, "%07.3f°", fmod(atan2(game.players[i].data.vely, game.players[i].data.velx) * 180/M_PI + 450, 360));
  }

  if (game.players[0].acc) {
    mvwaddch(ui1, 12, 4, "^\"*8°"[rand()%5]);
    mvwaddch(ui1, 12, 8, "^\"*8°"[rand()%5]);
  }
  if (game.players[1].acc) {
    mvwaddch(ui2, 12, 6, "^\"*8°"[rand()%5]);
  }

  wnoutrefresh(ui1);
  wnoutrefresh(ui2);

  doupdate();
}


// Draws the title screen/pause menu
void update_menu_screen(WINDOW *win, GameState game, int selected, int winner) {
  werase(win);

  mvwprintw(win, 10, (WIN_W-49)/2, " _____                                         _ ");
  mvwprintw(win, 11, (WIN_W-49)/2, "/  ___|                                       | |");
  mvwprintw(win, 12, (WIN_W-49)/2, "\\ `--. _ __   __ _  ___ _____      ____ _ _ __| |");
  mvwprintw(win, 13, (WIN_W-49)/2, " `--. \\ '_ \\ / _` |/ __/ _ \\ \\ /\\ / / _` | '__| |");
  mvwprintw(win, 14, (WIN_W-49)/2, "/\\__/ / |_) | (_| | (_|  __/\\ V  V / (_| | |  |_|");
  mvwprintw(win, 15, (WIN_W-49)/2, "\\____/| .__/ \\__,_|\\___\\___| \\_/\\_/ \\__,_|_|  (_)");
  mvwprintw(win, 16, (WIN_W-49)/2, "      | |                                        ");
  mvwprintw(win, 17, (WIN_W-49)/2, "      |_|                                        ");

  if (selected == 0) {
    mvwprintw(win, 23, (WIN_W-13)/2, " ─┤ PLAY! ├─ ");
    mvwprintw(win, 26, (WIN_W-13)/2, "─╴  QUIT?  ╶─");
  }
  else {
    mvwprintw(win, 23, (WIN_W-13)/2, "─╴  PLAY?  ╶─");
    mvwprintw(win, 26, (WIN_W-13)/2, " ─┤ QUIT! ├─ ");
  }

  mvwprintw(win, 20, (WIN_W-3)/2-15, "P 1");
  mvwprintw(win, 20, (WIN_W-3)/2+15, "P 2");

  draw_ship(win, 22, (WIN_W-7)/2-15, 1);
  draw_ship(win, 22, (WIN_W-7)/2+15, 2);

  mvwprintw(win, 29, (WIN_W-7)/2-15, "W A S D");
  mvwprintw(win, 29, (WIN_W-7)/2+15, "↑ ← ↓ →");

  if (winner) {
    mvwprintw(win, 32, (WIN_W-13)/2, "PLAYER %d WINS", winner);
    mvwprintw(win, 34, (WIN_W-37)/2, "P1 SCORE  %05d       P2 SCORE  %05d", game.players[0].score, game.players[1].score);

    mvwaddch(win, 27, (WIN_W-7)/2-10, "^\"*8°"[rand() % 5]);
    mvwaddch(win, 27, (WIN_W-7)/2-14, "^\"*8°"[rand() % 5]);
    mvwaddch(win, 27, (WIN_W-7)/2+18, "^\"*8°"[rand() % 5]);
  }

  wrefresh(win);
}


// Handles the key presses for while the menu is open
void handle_menu_inputs(int keys[], int *pause_toggle, int *selected, int *quit) {
  for (int i = 0; i < 8 && keys[i] != ERR; i++) {
    if (keys[i] == 'w' || keys[i] == KEY_UP || keys[i] == 's' || keys[i] == KEY_DOWN) {
      *selected = !(*selected);
    }
    else if (keys[i] == '\n') {
      if (*selected == 1) { *quit = true; }
      else { *pause_toggle = true; }
    }
  }
}

// Handles the key presses for while the game is running 
void handle_game_inputs(GameState *game, int keys[], int *pause_toggle) {
  for (int i = 0; i < 8 && keys[i] != ERR; i++) {
    if (keys[i] == '\n') {
      *pause_toggle = true;
    }
    else if (keys[i] == 'w' && game->players[0].temp >= 0) {
      game->players[0].acc = !game->players[0].acc;
    }
    else if (keys[i] == 'a') {
      game->players[0].dir -= 1;
      if(game->players[0].dir < 0) { game->players[0].dir += 8;}
    }
    else if (keys[i] == 'd') {
      game->players[0].dir += 1;
      game->players[0].dir %= 8;
    }
    else if (keys[i] == 's' && game->bullets[0].type == ERR) {
      game->bullets[0] = new_bullet(game->players[0].data.y+2*thrust_vector(game->players[0].dir, Y), game->players[0].data.x+2*thrust_vector(game->players[0].dir, X));
      game->bullets[0].data.vely = game->players[0].data.vely + 0.5*thrust_vector(game->players[0].dir, Y);
      game->bullets[0].data.velx = game->players[0].data.velx + 0.5*thrust_vector(game->players[0].dir, X);
    }
    else if (keys[i] == KEY_UP  && game->players[1].temp >= 0) {
      game->players[1].acc = !game->players[1].acc;
    }
    else if (keys[i] == KEY_LEFT) {
      game->players[1].dir -= 1;
      if(game->players[1].dir < 0) { game->players[1].dir += 8;}
    }
    else if (keys[i] == KEY_RIGHT) {
      game->players[1].dir += 1;
      game->players[1].dir %= 8;
    }
    else if (keys[i] == KEY_DOWN && game->bullets[1].type == ERR) {
      game->bullets[1] = new_bullet(game->players[1].data.y+2*thrust_vector(game->players[1].dir, Y), game->players[1].data.x+2*thrust_vector(game->players[1].dir, X));
      game->bullets[1].data.vely = game->players[1].data.vely + 0.5*thrust_vector(game->players[1].dir, Y);
      game->bullets[1].data.velx = game->players[1].data.velx + 0.5*thrust_vector(game->players[1].dir, X);
    }
  }
}


/* MAIN
 * Runs initial setup of the windows and object population
 * Runs game loop
 */

int main() {
  setup();

  // Create main game windows and UI windows for HUDs in correct place in centre of screen
  int scrh;
  int scrw;
  getmaxyx(stdscr, scrh, scrw);

  WINDOW *win = newwin(WIN_H, WIN_W, (scrh-WIN_H)/2, (scrw-WIN_W)/2);
  WINDOW *ui1 = newwin(UI_SIZE-2, UI_SIZE, (scrh-WIN_H)/2, (scrw-UI_SIZE)/2-69);
  WINDOW *ui2 = newwin(UI_SIZE-2, UI_SIZE, (scrh-WIN_H)/2, (scrw-UI_SIZE)/2+69);
  wcolour(win, 1);
  wcolour(ui1, 1);
  wcolour(ui2, 1);

  // Initiate array of all game objects (the black hole, the players, and empty spots for torpedoes to spawn);
  GameState game;
  game.bh = (BlackHole){WIN_H+0.5, (double)WIN_W/2};
  game.players[0] = new_player(PLAYER1, P1_Y, P1_X, NW, 0);
  game.players[1] = new_player(PLAYER2, P2_Y, P2_X, SE, 0);
  game.bullets[0] = err_bullet();
  game.bullets[1] = err_bullet();

  // Start timing to calculate frametimes
  int frame = 0;
  int delta = 0;
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  int quit = false;
  int paused = true;
  int pause_toggle = false;
  int selected = 0;
  int winner = 0;

  while (!quit) { 
    if (delta >= 1000000000/FRAMERATE) {
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);

      int keys_pressed[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
      int ch;
      int ch_num = 0;
      while ((ch = getch()) != ERR) {
        keys_pressed[ch_num] = ch;
        ch_num++;
      }
      keys_pressed[ch_num] = ERR;

      if (pause_toggle && paused) {
        create_ui(ui1, 1);
        create_ui(ui2, 2);

        if (winner) {
          destroy(&game.players[0]);
          destroy(&game.players[1]);
          game.bullets[0] = err_bullet();
          game.bullets[1] = err_bullet();
          game.players[0].score = 0;
          game.players[1].score = 0;
        }

        paused = false;
        pause_toggle = false;
      }
      else if (pause_toggle && !paused) {
        werase(ui1);
        werase(ui2);
        wnoutrefresh(ui1);
        wnoutrefresh(ui2);
        doupdate();
        paused = true;
        pause_toggle = false;
      }

      if (paused) {
        handle_menu_inputs(keys_pressed, &pause_toggle, &selected, &quit);
        update_menu_screen(win, game, selected, winner);
      }
      else {
        handle_game_inputs(&game, keys_pressed, &pause_toggle);
        update_physics(&game, delta, frame);
        update_screen(win, ui1, ui2, game);

        if (game.players[0].score >= 1000 || game.players[1].score <= -1000) {
          pause_toggle = true;
          winner = 1;
        }
        else if (game.players[1].score >= 1000 || game.players[0].score <= -1000) {
          pause_toggle = true;
          winner = 2;
        }
      }

      delta = 0;
      frame++;
    }
    else {
      struct timespec end;
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);
      delta = get_delta(&start, &end);
    }
  
  }

  endwin();
  return 0;
}

#include "utils.h"

#define PHYSICS_SPEED 1.0
#define FRAMERATE 50

#define P1_Y 76.5
#define P1_X 25.5
#define P2_Y 26.5
#define P2_X 75.5


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
    "│           │ ┌──────────────┐"
    "│           │ │.~^~._.~^~._.~│"
    "└───────────┘ └──────────────┘"
    "┌─┤ HEADING ├─┐ ┌┤ WEAPONRY ├┐"
    "│             │ │            │"
    "│        ---  │ │ T  T  ---  │"
    "│   •         │ │ |  |       │"
    "│        ---  │ │ |  |  ---  │"
    "│             │ │ I==I       │"
    "└─────────────┘ └────────────┘"
    "┌────┤ STATUS READOUTS ├─────┐"
    "│                            │"
    "│ CURRENT VELOCITY           │"
    "│ TRAVEL DIRECTION           │"
    "│                            │"
    "└────────────────────────────┘"
    , player);


  if (player == 1) {
    mvwprintw(ui,  7, 3, "  /!\\  ");
    mvwprintw(ui,  8, 3, " (   ) ");
    mvwprintw(ui,  9, 3, " / θ \\");
    mvwprintw(ui, 10, 3, "( ___ )");
    mvwprintw(ui, 11, 3, "/_\\ /_\\");
  }
  else {
    mvwprintw(ui,  7, 3, " __!__ ");
    mvwprintw(ui,  8, 3, "(_   _)");
    mvwprintw(ui,  9, 3, "  |θ|  ");
    mvwprintw(ui, 10, 3, "  ( )  ");
    mvwprintw(ui, 11, 3, "  /_\\ ");
  }
  wrefresh(ui);
}


// Automates resetting the players position when they are destroyed
GameObject destroy(GameObject player) {
  if (player.type == PLAYER1) {
    return new_gameobject(PLAYER1, P1_Y, P1_X, N, player.score-50);
  }
  else if (player.type == PLAYER2) {
    return new_gameobject(PLAYER2, P2_Y, P2_X, S, player.score-50);
  }
  else {
    return err_gameobject();
  }
}


/* UPDATE PHYSICS
 * Steps all physics on each frame, with delta since last frame to correct for frametime differences
 * Includes:
 * - Calculating trail positions
 * - Applying engine acceleration
 * - Applying gravity to player spaceships
 * - Capping player speed
 * - Calculating collisions for players and bullets
 * - Moving players to opposite edge of window if bounds exceeded
 */

void update_physics(WINDOW* game_win, GameObject *objects, int delta, int frame) {
  double d = (double)delta/33333333.3 * PHYSICS_SPEED;
  int winh;
  int winw;
  getmaxyx(game_win, winh, winw);

  for (int i=0; i < 5; i++) {
    if (frame%4 == 0) {
      objects[i].y3 = objects[i].y2;
      objects[i].x3 = objects[i].x2;
      objects[i].y2 = objects[i].y1;
      objects[i].x2 = objects[i].x1;
      objects[i].y1 = objects[i].y;
      objects[i].x1 = objects[i].x;
    }

    if (objects[i].acc) {
      objects[i].vely += 0.005 * thrust_vector(objects[i].dir, 'y') * d;
      objects[i].velx += 0.005 * thrust_vector(objects[i].dir, 'x') * d;
    }

    if (objects[i].type == PLAYER1 || objects[i].type == PLAYER2) {
      double dy = objects[i].y - objects[0].y;
      double dx = objects[i].x - objects[0].x;
      double r2 = fabs(dy)*fabs(dy) + fabs(dx)*fabs(dx);
      double r = sqrt(r2);
      double g = -2 / r2;
      double unity = dy / r;
      double unitx = dx / r;
      objects[i].vely += g * unity * d;
      objects[i].velx += g * unitx * d;

      if (r < 0.5) {
        objects[i] = destroy(objects[i]);
      }

      dy = objects[i].y - objects[3-i].y;
      dx = objects[i].x - objects[3-i].x;
      r2 = fabs(dy)*fabs(dy) + fabs(dx)*fabs(dx);
      r = sqrt(r2);

      if (r < 1) {
        objects[i] = destroy(objects[i]);
        objects[3-i] = destroy(objects[3-i]);
      }

      double velxy = total_vel(objects[i]);
      if (velxy > 1) {
        objects[i].vely /= velxy;
        objects[i].velx /= velxy;
      }
    }

    if (objects[i].type == BULLET) {
      objects[i].score -= delta;
      if (objects[i].score < 0) {
        objects[i] = destroy(objects[i]);
      }

      for (int j=1; j<=2; j++) {
        double dy = objects[i].y - objects[j].y;
        double dx = objects[i].x - objects[j].x;
        double r = sqrt(fabs(dy)*fabs(dy) + fabs(dx)*fabs(dx));
        if (r < 1) {
          objects[j] = destroy(objects[j]);
          objects[i] = destroy(objects[i]);
          objects[3-j].score += 200;
        }
      }
    }

    objects[i].y += objects[i].vely * d;
    objects[i].x += objects[i].velx * d;

    if (objects[i].y >= 2*winh-2) { objects[i].y -= 2*winh-4; }
    else if (objects[i].y <= 2) { objects[i].y += 2*winh-4; }
    if (objects[i].x >= winw-1) { objects[i].x -= winw-2; }
    else if (objects[i].x <= 1) { objects[i].x += winw-2; }
  }
}


/* UPDATE SCREEN
 * Clears game screen and redraws new positions of all game objects
 * Updates dynamic parts of HUDs, including animations and status indicators
 */

void update_screen(WINDOW *game, WINDOW *ui1, WINDOW *ui2, GameObject *objects) {
  werase(game);
  box(game, 0, 0);

  mvwprintw(game, 0, 4, "┤ SPACEWAR! ├");
  for (int i=0; i < 5; i++) {
    wcolour(game, 3);
    mvwprintw(game, ((objects+i)->y3)/2, (objects+i)->x3, "%lc", charoftype((objects+i)->type));
    wcolour(game, 2);
    mvwprintw(game, ((objects+i)->y2)/2, (objects+i)->x2, "%lc", charoftype((objects+i)->type));
    mvwprintw(game, ((objects+i)->y1)/2, (objects+i)->x1, "%lc", charoftype((objects+i)->type));
    wcolour(game, 1);
    mvwprintw(game, ((objects+i)->y)/2, (objects+i)->x, "%lc", charoftype((objects+i)->type));
  }
  wnoutrefresh(game);

  mvwprintw(ui1, 2, 16, "%05d", objects[1].score);
  mvwprintw(ui2, 2, 16, "%05d", objects[2].score);

  if (objects[3].type == ERR) {
    mvwprintw(ui1, 7, 6, "!");
    mvwprintw(ui1, 16, 19, "╭╮");
    mvwprintw(ui1, 17, 19, "├┤");
    mvwprintw(ui1, 18, 19, "└┘");
    mvwprintw(ui1, 17, 23, "READY");
  }
  else {
    if (objects[3].score > INT_MAX/2) {
      mvwprintw(ui1, 7, 6, ".");
      mvwprintw(ui1, 16, 19, "  ");
      mvwprintw(ui1, 17, 19, "  ");
      mvwprintw(ui1, 18, 19, "  ");
      mvwprintw(ui1, 17, 23, "     ");
    }
    else if (objects[3].score > INT_MAX/4) {
      mvwprintw(ui1, 7, 6, ".");
      mvwprintw(ui1, 16, 19, "  ");
      mvwprintw(ui1, 17, 19, "  ");
      mvwprintw(ui1, 18, 19, "╭╮");
      mvwprintw(ui1, 17, 23, "     ");
    }
    else {
      mvwprintw(ui1, 7, 6, ".");
      mvwprintw(ui1, 16, 19, "  ");
      mvwprintw(ui1, 17, 19, "╭╮");
      mvwprintw(ui1, 18, 19, "├┤");
      mvwprintw(ui1, 17, 23, "     ");
    }
  }
  if (objects[4].type == ERR) {
    mvwprintw(ui2, 7, 6, "!");
    mvwprintw(ui2, 16, 19, "╭╮");
    mvwprintw(ui2, 17, 19, "├┤");
    mvwprintw(ui2, 18, 19, "└┘");
    mvwprintw(ui2, 17, 23, "READY");
  }
  else {
    if (objects[4].score > INT_MAX/2) {
      mvwprintw(ui2, 7, 6, ".");
      mvwprintw(ui2, 16, 19, "  ");
      mvwprintw(ui2, 17, 19, "  ");
      mvwprintw(ui2, 18, 19, "  ");
      mvwprintw(ui2, 17, 23, "     ");
    }
    else if (objects[4].score > INT_MAX/4) {
      mvwprintw(ui2, 7, 6, ".");
      mvwprintw(ui2, 16, 19, "  ");
      mvwprintw(ui2, 17, 19, "  ");
      mvwprintw(ui2, 18, 19, "╭╮");
      mvwprintw(ui2, 17, 23, "     ");
    }
    else {
      mvwprintw(ui2, 7, 6, ".");
      mvwprintw(ui2, 16, 19, "  ");
      mvwprintw(ui2, 17, 19, "╭╮");
      mvwprintw(ui2, 18, 19, "├┤");
      mvwprintw(ui2, 17, 23, "     ");
    }
  }

  if (objects[1].acc) {
    mvwprintw(ui1, 7, 16, "MAIN ENGINES");
    mvwprintw(ui1, 8, 16, " FULL POWER ");
    mvwprintw(ui1, 9, 16, "! ! !╶╴! ! !");
    mvwaddch(ui1, 12, 4, "^\"*8°"[rand()%5]);
    mvwaddch(ui1, 12, 8, "^\"*8°"[rand()%5]);
  }
  else {
    mvwprintw(ui1, 7, 16, "            ");
    mvwprintw(ui1, 8, 16, "            ");
    mvwprintw(ui1, 9, 16, "     ╶╴     ");
    mvwprintw(ui1, 12,  4, "     ");
  }
  if (objects[2].acc) {
    mvwprintw(ui2, 7, 16, "MAIN ENGINES");
    mvwprintw(ui2, 8, 16, " FULL POWER ");
    mvwprintw(ui2, 9, 16, "! ! !╶╴! ! !");
    mvwaddch(ui2, 12, 6, "^\"*8°"[rand()%5]);
  }
  else {
    mvwprintw(ui2, 7, 16, "            ");
    mvwprintw(ui2, 8, 16, "            ");
    mvwprintw(ui2, 9, 16, "     ╶╴     ");
    mvwprintw(ui2, 12,  4, "     ");
  }

  mvwprintw(ui1, 16, 2, "· · ·");
  mvwprintw(ui1, 17, 2, "· • ·");
  mvwprintw(ui1, 18, 2, "· · ·");
  mvwprintw(ui2, 16, 2, "     ");
  mvwprintw(ui2, 17, 2, "  •  ");
  mvwprintw(ui2, 18, 2, "     ");

  mvwprintw(ui1, 17+round(thrust_vector(objects[1].dir, 'y')), 4+2*round(thrust_vector(objects[1].dir, 'x')), "%lc", charofdir(objects[1].dir));
  mvwprintw(ui2, 17+round(thrust_vector(objects[2].dir, 'y')), 4+2*round(thrust_vector(objects[2].dir, 'x')), "%lc", charofdir(objects[2].dir));

  mvwprintw(ui1, 17, 9, "%03d°", objects[1].dir * 45);
  mvwprintw(ui2, 17, 9, "%03d°", objects[2].dir * 45);

  if (total_vel(objects[1]) > 0.995) {
    mvwprintw(ui1, 23, 20, "100.000%%");
  }
  else {
    mvwprintw(ui1, 23, 20, "%07.3f%%", total_vel(objects[1]) * 100);
  }
  if (total_vel(objects[2]) > 0.995) {
    mvwprintw(ui2, 23, 20, "100.000%%");
  }
  else {
    mvwprintw(ui2, 23, 20, "%07.3f%%", total_vel(objects[2]) * 100);
  }
  mvwprintw(ui1, 24, 20, "%07.3f°", fmod(atan2(objects[1].vely, objects[1].velx) * 180/M_PI + 450, 360));
  mvwprintw(ui2, 24, 20, "%07.3f°", fmod(atan2(objects[2].vely, objects[2].velx) * 180/M_PI + 450, 360));

  wnoutrefresh(ui1);
  wnoutrefresh(ui2);

  doupdate();
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

  int gameh = 51;
  int gamew = 101;
  int uisize = 30;
  WINDOW *game = newwin(gameh, gamew, (scrh-gameh)/2, (scrw-gamew)/2);
  WINDOW *ui1 = newwin(uisize-2, uisize, (scrh-gameh)/2, (scrw-uisize)/2-69);
  WINDOW *ui2 = newwin(uisize-2, uisize, (scrh-gameh)/2, (scrw-uisize)/2+69);
  wcolour(game, 1);
  wcolour(ui1, 1);
  wcolour(ui2, 1);

  create_ui(ui1, 1);
  create_ui(ui2, 2);

  // Initiate array of all game objects (the black hole, the players, and empty spots for torpedoes to spawn);
  GameObject game_objects[5];
  game_objects[0] = new_gameobject(BLACKHOLE, (double)gameh+0.5, (double)gamew/2, N, 0);
  game_objects[1] = new_gameobject(PLAYER1, P1_Y, P1_X, N, 0);
  game_objects[2] = new_gameobject(PLAYER2, P2_Y, P2_X, S, 0);
  game_objects[3] = err_gameobject();
  game_objects[4] = err_gameobject();

  // Start timing to calculate frametimes
  int frame = 0;
  int delta = 0;
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (true) {    
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

      for (int i = 0; i < 8 && keys_pressed[i] != ERR; i++) {
        if (keys_pressed[i] == 'w') {
          game_objects[1].acc = !game_objects[1].acc;
        }
        else if (keys_pressed[i] == 'a') {
          game_objects[1].dir -= 1;
          if(game_objects[1].dir < 0) { game_objects[1].dir += 8;}
        }
        else if (keys_pressed[i] == 'd') {
          game_objects[1].dir += 1;
          game_objects[1].dir %= 8;
        }
        else if (keys_pressed[i] == 's') {
          if (game_objects[3].type == ERR) {
            game_objects[3] = new_gameobject(BULLET, game_objects[1].y+2*thrust_vector(game_objects[1].dir, 'y'), game_objects[1].x+2*thrust_vector(game_objects[1].dir, 'x'), N, INT_MAX);
            game_objects[3].vely = game_objects[1].vely + 0.5*thrust_vector(game_objects[1].dir, 'y');
            game_objects[3].velx = game_objects[1].velx + 0.5*thrust_vector(game_objects[1].dir, 'x');
          }
        }
        else if (keys_pressed[i] == KEY_UP) {
          game_objects[2].acc = !game_objects[2].acc;
        }
        else if (keys_pressed[i] == KEY_LEFT) {
          game_objects[2].dir -= 1;
          if(game_objects[2].dir < 0) { game_objects[2].dir += 8;}
        }
        if (keys_pressed[i] == KEY_RIGHT) {
          game_objects[2].dir += 1;
          game_objects[2].dir %= 8;
        }
        else if (keys_pressed[i] == KEY_DOWN) {
          if (game_objects[4].type == ERR) {
            game_objects[4] = new_gameobject(BULLET, game_objects[2].y+2*thrust_vector(game_objects[2].dir, 'y'), game_objects[2].x+2*thrust_vector(game_objects[2].dir, 'x'), N, INT_MAX);
            game_objects[4].vely = game_objects[2].vely + 0.5*thrust_vector(game_objects[2].dir, 'y');
            game_objects[4].velx = game_objects[2].velx + 0.5*thrust_vector(game_objects[2].dir, 'x');
          }
        }
      }

      update_physics(game, game_objects, delta, frame);
      update_screen(game, ui1, ui2, game_objects);

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

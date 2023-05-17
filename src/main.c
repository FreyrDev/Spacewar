#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

// This is just because I was writing it on Windows and the error was annoying me
#ifndef CLOCK_MONOTONIC_RAW
  #define CLOCK_MONOTONIC_RAW 0
#endif

#define PHYSICS_SPEED 1.0
#define FRAMERATE 50

enum Type {
  BLACKHOLE,
  PLAYER1,
  PLAYER2,
  BULLET
};

enum Dir {
  N, NE, E, SE, S, SW, W, NW
};

typedef struct GameObject {
  enum Type type;
  double y, x, y1, x1, y2, x2, y3, x3, vely, velx;
  int acc, dir, score;
} GameObject;

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

void wcolour(WINDOW *win,int col) { wattron(win, COLOR_PAIR(col+1)); }
void colour(int col) { wcolour(stdscr, col); }

void create_ui(WINDOW *ui, int player) {
  box(ui, 0, 0);
  mvwprintw(ui, 0, 4, "┤ PLAYER %d ├", player);
  mvwprintw(ui, 4, 6, "┌───┤ ENGINE ├───┐ ┌────────┐");
  mvwprintw(ui, 5, 6, "│                │ │ooOo00oo│");
  mvwprintw(ui, 6, 6, "│                │ │00OoO0Oo│");
  mvwprintw(ui, 7, 6, "│                │ │ / || \\ │");
  mvwprintw(ui, 8, 6, "│                │ │ o 88 o │");
  mvwprintw(ui, 9, 6, "└────────────────┘ └────────┘");

  mvwprintw(ui, 12, 6, "┌────┤ STATUS READOUTS ├────┐");
  mvwprintw(ui, 13, 6, "│                           │");
  mvwprintw(ui, 14, 6, "│ Current Velocity:         │");
  mvwprintw(ui, 15, 6, "│                           │");
  mvwprintw(ui, 16, 6, "│ Current Heading:          │");
  mvwprintw(ui, 17, 6, "│                           │");
  mvwprintw(ui, 18, 6, "└───────────────────────────┘");

  if (player == 1) {
    mvwprintw(ui, 20, 6, "  /!\\  ");
    mvwprintw(ui, 21, 6, " (   ) ");
    mvwprintw(ui, 22, 6, " / θ \\");
    mvwprintw(ui, 23, 6, "( ___ )");
    mvwprintw(ui, 24, 6, "/_\\ /_\\");
  }
  else {
    mvwprintw(ui, 20, 6, " __!__ ");
    mvwprintw(ui, 21, 6, "(_   _)");
    mvwprintw(ui, 22, 6, "  |θ|  ");
    mvwprintw(ui, 23, 6, "  ( )  ");
    mvwprintw(ui, 24, 6, "  /_\\ ");
  }
  wrefresh(ui);
}

int get_delta(struct timespec *start, struct timespec *end) {
  int result = 0;
  result += end->tv_sec - start->tv_sec;
  result *= 1000000000;
  result += end->tv_nsec - start->tv_nsec;
}

wchar_t charof(enum Type type) {
  switch (type) {
    case PLAYER1:   return L'A';
    case PLAYER2:   return L'T';
    case BULLET:    return L'.';
    case BLACKHOLE: return L'@';
    default:        return L' ';
  }
}

wchar_t charofdir(enum Dir dir) {
  switch (dir) {
    case N:  return L'↑';
    case NE: return L'↗';
    case E:  return L'→';
    case SE: return L'↘';
    case S:  return L'↓';
    case SW: return L'↙';
    case W:  return L'←';
    case NW: return L'↖';
  }
}

GameObject new_gameobject(enum Type type, double y, double x, enum Dir dir, int score) {
  return (GameObject){type, y, x, y, x, y, x, y, x, 0, 0, 0, dir, score};
}

double total_vel(GameObject *object) {
  return object->vely*object->vely + object->velx*object->velx;
}

double thrust_vector(int object_dir, char axis) {
  double magnitude;
  double direction;

  switch (object_dir%2) {
    case 0: magnitude = 1;         break;
    case 1: magnitude = 1/sqrt(2); break;
  }
  
  if (axis == 'y') {
    switch (object_dir) {
      case N:
      case NE:
      case NW:
        direction = -1;
        break;
      case S:
      case SE:
      case SW:
        direction = 1;
        break;
      case E:
      case W:
        direction = 0;
        break;
    }
  }
  else {
    switch (object_dir) {
      case W:
      case NW:
      case SW:
        direction = -1;
        break;
      case E:
      case NE:
      case SE:
        direction = 1;
        break;
      case N:
      case S:
        direction = 0;
        break;
    }
  }

  return magnitude * direction;
}

GameObject destroy(GameObject player) {
  if (player.type == PLAYER1) {
    return new_gameobject(PLAYER1, 76.5, 25.5, N, player.score-50);
  }
  else {
    return new_gameobject(PLAYER2, 26.5, 75.5, S, player.score-50);
  }
}

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
      double r2  = fabs(dy)*fabs(dy) + fabs(dx)*fabs(dx);
      double r = sqrt(r2);
      double g = -2 / r2;
      double unity = dy / r;
      double unitx = dx / r;
      objects[i].vely += g * unity * d;
      objects[i].velx += g * unitx * d;

      if (r < 0.1) {
        objects[i] = destroy(objects[i]);
      }

      double velxy = total_vel(objects+i);
      if (velxy > 1) {
        objects[i].vely /= velxy;
        objects[i].velx /= velxy;
      }
    }

    if (objects[i].type == BULLET) {
      objects[i].score -= delta;
      if (objects[i].score < 0) {
        objects[i] = new_gameobject(ERR, 0, 0, 0, 0);
      }

      for (int j=1; j<=2; j++) {
        double dy = objects[i].y - objects[j].y;
        double dx = objects[i].x - objects[j].x;
        double r = sqrt(fabs(dy)*fabs(dy) + fabs(dx)*fabs(dx));
        if (r < 1) {
          objects[j] = destroy(objects[j]);
          objects[i] = new_gameobject(ERR, 0, 0, 0, 0);
          objects[-j+3].score += 200;
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

void update_screen(WINDOW *game, WINDOW *ui1, WINDOW *ui2, GameObject *objects) {
  werase(game);
  box(game, 0, 0);

  mvwprintw(game, 0, 4, "┤ SPACEWAR! ├");
  for (int i=0; i < 5; i++) {
    wcolour(game, 2);
    mvwprintw(game, ((objects+i)->y3)/2, (objects+i)->x3, "%lc", charof((objects+i)->type));
    wcolour(game, 1);
    mvwprintw(game, ((objects+i)->y2)/2, (objects+i)->x2, "%lc", charof((objects+i)->type));
    mvwprintw(game, ((objects+i)->y1)/2, (objects+i)->x1, "%lc", charof((objects+i)->type));
    wcolour(game, 0);
    mvwprintw(game, ((objects+i)->y)/2, (objects+i)->x, "%lc", charof((objects+i)->type));
  }
  wnoutrefresh(game);

  if (objects[1].acc) {
    mvwprintw(ui1, 6, 9, "MAIN ENGINES");
    mvwprintw(ui1, 7, 9, " FULL POWER ");
  }
  else {
    mvwprintw(ui1, 6, 9, "            ");
    mvwprintw(ui1, 7, 9, "            ");
  }
  if (objects[2].acc) {
    mvwprintw(ui2, 6, 9, "MAIN ENGINES");
    mvwprintw(ui2, 7, 9, " FULL POWER ");
  }
  else {
    mvwprintw(ui2, 6, 9, "            ");
    mvwprintw(ui2, 7, 9, "            ");
  }

  mvwprintw(ui1, 14, 26, "%07.4f\%", total_vel(objects+1)*100);
  mvwprintw(ui2, 14, 26, "%07.4f\%", total_vel(objects+2)*100);

  mvwprintw(ui1, 16, 26, "%03d° %lc", objects[1].dir*45, charofdir(objects[1].dir));
  mvwprintw(ui2, 16, 26, "%03d° %lc", objects[2].dir*45, charofdir(objects[2].dir));
  
  mvwprintw(ui1, 17, 26, "%d", objects[1].score);
  mvwprintw(ui2, 17, 26, "%d", objects[2].score);

  wnoutrefresh(ui1);
  wnoutrefresh(ui2);

  doupdate();
}

int main() {
  setup();

  int scrh;
  int scrw;
  getmaxyx(stdscr, scrh, scrw);

  int gameh = 51;
  int gamew = 101;
  int uiw = 41;
  WINDOW *game = newwin(gameh, gamew, (scrh-gameh)/2, (scrw-gamew)/2);
  WINDOW *ui1 = newwin(51, uiw, (scrh-gameh)/2, (scrw-uiw)/2-72);
  WINDOW *ui2 = newwin(51, uiw, (scrh-gameh)/2, (scrw-uiw)/2+72);
  wcolour(game, 0);
  wcolour(ui1, 0);
  wcolour(ui2, 0);

  create_ui(ui1, 1);
  create_ui(ui2, 2);

  // Initiate array of all game objects (the black hole, the players, and empty spots for bullets to spawn);
  GameObject game_objects[5];
  game_objects[0] = new_gameobject(BLACKHOLE, (double)gameh+0.5, (double)gamew/2, N, 0);
  game_objects[1] = new_gameobject(PLAYER1, 76.5, 25.5, N, 0);
  game_objects[2] = new_gameobject(PLAYER2, 26.5, 75.5, S, 0);
  game_objects[3] = new_gameobject(ERR, 0, 0, 0, 0);
  game_objects[4] = new_gameobject(ERR, 0, 0, 0, 0);

  // Start timing to ensure consitent frame rate
  int frame = 0;
  int delta = 0;
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (true) {    
    if (delta >= 1000000000/FRAMERATE) { // Frametime set to 20 million nanoseconds (50 FPS)
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
          // game_objects[1].dir %= 8;
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

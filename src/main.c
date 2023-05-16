#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

// This is just because I was writing it on Windows and the error was annoying me
#ifndef CLOCK_MONOTONIC_RAW
  #define CLOCK_MONOTONIC_RAW 0
#endif

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
  float y, x, y1, x1, y2, x2, y3, x3, vely, velx;
  int acc, dir;
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

void create_ui(WINDOW *ui, char title[]) {
  box(ui, 0, 0);
  mvwprintw(ui, 0, 4, title);
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
  wrefresh(ui);
}

int get_delta(struct timespec *start, struct timespec *end) {
  int result = 0;
  result += end->tv_sec - start->tv_sec;
  result *= 1000000000;
  result += end->tv_nsec - start->tv_nsec;
}

char charof(enum Type type) {
  if (type == PLAYER1) {
    return '1';
  }
  if (type == PLAYER2) {
    return '2';
  }
  else if (type == BULLET) {
    return '*';
  }
  else if (type == BLACKHOLE) {
    return '@';
  }
  else {
    return ' ';
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

GameObject new_gameobject(enum Type type, float y, float x, enum Dir dir) {
  return (GameObject){type, y, x, y, x, y, x, y, x, 0, 0, 0, dir};
}

float total_vel(GameObject *object) {
  return object->vely*object->vely + object->velx*object->velx;
}

void update_physics(WINDOW* game_win, GameObject *objects) {
  int winh;
  int winw;
  getmaxyx(game_win, winh, winw);

  for (int i=0; i < 16; i++) {
    objects[i].y3 = objects[i].y2;
    objects[i].x3 = objects[i].x2;
    objects[i].y2 = objects[i].y1;
    objects[i].x2 = objects[i].x1;
    objects[i].y1 = objects[i].y;
    objects[i].x1 = objects[i].x;

    if (objects[i].acc) {
      switch (objects[i].dir) {
        case N:  objects[i].vely -= 0.005;  break;
        case NE: objects[i].vely -= 0.0035;
                 objects[i].velx += 0.0035; break;
        case E:  objects[i].velx += 0.005;  break;
        case SE: objects[i].vely += 0.0035;
                 objects[i].velx += 0.0035; break;
        case S:  objects[i].vely += 0.005;  break;
        case SW: objects[i].vely += 0.0035;
                 objects[i].velx -= 0.0035; break;
        case W:  objects[i].velx -= 0.005;  break;
        case NW: objects[i].vely -= 0.0035;
                 objects[i].velx -= 0.0035; break;
      }
    }

    if (objects[i].type == PLAYER1 || objects[i].type == PLAYER2) {
      float dy = objects[i].y - objects[0].y;
      float dx = objects[i].x - objects[0].x;
      float r2  = fabs(dy)*fabs(dy) + fabs(dx)*fabs(dx);
      float r = sqrt(r2);
      float g = -2 / r2;
      float unity = dy / r;
      float unitx = dx / r;
      objects[i].vely += g * unity; 
      objects[i].velx += g * unitx;

      if (r < 0.5) {
        if (objects[i].type == PLAYER1) {
          objects[i] = new_gameobject(PLAYER1, 75.5, 25.5, N);
        }
        else {
          objects[i] = new_gameobject(PLAYER2, 26.5, 76.5, S);
        }
      }

      float velxy = total_vel(objects+i);
      if (velxy > 1) {
        objects[i].vely /= velxy;
        objects[i].velx /= velxy;
      }
    }

    objects[i].y += objects[i].vely;
    objects[i].x += objects[i].velx;

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
  for (int i=0; i < 16; i++) {
    wcolour(game, 2);
    mvwaddch(game, ((objects+i)->y3)/2, (objects+i)->x3, charof((objects+i)->type));
    wcolour(game, 1);
    mvwaddch(game, ((objects+i)->y2)/2, (objects+i)->x2, charof((objects+i)->type));
    mvwaddch(game, ((objects+i)->y1)/2, (objects+i)->x1, charof((objects+i)->type));
    wcolour(game, 0);
    mvwaddch(game, ((objects+i)->y)/2, (objects+i)->x, charof((objects+i)->type));
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

  create_ui(ui1, "┤ PLAYER 1 ├");
  create_ui(ui2, "┤ PLAYER 2 ├");

  // Initiate array of all game objects (the black hole, the players, and empty spots for bullets to spawn);
  GameObject game_objects[16];
  for (int i=0; i < 16; i++) {
    game_objects[i] = new_gameobject(ERR, 0, 0, 0);
  }
  game_objects[0] = new_gameobject(BLACKHOLE, gameh+0.5, gamew/2+0.5, N);
  game_objects[1] = new_gameobject(PLAYER1, 75.5, 25.5, N);
  game_objects[2] = new_gameobject(PLAYER2, 26.5, 76.5, S);

  // Start timing to ensure consitent frame rate
  int delta = 0;
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (true) {    
    if (delta >= 33333333) { // Frametime set to 33.3 million nanoseconds (30 FPS)
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
        if(keys_pressed[i] == 'w') {
          game_objects[1].acc = !game_objects[1].acc;
        }
        if(keys_pressed[i] == 'a') {
          game_objects[1].dir -= 1;
          // game_objects[1].dir %= 8;
          if(game_objects[1].dir < 0) { game_objects[1].dir += 8;}
        }
        if(keys_pressed[i] == 'd') {
          game_objects[1].dir += 1;
          game_objects[1].dir %= 8;
        }
        if(keys_pressed[i] == KEY_UP) {
          game_objects[2].acc = !game_objects[2].acc;
        }
        if(keys_pressed[i] == KEY_LEFT) {
          game_objects[2].dir -= 1;
          // game_objects[2].dir %= 8;
          if(game_objects[2].dir < 0) { game_objects[2].dir += 8;}
        }
        if(keys_pressed[i] == KEY_RIGHT) {
          game_objects[2].dir += 1;
          game_objects[2].dir %= 8;
        }
      }

      update_physics(game, game_objects);
      update_screen(game, ui1, ui2, game_objects);

      delta = 0;
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

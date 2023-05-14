#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

// gcc -o out/main src/main.c -lncursesw -lm && ./out/main

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
  float y, x, vely, velx;
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

  init_color(COLOR_BLUE, 400, 850, 975);
  // init_color(COLOR_CYAN, 250, 375, 150);
  // init_color(COLOR_GREEN, 125, 150, 100);
  // init_color(COLOR_YELLOW, 75, 100, 75);
  init_color(COLOR_BLACK, 50, 50, 50);
  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  // init_pair(2, COLOR_CYAN, COLOR_BLACK);
  // init_pair(3, COLOR_GREEN, COLOR_BLACK);
  // init_pair(4, COLOR_YELLOW, COLOR_BLACK);
}

void wcolour(WINDOW *win,int col) { wattron(win, COLOR_PAIR(col+1)); }
void colour(int col) { wcolour(stdscr, col); }

void create_ui(WINDOW *ui, char title[]) {
  box(ui, 0, 0);
  mvwprintw(ui, 0, 4, title);
  mvwprintw(ui, 3, 5, "THRUSTERS");
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

void update_physics(WINDOW* game_win, GameObject *objects) {
  int winh;
  int winw;
  getmaxyx(game_win, winh, winw);

  for (int i=0; i < 16; i++) {
    if (objects[i].acc) {
      switch (objects[i].dir) {
        case N:  objects[i].vely -= 0.01;  break;
        case NE: objects[i].vely -= 0.007;
                 objects[i].velx += 0.007; break;
        case E:  objects[i].velx += 0.01;  break;
        case SE: objects[i].vely += 0.007;
                 objects[i].velx += 0.007; break;
        case S:  objects[i].vely += 0.01;  break;
        case SW: objects[i].vely += 0.007;
                 objects[i].velx -= 0.007; break;
        case W:  objects[i].velx -= 0.01;  break;
        case NW: objects[i].vely -= 0.007;
                 objects[i].velx -= 0.007; break;
      }
    }

    if (objects[i].type == PLAYER1 || objects[i].type == PLAYER2) {
      float dy = objects[i].y - objects[0].y;
      float dx = objects[i].x - objects[0].x;
      float r2  = fabs(dy)*fabs(dy) + fabs(dx)*fabs(dx);
      float r = sqrt(r2);
      float g = -1 / r2;
      float unity = dy / r;
      float unitx = dx / r;
      objects[i].vely += g * unity; 
      objects[i].velx += g * unitx;

      float total_vel = objects[i].vely*objects[i].vely + objects[i].velx*objects[i].velx;
      if (total_vel > 1) {
        objects[i].vely /= total_vel;
        objects[i].velx /= total_vel;
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
  wclear(game);
  box(game, 0, 0);

  mvwprintw(game, 0, 4, "┤ SPACEWAR! ├");
  for (int i=0; i < 16; i++) {
    mvwaddch(game, ((objects+i)->y)/2, (objects+i)->x, charof((objects+i)->type));
  }
  wnoutrefresh(game);
  if (objects[1].acc) { mvwprintw (ui1, 2, 5, "THRUSTING"); }
  else                { mvwprintw (ui1, 2, 5, "         "); }
  if (objects[2].acc) { mvwprintw (ui2, 2, 5, "THRUSTING"); }
  else                { mvwprintw (ui2, 2, 5, "         "); }

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
    game_objects[i] = (GameObject){ERR, 0, 0, 0, 0, 0, 0};
  }
  game_objects[0] = (GameObject){BLACKHOLE, gameh, gamew/2, 0, 0, 0, N};
  game_objects[1] = (GameObject){PLAYER1, 75, 25, 0, 0, 0, N};
  game_objects[2] = (GameObject){PLAYER2, 25, 75, 0, 0, 0, S};

  // Start timing to ensure consitent frame rate
  int delta = 0;
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (true) {    
    if (delta >= 66666666) { // Frametime set to 66.6 million nanoseconds (15 FPS)
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);

      int keys_pressed[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
      int ch;
      int ch_num = 0;
      while ((ch = getch()) != ERR) {
        keys_pressed[ch_num] = ch;
        ch_num++;
      }
      keys_pressed[ch_num] = ERR;

      move(0,0);
      clrtoeol();
      for (int i = 0; i < 8 && keys_pressed[i] != ERR; i++) {
        if(keys_pressed[i] == 'w') {
          game_objects[1].acc = !game_objects[1].acc;
        }
        if(keys_pressed[i] == 'a') {
          game_objects[1].dir -= 1;
          game_objects[1].dir %= 8;
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
          game_objects[2].dir %= 8;
          if(game_objects[2].dir < 0) { game_objects[2].dir += 8;}
        }
        if(keys_pressed[i] == KEY_RIGHT) {
          game_objects[2].dir += 1;
          game_objects[2].dir %= 8;
        }
      }

      update_physics(game, game_objects);
      update_screen(game, ui1, ui2, game_objects);

      // mvprintw(0,0, "%f,%f", game_objects[1].y,game_objects[1].x);
      // mvprintw(1,0, "%f,%f", game_objects[2].y,game_objects[2].x);
      // refresh();

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

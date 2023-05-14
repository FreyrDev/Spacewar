#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>

// gcc -o out/main src/main.c -lncursesw && ./out/main

// This is just because I was writing it on Windows and the error was annoying me
#ifndef CLOCK_MONOTONIC_RAW
  #define CLOCK_MONOTONIC_RAW 0
#endif

enum Type {
  PLAYER,
  BULLET,
  STAR
};

typedef struct GameObject {
  enum Type type;
  float y, x, vely, velx, dir, mass;
} GameObject;

void setup() {
  setlocale(LC_ALL, "");
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  start_color();
  curs_set(0);
  refresh();

  init_color(COLOR_BLUE, 400, 850, 975);
  init_color(COLOR_CYAN, 250, 375, 150);
  init_color(COLOR_GREEN, 125, 150, 100);
  init_color(COLOR_YELLOW, 75, 100, 75);
  init_color(COLOR_BLACK, 50, 50, 50);
  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  init_pair(2, COLOR_CYAN, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
}

void wcolour(WINDOW *win,int col) { wattron(win, COLOR_PAIR(col+1)); }
void colour(int col) { wcolour(stdscr, col); }

GameObject create_gravity_source(WINDOW *win, int y, int x, int mass, char ch) {
  mvwaddch(win, y, x, ch);
  GameObject gravity = {STAR, y, x, 0, 0, 0, mass};
  return gravity;
}

int get_delta(struct timespec *start, struct timespec *end) {
  int result = 0;
  result += end->tv_sec - start->tv_sec;
  result *= 1000000000;
  result += end->tv_nsec - start->tv_nsec;
}

char charof(enum Type type) {
  if (type == PLAYER) {
    return '1';
  }
  else if (type == BULLET) {
    return '*';
  }
  else {
    return '@';
  }
}

void update_physics(GameObject *objects, int num) {
  int i = 0;
  while (i < num) {
    (objects+i)->y += (objects+i)->vely;
    (objects+i)->x += (objects+i)->velx;
    i++;
  }
}

void update_screen(WINDOW *game_win, GameObject *objects, int num) {
  wclear(game_win);
  box(game_win, 0, 0);
  int i = 0;
  while (i < num) {
    mvwaddch(game_win, (objects+i)->y, (objects+i)->x, charof((objects+i)->type));
    i++;
  }
}

int main() {
  setup();
  int delta = 0;

  int scrh;
  int scrw;
  getmaxyx(stdscr, scrh, scrw);

  int gameh = 51;
  int gamew = 101;
  WINDOW *game = newwin(gameh, gamew, (scrh-gameh)/2, (scrw-gamew)/2);
  wcolour(game, 0);

  GameObject black_hole = create_gravity_source(game, gameh/2,gamew/2,100,'@');
  GameObject p1 = {PLAYER, 10, 10, 0.5, 1, 0, 1};
  GameObject game_objects[64] = {black_hole, p1};

  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (true) {    
    if (delta >= 33333333) {
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);

      update_physics(game_objects, 2);
      update_screen(game, game_objects, 2);
      wnoutrefresh(game);

      mvprintw(0,0, "%d", 1000000000/delta);
      wnoutrefresh(stdscr);
      doupdate();

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

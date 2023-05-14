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
  PLAYER1,
  PLAYER2,
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
  nodelay(stdscr, TRUE);
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
  else {
    return '@';
  }
}

void update_physics(WINDOW* game_win, GameObject *objects, int num) {
  int winh;
  int winw;
  getmaxyx(game_win, winh, winw);

  int i = 0;
  while (i < num) {
    objects[i].y += objects[i].vely;
    objects[i].x += objects[i].velx;
    if (objects[i].y >= winh-1) { objects[i].y -= winh-2; }
    else if (objects[i].y <= 1) { objects[i].y += winh-2; }
    if (objects[i].x >= winw-1) { objects[i].x -= winw-2; }
    else if (objects[i].x <= 1) { objects[i].x += winw-2; }
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

  GameObject black_hole = {STAR, gameh/2, gamew/2, 0, 0, 0, 100};
  GameObject p1 = {PLAYER1, 38, 20, 0, 0, 0, 1};
  GameObject p2 = {PLAYER2, 12, 80, 0, 0, 180, 1};
  GameObject game_objects[64] = {black_hole, p1, p2};

  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  while (true) {    
    if (delta >= 33333333) {
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
          game_objects[1].vely -= 0.005;
        }
        if(keys_pressed[i] == 'a') {
          game_objects[1].velx -= 0.005;
        }
        if(keys_pressed[i] == 's') {
          game_objects[1].vely += 0.005;
        }
        if(keys_pressed[i] == 'd') {
          game_objects[1].velx += 0.005;
        }
      }

      update_physics(game, game_objects, 3);
      update_screen(game, game_objects, 3);
      wrefresh(game);

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

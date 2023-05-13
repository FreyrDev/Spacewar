#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>

// gcc -o out/main src/main.c -lncursesw && ./out/main

enum Type {
  PLAYER,
  BULLET,
  STAR
};

typedef struct GameObject {
  enum Type type;
  float y, x, vely, velx, mass, dir;
} GameObject;

typedef struct Player {
  float y, x, vely, velx, dir;
} Player;

typedef struct Gravity {
  int y, x, str;
} Gravity;

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

Gravity create_gravity_source(WINDOW *win, int y, int x, int str, char ch) {
  mvwaddch(win, y, x, ch);
  Gravity gravity = { y, x, str };
  return gravity;
}

int get_delta(struct timespec *start, struct timespec *end) {
  int result = 0;
  result += end->tv_sec - start->tv_sec;
  result *= 1000000000;
  result += end->tv_nsec - start->tv_nsec;
}

void update_physics(Player *p1) {
  p1->y += p1->vely;
  p1->x += p1->velx;
}

void update_screen(WINDOW *game_win, Player *p1) {
  wclear(game_win);
  box(game_win, 0, 0);
  mvwaddch(game_win, p1->y, p1->x, '1');
}

int main() {
  setup();


  int scrh;
  int scrw;
  getmaxyx(stdscr, scrh, scrw);

  int gameh = 51;
  int gamew = 101;
  WINDOW *game = newwin(gameh, gamew, (scrh-gameh)/2, (scrw-gamew)/2);
  wcolour(game, 0);
  box(game,0,0);
  Gravity black_hole = create_gravity_source(game, gameh/2,gamew/2,100,'@');
  wrefresh(game);

  Player p1 = {10, 10, 0, 1, 0};


  while(true) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    update_physics(&p1);
    update_screen(game, &p1);

    usleep(20000);
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    clrtoeol();
    mvprintw(0,0,"%d", get_delta(&start, &end));
    refresh();
    wrefresh(game);
  }

  endwin();
  return 0;
}

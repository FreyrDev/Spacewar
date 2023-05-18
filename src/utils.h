#include <curses.h>
#include <locale.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>

#ifndef MY_UTILS_H
#define MY_UTILS_H

// This is just because I was writing it on Windows and the error was annoying me
#ifndef CLOCK_MONOTONIC_RAW
  #define CLOCK_MONOTONIC_RAW 0
#endif
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

enum Type { BLACKHOLE, PLAYER1, PLAYER2, BULLET };
enum Dir  { N, NE, E, SE, S, SW, W, NW };

typedef struct GameObject {
  enum Type type;
  double y, x;
  double y1, x1;
  double y2, x2;
  double y3, x3;
  double vely, velx;
  int acc, dir, score;
} GameObject;

void wcolour(WINDOW *win, int col);
void colour(int col);
int get_delta(struct timespec *start, struct timespec *end);
GameObject new_gameobject(enum Type type, double y, double x, enum Dir dir, int score);
GameObject err_gameobject();
double total_vel(GameObject object);
double thrust_vector(int object_dir, char axis);
wchar_t charoftype(enum Type type);
wchar_t charofdir(enum Dir dir);

#endif

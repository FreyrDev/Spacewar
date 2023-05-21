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
enum Axis { Y, X };

typedef struct ObjectData {
  double y, x;
  double y1, x1;
  double y2, x2;
  double y3, x3;
  double vely, velx;
} ObjectData;

typedef struct BlackHole {
  double y, x;
} BlackHole;

typedef struct Player {
  enum Type type;
  ObjectData data;
  float temp;
  int acc, dir, score;
} Player;

typedef struct Bullet {
  enum Type type;
  ObjectData data;
  int fuse;
} Bullet;

typedef struct GameState {
  BlackHole bh;
  Player players[2];
  Bullet bullets[2];
} GameState;

void wcolour(WINDOW *win, int col);
void colour(int col);
int get_delta(struct timespec *start, struct timespec *end);
ObjectData new_objectdata(double y, double x);
Player new_player(enum Type type, double y, double x, int dir, int score);
Bullet new_bullet(double y, double x);
Bullet err_bullet();
double total_dist_squared(double dy, double dx);
double total_vel(ObjectData object);
void shift_trails(ObjectData *data);
double thrust_vector(int object_dir, enum Axis axis);
wchar_t charoftype(enum Type type);
wchar_t charofdir(enum Dir dir);

#endif

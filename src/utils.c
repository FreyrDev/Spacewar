#include "utils.h"


// So I can change the colour pairs in a slightly neather way
// wcolour() for windows and colour() for stdscr
void wcolour(WINDOW *win, int col) {
  wattron(win, COLOR_PAIR(col));
}

// So I can change the colour pairs in a slightly neather way
// wcolour() for windows and colour() for stdscr
void colour(int col) {
  wcolour(stdscr, col);
}

// Get the time delta between two timespec structs, return value in nanoseconds
int get_delta(struct timespec *start, struct timespec *end) {
  int result = 0;
  result += end->tv_sec - start->tv_sec;
  result *= 1000000000;
  result += end->tv_nsec - start->tv_nsec;
}

// Constructor function for ObjectData, so you don't have to type coords four times
ObjectData new_objectdata(double y, double x) {
  return (ObjectData){y, x, y, x, y, x, y, x, 0, 0};
}

// Constructor function for Players
Player new_player(enum Type type, double y, double x, int dir, int score) {
  return (Player){type, new_objectdata(y, x), 0, 0, dir, score};
}

// Constructor function for Bullets
Bullet new_bullet(double y, double x) {
  return (Bullet){BULLET, new_objectdata(y, x), INT_MAX};
}

// Blank Bullet with ERR type
Bullet err_bullet() {
  return (Bullet){ERR, new_objectdata(0, 0), 0};
}

// Calculates the total velocity from the component parts
double total_vel(ObjectData object) {
  return sqrt(object.vely*object.vely + object.velx*object.velx);
}

// Calculates the strength/direction of thrust along a given axis based on a facing direction
// Used for the ship engine acceletation and torpedoes' initial thrust
double thrust_vector(int object_dir, char axis) {
  double magnitude;
  double direction;

  switch (object_dir%2) {
    case 0: magnitude = 1;         break;
    case 1: magnitude = 1/sqrt(2); break;
  }
  
  if (axis == 'x') {
    object_dir += 2;
    object_dir %= 8;
  }
  
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

  return magnitude * direction;
}

// Returns the character that represents each type of GameObject 
wchar_t charoftype(enum Type type) {
  switch (type) {
    case PLAYER1:   return L'A';
    case PLAYER2:   return L'T';
    case BULLET:    return L'·';
    case BLACKHOLE: return L'@';
    default:        return L' ';
  }
}

// Returns an arrow character representing each cardinal and intercardinal direction.
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

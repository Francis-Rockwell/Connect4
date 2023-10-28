#include "Judge.h"
#include "Point.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <float.h>

class UCT_Node {
public:
  static clock_t start;
  static int width;
  static int height;
  static int noX;
  static int noY;
  bool myturn;
  int last_x;
  int last_y;
  double score;
  int visit;
  UCT_Node *parent;
  int **board;
  int *top;
  int expandable_num;
  int *expandable_child;
  UCT_Node **children;
  UCT_Node(bool _myturn, int _last_x, int _last_y, int **_board, int *_top);
  ~UCT_Node();
  int must();
  int search();
  UCT_Node *expand();
  UCT_Node *best_child();
  UCT_Node *select();
  double simulate();
};
#include "UCT_Node.h"
#include "Judge.h"
#include "Point.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <float.h>
#include <iostream>

using namespace std;

long TIMELIMIT = 2.2 * CLOCKS_PER_SEC;
double c = 0.7;
int static_board[12][12];
int static_top[12];

clock_t UCT_Node::start;
int UCT_Node::width;
int UCT_Node::height;
int UCT_Node::noX;
int UCT_Node::noY;

int **get_static_board_pointer() {
  static int *pointers[12];
  for (int i = 0; i < 12; i++)
    pointers[i] = static_board[i];
  return pointers;
}

int UCT_Node::must() {
  for (int i = 0; i < width; i++) {
    if (top[i] > 0) {
      int play_y = i;
      int play_x = top[i] - 1;
      board[play_x][play_y] = 2;
      if (machineWin(play_x, play_y, height, width, board)) {
        board[play_x][play_y] = 0;
        return play_y;
      }
      board[play_x][play_y] = 0;
    }
  }
  for (int i = 0; i < width; i++) {
    if (top[i] > 0) {
      int play_y = i;
      int play_x = top[i] - 1;
      board[play_x][play_y] = 1;
      if (userWin(play_x, play_y, height, width, board)) {
        board[play_x][play_y] = 1;
        return play_y;
      }
      board[play_x][play_y] = 0;
    }
  }
  return -1;
}

UCT_Node::UCT_Node(bool _myturn, int _last_x, int _last_y, int **_board,
                   int *_top)
    : myturn(_myturn), last_x(_last_x), last_y(_last_y) {
  score = 0.0;
  visit = 0;
  parent = nullptr;

  board = new int *[height];
  for (int i = 0; i < height; i++) {
    board[i] = new int[width];
    for (int j = 0; j < width; j++) {
      board[i][j] = _board[i][j];
    }
  }

  top = new int[width];
  for (int i = 0; i < width; i++) {
    top[i] = _top[i];
  }

  expandable_num = 0;
  expandable_child = new int[width];
  for (int i = 0; i < width; i++) {
    if (top[i] > 0) {
      expandable_child[expandable_num++] = i;
    }
  }

  children = new UCT_Node *[width];
  for (int i = 0; i < width; i++) {
    children[i] = nullptr;
  }
}

UCT_Node::~UCT_Node() {
  for (int i = 0; i < height; i++) {
    delete[] board[i];
  }
  delete[] board;
  delete[] top;

  delete[] expandable_child;

  for (int i = 0; i < width; i++) {
    if (children[i]) {
      delete children[i];
    }
  }
  delete[] children;
}

UCT_Node *UCT_Node::expand() {
  // 假设为 1 2 4 6，num=4
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      static_board[i][j] = board[i][j];
    }
  }
  for (int i = 0; i < width; i++) {
    static_top[i] = top[i];
  }

  // index = 2;
  int index = rand() % expandable_num;
  // play_y = 4;
  int play_y = expandable_child[index];
  int play_x = --static_top[play_y];

  static_board[play_x][play_y] = myturn ? 2 : 1;

  if (static_top[play_y] - 1 == noX && play_y == noY) {
    static_top[play_y]--;
  }

  UCT_Node *child = new UCT_Node(!myturn, play_x, play_y,
                                 get_static_board_pointer(), (int *)static_top);
  child->parent = this;
  // children[4]
  children[play_y] = child;

  // last = 6;
  int last = expandable_child[expandable_num - 1];
  // 1 2 4 4，num=3
  expandable_child[--expandable_num] = play_y;
  // 1 2 6 4
  expandable_child[index] = last;

  return child;
}

UCT_Node *UCT_Node::best_child() {
  double best_ucb1 = -DBL_MAX;
  UCT_Node *choice = nullptr;
  for (int i = 0; i < width; i++) {
    if (children[i]) {
      double child_ucb1 = children[i]->score / children[i]->visit +
                          c * sqrt(2 * log(visit) / children[i]->visit);
      if (child_ucb1 > best_ucb1) {
        best_ucb1 = child_ucb1;
        choice = children[i];
      }
    }
  }
  return choice;
}

UCT_Node *UCT_Node::select() {
  UCT_Node *node = this;
  while (true) {
    if (node->parent &&
        (machineWin(node->last_x, node->last_y, height, width, node->board) ||
         userWin(node->last_x, node->last_y, height, width, node->board) ||
         isTie(width, node->top))) {
      return node;
    } else {
      if (node->expandable_num) {
        return node->expand();
      } else {
        node = node->best_child();
      }
    }
  }
}

double UCT_Node::simulate() {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      static_board[i][j] = board[i][j];
    }
  }

  for (int i = 0; i < width; i++) {
    static_top[i] = top[i];
  }

  bool myturn_ = myturn;
  int play_y = last_y;
  int play_x = last_x;

  while (true) {
    if (play_x >= 0 && play_y >= 0) {
      if (machineWin(play_x, play_y, height, width,
                     get_static_board_pointer())) {
        return 1.0;
      }
      if (userWin(play_x, play_y, height, width, get_static_board_pointer())) {
        return -1.0;
      }
      if (isTie(width, (int *)static_top)) {
        return 0.0;
      }
    }

    play_y = rand() % width;
    while (static_top[play_y] <= 0) {
      play_y = rand() % width;
    }

    play_x = --static_top[play_y];
    if (static_top[play_y] - 1 == noX && play_y == noY) {
      static_top[play_y]--;
    }
    static_board[play_x][play_y] = myturn_ ? 2 : 1;
    myturn_ = !myturn_;
  }
}

int UCT_Node::search() {
  int must_play = must();
  if (must_play != -1) {
    return must_play;
  } else {
    while (clock() - start < TIMELIMIT) {
      UCT_Node *select_node = select();
      double current_score = select_node->simulate();
      while (select_node) {
        select_node->visit++;
        select_node->score +=
            select_node->myturn ? -current_score : current_score;
        select_node = select_node->parent;
      }
    }

    int play_y = 0;
    double best_rate = -DBL_MAX;
    for (int i = 0; i < width; i++) {
      if (children[i]) {
        double rate = children[i]->score / children[i]->visit;
        if (rate > best_rate) {
          best_rate = rate;
          play_y = i;
        }
      }
    }
    return play_y;
  }
}
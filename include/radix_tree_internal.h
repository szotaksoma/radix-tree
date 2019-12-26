#ifndef RADIX_TREE_INTERNAL
#define RADIX_TREE_INTERNAL

#include "radix_tree.h"

typedef struct Node {
  struct Node** children;
  char* label;
  int childCount;
  int level;
  int id;
  int isEndpoint;
  int value;
} Node;

// Internals
Node* _new_node(const char *, int, int);
void _add_child(Node*, const char*, int, int);
void _attach_child(Node*, Node*);
void _strmtc(const char*, const char*, char*);
int  _strcnt(const char*, const char*);
void _update_levels(Node*, int);
void _update_label(Node*, const char*);
void _insert(Node*, const char*, int);
Node* _find(Node*, const char*);
void _free_node(Node*);
void _print_node(Node*);
void _print_tree(Node*);

// Wrappers for Tree
void rtree_insert(const char*, int);
const int* rtree_find(const char*);
void rtree_print();
void rtree_free();
int rtree_haskey();
int rtree_keycount();

// Tree initializer
RadixTree* init_radix_tree();

#endif // !RADIX_TREE_INTERNAL
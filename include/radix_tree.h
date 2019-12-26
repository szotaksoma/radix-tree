#ifndef RADIX_TREE
#define RADIX_TREE

#define RADIX_MAX_KEY_SIZE 512

typedef struct RadixTree {
  void (*insert)(const char*, int);
  const int* (*find)(const char*);
  void (*print)(void);
  void (*free)(void);
  int (*haskey)(const char*);
  int (*keycount)(void);
} RadixTree;

RadixTree *init_radix_tree();

#endif // !RADIX_TREE
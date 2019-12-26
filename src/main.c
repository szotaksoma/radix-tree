#include <stdio.h>
#include <stdlib.h>

#include "../include/radix_tree.h"

int main() {

  RadixTree* tree = init_radix_tree();

  char key[RADIX_MAX_KEY_SIZE];
  for(int i = 0; i < RADIX_MAX_KEY_SIZE; i++) {
    key[i] = '\0';
  }

  int value = 0;
  system("clear");
  printf("Input key-value pairs [<string> <integer>]\nType '@' to finish.\n");
  while(key[0] != '@') {
    printf("> ");
    scanf("%s", key);
    if(key[0] == '@') {
      break;
    }
    scanf("%d", &value);
    if(key[0] != '@') {
      tree->insert(key, value);
      system("clear");
      tree->print();
    }
  }

  // Quit if no key was inserted
  if(tree->keycount() == 0) {
    tree->free();
    return 0;
  }

  key[0] = '\0';

  system("clear");
  tree->print();
  printf("Input a key to find in the dictionary. Type '@' to finish.\n");
  while(key[0] != '@') {
    printf("> ");
    scanf("%s", key);
    if(key[0] != '@') {
      system("clear");
      tree->print();
      const int* val_ptr = tree->find(key);
      if(val_ptr) {
        printf("> \"%s\" -> %d\n", key, *val_ptr);
      } else {
        printf("> Key \"%s\" not found.\n", key);
      }
    }
  }

  tree->free();
  return 0;
}
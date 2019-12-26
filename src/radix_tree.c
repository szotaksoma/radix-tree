#include "../include/radix_tree_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _node_count = 0;
int _key_count = 0;
Node* _head = NULL;
Tree* _tree = NULL;

void _on_error() {
  if(_tree != NULL) {
    rtree_free(_head);
    free(_tree);
  }
  exit(1);
}

Node *_new_node(const char *label, int isEndpoint, int value) {
  Node *n = (Node *)calloc(1, sizeof(Node));
  n->id = _node_count;
  if (label == NULL) {
    n->label = NULL;
  } else {
    n->label = (char *)calloc(strlen(label) + 1, sizeof(char));
    strcpy(n->label, label);
  }
  n->childCount = 0;
  n->level = 0;
  n->isEndpoint = isEndpoint;
  n->value = value;
  _node_count++;
  return n;
}

void _add_child(Node *target, const char *label, int isEndpoint, int value) {
  target->childCount++;
  if (target->childCount == 1) {
    target->children = (Node **)calloc(1, sizeof(Node *));
  } else {
    Node **ptr = realloc(target->children, sizeof(Node*) * target->childCount);
    if (ptr == NULL) {
      printf("FATAL: Failed to reallocate children array of node #%d\n", target->id);
      _on_error();
    } else {
      target->children = ptr;
    }
  }
  Node *child = _new_node(label, isEndpoint, value);
  child->level = target->level + 1;
  target->children[target->childCount - 1] = child;
}

void _attach_child(Node *target, Node *child) {
  target->childCount++;
  if (target->childCount == 1) {
    target->children = (Node **)malloc(sizeof(Node *));
  } else {
    Node **ptr = realloc(target->children, sizeof(Node *) * target->childCount);
    if (ptr == NULL) {
      printf("FATAL: Failed to reallocate children array of node #%d\n", target->id);
      _on_error();
    } else {
      target->children = ptr;
    }
  }
  child->level = target->level + 1;
  target->children[target->childCount - 1] = child;
}

// Writes the initial matching part of 'a' and 'b' to 'result'
void _strmtc(const char *a, const char *b, char *result) {
  int i = 0;
  while (a[i] == b[i] && a[i] != 0) {
    i++;
  }
  if (i > 0) {
    strncpy(result, a, i);
  }
}

// Returns 1 if 'a' contains 'b', 0 otherwise
int _strcnt(const char *a, const char *b) {
  int i = 0;
  for (i = 0; a[i] != 0 && b[i] != 0; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  if (a[i] == b[i]) {
    return 1;
  }
  return a[i] == 0 ? 0 : 1;
}

void _update_levels(Node *n, int level) {
  n->level = level;
  for (int i = 0; i < n->childCount; i++) {
    _update_levels(n->children[i], level + 1);
  }
}

void _update_label(Node *n, const char *label) {
  free((void *)n->label);
  n->label = (char *)calloc(strlen(label) + 1, sizeof(char));
  strcpy(n->label, label);
}

void _insert(Node *n, const char *key, int value) {

  if (strlen(key) == 0) {
    printf("WARNING: You are trying to insert an empty key.\n");
    return;
  }

  // Find relevant child (if any)
  Node *rel = NULL;
  int relId = 0;
  for (int i = 0; i < n->childCount; i++) {
    if (n->children[i]->label[0] == key[0]) {
      rel = n->children[i];
      relId = i;
      break;
    }
  }
  if (rel) {

    // Exact match
    if (strcmp(rel->label, key) == 0) {
      rel->isEndpoint = 1;
      rel->value = value;
      return;
    }

    const int keysize = strlen(key);
    if (keysize > RADIX_MAX_KEY_SIZE) {
      printf("FATAL: Maximum key size (%d bytes) exceeded.\n", keysize);
      _on_error();
    }
    char *match = calloc(keysize + 1, sizeof(char));
    _strmtc(rel->label, key, match);

    // Key contains label
    if (strlen(match) == strlen(rel->label)) {
      free((void *)match);
      _insert(rel, (key + strlen(rel->label)), value);
      return;
    }

    // Label contains key or part of key
    if (strlen(match) > 0) {
      Node *commonParent = _new_node(match, 0, 0);
      commonParent->level = n->level + 1;
      if(strlen(match) == strlen(key)) {
        commonParent->isEndpoint = 1;
        commonParent->value = value;
      } else {
        _add_child(commonParent, (key + strlen(match)), 1, value);
      }
      _update_levels(rel, rel->level + 1);
      char child_label[strlen(rel->label)];
      strcpy(child_label, rel->label);
      _update_label(rel, (child_label + strlen(match)));
      _attach_child(commonParent, rel);
      n->children[relId] = commonParent;
      free((void *)match);
      return;
    }

  } else {
    // No (relevant) child, insert key under 'n'
    _add_child(n, key, 1, value);
    return;
  }

  printf("ERROR: unhandled case upon inserting key \"%s\"\n", key);
}

Node* _find(Node *n, const char *key) {
  printf("> \"%s\"\n", key);
  for (int i = 0; i < n->childCount; i++) {
    if (strcmp(n->children[i]->label, key) == 0 &&
        n->children[i]->isEndpoint == 1) {
      return n->children[i];
    } else {
      if (_strcnt(key, n->children[i]->label)) {
        return _find(n->children[i], (key + strlen(n->children[i]->label)));
      }
    }
  }
  return NULL;
}

void _free_node(Node *n) {
  for (int i = 0; i < n->childCount; i++) {
    _free_node(n->children[i]);
  }
  free((void *)n->label);
  free((void *)n->children);
  free((void *)n);
}

void _print_node(Node *n) {
  for(int i = 0; i < n->level; i++) {
    printf("   ");
  }
  printf("| %s\n", n->label == NULL ? "HEAD" : n->label);
}

void _print_tree(Node *n) {
  _print_node(n);
  for (int i = 0; i < n->childCount; i++) {
    _print_tree(n->children[i]);
  }
}

// Library functions

// Return a new Tree with function pointers initialized
Tree* new_tree() {
  if(_tree == NULL) {
    _head = _new_node(NULL, 0, 0);
    _tree = (Tree*)calloc(1, sizeof(Tree));
    _tree->insert = &rtree_insert;
    _tree->find = &rtree_find;
    _tree->print = &rtree_print;
    _tree->free = &rtree_free;
    _tree->haskey = &rtree_haskey;
    _tree->keycount = &rtree_keycount;
    return _tree;
  } else {
    return NULL;
  }
}

// Add or update a value
void rtree_insert(const char* key, int value) {
  _insert(_head, key, value);
  _key_count++;
}

void rtree_print() {
  if(_head) {
    _print_tree(_head);
  }
}

// Find an element represented by 'key'. Returns the corresponding Node*.
const int* rtree_find(const char *key) {
  Node* n = _find(_head, key);
  return n ? &(n->value) : NULL;
}

// Recursively free radix tree
void rtree_free() {
  _free_node(_head);
  free(_tree);
}

// Returns 1 if 'key' is stored in the tree, 0 otherwise
int rtree_haskey(const char* key) {
  return rtree_find(key) ? 1 : 0;
}

// Returns the total number of keys stored in the tree
int rtree_keycount() {
  return _key_count;
}
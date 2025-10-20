#ifndef LISP_H
#define LISP_H

#include <stdbool.h>

typedef struct {
    bool allocated;
    enum NodeType {ATOM, LIST} node_type;
    union {
        struct {
            int atom_id;
        };
        struct {
            int left_child, right_child;
        };
    };
} Node;

typedef char AtomName[20]; // max 20 letters


extern Node nodes[100000];

extern AtomName atom_names[1000];
extern int atom_names_first_empty_index;

void draw_tree_ascii(int top_node_id);
int register_atom_id(char* atom_name);
int allocate_node(void);
void save_png_of_nodes (void);
void garbage_collect(int top_node_id );
// void deallocate_tree(int root);
// void deallocate_node(int n);

#endif // LISP_H
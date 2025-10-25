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
void print_str20(const char *s);
void print_ast_no_spaces(int top_node_id );
void save_png_of_tree (int top_node_id);

#endif // LISP_H
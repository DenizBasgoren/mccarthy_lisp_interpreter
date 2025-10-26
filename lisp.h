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

#define ATOM_MAX_LEN 19
typedef char AtomName[ATOM_MAX_LEN+1];

#define NODE_TOTAL 100000
extern Node nodes[NODE_TOTAL];

#define ATOM_TOTAL 100000
extern AtomName atom_names[ATOM_TOTAL];

extern int atom_names_first_empty_index;

void draw_tree_ascii(int top_node_id);
int register_atom_id(char* atom_name);
int allocate_node(void);
void save_png_of_nodes (void);
void garbage_collect(int top_node_id );
void print_ast_no_spaces(int top_node_id );
void save_png_of_tree (int top_node_id);

#endif // LISP_H
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
int allocate_node(bool add_to_roots);
void save_png_of_nodes (void);
void garbage_collect(void );
void print_ast_no_spaces(int top_node_id );
void save_png_of_tree (int top_node_id);
bool is_gc_root(int node_id);
void add_to_gc_roots(int node_id );
void delete_from_gc_roots(int node_id);
void init_gc_roots(void);

#endif // LISP_H
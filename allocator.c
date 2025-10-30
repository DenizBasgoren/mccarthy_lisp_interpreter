#include "lisp.h"
#include <stdio.h>
#include <stdlib.h>

// For this array, -1 means no entry, 0 to NODE_TOTAL means node_id
// that's considered a root for gc purposes.
// Same node_id can appear multiple times. Dropping a node_id
// deletes only one entry from the list.
#define GC_ROOT_TOTAL 10
int gc_roots[GC_ROOT_TOTAL];

void init_gc_roots(void) {
    for (int i = 0; i<GC_ROOT_TOTAL; i++) {
        gc_roots[i] = -1;
    }
}


void add_to_gc_roots(int node_id ) {
    for (int i = 0; i<GC_ROOT_TOTAL; i++) {
        if ( gc_roots[i] < 0) {
            gc_roots[i] = node_id;
            return;
        }
    }

    puts("Too many gc roots");
    exit(1);
}

void delete_from_gc_roots(int node_id) {
    for (int i = 0; i<GC_ROOT_TOTAL; i++) {
        if (gc_roots[i] == node_id) {
            gc_roots[i] = -1;
            return;
        }
    }

    puts("Deleting a nonexistent node");
    exit(1);
}

bool is_gc_root(int node_id) {
    for (int i = 0; i<GC_ROOT_TOTAL; i++) {
        if ( gc_roots[i] == node_id) return true;
    }
    return false;
}

void mark_allocated_recursive(int node_id) {
    nodes[node_id].allocated = true;
    if (nodes[node_id].node_type == LIST ) {
        mark_allocated_recursive(nodes[node_id].left_child);
        mark_allocated_recursive(nodes[node_id].right_child);
    }
}

void garbage_collect(void ) {
    for (int i = 0; i<NODE_TOTAL; i++) {
        nodes[i].allocated = false;
    }

    for (int i = 0; i<GC_ROOT_TOTAL; i++) {
        if (gc_roots[i] >= 0) {
            mark_allocated_recursive( gc_roots[i] );
        }
    }

}

int allocate_node(bool add_to_roots) {
    for (int i = 0; i<NODE_TOTAL; i++) {
        if ( !nodes[i].allocated ) {
            nodes[i].allocated = true;
            if ( add_to_roots) add_to_gc_roots(i);
            return i;
        }
    }

    // all allocated. do garbage collection
    garbage_collect();

    for (int i = 0; i<NODE_TOTAL; i++) {
        if ( !nodes[i].allocated ) {
            nodes[i].allocated = true;
            if ( add_to_roots) add_to_gc_roots(i);
            return i;
        }
    }

    puts("Can't allocate. Out of memory");
    exit(1);
}

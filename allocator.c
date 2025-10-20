#include "lisp.h"
#include <stdio.h>
#include <stdlib.h>

void mark_allocated_recursive(int node_id) {
    nodes[node_id].allocated = true;
    if (nodes[node_id].node_type == LIST ) {
        mark_allocated_recursive(nodes[node_id].left_child);
        mark_allocated_recursive(nodes[node_id].right_child);
    }
}

void garbage_collect(int top_node_id ) {
    for (int i = 0; i<100000; i++) {
        nodes[i].allocated = false;
    }

    mark_allocated_recursive( top_node_id );

}

int allocate_node(void) {
    for (int i = 0; i<100000; i++) {
        if ( !nodes[i].allocated ) {
            nodes[i].allocated = true;
            return i;
        }
    }

    // all allocated. do garbage collection
    garbage_collect(0);

    for (int i = 0; i<100000; i++) {
        if ( !nodes[i].allocated ) {
            nodes[i].allocated = true;
            return i;
        }
    }

    printf("Can't allocate. Out of memory\n");
    exit(1);
}

// void deallocate_tree(int root) {
//     Node *n = &nodes[root];

//     if ( n->node_type == UNALLOCATED ) return;
//     else if ( n->node_type == ATOM ) {
//         n->node_type = UNALLOCATED;
//         return;
//     }
//     else { // LIST
//         n->node_type = UNALLOCATED;
//         deallocate_tree(n->left_child);
//         deallocate_tree(n->right_child);
//         return;
//     }
// }

// void deallocate_node(int n) {
//     nodes[n].node_type = UNALLOCATED;
// }
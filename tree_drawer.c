#include "lisp.h"
#include <string.h>
#include <stdio.h>

// recursive printing in "tree" style
void draw_node(int nodeIndex, char *prefix, int isLast) {
    if (nodeIndex < 0) return;

    Node *n = &nodes[nodeIndex];

    printf("%s", prefix);
    if (isLast)
        printf("└── ");
    else
        printf("├── ");

    if (n->node_type == ATOM) {
        printf("%s\n", atom_names[n->atom_id]);
    } else {
        printf("\n");

        // prepare prefix for children
        char newPrefix[1024];
        strcpy(newPrefix, prefix);
        if (isLast)
            strcat(newPrefix, "    ");
        else
            strcat(newPrefix, "│   ");

        // left child (if any)
        if (n->left_child >= 0 && n->right_child >= 0) {
            draw_node(n->left_child, newPrefix, 0);
            draw_node(n->right_child, newPrefix, 1);
        } else if (n->left_child >= 0) {
            draw_node(n->left_child, newPrefix, 1);
        } else if (n->right_child >= 0) {
            draw_node(n->right_child, newPrefix, 1);
        }
    }
}

void draw_tree(int top_node_id) {
    // print from root
    draw_node(top_node_id, "", 1);
}

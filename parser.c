#include "lisp.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>




Node nodes[NODE_TOTAL];

AtomName atom_names[ATOM_TOTAL];
int atom_names_first_empty_index = 0;

bool str_equal(char* str1, char* str2) {
    if (str1[0]==0 || str2[0]==0) {
        puts("trying to compare empty symbol name");
        exit(1);
    }

    return strcmp(str1, str2) == 0;
}

int register_atom_id(char* atom_name) {
    for (int i = 0; i<atom_names_first_empty_index; i++) {
        if ( str_equal(atom_name, atom_names[i]) ) {
            return i;
        }
    }
    if (atom_names_first_empty_index == ATOM_TOTAL) {
        puts("too many symbols");
        exit(1);
    }
    // copy the name
    for (int i = 0; i<ATOM_MAX_LEN+1; i++) {
        atom_names[atom_names_first_empty_index][i] = atom_name[i];
    }
    return atom_names_first_empty_index++;
}


// exp -> atom | list
// atom -> [a-z]+
// list -> '(' expcomma* colonexp? ')'
// expcomma -> exp ','
// colonexp -> ':' exp

typedef struct {
    bool accepted;
    int advance_by_n_chars;
    int node_id;
} ParseResult;


bool is_atom_char(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}

// counts itself, and all children
// int count_nodes(int node_id) {
//     Node node = nodes[node_id];
//     if (node.node_type==ATOM) return 1;
//     int left_sum = count_nodes(node.left_child);
//     int right_sum = count_nodes(node.right_child);
//     return left_sum + right_sum + 1;
// }


ParseResult parse_atom(char *src) {
    AtomName atom = {0};
    int i = 0;
    for ( ; i<ATOM_MAX_LEN; i++) {
        if ( is_atom_char( src[i] ) ) {
            atom[i] = src[i];
        }
        else break;
    }

    // not an atom
    if (i==0) return (ParseResult) {.accepted=false};
    
    int new_node_id = allocate_node();
    Node* node = &nodes[ new_node_id ];
    node->node_type = ATOM;
    node->atom_id = register_atom_id(atom);

    // is an atom
    return (ParseResult) {
        .accepted=true,
        .advance_by_n_chars=i,
        .node_id=new_node_id
    };
}


ParseResult parse_exp(char *src);
ParseResult parse_list(char *src);

ParseResult parse_exp(char *src) {
    ParseResult res = parse_atom(src);
    if (res.accepted) return res;
    res = parse_list(src);
    if (res.accepted) return res;
    return res; // return accepted=false thing
}

ParseResult parse_expcomma(char *src) {
    ParseResult res = parse_exp(src);
    if (!res.accepted) return res;
    // exp parsed
    if (src[res.advance_by_n_chars]!=',') {
        // deallocate_tree(res.node_id);
        return (ParseResult) {.accepted=false};
    }
    // comma present
    res.advance_by_n_chars++;
    return res;
}

ParseResult parse_colonexp(char *src) {
    if (*src!=':') return (ParseResult) {.accepted=false};
    ParseResult res = parse_exp(src+1);
    if (!res.accepted) return res;
    // exp parsed
    res.advance_by_n_chars++;
    return res;
}

ParseResult parse_list(char *src) {
    int advance_by = 0;
    if (*src!='(') return (ParseResult) {.accepted=false};
    advance_by = 1;

    int top_node_id = allocate_node();
    int current_node_id = top_node_id;

    nodes[current_node_id].node_type = LIST;

    while(true) {
        ParseResult res = parse_expcomma(src + advance_by);
        if (!res.accepted) break;
        advance_by += res.advance_by_n_chars;
        // place the node
        nodes[current_node_id].left_child = res.node_id;
        current_node_id = nodes[current_node_id].right_child = allocate_node();
        nodes[current_node_id].node_type = LIST;
    }

    ParseResult res = parse_exp(src + advance_by);
    if (!res.accepted) return res;
    advance_by += res.advance_by_n_chars;
    nodes[current_node_id].left_child = res.node_id;

    // now, optional colonexp
    res = parse_colonexp(src + advance_by);
    if (res.accepted) {
        // add node
        advance_by += res.advance_by_n_chars;
        nodes[current_node_id].right_child = res.node_id;
    }
    else {
        // add NIL
        int nil_node_id = allocate_node();
        nodes[nil_node_id].node_type = ATOM;
        nodes[nil_node_id].atom_id = register_atom_id("NIL");
        nodes[current_node_id].right_child = nil_node_id;
    }

    if (src[advance_by]!=')') return (ParseResult) {.accepted=false};
    advance_by += 1;
    return (ParseResult) {
        .accepted=true,
        .advance_by_n_chars=advance_by,
        .node_id=top_node_id
    };
}

void print_node(int node_id, int current_depth);

void print_all(int top_node_id) {
    puts("\nAtoms-----");
    for (int i = 0; i<atom_names_first_empty_index; i++) {
        printf("id=%4u name=%s\n", i, atom_names[i]);
    }

    puts("\nNodes-----");
    for (int i = 0; i<NODE_TOTAL; i++) {
        Node node = nodes[i];
        if (!node.allocated) continue;
        if (node.node_type == ATOM) {
            printf("id=%4u atom_id=%4u \n", i, node.atom_id);
        }
        else if (node.node_type == LIST) {
            printf("id=%4u left=%4u right=%4u \n", i, node.left_child, node.right_child);
        }
    }

    puts("\nTree-----\n");
    // print_node(top_node_id, 0);
    draw_tree_ascii(top_node_id);
}

char* remove_spaces( char *src) {
    int len = strlen(src);
    char *new_src = calloc( len, sizeof(char) );
    int new_src_first_empty = 0;
    for (int i = 0; i<len; i++) {
        if (src[i]==' ' || src[i]=='\t' || src[i]=='\n' || src[i]=='\r') continue;
        new_src[new_src_first_empty++] = src[i];
    }
    return new_src;
}

char* read_file_to_string(const char* filename) {
    FILE* file = fopen(filename, "rb"); // open in binary mode
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size < 0) {
        perror("ftell failed");
        fclose(file);
        return NULL;
    }

    // Allocate memory for the content + null terminator
    char* buffer = malloc(size + 1);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read file into buffer
    size_t read_size = fread(buffer, 1, size, file);
    if (read_size != size) {
        perror("Failed to read file completely");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[size] = '\0'; // null-terminate the string
    fclose(file);
    return buffer;
}

// void print_node(int node_id, int current_depth) {
//     Node node = nodes[node_id];

//     if (node.node_type==ATOM) {
//         for (int i = 0; i<current_depth; i++) {
//             printf(" ");
//         }
//         if (node.atom_id >= atom_names_first_empty_index) {
//             printf("? (%d)\n", node.atom_id);
//         }
//         else {
//             printf("%s\n", atom_names[node.atom_id]);
//         }
//     }
//     else { // LIST
//         print_node(node.left_child, current_depth+4);
//         print_node(node.right_child, current_depth+4);
//     }
// }

int main(int argc, char**argv ) {

    // "(LABEL," \
    // "SUBST," \
    // "(LAMBDA, (X, Y, Z)," \
    // "    (COND," \
    // "        ((ATOM, Z)," \
    // "            (COND," \
    // "                ((EQ, Y, Z), X)," \
    // "                ((QUOTE, T), Z)" \
    // "            )" \
    // "        )," \
    // "        ((QUOTE, T)," \
    // "            (CONS, (SUBST, X, Y, (CAR, Z)), (SUBST, X, Y, (CDR, Z)))" \
    // "        )" \
    // "    )" \
    // ")" \
    // ")";

    if (argc != 2) {
        printf("Usage: %s lisp_source_code.txt\n", argv[0]);
        exit(1);
    }

    char *src = read_file_to_string(argv[1]);
    if (!src) {
        printf("Failed to read %s\n", argv[1]);
        exit(1);
    }

    char *src_nospaces = remove_spaces(src);

    puts("source code (with, without spaces):");
    puts(src);
    puts(src_nospaces);

    ParseResult res = parse_exp(src_nospaces);
    if (!res.accepted || res.advance_by_n_chars!=strlen(src_nospaces)) {
        puts(":(");
        exit(1);
    }
    printf("top node id = %4u \n\n", res.node_id);

    garbage_collect(0);
    print_all(res.node_id);

    save_png_of_tree(0);

    puts("Exp serialized before gc:");
    print_ast_no_spaces(0);



    

}
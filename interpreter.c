
#include "lisp.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


int new_atom_node(char* atom_name) {
    int atom_id = register_atom_id(atom_name);
    int new_node_id = allocate_node();
    nodes[new_node_id] = (Node) {.node_type=ATOM, .atom_id=atom_id};
    return new_node_id;
}

void assert_is_allocated(int x) {
    if ( !nodes[x].allocated ) {
        puts("Passed an unallocated node");
        exit(1);
    }
}

void assert_is_atom(int x) {
    if ( nodes[x].node_type != ATOM ) {
        puts("Must be an atom");
        exit(1);
    }
}

void assert_is_list(int x) {
    if ( nodes[x].node_type != LIST ) {
        puts("Must be an list");
        exit(1);
    }
}

void assert_is_list_or_nil(int x) {
    if ( (nodes[x].node_type==LIST) ||
         (nodes[x].node_type==ATOM && nodes[x].atom_id==register_atom_id("NIL")) ) {
            return; // ok
    }
    puts("Must be a list or NIL");
    exit(1);
}

int atom(int x) {
    assert_is_allocated(x);

    if ( nodes[x].node_type==ATOM ) {
        return new_atom_node("T");
    }
    else { // LIST
        return new_atom_node("F");
    }
}

int eq(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);
    assert_is_atom(x);
    assert_is_atom(y);

    if (nodes[x].atom_id == nodes[y].atom_id) {
        return new_atom_node("T");
    }
    else {
        return new_atom_node("F");
    }
}

int car(int x) {
    assert_is_allocated(x);
    assert_is_list(x);
    
    return nodes[x].left_child;
}

int cdr(int x) {
    assert_is_allocated(x);
    assert_is_list(x);
    
    return nodes[x].right_child;
}

int cons(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);

    int new_node_id = allocate_node();
    nodes[new_node_id] = (Node) {
        .node_type=LIST, .left_child=x, .right_child=y};
    return new_node_id;
}

bool is_T(int x) {
    assert_is_allocated(x);

    if (nodes[x].node_type==ATOM) {
        return nodes[x].atom_id == register_atom_id("T");
    }
    else return false;
}

bool is_F(int x) {
    assert_is_allocated(x);

    if (nodes[x].node_type==ATOM) {
        return nodes[x].atom_id == register_atom_id("F");
    }
    else return false;
}

void assert_is_TF(int x) {
    assert_is_atom(x);

    if ( !is_T(x) && !is_F(x) ) {
        puts("Must be a T or F node");
        exit(1);
    }
}

int and(int x, int y) {
    assert_is_TF(x);
    assert_is_TF(y);

    if (is_T(x) && is_T(y) ) return new_atom_node("T");
    else return new_atom_node("F");
}

int or(int x, int y) {
    assert_is_TF(x);
    assert_is_TF(y);

    if (is_T(x) || is_T(y) ) return new_atom_node("T");
    else return new_atom_node("F");
}

int ff(int x) {
    assert_is_allocated(x);

    if( is_T(atom(x)) ) return x;
    else return ff(car(x));
}

int subst(int x, int y, int z) {
    assert_is_allocated(x);
    assert_is_allocated(y);
    assert_is_allocated(z);
    assert_is_atom(y);

    if ( is_T(atom(z)) ) {
        if ( is_T(eq(z, y))) return x;
        else return z;
    }
    else return cons(subst(x,y,car(z)), subst(x,y,cdr(z)));
}

int equal(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);

    if ( is_T(atom(x)) && is_T(atom(y)) ) return eq(x, y);
    else if ( is_F(atom(x)) && is_F(atom(y)) ) {
        return and( equal(car(x), car(y)), 
                    equal(cdr(x), cdr(y)));
    }
    else return new_atom_node("F");
}

int nil(int x) {
    assert_is_allocated(x);

    if ( is_T(atom(x)) && is_T(eq(x, new_atom_node("NIL")))) {
        return new_atom_node("T");
    }
    else return new_atom_node("F");
}

int list(int arg_count, ...) {
    va_list args;
    va_start(args, arg_count);

    int top_node_id = allocate_node();
    int current_node_id = top_node_id;
    nodes[current_node_id] = (Node) {
        .allocated=true,
        .node_type=ATOM,
        .atom_id=register_atom_id("NIL")
    };

    for (int i = 0; i<arg_count; i++) {
        nodes[current_node_id] = (Node) {
            .allocated=true,
            .node_type=LIST,
            .left_child=va_arg(args, int),
            .right_child=allocate_node()
        };
        current_node_id = nodes[current_node_id].right_child;
        
        nodes[current_node_id] = (Node) {
            .allocated=true,
            .node_type=ATOM,
            .atom_id=register_atom_id("NIL")
        };
    }

    va_end(args);
    return top_node_id;
}

int append(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);
    assert_is_list_or_nil(x);

    if ( is_T(nil(x)) ) return y;
    else return cons(car(x), append(cdr(x),y));
}

int among(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);
    assert_is_list_or_nil(y);

    if ( is_T(nil(y)) ) return new_atom_node("F");
    return or( equal(x, car(y)),
               among(x, cdr(y))
           );
}

int pair(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);
    assert_is_list_or_nil(x);
    assert_is_list_or_nil(y);

    if ( is_T(nil(x)) && is_T(nil(y)) ) return new_atom_node("NIL");
    return cons(
        list(2, car(x), car(y)),
        pair( cdr(x), cdr(y) )
    );
}

int caar(int x) {
    return car(car(x));
}

int cadar(int x) {
    return car(cdr(car(x)));
}

int cadr(int x) {
    return car(cdr(x));
}

int caddr(int x) {
    return car(cdr(cdr(x)));
}

int caddar(int x) {
    return car(cdr(cdr(car(x))));
}



// NOTE: this assumes assoc always finds a value corresponding to x
int assoc(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);
    assert_is_atom(x);
    
    if ( is_T(eq(caar(y),x)) ) return cadar(y);
    return assoc(x, cdr(y));
}

int sub2(int x, int z) {
    assert_is_allocated(x);
    assert_is_allocated(z);
    assert_is_list_or_nil(x);

    if (is_T(nil(x)) ) return z;
    else if ( is_T(eq( caar(x),z)) ) return cadar(x);
    else return sub2(cdr(x),z);
}

int sublis(int x, int y) {
    assert_is_allocated(x);
    assert_is_allocated(y);

    if ( is_T(atom(y)) ) return sub2(x,y);
    return cons(
        sublis(x, car(y)),
        sublis(x, cdr(y))
    );
}

int appq(int m) {
    assert_is_allocated(m);
    assert_is_list_or_nil(m);

    if ( is_T(nil(m)) ) return new_atom_node("NIL");
    return cons(
        list(2, new_atom_node("QUOTE"), car(m)),
        appq(cdr(m))
    );
}

int eval(int e, int a);

int evcon(int c, int a) {
    assert_is_allocated(c);
    assert_is_allocated(a);

    if ( is_T(eval(caar(c),a)) ) return eval(cadar(c),a);
    return evcon(cdr(c),a);
}

int evlis(int m, int a) {
    assert_is_allocated(m);
    assert_is_allocated(a);

    if (is_T(nil(m)) ) return new_atom_node("NIL");
    return cons(
        eval(car(m),a),
        evlis(cdr(m),a)
    );
}

int eval(int e, int a) {
    assert_is_allocated(e);
    assert_is_allocated(a);

    if ( is_T(atom(e)) ) return assoc(e,a);
    if ( is_T(atom(car(e))) ) {

    }
    if ( is_T( eq(caar(e), new_atom_node("LABEL")) ) ) return eval(
        cons(caddar(e), cdr(e)),
        cons(list(2, cadar(e), car(e)), a)
    );
    if ( is_T( eq(caar(e), new_atom_node("LAMBDA")) ) ) return eval(
        caddar(e),
        append(
            pair(cadar(e), evlis(cdr(e),a)),
            a
        )
    );

    puts("Unknown eval input format");
    exit(1);
}

int apply(int f, int args) {
    return eval(
        cons(f, appq(args)),
        new_atom_node("NIL")
    );
}
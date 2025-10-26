
#include "lisp.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 5400
#define HEIGHT 3200
#define N_NODES_IN_ROW 10
#define N_NODES_IN_COL 10


// =======================
// AST to S-expression
// =======================

void print_ast_no_spaces(int top_node_id ) {
    Node n = nodes[top_node_id];

    if ( !n.allocated ) {
        puts("Error: Trying to print an unallocated node");
        exit(1);
    }

    if ( n.node_type==ATOM ) {
        printf("%s", atom_names[n.atom_id]);
        return;
    }
    
    // n is LIST

    putchar('(');
    print_ast_no_spaces(n.left_child);

    n = nodes[n.right_child];
    if ( !n.allocated ) {
        puts("Error: Trying to print an unallocated node");
        exit(1);
    }

    while( n.node_type==LIST ) {
        putchar(',');
        print_ast_no_spaces(n.left_child);
        n = nodes[n.right_child];
        if ( !n.allocated ) {
            puts("Error: Trying to print an unallocated node");
            exit(1);
        }
    }

    // n is ATOM

    if ( n.atom_id != register_atom_id("NIL")) {
        putchar(':');
        printf("%s", atom_names[n.atom_id] );
    }

    putchar(')');
}


// =============
// Ascii tree
// =============
char* str(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    static char buf[1000] = {0};

    va_start(args, fmt);
    vsnprintf(buf, size + 1, fmt, args);
    va_end(args);
    return buf;
}

// recursive printing in "tree" style
void draw_node_ascii(int nodeIndex, char *prefix, int isLast) {
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
            draw_node_ascii(n->left_child, newPrefix, 0);
            draw_node_ascii(n->right_child, newPrefix, 1);
        } else if (n->left_child >= 0) {
            draw_node_ascii(n->left_child, newPrefix, 1);
        } else if (n->right_child >= 0) {
            draw_node_ascii(n->right_child, newPrefix, 1);
        }
    }
}

void draw_tree_ascii(int top_node_id) {
    // print from root
    draw_node_ascii(top_node_id, "", 1);
}


// ==========
// Raylib All Nodes
// ==========


bool first_time_saving = true;
int saved_png_count = 0;

Font font;
void init_raylib(void) {
    SetTraceLogLevel(LOG_FATAL);
    InitWindow(1, 1, "Title");
    SetWindowState(FLAG_WINDOW_HIDDEN);
    font = LoadFont("font/Aldrich-Regular.ttf");
    first_time_saving = false;
}


Vector2 get_cell( int node_id ) {
    return (Vector2) {
        .x = node_id / N_NODES_IN_COL,
        .y = node_id % N_NODES_IN_COL
    };
}

// Draws an arrow from (fromX, fromY) to (toX, toY)
// (Chatgpt generated)
void DrawArrow(float fromX, float fromY, float toX, float toY, Color color)
{
    // Draw main line
    DrawLineEx((Vector2){fromX, fromY}, (Vector2){toX, toY}, 5, color);

    // Compute angle of the line
    float angle = atan2f(toY - fromY, toX - fromX);

    // Arrowhead size
    float arrowSize = 50.0f; // pixels
    float arrowAngle = 30.0f * (PI / 180.0f); // 30 degrees in radians

    // Compute endpoints of the arrowhead lines
    float x1 = toX - arrowSize * cosf(angle - arrowAngle);
    float y1 = toY - arrowSize * sinf(angle - arrowAngle);

    float x2 = toX - arrowSize * cosf(angle + arrowAngle);
    float y2 = toY - arrowSize * sinf(angle + arrowAngle);

    // Draw arrowhead
    DrawLineEx((Vector2){toX, toY}, (Vector2){x1, y1}, 5, color);
    DrawLineEx((Vector2){toX, toY}, (Vector2){x2, y2}, 5, color);
}

void save_png_of_nodes (void) {
    if (first_time_saving) init_raylib();

    RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);
    BeginTextureMode(target);
        ClearBackground(WHITE);

        const double margin = 0.1; // 10% of node dimensions

        const double node_width = WIDTH * 1. / N_NODES_IN_ROW;
        const double node_height = HEIGHT * 1. / N_NODES_IN_COL;

        const double node_width_minus_margin = node_width * (1 - margin);
        const double node_height_minus_margin = node_height * (1 - margin);

        for (int node_id = 0; node_id<N_NODES_IN_COL*N_NODES_IN_ROW; node_id++) {

            Vector2 this_cell = get_cell(node_id);

            double top_left_x = node_width * this_cell.x;
            double top_left_y = node_height * this_cell.y;

            top_left_x += (node_width * margin / 2);
            top_left_y += (node_height * margin / 2);
            
            if (nodes[node_id].allocated) {
                DrawRectangle(top_left_x, top_left_y, node_width_minus_margin, node_height_minus_margin, GREEN);
            }
            else {
                DrawRectangle(top_left_x, top_left_y, node_width_minus_margin, node_height_minus_margin, GRAY);
            }

            // node_id
            DrawTextEx(font,
                str("%d", node_id),
                (Vector2){top_left_x, top_left_y},
                20, 0, BLACK);

            double middle_left_x = top_left_x;
            double middle_left_y = top_left_y + (node_height / 2);

            // atom_text
            if ( nodes[node_id].allocated && nodes[node_id].node_type==ATOM) {
                DrawTextEx( font,
                    atom_names[nodes[node_id].atom_id],
                    (Vector2){middle_left_x, middle_left_y},
                    1200. / N_NODES_IN_ROW,
                    0, BLACK);
            }
        }

        for (int node_id = 0; node_id<N_NODES_IN_COL*N_NODES_IN_ROW; node_id++) {
            
            Vector2 this_cell = get_cell(node_id);

            double top_left_x = node_width * this_cell.x;
            double top_left_y = node_height * this_cell.y;

            top_left_x += (node_width * margin / 2);
            top_left_y += (node_height * margin / 2);
            
            // arrows
            if ( nodes[node_id].allocated && nodes[node_id].node_type==LIST ) {
                double arrow_from_x = top_left_x + (random() % (int)node_width_minus_margin);
                double arrow_from_y = top_left_y + (random() % (int)node_height_minus_margin);

                Vector2 left_child_cell = get_cell(nodes[node_id].left_child);
                double arrow_to_x = arrow_from_x + (left_child_cell.x-this_cell.x) * node_width;
                double arrow_to_y = arrow_from_y + (left_child_cell.y-this_cell.y) * node_height;
                DrawArrow(arrow_from_x, arrow_from_y, arrow_to_x, arrow_to_y, BLUE);
                
                arrow_from_x += 8;
                arrow_from_y += 8;

                Vector2 right_child_cell = get_cell(nodes[node_id].right_child);
                arrow_to_x = arrow_from_x + (right_child_cell.x-this_cell.x) * node_width;
                arrow_to_y = arrow_from_y + (right_child_cell.y-this_cell.y) * node_height;
                DrawArrow(arrow_from_x, arrow_from_y, arrow_to_x, arrow_to_y, RED);
            }

        }


    EndTextureMode();

    Image image = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&image);

    ExportImage(image, str("pngs/%d.png", saved_png_count++) );
    UnloadImage(image);
}



// ==============
// Raylib Tree
// ==============

int minimum_x, maximum_x, maximum_y;

int calculate_dimensions_recursive(int node_id, int current_x, int current_y) {
    Node n = nodes[node_id];
    
    if (current_x < minimum_x) minimum_x = current_x;
    if (current_x > maximum_x) maximum_x = current_x;
    if (current_y > maximum_y) maximum_y = current_y;

    if (!n.allocated) {
        puts("Node not allocated.");
        exit(1);
    }

    if ( n.node_type == ATOM ) {
        return 1;
    }

    // node type is LIST
    int displacement_to_right = calculate_dimensions_recursive(
        n.left_child,
        current_x-1,
        current_y+1
    );

    if ( nodes[n.right_child].node_type == ATOM) {
        int displacement_to_right_2 = calculate_dimensions_recursive(
            n.right_child,
            current_x+1,
            current_y+1
        );
        return displacement_to_right + displacement_to_right_2;
    }
    else { // right child is list
        int displacement_to_right_2 = calculate_dimensions_recursive(
            n.right_child,
            current_x+displacement_to_right,
            current_y+displacement_to_right
        );
        return displacement_to_right + displacement_to_right_2;

    }

}

typedef struct {
    int root_x;
    int root_y;
    int image_width;
    int image_height;
} TreeDimensions;

const int TREE_SCALE = 100; // pixels

TreeDimensions calculate_dimensions(int top_node_id) {
    minimum_x = maximum_x = maximum_y = 0;
    calculate_dimensions_recursive(top_node_id, 0, 0);

    const int padding_w = 150;
    const int padding_h = 50;
    const int padding_x = 70;
    const int padding_y = 20;

    return (TreeDimensions) {
        .root_x = 1. * (-minimum_x) * TREE_SCALE / 1.41421 + padding_x,
        .root_y = padding_y,
        .image_width = 1. * (maximum_x-minimum_x) * TREE_SCALE / 1.41421 + padding_w,
        .image_height = 1. * (maximum_y) * TREE_SCALE / 1.41421 + padding_h
    };
}

int draw_node_recursive(int node_id, int x_coord, int y_coord) {
    Node n = nodes[node_id];

    if (n.node_type == ATOM) {
        if (n.atom_id == register_atom_id("NIL")) {
            DrawTextEx(
                font,
                ".",
                (Vector2){x_coord,y_coord},
                15, 0, BLACK);   
        }
        else {
            int len = strlen(atom_names[n.atom_id]);

            DrawTextEx(
                font,
                atom_names[n.atom_id],
                (Vector2){
                    x_coord - (1. * len/2*15),
                    y_coord+5
                },
                20, 0, BLACK);
        }
        return (1. * TREE_SCALE / 1.41421);
    }

    // node type is LIST

    DrawCircle(x_coord, y_coord, 10, RED);
    DrawLineEx(
        (Vector2){x_coord,y_coord},
        (Vector2){
            x_coord - (1. * TREE_SCALE / 1.41421),
            y_coord + (1. * TREE_SCALE / 1.41421)
        },
        5, RED);
    
    int displacement_to_right = draw_node_recursive(
        n.left_child,
        x_coord - (1. * TREE_SCALE / 1.41421),
        y_coord + (1. * TREE_SCALE / 1.41421)
    );

    

    if ( nodes[n.right_child].node_type == ATOM) {
        DrawLineEx(
        (Vector2){x_coord,y_coord},
        (Vector2){
            x_coord + (1. * TREE_SCALE / 1.41421),
            y_coord + (1. * TREE_SCALE / 1.41421)
        },
        5, RED);

        int displacement_to_right_2 = draw_node_recursive(
            n.right_child,
            x_coord + (1. * TREE_SCALE / 1.41421),
            y_coord + (1. * TREE_SCALE / 1.41421)
        );

        return displacement_to_right + displacement_to_right_2;
    }
    else { // right child is list 
        DrawLineEx(
            (Vector2){x_coord,y_coord},
            (Vector2){
                x_coord + displacement_to_right,
                y_coord + displacement_to_right
            },
            5, RED);

        int displacement_to_right_2 = draw_node_recursive(
            n.right_child,
            x_coord + displacement_to_right,
            y_coord + displacement_to_right
        );

        return displacement_to_right + displacement_to_right_2;
    }
}

void save_png_of_tree (int top_node_id) {
    if (first_time_saving) init_raylib();

    TreeDimensions td = calculate_dimensions(top_node_id);

    RenderTexture2D target = LoadRenderTexture(td.image_width, td.image_height);
    // RenderTexture2D target = LoadRenderTexture(2000, 2000);
    BeginTextureMode(target);
        ClearBackground(WHITE);

        draw_node_recursive(top_node_id, td.root_x, td.root_y);
        // draw_node_recursive(top_node_id, 500, 500);

    EndTextureMode();

    Image image = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&image);

    ExportImage(image, str("pngs/%d.png", saved_png_count++) );
    UnloadImage(image);
}


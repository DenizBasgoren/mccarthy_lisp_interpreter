
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

bool first_time_saving = true;
int saved_png_count = 0;


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


Font font;
void init_raylib(void) {
    InitWindow(WIDTH, HEIGHT, "Title");
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
    printf("arrow x=%f y=%f ---> x=%f y=%f\n", fromX, fromY, toX, toY);
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

        printf("nw=%d nh=%d\n", (int)node_width, (int)node_height);
        // Color color = GREEN;

        for (int node_id = 0; node_id<N_NODES_IN_COL*N_NODES_IN_ROW; node_id++) {

            Vector2 this_cell = get_cell(node_id);

            double top_left_x = node_width * this_cell.x;
            double top_left_y = node_height * this_cell.y;

            top_left_x += (node_width * margin / 2);
            top_left_y += (node_height * margin / 2);
            
            printf("i=%d x=%d y=%d \n", node_id, (int)top_left_x, (int)top_left_y);

            if (nodes[node_id].allocated) {
                DrawRectangle(top_left_x, top_left_y, node_width_minus_margin, node_height_minus_margin, GREEN);
            }
            else {
                DrawRectangle(top_left_x, top_left_y, node_width_minus_margin, node_height_minus_margin, GRAY);
            }
            // color.g++;

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




/*
 * atop - opening database for atomic chess
 * Copyright (C) 2018  Keyboard Fire <andy@keyboardfire.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "atop.h"

#include <gtk/gtk.h>
#include <stdlib.h>

struct piece {
    GtkWidget *eb;
    int x;
    int y;
};

GtkFixed *board;
float click_x, click_y;
struct piece *clicked = NULL;

static void apply_css(GtkWidget *widget, GtkStyleProvider *provider) {
    gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider, G_MAXUINT);
    if (GTK_IS_CONTAINER(widget)) {
        gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)apply_css, provider);
    }
}

static void generate_board(GtkGrid *grid) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            GtkWidget *img = gtk_image_new_from_file(
                    (i + j) % 2 ? "img/black.png" : "img/white.png");
            gtk_grid_attach(grid, img, i, j, 1, 1);
        }
    }
}

static gboolean piece_clicked(GtkWidget *eb, GdkEventButton *event, gpointer data) {
    click_x = event->x;
    click_y = event->y;
    clicked = data;
    return TRUE;
}

static void add_piece(char *path, int x, int y) {
    GtkWidget *img = gtk_image_new_from_file(path);
    GtkWidget *eb = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(eb), img);

    struct piece *piece = malloc(sizeof *piece);
    piece->eb = eb;
    piece->x = x;
    piece->y = y;
    g_signal_connect(G_OBJECT(eb), "button_press_event",
            G_CALLBACK(piece_clicked), piece);

    gtk_fixed_put(board, eb, x*64, y*64);
}

static gboolean mouse_moved(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    g_print("%f %f\n", event->x, event->y);
    if (clicked) {
        gtk_fixed_move(board, clicked->eb,
                clicked->x*64 + event->x - click_x,
                clicked->y*64 + event->y - click_y);
    }
    return TRUE;
}

static gboolean mouse_released(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    g_print("released\n");
    clicked = NULL;
    return TRUE;
}

static void generate_pieces(GtkOverlay *overlay) {
    board = GTK_FIXED(gtk_fixed_new());

    for (int i = 0; i < 8; ++i) {
        add_piece("img/bp.png", i, 1);
        add_piece("img/wp.png", i, 6);
    }

    add_piece("img/br.png", 0, 0);
    add_piece("img/br.png", 7, 0);
    add_piece("img/wr.png", 0, 7);
    add_piece("img/wr.png", 7, 7);

    add_piece("img/bn.png", 1, 0);
    add_piece("img/bn.png", 6, 0);
    add_piece("img/wn.png", 1, 7);
    add_piece("img/wn.png", 6, 7);

    add_piece("img/bb.png", 2, 0);
    add_piece("img/bb.png", 5, 0);
    add_piece("img/wb.png", 2, 7);
    add_piece("img/wb.png", 5, 7);

    add_piece("img/bq.png", 3, 0);
    add_piece("img/wq.png", 3, 7);

    add_piece("img/bk.png", 4, 0);
    add_piece("img/wk.png", 4, 7);

    gtk_overlay_add_overlay(overlay, GTK_WIDGET(board));
}

void atop_init(int *argc, char ***argv) {
    gtk_init(argc, argv);

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "src/builder.ui", NULL);
    GObject *win = gtk_builder_get_object(builder, "window");

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/builder.css", NULL);
    apply_css(GTK_WIDGET(win), GTK_STYLE_PROVIDER(provider));

    gtk_widget_add_events(win, GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK);

    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(win, "motion_notify_event", G_CALLBACK(mouse_moved), NULL);
    g_signal_connect(win, "button_release_event", G_CALLBACK(mouse_released), NULL);

    generate_board(GTK_GRID(gtk_builder_get_object(builder, "board")));
    generate_pieces(GTK_OVERLAY(gtk_builder_get_object(builder, "overlay")));

    gtk_widget_show_all(GTK_WIDGET(win));

    gtk_main();
}

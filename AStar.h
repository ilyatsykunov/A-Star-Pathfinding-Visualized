#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>

GtkWidget* window,* box,* top_box,* grid,* entry_x,* entry_y,* btn_enter;

int len_x, len_y, start_x, start_y, end_x, end_y;

bool setting_start;
bool setting_end;
bool setting_obstacles;

struct node
{
	int             x;
	int             y;
	int             s_cost;
	int             e_cost;
	int             t_cost;
	bool            discovered;
	bool            visited;
	bool            obstacle;
	int             prev_x;
	int             prev_y;
	GtkWidget*      button;
};

struct node_in_graph
{
	int             index;
	struct node*    p_graph;
};

void set_css ();
static void activate (GtkApplication*, gpointer);
static void set_grid (GtkWidget*, gpointer);
static void set_grid_button (GtkWidget*, gpointer);
void make_path (gpointer);
struct node* get_cheapest_node (gpointer);
void display_path (gpointer);
void reset_graph (gpointer);


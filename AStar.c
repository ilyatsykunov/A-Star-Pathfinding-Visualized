#include "AStar.h"

int 
main (int argc, char **argv)
{
	GtkApplication* app;
	int status;
	app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}

static void 
activate (GtkApplication* app, gpointer user_data)
{

	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Window");
	gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
	
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_container_add (GTK_CONTAINER (window), box);

	set_css ();
	
	top_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_container_add (GTK_CONTAINER (box), top_box);
	
	entry_x = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (top_box), entry_x);
	entry_y = gtk_entry_new();
	gtk_container_add (GTK_CONTAINER (top_box), entry_y);
	
	
	btn_enter = gtk_button_new_with_label ("Enter");
	gtk_container_add (GTK_CONTAINER (top_box), btn_enter);

	g_signal_connect (btn_enter, "clicked", G_CALLBACK (set_grid), NULL);
	


	gtk_widget_show_all (window);
}

void 
set_css ()
{
	GtkCssProvider* provider = gtk_css_provider_new ();
	GdkDisplay* display = gdk_display_get_default ();
	GdkScreen* screen = gdk_display_get_default_screen (display);
	gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	
	const gchar* css_file = "src/style.css";
	GError* error = 0;

	gtk_css_provider_load_from_file (provider, g_file_new_for_path (css_file), &error);
	g_object_unref (provider);
}

static void 
set_grid (GtkWidget* widget, gpointer data)
{	
	// Regenerate the grid
	if (data != NULL)
		g_free (data);
	if (grid != NULL)
	{
		GList* children,* iter;

		children = gtk_container_get_children(GTK_CONTAINER (grid));
		for (iter = children; iter != NULL; iter = g_list_next (iter))
			gtk_widget_destroy (GTK_WIDGET (iter->data));
		g_list_free (children);
		gtk_widget_destroy (GTK_WIDGET (grid));
	}
	
	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (box), grid);
	
	len_x = atoi (gtk_entry_get_text (GTK_ENTRY (entry_x)));
	len_y = atoi (gtk_entry_get_text (GTK_ENTRY (entry_y)));
	
	struct node* p_graph = g_malloc (sizeof (struct node) * (len_x * len_y));
	
	// Spawn a clickable button widget in each grid space
	for (int x = 0; x < len_x; x++)
	{
		for (int y = 0; y < len_y; y++)
		{
			GtkWidget* btn_new = gtk_button_new ();

			struct node temp_node = {x, y, 0, 0, 0, false, false, false, 0, 0, btn_new}; 

			p_graph[x * len_y + y] = temp_node;
			
			struct node_in_graph* p_node_in_graph = g_malloc (sizeof (struct node));
			p_node_in_graph->index = x * len_y + y;
			p_node_in_graph->p_graph = p_graph;
			
			g_signal_connect (btn_new, "clicked", G_CALLBACK (set_grid_button), p_node_in_graph);
			gtk_grid_attach (GTK_GRID (grid), btn_new, x, y, 1, 1);

		}
	}
	
	setting_start = true;
	setting_end = false;
	setting_obstacles = false;
	
	g_signal_connect (btn_enter, "clicked", G_CALLBACK (set_grid), p_graph);
	
	gtk_widget_show_all (window);
}

// Sets a tile as start, end, obstacle or default when user clicks on it
static void 
set_grid_button (GtkWidget* widget, gpointer data)
{
	struct node_in_graph* p_node_in_graph = data;
	int index = p_node_in_graph->index;
	
	struct node* p_graph = p_node_in_graph->p_graph;
	struct node node = p_graph[index];
	
	gchar* btn_new_name;
	
	if (setting_start)
	{	
		btn_new_name = "start";
		
		start_x = node.x;
		start_y = node.y;
		
		setting_end = true;
		setting_start = false;

	}
	else if (setting_end)
	{
		btn_new_name = "end";
		
		end_x = node.x;
		end_y = node.y;
		
		setting_obstacles = true;
		setting_end = false;
	}
	else if (setting_obstacles)
	{
		if ((node.x == start_x && node.y == start_y) || (node.x == end_x && node.y == end_y))
			return;
		
		if(node.obstacle)
		{
			btn_new_name = "default";
			p_graph[index].obstacle = false;
		}
		else
		{
			btn_new_name = "obstacle";
			p_graph[index].obstacle = true;		
		}
	}

	gtk_widget_set_name(widget, btn_new_name);

	make_path (p_graph); 
}

//Finds the best path on a map of nodes
void 
make_path (gpointer data)
{
	if (!setting_obstacles)
		return;

	struct node* p_graph = data; //array of nodes
	
	reset_graph (p_graph);
	
	//Set end values
	for (int x = 0; x < len_x; x++)
	{
		for (int y = 0; y < len_y; y++)
		{	
			int curr_i = x * len_y + y;
			int newe_cost = 0;
			if(x == end_x && y == end_y)
				continue;
			if (x < 0 || x > len_x || y < 0 || y > len_y || p_graph[curr_i].obstacle)
				continue;
			if (end_x <= x)
			{
				if(end_y <= y)
					newe_cost = (x - end_x) + (y - end_y);
				else
					newe_cost = (x - end_x) + (end_y - y);
			}
			else
			{
				if (end_y <= y)
					newe_cost = (end_x - x) + (y - end_y);
				else
					newe_cost = (end_x - x) + (end_y - y);
			}
			newe_cost *= 10;

			if (x != end_x && y != end_y)
				newe_cost += 4;

			p_graph[x * len_y + y].e_cost = newe_cost;
			p_graph[x * len_y + y].t_cost = newe_cost;
		}
	}

	int start_i = start_x * len_y + start_y;
	p_graph[start_i].s_cost = 0;
	p_graph[start_i].discovered = true;
	p_graph[start_i].visited = true;
	struct node* curr_node = &p_graph[start_i];

	int end_i = end_x * len_y + end_y;
	
	
	// Visits node 
	while (p_graph[end_i].visited == false)
	{	
		//Set s_cost for discovered nodes
		for (int x = curr_node->x - 1; x < curr_node->x + 2; x++)
		{
			for (int y = curr_node->y - 1; y < curr_node->y + 2; y++)
			{	
				//printf("SET START VALUE %d:%d\n", y, x);
				int curr_i = x * len_y + y;
				if (x < 0 || x >= len_x || y < 0 || y >= len_y || p_graph[curr_i].obstacle)
					continue;

				int news_cost = curr_node->s_cost + 10;

				if (x != curr_node->x && y != curr_node->y)
					news_cost += 4;

				if (!p_graph[curr_i].discovered || 
						(p_graph[curr_i].discovered && news_cost < p_graph[curr_i].s_cost))
				{
					p_graph[curr_i].s_cost = news_cost;
					p_graph[curr_i].t_cost = news_cost += p_graph[curr_i].e_cost;
					p_graph[curr_i].prev_x = curr_node->x;
					p_graph[curr_i].prev_y = curr_node->y;
					p_graph[curr_i].discovered = true;
				}
			}
		}
		struct node* cheapest_node = get_cheapest_node (data);
		//Visit nodes
		if(cheapest_node == NULL)
			return;

		curr_node = cheapest_node;
		curr_node->visited = true;
	}
	display_path (p_graph);
}

// Returns a node with the lowest cost
struct node*
get_cheapest_node (gpointer data)
{
	struct node* p_graph = data;
	
	struct node* cheapest_node = NULL;
	for (int i = 0; i < len_x * len_y; i++)
	{
		if (p_graph[i].discovered && !p_graph[i].visited && !p_graph[i].obstacle)
		{
			if(cheapest_node == NULL || p_graph[i].t_cost < cheapest_node->t_cost)
			{
				cheapest_node = &p_graph[i];
			}
		}
	}
	return cheapest_node;
}

// Visualizes the best path on the map by painting nodes on the path blue
void 
display_path (gpointer data)
{
	struct node* p_graph = data; //array of nodes

	struct node print_node = p_graph[end_x * len_y + end_y];
	struct node path[len_x*len_y];
	int path_counter = 0;
	while (true)
	{
		path[path_counter] = print_node;

		gchar * btn_new_name = "path";
		gtk_widget_set_name (path[path_counter].button, btn_new_name);

		int prev_x = print_node.prev_x;
		int prev_y = print_node.prev_y;
		print_node = p_graph[prev_x * len_y + prev_y];
		path_counter++;

		if (print_node.x == start_x && print_node.y == start_y)
			break;
	}
}

//Resets the graph so that new best path can be generated
void 
reset_graph (gpointer data)
{
	struct node* p_graph = data;
	
	for (int x = 0; x < len_x; x++)
	{
		for (int y = 0; y < len_y; y++)
		{
			p_graph[x * len_y + y].s_cost = 0;
			p_graph[x * len_y + y].e_cost = 0;
			p_graph[x * len_y + y].t_cost = 0;
			p_graph[x * len_y + y].discovered = false;
			p_graph[x * len_y + y].visited = false;
			gchar* btn_new_name;

			if (x == start_x && y == start_y)
				btn_new_name = "start";
			else if (x == end_x && y == end_y)
				btn_new_name = "end";
			else if (p_graph[x * len_y + y].obstacle)
				btn_new_name = "obstacle";
			else
				btn_new_name = "default";

			gtk_widget_set_name (p_graph[x * len_y + y].button, btn_new_name);
		}
	}
}

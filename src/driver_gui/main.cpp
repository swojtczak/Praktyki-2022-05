#include <gtk/gtk.h>

static void wipe_clicked(GtkWidget* widget, gpointer data)
{
    g_print("wipe");
}

static void wipeFluid_clicked(GtkWidget* widget, gpointer data)
{
    g_print("wipe Fluid");
}

static void left_clicked(GtkWidget* widget, gpointer data)
{
    g_print("left");
}

static void right_clicked(GtkWidget* widget, gpointer data)
{
    g_print("right");
}

static void hazard_clicked(GtkWidget* widget, gpointer data)
{
    g_print("hazard");
}

static void frontLeftUp_clicked(GtkWidget* widget, gpointer data)
{
    g_print("frontLeftUp");
}

static void frontLeftDown_clicked(GtkWidget* widget, gpointer data)
{
    g_print("frontLeftDown");
}

static void frontRightUp_clicked(GtkWidget* widget, gpointer data)
{
    g_print("frontRightUp");
}

static void frontRightDown_clicked(GtkWidget* widget, gpointer data)
{
    g_print("frontRgihtDown");
}

static void backLeftUp_clicked(GtkWidget* widget, gpointer data)
{
    g_print("backLeftUp");
}

static void backLeftDown_clicked(GtkWidget* widget, gpointer data)
{
    g_print("backLeftDown");
}

static void backRightUp_clicked(GtkWidget* widget, gpointer data)
{
    g_print("backRightUp");
}

static void backRightDown_clicked(GtkWidget* widget, gpointer data)
{
    g_print("backRightDown");
}


GtkBuilder	*builder;
GtkWidget	*window;
GtkWidget	*wipe, *wipeFluid, *left, *right, *hazard;
GtkWidget	*frontLeftUp, *frontLeftDown, *frontRightUp, *frontRightDown, *backLeftUp, *backLeftDown, *backRightUp, *backRightDown;

int main(int argc, char *argv[]) {
	gtk_init(&argc, &argv);

	builder = gtk_builder_new_from_file ("../driver.glade");
 
	window = GTK_WIDGET(gtk_builder_get_object(builder, "Window"));

	wipe = GTK_WIDGET(gtk_builder_get_object(builder, "wipe"));
	wipeFluid = GTK_WIDGET(gtk_builder_get_object(builder, "wipeFluid"));
	
	left = GTK_WIDGET(gtk_builder_get_object(builder, "left"));
	right = GTK_WIDGET(gtk_builder_get_object(builder, "right"));
	hazard = GTK_WIDGET(gtk_builder_get_object(builder, "hazard"));
	
	frontLeftUp = GTK_WIDGET(gtk_builder_get_object(builder, "frontLeftUp"));
	frontLeftDown = GTK_WIDGET(gtk_builder_get_object(builder, "frontLeftDown"));
	frontRightUp  = GTK_WIDGET(gtk_builder_get_object(builder, "frontRightUp"));
	frontRightDown = GTK_WIDGET(gtk_builder_get_object(builder, "frontRightDown"));
	backLeftUp = GTK_WIDGET(gtk_builder_get_object(builder, "backLeftUp"));
	backLeftDown = GTK_WIDGET(gtk_builder_get_object(builder, "backLeftDown"));
	backRightUp  = GTK_WIDGET(gtk_builder_get_object(builder, "backRightUp"));
	backRightDown = GTK_WIDGET(gtk_builder_get_object(builder, "backRightDown"));

    
	
	gtk_window_set_default_size(GTK_WINDOW(window), 290, 200);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(wipe, "clicked", G_CALLBACK(wipe_clicked), NULL);
    g_signal_connect(wipeFluid, "clicked", G_CALLBACK(wipeFluid_clicked), NULL);
    g_signal_connect(left, "clicked", G_CALLBACK(left_clicked), NULL);
    g_signal_connect(right, "clicked", G_CALLBACK(right_clicked), NULL);
    g_signal_connect(hazard, "clicked", G_CALLBACK(hazard_clicked), NULL);
    g_signal_connect(frontLeftUp, "clicked", G_CALLBACK(frontLeftUp_clicked), NULL); 
    g_signal_connect(frontLeftDown, "clicked", G_CALLBACK(frontLeftDown_clicked), NULL);
    g_signal_connect(frontRightUp, "clicked", G_CALLBACK(frontRightUp_clicked), NULL);
    g_signal_connect(frontRightDown, "clicked", G_CALLBACK(frontRightDown_clicked), NULL);
    g_signal_connect(backLeftUp, "clicked", G_CALLBACK(backLeftUp_clicked), NULL);
    g_signal_connect(backLeftDown, "clicked", G_CALLBACK(backLeftDown_clicked), NULL);
    g_signal_connect(backRightUp, "clicked", G_CALLBACK(backRightUp_clicked), NULL);
    g_signal_connect(backRightDown, "clicked", G_CALLBACK(backRightDown_clicked), NULL);
	
	gtk_widget_show(window);

	gtk_main();

	return EXIT_SUCCESS;
}
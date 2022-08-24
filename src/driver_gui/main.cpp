#include <gtk/gtk.h>
 
static void button_clicked(GtkWidget* widget, gpointer data)
{
    g_print("Wipe is pressed\n");
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
    g_signal_connect(wipe, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(wipeFluid, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(left, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(right, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(hazard, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(frontLeftUp, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(frontLeftDown, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(frontRightUp, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(frontRightDown, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(backLeftUp, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(backLeftDown, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(backRightUp, "clicked", G_CALLBACK(button_clicked), NULL);
    g_signal_connect(backRightDown, "clicked", G_CALLBACK(button_clicked), NULL);
	
	gtk_widget_show(window);

	gtk_main();

	return EXIT_SUCCESS;
}
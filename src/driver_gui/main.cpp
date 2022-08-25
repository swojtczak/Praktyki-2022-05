#include "mqtt/client.h"
#include <string>
#include <iostream>
#include <fstream>
#include <gtk/gtk.h>

GtkBuilder	*builder;
GtkWidget	*window;
GtkWidget	*wipe, *wipeFluid, *wipeOnce, *wipeFluidOnce, *left, *right, *hazard;
GtkWidget	*frontLeftUp, *frontLeftDown, *frontRightUp, *frontRightDown, *backLeftUp, *backLeftDown, *backRightUp, *backRightDown;

GdkDisplay *display;
GdkScreen *screen;

GtkLabel    *debugLog;

const std::string ADDRESS {"tcp://localhost:1883"};
const std::string CLIENT_ID {"driver_gui"};
const int QOS = 1;
mqtt::connect_options connOpts;
mqtt::client cli(ADDRESS, CLIENT_ID);

bool on = false;
bool onFluid = false;
bool onHazard = false;

std::string frontLeft = "";
std::string frontRight = "";
std::string backLeft = "";
std::string backRight = "";

std::string indi = "";

std::string debLog;

int line = 40;

std::string getLastLines(int lineCount )
{
    size_t const granularity = 100 * lineCount;
    std::ifstream source("/home/autox/.local/share/autox.log", std::ios_base::binary );
    source.seekg( 0, std::ios_base::end );
    size_t size = static_cast<size_t>( source.tellg() );
    std::vector<char> buffer;
    int newlineCount = 0;
    while ( source 
            && buffer.size() != size
            && newlineCount < lineCount ) {
        buffer.resize( std::min( buffer.size() + granularity, size ) );
        source.seekg( -static_cast<std::streamoff>( buffer.size() ),
                      std::ios_base::end );
        source.read( buffer.data(), buffer.size() );
        newlineCount = std::count( buffer.begin(), buffer.end(), '\n');
    }
    std::vector<char>::iterator start = buffer.begin();
    while ( newlineCount > lineCount ) {
        start = std::find( start, buffer.end(), '\n' ) + 1;
        -- newlineCount;
    }
    std::vector<char>::iterator end = remove( start, buffer.end(), '\r' );
    return std::string( start, end );
}

void sendMessage(std::string top, std::string data)
{
    debLog = getLastLines(line);
    
    gtk_label_set_text(debugLog, ( (gchar *) &debLog[0] ));

    char* payload =  &data[0];

    try {
        cli.connect(connOpts);
        cli.publish(top, payload, strlen(payload), 0, false);
        cli.disconnect();
    }catch (const mqtt::exception& exc) {}

}


static void wipe_clicked(GtkWidget* widget, gpointer data)
{
    if(on){
        sendMessage("/car/wipers/front", "off");
    }else{
        sendMessage("/car/wipers/front", "on");
    }
    on = !on;
    onFluid = false;
}

static void wipeFluid_clicked(GtkWidget* widget, gpointer data)
{
    if(onFluid){
        sendMessage("/car/wipers/front", "off");
    }else{
        sendMessage("/car/wipers/front", "fluid");
    }
    onFluid = !onFluid;
    on = false;
}

static void wipeOnce_clicked(GtkWidget* widget, gpointer data){
    sendMessage("/car/wipers/front", "once");
}

static void wipeFluidOnce_clicked(GtkWidget* widget, gpointer data){
    sendMessage("/car/wipers/front", "fluid_once");
}

static void left_clicked(GtkWidget* widget, gpointer data)
{
    if(indi == "left"){
        sendMessage("/car/indicator/left", "off");
        indi = "";
    }else{
        sendMessage("/car/indicator/left", "on");
        indi = "left";
    }
}

static void right_clicked(GtkWidget* widget, gpointer data)
{
    if(indi == "right"){
        sendMessage("/car/indicator/right", "off");
        indi = "";
    }else{
        sendMessage("/car/indicator/right", "on");
        indi = "right";
    }
}

static void hazard_clicked(GtkWidget* widget, gpointer data)
{
    if(onHazard){
        sendMessage("/car/indicator/hazard", "off");
    }else{
        sendMessage("/car/indicator/hazard", "on");
    }
    onHazard = !onHazard;
}

static void frontLeftUp_clicked(GtkWidget* widget, gpointer data)
{
    if(frontLeft == "up"){
        sendMessage("/car/window/0", "stop");
        frontLeft = "";
    }else{
        sendMessage("/car/window/0", "up");
        frontLeft = "up";
    }

}

static void frontLeftDown_clicked(GtkWidget* widget, gpointer data)
{
    if(frontLeft == "down"){
        sendMessage("/car/window/0", "stop");
        frontLeft = "";
    }else{
        sendMessage("/car/window/0", "down");
        frontLeft = "down";
    }
}

static void frontRightUp_clicked(GtkWidget* widget, gpointer data)
{
    if(frontRight == "up"){
        sendMessage("/car/window/1", "stop");
        frontRight = "";
    }else{
        sendMessage("/car/window/1", "up");
        frontRight = "up";
    }
}

static void frontRightDown_clicked(GtkWidget* widget, gpointer data)
{
    if(frontRight == "down"){
        sendMessage("/car/window/1", "stop");
        frontRight = "";
    }else{
        sendMessage("/car/window/1", "down");
        frontRight = "down";
    }
}

static void backLeftUp_clicked(GtkWidget* widget, gpointer data)
{
    if(backLeft == "up"){
        sendMessage("/car/window/2", "stop");
        backLeft = "";
    }else{
        sendMessage("/car/window/2", "up");
        backLeft = "up";
    }
}

static void backLeftDown_clicked(GtkWidget* widget, gpointer data)
{
    if(backLeft == "down"){
        sendMessage("/car/window/2", "stop");
        backLeft = "";
    }else{
        sendMessage("/car/window/2", "down");
        backLeft = "down";
    }
}

static void backRightUp_clicked(GtkWidget* widget, gpointer data)
{
    if(backRight == "up"){
        sendMessage("/car/window/3", "stop");
        backRight = "";
    }else{
        sendMessage("/car/window/3", "up");
        backRight = "up";
    }
}

static void backRightDown_clicked(GtkWidget* widget, gpointer data)
{
    if(backRight == "down"){
        sendMessage("/car/window/3", "stop");
        backRight = "";
    }else{
        sendMessage("/car/window/3", "down");
        backRight = "down";
    }
}

int main(int argc, char *argv[]) {
	
    gtk_init(&argc, &argv);
    
    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);


    builder = gtk_builder_new_from_file("../driver.glade");
	wipe = GTK_WIDGET(gtk_builder_get_object(builder, "wipe"));
	wipeFluid = GTK_WIDGET(gtk_builder_get_object(builder, "wipeFluid"));
	wipeOnce = GTK_WIDGET(gtk_builder_get_object(builder, "wipeOnce"));
	wipeFluidOnce = GTK_WIDGET(gtk_builder_get_object(builder, "wipeFluidOnce"));

	window = GTK_WIDGET(gtk_builder_get_object(builder, "Window"));

	gtk_window_set_default_size(GTK_WINDOW(window), 290, 200);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	
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
	
    debugLog = GTK_LABEL(gtk_builder_get_object(builder, "debugLog"));

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(wipe, "clicked", G_CALLBACK(wipe_clicked), NULL);
    g_signal_connect(wipeFluid, "clicked", G_CALLBACK(wipeFluid_clicked), NULL);
    g_signal_connect(wipeOnce, "clicked", G_CALLBACK(wipeOnce_clicked), NULL);
    g_signal_connect(wipeFluidOnce, "clicked", G_CALLBACK(wipeFluidOnce_clicked), NULL);
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


    GtkCssProvider *provider;

    provider = gtk_css_provider_new ();

    gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_css_provider_load_from_data(provider, ".hazard{color: red;}", -1, NULL);
    
    debLog = getLastLines(line);

    gtk_label_set_text(debugLog, ( (gchar *) &debLog[0] ));
	
  	gtk_widget_show(window);
    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show(window);

    gtk_main();

    return EXIT_SUCCESS;
}
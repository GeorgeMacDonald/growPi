#include <stdlib.h>
#include <time.h>

#include <cairo.h>
#include <gtk/gtk.h>

void do_drawing(cairo_t *, GtkWidget *);

typedef void *gameSound;

typedef struct {
	double red;
	double green;
	double blue;
} gameColor;


struct {
  gboolean startup; 
  gdouble alpha;
  gdouble size;
  gameColor c;
  int     count;
  int     pulse;
  int     pulseChanged;
  int     windowWidth;
  int     windowHeight;
  char   *windowName;
  guint threadID;
  gameSound  startSound;
} Glob;

char *numStr[]={"Start!", "One", "Two", "Three", "Four", "Five"};
int counter=(sizeof(numStr)/sizeof(numStr[0]))-1;

void do_countDown(cairo_t *cr, GtkWidget *widget);

extern int  doGameInit(int argc, char *argv[]);
extern void doGameFrame(void *widget, void *ctx, void *user_data, int width, int height);
extern void doGameEvent(void *widget, void *event, void *user_data);

typedef void *gameImage;
typedef void *gameSound;

void      gameSetColor(double r, double g, double b);
void      gameSetFontSize(double s);

void      gamePutText(void *ctx, char *text, double x, double y);

gameImage gameGetImage(char *imageName);
void      gamePutImage(void *ctx, gameImage img, double x, double y);

gameSound gameGetSound(char *soundName);
void      gamePlaySound( void *ctx, gameSound snd );

void      gameSetRGB(  void *ctx, double red, double green, double blue);
void      gameProject( void *ctx );

int       gameGetMouseButton( void *event );
int       gameGetMouseX(      void *event );
int       gameGetMouseY(      void *event );

void      gameSetWindowSize( int width, int height );   /* Use in doGameInit */
void      gameSetWindowName( char *name  );             /* Use in doGameInit */

static gboolean 
on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{      

  GtkWidget *win = gtk_widget_get_toplevel(widget);
  
  gint width, height;
  gtk_window_get_size(GTK_WINDOW(win), &width, &height);  
  
  if ( counter >= 0 )
      do_countDown(cr, widget);
  else
      doGameFrame( (void *)widget, (void *)cr, (void *) user_data, width, height);

  return FALSE;
}

void      
gameSetWindowSize( int width, int height )
{
  Glob.windowWidth = width;
  Glob.windowHeight = height;
}

void      
gameSetWindowName( char *name  )
{
  Glob.windowName = name;
}

void      
gameSetRGB(  void *ctx, double red, double green, double blue)
{
    cairo_set_source_rgb( (cairo_t *) ctx, red, green, blue); 
}

void      
gameSetPulse(  int pulseTime )
{
  Glob.pulse        = pulseTime;
  Glob.pulseChanged = 1;
}

void      
gameSetColor(double r, double g, double b)
{
  Glob.c.red        = r;
  Glob.c.green      = g;
  Glob.c.blue       = b;
}

void      
gameSetFontSize(double s)
{
  Glob.size       = s;
}

void      
gamePutText( void *ctx, char *text, double x, double y)
{
  cairo_text_extents_t extents;
  cairo_t *cr;
  gdouble size;

  cr = (cairo_t *)ctx;
    
  cairo_save(cr);
  cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
 
  size  = Glob.size; // 20.8;

  cairo_set_font_size(cr, size);

  cairo_set_source_rgb(cr, Glob.c.red, Glob.c.green, Glob.c.blue); 

  cairo_text_extents(cr, text, &extents);
  cairo_move_to(cr, x, y);
  cairo_text_path(cr, text);
  cairo_clip(cr);

  cairo_paint(cr);
  
  cairo_restore(cr);
}


gameImage
gameGetImage(char *imageName)
{
  return (void *) cairo_image_surface_create_from_png( imageName );
}

void      
gamePutImage( void *ctx, gameImage img, double x, double y)
{
    cairo_set_source_surface((cairo_t *) ctx, (cairo_surface_t *) img, x, y);
}

gameSound
gameGetSound(char *soundName)
{
   return (gameSound) soundName ;
}

void      
gamePlaySound( void *ctx, gameSound snd )
{
    char cmdStr[256];

    sprintf( cmdStr, "aplay -q %s\n", (char *) snd );
    system( cmdStr );
}

void
gameProject( void *ctx )
{
  cairo_paint( (cairo_t *) ctx );
}

int       
gameGetMouseButton( void *event )
{
	GdkEventButton *btnEvent = (GdkEventButton *)event;

	return ( btnEvent->button );
}

int       
gameGetMouseX( void *event )
{
	GdkEventButton *btnEvent = (GdkEventButton *)event;

	return ( btnEvent->x );
}

int       
gameGetMouseY( void *event )
{
	GdkEventButton *btnEvent = (GdkEventButton *)event;

	return ( btnEvent->y );
}


static gboolean 
clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    doGameEvent((void *)widget, (void *)event, (void *)user_data);

    return TRUE;
}


void 
do_countDown(cairo_t *cr, GtkWidget *widget)
{
  if ( counter < 0 )
 	return;

  cairo_text_extents_t extents;

  GtkWidget *win = gtk_widget_get_toplevel(widget);
  
  gint width, height;
  gtk_window_get_size(GTK_WINDOW(win), &width, &height);  
  
  gint x = width/2;
  gint y = height/2;
  
  cairo_set_source_rgb(cr, 0.5, 0, 0); 
  cairo_paint(cr);   

  cairo_select_font_face(cr, "Courier",
      CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_BOLD);
 
  Glob.size += 0.8;

  if (Glob.size > 20) {
      Glob.alpha -= 0.01;
  }

  cairo_set_font_size(cr, Glob.size);
  cairo_set_source_rgb(cr, 1, 1, 1); 

  cairo_text_extents(cr, numStr[counter], &extents);
  cairo_move_to(cr, x - extents.width/2, y);
  cairo_text_path(cr, numStr[counter]);
  cairo_clip(cr);

  cairo_paint_with_alpha(cr, Glob.alpha);
  
  if (Glob.alpha <= 0) {
      if ( counter <= 0 )
      {
          gamePlaySound( (void *)widget, Glob.startSound );
          Glob.startup = FALSE;
          counter -= 1;
      }
      else
      {  
         counter -= 1;
         Glob.alpha = 1.0;
         Glob.size = 1.0;
      }
  }     
}

static gboolean 
time_handler(GtkWidget *widget)
{ 
  if (!Glob.startup) 
  {
      if ( Glob.pulseChanged  )
      {
           g_source_remove(Glob.threadID);
           Glob.threadID = g_timeout_add( Glob.pulse, (GSourceFunc) time_handler,(gpointer) widget);
           Glob.pulseChanged = 0;
      }
     // return FALSE;
  }

  gtk_widget_queue_draw(widget);

  return TRUE;
}


int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;  
  
  Glob.startup = TRUE;
  Glob.alpha = 1.0;
  Glob.size = 1.0;
  Glob.c.red = 1.0;
  Glob.c.green = 1.0;
  Glob.c.blue = 1.0;
  Glob.windowWidth=640;
  Glob.windowHeight=480;
  Glob.windowName="gameApp";

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER (window), darea);

  gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);

  doGameInit( argc, argv);

  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_draw_event), NULL); 
  g_signal_connect(window, "destroy",
      G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(window, "button-press-event",
      G_CALLBACK(clicked), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), Glob.windowWidth, Glob.windowHeight); 
  gtk_window_set_title(GTK_WINDOW(window), Glob.windowName);

  Glob.startSound =  gameGetSound( "startGame.wav" );
  Glob.pulse=14;
  Glob.threadID = g_timeout_add( Glob.pulse, (GSourceFunc) time_handler, (gpointer) window);

  Glob.pulseChanged = 1;
  Glob.pulse        = 500;   /* default to half second */
  counter = 2;

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}

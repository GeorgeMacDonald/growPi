#ifndef GAME_PKG_HDR
#define GAME_PKG

typedef void *gameImage;
typedef void *gameSound;

extern void      gameSetWindowSize( int width, int height );   /* Use in doGameInit */
extern void      gameSetWindowName( char *name  );             /* Use in doGameInit */

extern void      gameSetColor(double r, double g, double b);

extern void      gameSetFontSize(double s);


extern void      gamePutText(void *ctx, char *text, double x, double y);

extern gameImage gameGetImage(char *imageName);
extern void      gamePutImage(void *ctx, gameImage img, double x, double y);

extern gameSound gameGetSound(char *soundName);
extern void      gamePlaySound( void *ctx, gameSound snd );

extern void      gameSetRGB(  void *ctx, double red, double green, double blue);
extern void	 gameProject( void *ctx );

extern int       gameGetMouseButton( void *event );
extern int	 gameGetMouseX(      void *event );
extern int	 gameGetMouseY(      void *event );

extern void      gameSetPulse( int );   /* Only change inside doGameFrame call */


/* Implement these in your game*/

int  doGameInit(int argc, char *argv[]);
void doGameFrame(void *widget, void *ctx, void *user_data, int width, int height);
void doGameEvent(void *widget, void *event, void *user_data);

#endif

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "game.h"

float  randomFloat(    float min, float max);
float  randomMovement( float min, float max);

double moveTowardsObj(      double staticObjPos, double movingObjPos, double maxMovement );
int    movingObjectAtPlant( int i, double objX, double objY );


#define NOPLANT 0
#define SEEDED  1
#define SPROUT  2
#define GROWING 3
#define PLANTED 4
#define WITHERED 5
#define FRUIT   6
#define DEAD    7

#define MAX_PLANTS  10
#define MAX_SEEDS   20

struct {
  int        count;
  double     coordx[1000];
  double     coordy[1000];
  int        plantState[1000];
  int        plantCycles[1000];  /* How many cycles in this state */
  gameImage *imageSeeded;
  gameImage *imageSprouted;
  gameImage *imageGrowing;
  gameImage *imagePlanted;
  gameImage *imageWithered;
  gameImage *imageFruited;
  gameImage *imageDead;
  gameImage *locust;
  gameImage *locust1;
  gameImage *farmer;
  gameImage *farmer1;
  gameImage *gameOver;
  gameImage *gameWon;
  gameImage *pieSlinger;
  gameImage *delayImage;
  gameSound *soundCanNotDo;
  double     locustx;
  double     locusty;
  int	     locustIndex;
  double     farmerx;
  double     farmery;
  int	     farmerIndex;
  int        numPlanted;
  int        numSeeded;
  uint       threadID;
  int	     doExit;
  int	     doRound;
  int        delay;
  int        round;
  int	     locustEats;
  int	     farmerPicked;
  char       scoreStr[128];
  char       farmerScoreStr[128];
  char       seedCount[128];
  char       plantCount[128];
} glob;

char * appName="growPi";

void doGame( void *ctx, void *widget, int width, int height );

void
doGameFrame(void *widget, void *ctx, void *user_data, int width, int height)
{      
  doGame(ctx, widget, width, height);
}


float 
randomFloat(float min, float max) 
{
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    float random_value = (float)rand() / (float)RAND_MAX;
    return (random_value * (max - min)) + min;
}

float 
randomMovement(float min, float max) 
{
    float delta;
    float dir;

    delta = randomFloat(min, max);
    dir   = randomFloat(min, max);

    if ( dir < ((max - min)/2) + min )
        delta = 0.0 - delta;

    return delta;
}

double
constrainWrapValue( double v, double min, double max )
{
      if ( v < min ) return max;
      if ( v > max ) return min;

      return v;
}

double
moveTowardsObj( double staticObjPos, double movingObjPos, double maxMovement )
{
        double delta;

        delta = staticObjPos - movingObjPos;

	if ( delta > maxMovement ) 
            delta = maxMovement;
	if ( delta < -maxMovement )
            delta = -maxMovement;

	return movingObjPos + delta;
}

int
movingObjectAtPlant( int i, double objX, double objY )
{
	double dx, dy;

	dx = glob.coordx[i] - objX;
        dy = glob.coordy[i] - objY;

        if (( (dx > -1.0 ) && ( dx < 1.0 ) ) && ( (dy > -1.0 ) && ( dy < 1.0 ) ))
	   return 1;
	else
	   return 0;
}

void
newRound()
{
  glob.locustx = randomFloat( 10.0, 700.0 );
  glob.locusty = randomFloat( 40.0, 500.0 );
  glob.locustIndex = 0;

  glob.farmerx = randomFloat( 10.0, 760.0 );
  glob.farmery = randomFloat( 41.0, 550.0 );
  glob.farmerIndex = 0;

  glob.numPlanted = 0;
  glob.numSeeded  = 0;

  glob.count   = 0;
  glob.doRound = 0;
  glob.delay   = -1;

  glob.locustEats    = 0;
  glob.farmerPicked  = 0;
}

void doGame( void *ctx, void *widget, int width, int height )
{
  int     i;
  float   dir;
  int     growingThings=0;
  double  distToObject;
  double  shortestDistToEdible;
  double  shortestDistToPickable;
  double  dx, dy;
  int     closestEdible;
  int     closestPickable;
  static int first=1;

  int x = width/2;
  int y = height/2;
  
  if ( first )
  {
     gameSetPulse( 500  );  // 500 works
     first = 0;
  }
  
  if ( glob.delay > 0 )  // If delaying put up something on screen
  {
      gamePutImage(ctx, glob.delayImage, 0.1, 0.1);
      gameProject(ctx);   
      glob.delay -= 1;
      return;
  }

  if ( glob.doRound > 0  && glob.doExit < 2 )  /* new round */
  {
     if ( glob.round > 0 )  /* It's Ok to start another round */
     {
	    glob.doRound = 0;
            gameSetPulse( 500 - ((10 - glob.round) * 40) );  // 500 works
	    newRound();
	    glob.round -= 1;
            glob.delay = 10;  /* Leave up result screen for a bit */
      	    gamePutImage(ctx, glob.delayImage, 0.1, 0.1);
      	    gameProject(ctx);   
            return;
     }
     else  /* We are done, display results for a while and exit */
     {
      	gamePutImage(ctx, glob.delayImage, 0.1, 0.1);
      	gameProject(ctx);   
        glob.delay = 10;
        glob.doExit += 1;
	return;
     }
  }

  if ( glob.doExit >= 1 )
  {
	exit( 1 );
  }

  gameSetRGB(ctx, 0.0, 0.4, 0.0);    /* Fields of Green.... */
  gameProject(ctx);   

  closestEdible          = -1;
  closestPickable        = -1;
  distToObject           = 0.0;
  shortestDistToEdible   = 999999.0;
  shortestDistToPickable = 999999.0;
  
  for ( i = 0; i < glob.count ; i++ )
  {
        glob.plantCycles[i] += 1;
        /* printf("Plant[%i]at(%g)(%g) state(%d) cycles(%d)\n",i, glob.coordx[i], glob.coordy[i], 
				glob.plantState[i], glob.plantCycles[i] ); */

        growingThings++;  /* We subtract one if no case matches - see default */
	switch ( glob.plantState[i] ) {

	  case SEEDED : 
		gamePutImage(ctx, glob.imageSeeded, glob.coordx[i], glob.coordy[i]);
		if ( glob.plantCycles[i] > 10 )
		{
	            glob.plantState[i]  = SPROUT;
		    glob.plantCycles[i] = 0;
                }
		break;

	  case SPROUT : 
		gamePutImage(ctx, glob.imageSprouted, glob.coordx[i], glob.coordy[i]);
		if ( glob.plantCycles[i] > 10 )
		{
	            glob.plantState[i]  = PLANTED;
		    glob.plantCycles[i] = 0;
		}
		break;

	  case PLANTED : 
		gamePutImage(ctx, glob.imagePlanted, glob.coordx[i], glob.coordy[i]);
		if ( glob.plantCycles[i] > 10 )
		{
	            glob.plantState[i]  = GROWING;
		    glob.plantCycles[i] = 0;
		}
		break;

	  case GROWING : 
		gamePutImage(ctx, glob.imageGrowing, glob.coordx[i], glob.coordy[i]);
		if ( glob.plantCycles[i] > 10 )
		{
	            glob.plantState[i]  = FRUIT;
		    glob.plantCycles[i] = 0;
		}

		/* Locust eats growing or fruited plants */

		dx = glob.coordx[i] - glob.locustx;
                dy = glob.coordy[i] - glob.locusty;
		distToObject = (dx * dx) + (dy * dy);
                if ( distToObject < shortestDistToEdible  )
                {
  		    //printf("Closest edible(%d)\n", i);
  		    closestEdible        = i;
  		    shortestDistToEdible = distToObject;
  		}
                
		break;

	  case FRUIT : 
		gamePutImage(ctx, glob.imageFruited, glob.coordx[i], glob.coordy[i]);
		if ( glob.plantCycles[i] > 10 )
		{
	            glob.plantState[i]  = WITHERED;
		    glob.plantCycles[i] = 0;
		}
                
		dx = glob.coordx[i] - glob.locustx;
                dy = glob.coordy[i] - glob.locusty;
		distToObject = (dx * dx) + (dy * dy);
                if ( distToObject < shortestDistToEdible  )
                {
  		    //printf("Closest edible(%d)\n", i);
  		    closestEdible        = i;
  		    shortestDistToEdible = distToObject;
  		}

	        /* Looking for closest berries for farmer to harvest */
                
		dx = glob.coordx[i] - glob.farmerx;
                dy = glob.coordy[i] - glob.farmery;
		distToObject = (dx * dx) + (dy * dy);
                if ( distToObject < shortestDistToPickable  )
                {
  		    //printf("Closest pickable(%d)\n", i);
  		    closestPickable        = i;
  		    shortestDistToPickable = distToObject;
		}

		break;

	  case WITHERED : 
		gamePutImage(ctx, glob.imageWithered, glob.coordx[i], glob.coordy[i]);
		if ( glob.plantCycles[i] > 10 )
		{
	            glob.plantState[i]  = DEAD;
		    glob.plantCycles[i] = 0;
		}
		break;

	  case DEAD : 
		gamePutImage(ctx, glob.imageDead, glob.coordx[i], glob.coordy[i]);
		if ( glob.plantCycles[i] > 10 )
		{
	            glob.plantState[i]  = NOPLANT;
		    glob.plantCycles[i] = 0;
		}
		break;

          default:
                growingThings--;
  		break;
	}
  	gameProject(ctx);   
  }

  if ( growingThings <= 0 )  /* If we grew 10+ things and nothings growing we give up */
  {
      if ( glob.count > 10 )
      {
          if ( glob.farmerPicked > 20 && ( glob.farmerPicked > glob.locustEats ))
	  {
	    if ( glob.farmerPicked > 200 )
		glob.delayImage = glob.pieSlinger;
            else
		glob.delayImage = glob.gameWon;
	  }
          else
            glob.delayImage = glob.gameOver;

	  glob.doRound=1;
      	  gamePutImage(ctx, glob.delayImage, 0.1, 0.1);
          gameProject(ctx);   
	  return;
      }
  }       


     /* ----------------------- Do Locust ------------------------ */

  // Move locust to food if any, or move randomly

  if ( closestEdible >= 0 )   /* Something is Edible so move to it and eat when there */
  {
  	i = closestEdible;

        glob.locustx = moveTowardsObj( glob.coordx[i], glob.locustx, 20.0 );
        glob.locusty = moveTowardsObj( glob.coordy[i], glob.locusty, 20.0 );

        if ( movingObjectAtPlant( i, glob.locustx, glob.locusty ))
	{
    	  glob.locustEats  += 1;
          //printf("Locust eats berries/plant(%d)\n", glob.locustEats);
	}
  }
  else /* Move around randomly */
  {
      glob.locustx += randomMovement( 8.0, 40.0 );
      glob.locusty += randomMovement( 6.0, 40.0 );
    
      glob.locustx = constrainWrapValue( glob.locustx, 1.1, width-1.0 );
      glob.locusty = constrainWrapValue( glob.locusty, 21.0, height-1.0 );
  } 

  if ( glob.locustIndex )
     gamePutImage(ctx, glob.locust, glob.locustx, glob.locusty);
   else
     gamePutImage(ctx, glob.locust1, glob.locustx, glob.locusty);

  glob.locustIndex +=  1;
  if ( glob.locustIndex > 1 )
  	glob.locustIndex = 0;

  gameProject(ctx);   

     /* ----------------------- Do Farmer ------------------------ */

     if ( closestPickable >= 0 )   /* A Plant has berries so move to it and pick em */
     {
  	i = closestPickable;

        glob.farmerx = moveTowardsObj( glob.coordx[i], glob.farmerx, 40.0 );
        glob.farmery = moveTowardsObj( glob.coordy[i], glob.farmery, 40.0 );

        if ( movingObjectAtPlant( i, glob.farmerx, glob.farmery )) /* Pick some berries */ 
	{
    	  glob.farmerPicked  += 1;
          //printf("Farmers berries(%d)\n", glob.farmerPicked);
	}
     }

     if ( glob.farmerPicked > 0 ) /* Show image with berries in basket */
     {
       // use farmer index to animate farmer
       gamePutImage(ctx, glob.farmer1, glob.farmerx, glob.farmery);
     }
     else
     {
       gamePutImage(ctx, glob.farmer, glob.farmerx, glob.farmery);
     }
  
     gameProject(ctx);   

  /* Update score and counts of seeds/plants left */

  gameSetColor(1.0,1.0,1.0);
  gameSetFontSize(20.8);

  sprintf( glob.scoreStr,       "Locust: %d", glob.locustEats );
  sprintf( glob.farmerScoreStr, "Farmer: %d", glob.farmerPicked );

  sprintf( glob.seedCount,  "Click Mouse Left -> [Seeds(%d)]", MAX_SEEDS - glob.numSeeded );
  sprintf( glob.plantCount, "[Plants(%d)] <- Click Mouse Right", MAX_PLANTS - glob.numPlanted );

  gamePutText(ctx, &(glob.scoreStr[0]), 10.0, 20.1);
  gameProject(ctx);   

  gamePutText(ctx, &(glob.farmerScoreStr[0]), 660.0, 20.1);
  gameProject(ctx);   

  gameSetColor(0.0,1.0,1.0);
  gameSetFontSize(14.0);

  gamePutText(ctx, &(glob.seedCount[0]), 135.0, 20.1);
  gameProject(ctx);   

  gamePutText(ctx, &(glob.plantCount[0]), 388.0, 20.1);
  gameProject(ctx);   

}

void
doGameEvent(void *widget, void *event, void *user_data)
{
    int button;
    double x, y;

    button = gameGetMouseButton( event );
    x = gameGetMouseX( event );
    y = gameGetMouseY( event );

    // printf("doGameEvent( Btn(%d), x(%g), y(%g)\n", button, x, y );

    if ( button == 1) {
	if ( glob.numSeeded >= MAX_SEEDS || y <= 20.0 )
	  gamePlaySound( widget, glob.soundCanNotDo );
	else
	{
          glob.plantState[glob.count] = SEEDED;
          glob.plantCycles[glob.count] = 0;
          glob.coordx[glob.count] = x;
          glob.coordy[glob.count++] = y;
 	  glob.numSeeded++;
          //printf("Seeded[%d](%g)(%g)\n", glob.numSeeded, x, y);
	}
    }

    if ( button == 3) {
	if ( glob.numPlanted >= MAX_PLANTS  || y < 20.0 )
	  gamePlaySound( widget, glob.soundCanNotDo );
	else
	{
          glob.plantState[glob.count] = PLANTED;
          glob.plantCycles[glob.count] = 0;
          glob.coordx[glob.count] = x;
          glob.coordy[glob.count++] = y;
 	  glob.numPlanted++;
          //printf("Planted[%d](%g)(%g)\n", glob.numPlanted, x, y);
	}
    }
}


int doGameInit(int argc, char *argv[])
{
  glob.imageSeeded    = gameGetImage("seeded.png");
  glob.imageSprouted  = gameGetImage("sprouted.png");
  glob.imageGrowing   = gameGetImage("growing.png");
  glob.imagePlanted   = gameGetImage("planted.png");
  glob.imageFruited   = gameGetImage("fruited.png");
  glob.imageWithered  = gameGetImage("withered.png");
  glob.imageDead      = gameGetImage("dead.png");
  
  glob.soundCanNotDo  = gameGetSound("canNotDo.wav");

  glob.locust       = gameGetImage("locust.png");
  glob.locust1      = gameGetImage("locust1.png");

  glob.farmer       = gameGetImage("farmer.png");
  glob.farmer1      = gameGetImage("farmer1.png");

  glob.gameOver     = gameGetImage("gameOver.png");
  glob.gameWon      = gameGetImage("gameWon.png");
  glob.pieSlinger   = gameGetImage("pieSlinger.png");

  glob.delayImage   = glob.gameWon;

  gameSetWindowSize( 800, 600 );
  gameSetWindowName( appName  );

  glob.round        = 5;

  newRound();

  return 0;
}

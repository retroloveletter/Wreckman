#ifndef PACMAN_H
#define PACMAN_H

#include "displayutils.h"
#include "celutils.h"
#include "animutils.h"
#include "io.h"
#include "controlpad.h"
#include "event.h"
#include "timerutils.h"
#include "operamath.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "hardware.h"
#include "audio.h"
#include "filefunctions.h"
#include "textlib.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define NUM_SCREENS 2
#define FRACBITS_16 16
#define FRACBITS_20 20
#define MAP_WIDTH 27
#define MAP_HEIGHT 30
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)
#define MAX_NODES 160
#define TILE_SIZE 6
#define STEP_SIZE 2
#define X_OFFSET 80
#define Y_OFFSET 30
#define MAX_GHOSTS 4
#define ANGLE_0 0
#define ANGLE_90 90
#define ANGLE_180 180
#define ANGLE_270 270
#define SPRITE_SIZE 12
#define HALF_SPRITE_SIZE (SPRITE_SIZE/2)
#define MAX_ANIMS 14
#define DEBUG_VERBOSE 0
#define PRINT_FPS 0
#define GOD_MODE 0
#define GET_GFX_ASSET_PATH(filename)("Assets/GFX/"filename)
#define GET_SFX_ASSET_PATH(filename)("Assets/SFX/"filename)
#define IS_AT_INTERSECTION(x,y) ((x % TILE_SIZE == 0) && (y % TILE_SIZE == 0))
#define HIDE_SPRITE(CCB) (CCB->ccb_Flags |= CCB_SKIP)
#define SHOW_SPRITE(CCB) (CCB->ccb_Flags &= ~CCB_SKIP)


typedef struct 
{
	bool ready;
	int32 delay;
	int32 previousTime;
} SimpleTimer;

typedef struct 
{
	int32 x;
	int32 y;
} Vector;

typedef struct 
{
	CCB *ccb;
	int32 setIndex;
	int32 frame;
	int32 lastAnimTime;
	int32 animDelay;
	int32 worldx;
	int32 worldy;
	Vector currentVec;	// where player is heading
	Vector desiredVec;	// where player wants to go
} Player;

typedef enum 
{
	FOLLOW_PLAYER = 0,	// chase player
	RANDOM,				// move around board randomly
	RANDOM_TOP,			// move around top half of board randomly
	RANDOM_BOTTOM		// move around bottom half of board randomly
} GhostBehavior;

typedef enum 
{
	JAILED = 0, // ghosts sit and wait to be released
	ESCAPE,		// ghosts are ready to break free
	PLAY,		// ghosts are exploring the board looking for player
	RUN, 		// ghosts are at risk of being eaten
	RETURN		// ghosts were eaten and are returning to jail
} GhostState;

typedef struct  
{
	CCB *base;
	CCB *eyes;
	GhostState state;
	Vector currentVec;
	int32 index;
	int32 frame;
	int32 baseSpritesetId;
	int32 lastAnimTime;
	int32 animDelay;
	int32 jailTime;
	int32 worldx;
	int32 worldy;
	// pathfinding data
	bool hasPath;
	int32 nodeIndex;
	Point nodes[MAX_NODES];
	Point target;
} Ghost;

extern ScreenContext *sc;
extern Item sport;
extern Item timerIOReq;
extern Item vbl;
extern uint32 buttons;
extern uint32 prevButtons;
extern Player player;
extern int32 successionCount;
extern ControlPadEventData cped;

// game.c
extern SimpleTimer powerupTimer;
extern Boolean map[MAP_SIZE];
extern void gameHandler(void);
extern bool checkMove(Coord x, Coord y, Vector *vec);
extern void warp(Coord *x, int32 xvector);
extern Boolean isPointOnGameBoard(Coord x, Coord y);

// pathfinder.c
extern void initPathfinder(void);
extern void buildPath(Point *start, Point *target, Point *dest, int32 *index);
extern Boolean isTargetValid(int32 tx, int32 ty);

// ghosts.c
extern Ghost ghosts[MAX_GHOSTS];
extern void initGhosts(void);
extern void cleanupGhosts(void);
extern void updateGhosts(void);
extern void resetGhosts(void);
extern void setGhostState(Ghost *ghost, int32 state);

// audio_util.c
extern void audioInit(void);
extern void audioCleanup(void);
extern void playSound(Item sound, int32 priority);
extern void audioUpdate(void);
extern int32 getSoundFrameCount(Item sound);

// util.c
extern void printLine(char *pstring, ...); // only outputs if debug mode is on, it's safe to call.
extern bool rectCollision(SRect *r1, SRect *r2);
extern void execProgram(char *name);
extern void restartSimpleTimer(SimpleTimer *timer);
extern void initSimpleTimer(SimpleTimer *timer, int32 delay);
extern bool isSimpleTimerReady(SimpleTimer *timer);

// sprites.c
extern int32 loadSpriteset(char *path);
extern void freeSpritesets(void);
extern CCB* cloneSpriteCCB(int32 animIndex);
extern void setSpritePosition(CCB *ccb, Coord x, Coord y);
extern void translateSprite(CCB *ccb, Coord x, Coord y);
extern void rotSprite(CCB *ccb, int32 angle);
extern void incAnimFrame(int32 setIndex, int32 *frame);
extern void setSpriteFrame(CCB *ccb, int32 setIndex, int32 frame);

// main.c
extern void (*handlerPtr)(void);
extern uint32 black;
extern uint32 white;

#endif // PACMAN_H

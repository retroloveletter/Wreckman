#include "pacman.h"

/********************************************************************/
/* --------------------- CONSTANTS AND MACROS --------------------- */
/********************************************************************/

#define JAIL_MIN_X 66
#define JAIL_MAX_X 84
#define JAIL_BREAK_X 76
#define JAIL_BREAK_Y 60

/********************************************************************/
/* ------------------- TYPES AND DATASTRUCTURES ------------------- */
/********************************************************************/

typedef enum
{
	GhostSS1 = 0,
	GhostSS2,
	GhostSS3,
	GhostSS4,
	GhostRunSS,
	GhostEyesSS,
	MaxGhostSS
} GhostSS;


/********************************************************************/
/* -------------------- STATIC AND EXTERN VARS -------------------- */
/********************************************************************/

// extern
Ghost ghosts[MAX_GHOSTS];

// static
static int32 spritesetIds[MaxGhostSS];


/********************************************************************/
/* ------------------- STATIC FUNCTION PROTOTYPES ----------------- */
/********************************************************************/


static void updateGhostFollow(Ghost *ghost);
static void ghostToScreen(Ghost *ghost);
static void setGhostPosition(Ghost *ghost, int32 x, int32 y);
static void translateGhost(Ghost *ghost);
static void buildPlayerPath(Ghost *ghost);
static void updateGhostPath(Ghost *ghost);
static void updateGhostRandom(Ghost *ghost, int32 influence);
static void setGhostVector(Ghost *ghost, int32 x, int32 y);
static void setGhostTarget(Ghost *ghost, int32 mapX, int32 mapY);
static bool hasGhostReachedTarget(Ghost *ghost);

/********************************************************************/
/* ---------------- EXTERN FUNCTION IMPLEMENTATION ---------------- */
/********************************************************************/


void initGhosts(void)
{
	int32 i;
	Ghost *ghost;
	
	// load spritesets
	spritesetIds[GhostSS1] = loadSpriteset(GET_GFX_ASSET_PATH("ghost1.anim"));
	spritesetIds[GhostSS2] = loadSpriteset(GET_GFX_ASSET_PATH("ghost2.anim"));
	spritesetIds[GhostSS3] = loadSpriteset(GET_GFX_ASSET_PATH("ghost3.anim"));
	spritesetIds[GhostSS4] = loadSpriteset(GET_GFX_ASSET_PATH("ghost4.anim"));
	spritesetIds[GhostEyesSS] = loadSpriteset(GET_GFX_ASSET_PATH("eyes.anim"));
	spritesetIds[GhostRunSS] = loadSpriteset(GET_GFX_ASSET_PATH("run.anim"));
	
	for (i = 0; i < MAX_GHOSTS; i++)
	{
		ghost = &ghosts[i];
		ghost->index = i;
		
		ghost->base = cloneSpriteCCB(spritesetIds[i]);
		ghost->lastAnimTime = 0;
		ghost->animDelay = 80;
		ghost->baseSpritesetId = spritesetIds[i];
		ghost->eyes = cloneSpriteCCB(spritesetIds[GhostEyesSS]);
		setSpriteFrame(ghost->eyes, spritesetIds[GhostEyesSS], 0);
		ghost->base->ccb_NextPtr = ghost->eyes;
		ghost->hasPath = FALSE;
		
		// link the ghosts and eyes together

		if(ghost->base->ccb_Flags & CCB_LAST)
			ghost->base->ccb_Flags &= ~CCB_LAST;

		if(ghost->eyes->ccb_Flags & CCB_LAST)
			ghost->eyes->ccb_Flags &= ~CCB_LAST;
		
		if(i > 0)
			ghosts[i-1].eyes->ccb_NextPtr = ghost->base;
	}		
}

void cleanupGhosts(void)
{
	int32 i;
	
	for (i = 0; i < MAX_GHOSTS; i++)
	{
		if (ghosts[i].base != NULL)
			DeleteCel(ghosts[i].base);			
		
		if (ghosts[i].eyes != NULL)
			DeleteCel(ghosts[i].eyes);
	}		
}

void updateGhosts(void)
{
	int32 i;
	Coord tempx;
	int32 currentTime = GetMSecTime(timerIOReq);
	
	for (i = 0; i < MAX_GHOSTS; ++i)
	{
		Ghost *ghost = &ghosts[i];
		
		#if DEBUG_VERBOSE
		if (ghost->worldx % 2 != 0 || ghost->worldy % 2 != 0)
			printLine("updateGhosts() - Ghost %d has odd position? %dx%d", i, ghost->worldx, ghost->worldy);
		#endif

		// check if it's time to animate
		if (currentTime - ghost->lastAnimTime >= ghost->animDelay)
		{
			incAnimFrame(ghost->baseSpritesetId, &ghost->frame);
			setSpriteFrame(ghost->base, ghost->baseSpritesetId, ghost->frame);
			ghost->lastAnimTime = currentTime;
		}

		if (ghost->state == JAILED)
		{
			// keep ghost bouncing around in jail
			tempx = ghost->worldx + ghost->currentVec.x * STEP_SIZE;

			if (tempx <= JAIL_MIN_X)
			{
				tempx = JAIL_MIN_X;
				setGhostPosition(ghost, tempx, ghost->worldy);
				setGhostVector(ghost, 1, ghost->currentVec.y);
			}
			if (tempx >= JAIL_MAX_X) 
			{
				tempx = JAIL_MAX_X;
				setGhostPosition(ghost, tempx, ghost->worldy);
				setGhostVector(ghost, -1, ghost->currentVec.y);
			}

			translateGhost(ghost);

			if (ghost->worldx == JAIL_BREAK_X)
			{
				if (currentTime - ghost->jailTime >= 2000)
				{
					setGhostState(ghost, ESCAPE);
					setGhostVector(ghost, 0, -1);
				}
			}
		}
		else if (ghost->state == ESCAPE)
		{
			translateGhost(ghost);

			if (ghost->worldy == JAIL_BREAK_Y)
			{
				// ghost->worldx = 72; // tile-align it
				setGhostState(ghost, PLAY);
				setGhostVector(ghost, (rand() % 2 == 0) ? -1 : 1, 0);
			}
		}
		else if (ghost->state == PLAY)
		{				
			switch(i)
			{
			case FOLLOW_PLAYER:
				updateGhostFollow(ghost);
				break;
				
			case RANDOM:
				updateGhostRandom(ghost, 0);
				break;
				
			case RANDOM_TOP:
				updateGhostRandom(ghost, -1);
				break;
				
			case RANDOM_BOTTOM:
				updateGhostRandom(ghost, 1);
				break;
				
			default:
				break;
			}			
		}
		else if (ghost->state == RUN)
		{
			updateGhostRandom(ghost, 0);
			
			if (isSimpleTimerReady(&powerupTimer) == TRUE)
			{
				// lets switch ghosts back to play state once they are evenly within a tile			
				if (IS_AT_INTERSECTION(ghost->worldx, ghost->worldy))
				{
					// rare edge case check
					// what if ghost switches back to play state while in tunnel?
					// the ghost would be off the board, and if this is a follow ghost, path building will break
					if (!isPointOnGameBoard(ghost->worldx, ghost->worldy))
					{
						printLine("updateGhosts() - Ghost %d needs new path to player but is off game board", ghost->index);

						// find out which tunnel the ghost is in

						if (ghost->worldx < 80)
						{
							// left tunnel
							ghost->worldx = 0; // snap on board
							printLine("snapping ghost to left of board");
						}
						else 
						{
							// right tunnel
							ghost->worldx = 150; // snap on board
							printLine("snapping ghost to right of board");
						}
					}

					setGhostState(ghost, PLAY);
					// need to set frame right away due to race condition
					// otherwise ghost color will be off while player collision is made
					setSpriteFrame(ghost->base, ghost->baseSpritesetId, ghost->frame);
					// if ghost had a path prior to run state, it is gone now, force a new one
					ghost->hasPath = FALSE;
				}
				
				successionCount = 0;
			}
		}
		else if (ghost->state == RETURN)
		{
			if (ghost->hasPath == FALSE)
			{				
				if (IS_AT_INTERSECTION(ghost->worldx, ghost->worldy))
				{
					// rare edge case check 
					// what if ghost is eaten while in tunnel and off game board?
					// path to return to jail will fail since starting position is out of bounds
					
					if (!isPointOnGameBoard(ghost->worldx, ghost->worldy))
					{
						printLine("updateGhosts() - Ghost needs new path to jail but is off game board");

						// find out which tunnel the ghost is in

						if (ghost->worldx < 80)
						{
							// left tunnel
							ghost->worldx = 0; // snap on board
							printLine("snapping ghost to left of board");
						}
						else 
						{
							// right tunnel
							ghost->worldx = 150; // snap on board
							printLine("snapping ghost to right of board");
						}
					}

					setGhostTarget(ghost, 12, 13);
				}
			}

			if (ghost->hasPath)
			{
				if (hasGhostReachedTarget(ghost))
				{
					ghost->jailTime = currentTime;				
					setGhostState(ghost, JAILED);
					continue;
				}
				else 
				{
					updateGhostPath(ghost);
				}
			}

			translateGhost(ghost);
		}

		// check for ghost tunnel warp
		warp(&ghost->worldx, ghost->currentVec.x);

		// update ghost eyes
	
		if (ghost->currentVec.x < 0)
			setSpriteFrame(ghost->eyes, spritesetIds[GhostEyesSS], 0);
		else if (ghost->currentVec.x > 0)
			setSpriteFrame(ghost->eyes, spritesetIds[GhostEyesSS], 1);
		else if (ghost->currentVec.y < 0)
			setSpriteFrame(ghost->eyes, spritesetIds[GhostEyesSS], 3);
		else if (ghost->currentVec.y > 0)
			setSpriteFrame(ghost->eyes, spritesetIds[GhostEyesSS], 2);

		ghostToScreen(ghost);
	}
}

void resetGhosts(void)
{
	int32 i;
	Ghost *ghost;
	
	for (i = 0; i < MAX_GHOSTS; i++)
	{
		ghost = &ghosts[i];
		
		ghost->frame = 0;
		ghost->baseSpritesetId = spritesetIds[ghost->index];
		setSpriteFrame(ghost->base, ghost->baseSpritesetId, ghost->frame);		
		setGhostState(ghost, JAILED);
		setGhostPosition(ghost, (11 + i) * TILE_SIZE, 13 * TILE_SIZE);	
		SHOW_SPRITE(ghost->base);
		SHOW_SPRITE(ghost->eyes);
		ghost->hasPath = FALSE;
	}
}

void setGhostState(Ghost *ghost, int32 state)
{
	if (state == RUN)
	{
		ghost->baseSpritesetId = spritesetIds[GhostRunSS];
		HIDE_SPRITE(ghost->eyes);
	}
	else if (state == PLAY)
	{
		ghost->baseSpritesetId = spritesetIds[ghost->index];		
		SHOW_SPRITE(ghost->eyes);		
	}
	else if (state == RETURN)
	{
		HIDE_SPRITE(ghost->base);
		SHOW_SPRITE(ghost->eyes);
		ghost->hasPath = FALSE; // in case it had one, it's gone now
	}
	else if (state == JAILED)
	{
		ghost->baseSpritesetId = spritesetIds[ghost->index];
		setGhostVector(ghost, (rand() % 2 == 0) ? 1 : -1, 0);
		SHOW_SPRITE(ghost->base);
		ghost->hasPath = FALSE; // force new path when he gets out
	}
	
	ghost->state = (GhostState) state;
}

/********************************************************************/
/* ---------------- STATIC FUNCTION IMPLEMENTATION ---------------- */
/********************************************************************/

bool hasGhostReachedTarget(Ghost *ghost)
{
	// check if ghost is evenly within a map tile
	if (IS_AT_INTERSECTION(ghost->worldx, ghost->worldy))
	{
		// check if map position equals target
		if (ghost->target.pt_X == ghost->worldx / TILE_SIZE && ghost->target.pt_Y == ghost->worldy / TILE_SIZE)
			return(TRUE);
	}
	
	return(FALSE);
}

void setGhostTarget(Ghost *ghost, int32 mapx, int32 mapy)
{
	Point start;
	start.pt_X = ghost->worldx;
	start.pt_Y = ghost->worldy;

	if (ghost->worldx < 0 || ghost->worldx >= TILE_SIZE * MAP_WIDTH || 
		ghost->worldy < 0 || ghost->worldy >= TILE_SIZE * MAP_HEIGHT)
	{
		printLine("setGhostTarget() ERROR - can't set ghost %d target path start when off board! %dx%d", ghost->index, ghost->worldx, ghost->worldy);
	}
	else 
	{
		start.pt_X /= TILE_SIZE;
		start.pt_Y /= TILE_SIZE;
		ghost->target.pt_X = mapx;
		ghost->target.pt_Y = mapy;

		buildPath(&start, &ghost->target, ghost->nodes, &ghost->nodeIndex);
		ghost->hasPath = TRUE;
	}
}

void setGhostVector(Ghost *ghost, int32 x, int32 y)
{
	ghost->currentVec.x = x;
	ghost->currentVec.y = y;		
}

/*
	Random movement AI
	influence -1	= patrol upper board 
	influence 0 	= patrol entire board 
	influence 1 	= patrol lower board 
*/
void updateGhostRandom(Ghost *ghost, int32 influence)
{
	Vector xchecks[2];	// vectors along the x plane
	Vector ychecks[2];	// vectors along the y plane
	Vector *vPtr;
	Coord checkx;
	Coord checky;
	
	if (IS_AT_INTERSECTION(ghost->worldx, ghost->worldy))
	{
		xchecks[0].x = -1;	// left
		xchecks[0].y = 0;
		xchecks[1].x = 1;	// right
		xchecks[1].y = 0;
		
		ychecks[0].x = 0;	// top
		ychecks[0].y = -1;
		ychecks[1].x = 0;	// bottom
		ychecks[1].y = 1;

		if (ghost->currentVec.x == 0 && ghost->currentVec.y != 0)
		{
			// ghost is moving vertically, check horizontally
			vPtr = &xchecks[rand() % 2]; // 0 - 1			
		}
		else if (ghost->currentVec.x != 0 && ghost->currentVec.y == 0)
		{
			// ghost is moving horizontally, check vertically

			vPtr = &ychecks[rand() % 2]; // default, 0 - 1

			if (influence == -1)
			{
				// prioritize top half of board
				if (ghost->worldy >= 78)
					vPtr = &ychecks[0];
			}
			else if (influence == 1)
			{
				// prioritize bottom half of board
				if (ghost->worldy <= 78)
					vPtr = &ychecks[1];
			}
		}
		else 
		{
			// should never reach here 
			printLine("updateGhostRandom() ERROR - Ghost %d has mixed vector %dx%d", ghost->index, ghost->currentVec.x, ghost->currentVec.y);
		}

		checkx = ghost->worldx + vPtr->x * STEP_SIZE;
		checky = ghost->worldy + vPtr->y * STEP_SIZE;

		if (checkMove(checkx, checky, vPtr))
		{
			setGhostVector(ghost, vPtr->x, vPtr->y);
			translateGhost(ghost);
		}
		else 
		{
			// could not change direction this intersection
			// see if we need to flip

			checkx = ghost->worldx + ghost->currentVec.x * STEP_SIZE;
			checky = ghost->worldy + ghost->currentVec.y * STEP_SIZE;

			if (checkMove(checkx, checky, &ghost->currentVec))
			{
				// stay the course 
				translateGhost(ghost);
			}
			else 
			{
				// flip 
				setGhostVector(ghost, ghost->currentVec.x * -1, ghost->currentVec.y * -1);
			}
		}
	}
	else 
	{
		translateGhost(ghost); // keep doing you, ghost
	}
}

void updateGhostPath(Ghost *ghost)
{
	Point worldpt;

	if (ghost->worldx < 0 || ghost->worldy < 0 || ghost->worldx >= TILE_SIZE * MAP_WIDTH || ghost->worldy >= TILE_SIZE * MAP_HEIGHT)
	{
		printLine("updateGhostPath() ERROR - Ghost %d world coords are out of bounds %dx%d", ghost->index, ghost->worldx, ghost->worldy);
		return;
	}

	// check for intersection 
	if (IS_AT_INTERSECTION(ghost->worldx, ghost->worldy))
	{
		if (ghost->nodeIndex > -1)
		{
			worldpt.pt_X = ghost->worldx / TILE_SIZE;
			worldpt.pt_Y = ghost->worldy / TILE_SIZE;

			// set vector based on heading
			
			// check left, up, right, down
			if (ghost->nodes[ghost->nodeIndex].pt_X < worldpt.pt_X)
			{
				// left
				setGhostVector(ghost, -1, 0);
			}
			else if (ghost->nodes[ghost->nodeIndex].pt_X > worldpt.pt_X) 
			{
				// right
				setGhostVector(ghost, 1, 0);
			}
			else if (ghost->nodes[ghost->nodeIndex].pt_Y < worldpt.pt_Y) 
			{
				// up
				setGhostVector(ghost, 0, -1);
			}
			else if (ghost->nodes[ghost->nodeIndex].pt_Y > worldpt.pt_Y) 
			{
				// down
				setGhostVector(ghost, 0, 1);
			}

			--ghost->nodeIndex;
		}
		else 
		{
			// ghost is over target / reached the end
			setGhostVector(ghost, 0, 0);
		}
	}
}

void updateGhostFollow(Ghost *ghost)
{
	if (ghost->hasPath == FALSE)
	{
		// track player, first run
		// only start during intersection
		// ghost will have a random -1 or 1 x vector until then
		if (IS_AT_INTERSECTION(ghost->worldx, ghost->worldy))
			buildPlayerPath(ghost);
	}

	if (ghost->hasPath == TRUE)
	{
		if (hasGhostReachedTarget(ghost) == TRUE) // can only be true once ghost is at intersection			
			buildPlayerPath(ghost); // keep searching for player, he could have moved, if not he is dead

		updateGhostPath(ghost); // correct vector
	}

	translateGhost(ghost); // if vector is not properly set before calling this all hell breaks loose
}

void ghostToScreen(Ghost *ghost)
{
	ghost->base->ccb_XPos = (ghost->worldx + X_OFFSET) << FRACBITS_16;
	ghost->base->ccb_YPos = (ghost->worldy + Y_OFFSET) << FRACBITS_16;
	ghost->eyes->ccb_XPos = ghost->base->ccb_XPos;
	ghost->eyes->ccb_YPos = ghost->base->ccb_YPos;
}

void setGhostPosition(Ghost *ghost, int32 x, int32 y) 
{
	ghost->worldx = x;
	ghost->worldy = y;
	ghostToScreen(ghost);
}

void translateGhost(Ghost *ghost)
{
	ghost->worldx += ghost->currentVec.x * STEP_SIZE;
	ghost->worldy += ghost->currentVec.y * STEP_SIZE;
	ghostToScreen(ghost);

	#if DEBUG_VERBOSE
	if (ghost->worldx < 0 || ghost->worldy < 0 || ghost->worldx >= TILE_SIZE * MAP_WIDTH || ghost->worldy >= TILE_SIZE * MAP_HEIGHT)
	{
		if (ghost->worldy != 78) // ignore tunnel, special case
			printLine("translateGhost() ERROR - Ghost %d is out of bounds %dx%d", ghost->index, ghost->worldx, ghost->worldy);
	}
	#endif
}

void buildPlayerPath(Ghost *ghost)
{
	Coord targetx = player.worldx;
	Coord targety = player.worldy;
	int32 mapx;
	int32 mapy;

	if (isPointOnGameBoard(targetx, targety))
	{		
		mapx = targetx / TILE_SIZE;
		mapy = targety / TILE_SIZE;		
	}
	else 
	{
		printLine("buildPlayerPath() - Player is off board, go to center");
		mapx = 13;
		mapy = 16;
	}

	if (!isTargetValid(mapx, mapy))
	{
		// this should not happen, but just in case
		printLine("buildPlayerPath() ERROR - Failed to build ghost %d path for %dx%d, setting 0x0.", ghost->index, mapx, mapy);
		mapx = 0;
		mapy = 0;
	}
	
	setGhostTarget(ghost, mapx, mapy);
}
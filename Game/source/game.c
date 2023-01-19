#include "pacman.h"


/********************************************************************/
/* --------------------- CONSTANTS AND MACROS --------------------- */
/********************************************************************/


#define INC_SCORE(amount) (score += amount)
#define PLAYER_MOVE_DELAY 29000
#define GHOST_MOVE_DELAY 40000
#define MAX_POWERUPS 4
// player starts with 3 lives and is awared 1 extra life at 10K points
#define MAX_LIVES 4
#define PELLETS_PER_MATCH 254
#define PELLET_SCORE_VALUE 10
#define PUP_SCORE_VALUE 50
// score multiplier per ghost eaten in succession
#define GHOST_SCORE_MULT 2
#define GHOST_SCORE_VALUE 200
#define EXTRA_LIFE_SCORE 10000
// colors 
#define RGB15_WHITE 32767
#define RGB15_YELLOW 30624
#define RGB15_RED 22594


/********************************************************************/
/* ------------------- TYPES AND DATASTRUCTURES ------------------- */
/********************************************************************/


typedef enum
{
	GET_READY_SD = 0,
	SIREN_SD,
	EAT_SD,
	DEATH_SD,
	CHOMP_SD,
	ONE_UP,
	MAX_SD
} Sounds;

typedef enum
{
	GAME_GET_READY = 0,
	GAME_GET_SET,
	GAME_PLAY,
	GAME_DEATH,
	GAME_RESTART,
	GAME_OVER,
	GAME_STATE_MAX
} GameState;

typedef enum
{
	CHERRY = 0,
	STRAWBERRY,
	ORANGE,
	APPLE,
	GRAPE,
	GALAXIAN,
	BELL,
	KEY,
	MAX_FRUITS
} Fruit;

// score indices align with fruit enum order
static int32 fruitScoreValues[MAX_FRUITS] = 
{
	100,
	300,
	500,
	700,
	1000,
	2000,
	3000,
	5000
};


/********************************************************************/
/* -------------------- STATIC AND EXTERN VARS -------------------- */
/********************************************************************/


// extern
Player player;
int32 successionCount;
SimpleTimer powerupTimer;

Boolean map[MAP_SIZE] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,1,1,
	1,1,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,1,1,
	1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,
	0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,1,1,
	1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,
	1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,
	0,0,1,1,0,1,1,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,0,1,1,0,0,
	1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,
	1,1,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
}; 

// static
static SimpleTimer getReadyTimer;
static SimpleTimer sirenTimer;
static SimpleTimer chompTimer;
static SimpleTimer fpsTimer;
static uint32 fpsCount;
static uint32 fps;
static ubyte *gameBackground = NULL;
static uint32 moveTimerSignal;
static uint32 ghostTimerSignal;
static TimerHandle moveTimerHandle;
static TimerHandle ghostTimerHandle;
static int32 pelletsEaten; 
static GameState gameState;
static int32 level;
static CCB *powerups[MAX_POWERUPS];
static int32 nLives;
static CCB *lives[MAX_LIVES];
static CCB *fruit;
static CCB *fruitHUD;
static int32 fruitSpritesetIndex;
static Fruit activeFruitIndex;
static int32 fruitTriggerCounts[2] = {85, 170};
static int32 fruitTriggerIndex;
static int32 score;
static Boolean extraLifeAwarded;
static Item sounds[MAX_SD];
static int32 chompInstIndex;
static Boolean chomping;
static TextCel *scoretcel;
static TextCel *overlaytcel;
static FontDescriptor *gfont;
static void(*gameStateHandlers[GAME_STATE_MAX])(void);


/********************************************************************/
/* ------------------- STATIC FUNCTION PROTOTYPES ----------------- */
/********************************************************************/


static void loadAudio(void);
static void unloadAudio(void);
static void setupFruit(void);
static void updateScore(void);
static void resetMatch(Boolean wipe);
static void nextLevel(void);
static void resetGame(void);
static void updatePlayer(void);
static void checkCollisions(void);
static void initPowerups(void);
static void linkCCBs(void);
static void gameStateHandlerReady(void);
static void gameStateHandlerSet(void);
static void gameStateHandlerDeath(void);
static void gameStateHandlerRestart(void);
static void gameStateHandlerPlay(void);
static void gameStateHandlerOver(void);
static void initGame(void);
static void vramDump(void);
static void playerToScreen(void);
// static void unloadGame(void);

/********************************************************************/
/* ---------------- STATIC FUNCTION IMPLEMENTATION ---------------- */
/********************************************************************/

void loadAudio(void)
{
	sounds[GET_READY_SD] = LoadSample(GET_SFX_ASSET_PATH("ready.aiff"));
	sounds[SIREN_SD] = LoadSample(GET_SFX_ASSET_PATH("siren.aiff"));
	sounds[EAT_SD] = LoadSample(GET_SFX_ASSET_PATH("eat.aiff"));
	sounds[DEATH_SD] = LoadSample(GET_SFX_ASSET_PATH("death.aiff"));
	sounds[ONE_UP] = LoadSample(GET_SFX_ASSET_PATH("1up.aiff"));
	sounds[CHOMP_SD] = LoadSample(GET_SFX_ASSET_PATH("chomp.aiff"));
}

void unloadAudio(void)
{
	UnloadSample(sounds[GET_READY_SD]);
	UnloadSample(sounds[SIREN_SD]);
	UnloadSample(sounds[EAT_SD]);
	UnloadSample(sounds[DEATH_SD]);
	UnloadSample(sounds[ONE_UP]);
	UnloadSample(sounds[CHOMP_SD]);
}

void playerToScreen(void)
{
	player.ccb->ccb_XPos = (player.worldx + X_OFFSET) << FRACBITS_16;
	player.ccb->ccb_YPos = (player.worldy + Y_OFFSET) << FRACBITS_16;

	if (player.currentVec.x < 0)
		rotSprite(player.ccb, ANGLE_180);
	else if (player.currentVec.x > 0) 
		rotSprite(player.ccb, ANGLE_0);
	else if (player.currentVec.y < 0)
		rotSprite(player.ccb, ANGLE_270);
	else if (player.currentVec.y > 0)
		rotSprite(player.ccb, ANGLE_90);

	if (player.ccb->ccb_HDX < 0)
		player.ccb->ccb_XPos += SPRITE_SIZE << FRACBITS_16;
	
	if (player.ccb->ccb_HDY < 0)
		player.ccb->ccb_YPos += SPRITE_SIZE << FRACBITS_16;
}

void setupFruit(void)
{
	/********************************************************************************
		Set active fruit from current level 
		
		Cherry on level 1 
		Strawberry on level 2
		Orange on levels 3 and 4
		Apple on levels 5 and 6
		Grape on levels 7 and 8
		Galaxian on levels 9 and 10
		Bell on levels 11 and 12
		Key on levels 13 and above
	********************************************************************************/
	
	switch(level)
	{
	case 1:
		activeFruitIndex = CHERRY;
		break;
	case 2:
		activeFruitIndex = STRAWBERRY;
		break;
	case 3:
	case 4:
		activeFruitIndex = ORANGE;
		break;
	case 5:
	case 6:
		activeFruitIndex = APPLE;
		break;
	case 7:
	case 8:
		activeFruitIndex = GRAPE;
		break;
	case 9:
	case 10:
		activeFruitIndex = GALAXIAN;
		break;
	case 11:
	case 12:
		activeFruitIndex = BELL;
		break;
	default:
		activeFruitIndex = KEY;
		break;
	}
	
	setSpriteFrame(fruit, fruitSpritesetIndex, activeFruitIndex);
	setSpriteFrame(fruitHUD, fruitSpritesetIndex, activeFruitIndex);
	
	HIDE_SPRITE(fruit);
}

void updateScore(void)
{
	char scoreTextString[50];
	sprintf(scoreTextString, "SCORE %u", score);
    UpdateTextInCel(scoretcel, TRUE, scoreTextString);
}

/*
// no use case for this to ever run at this time
void unloadGame(void)
{
	int32 i;
	int32 signals;

	if (gameBackground != NULL)
		UnloadImage(gameBackground);
	
	if (player.ccb != NULL)
		DeleteCel(player.ccb);
	
	if (fruit != NULL)
		DeleteCel(fruit);

	if (fruitHUD != NULL)
		DeleteCel(fruitHUD);
	
	UnloadFont(gfont);

	DeleteTextCel(scoretcel);
	DeleteTextCel(overlaytcel);

	cleanupGhosts();
	
	for (i = 0; i < MAX_POWERUPS; i++)
	{
		if (powerups[i] != NULL)
			DeleteCel(powerups[i]);
	}
	
	for (i = 0; i < MAX_LIVES; i++)
	{
		if (lives[i] != NULL)
			DeleteCel(lives[i]);
	}
	
	freeSpritesets();
		
	TimerCancel(moveTimerHandle);
	TimerCancel(ghostTimerHandle);
	
	signals = GetCurrentSignals();
	
	if(signals & moveTimerSignal)
		WaitSignal(moveTimerSignal);
	
	if(signals & ghostTimerSignal)
		WaitSignal(ghostTimerSignal);

	FreeSignal(moveTimerSignal);
	FreeSignal(ghostTimerSignal);
	
	TimerServicesShutdown();
	audioCleanup();
	unloadAudio();
}
*/

void resetMatch(Boolean wipe)
{
	int32 x;
	int32 y;
	
	// reset player
	player.frame = 0;
	setSpriteFrame(player.ccb, player.setIndex, player.frame);
	player.currentVec.x = 0;
	player.currentVec.y = 0;
	player.desiredVec = player.currentVec;
	player.worldx = 76;
	player.worldy = 132;
	rotSprite(player.ccb, ANGLE_0);
	playerToScreen();
	
	// reset ghosts
	resetGhosts();
	
	/*	Draw food pellets over background image.
		As pacman "eats" them, use dirty rectangles to black them out.
	*/
	
	if (wipe == TRUE)
	{
		int32 plotX;
		int32 plotY;
		uint32 *ptr = (uint32*)gameBackground;
		uint32 color = white;
		pelletsEaten = 0;
		fruitTriggerIndex = 0;
		
		for (y = 0; y < MAP_HEIGHT - 1; y++)
		{
			for (x = 0; x < MAP_WIDTH - 1; x++)
			{
				// skip center area because this is what the original game did
				if (x > 5 && x < 20 && y > 7 && y < 19)
					continue;

				// also skip 2 pellets under pacman
				if (y == 22 && (x == 12 || x == 13))
					continue;
				
				/* 
					Plot for each 2x2 tile of 8x8 pixels.
					The code below will draw a pellet at the center of each 2x2 matrix.
					Read this code while viewing a fully drawn gameboard with pellets for reference.
				*/
				
				if (map[y * MAP_WIDTH + x] != 0 &&
					map[y * MAP_WIDTH + (x+1)] != 0 &&
					map[(y+1) * MAP_WIDTH + x] != 0 &&
					map[(y+1) * MAP_WIDTH + (x+1)] != 0)
				{
					// draw 1x2 pellet
					plotX = x * TILE_SIZE + TILE_SIZE + X_OFFSET;
					plotY = y * TILE_SIZE + TILE_SIZE + Y_OFFSET;
					ptr[(plotY / 2) * SCREEN_WIDTH + plotX] = color;
				}			
			}
		}
		
		// restore powerups
		for (x = 0; x < MAX_POWERUPS; x++)
			SHOW_SPRITE(powerups[x]);
	}		
	
	// reset game state
	gameState = GAME_GET_READY;
	
	successionCount = 0;
	
	// show the correct number of lives remaining
	
	for (x = MAX_LIVES; x > 0; x--)
	{
		if (nLives < x)
			HIDE_SPRITE(lives[x-1]);
		else 
			SHOW_SPRITE(lives[x-1]);
	}
	
	// determine which fruit is the level's active fruit
	setupFruit();
}

void nextLevel(void)
{
	// increment level, way to go
	level++;
	
	#if 1
	// play intermission every two levels
	if(level % 2 == 0)
	{
		audioCleanup();
		unloadAudio();
		execProgram("Intermission");
		audioInit();
		loadAudio();
	}
	#endif 

	// get new match ready
	resetMatch(TRUE);

	vramDump();
}

void resetGame(void)
{
	score = 0;
	level = 1;
	nLives = 3;
	printLine("resetting lives to %d\n", nLives);
	extraLifeAwarded = FALSE;
	resetMatch(TRUE);
}

void updatePlayer(void)
{
	Coord checkx;
	Coord checky;
	uint32 *ptr = (uint32*)gameBackground;
	uint32 offset;
	int32 mapx, mapy;

	// check if player has a desired vector / wants to move 
	if (player.desiredVec.x != player.currentVec.x || player.desiredVec.y != player.currentVec.y)
	{
		checkx = player.worldx + player.desiredVec.x * STEP_SIZE;
		checky = player.worldy + player.desiredVec.y * STEP_SIZE;

		// do an early spot-check just to save some cycles in checkMove()..
		if (isPointOnGameBoard(checkx, checky))
		{
			if (checkMove(checkx, checky, &player.desiredVec))
				player.currentVec = player.desiredVec; // set it
		}
	}

	// are we moving?

	if (player.currentVec.x + player.currentVec.y != 0)
	{
		// we are moving, check next move

		checkx = player.worldx + player.currentVec.x * STEP_SIZE;
		checky = player.worldy + player.currentVec.y * STEP_SIZE;
	
		if (!checkMove(checkx, checky, &player.currentVec))
		{
			// invalid move, stop player
			player.currentVec.x = 0;
			player.currentVec.y = 0;
			player.desiredVec = player.currentVec;
		}
		else 
		{
			// move
			player.worldx = checkx;
			player.worldy = checky;

			mapx = player.worldx / TILE_SIZE;
			mapy = player.worldy / TILE_SIZE;

			// check for pellet

			// map check just to be safe
			if (mapx >= 0 && mapx < MAP_WIDTH * TILE_SIZE && mapy >= 0 && mapy < MAP_HEIGHT * TILE_SIZE)
			{
				offset = (player.worldy + Y_OFFSET + TILE_SIZE) / 2 * SCREEN_WIDTH + (player.worldx + X_OFFSET + TILE_SIZE);
				
				if (ptr[offset] == white)
				{
					// remove pellet, nom nom
					ptr[offset] = black;
					++pelletsEaten;

					INC_SCORE(PELLET_SCORE_VALUE);
					
					if (isSimpleTimerReady(&chompTimer) == TRUE)
					{
						playSound(sounds[CHOMP_SD], 200);
						restartSimpleTimer(&chompTimer);
					}
					
					if (fruitTriggerIndex < 2 && pelletsEaten >= fruitTriggerCounts[fruitTriggerIndex])
					{
						// show fruit
						SHOW_SPRITE(fruit);
						++fruitTriggerIndex;
					}
				}			
			}
		}
	}

	// check for warp
	warp(&player.worldx, player.currentVec.x);

	// prep for drawing
	playerToScreen();
}

void checkCollisions(void)
{
	SRect rect1;
	SRect rect2;
	int32 i;
	
	// rect 1 will equal player, use for all player collision checks
	rect1.pos.x = player.worldx;
	rect1.pos.y = player.worldy;
	rect1.size.x = SPRITE_SIZE;
	rect1.size.y = SPRITE_SIZE;
	
	// check for powerups
	
	for (i = 0; i < MAX_POWERUPS; i++)
	{
		if( !(powerups[i]->ccb_Flags & CCB_SKIP) ) // active powerups only
		{
			rect2.pos.x = (powerups[i]->ccb_XPos >> FRACBITS_16) - X_OFFSET;
			rect2.pos.y = (powerups[i]->ccb_YPos >> FRACBITS_16) - Y_OFFSET;
			rect2.size.x = 8;
			rect2.size.y = 8;
			
			if (rectCollision(&rect1, &rect2) == TRUE)
			{
				// hit
				int32 j;
				
				HIDE_SPRITE(powerups[i]);
				playSound(sounds[EAT_SD], 30);
				INC_SCORE(PUP_SCORE_VALUE);
				
				restartSimpleTimer(&powerupTimer);
					
				for (j = 0; j < MAX_GHOSTS; ++j)
				{
					if (ghosts[j].state != PLAY) 
						continue;

					setGhostState(&ghosts[j], RUN);
				}
				
				break; // we know we can't be at two places at once!
			}
		}
	}
	
	// check for fruit
	
	if (!(fruit->ccb_Flags & CCB_SKIP))
	{
		rect2.pos.x = (fruit->ccb_XPos >> FRACBITS_16) - X_OFFSET;
		rect2.pos.y = (fruit->ccb_YPos >> FRACBITS_16) - Y_OFFSET;
		rect2.size.x = SPRITE_SIZE;
		rect2.size.y = SPRITE_SIZE;
		
		if (rectCollision(&rect1, &rect2) == TRUE)
		{
			// hit
			HIDE_SPRITE(fruit);
			playSound(sounds[EAT_SD], 30);
			INC_SCORE(fruitScoreValues[activeFruitIndex]);
		}
	}
	
	// check for ghosts

	for (i = 0; i < MAX_GHOSTS; i++)
	{
		Ghost *ghost = &ghosts[i];
		
		if (ghost->state != PLAY && ghost->state != RUN)
			continue; // in jail or returning to jail
		
		rect2.pos.x = ghost->worldx + 4; // make bounding box a little smaller so collision isn't so strict
		rect2.pos.y = ghost->worldy + 4;
		rect2.size.x = SPRITE_SIZE - 8;
		rect2.size.y = SPRITE_SIZE - 8;
		
		if (rectCollision(&rect1, &rect2) == TRUE)
		{
			// hit!
			if (ghost->state == RUN)
			{
				// eat ghost 				
				setGhostState(ghost, RETURN);
				++successionCount;
				INC_SCORE(successionCount * GHOST_SCORE_VALUE);
				playSound(sounds[EAT_SD], 100);
			}
			else 
			{
				// player eaten 

				#if !GOD_MODE
				playSound(sounds[DEATH_SD], 100);
				gameState = GAME_DEATH;
				player.frame = 0;
				setSpriteFrame(player.ccb, player.setIndex, player.frame);
				player.currentVec.x = 0;
				player.currentVec.y = 0;
				player.desiredVec = player.currentVec;
				
				HIDE_SPRITE(lives[nLives-1]);
				--nLives;
				#endif

				printLine("player hit, %d lives remaining", nLives);

				break; // one ghost is enough!
			}
		}
	}
}

void initPowerups(void)
{
	int32 i;
	int32 tempPUX[MAX_POWERUPS] = {82, 232, 82, 232};
	int32 tempPUY[MAX_POWERUPS] = {45, 45, 165, 165};
	
	for (i = 0; i < MAX_POWERUPS; i++)
	{
		if (i == 0)		
			powerups[0] = LoadCel(GET_GFX_ASSET_PATH("pup.cel"), MEMTYPE_CEL);
		else
			powerups[i] = CloneCel(powerups[0], CLONECEL_CCB_ONLY);
		
		if (powerups[i]->ccb_Flags & CCB_LAST)
			powerups[i]->ccb_Flags &= ~CCB_LAST;

		setSpritePosition(powerups[i], tempPUX[i], tempPUY[i]);
		
		if(i > 0)
			powerups[i-1]->ccb_NextPtr = powerups[i];
	}
}

void linkCCBs(void)
{
	// draw in one swoop
	ghosts[MAX_GHOSTS-1].eyes->ccb_NextPtr = powerups[0];	
	powerups[MAX_POWERUPS-1]->ccb_NextPtr = lives[0];
	lives[MAX_LIVES-1]->ccb_NextPtr = fruitHUD;
	fruitHUD->ccb_NextPtr = fruit;
	fruit->ccb_NextPtr = player.ccb;
	player.ccb->ccb_NextPtr = scoretcel->tc_CCB;
	scoretcel->tc_CCB->ccb_NextPtr = overlaytcel->tc_CCB;
	overlaytcel->tc_CCB->ccb_Flags |= CCB_LAST;
	overlaytcel->tc_CCB->ccb_NextPtr = NULL;
}

void gameStateHandlerReady(void)
{
	// show ready message 
	SetTextCelColor(overlaytcel, 0, RGB15_YELLOW);
    SetTextCelCoords(overlaytcel, 144 << FRACBITS_16, 127 << FRACBITS_16);
	UpdateTextInCel(overlaytcel, TRUE, "READY!");
	SHOW_SPRITE(overlaytcel->tc_CCB);

	// play a little tune
	playSound(sounds[GET_READY_SD], 50);
	gameState = GAME_GET_SET;		
	restartSimpleTimer(&getReadyTimer);
}

void gameStateHandlerSet(void) 
{
	int32 i;
	int32 currentTime = GetMSecTime(timerIOReq);

	// check if it's time to play
	if (isSimpleTimerReady(&getReadyTimer) == TRUE)
	{
		gameState = GAME_PLAY;			
		
		for (i = 0; i < MAX_GHOSTS; i++)
			ghosts[i].jailTime = currentTime;
			
		restartSimpleTimer(&sirenTimer);

		HIDE_SPRITE(overlaytcel->tc_CCB);
	}
}

void gameStateHandlerDeath(void)
{
	incAnimFrame(player.setIndex, &player.frame);
		
	if (player.frame == 0) // wrapped around?
		gameState = GAME_RESTART;
	else
		setSpriteFrame(player.ccb, player.setIndex, player.frame);

	WaitVBL(timerIOReq, 20);
}

void gameStateHandlerRestart(void)
{
	WaitVBL(timerIOReq, 120);
	
	if (nLives == 0)
	{
		// game over			
		gameState = GAME_OVER;
		
		// show message
		SetTextCelColor(overlaytcel, 0, RGB15_RED);
		SetTextCelCoords(overlaytcel, 133 << FRACBITS_16, 127 << FRACBITS_16);
		UpdateTextInCel(overlaytcel, TRUE, "GAME OVER");
		SHOW_SPRITE(overlaytcel->tc_CCB);				
	}
	else 
		resetMatch(FALSE);
}

void gameStateHandlerPlay(void)
{
	int32 currentTime = GetMSecTime(timerIOReq);
	int32 signals = GetCurrentSignals();	

	// player controls
	if (buttons & ControlUp)
	{
		player.desiredVec.x = 0;
		player.desiredVec.y = -1;
	}
	else if (buttons & ControlDown)
	{
		player.desiredVec.x = 0;
		player.desiredVec.y = 1;
	}
	else if (buttons & ControlLeft)
	{
		player.desiredVec.x = -1;
		player.desiredVec.y = 0;
	}
	else if (buttons & ControlRight)
	{
		player.desiredVec.x = 1;
		player.desiredVec.y = 0;
	}

	// play a little ghost siren every now and then
	
	if (isSimpleTimerReady(&sirenTimer) == TRUE)
	{
		playSound(sounds[SIREN_SD], 40);
		restartSimpleTimer(&sirenTimer);
	}
	
	// check if it's time for player to move
	if (signals & moveTimerSignal)
	{
		updatePlayer();
		WaitSignal(moveTimerSignal);
	}

	// check various game collisions
	checkCollisions();

	// check if it's time for ghosts to move
	if (signals & ghostTimerSignal)
	{
		updateGhosts();
		WaitSignal(ghostTimerSignal);
	}
				
	// update player animation
	if (currentTime - player.lastAnimTime >= player.animDelay)
	{
		if (player.frame + 1 > 2) player.frame = 0;
		else player.frame++;
		setSpriteFrame(player.ccb, player.setIndex, player.frame);
		player.lastAnimTime = currentTime;
	}		
		
	// check for extra life award
	
	if (extraLifeAwarded == FALSE)
	{
		if (score >= EXTRA_LIFE_SCORE)
		{
			extraLifeAwarded = TRUE;
			playSound(sounds[ONE_UP], 50);
			nLives++;				
			SHOW_SPRITE(lives[nLives-1]);
		}
	}

	// go to next level if player has eaten all pellets
	if (pelletsEaten == PELLETS_PER_MATCH)
		nextLevel();
}

void gameStateHandlerOver(void) 
{
	if (!(prevButtons & ControlStart) && buttons & ControlStart)
		resetGame();
}

void initGame(void)
{
	int32 i;

	printLine("init game");

	gameBackground = (ubyte*) LoadImage(GET_GFX_ASSET_PATH("bg.imag"), NULL, NULL, sc);
	
	// init player
	player.setIndex = loadSpriteset(GET_GFX_ASSET_PATH("pac.anim"));
	player.ccb = cloneSpriteCCB(player.setIndex);
	player.lastAnimTime = 0;
	player.animDelay = 80;
	if (player.ccb->ccb_Flags & CCB_LAST)
		player.ccb->ccb_Flags &= ~CCB_LAST;
	
	initGhosts();
	
	// next fruit - this is static and shown bottom-right of screen
	fruitSpritesetIndex = loadSpriteset(GET_GFX_ASSET_PATH("fruits.anim"));
	fruitHUD = cloneSpriteCCB(fruitSpritesetIndex);
	fruitHUD->ccb_XPos = 233 << FRACBITS_16;
	fruitHUD->ccb_YPos = 214 << FRACBITS_16;
	fruitHUD->ccb_Flags &= ~CCB_LAST;

	// in-game fruit - this is picked up on the game board
	fruit = cloneSpriteCCB(fruitSpritesetIndex); // CloneCel(fruitHUD, CLONECEL_CCB_ONLY);
	fruit->ccb_XPos = 155 << FRACBITS_16;
	fruit->ccb_YPos = 126 << FRACBITS_16;
	fruit->ccb_Flags &= ~CCB_LAST;
	HIDE_SPRITE(fruit);
	
	initPowerups();
	
	moveTimerSignal = AllocSignal(0);
	ghostTimerSignal = AllocSignal(0);
	
	// starts the TimerServices thread
	if (TimerServicesStartup(0) == 1)
		TimerServicesOpen(); // already running
	
	moveTimerHandle = TimerSignalHeartbeat(moveTimerSignal, 0, PLAYER_MOVE_DELAY);
	ghostTimerHandle = TimerSignalHeartbeat(ghostTimerSignal, 0, GHOST_MOVE_DELAY);
	
	initPathfinder();
		
	loadAudio();
	
	for (i = 0; i < MAX_LIVES; i++)
	{
		lives[i] = cloneSpriteCCB(player.setIndex);
		setSpriteFrame(lives[i], player.setIndex, 0);
		setSpritePosition(lives[i], 76 + i * 14, 214);	
		if (lives[i]->ccb_Flags & CCB_LAST)
			lives[i]->ccb_Flags &= ~CCB_LAST;
		
		if (i > 0)
			lives[i-1]->ccb_NextPtr = lives[i];
	}

	initSimpleTimer(&getReadyTimer, 4480);
	initSimpleTimer(&sirenTimer, 8000);
	initSimpleTimer(&powerupTimer, 5000);
	initSimpleTimer(&chompTimer, 220);
	initSimpleTimer(&fpsTimer, 1000);
	
	chompInstIndex = -1;
	chomping = FALSE;

	// set game state handlers
	gameStateHandlers[(uint32)GAME_GET_READY] = &gameStateHandlerReady;
	gameStateHandlers[(uint32)GAME_GET_SET] = &gameStateHandlerSet;
	gameStateHandlers[(uint32)GAME_PLAY] = &gameStateHandlerPlay;
	gameStateHandlers[(uint32)GAME_DEATH] = &gameStateHandlerDeath;
	gameStateHandlers[(uint32)GAME_RESTART] = &gameStateHandlerRestart;
	gameStateHandlers[(uint32)GAME_OVER] = &gameStateHandlerOver;

	gfont = LoadFont("Fonts/zx.3do", MEMTYPE_ANY);
	
	// set up text cels

	scoretcel = CreateTextCel(gfont, TC_FORMAT_LEFT_JUSTIFY, 0, 0);
    SetTextCelColor(scoretcel, 0, RGB15_WHITE);
    SetTextCelCoords(scoretcel, 80 << FRACBITS_16, 14 << FRACBITS_16);
	scoretcel->tc_CCB->ccb_NextPtr = NULL;
	if (scoretcel->tc_CCB->ccb_Flags & CCB_LAST)
		scoretcel->tc_CCB->ccb_Flags &= ~CCB_LAST;
		
	overlaytcel = CreateTextCel(gfont, TC_FORMAT_LEFT_JUSTIFY, 0, 0);
    overlaytcel->tc_CCB->ccb_NextPtr = NULL;
	if (overlaytcel->tc_CCB->ccb_Flags & CCB_LAST)
		overlaytcel->tc_CCB->ccb_Flags &= ~CCB_LAST;

	linkCCBs();

	fps = fpsCount = 0;

	// check for easter egg
	// hold X and B together during startup to make pacman invisible!
	GetControlPad(1, FALSE, &cped);
    buttons = cped.cped_ButtonBits;
	if (buttons & ControlX && buttons & ControlB)
	{
		printLine("enable invisible pacman!");
		HIDE_SPRITE(player.ccb);
	}
	buttons = 0;
}

/********************************************************************/
/* ---------------- EXTERN FUNCTION IMPLEMENTATION ---------------- */
/********************************************************************/

Boolean checkMove(Coord x, Coord y, Vector *vec)
{
	Point points[8];
	Point end;
	int32 i;
	Boolean result = FALSE;
	
	end.pt_X = x;
	end.pt_Y = y;
	
	// first check tunnel
	if (y == 78 && vec->y == 0 && (x < 0 || x + SPRITE_SIZE >= MAP_WIDTH * TILE_SIZE))
	{
		// entity is warping in tunnel, allow it to keep moving
		result = TRUE;
	}
	else 
	{
		// entity is on the game board
		
		// define map points to check
		points[0].pt_X = end.pt_X;						// top left
		points[0].pt_Y = end.pt_Y;
		points[1].pt_X = end.pt_X + HALF_SPRITE_SIZE;	// top mid
		points[1].pt_Y = end.pt_Y;
		points[2].pt_X = end.pt_X + SPRITE_SIZE - 1;	// top right
		points[2].pt_Y = end.pt_Y;
		points[3].pt_X = end.pt_X + SPRITE_SIZE - 1;	// mid right
		points[3].pt_Y = end.pt_Y + HALF_SPRITE_SIZE;
		points[4].pt_X = end.pt_X + SPRITE_SIZE - 1;	// bottom right
		points[4].pt_Y = end.pt_Y + SPRITE_SIZE - 1;
		points[5].pt_X = end.pt_X + HALF_SPRITE_SIZE;	// bottom mid
		points[5].pt_Y = end.pt_Y + SPRITE_SIZE - 1;
		points[6].pt_X = end.pt_X;						// bottom left
		points[6].pt_Y = end.pt_Y + SPRITE_SIZE - 1;
		points[7].pt_X = end.pt_X;						// mid left
		points[7].pt_Y = end.pt_Y + HALF_SPRITE_SIZE;
	
		i = 0;

		do
		{
			// horizontal and vertical map bounds check
			if ((points[i].pt_X >= 0 && points[i].pt_X < (MAP_WIDTH * TILE_SIZE)) && (points[i].pt_Y >= 0 && points[i].pt_Y < (MAP_HEIGHT * TILE_SIZE)))
			{
				// check map
				if (!map[(points[i].pt_Y / TILE_SIZE) * MAP_WIDTH + (points[i].pt_X / TILE_SIZE)])
				{
					// solid tile
					break;
				}
			}
			else
			{
				// out of bounds, trying to move outside of board
				break;
			}
		} while(++i < 8);
		
		if (i >= 8)
			result = TRUE; // no solids hit
	}
	
	return(result);
}

#if 0
// this version works but makes player movement choppy because you can only call when 
// player is tile-aligned
Boolean checkMove(Coord x, Coord y, Vector *vec)
{
	int32 mapx, mapy;
	int32 mapx2, mapy2;
	Boolean result = FALSE; // assume for now

	// first check tunnel
	if (y == 78 && vec->y == 0 && (x < 0 || x + SPRITE_SIZE >= MAP_WIDTH * TILE_SIZE))
	{
		// entity is warping in tunnel, allow it to keep moving
		result = TRUE;
	}
	else 
	{
		// entity is on the game board

		if (vec->x < 0 && vec->y == 0) 
		{
			// heading left
			mapx = (x > -1) ? (x / TILE_SIZE) : -1; // careful with integer negative division
			mapy = y / TILE_SIZE;	
			mapx2 = mapx;
			mapy2 = mapy + 1;
		}
		else if (vec->x > 0 && vec->y == 0)
		{
			// heading right
			mapx = (x + SPRITE_SIZE-1) / TILE_SIZE;
			mapy = y / TILE_SIZE;
			mapx2 = mapx;
			mapy2 = mapy + 1;
		}
		else if (vec->x == 0 && vec->y < 0)
		{
			// heading up
			mapx = x / TILE_SIZE;
			mapy = (y > -1) ? (y / TILE_SIZE) : -1; // careful with integer negative division
			mapx2 = mapx + 1;
			mapy2 = mapy;
		}
		else if (vec->x == 0 && vec->y > 0)
		{
			// heading down
			mapx = x / TILE_SIZE;
			mapy = (y + SPRITE_SIZE-1) / TILE_SIZE;
			mapx2 = mapx + 1;
			mapy2 = mapy;
		}
		else 
		{
			return(FALSE); // break early, no vector. This can happen with the player.
		}

		// now check 

		if (mapx >= 0 && mapx < MAP_WIDTH && mapx2 >= 0 && mapx2 < MAP_WIDTH)
		{
			if (mapy >= 0 && mapy < MAP_HEIGHT && mapy2 >= 0 && mapy2 < MAP_HEIGHT)
			{
				if (map[mapy * MAP_WIDTH + mapx] && map[mapy2 * MAP_WIDTH + mapx2])
					result = TRUE; // OK to move
			}
		}
	}

	return(result);
}
#endif

void gameHandler(void)
{
	static Boolean firstRun = TRUE;
	static GrafCon gc;
	Rect r;

	if (firstRun)
	{
		printLine("Performing first game setup");
		initGame();
		resetGame();
		firstRun = FALSE;
		vramDump();
	}

	// read controller
	prevButtons = buttons;
	GetControlPad(1, FALSE, &cped);
    buttons = cped.cped_ButtonBits;
	
	// audio maintenance
	audioUpdate();
	
	// run game state specific handler
	gameStateHandlers[(uint32)gameState]();

	updateScore();

	// draw code below this point
	
	CopyVRAMPages(sport, sc->sc_Bitmaps[sc->sc_CurrentScreen]->bm_Buffer, gameBackground, sc->sc_NumBitmapPages, 0xFFFFFFFF);
	
	// draw cel list	
	DrawCels(sc->sc_BitmapItems[sc->sc_CurrentScreen], ghosts[0].base);
	
	// cover tunnel exits for better sprite effects
	
	SetFGPen(&gc, black);
	
	// left tunnel
	r.rect_XLeft = 62;
	r.rect_YTop = 107;
	r.rect_XRight = r.rect_XLeft + SPRITE_SIZE;
	r.rect_YBottom = r.rect_YTop + SPRITE_SIZE;
	FillRect(sc->sc_BitmapItems[sc->sc_CurrentScreen], &gc, &r);
	
	// right tunnel
	r.rect_XLeft = 245;
	r.rect_YTop = 107;
	r.rect_XRight = r.rect_XLeft + SPRITE_SIZE;
	r.rect_YBottom = r.rect_YTop + SPRITE_SIZE;
	FillRect(sc->sc_BitmapItems[sc->sc_CurrentScreen], &gc, &r);
		
	DisplayScreen(sc->sc_ScreenItems[sc->sc_CurrentScreen], 0);
	sc->sc_CurrentScreen = 1 - sc->sc_CurrentScreen;

	// fps

	if (isSimpleTimerReady(&fpsTimer) == TRUE) 
	{
		fps = fpsCount;
		fpsCount = 0;
		restartSimpleTimer(&fpsTimer);
	}
	else 
	{
		++fpsCount;
	}

	#if PRINT_FPS
	printLine("fps %u", fps);
	#endif 
}

void vramDump(void)
{
	#if DEBUG_VERBOSE
    MemInfo memInfo;
    AvailMem(&memInfo, MEMTYPE_VRAM);
    printLine("VRAM Dump %u", memInfo.minfo_SysFree);
	#endif
}

void warp(Coord *x, int32 xvector)
{
	if (xvector > 0)
	{ 
		// heading right
		if (*x >= TILE_SIZE * MAP_WIDTH)
			*x = -12;
	}
	else if (xvector < 0)
	{
		// heading left
		if (*x + SPRITE_SIZE <= 0)
			*x = TILE_SIZE * MAP_WIDTH;
	}		
}

Boolean isPointOnGameBoard(Coord x, Coord y)
{
	if (x >= 0 && x < TILE_SIZE * MAP_WIDTH && y >= 0 && y < TILE_SIZE * MAP_HEIGHT)
		return(TRUE);

	return(FALSE);
}
/*	Excommunicado
	12-May-2021 / refactored in 2023
	Pacman clone for 3DO.
	NTSC and PAL are supported.

	### Soapbox	 
	This work is not affiliated with Namco and will not be released by the creator using namco or like assets.
	The creator is not responsible if others mod this program to resemble pacman.
	This work was created for enjoyment purposes, not to sell nor infringe on copyrights/trademarks.

	### Tech details
	- Calls to printLine() won't try to print anything unless DEBUG_VERBOSE is set to 1.
	- DEBUG_VERBOSE should be set to 0 for production build.
	- Audio files are mono 44100Hz 32-bit float Aiff files. 
	- Easter egg: Hold X and B buttons together during startup to make pacman invisible.
	- The intermission is a separate 3DO program I wrote to test 3DO's task launcher capabilities.
	The game task will execute the intermission as a separate task, then return after it is done.
	If intermission code is updated, rename its LaunchMe to Intermission and place it in the game's CD folder.
	The intermission program references the same asset folder as the game program.

	############################## SCORE SYSTEM ###############################

	10 points for each pellet eaten.
	
	50 points for each powerup eaten.
	
	There are 8 fruit types presented in order, below are the points for each fruit eaten:
	-- 100
	-- 300
	-- 500
	-- 700
	-- 1000
	-- 2000
	-- 3000
	-- 5000

	Fruits only spawn twice per level once you eat enough pellets.

	Eating ghosts during a powerup phase:
	-- Points per ghost eaten = (number of ghosts eaten in succession) x 200

	You get 1 and only 1 extra life at 10K points.

	###########################################################################
*/

#include "pacman.h"

ScreenContext *sc = NULL;
Item sport = -1;
Item timerIOReq = -1;
Item vbl = -1;
uint32 buttons = 0;
uint32 prevButtons = 0;
uint32 black;
uint32 white;
ControlPadEventData cped;

static bool init(void)
{
	uint32 displayType;

	printLine("System initialization");

	if (OpenGraphicsFolio() < 0)
	{
		printLine("Failed to open graphics folio");
		return(FALSE);
	}
	
	if (OpenMathFolio() < 0)
	{
		printLine("Failed to open math folio");
		return(FALSE);
	}
	
	if (OpenAudioFolio() < 0)
	{
		printLine("Failed to open audio folio");
		return(FALSE);
	}
	
	sc = AllocMem(sizeof(ScreenContext), MEMTYPE_ANY);
	
	// support ntsc and pal 

	QueryGraphics(QUERYGRAF_TAG_DEFAULTDISPLAYTYPE, &displayType);
	if ((displayType == DI_TYPE_PAL1) || (displayType == DI_TYPE_PAL2))
	{
		displayType = DI_TYPE_PAL1;
		printLine("PAL system was detected");
	}
	else
	{
		displayType = DI_TYPE_NTSC;
		printLine("NTSC system was detected");
	}

	if (CreateBasicDisplay(sc, displayType, NUM_SCREENS) < 0)
	{
		printLine("Failed to create display");
		return(FALSE);
	}
	
	sc->sc_CurrentScreen = 0;
	
	sport = GetVRAMIOReq();
	if(sport < 0)
	{
		printLine("Failed to obtain sport io req");
		return(FALSE);
	}
	
	timerIOReq = GetTimerIOReq();
	if(timerIOReq < 0)
	{
		printLine("Failed to obtain timer io req");
		return(FALSE);
	}
	
	vbl = GetVBLIOReq();
	if(vbl < 0)
	{
		printLine("Failed to obtain vbl io req");
		return(FALSE);
	}
	
	if (InitEventUtility(1, 0, LC_ISFOCUSED) < 0)
	{
		printLine("Failed to init event broker");
		return(FALSE);
	}
	
	srand(ReadHardwareRandomNumber());
	
	black = 0;
	white = MakeRGB15Pair(31, 31, 31);
	
	audioInit();
	
	return(TRUE);
}

static void cleanup(void)
{
	// if execution gets here something went very wrong 

	printLine("Cleanup start");

	if (sc != NULL)
	{
		DeleteBasicDisplay(sc);
		sc = NULL;
	}
	
	if (sport > -1)
		DeleteIOReq(sport);
	
	if (timerIOReq > -1)
		DeleteIOReq(timerIOReq);

	if (vbl > -1)
		DeleteIOReq(vbl);
	
	audioCleanup();
	
	KillEventUtility();
	CloseMathFolio();
	CloseAudioFolio();
	CloseGraphicsFolio();

	printLine("Cleanup done, goodbye.");
}

/*
static void title(void)
{
	ubyte* bg = (ubyte*) LoadImage(GET_GFX_ASSET_PATH("intro.imag"), NULL, NULL, sc);
	
	CopyVRAMPages(sport, sc->sc_Bitmaps[0]->bm_Buffer, bg, sc->sc_NumBitmapPages, 0xFFFFFFFF);
	DisplayScreen(sc->sc_ScreenItems[0], 0);
	
	do
	{
		GetControlPad(1, FALSE, &cped);
    	buttons = cped.cped_ButtonBits;
		WaitVBL(vbl, 1);		
	} while( !(buttons & ControlStart) );
	
	UnloadImage(bg);
}
*/

int main(int argc, char* argv[])
{
	if (init() == FALSE)
		goto END_MAIN;

	// skip title screen and jump right into game
	// title();
	
	// main game loop
	for(;;)
		gameHandler();
	
END_MAIN:
	cleanup();
}

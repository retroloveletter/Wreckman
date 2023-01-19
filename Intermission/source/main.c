/*	This little program just has the ghosts chasing pacman across the screen and is called by the main program.
	They start from left to right, then right to left. 
	Pacman is the one doing the chasing once they double-back. 
	Since this app shares assets from the main app, I had to deal with ghost bodies and eyes being separate.
	This made logic here a bit more complex, but simplifies asset management.
	In the main app, the eyes are separate to support game logic and visual effects at times.
*/

#include "displayutils.h"
#include "celutils.h"
#include "animutils.h"
#include "audio.h"
#include "timerutils.h"
#include "stdio.h"

#define NUM_SCREENS 2
// 4 ghosts, 4 eyes, and pacman
#define MAX_CCBS 9
// 4 ghosts, 4 eyes, and pacman
#define MAX_ANIMS 9
#define DEBUG_MODE 0

static ScreenContext *sc;
static Item sport;
static Item mixer;
static Item timerIO;
static Item vbl;
static Item jig;
static Item sound;
static Item attachment;
static Item ampKnob;
static Item cue;
static int32 sig;
static int32 vectorX;
static int32 lastAnimationTime;
static int32 ccbIndex;
static int32 animIndex;
static CCB *ccbList[MAX_CCBS];
static ANIM *animList[MAX_ANIMS];


static CCB *unifyAnim(ANIM *anim)
{
	int32 i;
	CCB *ccb = anim->pentries[0].af_CCB;
	
	for (i = 0; i < anim->num_Frames; ++i)
		anim->pentries[i].af_CCB = ccb;
	
	return(ccb);
}

static void loadGhost(int32 number, int32 xpos, int32 ypos)
{
	ANIM *anim;
	CCB *ccb;
	char sbuffer[30];

	sprintf(sbuffer, "Assets/GFX/ghost%d.anim", number);
	anim = LoadAnim(sbuffer, MEMTYPE_CEL);
	ccb = unifyAnim(anim);	
	anim->cur_Frame = 0;
	GetAnimCel(anim, 0);
	ccb->ccb_XPos = xpos << 16;
	ccb->ccb_YPos = ypos << 16;

	ccbList[ccbIndex++] = ccb;
	animList[animIndex++] = anim;

	// add eyes 
	anim = LoadAnim("Assets/GFX/eyes.anim", MEMTYPE_CEL);
	ccb = unifyAnim(anim);
	anim->cur_Frame = 1 << 16; // looking right
	GetAnimCel(anim, 0);
	ccb->ccb_XPos = xpos << 16; // on top of ghost
	ccb->ccb_YPos = ypos << 16;
		
	ccbList[ccbIndex++] = ccb;
	animList[animIndex++] = anim;
}

static void loadPacman(int32 xpos, int32 ypos)
{
	ANIM *anim = LoadAnim("Assets/GFX/pac.anim", MEMTYPE_CEL);
	CCB *ccb = unifyAnim(anim);
	anim->cur_Frame = 0;
	GetAnimCel(anim, 0);
	ccb->ccb_XPos = xpos << 16;
	ccb->ccb_YPos = ypos << 16;

	ccbList[ccbIndex++] = ccb;
	animList[animIndex++] = anim;
}

static void linkCels(void)
{
	int32 i;

	for (i = 0; i < ccbIndex; ++i) 
	{
		ccbList[i]->ccb_Flags &= ~CCB_LAST;

		if (i > 0) 
			ccbList[i-1]->ccb_NextPtr = ccbList[i];
	}

	ccbList[ccbIndex-1]->ccb_NextPtr = NULL;
	ccbList[ccbIndex-1]->ccb_Flags |= CCB_LAST;
}

static void vramDump(void)
{
	#if DEBUG_MODE
    MemInfo memInfo;
    AvailMem(&memInfo, MEMTYPE_VRAM);
    printf("VRAM Dump %u\n", memInfo.minfo_SysFree);
	#endif
}

static void playLittleJig(void)
{
	int32 ampValue;
	TagArg tags[2];
	
	// avoid 3do audio pop sound by fading out/in over a few ms

	do // fade out
	{			
		GetAudioItemInfo(ampKnob, tags);
		ampValue = (int32)tags[0].ta_Arg;			
		ampValue -= 10000;
		if (ampValue < 0) ampValue = 0;
		TweakKnob(ampKnob, ampValue);
	} while(ampValue > 0);

	StartInstrument(jig, NULL);

	// fade in

	while (ampValue != 32767)
	{	
		ampValue += 10000;			
		if (ampValue > 32767) ampValue = 32767;
		TweakKnob(ampKnob, ampValue);
	}
}

static void init(void)
{
	int32 i;

	vramDump();

	OpenGraphicsFolio();
	OpenAudioFolio();
	
	sc = AllocMem(sizeof(ScreenContext), MEMTYPE_ANY);
	
	CreateBasicDisplay(sc, DI_TYPE_DEFAULT, NUM_SCREENS);
	sc->sc_CurrentScreen = 0;
	
	sport = GetVRAMIOReq();	
	vbl = GetVBLIOReq();
	timerIO = GetTimerIOReq();

	vectorX = 1; // start heading to the right
	
	// load graphics

	ccbIndex = 0;
	animIndex = 0;

	// ghosts
	for (i = 0; i < 4; ++i)
		loadGhost(i+1, 16 * i - 70, 100); // index, x, and y

	loadPacman((ccbList[6]->ccb_XPos >> 16) + 26, 100); // space a bit further from last ghost
	linkCels();	

	// audio
	sound = LoadSample("Assets/SFX/interm.aiff");	
	mixer = LoadInstrument("directout.dsp", 0, 100);
	jig = LoadInstrument("fixedmonosample.dsp", 0, 100);
	ConnectInstruments(jig, "Output", mixer, "InputLeft");
	ConnectInstruments(jig, "Output", mixer, "InputRight");
	attachment = AttachSample(jig, sound, NULL);
	cue = CreateCue(NULL);
	sig = GetCueSignal(cue);
	MonitorAttachment(attachment, cue, CUE_AT_END);	
	StartInstrument(mixer, NULL);
	ampKnob = GrabKnob(jig, "amplitude");
	playLittleJig();
		
	lastAnimationTime = GetMSecTime(timerIO);
}

static void cleanup(void)
{	
	int32 i;
	
	for (i = 0; i < animIndex; ++i)
		UnloadAnim(animList[i]);
	
	StopInstrument(jig, NULL);
	StopInstrument(mixer, NULL);
	DetachSample(attachment);
	UnloadSample(sound);
	DeleteCue(cue);
	DisconnectInstruments(jig, "Output", mixer, "InputLeft");
	DisconnectInstruments(jig, "Output", mixer, "InputRight");
	UnloadInstrument(jig);
	UnloadInstrument(mixer);
	
	DeleteIOReq(sport);
	DeleteIOReq(vbl);
	DeleteIOReq(timerIO);
	
	DeleteBasicDisplay(sc);
	
	CloseAudioFolio();
	CloseGraphicsFolio();

	vramDump();
}

static void animates(ANIM *anim, int32 wrapFrame) 
{
	if (!wrapFrame) 
		GetAnimCel(anim, 1 << 16); // just cycle it, easy
	else 
	{
		int32 frame = anim->cur_Frame >> 16;

		if (frame >= wrapFrame) 
		{
			// wrap early, some frames may not be movement frames
			anim->cur_Frame = 0;
			GetAnimCel(anim, 0); // load it
		}
		else 
			GetAnimCel(anim, 1 << 16); // not there yet
	}
}

int main(int argc, char* argv)
{
	int32 signals;
	int32 currentTime;
	int32 i;
	
	init();
	
	for(;;)
	{
		signals = GetCurrentSignals();
		currentTime = GetMSecTime(timerIO);
		
		// restart jig once it is over
		// at this point all sprites reached the edge of the screen
		if (signals & sig)
		{
			WaitSignal(sig);

			// do we exit now?
			if (vectorX < 0)
				break; // yes, we already double-backed

			playLittleJig(); // restart song

			vectorX = -1; // go the other way

			// flip sprites 
			for (i = 0; i < ccbIndex; ++i) 
				ccbList[i]->ccb_HDX = -1 << 20;
		}
		
		// update ccb positions and animation
		for (i = 0; i < ccbIndex; ++i)
			ccbList[i]->ccb_XPos += 98304 * vectorX;			

		if (currentTime - lastAnimationTime >= 100)
		{
			// animate sprites

			// ghosts first, skip eyes
			animates(animList[0], 0);
			animates(animList[2], 0);
			animates(animList[4], 0);
			animates(animList[6], 0);
			animates(animList[8], 2); // pacman						

			lastAnimationTime = currentTime;
		}

		// show it all
		
		SetVRAMPages(sport, sc->sc_Bitmaps[sc->sc_CurrentScreen]->bm_Buffer, 0, sc->sc_NumBitmapPages, 0xFFFFFFFF);
		DrawCels(sc->sc_BitmapItems[sc->sc_CurrentScreen], ccbList[0]);			
		DisplayScreen(sc->sc_ScreenItems[sc->sc_CurrentScreen], 0);
		sc->sc_CurrentScreen = 1 - sc->sc_CurrentScreen;
		
		WaitVBL(vbl, 1);
	}
	
	cleanup();

	return(0);
}


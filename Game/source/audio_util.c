#include "pacman.h"

#define MAX_CHANNELS 4
#define FLAG_REPEAT 0x00000001

typedef struct
{
	int32 priority;
	bool busy;
	Item instrument;
	Item attachment;
	Item cue;
	Item sound;
	int32 signal;
	Item ampKnob;
} SoundSlot;


typedef struct
{
	Item instrument;
	Item gainsL[MAX_CHANNELS];
	Item gainsR[MAX_CHANNELS];
} Mixer;


static SoundSlot soundSlots[MAX_CHANNELS];
static Mixer mixer;

void audioInit(void)
{
	int32 i;
	char buffer[50];
	
	// create mixer and turn it on
	
	mixer.instrument = LoadInstrument("mixer4x2.dsp", 0, 100);
	
	// you won't hear anything unless you adjust gains
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		sprintf(buffer, "LeftGain%d", i);
		mixer.gainsL[i] = GrabKnob(mixer.instrument, buffer);
		sprintf(buffer, "RightGain%d", i);
		mixer.gainsR[i] = GrabKnob(mixer.instrument, buffer);
		
		TweakKnob(mixer.gainsL[i], 0x6000);
		TweakKnob(mixer.gainsR[i], 0x6000);
	}
	
	StartInstrument(mixer.instrument, NULL);
	
	// set up channel instruments
	
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		soundSlots[i].busy = FALSE;
		soundSlots[i].instrument = LoadInstrument("fixedmonosample.dsp", 0, 100);
		soundSlots[i].attachment = -1;
		soundSlots[i].cue = CreateCue(NULL);
		soundSlots[i].signal = GetCueSignal(soundSlots[i].cue);
		soundSlots[i].priority = 0;
		soundSlots[i].ampKnob = GrabKnob(soundSlots[i].instrument, "amplitude");		
		sprintf(buffer, "Input%d", i);
		ConnectInstruments(soundSlots[i].instrument, "Output", mixer.instrument, buffer);
	}
}

static void stopSound(int32 instrumentIndex)
{
	if (soundSlots[instrumentIndex].busy == TRUE)
	{
		// get current instrument amplitude
		TagArg tags[2];
		int32 ampValue;

		tags[0].ta_Tag = AF_TAG_CURRENT;
		tags[1].ta_Tag = TAG_END;
		tags[1].ta_Arg = 0;
		
		do // fade out amplitude to avoid pop
		{			
			GetAudioItemInfo(soundSlots[instrumentIndex].ampKnob, tags);
			ampValue = (int32)tags[0].ta_Arg;			
			ampValue -= 10000;
			if (ampValue < 0) ampValue = 0;
			TweakKnob(soundSlots[instrumentIndex].ampKnob, ampValue);
		} while(ampValue > 0);

		StopInstrument(soundSlots[instrumentIndex].instrument, NULL);
		DetachSample(soundSlots[instrumentIndex].attachment);
		soundSlots[instrumentIndex].attachment = -1;				
		soundSlots[instrumentIndex].busy = FALSE;

		// reset amp for next sound
		TweakKnob(soundSlots[instrumentIndex].ampKnob, 32767);
	}
}

void playSound(Item sound, int32 priority)
{
	// search for available slot
	
	int32 i;
	SoundSlot *slotPtr = NULL;
	
	for (i = 0 ; i < MAX_CHANNELS; i++)
	{
		if (soundSlots[i].busy == FALSE)
		{
			// found empty slot
			slotPtr = &soundSlots[i];
			break;
		}
	}
	
	if (slotPtr == NULL)
	{
		// all slots are busy, check priorities
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			if (priority > soundSlots[i].priority)
			{
				// hijack slot
				stopSound(i);
				slotPtr = &soundSlots[i];
			}
		}
	}
	
	if (slotPtr != NULL)
	{
		// play it

		TagArg tags[2];
		int32 ampValue = 0;

		slotPtr->priority = priority;
		slotPtr->busy = TRUE;
		slotPtr->sound = sound;
		slotPtr->attachment = AttachSample(slotPtr->instrument, sound, NULL);
		
		tags[0].ta_Tag = AF_TAG_SET_FLAGS;
		tags[0].ta_Arg = (void*)AF_ATTF_FATLADYSINGS;
		tags[1].ta_Tag = TAG_END;
		SetAudioItemInfo(slotPtr->attachment, tags);
		
		// fade in amplitude to avoid pop

		TweakKnob(slotPtr->ampKnob, ampValue);
		StartInstrument(slotPtr->instrument, NULL);

		while (ampValue != 32767)
		{	
			ampValue += 10000;			
			if (ampValue > 32767) ampValue = 32767;
			TweakKnob(slotPtr->ampKnob, ampValue);
		}
	}
}

void audioUpdate(void)
{
	int32 i;
	
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (soundSlots[i].busy == TRUE)
		{
			TagArg tags[2];
			tags[0].ta_Tag = AF_TAG_STATUS;
			tags[1].ta_Tag = TAG_END;
			GetAudioItemInfo(soundSlots[i].instrument, tags);
			
			if ((int32)(tags[0].ta_Arg) == AF_STOPPED)
				stopSound(i);			
		}
	}
}

void audioCleanup(void)
{
	int32 i;
	char buffer[50];
	
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (soundSlots[i].busy == TRUE)
			stopSound(i);			
		
		sprintf(buffer, "Input%d", i);
		DisconnectInstruments(soundSlots[i].instrument, "Output", mixer.instrument, buffer);
		UnloadInstrument(soundSlots[i].instrument);
	}
	
	StopInstrument(mixer.instrument, NULL);
	UnloadInstrument(mixer.instrument);
}

int32 getSoundFrameCount(Item sound)
{
	TagArg tags[2];
	tags[0].ta_Tag = AF_TAG_FRAMES;
	tags[1].ta_Tag = TAG_END;
	GetAudioItemInfo(sound, tags);
	return((int32)tags[0].ta_Arg);
}
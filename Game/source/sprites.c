#include "pacman.h"

static ANIM* spritesets[MAX_ANIMS];
static int32 ssCounter = 0;

extern void rotSprite(CCB *ccb, int32 angle)
{
	switch(angle)
	{
	case ANGLE_0:
		// right/normal
		ccb->ccb_HDX = 1 << FRACBITS_20;
		ccb->ccb_HDY = 0;
		ccb->ccb_VDX = 0;
		ccb->ccb_VDY = 1 << FRACBITS_16;		
		break;
		
	case ANGLE_90:
		// down
		ccb->ccb_HDX = 0;
		ccb->ccb_HDY = 1 << FRACBITS_20;
		ccb->ccb_VDX = 1 << FRACBITS_16;
		ccb->ccb_VDY = 0;
		break;
		
	case ANGLE_180:
		// left
		ccb->ccb_HDX = -1 << FRACBITS_20;
		ccb->ccb_HDY = 0;
		ccb->ccb_VDX = 0;
		ccb->ccb_VDY = 1 << FRACBITS_16;
		break;
		
	case ANGLE_270:
		// up
		ccb->ccb_HDX = 0;
		ccb->ccb_HDY = -1 << FRACBITS_20;
		ccb->ccb_VDX = 1 << FRACBITS_16;
		ccb->ccb_VDY = 0;
		break;
		
	default:
		break;
	}
}

int32 loadSpriteset(char *path)
{
	if (ssCounter + 1 >= MAX_ANIMS) return(-1);
	spritesets[ssCounter] = LoadAnim(path, MEMTYPE_ANY);
	return(ssCounter++);
}

void freeSpritesets(void)
{
	int32 i;
	
	for (i = 0; i < ssCounter; i++)
		UnloadAnim(spritesets[i]);
	
	ssCounter = 0;
}

void setSpriteFrame(CCB *ccb, int32 setIndex, int32 frame)
{
	ANIM *set = spritesets[setIndex];
	if (frame >= set->num_Frames) frame = 0;
	set->cur_Frame = frame << FRACBITS_16;
	GetAnimCel(set, 0);
	ccb->ccb_SourcePtr = set->pentries[frame].af_CCB->ccb_SourcePtr;
}

CCB *cloneSpriteCCB(int32 animIndex)
{
	ANIM *anim;
	CCB *ccb;
	
	if (animIndex >= ssCounter)
		return(NULL);
	
	anim = spritesets[animIndex];
	ccb = CloneCel(anim->pentries[0].af_CCB, CLONECEL_CCB_ONLY);
	
	return(ccb);
}

void setSpritePosition(CCB *ccb, Coord x, Coord y)
{
	ccb->ccb_XPos = x << FRACBITS_16;
	ccb->ccb_YPos = y << FRACBITS_16;
}

void translateSprite(CCB *ccb, Coord x, Coord y)
{
	ccb->ccb_XPos += x << FRACBITS_16;
	ccb->ccb_YPos += y << FRACBITS_16;
}

void incAnimFrame(int32 setIndex, int32 *frame)
{
	ANIM *set = spritesets[setIndex];
	
	(*frame)++;

	if(*frame >= set->num_Frames) *frame = 0;
}

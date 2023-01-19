#include "pacman.h"

bool rectCollision(SRect* r1, SRect* r2)
{
	// x plane
	if (r1->pos.x + r1->size.x >= r2->pos.x && r1->pos.x < r2->pos.x + r2->size.x)
	{
		// y plane
		if (r1->pos.y + r1->size.y >= r2->pos.y && r1->pos.y < r2->pos.y + r2->size.y)
		{
			return(TRUE);
		}
	}
	
	return(FALSE);
}

void execProgram(char* name)
{
	Item handle = LoadProgram(name);
	
	do
	{
		Yield();
	} while(LookupItem(handle));
	
	DeleteItem(handle);
}

void initSimpleTimer(SimpleTimer *timer, int32 delay)
{
	timer->delay = delay;
	timer->ready = FALSE;
}

void restartSimpleTimer(SimpleTimer *timer)
{
	timer->ready = FALSE;
	timer->previousTime = GetMSecTime(timerIOReq);
}

bool isSimpleTimerReady(SimpleTimer *timer)
{
	int32 currentTime = GetMSecTime(timerIOReq);
	
	if(timer->ready == FALSE && (currentTime - timer->previousTime >= timer->delay))
		timer->ready = TRUE;
	
	return(timer->ready);
}

void printLine(char *pstring, ...)
{
	#if DEBUG_VERBOSE
	va_list args;
    va_start(args, pstring);
    vprintf(pstring, args);
    va_end(args);
	printf("\n");
	#endif 
}

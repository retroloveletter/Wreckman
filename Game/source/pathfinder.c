/* 
	A* pathfinding
	This is a little more complex than it probably needs to be, but speed really matters.. 
*/

#include "pacman.h"

#define OPEN_LIST_DEPTH 50
#define TOUCHED_LIST_DEPTH 300
#define MOVE_COST 10

#define PUSH_OPEN_LIST(node) {\
	node->open = TRUE;\
	openList[openListIndex++] = node;\
	touched[touchedIndex++] = node;\
}


static int32 openListIndex;
static int32 touchedIndex;

static int32 pathTemplate[MAP_SIZE] = 
{
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,99,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,
	-1,99,99,99,99,-1,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,-1,99,99,99,99,-1,99,
	-1,99,99,99,99,-1,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,-1,99,99,99,99,-1,99,
	-1,99,99,99,99,-1,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,-1,99,99,99,99,-1,99,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,
	-1,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,-1,99,
	-1,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,-1,99,
	-1,-1,-1,-1,-1,-1,99,99,-1,-1,-1,-1,99,99,-1,-1,-1,-1,99,99,-1,-1,-1,-1,-1,-1,99,
	99,99,99,99,99,-1,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,-1,99,99,99,-1,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,-1,99,99,99,-1,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,99,99,99,-1,99,99,99,99,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,
	99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,99,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,
	-1,99,99,99,99,-1,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,-1,99,99,99,99,-1,99,
	-1,99,99,99,99,-1,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,-1,99,99,99,99,-1,99,
	-1,-1,-1,99,99,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,99,-1,-1,-1,99,
	99,99,-1,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,-1,99,99,99,
	99,99,-1,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,-1,99,99,99,
	-1,-1,-1,-1,-1,-1,99,99,-1,-1,-1,-1,99,99,-1,-1,-1,-1,99,99,-1,-1,-1,-1,-1,-1,99,
	-1,99,99,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,99,99,-1,99,
	-1,99,99,99,99,99,99,99,99,99,99,-1,99,99,-1,99,99,99,99,99,99,99,99,99,99,-1,99,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99
};


typedef struct PNode
{
	struct PNode *parent;
	int32 mapX;
	int32 mapY;
	int32 f;
	int32 g;
	bool closed;
	bool open;
	bool illegal;
} PNode;

static PNode nodes[MAP_SIZE];
static PNode *openList[OPEN_LIST_DEPTH];
static PNode *touched[TOUCHED_LIST_DEPTH];


static bool search(int32 targetX, int32 targetY)
{
	int32 x;
	int32 y;
	PNode *parent = openList[0]; // always grab the first node
	PNode *current = NULL;
	PNode *temp = NULL;		
	
	// did we reach our destination?
		
	if (parent->mapX == targetX && parent->mapY == targetY)
		return(TRUE);
	
	// close parent node and shift
	
	parent->closed = TRUE;
	
	for (x = 0; x < OPEN_LIST_DEPTH; x++)
	{
		temp = openList[x+1];
		openList[x] = temp;
		if(temp == NULL) break;
	}
	
	openListIndex = x;
	
	// scan adjacent nodes
	
	for (y = parent->mapY - 1; y <= parent->mapY + 1; y++)
	{
		if (y < 0 || y >= MAP_HEIGHT) continue;
		
		for (x = parent->mapX - 1; x <= parent->mapX + 1; x++)
		{
			int32 g;
			int32 h;
			int32 f;
			
			if (x < 0 || x >= MAP_WIDTH) continue;
			
			current = &nodes[y * MAP_WIDTH + x];

			// skip closed nodes
			if (current->closed == TRUE) continue; 
			
			// skip walls
			if (current->illegal == TRUE) continue;
			
			// skip diagonals
			if (x < parent->mapX && y < parent->mapY) continue;
			if (x > parent->mapX && y < parent->mapY) continue;
			if (x < parent->mapX && y > parent->mapY) continue;
			if (x > parent->mapX && y > parent->mapY) continue;
			
			// calculate score
			g = MOVE_COST + parent->g;
			h = ((targetX - current->mapX) + (targetY - current->mapY)) * MOVE_COST;
			h = (h < 0) ? h * -1 : h; // keep heuristic positive
			f = g + h;
			
			// add to open list if it's not already there
			if (current->open == FALSE)
			{
				current->f = f;
				current->g = g;
				current->parent = parent;
				PUSH_OPEN_LIST(current);

				#if DEBUG_VERBOSE
				if (openListIndex >= OPEN_LIST_DEPTH)
					printLine("search() - openListIndex out of bounds %d", openListIndex);
				if (touchedIndex >= TOUCHED_LIST_DEPTH)
					printLine("search() - touchedIndex out of bounds %d", touchedIndex);
				#endif
			}
			else 
			{
				/* already in list, update score and parent if lower */
				if(f < current->f)
				{
					current->parent = parent;
					current->f = f;
					current->g = g;
				}
			}						
		}
	}


	/* 
		this does a single sort pass
		since search() is called multiple times this ends up sorting over time 
		there is room for error but this seems like a quick solution with fuzzy but good tracking results
	*/
	
	#if 1
	for(x = 0; x < OPEN_LIST_DEPTH-1; x++)
	{
		if(openList[x] == NULL || openList[x+1] == NULL) break;
		
		if(openList[x+1]->f < openList[x]->f)
		{
			temp = openList[x];
			openList[x] = openList[x+1];
			openList[x+1] = temp;
		}
	}
	#endif 
	
	
	/* 
		bubble sort method, less error but slower and awkward looking
	*/
	#if 0
	for(y = 0; y < OPEN_LIST_DEPTH; y++)
	{
		if(openList[y] == NULL) break;
		
		for(x = 0; x < OPEN_LIST_DEPTH-1; x++)
		{
			if(openList[x] == NULL || openList[x+1] == NULL) break;
			
			if(openList[x+1]->f < openList[x]->f)
			{
				temp = openList[x];
				openList[x] = openList[x+1];
				openList[x+1] = temp;
			}
		}
	}
	#endif

	return(FALSE);
}


void buildPath(Point *start, Point *target, Point *dest, int32 *index)
{
	int32 i;
	PNode *node = &nodes[start->pt_Y * MAP_WIDTH + start->pt_X];
	
	memset(openList, 0, sizeof(PNode*) * OPEN_LIST_DEPTH);
		
	openListIndex = 0;
	touchedIndex = 0;
	
	PUSH_OPEN_LIST(node); // first node

	// search
	for(;;)
	{
		if (search(target->pt_X, target->pt_Y) == TRUE)
		{
			// path found, first open list node is our destination
			
			{
				node = openList[0];
				i = 0;
				
				while (node->parent != NULL)
				{
					dest[i].pt_X = node->mapX;
					dest[i].pt_Y = node->mapY;
					i++;
					node = node->parent;
				}
				
				(*index) = i - 1;
			}
			
			break; 
		}
	}
	
	/* reset data for next build */
	
	/* too slow, switched to using a "touched" list 
	for(i = 0; i < MAP_SIZE; i++)
	{
		node = &nodes[i];
		node->parent = NULL;
		node->g = 0;
		node->f = 0;
		node->closed = FALSE;
		node->open = FALSE;
	} */
	
	
	for(i = 0; i < touchedIndex; i++)
	{
		node = touched[i];
		node->parent = NULL;
		node->g = 0;
		node->f = 0;
		node->closed = FALSE;
		node->open = FALSE;
		touched[i] = NULL;
	}
}


extern void initPathfinder(void)
{
	int32 x;
	int32 y;	
	PNode *node = NULL;
	
	for(y = 0; y < MAP_HEIGHT; y++)
	{
		for(x = 0; x < MAP_WIDTH; x++)
		{
			int32 pos = y * MAP_WIDTH + x;
			node = &nodes[pos];
			node->mapX = x;
			node->mapY = y;
			node->parent = NULL;
			node->g = 0;
			node->f = 0;
			node->illegal = (pathTemplate[pos] == 99) ? TRUE : FALSE;
			node->closed = FALSE;
			node->open = FALSE;
		}
	}
	
	openListIndex = 0;
	touchedIndex = 0;
	
	memset(touched, 0, sizeof(PNode*) * TOUCHED_LIST_DEPTH);
}

Boolean isTargetValid(int32 tx, int32 ty)
{
	if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT)
		return(FALSE);

	if (pathTemplate[ty * MAP_WIDTH + tx] == 99)
		return(FALSE);

	return(TRUE);
}
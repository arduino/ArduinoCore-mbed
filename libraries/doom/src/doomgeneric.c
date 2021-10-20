#include "doomgeneric.h"

uint32_t* DG_ScreenBuffer = 0;

void dg_Create()
{
	DG_Init();
	printf("[%s] ea_malloc %d\n", __func__, DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
#ifdef DEBUG_CM7_VIDEO
	DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY);
#endif
}
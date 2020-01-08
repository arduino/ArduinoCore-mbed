#include "doomgeneric.h"

uint32_t* DG_ScreenBuffer = 0;


void dg_Create()
{
	DG_Init();
	printf("[%s] ea_malloc %d\n", __func__, DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
	DG_ScreenBuffer = ea_malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
}
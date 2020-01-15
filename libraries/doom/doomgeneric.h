#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdint.h>
#include <SDRAM.h>

#define DOOMGENERIC_RESX 640
#define DOOMGENERIC_RESY 400

//#define DEBUG_CM7_VIDEO

extern uint32_t* DG_ScreenBuffer;

#ifdef __cplusplus
extern "C" {
#endif
void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);
void DG_OnPaletteReload();
#ifdef __cplusplus
}
#endif

#endif //DOOM_GENERIC

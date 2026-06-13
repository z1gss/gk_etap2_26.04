#ifndef GK2026_PLIKI_H_INCLUDED
#define GK2026_PLIKI_H_INCLUDED

#include <SDL2/SDL.h>

int konwersjaBMPdoGK26(const char* nazwaWej, const char* nazwaWyj,
                       int tryb, int dithering);

int konwersjaGK26doBMP(const char* nazwaWej, const char* nazwaWyj);

int ladujGK26(const char* nazwa);

#endif // GK2026_PLIKI_H_INCLUDED
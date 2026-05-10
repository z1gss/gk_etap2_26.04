// zmienne globalne
#include "GK2026-Zmienne.h"

SDL_Window*  window = NULL;
SDL_Surface* screen = NULL;

SDL_Color obrazRGB[szerokosc * wysokosc];

SDL_Color paletaKolorNarzucona[32];
Uint8     paletaSzaryNarzucona[32];
SDL_Color paletaKolorDedykowana[32];
Uint8     paletaSzaryDedykowana[32];

// palety dla wersji 3-bit..7-bit
SDL_Color paletaKolorMC[5][128];
Uint8     paletaSzaryMC[5][128];
const int ROZMIARY_PALET[5] = {8, 16, 32, 64, 128};

SDL_Color paleta8[szerokosc * wysokosc];
int       ileKolorow = 0;
SDL_Color paleta8k[256];
SDL_Color paleta8s[256];

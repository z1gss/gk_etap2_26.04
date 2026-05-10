// zmienne globalne
#ifndef GK2026_ZMIENNE_H_INCLUDED
#define GK2026_ZMIENNE_H_INCLUDED

#include <SDL2/SDL.h>

#define szerokosc 640
#define wysokosc  400

#define tytul "GK2026 - Projekt - Zespol XX"

// stale formatu .gk26
#define GK26_MAGIC0 'G'
#define GK26_MAGIC1 'K'
#define GK26_MAGIC2 '2'
#define GK26_MAGIC3 '6'
#define GK26_WERSJA 0x01
#define GK26_BPP    0x05
#define GK26_LICZBA_KOLOROW 32

// tryby pracy
#define TRYB_KOLOR_NARZUCONY  0x01
#define TRYB_SZARY_NARZUCONY  0x02
#define TRYB_KOLOR_DEDYKOWANY 0x03
#define TRYB_SZARY_DEDYKOWANY 0x04

extern SDL_Window*  window;
extern SDL_Surface* screen;

// bufor obrazu logicznego (640x400)
extern SDL_Color obrazRGB[szerokosc * wysokosc];

// palety 32-elementowe (format .gk26)
extern SDL_Color paletaKolorNarzucona[32];
extern Uint8     paletaSzaryNarzucona[32];
extern SDL_Color paletaKolorDedykowana[32];
extern Uint8     paletaSzaryDedykowana[32];

// palety dedykowane dla roznych wersji bitowych (zadanie projektowe)
// indeks 0 = 3-bit (8), 1 = 4-bit (16), 2 = 5-bit (32), 3 = 6-bit (64), 4 = 7-bit (128)
extern SDL_Color paletaKolorMC[5][128];   // max 128 kolorow
extern Uint8     paletaSzaryMC[5][128];   // max 128 odcieni
extern const int ROZMIARY_PALET[5];       // {8, 16, 32, 64, 128}

// stara paleta 256 (zgodnosc)
extern SDL_Color paleta8[szerokosc * wysokosc];
extern int       ileKolorow;
extern SDL_Color paleta8k[256];
extern SDL_Color paleta8s[256];

#endif // GK2026_ZMIENNE_H_INCLUDED

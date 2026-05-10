// funkcje do redukcji kolorow i tworzenia palet
#ifndef GK2026_PALETA_H_INCLUDED
#define GK2026_PALETA_H_INCLUDED

#include <SDL2/SDL.h>

// budowa palet narzuconych (zawsze takie same)
void zbudujPaleteKolorNarzucona();   // 32 barwy: 4 R x 4 G x 2 B
void zbudujPaleteSzaryNarzucona();   // 32 odcienie szarosci, liniowo

// budowa palet dedykowanych dla zadanego bufora pikseli
void zbudujPaleteKolorDedykowana(const SDL_Color* piksele, int liczba);
void zbudujPaleteSzaryDedykowana(const SDL_Color* piksele, int liczba);

// najblizszy kolor / szarosc w danej palecie -> indeks 0..31
int najblizszyKolorNarzucony(Uint8 r, Uint8 g, Uint8 b);
int najblizszyKolorDedykowany(Uint8 r, Uint8 g, Uint8 b);
int najblizszySzaryNarzucony(Uint8 y);
int najblizszySzaryDedykowany(Uint8 y);

// pomocnicze
Uint8 luminancja(Uint8 r, Uint8 g, Uint8 b);

// macierz Bayera 4x4 dla ditheringu uporzadkowanego (wartosci 0..15)
extern const int bayer4x4[4][4];

#endif // GK2026_PALETA_H_INCLUDED

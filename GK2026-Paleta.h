#ifndef GK2026_PALETA_H_INCLUDED
#define GK2026_PALETA_H_INCLUDED

#include <SDL2/SDL.h>

void zbudujPaleteKolorNarzucona();
void zbudujPaleteSzaryNarzucona();

void zbudujPaleteKolorDedykowana(const SDL_Color* piksele, int liczba);
void zbudujPaleteSzaryDedykowana(const SDL_Color* piksele, int liczba);

int najblizszyKolorNarzucony(Uint8 r, Uint8 g, Uint8 b);
int najblizszyKolorDedykowany(Uint8 r, Uint8 g, Uint8 b);
int najblizszySzaryNarzucony(Uint8 y);
int najblizszySzaryDedykowany(Uint8 y);

Uint8 luminancja(Uint8 r, Uint8 g, Uint8 b);

extern const int bayer4x4[4][4];

#endif // GK2026_PALETA_H_INCLUDED
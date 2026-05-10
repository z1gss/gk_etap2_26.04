// algorytm kwantyzacji barw / poziomow szarosci (MedianCut)
#ifndef GK2026_MEDIANCUT_H_INCLUDED
#define GK2026_MEDIANCUT_H_INCLUDED

#include <SDL2/SDL.h>

// wyznacza dokladnie ileBarw (potega 2) reprezentantow palety dedykowanej
// dla zbioru pikseli RGB.
void medianCutKolor(const SDL_Color* piksele, int liczbaPikseli,
                    SDL_Color* paleta, int ileBarw);

// to samo, ale w przestrzeni szarosci (luminancja)
void medianCutSzary(const SDL_Color* piksele, int liczbaPikseli,
                    Uint8* paleta, int ileBarw);

#endif // GK2026_MEDIANCUT_H_INCLUDED

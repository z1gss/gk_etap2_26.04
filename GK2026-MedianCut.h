#ifndef GK2026_MEDIANCUT_H_INCLUDED
#define GK2026_MEDIANCUT_H_INCLUDED

#include <SDL2/SDL.h>

void medianCutKolor(const SDL_Color* piksele, int liczbaPikseli,
                    SDL_Color* paleta, int ileBarw);

void medianCutSzary(const SDL_Color* piksele, int liczbaPikseli,
                    Uint8* paleta, int ileBarw);

#endif // GK2026_MEDIANCUT_H_INCLUDED
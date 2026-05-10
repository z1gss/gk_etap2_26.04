// funkcje do redukcji kolorow i tworzenia palet
#include "GK2026-Paleta.h"
#include "GK2026-Zmienne.h"
#include "GK2026-MedianCut.h"

#include <stdlib.h>
#include <math.h>

// klasyczny wzorzec Bayera 4x4 - wartosci 0..15
const int bayer4x4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 }
};

Uint8 luminancja(Uint8 r, Uint8 g, Uint8 b) {
    // klasyczne wagi BT.601
    int y = (299 * r + 587 * g + 114 * b) / 1000;
    if (y < 0)   y = 0;
    if (y > 255) y = 255;
    return (Uint8)y;
}

// 32 barwy narzucone: 4 poziomy R, 4 poziomy G, 2 poziomy B
// indeks = (R<<3) | (G<<1) | B  (R,G in 0..3, B in 0..1)
void zbudujPaleteKolorNarzucona() {
    static const Uint8 lvl4[4] = {0, 85, 170, 255};
    static const Uint8 lvl2[2] = {0, 255};
    int i = 0;
    for (int r = 0; r < 4; r++) {
        for (int g = 0; g < 4; g++) {
            for (int b = 0; b < 2; b++) {
                paletaKolorNarzucona[i].r = lvl4[r];
                paletaKolorNarzucona[i].g = lvl4[g];
                paletaKolorNarzucona[i].b = lvl4[b];
                paletaKolorNarzucona[i].a = 255;
                i++;
            }
        }
    }
}

// 32 odcienie szarosci narzucone, liniowo 0..255
void zbudujPaleteSzaryNarzucona() {
    for (int i = 0; i < 32; i++) {
        paletaSzaryNarzucona[i] = (Uint8)((i * 255 + 15) / 31);
    }
}

void zbudujPaleteKolorDedykowana(const SDL_Color* piksele, int liczba) {
    medianCutKolor(piksele, liczba, paletaKolorDedykowana, 32);
}

void zbudujPaleteSzaryDedykowana(const SDL_Color* piksele, int liczba) {
    medianCutSzary(piksele, liczba, paletaSzaryDedykowana, 32);
}

static int kwadrat(int x) { return x * x; }

int najblizszyKolorNarzucony(Uint8 r, Uint8 g, Uint8 b) {
    int najIdx = 0;
    int najD   = 1 << 30;
    for (int i = 0; i < 32; i++) {
        int d = kwadrat((int)paletaKolorNarzucona[i].r - r)
              + kwadrat((int)paletaKolorNarzucona[i].g - g)
              + kwadrat((int)paletaKolorNarzucona[i].b - b);
        if (d < najD) { najD = d; najIdx = i; }
    }
    return najIdx;
}

int najblizszyKolorDedykowany(Uint8 r, Uint8 g, Uint8 b) {
    int najIdx = 0;
    int najD   = 1 << 30;
    for (int i = 0; i < 32; i++) {
        int d = kwadrat((int)paletaKolorDedykowana[i].r - r)
              + kwadrat((int)paletaKolorDedykowana[i].g - g)
              + kwadrat((int)paletaKolorDedykowana[i].b - b);
        if (d < najD) { najD = d; najIdx = i; }
    }
    return najIdx;
}

int najblizszySzaryNarzucony(Uint8 y) {
    int idx = ((int)y * 31 + 127) / 255;
    if (idx < 0)  idx = 0;
    if (idx > 31) idx = 31;
    return idx;
}

int najblizszySzaryDedykowany(Uint8 y) {
    int najIdx = 0;
    int najD   = 1 << 30;
    for (int i = 0; i < 32; i++) {
        int d = abs((int)paletaSzaryDedykowana[i] - (int)y);
        if (d < najD) { najD = d; najIdx = i; }
    }
    return najIdx;
}

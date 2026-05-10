// funkcje do operacji na plikach
#ifndef GK2026_PLIKI_H_INCLUDED
#define GK2026_PLIKI_H_INCLUDED

#include <SDL2/SDL.h>

// konwersja BMP -> .gk26 z wybranym trybem i ditheringiem
// nazwaWej : sciezka do pliku BMP
// nazwaWyj : sciezka do pliku .gk26
// tryb     : TRYB_KOLOR_NARZUCONY / TRYB_SZARY_NARZUCONY /
//            TRYB_KOLOR_DEDYKOWANY / TRYB_SZARY_DEDYKOWANY
// dithering: 0 = brak, 1 = Floyd-Steinberg, 2 = Bayer 4x4 (uporzadkowany)
// zwraca 0 przy sukcesie
int konwersjaBMPdoGK26(const char* nazwaWej, const char* nazwaWyj,
                       int tryb, int dithering);

// konwersja .gk26 -> BMP (z odtworzeniem palety)
int konwersjaGK26doBMP(const char* nazwaWej, const char* nazwaWyj);

// wczytanie .gk26 do bufora obrazRGB i odswiezenie ekranu
int ladujGK26(const char* nazwa);

#endif // GK2026_PLIKI_H_INCLUDED

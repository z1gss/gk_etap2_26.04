// podstawowe funkcje
#ifndef GK2026_FUNKCJE_H_INCLUDED
#define GK2026_FUNKCJE_H_INCLUDED

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

// sloty istniejace (palety narzucone i dedykowane 5-bit/32 kolory)
void Funkcja1();   // narzucona kolorowa, bez ditheringu
void Funkcja2();   // narzucona kolorowa, Floyd-Steinberg
void Funkcja3();   // narzucone szarosci, bez ditheringu
void Funkcja4();   // narzucone szarosci, Floyd-Steinberg
void Funkcja5();   // dedykowana kolorowa MedianCut, bez ditheringu
void Funkcja6();   // dedykowana kolorowa MedianCut, Floyd-Steinberg
void Funkcja7();   // dedykowane szarosci MedianCut, bez ditheringu
void Funkcja8();   // dedykowane szarosci MedianCut, Floyd-Steinberg
void Funkcja9();   // .gk26 -> BMP roundtrip

void Funkcja10();  // narzucona kolorowa + Bayer 4x4
void Funkcja11();  // narzucone szarosci + Bayer 4x4

// === ZADANIE PROJEKTOWE: MedianCut rozne wersje bitowe ===
// bitIndex: 0=3-bit(8), 1=4-bit(16), 2=5-bit(32), 3=6-bit(64), 4=7-bit(128)
void zadMedianCutKolor(int bitIndex, int dithering);  // 0=brak, 1=Floyd-Steinberg
void zadMedianCutSzary(int bitIndex, int dithering);

// pokaz wynik na ekranie (lewo=oryginal, prawo=wynik, dol=paleta)
void pokazWynikZadania(int bitIndex, int kolor);

void pokazPalete(int tryb);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
SDL_Color getPixel(int x, int y);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
void ladujBMP(char const* nazwa, int x, int y);

extern char ostatniBMP[256];

#endif // GK2026_FUNKCJE_H_INCLUDED

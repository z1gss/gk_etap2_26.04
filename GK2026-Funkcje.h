#ifndef GK2026_FUNKCJE_H_INCLUDED
#define GK2026_FUNKCJE_H_INCLUDED

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

void Funkcja1();
void Funkcja2();
void Funkcja3();
void Funkcja4();
void Funkcja5();
void Funkcja6();
void Funkcja7();
void Funkcja8();
void Funkcja9();

void Funkcja10();
void Funkcja11();

void zadMedianCutKolor(int bitIndex, int dithering);
void zadMedianCutSzary(int bitIndex, int dithering);

void pokazWynikZadania(int bitIndex, int kolor);

void pokazPalete(int tryb);

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
SDL_Color getPixel(int x, int y);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
void ladujBMP(char const* nazwa, int x, int y);

extern char ostatniBMP[256];

#endif // GK2026_FUNKCJE_H_INCLUDED
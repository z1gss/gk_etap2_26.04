// glowny plik projektu - format graficzny .gk26 (5-bit, 32 barwy)
// Klawiatura:
//   a..l  -> wybor jednego z 9 plikow BMP jako biezace zrodlo
//   1     -> konwersja: paleta kolorowa narzucona, bez ditheringu
//   2     -> konwersja: paleta kolorowa narzucona, z ditheringiem (Floyd-Steinberg)
//   3     -> konwersja: szarosci narzucone, bez ditheringu
//   4     -> konwersja: szarosci narzucone, z ditheringiem (Floyd-Steinberg)
//   5     -> konwersja: paleta kolorowa dedykowana (median-cut 32), bez ditheringu
//   6     -> konwersja: paleta kolorowa dedykowana, z ditheringiem (Floyd-Steinberg)
//   7     -> konwersja: szarosci dedykowane, bez ditheringu
//   8     -> konwersja: szarosci dedykowane, z ditheringiem (Floyd-Steinberg)
//   9     -> ostatni .gk26 -> obrazek_out.bmp i podglad
//   q     -> Zad. 4: paleta kolorowa narzucona + Bayer 4x4 (uporzadkowany)
//   w     -> Zad. 3: szarosci narzucone + Bayer 4x4 (uporzadkowany)
//   p     -> podglad palety kolorowej narzuconej (bloki 60x60)
//   o     -> podglad palety szarosci narzuconej
//   i     -> podglad palety kolorowej dedykowanej (po ostatniej konwersji)
//   u     -> podglad palety szarosci dedykowanej (po ostatniej konwersji)
//   b     -> czyszczenie ekranu
//   ESC   -> wyjscie
//
// === ZADANIE PROJEKTOWE - MedianCut rozne wersje bitowe ===
//   Kolory MedianCut (lewa=oryginal, prawa=wynik):
//   z -> 3-bit (8 kolorow),  bez ditheringu
//   x -> 4-bit (16 kolorow), bez ditheringu
//   c -> 5-bit (32 kolory),  bez ditheringu
//   v -> 6-bit (64 kolory),  bez ditheringu
//   n -> 7-bit (128 kolorow), bez ditheringu
//   Z -> 3-bit (8 kolorow),  Floyd-Steinberg
//   X -> 4-bit (16 kolorow), Floyd-Steinberg
//   C -> 5-bit (32 kolory),  Floyd-Steinberg
//   V -> 6-bit (64 kolory),  Floyd-Steinberg
//   N -> 7-bit (128 kolorow), Floyd-Steinberg
//
//   Szarosci MedianCut:
//   r -> 3-bit (8 odcieni),   bez ditheringu
//   t -> 4-bit (16 odcieni),  bez ditheringu
//   y -> 5-bit (32 odcienie), bez ditheringu
//   e -> 6-bit (64 odcienie), bez ditheringu (uwaga: e zeby nie kolidowac)
//   m -> 7-bit (128 odcieni), bez ditheringu
//   R -> 3-bit, Floyd-Steinberg
//   T -> 4-bit, Floyd-Steinberg
//   Y -> 5-bit, Floyd-Steinberg
//   E -> 6-bit, Floyd-Steinberg
//   M -> 7-bit, Floyd-Steinberg

#include "GK2026-Zmienne.h"
#include "GK2026-Funkcje.h"
#include "GK2026-Paleta.h"
#include "GK2026-MedianCut.h"
#include "GK2026-Pliki.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow(tytul,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              szerokosc * 2, wysokosc * 2,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    screen = SDL_GetWindowSurface(window);
    if (screen == NULL) {
        fprintf(stderr, "SDL_GetWindowSurface Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    zbudujPaleteKolorNarzucona();
    zbudujPaleteSzaryNarzucona();

    SDL_UpdateWindowSurface(window);

    // Wypisz skroty klawiszowe
    printf("=== GK2026 - Projekt ===\n");
    printf("Ladowanie obrazkow: a=obr1  s=obr2  d=obr3  f=obr4  g=obr5\n");
    printf("                    h=obr6  j=obr7  k=obr8  l=obr9\n");
    printf("Konwersje narzucone (32 kolory): 1-4, q, w\n");
    printf("Konwersje dedykowane (32 kolory): 5-8\n");
    printf("--- ZADANIE: MedianCut rozne bity ---\n");
    printf("Kolory: z=3bit x=4bit c=5bit v=6bit n=7bit (bez ditheringu)\n");
    printf("        Z=3bit X=4bit C=5bit V=6bit N=7bit (Floyd-Steinberg)\n");
    printf("Szary:  r=3bit t=4bit y=5bit e=6bit m=7bit (bez ditheringu)\n");
    printf("        R=3bit T=4bit Y=5bit E=6bit M=7bit (Floyd-Steinberg)\n");
    printf("Palety: p=nar.kolor  o=nar.szary  i=ded.kolor  u=ded.szary\n");
    printf("b=czyszczenie  ESC=wyjscie\n\n");

    bool done = false;
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_KEYDOWN: {
                SDL_Keycode k = event.key.keysym.sym;
                Uint16 mod = event.key.keysym.mod;
                int shift = (mod & KMOD_SHIFT) ? 1 : 0;

                if (k == SDLK_ESCAPE) done = true;

                // Istniejace funkcje
                if (k == SDLK_1) Funkcja1();
                if (k == SDLK_2) Funkcja2();
                if (k == SDLK_3) Funkcja3();
                if (k == SDLK_4) Funkcja4();
                if (k == SDLK_5) Funkcja5();
                if (k == SDLK_6) Funkcja6();
                if (k == SDLK_7) Funkcja7();
                if (k == SDLK_8) Funkcja8();
                if (k == SDLK_9) Funkcja9();
                if (k == SDLK_q) Funkcja10();
                if (k == SDLK_w) Funkcja11();

                if (k == SDLK_p) pokazPalete(TRYB_KOLOR_NARZUCONY);
                if (k == SDLK_o) pokazPalete(TRYB_SZARY_NARZUCONY);
                if (k == SDLK_i) pokazPalete(TRYB_KOLOR_DEDYKOWANY);
                if (k == SDLK_u) pokazPalete(TRYB_SZARY_DEDYKOWANY);

                if (k == SDLK_a) ladujBMP("obrazek1.bmp", 0, 0);
                if (k == SDLK_s) ladujBMP("obrazek2.bmp", 0, 0);
                if (k == SDLK_d) ladujBMP("obrazek3.bmp", 0, 0);
                if (k == SDLK_f) ladujBMP("obrazek4.bmp", 0, 0);
                if (k == SDLK_g) ladujBMP("obrazek5.bmp", 0, 0);
                if (k == SDLK_h) ladujBMP("obrazek6.bmp", 0, 0);
                if (k == SDLK_j) ladujBMP("obrazek7.bmp", 0, 0);
                if (k == SDLK_k) ladujBMP("obrazek8.bmp", 0, 0);
                if (k == SDLK_l) ladujBMP("obrazek9.bmp", 0, 0);

                if (k == SDLK_b) czyscEkran(0, 0, 0);

                // ===== ZADANIE: MedianCut kolory =====
                // z/Z = 3-bit (8 kolorow)
                if (k == SDLK_z) zadMedianCutKolor(0, shift);
                // x/X = 4-bit (16 kolorow)
                if (k == SDLK_x) zadMedianCutKolor(1, shift);
                // c/C = 5-bit (32 kolory)
                if (k == SDLK_c) zadMedianCutKolor(2, shift);
                // v/V = 6-bit (64 kolory)
                if (k == SDLK_v) zadMedianCutKolor(3, shift);
                // n/N = 7-bit (128 kolorow)
                if (k == SDLK_n) zadMedianCutKolor(4, shift);

                // ===== ZADANIE: MedianCut szarosci =====
                // r/R = 3-bit (8 odcieni)
                if (k == SDLK_r) zadMedianCutSzary(0, shift);
                // t/T = 4-bit (16 odcieni)
                if (k == SDLK_t) zadMedianCutSzary(1, shift);
                // y/Y = 5-bit (32 odcienie)
                if (k == SDLK_y) zadMedianCutSzary(2, shift);
                // e/E = 6-bit (64 odcienie)
                if (k == SDLK_e) zadMedianCutSzary(3, shift);
                // m/M = 7-bit (128 odcieni)
                if (k == SDLK_m) zadMedianCutSzary(4, shift);

                break;
            }
        }
        if (done) break;
    }

    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

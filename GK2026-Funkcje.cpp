#include "GK2026-Funkcje.h"
#include "GK2026-Zmienne.h"
#include "GK2026-Paleta.h"
#include "GK2026-MedianCut.h"
#include "GK2026-Pliki.h"

#include <stdio.h>
#include <string.h>
#include <vector>
using namespace std;

char ostatniBMP[256] = "obrazek1.bmp";

static void konwertujIPokaz(int tryb, int dithering) {
    const char* nazwaWyj = "obrazek.gk26";
    if (konwersjaBMPdoGK26(ostatniBMP, nazwaWyj, tryb, dithering) != 0) return;
    ladujGK26(nazwaWyj);
}

void Funkcja1() { konwertujIPokaz(TRYB_KOLOR_NARZUCONY,  0); }
void Funkcja2() { konwertujIPokaz(TRYB_KOLOR_NARZUCONY,  1); }
void Funkcja3() { konwertujIPokaz(TRYB_SZARY_NARZUCONY,  0); }
void Funkcja4() { konwertujIPokaz(TRYB_SZARY_NARZUCONY,  1); }
void Funkcja5() { konwertujIPokaz(TRYB_KOLOR_DEDYKOWANY, 0); }
void Funkcja6() { konwertujIPokaz(TRYB_KOLOR_DEDYKOWANY, 1); }
void Funkcja7() { konwertujIPokaz(TRYB_SZARY_DEDYKOWANY, 0); }
void Funkcja8() { konwertujIPokaz(TRYB_SZARY_DEDYKOWANY, 1); }

void Funkcja9() {
    if (konwersjaGK26doBMP("obrazek.gk26", "obrazek_out.bmp") == 0)
        ladujBMP("obrazek_out.bmp", 0, 0);
}

void Funkcja10() { konwertujIPokaz(TRYB_KOLOR_NARZUCONY, 2); }
void Funkcja11() { konwertujIPokaz(TRYB_SZARY_NARZUCONY, 2); }

static int clamp(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

static void rysujPasekPaletKolor(int bitIndex) {
    int n = ROZMIARY_PALET[bitIndex];
    int W = szerokosc / 2;
    int blok = W / n;
    int yStart = wysokosc - 20;
    for (int i = 0; i < n; i++) {
        for (int yy = yStart; yy < wysokosc; yy++) {
            for (int xx = i * blok; xx < (i + 1) * blok && xx < W; xx++) {
                setPixel(szerokosc / 2 + xx, yy,
                         paletaKolorMC[bitIndex][i].r,
                         paletaKolorMC[bitIndex][i].g,
                         paletaKolorMC[bitIndex][i].b);
            }
        }
    }
}

static void rysujPasekPaletSzary(int bitIndex) {
    int n = ROZMIARY_PALET[bitIndex];
    int W = szerokosc / 2;
    int blok = W / n;
    int yStart = wysokosc - 20;
    for (int i = 0; i < n; i++) {
        Uint8 v = paletaSzaryMC[bitIndex][i];
        for (int yy = yStart; yy < wysokosc; yy++) {
            for (int xx = i * blok; xx < (i + 1) * blok && xx < W; xx++) {
                setPixel(szerokosc / 2 + xx, yy, v, v, v);
            }
        }
    }
}

static int znajdzKolor(int bitIndex, Uint8 r, Uint8 g, Uint8 b) {
    int n = ROZMIARY_PALET[bitIndex];
    int bestIdx = 0, bestD = 1 << 30;
    for (int i = 0; i < n; i++) {
        int dr = (int)paletaKolorMC[bitIndex][i].r - r;
        int dg = (int)paletaKolorMC[bitIndex][i].g - g;
        int db = (int)paletaKolorMC[bitIndex][i].b - b;
        int d = dr*dr + dg*dg + db*db;
        if (d < bestD) { bestD = d; bestIdx = i; }
    }
    return bestIdx;
}

static int znajdzSzary(int bitIndex, Uint8 y) {
    int n = ROZMIARY_PALET[bitIndex];
    int bestIdx = 0, bestD = 1 << 30;
    for (int i = 0; i < n; i++) {
        int d = abs((int)paletaSzaryMC[bitIndex][i] - (int)y);
        if (d < bestD) { bestD = d; bestIdx = i; }
    }
    return bestIdx;
}

void zadMedianCutKolor(int bitIndex, int dithering) {
    int n = ROZMIARY_PALET[bitIndex];
    int W = szerokosc;
    int H = wysokosc;
    int pixCount = W * H;

    medianCutKolor(obrazRGB, pixCount, paletaKolorMC[bitIndex], n);

    vector<int> bufR(pixCount), bufG(pixCount), bufB(pixCount);
    for (int i = 0; i < pixCount; i++) {
        bufR[i] = obrazRGB[i].r;
        bufG[i] = obrazRGB[i].g;
        bufB[i] = obrazRGB[i].b;
    }

    for (int y = 0; y < H; y++)
        for (int x = 0; x < W/2; x++)
            setPixel(x, y, obrazRGB[y*W+x].r, obrazRGB[y*W+x].g, obrazRGB[y*W+x].b);

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W/2; x++) {
            int p = y * W + x;
            Uint8 r = (Uint8)clamp(bufR[p]);
            Uint8 g = (Uint8)clamp(bufG[p]);
            Uint8 b = (Uint8)clamp(bufB[p]);

            int idx = znajdzKolor(bitIndex, r, g, b);
            SDL_Color pal = paletaKolorMC[bitIndex][idx];
            setPixel(W/2 + x, y, pal.r, pal.g, pal.b);

            if (dithering == 1) {
                int eR = (int)r - pal.r;
                int eG = (int)g - pal.g;
                int eB = (int)b - pal.b;
                if (x+1 < W/2) {
                    bufR[p+1] += eR*7/16;
                    bufG[p+1] += eG*7/16;
                    bufB[p+1] += eB*7/16;
                }
                if (y+1 < H) {
                    if (x > 0) {
                        bufR[p+W-1] += eR*3/16;
                        bufG[p+W-1] += eG*3/16;
                        bufB[p+W-1] += eB*3/16;
                    }
                    bufR[p+W] += eR*5/16;
                    bufG[p+W] += eG*5/16;
                    bufB[p+W] += eB*5/16;
                    if (x+1 < W/2) {
                        bufR[p+W+1] += eR*1/16;
                        bufG[p+W+1] += eG*1/16;
                        bufB[p+W+1] += eB*1/16;
                    }
                }
            }
        }
    }

    rysujPasekPaletKolor(bitIndex);

    char tytulOkna[128];
    const char* dithStr = dithering ? " + Floyd-Steinberg" : "";
    sprintf(tytulOkna, "GK2026 - MedianCut %d-bit (%d kolorow)%s",
            bitIndex + 3, n, dithStr);
    SDL_SetWindowTitle(window, tytulOkna);

    SDL_UpdateWindowSurface(window);
    printf("MedianCut kolor %d-bit (%d kolorow)%s\n", bitIndex+3, n, dithStr);
}

void zadMedianCutSzary(int bitIndex, int dithering) {
    int n = ROZMIARY_PALET[bitIndex];
    int W = szerokosc;
    int H = wysokosc;
    int pixCount = W * H;

    medianCutSzary(obrazRGB, pixCount, paletaSzaryMC[bitIndex], n);

    vector<int> bufY(pixCount);
    for (int i = 0; i < pixCount; i++) {
        bufY[i] = luminancja(obrazRGB[i].r, obrazRGB[i].g, obrazRGB[i].b);
    }

    for (int y = 0; y < H; y++)
        for (int x = 0; x < W/2; x++)
            setPixel(x, y, obrazRGB[y*W+x].r, obrazRGB[y*W+x].g, obrazRGB[y*W+x].b);

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W/2; x++) {
            int p = y * W + x;
            Uint8 lum = (Uint8)clamp(bufY[p]);

            int idx = znajdzSzary(bitIndex, lum);
            Uint8 v = paletaSzaryMC[bitIndex][idx];
            setPixel(W/2 + x, y, v, v, v);

            if (dithering == 1) {
                int e = (int)lum - (int)v;
                if (x+1 < W/2)  bufY[p+1]   += e*7/16;
                if (y+1 < H) {
                    if (x > 0)   bufY[p+W-1] += e*3/16;
                    bufY[p+W]               += e*5/16;
                    if (x+1 < W/2) bufY[p+W+1] += e*1/16;
                }
            }
        }
    }

    rysujPasekPaletSzary(bitIndex);

    char tytulOkna[128];
    const char* dithStr = dithering ? " + Floyd-Steinberg" : "";
    sprintf(tytulOkna, "GK2026 - Szarosci MedianCut %d-bit (%d odcieni)%s",
            bitIndex + 3, n, dithStr);
    SDL_SetWindowTitle(window, tytulOkna);

    SDL_UpdateWindowSurface(window);
    printf("MedianCut szary %d-bit (%d odcieni)%s\n", bitIndex+3, n, dithStr);
}

void pokazPalete(int tryb) {
    czyscEkran(0, 0, 0);
    const int blok = 60;
    const int kolumn = 8;
    const int marg = 4;
    for (int i = 0; i < 32; i++) {
        int kx = (i % kolumn) * blok + marg;
        int ky = (i / kolumn) * blok + marg;
        Uint8 r = 0, g = 0, b = 0;
        switch (tryb) {
            case TRYB_KOLOR_NARZUCONY:
                r = paletaKolorNarzucona[i].r;
                g = paletaKolorNarzucona[i].g;
                b = paletaKolorNarzucona[i].b; break;
            case TRYB_KOLOR_DEDYKOWANY:
                r = paletaKolorDedykowana[i].r;
                g = paletaKolorDedykowana[i].g;
                b = paletaKolorDedykowana[i].b; break;
            case TRYB_SZARY_NARZUCONY:
                r = g = b = paletaSzaryNarzucona[i]; break;
            case TRYB_SZARY_DEDYKOWANY:
                r = g = b = paletaSzaryDedykowana[i]; break;
        }
        for (int yy = 0; yy < blok - 2*marg; yy++)
            for (int xx = 0; xx < blok - 2*marg; xx++)
                setPixel(kx+xx, ky+yy, r, g, b);
    }
    SDL_UpdateWindowSurface(window);
}

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B) {
    if ((x>=0)&&(x<szerokosc)&&(y>=0)&&(y<wysokosc)) {
        Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);
        int bpp = screen->format->BytesPerPixel;
        Uint8 *p1 = (Uint8*)screen->pixels + (y*2)*screen->pitch   + (x*2)*bpp;
        Uint8 *p2 = (Uint8*)screen->pixels + (y*2+1)*screen->pitch + (x*2)*bpp;
        Uint8 *p3 = (Uint8*)screen->pixels + (y*2)*screen->pitch   + (x*2+1)*bpp;
        Uint8 *p4 = (Uint8*)screen->pixels + (y*2+1)*screen->pitch + (x*2+1)*bpp;
        switch(bpp) {
            case 1: *p1=*p2=*p3=*p4=(Uint8)pixel; break;
            case 2: *(Uint16*)p1=*(Uint16*)p2=*(Uint16*)p3=*(Uint16*)p4=(Uint16)pixel; break;
            case 3:
                if(SDL_BYTEORDER==SDL_BIG_ENDIAN){
                    p1[0]=p2[0]=p3[0]=p4[0]=(pixel>>16)&0xff;
                    p1[1]=p2[1]=p3[1]=p4[1]=(pixel>>8)&0xff;
                    p1[2]=p2[2]=p3[2]=p4[2]=pixel&0xff;
                } else {
                    p1[0]=p2[0]=p3[0]=p4[0]=pixel&0xff;
                    p1[1]=p2[1]=p3[1]=p4[1]=(pixel>>8)&0xff;
                    p1[2]=p2[2]=p3[2]=p4[2]=(pixel>>16)&0xff;
                }
                break;
            case 4: *(Uint32*)p1=*(Uint32*)p2=*(Uint32*)p3=*(Uint32*)p4=pixel; break;
        }
    }
}

void setPixelSurface(int x, int y, Uint8 R, Uint8 G, Uint8 B) {
    if ((x>=0)&&(x<szerokosc*2)&&(y>=0)&&(y<wysokosc*2)) {
        Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);
        int bpp = screen->format->BytesPerPixel;
        Uint8 *p = (Uint8*)screen->pixels + y*screen->pitch + x*bpp;
        switch(bpp) {
            case 1: *p=(Uint8)pixel; break;
            case 2: *(Uint16*)p=(Uint16)pixel; break;
            case 3:
                if(SDL_BYTEORDER==SDL_BIG_ENDIAN){p[0]=(pixel>>16)&0xff;p[1]=(pixel>>8)&0xff;p[2]=pixel&0xff;}
                else {p[0]=pixel&0xff;p[1]=(pixel>>8)&0xff;p[2]=(pixel>>16)&0xff;}
                break;
            case 4: *(Uint32*)p=pixel; break;
        }
    }
}

SDL_Color getPixel(int x, int y) {
    SDL_Color color = {0,0,0,255};
    Uint32 col = 0;
    if ((x>=0)&&(x<szerokosc)&&(y>=0)&&(y<wysokosc)) {
        char* pPos=(char*)screen->pixels;
        pPos += screen->pitch*y*2;
        pPos += screen->format->BytesPerPixel*x*2;
        memcpy(&col, pPos, screen->format->BytesPerPixel);
        SDL_GetRGB(col, screen->format, &color.r, &color.g, &color.b);
    }
    return color;
}

SDL_Color getPixelSurface(int x, int y, SDL_Surface* surface) {
    SDL_Color color = {0,0,0,255};
    Uint32 col = 0;
    if ((x>=0)&&(x<surface->w)&&(y>=0)&&(y<surface->h)) {
        char* pPos=(char*)surface->pixels;
        pPos += surface->pitch*y;
        pPos += surface->format->BytesPerPixel*x;
        memcpy(&col, pPos, surface->format->BytesPerPixel);
        SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
    }
    return color;
}

void ladujBMP(char const* nazwa, int x, int y) {
    (void)x; (void)y;
    SDL_Surface* bmp = SDL_LoadBMP(nazwa);
    if (!bmp) { printf("Nie mozna wczytac: %s\n", SDL_GetError()); return; }

    strncpy(ostatniBMP, nazwa, sizeof(ostatniBMP)-1);
    ostatniBMP[sizeof(ostatniBMP)-1] = 0;

    int W = (bmp->w < szerokosc) ? bmp->w : szerokosc;
    int H = (bmp->h < wysokosc)  ? bmp->h : wysokosc;
    for (int yy=0; yy<H; yy++) {
        for (int xx=0; xx<W; xx++) {
            SDL_Color kolor = getPixelSurface(xx, yy, bmp);
            obrazRGB[yy*szerokosc+xx] = kolor;
            setPixel(xx, yy, kolor.r, kolor.g, kolor.b);
        }
    }
    SDL_FreeSurface(bmp);
    SDL_SetWindowTitle(window, tytul);
    SDL_UpdateWindowSurface(window);
    printf("Zaladowano: %s\n", nazwa);
}

void czyscEkran(Uint8 R, Uint8 G, Uint8 B) {
    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, R, G, B));
    SDL_UpdateWindowSurface(window);
}
// funkcje do operacji na plikach
#include "GK2026-Pliki.h"
#include "GK2026-Zmienne.h"
#include "GK2026-Funkcje.h"
#include "GK2026-Paleta.h"
#include "GK2026-MedianCut.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

// ---------- bitstream 5-bit ----------

struct ZapisBitow {
    vector<Uint8> bajty;
    int           bitWBajcie; // 0..7, ile bitow juz zajmuje aktualny bajt (od MSB)

    ZapisBitow() : bitWBajcie(0) {}

    void wpisz5(Uint8 v) {
        v &= 0x1F;
        for (int i = 4; i >= 0; i--) {
            int bit = (v >> i) & 1;
            if (bitWBajcie == 0) bajty.push_back(0);
            bajty.back() |= (Uint8)(bit << (7 - bitWBajcie));
            bitWBajcie = (bitWBajcie + 1) & 7;
        }
    }
};

struct OdczytBitow {
    const Uint8* bajty;
    int          rozmiar;
    int          poz;       // licznik bitow od poczatku

    OdczytBitow(const Uint8* b, int r) : bajty(b), rozmiar(r), poz(0) {}

    Uint8 czytaj5() {
        Uint8 v = 0;
        for (int i = 0; i < 5; i++) {
            int bajtIdx = poz >> 3;
            int bitIdx  = 7 - (poz & 7);
            int bit     = (bajtIdx < rozmiar) ? ((bajty[bajtIdx] >> bitIdx) & 1) : 0;
            v = (Uint8)((v << 1) | bit);
            poz++;
        }
        return v;
    }
};

// ---------- konwersje obraz -> indeksy ----------

// stosuje filtracje + dithering wybranego rodzaju
//   dithering: 0 = brak, 1 = Floyd-Steinberg, 2 = Bayer 4x4 (uporzadkowany)
// zapisuje wynikowe indeksy 0..31 do tablicy idx (rozmiar w*h)
static void filtrujKolor(const SDL_Color* zrodlo, int w, int h,
                         Uint8* idx, int dithering, int dedykowana) {
    // pracujemy na buforze typu int dla bezpiecznego rozprowadzania bledu
    vector<int> rR(w * h), rG(w * h), rB(w * h);
    for (int i = 0; i < w * h; i++) {
        rR[i] = zrodlo[i].r;
        rG[i] = zrodlo[i].g;
        rB[i] = zrodlo[i].b;
    }

    // skoki kwantyzacji dla narzuconej palety 2-2-1 (R 4 poziomy, G 4, B 2)
    // wykorzystywane przy ditheringu Bayera, aby bias byl wlasciwego rzedu
    const int stepR = 256 / 4;
    const int stepG = 256 / 4;
    const int stepB = 256 / 2;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int p = y * w + x;
            int r = rR[p];
            int g = rG[p];
            int b = rB[p];

            // Bayer 4x4: dodaj bias z zakresu (-step/2, +step/2)
            // wzor: (M+0.5)/16 - 0.5  -> calkowicie: (2M - 15) / 32
            if (dithering == 2) {
                int M = bayer4x4[y & 3][x & 3];
                r += ((2 * M - 15) * stepR) / 32;
                g += ((2 * M - 15) * stepG) / 32;
                b += ((2 * M - 15) * stepB) / 32;
            }

            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;

            int k = dedykowana
                ? najblizszyKolorDedykowany((Uint8)r, (Uint8)g, (Uint8)b)
                : najblizszyKolorNarzucony((Uint8)r, (Uint8)g, (Uint8)b);
            idx[p] = (Uint8)k;

            if (dithering == 1) {
                // Floyd-Steinberg - rozprowadzanie bledu kwantyzacji do sasiadow
                SDL_Color pal = dedykowana ? paletaKolorDedykowana[k]
                                           : paletaKolorNarzucona[k];
                int eR = r - pal.r;
                int eG = g - pal.g;
                int eB = b - pal.b;

                if (x + 1 < w) {
                    rR[p + 1]     += eR * 7 / 16;
                    rG[p + 1]     += eG * 7 / 16;
                    rB[p + 1]     += eB * 7 / 16;
                }
                if (y + 1 < h) {
                    if (x > 0) {
                        rR[p + w - 1] += eR * 3 / 16;
                        rG[p + w - 1] += eG * 3 / 16;
                        rB[p + w - 1] += eB * 3 / 16;
                    }
                    rR[p + w]     += eR * 5 / 16;
                    rG[p + w]     += eG * 5 / 16;
                    rB[p + w]     += eB * 5 / 16;
                    if (x + 1 < w) {
                        rR[p + w + 1] += eR * 1 / 16;
                        rG[p + w + 1] += eG * 1 / 16;
                        rB[p + w + 1] += eB * 1 / 16;
                    }
                }
            }
        }
    }
}

static void filtrujSzary(const SDL_Color* zrodlo, int w, int h,
                         Uint8* idx, int dithering, int dedykowana) {
    vector<int> rY(w * h);
    for (int i = 0; i < w * h; i++) {
        rY[i] = luminancja(zrodlo[i].r, zrodlo[i].g, zrodlo[i].b);
    }

    // skok kwantyzacji dla 32 narzuconych poziomow szarosci
    const int stepY = 256 / 32;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int p = y * w + x;
            int yy = rY[p];

            // Bayer 4x4: bias z (-step/2, +step/2)
            if (dithering == 2) {
                int M = bayer4x4[y & 3][x & 3];
                yy += ((2 * M - 15) * stepY) / 32;
            }

            if (yy < 0) yy = 0; if (yy > 255) yy = 255;

            int k = dedykowana
                ? najblizszySzaryDedykowany((Uint8)yy)
                : najblizszySzaryNarzucony((Uint8)yy);
            idx[p] = (Uint8)k;

            if (dithering == 1) {
                // Floyd-Steinberg
                Uint8 pal = dedykowana ? paletaSzaryDedykowana[k]
                                       : paletaSzaryNarzucona[k];
                int e = yy - pal;
                if (x + 1 < w) rY[p + 1]     += e * 7 / 16;
                if (y + 1 < h) {
                    if (x > 0)     rY[p + w - 1] += e * 3 / 16;
                    rY[p + w]     += e * 5 / 16;
                    if (x + 1 < w) rY[p + w + 1] += e * 1 / 16;
                }
            }
        }
    }
}

// ---------- konwersja BMP -> GK26 ----------

int konwersjaBMPdoGK26(const char* nazwaWej, const char* nazwaWyj,
                       int tryb, int dithering) {
    SDL_Surface* bmp = SDL_LoadBMP(nazwaWej);
    if (!bmp) {
        printf("Nie mozna wczytac BMP: %s\n", SDL_GetError());
        return -1;
    }

    int w = bmp->w;
    int h = bmp->h;

    // wczytujemy do tymczasowego bufora o rozmiarze odpowiadajacym BMP
    vector<SDL_Color> zrodlo(w * h);
    SDL_LockSurface(bmp);
    for (int yy = 0; yy < h; yy++) {
        for (int xx = 0; xx < w; xx++) {
            Uint32 col = 0;
            char* pPos = (char*)bmp->pixels + bmp->pitch * yy
                       + bmp->format->BytesPerPixel * xx;
            memcpy(&col, pPos, bmp->format->BytesPerPixel);
            SDL_Color c;
            SDL_GetRGB(col, bmp->format, &c.r, &c.g, &c.b);
            c.a = 255;
            zrodlo[yy * w + xx] = c;
        }
    }
    SDL_UnlockSurface(bmp);
    SDL_FreeSurface(bmp);

    // kopia do globalu obrazRGB (z przycieciem do pojemnosci)
    int W = (w < szerokosc) ? w : szerokosc;
    int H = (h < wysokosc)  ? h : wysokosc;
    for (int yy = 0; yy < H; yy++)
        for (int xx = 0; xx < W; xx++)
            obrazRGB[yy * szerokosc + xx] = zrodlo[yy * w + xx];

    // budujemy palete wlasciwa dla trybu
    int kolor      = (tryb == TRYB_KOLOR_NARZUCONY || tryb == TRYB_KOLOR_DEDYKOWANY);
    int dedykowana = (tryb == TRYB_KOLOR_DEDYKOWANY || tryb == TRYB_SZARY_DEDYKOWANY);
    if (tryb == TRYB_KOLOR_NARZUCONY)  zbudujPaleteKolorNarzucona();
    if (tryb == TRYB_SZARY_NARZUCONY)  zbudujPaleteSzaryNarzucona();
    if (tryb == TRYB_KOLOR_DEDYKOWANY) zbudujPaleteKolorDedykowana(&zrodlo[0], w * h);
    if (tryb == TRYB_SZARY_DEDYKOWANY) zbudujPaleteSzaryDedykowana(&zrodlo[0], w * h);

    vector<Uint8> idx(w * h);
    if (kolor) filtrujKolor(&zrodlo[0], w, h, &idx[0], dithering, dedykowana);
    else       filtrujSzary(&zrodlo[0], w, h, &idx[0], dithering, dedykowana);

    // pakowanie 5-bit
    ZapisBitow bs;
    for (int i = 0; i < w * h; i++) bs.wpisz5(idx[i]);

    // zapis pliku
    FILE* f = fopen(nazwaWyj, "wb");
    if (!f) { printf("Nie mozna otworzyc pliku do zapisu: %s\n", nazwaWyj); return -1; }

    Uint8 hdr[16];
    hdr[0] = GK26_MAGIC0;
    hdr[1] = GK26_MAGIC1;
    hdr[2] = GK26_MAGIC2;
    hdr[3] = GK26_MAGIC3;
    hdr[4] = GK26_WERSJA;
    hdr[5] = (Uint8)tryb;
    hdr[6] = (Uint8)(dithering & 0xFF); // 0=brak, 1=Floyd-Steinberg, 2=Bayer 4x4
    hdr[7] = GK26_BPP;
    hdr[8]  = (Uint8)(w & 0xFF);
    hdr[9]  = (Uint8)((w >> 8) & 0xFF);
    hdr[10] = (Uint8)(h & 0xFF);
    hdr[11] = (Uint8)((h >> 8) & 0xFF);
    Uint32 dataSize = (Uint32)bs.bajty.size();
    hdr[12] = (Uint8)( dataSize        & 0xFF);
    hdr[13] = (Uint8)((dataSize >>  8) & 0xFF);
    hdr[14] = (Uint8)((dataSize >> 16) & 0xFF);
    hdr[15] = (Uint8)((dataSize >> 24) & 0xFF);
    fwrite(hdr, 1, 16, f);

    // paleta tylko dla trybow dedykowanych
    if (tryb == TRYB_KOLOR_DEDYKOWANY) {
        for (int i = 0; i < 32; i++) {
            Uint8 rgb[3] = { paletaKolorDedykowana[i].r,
                             paletaKolorDedykowana[i].g,
                             paletaKolorDedykowana[i].b };
            fwrite(rgb, 1, 3, f);
        }
    } else if (tryb == TRYB_SZARY_DEDYKOWANY) {
        fwrite(paletaSzaryDedykowana, 1, 32, f);
    }

    if (!bs.bajty.empty())
        fwrite(&bs.bajty[0], 1, bs.bajty.size(), f);
    fclose(f);

    printf("Zapisano %s (tryb=%d dithering=%d) -- %d bajtow danych\n",
           nazwaWyj, tryb, dithering, (int)dataSize);
    return 0;
}

// ---------- odczyt .gk26 ----------

// uniwersalny odczyt do bufora obrazRGB; zwraca 0 ok, -1 blad
static int wczytajGK26(const char* nazwa, int* zwrocW, int* zwrocH,
                       int* zwrocTryb, int* zwrocDith) {
    FILE* f = fopen(nazwa, "rb");
    if (!f) { printf("Nie mozna otworzyc %s\n", nazwa); return -1; }

    Uint8 hdr[16];
    if (fread(hdr, 1, 16, f) != 16) { fclose(f); return -1; }
    if (hdr[0] != GK26_MAGIC0 || hdr[1] != GK26_MAGIC1 ||
        hdr[2] != GK26_MAGIC2 || hdr[3] != GK26_MAGIC3) {
        printf("To nie jest plik .gk26\n"); fclose(f); return -1;
    }

    int wer  = hdr[4];
    int tryb = hdr[5];
    int dith = hdr[6];
    int bpp  = hdr[7];
    int w    = hdr[8]  | (hdr[9]  << 8);
    int h    = hdr[10] | (hdr[11] << 8);
    Uint32 dataSize = (Uint32)hdr[12]
                    | ((Uint32)hdr[13] << 8)
                    | ((Uint32)hdr[14] << 16)
                    | ((Uint32)hdr[15] << 24);

    if (wer != GK26_WERSJA || bpp != GK26_BPP) {
        printf("Niewspierana wersja/bpp pliku\n"); fclose(f); return -1;
    }

    // wczytujemy palete (jesli tryb dedykowany) i odtwarzamy palety narzucone
    if (tryb == TRYB_KOLOR_NARZUCONY) zbudujPaleteKolorNarzucona();
    if (tryb == TRYB_SZARY_NARZUCONY) zbudujPaleteSzaryNarzucona();
    if (tryb == TRYB_KOLOR_DEDYKOWANY) {
        for (int i = 0; i < 32; i++) {
            Uint8 rgb[3];
            if (fread(rgb, 1, 3, f) != 3) { fclose(f); return -1; }
            paletaKolorDedykowana[i].r = rgb[0];
            paletaKolorDedykowana[i].g = rgb[1];
            paletaKolorDedykowana[i].b = rgb[2];
            paletaKolorDedykowana[i].a = 255;
        }
    } else if (tryb == TRYB_SZARY_DEDYKOWANY) {
        if (fread(paletaSzaryDedykowana, 1, 32, f) != 32) { fclose(f); return -1; }
    }

    vector<Uint8> dane(dataSize);
    if (dataSize > 0 && fread(&dane[0], 1, dataSize, f) != dataSize) {
        fclose(f); return -1;
    }
    fclose(f);

    OdczytBitow ob(dane.empty() ? NULL : &dane[0], (int)dataSize);
    int n = w * h;
    int maxN = szerokosc * wysokosc;
    if (n > maxN) n = maxN;
    for (int i = 0; i < n; i++) {
        Uint8 idx = ob.czytaj5();
        if (idx > 31) idx = 31;
        if (tryb == TRYB_KOLOR_NARZUCONY) {
            obrazRGB[i] = paletaKolorNarzucona[idx];
        } else if (tryb == TRYB_KOLOR_DEDYKOWANY) {
            obrazRGB[i] = paletaKolorDedykowana[idx];
        } else if (tryb == TRYB_SZARY_NARZUCONY) {
            Uint8 v = paletaSzaryNarzucona[idx];
            obrazRGB[i].r = v; obrazRGB[i].g = v; obrazRGB[i].b = v; obrazRGB[i].a = 255;
        } else if (tryb == TRYB_SZARY_DEDYKOWANY) {
            Uint8 v = paletaSzaryDedykowana[idx];
            obrazRGB[i].r = v; obrazRGB[i].g = v; obrazRGB[i].b = v; obrazRGB[i].a = 255;
        }
    }

    *zwrocW = w; *zwrocH = h;
    *zwrocTryb = tryb; *zwrocDith = dith;
    return 0;
}

int ladujGK26(const char* nazwa) {
    int w, h, tryb, dith;
    if (wczytajGK26(nazwa, &w, &h, &tryb, &dith) != 0) return -1;

    czyscEkran(0, 0, 0);
    int W = (w < szerokosc) ? w : szerokosc;
    int H = (h < wysokosc)  ? h : wysokosc;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            SDL_Color c = obrazRGB[y * w + x];
            setPixel(x, y, c.r, c.g, c.b);
        }
    }
    SDL_UpdateWindowSurface(window);
    printf("Wczytano %s: %dx%d tryb=%d dith=%d\n", nazwa, w, h, tryb, dith);
    return 0;
}

int konwersjaGK26doBMP(const char* nazwaWej, const char* nazwaWyj) {
    int w, h, tryb, dith;
    if (wczytajGK26(nazwaWej, &w, &h, &tryb, &dith) != 0) return -1;

    SDL_Surface* out = SDL_CreateRGBSurfaceWithFormat(
        0, w, h, 24, SDL_PIXELFORMAT_RGB24);
    if (!out) {
        printf("SDL_CreateRGBSurface: %s\n", SDL_GetError());
        return -1;
    }
    SDL_LockSurface(out);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SDL_Color c = obrazRGB[y * w + x];
            Uint8* p = (Uint8*)out->pixels + y * out->pitch + x * 3;
            // SDL_PIXELFORMAT_RGB24 = R,G,B w pamieci
            p[0] = c.r; p[1] = c.g; p[2] = c.b;
        }
    }
    SDL_UnlockSurface(out);
    int rc = SDL_SaveBMP(out, nazwaWyj);
    SDL_FreeSurface(out);
    if (rc != 0) { printf("SDL_SaveBMP: %s\n", SDL_GetError()); return -1; }
    printf("Zapisano BMP: %s (%dx%d)\n", nazwaWyj, w, h);
    return 0;
}

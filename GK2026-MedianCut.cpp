// algorytm kwantyzacji barw / poziomow szarosci (MedianCut)
#include "GK2026-MedianCut.h"
#include "GK2026-Zmienne.h"

#include <algorithm>
#include <vector>
#include <stdlib.h>

using namespace std;

// pojedynczy "kubelek" - zakres pikseli i jego rozpietosci po skladowych
struct Kubelek {
    int start;
    int koniec; // wylacznie
};

static int rozpietoscR(const SDL_Color* px, int s, int e) {
    int mn = 255, mx = 0;
    for (int i = s; i < e; i++) {
        if (px[i].r < mn) mn = px[i].r;
        if (px[i].r > mx) mx = px[i].r;
    }
    return mx - mn;
}
static int rozpietoscG(const SDL_Color* px, int s, int e) {
    int mn = 255, mx = 0;
    for (int i = s; i < e; i++) {
        if (px[i].g < mn) mn = px[i].g;
        if (px[i].g > mx) mx = px[i].g;
    }
    return mx - mn;
}
static int rozpietoscB(const SDL_Color* px, int s, int e) {
    int mn = 255, mx = 0;
    for (int i = s; i < e; i++) {
        if (px[i].b < mn) mn = px[i].b;
        if (px[i].b > mx) mx = px[i].b;
    }
    return mx - mn;
}

static bool cmpR(const SDL_Color& a, const SDL_Color& b) { return a.r < b.r; }
static bool cmpG(const SDL_Color& a, const SDL_Color& b) { return a.g < b.g; }
static bool cmpB(const SDL_Color& a, const SDL_Color& b) { return a.b < b.b; }

void medianCutKolor(const SDL_Color* piksele, int liczbaPikseli,
                    SDL_Color* paleta, int ileBarw) {
    if (liczbaPikseli <= 0 || ileBarw <= 0) return;

    // pracujemy na kopii - bedziemy sortowac
    vector<SDL_Color> bufor(piksele, piksele + liczbaPikseli);

    vector<Kubelek> kub;
    kub.push_back({0, liczbaPikseli});

    while ((int)kub.size() < ileBarw) {
        // wybieramy kubelek z najwieksza rozpietoscia
        int idxNaj = -1;
        int najR   = -1;
        char najOs = 'R';
        for (size_t k = 0; k < kub.size(); k++) {
            int s = kub[k].start, e = kub[k].koniec;
            if (e - s < 2) continue;
            int rr = rozpietoscR(&bufor[0], s, e);
            int gg = rozpietoscG(&bufor[0], s, e);
            int bb = rozpietoscB(&bufor[0], s, e);
            int najLok = rr; char os = 'R';
            if (gg > najLok) { najLok = gg; os = 'G'; }
            if (bb > najLok) { najLok = bb; os = 'B'; }
            if (najLok > najR) { najR = najLok; idxNaj = (int)k; najOs = os; }
        }
        if (idxNaj < 0) break; // nie da sie juz dzielic

        int s = kub[idxNaj].start, e = kub[idxNaj].koniec;
        if (najOs == 'R') sort(bufor.begin() + s, bufor.begin() + e, cmpR);
        else if (najOs == 'G') sort(bufor.begin() + s, bufor.begin() + e, cmpG);
        else                   sort(bufor.begin() + s, bufor.begin() + e, cmpB);

        int srodek = (s + e) / 2;
        Kubelek lewa  = {s, srodek};
        Kubelek prawa = {srodek, e};
        kub[idxNaj] = lewa;
        kub.push_back(prawa);
    }

    // wyznaczamy reprezentanta (srednia) dla kazdego kubelka
    for (int i = 0; i < ileBarw; i++) {
        if (i < (int)kub.size() && kub[i].koniec > kub[i].start) {
            long long sR = 0, sG = 0, sB = 0;
            int n = kub[i].koniec - kub[i].start;
            for (int j = kub[i].start; j < kub[i].koniec; j++) {
                sR += bufor[j].r;
                sG += bufor[j].g;
                sB += bufor[j].b;
            }
            paleta[i].r = (Uint8)(sR / n);
            paleta[i].g = (Uint8)(sG / n);
            paleta[i].b = (Uint8)(sB / n);
            paleta[i].a = 255;
        } else {
            // mniej kubelkow niz potrzeba - wypelniamy duplikatem
            paleta[i] = paleta[0];
        }
    }
}

void medianCutSzary(const SDL_Color* piksele, int liczbaPikseli,
                    Uint8* paleta, int ileBarw) {
    if (liczbaPikseli <= 0 || ileBarw <= 0) return;

    vector<Uint8> bufor(liczbaPikseli);
    for (int i = 0; i < liczbaPikseli; i++) {
        int y = (299 * piksele[i].r + 587 * piksele[i].g + 114 * piksele[i].b) / 1000;
        if (y < 0) y = 0; if (y > 255) y = 255;
        bufor[i] = (Uint8)y;
    }

    vector<Kubelek> kub;
    kub.push_back({0, liczbaPikseli});

    while ((int)kub.size() < ileBarw) {
        int idxNaj = -1, najR = -1;
        for (size_t k = 0; k < kub.size(); k++) {
            int s = kub[k].start, e = kub[k].koniec;
            if (e - s < 2) continue;
            int mn = 255, mx = 0;
            for (int i = s; i < e; i++) {
                if (bufor[i] < mn) mn = bufor[i];
                if (bufor[i] > mx) mx = bufor[i];
            }
            int rr = mx - mn;
            if (rr > najR) { najR = rr; idxNaj = (int)k; }
        }
        if (idxNaj < 0) break;

        int s = kub[idxNaj].start, e = kub[idxNaj].koniec;
        sort(bufor.begin() + s, bufor.begin() + e);
        int srodek = (s + e) / 2;
        Kubelek lewa  = {s, srodek};
        Kubelek prawa = {srodek, e};
        kub[idxNaj] = lewa;
        kub.push_back(prawa);
    }

    for (int i = 0; i < ileBarw; i++) {
        if (i < (int)kub.size() && kub[i].koniec > kub[i].start) {
            long long sY = 0;
            int n = kub[i].koniec - kub[i].start;
            for (int j = kub[i].start; j < kub[i].koniec; j++) sY += bufor[j];
            paleta[i] = (Uint8)(sY / n);
        } else {
            paleta[i] = paleta[0];
        }
    }

    // sortujemy palete szarosci rosnaco - czytelne i ulatwia wyszukiwanie
    sort(paleta, paleta + ileBarw);
}

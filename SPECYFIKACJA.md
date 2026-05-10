# Specyfikacja formatu graficznego `.gk26`

Projekt GK2026 — rastrowy plik graficzny z 5-bitowym alfabetem wejściowym (32 symbole),
wspierający cztery tryby pracy:

| Tryb | Wartość | Zawartość                                       |
|------|---------|-------------------------------------------------|
| 1    | `0x01`  | Paleta **kolorowa narzucona** (32 barwy)        |
| 2    | `0x02`  | **Skala szarości narzucona** (32 odcienie)      |
| 3    | `0x03`  | Paleta **kolorowa dedykowana** (32 barwy)       |
| 4    | `0x04`  | **Skala szarości dedykowana** (32 odcienie)     |

Plik składa się z trzech sekcji: **nagłówka**, opcjonalnej **palety** (tylko w trybach
dedykowanych) oraz **danych pikseli** spakowanych po 5 bitów na piksel.

---

## 1. Nagłówek (16 bajtów)

Wszystkie pola wielobajtowe są zapisywane w kolejności **little-endian**.

| Offset | Rozmiar | Pole               | Opis                                                                 |
|-------:|--------:|--------------------|----------------------------------------------------------------------|
| 0      | 4 B     | `magic`            | Sygnatura formatu, wartość `'G' 'K' '2' '6'` (`0x47 0x4B 0x32 0x36`) |
| 4      | 1 B     | `wersja`           | Numer wersji formatu (obecnie `0x01`)                                |
| 5      | 1 B     | `tryb`             | Tryb pracy (`0x01`–`0x04`, patrz tabela powyżej)                     |
| 6      | 1 B     | `dithering`        | `0x00` = bez ditheringu, `0x01` = Floyd-Steinberg, `0x02` = Bayer 4×4 |
| 7      | 1 B     | `bpp`              | Liczba bitów na piksel — zawsze `0x05` (5-bitowy alfabet wejściowy)  |
| 8      | 2 B     | `szerokość`        | Szerokość obrazu w pikselach (LE)                                    |
| 10     | 2 B     | `wysokość`         | Wysokość obrazu w pikselach (LE)                                     |
| 12     | 4 B     | `rozmiar danych`   | Rozmiar bloku spakowanych pikseli w bajtach (LE)                     |

Pole `dithering` jest informacyjne — sygnalizuje, że dane zostały już zapisane jako
piksele po przeprowadzonym ditheringu. Dekoder nie wykonuje już żadnej dodatkowej
filtracji.

---

## 2. Paleta

Paleta jest zapisywana wyłącznie dla trybów **dedykowanych** (3 i 4). Dla trybów
narzuconych (1 i 2) paleta jest stała i odtwarzana po stronie dekodera.

### 2.1. Tryb 3 — paleta kolorowa dedykowana (96 B)

Zaraz po nagłówku zapisywanych jest 32 wpisów po 3 bajty:

```
R0 G0 B0  R1 G1 B1  ...  R31 G31 B31
```

Każda składowa jest 8-bitową wartością `0..255`. Indeks `i` w danych pikseli odnosi
się do `i`-tego wpisu palety.

### 2.2. Tryb 4 — skala szarości dedykowana (32 B)

```
Y0 Y1 Y2 ... Y31
```

Każdy bajt to wartość luminancji `0..255`. Paleta jest zapisywana posortowana
rosnąco (ułatwia analizę), a dekoder zwraca piksel `(Yk, Yk, Yk)`.

### 2.3. Paleta narzucona kolorowa (tryb 1) — odtwarzana po stronie dekodera

Wzorzec barw 4 × 4 × 2 (poziomy R × poziomy G × poziomy B):

| Składowa | Liczba poziomów | Wartości                       |
|----------|-----------------|--------------------------------|
| R        | 4               | `0, 85, 170, 255`              |
| G        | 4               | `0, 85, 170, 255`              |
| B        | 2               | `0, 255`                       |

Indeks `i` (0..31) jest wyznaczany jako `i = (R*4 + G)*2 + B`, gdzie `R,G ∈ {0,1,2,3}`
i `B ∈ {0,1}`.

### 2.4. Paleta narzucona szarości (tryb 2) — odtwarzana po stronie dekodera

Liniowy podział `0..255` na 32 poziomy:

```
Y_i = round(i * 255 / 31),    i = 0..31
```

---

## 3. Dane pikseli (5 bitów / piksel)

Piksele są zapisywane w kolejności **wierszami od góry do dołu**, w każdym
wierszu od lewej do prawej (kolejność rosnąca `y`, w niej rosnąca `x`).

Pojedynczy piksel to 5-bitowy indeks palety (`0..31`). Bity są pakowane do bajtów
w kolejności **MSB first**: pierwszy zapisywany bit lokuje się na pozycji 7 bajtu.
Po wstawieniu wszystkich pikseli ostatni niepełny bajt jest dopełniany zerami.

Schemat pakowania kolejnych 8 pikseli (`P0..P7`) w 5 bajtów:

```
bit:    7 6 5 4 3 | 2 1 0 7 6 | 5 4 3 2 1 | 0 7 6 5 4 | 3 2 1 0 7 | 6 5 4 3 2 | 1 0
piksel: P0 P0 P0 P0 P0 P1 P1 P1 P1 P1 P2 P2 P2 P2 P2 P3 P3 P3 P3 P3 P4 P4 P4 P4 P4 P5 P5 P5 P5 P5 P6 P6 P6 P6 P6 P7 P7 P7 P7 P7
```

`8 pikseli × 5 bitów = 40 bitów = 5 bajtów`, więc dla obrazu `W × H` przewidywany
rozmiar danych to `ceil(W * H * 5 / 8)` bajtów. Dla obrazka `640 × 400`:
`640 * 400 * 5 / 8 = 160 000 B`.

---

## 4. Filtracja danych wejściowych

Aplikacja wykonuje filtrację wejścia, gdy konwertuje BMP do `.gk26`:

1. Każdy piksel BMP zamieniany jest na (`R`, `G`, `B`) lub luminancję
   `Y = round(0.299 R + 0.587 G + 0.114 B)`.
2. **Bez ditheringu** — piksel jest przypisywany do indeksu palety o najmniejszej
   euklidesowej (kolor) lub bezwzględnej (szarość) odległości.
3. **Z ditheringiem Floyd-Steinberg** (`dithering = 0x01`) — dla każdego piksela
   wyznaczany jest najbliższy element palety, a różnica (`r − pal.r`, ...)
   rozdzielana jest na sąsiadów według wag:
   ```
                 *    7/16
       3/16    5/16    1/16
   ```
4. **Z ditheringiem uporządkowanym Bayer 4×4** (`dithering = 0x02`) — przed
   wyszukaniem najbliższego elementu palety do każdego piksela dodawany jest
   bias zależny od pozycji `(x, y)` w obrazie. Wykorzystywana jest klasyczna
   tablica progowa Bayera 4×4:
   ```
        0   8   2  10
       12   4  14   6
        3  11   1   9
       15   7  13   5
   ```
   Wartość biasu w danym kanale wynosi:
   ```
   bias = ((2 · M(x mod 4, y mod 4) - 15) · krok) / 32
   ```
   gdzie `krok = 256 / liczba_poziomów_kanału`. Dla narzuconej palety kolorowej
   2-2-1: `krokR = krokG = 64`, `krokB = 128`. Dla narzuconej palety szarości
   (32 poziomy): `krokY = 8`. Po dodaniu biasu wartość jest przycinana do
   `0..255` i klasyfikowana do najbliższego elementu palety bez dalszego
   rozprowadzania błędu.

Dla trybów dedykowanych paleta jest najpierw budowana algorytmem **median-cut**
(rekursywny podział „kubełków” pikseli wzdłuż osi o największej rozpiętości,
do uzyskania 32 grup; każda grupa daje jeden reprezentant — średnią arytmetyczną).
Dithering Bayer 4×4 jest udostępniony zgodnie z zadaniami projektowymi 3 i 4
dla **palet narzuconych** (kolorowej i szarości).

---

## 5. Konwersja `.gk26` → BMP

Dekoder czyta nagłówek, opcjonalną paletę oraz strumień bitów. Każde 5 bitów
zamieniane jest na indeks `0..31`, a następnie na barwę `(R, G, B)`:

- tryb 1: `(R, G, B) = paletaKolorNarzucona[i]`
- tryb 2: `(Y, Y, Y)` gdzie `Y = paletaSzaryNarzucona[i]`
- tryb 3: `(R, G, B) = paletaKolorDedykowana[i]`
- tryb 4: `(Y, Y, Y)` gdzie `Y = paletaSzaryDedykowana[i]`

Wynikowy obraz jest zapisywany jako 24-bitowy BMP (`SDL_PIXELFORMAT_RGB24`).

---

## 6. Interfejs aplikacji

Po uruchomieniu (`bin/Release/GK2026-Projekt.exe`) okno SDL akceptuje klawisze:

| Klawisze   | Działanie                                                                |
|------------|--------------------------------------------------------------------------|
| `a..l`     | Wybierz `obrazek1.bmp` … `obrazek9.bmp` jako bieżące źródło BMP          |
| `1`        | BMP → `.gk26`: paleta kolorowa narzucona, **bez** ditheringu             |
| `2`        | BMP → `.gk26`: paleta kolorowa narzucona, **z** ditheringiem             |
| `3`        | BMP → `.gk26`: szarości narzucone, **bez** ditheringu                    |
| `4`        | BMP → `.gk26`: szarości narzucone, **z** ditheringiem                    |
| `5`        | BMP → `.gk26`: paleta kolorowa dedykowana, **bez** ditheringu            |
| `6`        | BMP → `.gk26`: paleta kolorowa dedykowana, **z** ditheringiem            |
| `7`        | BMP → `.gk26`: szarości dedykowane, **bez** ditheringu                   |
| `8`        | BMP → `.gk26`: szarości dedykowane, **z** ditheringiem (Floyd-Steinberg) |
| `9`        | `.gk26` → `obrazek_out.bmp` i wyświetlenie podglądu                      |
| `q`        | **Zad. 4** — paleta kolorowa narzucona + Bayer 4×4 (uporządkowany)       |
| `w`        | **Zad. 3** — szarości narzucone + Bayer 4×4 (uporządkowany)              |
| `p`        | Podgląd palety kolorowej narzuconej (siatka 8×4 bloków 60×60)            |
| `o`        | Podgląd palety szarości narzuconej                                       |
| `i`        | Podgląd palety kolorowej dedykowanej (po wykonaniu konwersji `5`/`6`)    |
| `u`        | Podgląd palety szarości dedykowanej (po wykonaniu konwersji `7`/`8`)     |
| `b`        | Wyczyść ekran                                                            |
| `ESC`      | Wyjście                                                                  |

Po naciśnięciu klawisza `1..8` aplikacja zapisuje plik wynikowy `obrazek.gk26`
oraz natychmiast wczytuje go ponownie i wyświetla, dzięki czemu od razu widać,
jak wygląda obraz po filtracji do nowego formatu. Klawisz `9` wykonuje konwersję
zwrotną do `obrazek_out.bmp`.

---

## 7. Przykład — nagłówek dla obrazu 640 × 400, tryb 3, dithering

```
Offset  Hex                                      Komentarz
0x00    47 4B 32 36                              "GK26"
0x04    01                                       wersja = 1
0x05    03                                       tryb  = paleta kolor dedykowana
0x06    01                                       dithering = wlaczone
0x07    05                                       bpp = 5
0x08    80 02                                    szerokosc = 0x0280 = 640
0x0A    90 01                                    wysokosc  = 0x0190 = 400
0x0C    00 70 02 00                              dataSize  = 0x00027000 = 160000
0x10    R0 G0 B0 R1 G1 B1 ...                    96 B palety dedykowanej
0x70    [160 000 B spakowanych pikseli 5-bit]
```

Łączny rozmiar pliku w trybie 3: `16 + 96 + 160 000 = 160 112 B`.
W trybie 4: `16 + 32 + 160 000 = 160 048 B`.
W trybach 1 i 2: `16 + 0 + 160 000 = 160 016 B`.

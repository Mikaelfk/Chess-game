# Piece artwork

The Cburnett chess pieces are by **Colin M. Burnett**, licensed under **GNU GPL
version 2 or later**, independently of the application code. See `LICENSE`.

Original SVGs were obtained from Lichess's `public/piece/cburnett` directory at
commit `5b44d719ec5913dd62aa965f4b8d21e60a00c2c1`:
<https://github.com/lichess-org/lila/tree/5b44d719ec5913dd62aa965f4b8d21e60a00c2c1/public/piece/cburnett>.
Lichess attributes this directory and its GPLv2+ license in `COPYING.md`.

The unmodified vector sources are in `svg/`. PNGs are derived by rendering at
1024 pixels, then downsampling with antialiasing to 32, 64, 128, 256 and 512 pixels.
The application chooses the closest size at least as large as the physical draw
size, and uses SDL linear filtering. This avoids both enlarging the former
60-pixel images and undersampling a huge image for small capture icons.

To regenerate (development only):

```sh
python -m pip install CairoSVG==2.8.2 Pillow
python tools/render-pieces.py
```

All PNGs are checked in and embedded by CMake. Building or running the application
requires neither SVG rendering nor Python image packages. Desktop packages include
the original SVG sources, attribution and license; Android packages include the
same notices and sources in their assets.

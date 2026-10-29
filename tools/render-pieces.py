"""Regenerate checked-in, antialiased piece PNGs from the original SVG sources.

Development only: pip install CairoSVG==2.8.2 Pillow
Normal application builds use the checked-in PNGs and need no Python packages.
"""
from pathlib import Path
from io import BytesIO
import cairosvg
from PIL import Image

root = Path(__file__).resolve().parents[1] / 'Assets' / 'Pieces'
for side, prefix in [('White', 'w'), ('Black', 'b')]:
    for piece, letter in zip(['Pawn', 'Knight', 'Bishop', 'Rook', 'Queen', 'King'], 'PNBRQK'):
        rendered = cairosvg.svg2png(url=str(root / 'svg' / f'{prefix}{letter}.svg'), output_width=1024, output_height=1024)
        source = Image.open(BytesIO(rendered))
        for size in [32, 64, 128, 256, 512]:
            directory = root if size == 512 else root / 'mip' / str(size)
            directory.mkdir(parents=True, exist_ok=True)
            image = source.resize((size, size), Image.Resampling.LANCZOS)
            image.save(directory / f'{side} {piece}.png', optimize=True)

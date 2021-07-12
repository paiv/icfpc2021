#!/usr/bin/env python
import json
from pathlib import Path


def minmax_coordinates(path):
    minx = miny = 0x800000
    maxx = maxy = -0x800000
    for fn in path.glob('*.json'):
        with open(fn) as fp:
            obj = json.load(fp)
        hole = obj.get('hole', [])
        figure = obj.get('figure', {}).get('vertices', [])
        for [x, y] in (hole + figure):
            minx, maxx = min(minx, x), max(maxx, x)
            miny, maxy = min(miny, y), max(maxy, y)
    print(f'coordinate range: x={minx}:{maxx}, y={miny}:{maxy}')


def max_vertices(path):
    maxhole = maxfig = -0x800000
    for fn in path.glob('*.json'):
        with open(fn) as fp:
            obj = json.load(fp)
        hole = obj.get('hole', [])
        maxhole = max(maxhole, len(hole))
        figure = obj.get('figure', {}).get('vertices', [])
        maxfig = max(maxfig, len(figure))
    print('max vertices count')
    print('  hole', maxhole)
    print('  fig', maxfig)


def main():
    path = Path(__file__).with_name('spec') / 'problems'
    minmax_coordinates(path)
    max_vertices(path)


if __name__ == '__main__':
    main()

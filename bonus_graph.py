#!/usr/bin/env python
import json
from pathlib import Path


def scan_bonuses(path):
    for fn in path.glob('*.json'):
        with open(fn) as fp:
            obj = json.load(fp)
        pid = int(fn.stem)
        for bonus in obj.get('bonuses', []):
            bid = bonus['problem']
            t = bonus['bonus']
            yield (pid, bid, t)


def main():
    path = Path(__file__).with_name('spec') / 'problems'
    print('digraph {')
    for a, b, t in scan_bonuses(path):
        print(f'p{a} -> p{b} [label="{t}"];')
    print('}')


if __name__ == '__main__':
    main()

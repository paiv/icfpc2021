#!/usr/bin/env python
import json
import struct
import subprocess
from pathlib import Path


Problems = Path(__file__).with_name('spec') / 'problems'
Solutions = Path(__file__).with_name('solutions')
Solver = Path(__file__).with_name('solver') / 'solver'


def solve(n):
    with open(Problems / f'{n}.json') as fp:
        prob = json.load(fp)

    fn = Solutions / f'{n}.json'
    soa = None
    if fn.is_file():
        with open(fn) as fp:
            soa = json.load(fp)

    prob = encode_problem(prob, soa)
    sol = run_solver(prob)
    sol = decode_solution(sol)

    if sol:
        with open(Solutions / f'{n}.json', 'w') as fp:
            json.dump(sol, fp)


def encode_problem(problem, soa):
    eol = struct.pack('<ii', -0x80000000, -0x80000000)
    so = b''
    so += b''.join(struct.pack('<ii', *p) for p in problem['hole']) + eol
    so += b''.join(struct.pack('<ii', *p) for p in problem['figure']['vertices']) + eol
    so += b''.join(struct.pack('<ii', *p) for p in problem['figure']['edges']) + eol
    if soa:
        so += b''.join(struct.pack('<ii', *p) for p in soa['vertices']) + eol
    so += struct.pack('<ii', problem['epsilon'], -0x80000000)
    return so


def decode_solution(solution):
    n = len(solution) // 4
    sol = struct.unpack(f'<{n}I', solution)
    if sol:
        return {'vertices': list(zip(sol[0::2], sol[1::2]))}


def run_solver(problem):
    p = subprocess.run([str(Solver)], input=problem, stdout=subprocess.PIPE, check=True)
    return p.stdout


def main(start, stop=None):
    if stop is None:
        stop = start + 1
    for n in range(start, stop):
        solve(n)


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Solver')
    parser.add_argument('start', metavar='N', type=int, help='Problem number (start of range)')
    parser.add_argument('stop', metavar='M', type=int, nargs='?', help='Problem number (end of range)')
    args = parser.parse_args()
    main(args.start, args.stop)

#!/usr/bin/env python
import json
import struct
import subprocess
import time
from pathlib import Path


Problems = Path(__file__).with_name('spec') / 'problems'
Solutions = Path(__file__).with_name('solutions')
Solver = Path(__file__).with_name('solver') / 'solver'


def solve(n, timeout=None):
    with open(Problems / f'{n}.json') as fp:
        prob = json.load(fp)

    fn = Solutions / f'{n}.json'
    soa = None
    if fn.is_file():
        with open(fn) as fp:
            soa = json.load(fp)

    start = time.monotonic()
    while True:
        timeleft = (timeout - (time.monotonic() - start)) if timeout else None

        pro = encode_problem(prob, soa)
        sol = run_solver(pro, timeout=timeleft)
        if not sol: break
        soa = decode_solution(sol)

        with open(Solutions / f'{n}.json', 'w') as fp:
            json.dump(soa, fp)

        if timeout is None: break


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


def run_solver(problem, timeout=None):
    try:
        p = subprocess.run([str(Solver)], input=problem, stdout=subprocess.PIPE, check=True, timeout=timeout)
        return p.stdout
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
        pass


def main(start, stop=None, timeout=None):
    if stop is None:
        stop = start + 1
    for n in range(start, stop):
        solve(n, timeout=timeout)


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Solver')
    parser.add_argument('start', metavar='N', type=int, help='Problem number (start of range)')
    parser.add_argument('stop', metavar='M', type=int, nargs='?', help='Problem number (end of range)')
    parser.add_argument('--timeout', '-t', metavar='T', type=float, help='Time limit, in seconds')
    args = parser.parse_args()
    main(args.start, args.stop, timeout=args.timeout)

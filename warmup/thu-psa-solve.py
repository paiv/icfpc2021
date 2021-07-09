#!/usr/bin/env python

code = '''
icfpcontest
vbtib.lafvj
KE7ZFnZQ59D
'''.strip()

def dump_table(zs):
    for ps in zs:
        for x in ps:
            print(chr(x), end=' ')
        for x in ps:
            print(f'{x:03}', end=' ')
        for x in ps:
            print(f'{x:x}', end=' ')
        for x in ps:
            print(f'{x:08b}', end=' ')
        print()

xs = code.splitlines()
# dump_table(zip(*map(str.encode, xs)))

# ks = sorted(zip(xs[0].encode(), xs[1].encode()))
# dump_table(ks)

a,b,c = zip(*map(str.encode, xs))
print(a)

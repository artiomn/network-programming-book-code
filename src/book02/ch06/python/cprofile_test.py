#!/usr/bin/env python3

import cProfile
import io
import pstats


pr = cProfile.Profile()
pr.enable()

for i in range(10000):
    if 0 == i % 1000:
        print(f'I = {i}')

pr.disable()
with io.StringIO() as s:
    ps = pstats.Stats(pr, stream=s).sort_stats(pstats.SortKey.CUMULATIVE)
    ps.print_stats()
    print(s.getvalue())

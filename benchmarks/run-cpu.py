#!/usr/bin/env python

import signal
import subprocess
import sys
import yaml


def handle_signal(num, frame):
    global min_elapsed
    global max_gflops

    print('Premature ABORT!')
    print('Min Elapsed:  %f' % min_elapsed)
    print('Max GFlops:   %f' % max_gflops)



dims = int(sys.argv[1])
name = sys.argv[2]

configs = []

low = 4
high = 32
by = 4


# Time tile size
configs = [[x] for x in range(1, 16+1, 1)]

for d in range(1, dims+1):
    configs = [x+[y] for x in configs for y in range(low, high+1, by)]




num = 1

min_elapsed = 100000.0
max_gflops = 0.0


signal.signal(signal.SIGABRT, handle_signal)

try:

    for c in configs:

        print('Running %d of %d' % (num, len(configs)))
        num = num + 1

        # Build
        args = 'icc -std=c99 -fast -openmp ' + name + '-cpu.pocc.c -o /tmp/' + name + '-cpu.pocc'

        cnt = 0
        for size in c:
            args = args + ' -DT1c%d=%.1f' % (cnt, size)
            cnt = cnt + 1

        print(args)

        proc = subprocess.Popen(args, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (stdout, stderr) = proc.communicate()
        if proc.returncode != 0:
            print(stderr)
            exit(1)

        # Run
        args = '/tmp/' + name + '-cpu.pocc'
        proc = subprocess.Popen(args, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (stdout, _) = proc.communicate()
        assert(proc.returncode == 0)

        doc = yaml.load(stdout)
        elapsed = float(doc['Elapsed'])
        gflops = float(doc['GFlops'])

        print(stdout)

        min_elapsed = min(min_elapsed, elapsed)
        max_gflops = max(max_gflops, gflops)

        sys.stdout.flush()
except:
    print('Premature ABORT')

print('Min Elapsed:  %f' % min_elapsed)
print('Max GFlops:   %f' % max_gflops)

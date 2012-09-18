#!/usr/bin/env python

import os
import subprocess
import sys


runs = 0
success = 0
fail = []


test_dir = os.path.dirname(os.path.abspath(__file__))
build_dir = os.path.join(test_dir, 'build.temp')


def run_cuda_test(source):
    global runs, success, fail
    global build_dir, test_dir

    otsc_out = os.path.join(build_dir, 'otsc.out.cu')
    nvcc_out = os.path.join(build_dir, 'nvcc.out')

    runs = runs + 1
    ret = subprocess.call('otsc -c %s -o %s' % (source, otsc_out),
                          shell=True)
    if ret != 0:
        fail.append(source)
        return

    ret = subprocess.call('nvcc -Xptxas -v -arch sm_20 -O3 %s -o %s -I%s' % (otsc_out, nvcc_out, os.path.join(test_dir)),
                          shell=True)
    if ret != 0:
        fail.append(source)
        return

    ret = subprocess.call(nvcc_out)
    if ret != 0:
        fail.append(source)
        return

    success = success + 1

try:
    os.mkdir(build_dir)
except:
    pass



# CUDA tests
cuda_dir = os.path.join(test_dir, 'cuda')
for (_, _, files) in os.walk(cuda_dir):
    for f in files:

        # Apply filter
        if len(sys.argv) == 2:
            idx = f.find(sys.argv[1])
            if idx == -1:
                continue

        print('Running "%s"' % f)
        run_cuda_test(os.path.join(cuda_dir, f))


print('\n\nResults:')
print('Success:  %d' % success)
print('Failure:  %d' % len(fail))

if len(fail) > 0:
    print('Failing tests: %s' % str(fail))

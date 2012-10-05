#!/usr/bin/env python

import subprocess
import sys
import time
import yaml

configs = []

dim = int(sys.argv[1])
src = sys.argv[2]
arch = sys.argv[3]


time_steps = 1000

if dim == 2:
    x_range = range(32, 128+1, 32)
    y_range = range(4, 8+1, 2)
    z_range = [1]
    t_range = range(1, 6+1, 1)
    #ex_range = range(1, 12+1, 1)
    ex_range = [1]
    ey_range = range(1, 12+1, 1)
    ez_range = [1]

    problem_size = 6000
    gstencils_factor = float(problem_size)*float(problem_size)*float(time_steps)/1e9
elif dim == 3:
    x_range = range(8, 16+1, 4)
    y_range = range(4, 8+1, 2)
    z_range = range(4, 8+1, 2)
    t_range = range(1, 3+1, 1)
    #ex_range = range(1, 12+1, 1)
    ex_range = [1]
    ey_range = range(1, 3+1, 1)
    ez_range = range(1, 3+1, 1)

    problem_size = 500
    gstencils_factor = float(problem_size)*float(problem_size)*float(problem_size)*float(time_steps)/1e9
else:
    print('Unknown dimensionality!')
    exit(1)

configs = [[x, y, z, t, ex, ey, ez] for x in x_range for y in y_range for z in z_range for t in t_range for ex in ex_range for ey in ey_range for ez in ez_range]

sys.stderr.write('Num Configs: %d\n' % len(configs))
sys.stderr.flush()

curr = 1

sys.stdout.write('x,y,z,t,ex,ey,ez,gstencils,cpu_elapsed,compute_elapsed,\n')
sys.stdout.flush()

results = []

for c in configs:
    (x, y, z, t, ex, ey, ez) = c
    sys.stderr.write('Running %s (%d of %d)\n' % (str(c), curr, len(configs)))
    sys.stderr.flush()
    curr = curr + 1

    attrs = 'block:%d,%d,%d time:%d tile:%d,%d,%d' % (x, y, z, t, ex, ey, ez)

    # Create in-memory copy of result code
    with open(src, 'r') as src_in:
        src_text = src_in.read()
        src_text = src_text.replace('TILE_SIZE_PARAMS', attrs)

    # Write copy to disk
    with open('/tmp/overtile-search.cu', 'w') as src_out:
        src_out.write(src_text)

    # Run otsc
    ret = subprocess.call('otsc -c /tmp/overtile-search.cu -o /tmp/overtile-search.out.cu',
                          shell=True, stdout=sys.stderr, stderr=sys.stderr)

    if ret != 0:
        sys.stderr.write('Run error!\n')
        sys.stderr.flush()
        continue


    # Run nvcc
    ret = subprocess.call('nvcc -Xptxas -v -O3 -arch %s /tmp/overtile-search.out.cu -o /tmp/overtile-search.x -DPROBLEM_SIZE=%d -DTIME_STEPS=%d' % (arch, problem_size, time_steps),
                          shell=True, stdout=sys.stderr, stderr=sys.stderr)

    if ret != 0:
        sys.stdout.write('Run error!\n')
        sys.stdout.flush()
        continue

    # Run
    proc = subprocess.Popen('/tmp/overtile-search.x', shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    start_time = time.time()
    while proc.poll() == None:
        time.sleep(0.1)
        now = time.time()
        elapsed = now - start_time
        if elapsed > 60.0:
            sys.stderr.write('Watchdog timer expired!\n')
            sys.stderr.flush()
            proc.terminate()
            proc.wait()
            break
    end_time = time.time()

    if proc.returncode != 0:
        sys.stderr.write('Run error!\n')
        sys.stderr.flush()
        continue

    stdout = proc.stdout.read()
    stdout = stdout + "\n" + proc.stderr.read()

    try:
        doc = yaml.load(stdout)
        gstencils = float(doc['GStencils/sec'])
        cpu_elapsed = float(doc['CPU Elapsed'])
        elapsed = float(doc['Elapsed'])

        sys.stdout.write('%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,\n' % (x, y, z, t, ex, ey, ez, gstencils, cpu_elapsed, elapsed))
        sys.stdout.flush()

        results.append([x, y, z, t, ex, ey, ez, gstencils, cpu_elapsed, elapsed])
    except:
        sys.stderr.write('Run error!\n')

    sys.stdout.flush()
    sys.stderr.flush()


elapsed = [x for (_,_,_,_,_,_,_,_,_,x) in results]
min_elapsed = min(elapsed)

cpu_elapsed = [x for (_,_,_,_,_,_,_,_,x,_) in results]
min_cpu_elapsed = min(cpu_elapsed)

sys.stderr.write('Max Total GStencils/sec: %f\n' % (gstencils_factor / min_cpu_elapsed))
sys.stderr.write('Max Compute GStencils/sec: %f\n' % (gstencils_factor / min_elapsed))

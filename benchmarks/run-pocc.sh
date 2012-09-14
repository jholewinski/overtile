#!/bin/sh

BENCHMARKS="j1d j2d p2d j3d g2d"

for bench in $BENCHMARKS; do
    pocc --pluto --ptile --ptile-fts --vectorizer --pragmatizer --past-hoist-lb $bench-cpu.c

    # -d doesn't seem to work :(
    rm -f .vectorize .unroll __tmp .body.c .head .tail
done



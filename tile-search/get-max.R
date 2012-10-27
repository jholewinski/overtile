#!/usr/bin/env Rscript

require('utils')

a <- read.csv(file("stdin"))

a <- subset(a, compute_elapsed > 0.01)
a <- data.frame(a, gstencils_compute_2d=6000*6000*1000/1e9/a$compute_elapsed, gstencils_compute_3d=400*400*400*1000/1e9/a$compute_elapsed)

cat('GStencils/sec Total: ')
cat(max(a$gstencils))
cat('\n')

cat('GStencil/sec Compute-Only (if 2D): ')
cat(max(a$gstencils_compute_2d))
cat('\n')

cat('GStencil/sec Compute-Only (if 3D): ')
cat(max(a$gstencils_compute_3d))
cat('\n')


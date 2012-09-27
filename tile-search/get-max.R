#!/usr/bin/env Rscript

require('utils')

a <- read.csv(file("stdin"))
a <- subset(a, compute_elapsed > 0.01)
print(max(a$gstencils))


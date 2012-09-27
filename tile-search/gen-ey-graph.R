#!/usr/bin/env Rscript

require('utils')

a <- read.csv(file("stdin"))
a <- subset(a, compute_elapsed > 0.01)


# 32x8, 2
cat('32x8, 2:\n')
t <- subset(a, x==32 & y==8 & t==2)
t <- t[with(t, order(ey)),]

for(i in 1:nrow(t)) {
  row <- t[i,]
  cat(row$gstencils)
  cat(',\n')
}

# 32x8, 6
cat('32x8, 6:\n')
t <- subset(a, x==32 & y==8 & t==6)
t <- t[with(t, order(ey)),]

for(i in 1:nrow(t)) {
  row <- t[i,]
  cat(row$gstencils)
  cat(',\n')
}


# 64x8, 2
cat('64x8, 2:\n')
t <- subset(a, x==64 & y==8 & t==2)
t <- t[with(t, order(ey)),]

for(i in 1:nrow(t)) {
  row <- t[i,]
  cat(row$gstencils)
  cat(',\n')
}

# 64x8, 6
cat('64x8, 6:\n')
t <- subset(a, x==64 & y==8 & t==6)
t <- t[with(t, order(ey)),]

for(i in 1:nrow(t)) {
  row <- t[i,]
  cat(row$gstencils)
  cat(',\n')
}



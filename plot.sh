#! /bin/sh

gnuplot << EOF
set terminal png small color
set output "leonids60.png"
plot "leo60" with boxes
set output "leonids120.png"
plot "leo120" with lines
set output "leonids240.png"
plot "leo240" with impulses
EOF
# Other useful styles are steps and linespoints

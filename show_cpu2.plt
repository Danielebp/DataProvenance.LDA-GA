set term png small size 400,300 font "./LiberationMono-Bold.ttf, 10"
set output "cls-cpu-graph.png"

set ylabel "Resource Usage (%)"
#set y2label "Memory Usage (%)"
set xlabel "Elapsed Time (Hrs)"

set ytics nomirror
#set y2tics nomirror in

set yrange [0:100]
#set y2range [0:*]

set grid ytics lc rgb "#bbbbbb" lw 1 lt 0

plot "cls-mem-warp.log" using ($0/3600):($5/48) with lines lw 3 axes x1y1 title "CPU", \
    "cls-mem-warp.log" using ($0/3600):($2) with lines lw 3 axes x1y1 title "Memory"



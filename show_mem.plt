set term png small size 400,300 font "./LiberationMono-Bold.ttf, 10"
set output "mem-graph.png"

set ylabel "Memory Usage (GB)"
set xlabel "Elapsed Time (Hrs)"
#set y2label "RSS"
#set y2label "%MEM"

set ytics nomirror 
#set y2tics nomirror in

set yrange [0:*]
#set y2range [0:*]

set key right center #outside
set grid ytics lc rgb "#bbbbbb" lw 1 lt 0

plot "mem-warp.log" using ($0/3600):($3/1048576) with lines lw 3 axes x1y1 title "Virtual", \
    "mem-warp.log" using ($0/3600):($4/1048576) with lines lw 3 axes x1y1 title "Resident"
   # "mem-java.log" using 2 with lines axes x1y2 title "%MEM"


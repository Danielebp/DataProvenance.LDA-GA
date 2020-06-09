while true; do
    ps -C provenance -o pid=,%mem=,vsz=,rss=,%cpu= >> mem-warp.log
    gnuplot show_mem.plt
    gnuplot show_cpu.plt
    sleep 1
done 


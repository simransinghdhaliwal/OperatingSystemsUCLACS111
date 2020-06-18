#! /usr/bin/gnuplot
#NAME: Simran Dhaliwal
#EMAIL: sdhaliwal@ucla.edu
#ID: 905361068

# general plot parameters
set terminal png
set datafile separator ","
	
# lab2b_1.png
set title "Throughput vs. Number of Threads for Sync Methods"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Operations per Second"
set logscale y 10
set output 'lab2b_1.png'
plot \
    "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title 'Spin-Lock' with linespoints lc rgb 'red', \
    "< grep 'list-none-s,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title 'Mutex' with linespoints lc rgb 'blue'

# lab2b_2.png
set title "Mean Time per Mutex and Mean Time per Operation for M Sync"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Time(ns)"
set logscale y 10
set output 'lab2b_2.png'
plot \
    "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):($7) \
	    title 'Avg. Time Per Op.' with linespoints lc rgb 'orange', \
    "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):($8) \
	    title 'Wait-for-lock' with linespoints lc rgb 'blue'
	
# lab2b_3.png
set title "Sucessful Iterations vs. Threads For Each Sync Methods"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-s,.*,.*,4,' lab2b_list.csv" using ($2):($3) \
    	title 'Spin-Lock' with points lc rgb 'red', \
    "< grep 'list-id-m,.*,.*,4,' lab2b_list.csv" using ($2):($3) \
	    title 'Mutex' with points lc rgb 'blue', \
    "< grep 'list-id-none,.*,.*,4,' lab2b_list.csv" using ($2):($3) \
	    title 'Unprotected' with points lc rgb 'orange' 

# lab2b_4.png
set title "Throughput vs. Number Of Threads for Mutex Partitioned Lists"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Operations per Second"
set logscale y 10
set output 'lab2b_4.png'
plot \
    "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '1 List' with linespoints lc rgb 'red', \
    "< grep 'list-none-m,.*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '4 Lists' with linespoints lc rgb 'orange', \
    "< grep 'list-none-m,.*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '8 List' with linespoints lc rgb 'blue', \
    "< grep 'list-none-m,.*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '16 List' with linespoints lc rgb 'green'

# lab2b_5.png
set title "Throughput vs. Number Of Threads for Spin Partitioned Lists"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Operations per Second"
set logscale y 10
set output 'lab2b_5.png'
plot \
    "< grep 'list-none-s,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '1 List' with linespoints lc rgb 'red', \
    "< grep 'list-none-s,.*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '4 Lists' with linespoints lc rgb 'orange', \
    "< grep 'list-none-s,.*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '8 List' with linespoints lc rgb 'blue', \
    "< grep 'list-none-s,.*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	    title '16 List' with linespoints lc rgb 'green'
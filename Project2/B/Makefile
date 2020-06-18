#NAME: Simran Dhaliwal
#EMAIL: sdhaliwal@ucla.edu
#ID: 905361068

.SILENT:

default: build

graphs:
	./list.gp

build:
	gcc -Wall -Wextra -lpthread -lprofiler -g  -o lab2_list lab2_list.c SortedList.c

dist: build tests graphs 
	tar -czvf lab2b-905361068.tar.gz *.c *.gp *.csv *.png \
	SortedList.h README Makefile *.out 

profile:
	rm -f ./raw.gperf profile.out
	LD_PRELOAD=/u/cs/ugrad/dhaliwal/lib/libprofiler.so.0
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=list_helper ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

clean:
	rm -f *.tar.gz lab2_list

tests:
	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv

	-./lab2_list  --yield=id --threads=1  --iterations=1  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=2  --iterations=1  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=4  --iterations=1  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=8  --iterations=1  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=12 --iterations=1  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=16 --iterations=1  --lists=4 >> lab2b_list.csv

	-./lab2_list  --yield=id --threads=1  --iterations=2  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=2  --iterations=2  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=4  --iterations=2  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=8  --iterations=2  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=12 --iterations=2  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=16 --iterations=2  --lists=4 >> lab2b_list.csv

	-./lab2_list  --yield=id --threads=1  --iterations=4  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=2  --iterations=4  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=4  --iterations=4  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=8  --iterations=4  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=12 --iterations=4  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=16 --iterations=4  --lists=4 >> lab2b_list.csv

	-./lab2_list  --yield=id --threads=1  --iterations=8  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=2  --iterations=8  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=4  --iterations=8  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=8  --iterations=8  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=12 --iterations=8  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=16 --iterations=8  --lists=4 >> lab2b_list.csv

	-./lab2_list  --yield=id --threads=1  --iterations=16  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=2  --iterations=16  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=4  --iterations=16  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=8  --iterations=16  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=12 --iterations=16  --lists=4 >> lab2b_list.csv
	-./lab2_list  --yield=id --threads=16 --iterations=16  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=m --yield=id --threads=1  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=2  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=4  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=8  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=12 --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=16 --iterations=10  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=m --yield=id --threads=1  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=2  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=4  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=8  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=12 --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=16 --iterations=20  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=m --yield=id --threads=1  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=2  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=4  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=8  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=12 --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=16 --iterations=40  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=m --yield=id --threads=1  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=2  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=4  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=8  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=12 --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --yield=id --threads=16 --iterations=80  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=s --yield=id --threads=1  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=2  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=4  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=8  --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=12 --iterations=10  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=16 --iterations=10  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=s --yield=id --threads=1  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=2  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=4  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=8  --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=12 --iterations=20  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=16 --iterations=20  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=s --yield=id --threads=1  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=2  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=4  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=8  --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=12 --iterations=40  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=16 --iterations=40  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=s --yield=id --threads=1  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=2  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=4  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=8  --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=12 --iterations=80  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --yield=id --threads=16 --iterations=80  --lists=4 >> lab2b_list.csv

	./lab2_list --sync=m --threads=1  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --threads=2  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --threads=4  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --threads=8  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=m --threads=12  --iterations=1000  --lists=1 >> lab2b_list.csv


	./lab2_list --sync=m --threads=1  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --threads=2  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --threads=4  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --threads=8  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=m --threads=12  --iterations=1000  --lists=4 >> lab2b_list.csv


	./lab2_list --sync=m --threads=1  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --threads=2  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --threads=4  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --threads=8  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=m --threads=12  --iterations=1000  --lists=8 >> lab2b_list.csv


	./lab2_list --sync=m --threads=1  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --threads=2  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --threads=4  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --threads=8  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=m --threads=12  --iterations=1000  --lists=16 >> lab2b_list.csv


	./lab2_list --sync=s --threads=1  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --threads=2  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --threads=4  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --threads=8  --iterations=1000  --lists=1 >> lab2b_list.csv
	./lab2_list --sync=s --threads=12  --iterations=1000  --lists=1 >> lab2b_list.csv


	./lab2_list --sync=s --threads=1  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --threads=2  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --threads=4  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --threads=8  --iterations=1000  --lists=4 >> lab2b_list.csv
	./lab2_list --sync=s --threads=12  --iterations=1000  --lists=4 >> lab2b_list.csv


	./lab2_list --sync=s --threads=1  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --threads=2  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --threads=4  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --threads=8  --iterations=1000  --lists=8 >> lab2b_list.csv
	./lab2_list --sync=s --threads=12  --iterations=1000  --lists=8 >> lab2b_list.csv


	./lab2_list --sync=s --threads=1  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --threads=2  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --threads=4  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --threads=8  --iterations=1000  --lists=16 >> lab2b_list.csv
	./lab2_list --sync=s --threads=12  --iterations=1000  --lists=16 >> lab2b_list.csv

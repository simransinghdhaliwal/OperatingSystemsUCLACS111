#NAME: Simran Dhaliwal
#EMAIL: sdhaliwal@ucla.edu
#ID: 905361068


.SILENT:



default:
	gcc -Wall -Wextra lab1a.c -o lab1a

dist: lab1a.c Makefile README
	tar -czvf lab1a-905361068.tar.gz lab1a.c Makefile README

clean:
	rm -f lab1a
	rm -f *.tar.gz

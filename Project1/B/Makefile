#NAME: Simran Dhaliwal
#EMAIL: sdhaliwal@ucla.edu
#ID: 905361068



.SILENT:

default:
	gcc -Wall -lz -Wextra lab1b-client.c -o lab1b-client
	gcc -Wall -lz -Wextra lab1b-server.c -o lab1b-server

dist: lab1b-client.c lab1b-server.c README Makefile
	tar -czvf lab1b-905361068.tar.gz lab1b-client.c lab1b-server.c README Makefile

clean:
	rm -f lab1b-client lab1b-server *.tar.gz

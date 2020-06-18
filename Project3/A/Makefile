# NAME: Simran Dhaliwal
# ID: 905361068
# EMAIL: sdhaliwal@ucla.edu

# NAME: Jason Lai
# ID: 305426666
# EMAIL: jasonyslai@g.ucla.edu

CC = gcc
CFLAGS = -g3 -std=gnu99 -Wall -Wextra
TARGETS = lab3a

default: $(TARGETS)

lab3a: lab3a.c
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	rm -f lab3a *.tar.gz

dist_files = lab3a.c ext2_fs.h Makefile README
dist: $(dist_files)
	tar -czf lab3a-905361068.tar.gz $(dist_files)

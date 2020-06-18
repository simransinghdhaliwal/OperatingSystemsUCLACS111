# NAME: Simran Dhaliwal
# ID: 905361068
# EMAIL: sdhaliwal@ucla.edu
#
# NAME: Jason Lai
# ID: 305426666
# EMAIL: jasonyslai@g.ucla.edu

default: lab3b.sh
	ln lab3b.sh lab3b
	chmod 777 lab3b

clean:
	rm -f lab3b *.tar.gz

dist_files = lab3b.sh lab3b.py Makefile README
dist: $(dist_files)
	tar -czf lab3b-905361068.tar.gz $(dist_files)

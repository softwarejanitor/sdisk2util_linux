CC = gcc

TARGETS = dsk2nic nic2dsk

all: $(TARGETS)

dsk2nic: dsk2nic.c
	${CC} -o $@ dsk2nic.c

nic2dsk: nic2dsk.c
	${CC} -o $@ nic2dsk.c

clean:
	rm -f *.o $(TARGETS)


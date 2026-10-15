# Tomb Raider Level Modifier (TRMOD)
# b122251

# Program variables
PROGNAME=trmod
SRCFILES=command.c conv.c errors.c main.c osdep.c structs.c tr1pc.c tr1psx.c tr1ss.c tr2pc.c tr2psx.c tr3pc.c tr3psx.c trmodio.c util.c wrap.c
OBJFILES=$(SRCFILES:.c=.o)
# Basic compiler flags
CFLAGS=--static -O3
LFLAGS=-s

all:
	cc -c $(SRCFILES) $(CFLAGS)
	cc $(OBJFILES) $(LFLAGS) -o $(PROGNAME)
	rm -f $(OBJFILES)

clean:
	rm -f $(OBJFILES) $(PROGNAME)

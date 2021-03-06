
PROGRAM = gpsdiff

CSOURCES = main.c
COBJECTS = $(CSOURCES:.c=.o)
CFLAGS   = $(COPT) $(CDEF) $(CINC)
COPT     = -g
CDEF     = 
CINC     = 

LDFLAGS  = $(LDOPT) $(LDDIR) $(LDLIB)
LDOPT    = 
LDDIR    = 
LDLIB    = -lgps -lm

all: $(PROGRAM)

$(PROGRAM): $(COBJECTS)
	$(CC) -o $@ $(COBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

run: $(PROGRAM)
	./$< rpgx1 rpgx2

clean: objclean execlean
objclean:
	$(RM) $(COBJECTS)
execlean:
	$(RM) $(PROGRAM)


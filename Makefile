

# 1. Uncomment to link with the appropiate utils
# Warning: do not add tailing spaces
#CP_UTILS:=seq_omp
CP_UTILS:=mpi

# Comment this if you don't have Xlib
#CP_DISPLAY:=yes


# 2. Include the utils Makefile
include ../../utils/Makefile.inc

# 3. Configure the compiler and flags
# CP_FLAGS is necessary to include the utils
CC:= mpicc
CFLAGS:= -O1 -Wall $(CP_CFLAGS)
CFLAGS_DEBUG:= -g -O0 -Wall $(CP_CFLAGS)

# 4. Rules for practica, practica_display and debug
all: $(CP_TARGET)
	$(CC) $(CFLAGS) practica.c -o practica 

display: $(CP_TARGET)
	$(CC) $(CFLAGS) practica.c -o practica_display -DSHOW_DISPLAY 

debug: $(CP_TARGET)
	$(CC) $(CFLAGS_DEBUG) practica.c -o practica 

# 5. Clean all
clean:
	rm -f practica practica_display *.exe *.debug *~


# 6. Rules for compiling all student programs
PRACTICAS_SRCS=$(wildcard *.c)
PRACTICAS_EXES=$(PRACTICAS_SRCS:.c=.exe)
PRACTICAS_BUGS=$(PRACTICAS_SRCS:.c=.debug)

allpracticas: $(PRACTICAS_EXES) $(PRACTICAS_BUGS)

%.exe: %.c
	$(CC) $(CFLAGS) $< -o $@

%.debug: %.c
	$(CC) $(CFLAGS_DEBUG) $< -o $@


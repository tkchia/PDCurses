# GNU MAKE (4.1) Makefile for PDCurses library - ia16-elf-gcc for 16-bit DOS
#
# Usage: make -f [path\]gccdos16.mak [DEBUG=Y] [target]
#
# where target can be any of:
# [all|libs|demos|dist|pdcurses.a|testcurs.exe...]

O = o

# Change the memory MODEL here, if desired
MODEL		= s

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

include $(PDCURSES_SRCDIR)/version.mif
include $(PDCURSES_SRCDIR)/libobjs.mif

osdir		= $(PDCURSES_SRCDIR)/dos

PDCURSES_DOS_H	= $(osdir)/pdcdos.h

CC		= ia16-elf-gcc

ifeq ($(DEBUG),Y)
	CFLAGS  = -g -Wall -DPDCDEBUG -march=any_186
	LDFLAGS = -g -march=any_186
else
	CFLAGS  = -Os -Wall -march=any_186
	# also include `-Os' and `-march=any_186' as flags during linking,
	# to select the 80186- and size-optimized newlib multilib
	LDFLAGS = -Os -march=any_186 -Wl,-M
endif

ifeq ($(MODEL),t)
	# `tiny' is ia16-elf-gcc's default model
else
ifeq ($(MODEL),s)
	CFLAGS	+= -mcmodel=small
	LDFLAGS	+= -mcmodel=small
else
	$(error unsupported memory model $(MODEL)!)
endif
endif

CFLAGS += -I$(PDCURSES_SRCDIR)

LINK		= ia16-elf-gcc

LIBEXE		= ia16-elf-ar
LIBFLAGS	= rcv

LIBCURSES	= pdcurses.a

.PHONY: all libs clean demos dist

all:	libs demos

libs:	$(LIBCURSES)

clean:
	$(RM) *.[oa] *.exe

demos:	$(DEMOS)

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS)
	$(LIBEXE) $(LIBFLAGS) $@ $?
	-cp $(LIBCURSES) panel.a

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_DOS_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
panel.o : $(PANEL_HEADER)
terminfo.o: $(TERM_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(CC) -c $(CFLAGS) $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(CC) -c $(CFLAGS) $<

firework.exe newdemo.exe rain.exe testcurs.exe worm.exe xmas.exe \
ptest.exe: %.exe: $(demodir)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o$@ $< $(LIBCURSES)

tuidemo.exe: tuidemo.o tui.o
	$(LINK) $(LDFLAGS) -o$@ tuidemo.o tui.o $(LIBCURSES)

tui.o: $(demodir)/tui.c $(demodir)/tui.h $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<

tuidemo.o: $(demodir)/tuidemo.c $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<

PLATFORM1 = ia16-elf-gcc
PLATFORM2 = ia16-elf-gcc for 16-bit DOS
ARCNAME = pdc$(VER)g16

include $(PDCURSES_SRCDIR)/makedist.mif

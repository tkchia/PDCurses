# I renamed gccdos.mak to gccdosdj.mak to avoid potential confusion  -- tkchia

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

include $(PDCURSES_SRCDIR)/dos/gccdosdj.mak

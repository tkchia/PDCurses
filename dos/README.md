PDCurses for DOS
================

This directory contains PDCurses source code files specific to DOS.


Building
--------

- Choose the appropriate makefile for your compiler:

        bccdos.mak   - Borland C++ 3.0+
        gccdos16.mak - Lambertsen et al.'s ia16-elf-gcc (16-bit 80186
                       compatible, cross-compile from Linux, experimental)
        gccdosdj.mak - DJGPP V2
        mscdos.mak   - Microsoft C
        wccdos.mak   - Open Watcom 1.8+ (16-bit or 32-bit)

- For 16-bit compilers, you can change the memory MODEL in the makefile.
  (Large model is the default, and recommended.)  (The exception is
  ia16-elf-gcc, which only supports tiny and small models.)

  (For Open Watcom, pass MODEL=f (flat model) to "wmake" in order to use
  the 32-bit compiler. Selcting a 16-bit MODEL (e.g. MODEL=l) will
  invoke the 16-bit compiler.)

- Optionally, you can build in a different directory than the platform
  directory by setting PDCURSES_SRCDIR to point to the directory where
  you unpacked PDCurses, and changing to your target directory:

        set PDCURSES_SRCDIR=c:\pdcurses

- Build it:

        make -f makefile

  (For Watcom, use "wmake" instead of "make"; for MSVC, "nmake".) You'll
  get the libraries (pdcurses.lib or .a, depending on your compiler; and
  panel.lib or .a), the demos (*.exe), and a lot of object files. Note
  that the panel library is just a copy of the main library, provided 
  for convenience; both panel and curses functions are in the main 
  library.


Distribution Status
-------------------

The files in this directory are released to the Public Domain.


Acknowledgements
----------------

Watcom C port was provided by Pieter Kunst <kunst@prl.philips.nl>

DJGPP 1.x port was provided by David Nugent <davidn@csource.oz.au>

/* Public Domain Curses */

#include <curspriv.h>
#include <string.h>

# if(CHTYPE_LONG >= 2)     /* 64-bit chtypes */
    # define PDC_ATTR_SHIFT  23
# else
#ifdef CHTYPE_LONG         /* 32-bit chtypes */
    # define PDC_ATTR_SHIFT  19
#else                      /* 16-bit chtypes */
    # define PDC_ATTR_SHIFT  8
#endif
#endif

#if defined(_MSC_VER) || defined(_QC)
# define MSC 1
#endif

#if defined(__PACIFIC__) && !defined(__SMALL__)
# define __SMALL__
#endif

#if defined(__GNUC__) && defined(__ia16__)
# define GCC_IA16 1
# if !defined(__TINY__) && !defined(__SMALL__) && !defined(__MEDIUM__) && \
     !defined(__COMPACT__) && !defined(__LARGE__) && !defined(__HUGE__)
#   define __SMALL__       /* erm... */
# endif
#endif

#if defined(__HIGHC__) || MSC
# include <bios.h>
#endif

/*----------------------------------------------------------------------
 *  MEMORY MODEL SUPPORT:
 *
 *  MODELS
 *    TINY    cs,ds,ss all in 1 segment (not enough memory!)
 *    SMALL   cs:1 segment, ds:1 segment
 *    MEDIUM  cs:many segments, ds:1 segment
 *    COMPACT cs:1 segment, ds:many segments
 *    LARGE   cs:many segments, ds:many segments
 *    HUGE    cs:many segments, ds:segments > 64K
 */

#ifdef __TINY__
# define SMALL 1
#endif
#ifdef __SMALL__
# define SMALL 1
#endif
#ifdef __MEDIUM__
# define MEDIUM 1
#endif
#ifdef __COMPACT__
# define COMPACT 1
#endif
#ifdef __LARGE__
# define LARGE 1
#endif
#ifdef __HUGE__
# define HUGE 1
#endif

#ifndef GCC_IA16
# include <dos.h>
#endif

extern unsigned char *pdc_atrtab;
extern int pdc_adapter;
extern int pdc_scrnmode;
extern int pdc_font;
extern bool pdc_direct_video;
extern bool pdc_bogus_adapter;
extern unsigned pdc_video_seg;
extern unsigned pdc_video_ofs;

#ifdef __DJGPP__        /* Note: works only in plain DOS... */
# if DJGPP == 2
#  define _FAR_POINTER(s,o) ((((int)(s)) << 4) + ((int)(o)))
# else
#  define _FAR_POINTER(s,o) (0xe0000000 + (((int)(s)) << 4) + ((int)(o)))
# endif
#else
# ifdef __TURBOC__
#  define _FAR_POINTER(s,o) MK_FP(s,o)
# else
#  if defined(__WATCOMC__) && defined(__FLAT__)
#   define _FAR_POINTER(s,o) ((((int)(s)) << 4) + ((int)(o)))
#  else
#   define _FAR_POINTER(s,o) ((unsigned long)(s) << 16 | \
                              (unsigned long)(unsigned long)(o))
#   define _FP_SEGMENT(p)    ((unsigned short) \
                              ((unsigned long)(void PDC_FAR *)(p) >> 16))
#   define _FP_OFFSET(p)     ((unsigned short)(unsigned long) \
                              (void PDC_FAR *)(p))
#  endif
# endif
#endif

#ifdef __DJGPP__
# include <sys/movedata.h>
unsigned char getdosmembyte(int offs);
unsigned short getdosmemword(int offs);
unsigned long getdosmemdword(int offs);
void setdosmembyte(int offs, unsigned char b);
void setdosmemword(int offs, unsigned short w);
#else
# if GCC_IA16
#  define PDC_FAR volatile __far
void dosmemget(unsigned long, size_t, void *);
void dosmemput(const void *, size_t, unsigned long);
# elif SMALL || MEDIUM || MSC
#  define PDC_FAR far
# else
#  define PDC_FAR
# endif
# define getdosmembyte(offs) \
    (*((unsigned char PDC_FAR *) _FAR_POINTER(0,offs)))
# define getdosmemword(offs) \
    (*((unsigned short PDC_FAR *) _FAR_POINTER(0,offs)))
# define getdosmemdword(offs) \
    (*((unsigned long PDC_FAR *) _FAR_POINTER(0,offs)))
# define setdosmembyte(offs,x) \
    (*((unsigned char PDC_FAR *) _FAR_POINTER(0,offs)) = (x))
# define setdosmemword(offs,x) \
    (*((unsigned short PDC_FAR *) _FAR_POINTER(0,offs)) = (x))
#endif

#if defined(__WATCOMC__) && defined(__386__)

typedef union
{
    struct
    {
        unsigned long edi, esi, ebp, res, ebx, edx, ecx, eax;
    } d;

    struct
    {
        unsigned short di, di_hi, si, si_hi, bp, bp_hi, res, res_hi,
                       bx, bx_hi, dx, dx_hi, cx, cx_hi, ax, ax_hi,
                       flags, es, ds, fs, gs, ip, cs, sp, ss;
    } w;

    struct
    {
        unsigned char edi[4], esi[4], ebp[4], res[4],
                      bl, bh, ebx_b2, ebx_b3, dl, dh, edx_b2, edx_b3,
                      cl, ch, ecx_b2, ecx_b3, al, ah, eax_b2, eax_b3;
    } h;
} pdc_dpmi_regs;

void PDC_dpmi_int(int, pdc_dpmi_regs *);

#elif defined GCC_IA16

/* For now, define a `union REGS' type and an int86(...) routine compatible
   with those in Borland's Turbo C 2.01, except that int86(...) supposedly
   returns an undocumented `int' in Turbo C and I leave that out */
union REGS
{
    struct WORDREGS { unsigned ax, bx, cx, dx, si, di, cflag, flags; } x;
    struct BYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; } h;
};

void int86(int, union REGS *, union REGS *);

#endif

#ifdef __DJGPP__
# include <dpmi.h>
# define PDCREGS __dpmi_regs
# define PDCINT(vector, regs) __dpmi_int(vector, &regs)
#else
# ifdef __WATCOMC__
#  ifdef __386__
#   define PDCREGS pdc_dpmi_regs
#   define PDCINT(vector, regs) PDC_dpmi_int(vector, &regs)
#  else
#   define PDCREGS union REGPACK
#   define PDCINT(vector, regs) intr(vector, &regs)
#  endif
# else
#  define PDCREGS union REGS
#  define PDCINT(vector, regs) int86(vector, &regs, &regs)
# endif
#endif

/* Wide registers in REGS: w or x? */

#ifdef __WATCOMC__
# define W w
#else
# define W x
#endif

/* Monitor (terminal) type information */

enum
{
    _NONE, _MDA, _CGA,
    _EGACOLOR = 0x04, _EGAMONO,
    _VGACOLOR = 0x07, _VGAMONO,
    _MCGACOLOR = 0x0a, _MCGAMONO,
    _MDS_GENIUS = 0x30
};

/* Text-mode font size information */

enum
{
    _FONT8 = 8,
    _FONT14 = 14,
    _FONT15,    /* GENIUS */
    _FONT16
};

#ifdef __PACIFIC__
void movedata(unsigned, unsigned, unsigned, unsigned, unsigned);
#endif

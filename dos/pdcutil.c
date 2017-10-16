/* Public Domain Curses */

#include "pdcdos.h"

void PDC_beep(void)
{
    PDCREGS regs;

    PDC_LOG(("PDC_beep() - called\n"));

    regs.W.ax = 0x0e07;       /* Write ^G in TTY fashion */
    regs.W.bx = 0;
    PDCINT(0x10, regs);
}

void PDC_napms(int ms)
{
    PDCREGS regs;
    long goal, start, current;

    PDC_LOG(("PDC_napms() - called: ms=%d\n", ms));

    goal = DIVROUND((long)ms, 50);
    if (!goal)
        goal++;

    start = getdosmemdword(0x46c);

    goal += start;

    while (goal > (current = getdosmemdword(0x46c)))
    {
        if (current < start)    /* in case of midnight reset */
            return;

        regs.W.ax = 0x1680;
        PDCINT(0x2f, regs);
        PDCINT(0x28, regs);
    }
}

const char *PDC_sysname(void)
{
    return "DOS";
}

PDCEX PDC_version_info PDC_version = { PDC_PORT_DOS,
          PDC_VER_MAJOR, PDC_VER_MINOR, PDC_VER_CHANGE,
          sizeof( chtype),
               /* note that thus far,  'wide' and 'UTF8' versions exist */
               /* only for SDL2, X11,  Win32,  and Win32a;  elsewhere, */
               /* these will be FALSE */
#ifdef PDC_WIDE
          TRUE,
#else
          FALSE,
#endif
#ifdef PDC_FORCE_UTF8
          TRUE,
#else
          FALSE,
#endif
          };

#ifdef __DJGPP__

unsigned char getdosmembyte(int offset)
{
    unsigned char b;

    dosmemget(offset, sizeof(unsigned char), &b);
    return b;
}

unsigned short getdosmemword(int offset)
{
    unsigned short w;

    dosmemget(offset, sizeof(unsigned short), &w);
    return w;
}

unsigned long getdosmemdword(int offset)
{
    unsigned long dw;

    dosmemget(offset, sizeof(unsigned long), &dw);
    return dw;
}

void setdosmembyte(int offset, unsigned char b)
{
    dosmemput(&b, sizeof(unsigned char), offset);
}

void setdosmemword(int offset, unsigned short w)
{
    dosmemput(&w, sizeof(unsigned short), offset);
}

#endif

#if defined(__WATCOMC__) && defined(__386__)

void PDC_dpmi_int(int vector, pdc_dpmi_regs *rmregs)
{
    union REGPACK regs = {0};

    rmregs->w.ss = 0;
    rmregs->w.sp = 0;
    rmregs->w.flags = 0;

    regs.w.ax = 0x300;
    regs.h.bl = vector;
    regs.x.edi = FP_OFF(rmregs);
    regs.x.es = FP_SEG(rmregs);

    intr(0x31, &regs);
}

#elif defined(GCC_IA16)

void int86(int intno, union REGS *inregs, union REGS *outregs)
{
    unsigned long vect = getdosmemdword((unsigned)4 * (unsigned char)intno);
    union REGS inr = *inregs, outr;
    __asm __volatile("pushw %%bp; "
                     "pushfw; "
                     "lcallw *%7; "
                     "popw %%bp; "
                     "pushfw; "
                     "popw %6"
        : "=a" (outr.x.ax), "=b" (outr.x.bx), "=c" (outr.x.cx),
          "=d" (outr.x.dx), "=S" (outr.x.si), "=D" (outr.x.di),
          "=m" (outr.x.flags)
        : "m" (vect), "0" (inr.x.ax), "1" (inr.x.bx), "2" (inr.x.cx),
          "3" (inr.x.dx), "4" (inr.x.si), "5" (inr.x.di)
        : "cc", "memory");
    outr.x.cflag = outr.x.flags & 1;
    *outregs = outr;
}

void dosmemget(unsigned long addr, size_t len, void *buf)
{
    unsigned cx, si, di;
    __asm __volatile("pushw %%ds; "
                     "movw %3, %%ds; "
                     "shrw $1, %%cx; "
                     "rep; movsw; "  /* assume DF == 0 */
                     "jnc 0f; "
                     "movsb; "
                     "0: "
                     "popw %%ds"
        : "=c" (cx), "=S" (si), "=D" (di)
        : "0" (len), "rm" (_FP_SEGMENT(addr)), "1" (_FP_OFFSET(addr)),
          "e" (_FP_SEGMENT((void __far *)buf)), "2" (buf)
        : "cc", "memory");
}

void dosmemput(const void *buf, size_t len, unsigned long addr)
{
    unsigned cx, si, di;
    __asm __volatile("shrw $1, %%cx; "
                     "rep; movsw; "  /* assume DF == 0 */
                     "jnc 0f; "
                     "movsb; "
                     "0: "
        : "=c" (cx), "=S" (si), "=D" (di)
        : "0" (len), "1" (buf), "e" (_FP_SEGMENT(addr)), "2" (_FP_OFFSET(addr))
        : "cc", "memory");
}

#endif

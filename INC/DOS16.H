/***************************************************************************
 *
 *  dos16.h -- Definitions and structures for protected mode
 *
 *
 *  $Revision:   1.60  $
 *  $Date:   08 Jul 1992 13:13:14  $
 *  $Logfile:   V:/vcs/include/logfiles/dos16.h_v  $
 *  $Log:   V:/vcs/include/logfiles/dos16.h_v  $
 *  
 *     Rev 1.60   08 Jul 1992 13:13:14   CHAS
 *  Fix function declaration
 *  
 *     Rev 1.59   07 Jul 1992 15:00:00   CHAS
 *  Add D16SetBase prototype
 *  
 *     Rev 1.58   07 Jul 1992 13:16:10   CHAS
 *  TSR API prototypes
 *
 *     Rev 1.57   19 May 1992 16:19:34   ACB
 *  Add RUN_AND_RETURN flag for D16ExpCall()
 *
 *     Rev 1.56   13 May 1992 13:38:04   ACB
 *  Add D16ExpCall() prototype
 *
 *     Rev 1.55   23 Apr 1992 18:23:14   jab
 *  Moved GDT32 (along with all other future 386-specific additions) to
 *  dos4g.h, which will be maintained in parallel with DOS/4G for now...
 *
 *     Rev 1.54   23 Apr 1992 15:35:56   jab
 *  Added type GDT32 for easier access to 386-only descriptor fields.
 *
 *     Rev 1.53   27 Mar 1992 12:05:18   ACB
 *  Fix prototypes w/ void
 *
 *     Rev 1.52   17 Dec 1991 15:42:26   jab
 *  New flag for D16ProgramExec ()
 *
 *     Rev 1.51   08 Nov 1991 18:19:10   jab
 *  Added FAR keyword to __d16_selectors declaration in the DLL case so that
 *  dynamic linking will work.
 *
 *     Rev 1.50   26 Oct 1991 13:28:32   ACB
 *  Fix HugePtr declarations
 *
 *     Rev 1.49   25 Oct 1991 17:58:26   ACB
 *  Make MetaWare 1.61 compatible
 *
 *     Rev 1.48   22 Oct 1991 12:19:40   ACB
 *  Fix ACTION typedef to be more generic
 *
 *     Rev 1.47   18 Sep 1991 17:07:54   mike
 *  Added conditional around __d16_selectors declaration so that if building the
 *  DOS16LIB.DLL the declaration will be far and not near.
 *
 *     Rev 1.46   04 Sep 1991 18:24:30   ACB
 *  Fix typedef for TurboC
 *
 *     Rev 1.45   30 Aug 1991 10:44:34   mike
 *  Added #ifdef for __ZTC to redefine huge as far as part of ZORTECH 3.0 support.
 *
 *     Rev 1.44   28 Aug 1991 11:50:32   StanL
 *  Added to D16INFO structure a variety of machine-specific info (especially
 *  port addresses) for use by kernel, apps, 'D', DOS4G, etc.
 *
 *     Rev 1.43   01 Aug 1991 17:13:34   jbw
 *  Defined D16misc_DVX.
 *
 *     Rev 1.42   25 Jun 1991 10:09:00   ACB
 *  Fix program load info declarations
 *
 *     Rev 1.41   24 Jun 1991 17:36:26   jab
 *  Added __DOSX () and __INT31X () prototypes.
 *
 *     Rev 1.40   24 Jun 1991 17:34:44   jab
 *  Added new loader entry points and structures.
 *
 *     Rev 1.39   29 Apr 1991 09:27:48   jab
 *  Changed package function prototypes to satisfy Lattice...
 *
 *     Rev 1.38   24 Apr 1991 14:49:02   jab
 *  Added the essential contents of PACKAGE.H, so that DOS16.H now contains all
 *  definintions necessary for building and calling Packages.
 *
 *     Rev 1.37   24 Apr 1991 11:44:28   jab
 *  Declared __d16_selectors to be 'near' when referenced from DOS16LIB.  This
 *  avoids some compiler support problems, especially in huge models.
 *
 *     Rev 1.36   09 Apr 1991 18:36:22   gch
 *  Changed the RM_RCALL vector from 18h to 20h. This avoids problems on the
 *   NEC 9801
 *
 *     Rev 1.35   28 Feb 1991 17:07:08   jab
 *  msg.txt
 *
 *     Rev 1.34   28 Feb 1991 15:37:08   jab
 *  Moved makeptr () macro from pmode.h to here !
 *
 *     Rev 1.33   27 Feb 1991 10:16:20   GCH
 *  Added bit defs for switch ctrl word. NoVCPI,EnOverload, Rest_Ints
 *
 *     Rev 1.32   23 Jan 1991 14:48:48   ACB
 *  Add prototype for D16ExpLoadP()
 *
 *     Rev 1.31   28 Dec 1990 15:25:40   jab
 *  Changed TSF conditionals to be friendlier to non-typedef users.
 *
 *     Rev 1.30   28 Dec 1990 13:59:18   jab
 *  Added D16Memcopy () and D16Memset ()
 *
 *     Rev 1.29   28 Dec 1990 13:07:16   jab
 *  Added d16info.miscellaneous bit for plutonium kernel presence test
 *
 *     Rev 1.28   13 Dec 1990 09:57:42   mike
 *  Made #if __cplusplus and #ifdef to eliminate warning messages from Watcom7.0
 *  compiler.
 *
 *     Rev 1.27   10 Dec 1990 09:20:30   jab
 *  Changed the control of lead-underscore symbol generation.
 *
 *     Rev 1.26   05 Dec 1990 16:39:26   jab
 *  Added C++ support, alternate symbol names, etc...
 *
 *     Rev 1.25   29 Oct 1990 14:16:22   WALT
 *
 *     Rev 1.24   05 Oct 1990 20:07:06   JBW
 *  Added xbufadj to d16info.
 *
 *     Rev 1.23   01 Oct 1990 18:34:38   JAB
 *  Added D16GetSelIncr () for DPMI compatibility.
 *
 *     Rev 1.22   21 Sep 1990 15:40:48   JAB
 *  Added D16PhysAddress ()
 *
 *     Rev 1.21   21 Sep 1990 12:47:42   JBW
 *  Removed names from argument prototypes so will work with MSC 4.
 *
 *     Rev 1.20   20 Sep 1990 16:54:56   JAB
 *  Welcome D16Lock, D16Unlock, D16Reside, D16UnReside (from VM-land) for use
 *  under DPMI or VM.
 *
 *     Rev 1.19   19 Sep 1990 15:35:14   JAB
 *  Changed #define names FORTRAN, PASCAL, MODULA2 to be different to avoid
 *  conflicts with C language keywords.
 *
 *     Rev 1.18   07 Sep 1990 17:57:12   JAB
 *  Added semi-colon...
 *
 *     Rev 1.17   07 Sep 1990 17:53:54   JAB
 *  Added prototype for D16ExpLoad ()
 *
 *     Rev 1.16   07 Sep 1990 17:42:32   JAB
 *  Handle redefines of _loadds, huge, far in a more friendly way
 *
 *     Rev 1.15   20 Aug 1990 16:15:30   JAB
 *  Another instance of incompatibility with lattice 3.20 is worked around...
 *
 *     Rev 1.14   19 Aug 1990 16:44:34   ACB
 *  intflag definition is not 'far' for KERNEL
 *
 *     Rev 1.13   17 Aug 1990 18:42:04   JAB
 *  Removed thinks that MSC4.0 doesn't like...
 *
 *     Rev 1.12   17 Aug 1990 17:16:24   JAB
 *  Added machinery for building language-independent DOS16M function library.
 *
 *     Rev 1.11   11 Jun 1990 14:37:10   JBW
 *  Added d16info.miscellaneous D16misc_extra_mem bit.  True if using
 *  non-contiguous extended/shadow RAM, just below 16MB on some computers.
 *
 *     Rev 1.10   08 Jun 1990 18:41:42   JBW
 *  Added d16info.miscellaneous, and defined D16misc_AT_compat, which is true
 *  if computer has an AT-compatible user architecture.
 *
 *     Rev 1.9   07 Jun 1990 17:12:24   JBW
 *  DPMI compatability.  Corrected tabbing.
 *  Added d16info.machine_type.  This represents the basic architecture
 *  using .swmode numbering (1 = NEC, 2 = PS/2, 3 = 386-AT, ... 9 = 286-AT, etc.)
 *  except for environments (never 0 for DPMI, never 11 for VCPI).
 *
 *     Rev 1.5   03 Jan 1990 14:31:24   JAB
 *  Added conditional to use _far instead of far with Metaware.
 *
 *     Rev 1.4   03 Jan 1990 13:24:58   JBW
 *  Changed comment describing access to current passup frame (PUF).
 *
 *     Rev 1.3   19 Oct 1989 15:27:28   JBW
 *  In NOFAR compilers, #define NearPtr *  breaks the TSF structure.
 *  Now, if NOFAR, .prev_tsf is declared unsigned.
 *
 *     Rev 1.2   09 Oct 1989 12:23:50   JBW
 *  added D16INFO.slow_A20
 *
 *     Rev 1.1   04 Oct 1989 17:33:40   JBW
 *  Added MSW_bits and package_info_p to _d16info structure.
 *
 *     Rev 1.0   19 Sep 1989 09:22:44   ben
 *  Initial revision.
 *
 *
 *  Copyright (c) Rational Systems, Inc. 1987 - 1992
 *  All Rights Reserved
 *
 */

#ifdef __ZTC__             /* For Zortech (others ?) C++ */
#define huge far
#endif
#ifdef __cplusplus             /* For Zortech (others ?) C++ */
#define huge far
extern "C" {
#endif

#ifdef NOCDECL              /* Lattice 3.20 has no cdecl keyword */
#define cdecl
#endif

#ifdef D16L                 /* Most compilers have no _loadds           */
 #define _loadds _loadds    /* This is important only when compiling    */
#else                       /* the library, not when calling it.        */
 #define _loadds
#endif

#ifdef NOHUGE               /* Lattice 3.20 has no huge keyword */
 #define huge far
#endif

#ifndef DOS16_INCLUDED
#define DOS16_INCLUDED

#define makeptr(s, o)  ( (void FarPtr)((long)(s) << 16 | (unsigned)(o)) )

#ifdef LDSN     /* Literal Data Symbol Names (compiler will NOT prepend '_') */
#define __d16_selectors ___d16_selectors
#endif

#ifdef NOFAR                        /* NOFAR assumes large model */
#define FarPtr  *
#define HugePtr  *
/* NearPtr                             doesn't work in NOFAR large model */
#define cdecl
#define Near
#else   /*================================================================*/
#ifdef __HIGHC__
#define _d16_selectors __d16_selectors

pragma Global_aliasing_convention("_%r");
#define Far  _far
#define FarPtr  * _far
#define HugePtr  * _huge
#define NearPtr  * _near
#define cdecl
#define Near
#else   /*================================================================*/
#define Far  far
#define FarPtr  far *
#define HugePtr  huge *
#define NearPtr  near *
#define Near  near
#endif  /*================================================================*/
#endif

#ifdef __WATCOMC__  /* define pragma for 'cdecl' keyword to match Microsoft */

#ifdef NOLEAD_
#pragma aux cdecl "*" parm caller []\
   value struct float struct routine [ax]  modify [ax bx cx dx es];
#else
#pragma aux cdecl "_*" parm caller []\
   value struct float struct routine [ax]  modify [ax bx cx dx es];
#endif

/* define pragma for 'pascal' keyword to match Microsoft conventions */
#pragma aux pascal "*" parm routine reverse []\
   value struct float struct routine [ax]  modify [ax bx cx dx es];

#endif

#ifdef KERNEL
#define NODPMI 1
#endif

#ifdef NODPMI               /* Kernel uses old constants for selectors */
#define gdt_sel 0x8         /* selector for gdt */
#define idt_sel 0x10        /* selector for idt */
#define kernel_sel 0x18     /* selector for run time kernel code */
#define kernel_data_sel 0x20    /* selector for kernel data */
#define psp_sel 0x28        /* selector for PSP */
#define env_sel 0x30        /* selector for Environment */
#define mem0_sel 0x38       /* selector for physical memory 0 */
#define biosdata_sel 0x40   /* selector for BIOS data segment */
#define gdti_sel 0x50       /* selector for gdt auxiliary information segment */
#define debug_gdt (0x60/8)  /* scratch desc for debugging */
#define debug_sel 0x60
#define kernelx_sel 0x70    /* kernel code in extended memory */
#define user_sel 0x80       /* first user selecter (user_gdt << 3) */
#define user_gdt 16         /* first user gdt index */
#else                       /* programs reference system selectors indirectly */
#define gdt_sel __d16_selectors.gdt         /* selector for gdt */
#define idt_sel __d16_selectors.idt         /* selector for idt */
#define kernel_sel __d16_selectors.kernel   /* selector for run time kernel code */
#define kernel_data_sel __d16_selectors.kerneld /* selector for kernel data */
#define psp_sel __d16_selectors.psp         /* selector for PSP */
#define env_sel __d16_selectors.env         /* selector for Environment */
#define mem0_sel __d16_selectors.mem0       /* selector for physical memory 0 */
#define bios_data_sel __d16_selectors.biosdata /* selector for BIOS data segment */
#define gdti_sel __d16_selectors.gdti       /* sel for gdt info seg */
#define kernelx_sel __d16_selectors.kernelx /* kernel code in extended memory */
#define user_sel __d16_selectors.user       /* first user selecter */
#define user_gdt (user_sel >> 3)            /* first user gdt index */
#endif

#define acc_code 0x9A   /* present, priv 0, code, read, accessed */
#define acc_cmask 0x83
#define acc_data 0x92   /* present, priv 0, data, write, accessed */
#define acc_dmask 0x83
#define acc_DPL 0x60    /* privilege level bits */
#define acc_LDT 0x82    /* Local Descriptor Table type */
#define acc_INT 0x8E    /* interrupt gate */
#define acc_TSS 0x85    /* tss descriptor */
#define acc_write 0x02  /* writable data, readable code access */

#define is_code(a)      (((a) & 0x18) == 0x18)
#define is_data(a)      (((a) & 0x18) == 0x10)
#define is_system(a)    (((a) & 0x10) == 0)
#define Present     0x80
#define is_present(a)  (((a) & Present) != 0)

#define SelMask 0xFFF8          /* lose ring and table bits */

#define MPreferExt 0            /* prefer, but not force, extended */
#define MPreferLow 1            /* prefer, but not force, conventional memory */
#define MForceExt 2             /* force extended */
#define MForceLow 3             /* force conventional */
#define MTransparent 4          /* force conventional, allocate selectors for */
                                /*   transparent addressing */
#define MTransStack 5           /* transparent, but aligned for use as stack */

typedef union {
    char FarPtr p;              /* treat as pointer */
    void FarPtr pv;
    struct {                    /* treat as selector/offset */
        unsigned off;
        unsigned sel;
        } w;
    long l;                     /* treat as arithmetic item */
    } FPTR;

extern struct {
    int d16ptr_ver;             /* structure version, for extensions */
    unsigned gdt;               /* descriptor table (gdt OR ldt, by host) */
    unsigned idt;
    unsigned kernel;
    unsigned kerneld;
    unsigned psp;
    unsigned env;
    unsigned mem0;
    unsigned biosdata;
    unsigned rsvd0;
    unsigned gdti;
    unsigned rsvd1[3];
    unsigned kernelx;
    unsigned rsvd2;
    unsigned user;
#ifdef D16L
#ifdef D16L_DLL /* MUST BE FAR FOR .DLLS! */
    } far __d16_selectors;
#else
    } near __d16_selectors;
#endif
#else
    } __d16_selectors;
#endif

typedef struct _intvect
    {
    unsigned off;
    unsigned sel;
    } INTVECT;

typedef struct _gdt
    {
    unsigned gdtlen;
    unsigned gdtaddr;
    unsigned char gdtaddr_hi;
    unsigned char gdtaccess;
    unsigned gdtreserved;
    } GDT;

typedef struct _idt
    {
    unsigned idtoffset;
    unsigned idtsel;
    unsigned char idtun1;
    unsigned char idtaccess;
    unsigned idtreserved;
    } IDT;

typedef struct tsf_struct       TSF;
typedef struct tsf_struct_k286  TSF2;
typedef struct tsf_struct_k386  TSF3;

#ifdef NOFAR
typedef unsigned TSFPREV;
#else
typedef TSF NearPtr TSFPREV;
#endif

struct tsf_struct
    {
    TSFPREV  prev_tsf;
    unsigned ss, ds, es;
    unsigned di, si, bp, sp;
    unsigned bx, dx, cx, ax;
#ifdef KERN386
    int      int_id, int_code, codepad ;
    unsigned ip, ippad, cs, cspad, flags, flpad ;
#else
    int      int_id, int_code;
    unsigned ip, cs, flags;
#endif
    } ;

struct tsf_struct_k286
    {
    TSFPREV  prev_tsf;
    unsigned ss, ds, es;
    unsigned di, si, bp, sp;
    unsigned bx, dx, cx, ax;
    int      int_id, int_code;
    unsigned ip, cs, flags;
    } ;

struct tsf_struct_k386
    {
    TSFPREV  prev_tsf;
    unsigned ss, ds, es;
    unsigned di, si, bp, sp;
    unsigned bx, dx, cx, ax;
    int      int_id, int_code, codepad ;
    unsigned ip, ippad, cs, cspad, flags, flpad ;
    } ;

typedef union _tsf_union { TSF2 t2; TSF3 t3;} TSFX;

typedef struct _d16regs {
    unsigned ds, es;
    unsigned di, si, bp, sp;    /* note: sp is ignored */
    unsigned bx, dx, cx, ax;
    } D16REGS;                  /* general registers, for D16rmInterrupt() */

#define D16_PACKAGES_DEFINED    /* Tell package.h not to bother... */

typedef unsigned long ACTION_RETURN;
typedef ACTION_RETURN (FarPtr ACTION)();   /* Package actions return ulong */
typedef struct action_pack
{
    char FarPtr action_name;
    ACTION FarPtr action;
} ACTION_PACK;

typedef struct package
{
    struct package FarPtr next_package; /* Pointer to next package in chain */
    char FarPtr package_title;          /* Optional pointer to ascii name */
    unsigned char major_rev;          /* Optional revision levels */
    unsigned char minor_rev;
    int system_action_count;          /* System action count */
    ACTION_PACK FarPtr system_action;   /* Pointer to vector of pre-defined */
    int user_action_count;            /* Optional user count */
    ACTION_PACK FarPtr user_action;     /* Pointer to vector of user_defined */
} PACKAGE;

#define PackageNext(package) package->next_package
#define PackageTitle(package) package->package_title
#define PackageRev(package) package->major_rev * 100 + package->minor_rev
#define PackageMajRev(package) package->major_rev
#define PackageMinRev(package) package->minor_rev

#define RUN_AND_RETURN  2
#define RUN_PKGS_ONLY   1
#define RUN_ALL_EXPS    0

#define TSR_EXIT        0       /* Flags to D16TsrExit () */
#define TSR_INSTALL     1
#define TSR_UNINSTALL   2

#define TSR_PSP_IO_offset  0x18 /* offset of IO table in PSP */
typedef struct
{
    char pspd[32];      /* IO data from PSP */
    unsigned pspa;      /* Real mode PSP address */
} TSR_IO_STATE;

typedef void FarPtr FDORNAME;

#define makelong(h, l)  ( ((long)(h) << 16) | (l) )
#define make_fdorname(h, l)  ((FDORNAME) (((long)(h) << 16) | (l)) )

#define MAX_PROCESS_PER_PGM 16      /* (Packages + App) per SPLICED EXE <= 16 */

#define LI_PGMFLAG_NORMAL   0x0000
#define LI_PGMFLAG_PACKAGE  0x0002

typedef struct _pgm_load_info       /* info about programs */
{
    char type_or_error;
    char overload;
    unsigned first_sel;
    unsigned last_sel;
    unsigned init_cs;
    unsigned init_ip;
    unsigned pgm_flags;             /* JAB 2/20/91: flags for last loaded */
    int fd;
    long next;
    char package_path[65];
    unsigned short process_first_sel[MAX_PROCESS_PER_PGM];
    unsigned short process_last_sel[MAX_PROCESS_PER_PGM];
    unsigned short process_level;
} PGM_LOAD_INFO;

#define CarryFlag 0x1           /* flags register mask for carry flag */

typedef struct
    {
    int version;        /* DOS/16M version * 100 */
    unsigned passdn0;   /* offset to int 0 passdown in kernel seg */
    unsigned passdn255; /* offset of last passdown */
    FPTR PassupIntP;    /* pointer to passup interrupt index */
    FPTR xfer_buff;     /* area where data transfers are buffered */
    unsigned xfer_bsize;    /* transfer buffer size */
    unsigned delay_286; /* switch delay for 286 cpus */
    unsigned delay_386; /* switch delay for 386 cpus */
    int switch_control; /* switch technique control flags (see SwCtrl) */
#ifdef __WATCOMC__  /* 'cdecl' keyword inside () only for Microsoft */
    char FarPtr cdecl (FarPtr D16MStkP)();  /* pointer to D16MoveStack function */
    unsigned cdecl (FarPtr D16ToRMP)(); /* pointer to D16ToReal function */
    int cdecl (FarPtr D16ToPMP)();      /* rm! pointer to D16ToProtected function */
    int cdecl (FarPtr D16rmInP)(int, D16REGS FarPtr, D16REGS FarPtr);
                /* pointer to D16rmInterrupt function */
#else
    char FarPtr (cdecl FarPtr D16MStkP)();  /* pointer to D16MoveStack function */
    unsigned (cdecl FarPtr D16ToRMP)(); /* pointer to D16ToReal function */
    int (cdecl FarPtr D16ToPMP)();      /* rm! pointer to D16ToProtected function */
    int (cdecl FarPtr D16rmInP)(int, D16REGS FarPtr, D16REGS FarPtr);
                /* pointer to D16rmInterrupt function */
#endif
    unsigned bannerp;   /* true if display standard startup message */
    unsigned D16rdata;  /* real mode DOS/16M data segment */
    char FarPtr versionp;   /* pointer to DOS/16M version string in kernel data */
    char swmode;        /* switch mode number */
    char cpumod;        /* cpu type (2 = 286, 3 = 386) */
    unsigned copyright; /* offset of DOS/16M copyright string */
    unsigned dbginfop;
    unsigned char D16_intno;    /* inter-DOS/16M communications */
    char A20, A20_rigor, A20_orig;
    unsigned last_delseg;   /* last deleted segment (for fault suppression) */
    TSF FarPtr FarPtr tsp_ptr;  /* ptr to TSF pointer */
    char has_87;        /* true if coprocessor present and not emulating */
    char slow_A20;      /* A20 resets not synchronized */
    unsigned MSW_bits;  /* bits OR-ed into MSW going to protected mode */
                /* !! Note:  you should always preserve the */
                /* bits in this word that you are not using. */
    void FarPtr package_info_p; /* package information */
    char machine_type;      /* same as .swmode, except never VCPI or DPMI */
    char miscellaneous;     /* information flags (see D16misc_ below) */
    unsigned xbufadj;           /* transfer buffer size adjustment */
/* #ifdef MACHINEINFO */
    unsigned master_8259 ;              /* first port, each PIC */
    unsigned slave_8259 ;
    unsigned portstep_8259 ;            /* port increment for PICs */
    char m8259_int, s8259_int ;         /* first INT ID, each PIC */
    unsigned base_8254 ;                /* first PIT port */
    unsigned portstep_8254 ;            /* PIT port increment */
    long means_unused ;                 /* unused int vec contents, this machine */
/* #endif */
    } D16INFO ;

/* defines for bits in d16info.miscellaneous */
#define D16misc_AT_compat 0x01      /* flag if AT-compatible user architecture */
#define D16misc_extra_mem 0x02      /* extra memory just below 16MB in use */
#define D16misc_pluto     0x04      /* set if a plutonium kernel is running */
#define D16misc_DVX               0x80

typedef struct {            /* PassUp interrupt registers */
    /* usage:  ((PUF FarPtr)(*_d16info.tsp_ptr) - 1)->pu_xx */
    unsigned pu_ss;         /* real mode ss */
    unsigned pu_es;         /* real mode es */
    unsigned pu_ds;         /* real mode ds */
    unsigned pu_rsvd;
    unsigned pu_ip;         /* real mode interrupt return frame */
    unsigned pu_cs;
    unsigned pu_flags;
    } PUF;

#ifndef NOFAR              /* (can not address these in Lattice 3.10) */

#ifdef KERNEL
    extern D16INFO _d16info;
#else
#ifdef __HIGHC__
    extern D16INFO _far _d16info;
#else
    extern D16INFO far _d16info;
#endif
#endif /* KERNEL */

#define D16MoveStack(new_sp, context)   (*_d16info.D16MStkP)(new_sp, context)
#define D16ToReal()                     (*_d16info.D16ToRMP)()
#define D16ToProtected(pcs, pss, pds, pes)  (*_d16info.D16ToPMP)(pcs, pss, pds, pes)
#ifdef KERNEL
#define D16rmInterrupt(intno, inrs, outrs)  (*_d16info.D16rmInP)(intno, inrs, outrs)
#endif  /* KERNEL */
#endif  /* NOFAR */

/* bits in _d16info.switch_control */
#define SwCtrl_A20 0x01     /* force A20 line 0 on 386's in real mode */
#define NoVCPI  0x02             /* DIsable VCPI initialization */
#define BIOS16_inPM 0x04    /* dont defer BIOS int 16 function 1 */
#define EnOverload  0x08         /* Allow overloading */
#define Rest_Ints         0x10    /* restore changed interrupts */
#define MemInit00 0x20      /* set newly allocated memory to 0x00 */
#define MemInitFF 0x40      /* set newly allocated memory to 0xFF */
#define RotateSelector 0x80 /* do not quickly reuse cancelled selectors */


#define RMCALL_INT 0x20     /* "resident Basic" interrupt used by rmRCall */
#define MAX_PASSUP_INDEX 8  /* number of passup interrupts supported */
#define PASSUP_INDEX_INC 4  /* sizeof passup element */

#define gdti_locked  0x80       /* in gdti */
#define gdti_reside  0x40       /* in gdti */
#define gdti_special 0x20       /* in gdti */
#define gdti_alias   0x10       /* in gdti */
#define gdti_block   0x02       /* in gdti */
#define gdti_owner   0x01       /* in gdti */

#endif   /*  DOS16_INCLUDED    */

#ifndef DOS16LIB_INCLUDED
#define DOS16LIB_INCLUDED

#undef  FP_SEG
#undef  FP_OFF
#define FP_SEG(p)   (*((unsigned short FarPtr) &(p) + 1))
#define FP_OFF(p)   (*(unsigned short FarPtr) &(p))

#define NULL_PTR     ((void *) 0)
#define NULL_SEGREG  0

#define acc_ref_bit 1       /* descriptor referenced bit */

#ifndef __HIGHC__
 #ifndef __WATCOMC__
  #ifndef D16L_M2
   #ifndef D16L_FOR
    #ifndef D16L_PAS
     #ifndef D16L_DLL
#define RSIAPI cdecl _loadds far
     #endif
    #endif
   #endif
  #endif
 #endif
#endif

#ifdef __HIGHC__
 #define RSIAPI _far
#endif

#ifdef __WATCOMC__
 #define RSIAPI cdecl far
#endif

#ifdef D16L_M2
 #define RSIAPI pascal _loadds far
#endif

#ifdef D16L_FOR
 #define RSIAPI pascal _loadds far
#endif

#ifdef D16L_PAS
 #define RSIAPI pascal _loadds far
#endif

#ifdef D16L_DLL
 #define RSIAPI cdecl _loadds far
#endif

#ifdef KERNEL
extern int cdecl _intflag(int);
#else
extern int cdecl Far _intflag (int);
#endif

extern int cdecl Far __DOSX (unsigned, D16REGS FarPtr, unsigned, unsigned);
extern int cdecl Far __INT31X (unsigned, unsigned, D16REGS FarPtr, unsigned, unsigned);
extern int cdecl Far __DOS (unsigned, D16REGS FarPtr);
extern int cdecl Far __INT31 (unsigned, unsigned, D16REGS FarPtr);
extern int cdecl Far _is_pm (void);
extern int cdecl Far chainint (void (FarPtr) ());
extern int cdecl Far D16Memcopy (void FarPtr, void FarPtr, unsigned short);
extern int cdecl Far D16Memset (void FarPtr, unsigned short, unsigned char);

#ifndef NO_PROTO
extern  int RSIAPI D16GetAccess (void FarPtr);
extern  int RSIAPI D16SetAccess (void FarPtr,int);
extern  int RSIAPI D16SegProtect (void FarPtr,int);
extern  void FarPtr RSIAPI D16SegDSAlias (void (FarPtr) ());
extern  void (FarPtr RSIAPI D16SegCSAlias (void FarPtr)) ();
extern  int RSIAPI D16CallDebug (char FarPtr);
extern  int RSIAPI D16SegCancel (void FarPtr);
extern  int RSIAPI D16GetROMCheksum (unsigned long FarPtr);
extern  int RSIAPI D16GetDelay (unsigned int FarPtr);
extern  int RSIAPI D16SetDelay (unsigned int);
extern  int RSIAPI D16GetDesc (unsigned int,struct _gdt FarPtr);
extern  int RSIAPI D16SetDesc (unsigned int,struct _gdt FarPtr);
extern  int RSIAPI D16SetBase (unsigned int, unsigned long, unsigned int);
extern  unsigned int RSIAPI D16SegLimit (void FarPtr);
extern  int RSIAPI D16rmGetVector (unsigned int,struct _intvect FarPtr);
extern  int RSIAPI D16pmGetVector (unsigned int,struct _intvect FarPtr);
extern  void HugePtr RSIAPI huge_alloc (unsigned long, unsigned int);
extern  void HugePtr RSIAPI D16HugeAlloc (long);
extern  long RSIAPI D16HugeResize (void FarPtr,long);
extern  void HugePtr RSIAPI D16SelReserve (int);
extern  int RSIAPI D16isDOS16M (void);
extern  int RSIAPI D16isDPMI (void);
extern  int RSIAPI D16MemStrategy (int);
extern  void FarPtr RSIAPI D16MemAlloc (unsigned int);
extern  int RSIAPI D16MemFree (void FarPtr);
extern  long RSIAPI D16ExtAvail (void);
extern  long RSIAPI D16LowAvail (void);
extern  void FarPtr RSIAPI D16SegRealloc (void FarPtr);
extern  void FarPtr RSIAPI D16SegTransparent (unsigned int, unsigned int);
extern  void FarPtr RSIAPI D16SegDataPtr (void FarPtr, unsigned int);
extern  int RSIAPI D16SegResize (void FarPtr, unsigned int);
extern  int RSIAPI D16IntStrategy (int);
extern  int RSIAPI D16Passdown (int);
extern  int RSIAPI D16Passup (int);
extern  int RSIAPI D16pmInterrupt (int, struct _d16regs FarPtr, struct _d16regs FarPtr);
extern  unsigned long RSIAPI D16AbsAddress (void FarPtr);
extern  void FarPtr RSIAPI D16SegAbsolute (long, unsigned int);
extern  void FarPtr RSIAPI D16ProtectedPtr (void FarPtr, unsigned int);
extern  long RSIAPI D16ExtReserve (long);
extern  void FarPtr RSIAPI D16RealPtr (void FarPtr);
extern  int RSIAPI D16rmRCall (unsigned int, unsigned int, struct _d16regs FarPtr,struct _d16regs FarPtr);
extern  unsigned int RSIAPI D16NewSelector (void);
extern  int RSIAPI D16FreeSelector (unsigned int);
extern  int RSIAPI D16rmInstall (unsigned int, unsigned int, unsigned int, struct _intvect FarPtr);
extern  int RSIAPI D16pmInstall (unsigned int, unsigned int, unsigned int, struct _intvect FarPtr);
extern  int RSIAPI D16pmSetVector (int, char FarPtr, char FarPtr);
extern  int RSIAPI D16rmSetVector (int, char FarPtr,char FarPtr);
extern  void RSIAPI D16TermFunction (void (_loadds cdecl FarPtr) ());
extern  int RSIAPI D16Lock (void FarPtr);
extern  int RSIAPI D16UnLock (void FarPtr);
extern  int RSIAPI D16Reside (void FarPtr);
extern  int RSIAPI D16UnReside (void FarPtr);
extern  long RSIAPI D16PhysAddress (long);
extern  int RSIAPI D16GetSelIncr (void);

extern  int RSIAPI D16ProgramLoad (FDORNAME, long, TSF FarPtr);
extern  int RSIAPI D16ProgramLoadContinue (TSF FarPtr);
extern  int RSIAPI D16ProgramExec (struct tsf_struct FarPtr,char FarPtr, int);
extern void RSIAPI D16ProgramDispatch (TSF FarPtr);
extern  int RSIAPI D16ProgramUnload (void);
extern void RSIAPI D16GetLoadInfo (PGM_LOAD_INFO FarPtr (FarPtr));
extern void RSIAPI D16SetLoadInfo (PGM_LOAD_INFO FarPtr);

extern  int RSIAPI D16ExpLoad (int, TSF FarPtr);
extern  int RSIAPI D16ExpLoadP (int, TSF FarPtr, long);
extern  int RSIAPI D16Overload (int, struct tsf_struct FarPtr);
extern  int RSIAPI D16Execute (struct tsf_struct FarPtr,char FarPtr);
extern  int RSIAPI D16ExpCall (struct tsf_struct FarPtr,char FarPtr);


extern void FarPtr RSIAPI D16FindAction (PACKAGE FarPtr, char FarPtr);
extern PACKAGE FarPtr RSIAPI D16FindPackage (char FarPtr);
extern  int RSIAPI D16PackageUnregister (PACKAGE FarPtr);
extern void RSIAPI D16PackageRegister (PACKAGE FarPtr);
extern void RSIAPI D16PackageTsr (int);

extern unsigned int RSIAPI D16GetRmPspSeg(void);
extern int RSIAPI D16TsrExit (int, int);
extern int RSIAPI D16IsKernelSeg (void FarPtr);
extern int RSIAPI D16OkayToUninstall (void);
extern int RSIAPI D16GetFreeVector (unsigned int);
extern int RSIAPI D16IsFreeVector (int);
extern int RSIAPI D16FindTsrVector (char FarPtr, int, int, int);
extern int RSIAPI D16TsrSaveIOState (TSR_IO_STATE FarPtr);
extern int RSIAPI D16TsrSetIOState (TSR_IO_STATE FarPtr);
extern int RSIAPI D16TsrSwitchIOState (TSR_IO_STATE FarPtr, TSR_IO_STATE FarPtr);


#ifndef KERNEL
extern  int RSIAPI D16rmInterrupt (int, D16REGS FarPtr, D16REGS FarPtr);
#endif

#endif  /*  NO_PROTO          */
#endif  /*  DOS16LIB_INCLUDED */

#ifdef __HIGHC__
pragma Global_aliasing_convention();
#endif

#ifdef NOCDECL              /* Lattice 3.20 has no cdecl keyword */
#undef cdecl
#endif

#undef _loadds

#ifdef NOHUGE               /* Lattice 3.20 has no huge keyword */
#undef huge
#endif

#ifdef __cplusplus             /* Zortech C++ */
}
#endif

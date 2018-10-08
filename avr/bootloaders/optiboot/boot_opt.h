// Get all the "standard" definitions from the official boot.h
#include <avr/boot.h>


/*
 * Implement some optimized versions that will use OUT instead
 *   of STS to write SPMCSR.
 *   (However, omit the *_extended_short, since by the time you
 *   need _extended_, the extra byte shouldn't be relevant any more)
 *
 * The C preprocessor can not determin at compile time whether SPMCSR is
 *   "out of range" of the OUT instruction, but we CAN do that in the
 *   assembler.  We can even make it pretty with a macro.
 * With this modification, the _short functions should work on cpus
 *   (like ATmega128) where STS is required.
 */

asm(".macro __wr_spmcsr p, v \n\t"
    ".if \\p > 0x57	\n\t"
    "sts \\p, \\v	\n\t"
    ".else		\n\t"
    "out \\p-0x20, \\v	\n\t"
    ".endif		\n\t"
    ".endm		\n");


#if defined(__SPM_REG)

#define __boot_page_fill_short(address, data)    \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "movw  r0, %3\n\t"                       \
        "__wr_spmcsr %0, %1\n\t"                 \
        "spm\n\t"                                \
        "clr  r1\n\t"                            \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),         \
          "r" ((uint8_t)__BOOT_PAGE_FILL),       \
          "z" ((uint16_t)address),               \
          "r" ((uint16_t)data)                   \
        : "r0"                                   \
    );                                           \
}))

#define __boot_page_erase_short(address)         \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "__wr_spmcsr %0, %1\n\t"                 \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),         \
          "r" ((uint8_t)__BOOT_PAGE_ERASE),      \
          "z" ((uint16_t)address)                \
    );                                           \
}))

#define __boot_page_write_short(address)         \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "__wr_spmcsr %0, %1\n\t"                 \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),         \
          "r" ((uint8_t)__BOOT_PAGE_WRITE),      \
          "z" ((uint16_t)address)                \
    );                                           \
}))

#define __boot_rww_enable_short()                \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "__wr_spmcsr %0, %1\n\t"                 \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),         \
          "r" ((uint8_t)__BOOT_RWW_ENABLE)       \
    );                                           \
}))

#endif // __SPM_REG

#ifndef __boot_page_erase_short

/*
 * if __SPM_REG didn't get defined by now, but we didn't exit it means
 * we have some sort of new-fangled chip that post-dates the version
 * of boot.h that we know about.  In this case, it's possible that the
 * standard boot.h still has workable functions, so we'll alias those.
 */

#define __boot_page_fill_short(address, data) boot_page_fill(address, data)
#define __boot_page_erase_short(address) boot_page_erase(address)
#define __boot_page_write_short(address) boot_page_write(address)
#define __boot_rww_enable_short() boot_rww_enable()

#endif


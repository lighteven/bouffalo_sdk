/****************************************************************************************
* @file bl702_flash.ld
*
* @brief This file is the map file (gnuarm or armgcc).
*
* Copyright (C) BouffaloLab 2021
*
****************************************************************************************
*/

/* configure the CPU type */
OUTPUT_ARCH( "riscv" )

/* configure the entry point */
ENTRY(__start)

StackSize    = 0x0400; /*  1KB */
HeapMinSize  = 0x1000; /*  4KB */

MEMORY
{
    fw_header_memory  (rx)  : ORIGIN = 0x23000000 - 0x1000, LENGTH = 4K
    xip_memory  (rx)  : ORIGIN = 0x23000000, LENGTH = 1024K
    itcm_memory (rx)  : ORIGIN = 0x22014000, LENGTH = 14K
    dtcm_memory (rx)  : ORIGIN = 0x42017800, LENGTH = 4K
    ram_memory  (!rx) : ORIGIN = 0x42018800, LENGTH = 94K
    hbn_memory  (rx)  : ORIGIN = 0x40010000, LENGTH = 0xE00     /* hbn ram 4K used 3.5K*/
}

SECTIONS
{
    PROVIDE(__metal_chicken_bit = 0);

    .fw_header :
    {
        KEEP(*(.fw_header))
    } > fw_header_memory

    .init :
    {
        KEEP (*(SORT_NONE(.init)))
        KEEP (*(SORT_NONE(.vector)))

    } > xip_memory

    .text :
    {
        . = ALIGN(4);
        __text_code_start__ = .;

        *(.text)
        *(.text.*)

        /* section information for shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;

        /*put .rodata**/
        *(EXCLUDE_FILE( *bl702_glb*.o* \
                        *bl702_pds*.o* \
                        *bl702_common*.o* \
                        *bl702_sf_cfg*.o* \
                        *bl702_sf_cfg_ext*.o* \
                        *bl702_sf_ctrl*.o* \
                        *bl702_sflash*.o* \
                        *bl702_sflash_ext*.o* \
                        *bl702_xip_sflash*.o* \
                        *bl702_xip_sflash_ext*.o* \
                        *bl702_ef_ctrl*.o*) .rodata*)

        *(.srodata)
        *(.srodata.*)

        /* global object for c++ */
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        /* _dump_sections for bflb_coredump.c*/
        . = ALIGN(4);
        PROVIDE( _dump_sections = ABSOLUTE(.));
        LONG(ORIGIN(itcm_memory));
        LONG(LENGTH(itcm_memory));
        LONG(ORIGIN(dtcm_memory));
        LONG(LENGTH(dtcm_memory));
        LONG(ORIGIN(ram_memory));
        LONG(LENGTH(ram_memory));
        LONG(ORIGIN(hbn_memory));
        LONG(LENGTH(hbn_memory));
        LONG(0xffffffff);
        LONG(0xffffffff);
        /* _dump_sections for bl_coredump.c*/

        . = ALIGN(4);
        __text_code_end__ = .;
    } > xip_memory

    .build_id :
    {
        . = ALIGN(4);
        elf_build_id = ABSOLUTE(.) + 16;
        *(.note.gnu.build-id)
    } > xip_memory

    . = ALIGN(4);
    __itcm_load_addr = .;

    .itcm_region : AT (__itcm_load_addr)
    {
        . = ALIGN(4);
        __tcm_code_start__ = .;

        *(.tcm_code.*)
        *(.tcm_const.*)
        *(.sclock_rlt_code.*)
        *(.sclock_rlt_const.*)

        *bl702_glb*.o*(.rodata*)
        *bl702_pds*.o*(.rodata*)
        *bl702_common*.o*(.rodata*)
        *bl702_sf_cfg*.o*(.rodata*)
        *bl702_sf_cfg_ext*.o*(.rodata*)
        *bl702_sf_ctrl*.o*(.rodata*)
        *bl702_sflash*.o*(.rodata*)
        *bl702_sflash_ext*.o*(.rodata*)
        *bl702_xip_sflash*.o*(.rodata*)
        *bl702_xip_sflash_ext*.o*(.rodata*)
        *bl702_ef_ctrl*.o*(.rodata*)

        . = ALIGN(4);
        __tcm_code_end__ = .;
    } > itcm_memory

    __hbn_load_addr = __itcm_load_addr + SIZEOF(.itcm_region);

    .hbn_ram_region : AT (__hbn_load_addr)
    {
        __hbn_ram_start__ = ADDR(.hbn_ram_region);
        __hbn_ram_end__ = ADDR(.hbn_ram_region) + SIZEOF(.hbn_ram_region);

        /* put bl702_rf_private.o in hbnram to avoid ocram conflict during rf calibration */
        *libphyrf.a:bl702_rf_private.c.o*(.retention .retention_noinit)
        *libphyrf.a:bl702_rf_private.c.o*(.sdata.* .sbss.* .bss.* COMMON)

        *(.hbn_code.*)
        *(.hbn_data.*)
    } > hbn_memory
    
    ASSERT(SIZEOF(.hbn_ram_region) <= 0x400, ".hbn_ram_region overlapped with IOT2LP_PARA(refer to bl_lp.h)")

    __dtcm_load_addr = __hbn_load_addr + SIZEOF(.hbn_ram_region);

    .dtcm_region : AT (__dtcm_load_addr)
    {
        . = ALIGN(4);
        __tcm_data_start__ = .;

        *(.tcm_data)
        /* *finger_print.o(.data*) */

        . = ALIGN(4);
        __tcm_data_end__ = .;
    } > dtcm_memory

    /*************************************************************************/
    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (NOLOAD):
    {
        . = ALIGN(0x4);
        . = . + StackSize;
        . = ALIGN(0x4);
    } > dtcm_memory

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(dtcm_memory) + LENGTH(dtcm_memory);
    PROVIDE( __freertos_irq_stack_top = __StackTop);
    PROVIDE( _sp_main = __StackTop);
    PROVIDE( _sp_base = __StackTop - StackSize);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __tcm_data_end__, "region RAM overflowed with stack")
    /*************************************************************************/

    __ram_load_addr = __dtcm_load_addr + SIZEOF(.dtcm_region);

    /* Data section */
    RAM_DATA : AT (__ram_load_addr)
    {
        . = ALIGN(4);
        __ram_data_start__ = .;

        PROVIDE( __global_pointer$ = . + 0x800 );

        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
        *(.sdata2)
        *(.sdata2.*)

        *(.nocache_ram)
        
        . = ALIGN(4);
        __bflog_tags_start__ = .;
        *(.bflog_tags_array)
        . = ALIGN(4);
        __bflog_tags_end__ = .;
        __ram_data_end__ = .;
    } > ram_memory

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start__ = .;

        *(.bss*)
        *(.sbss*)
        *(COMMON)

        . = ALIGN(4);
        __bss_end__ = .;
    } > ram_memory

    .noinit_data (NOLOAD) :
    {
        . = ALIGN(4);
        __noinit_data_start__ = .;

        *(.noinit_data*)
        *(.nocache_noinit_ram)

        . = ALIGN(4);
        __noinit_data_end__ = .;
    } > ram_memory

    .heap (NOLOAD):
    {
        . = ALIGN(4);
        __HeapBase = .;

        KEEP(*(.heap*))

        . = ALIGN(4);
        __HeapLimit = .;
    } > ram_memory

    __HeapLimit = ORIGIN(ram_memory) + LENGTH(ram_memory);

    ASSERT((__HeapLimit -  __HeapBase ) >= HeapMinSize, "heap size is too short.")

}



#include <drivers/framebuffer/framebuffer.h>
#include <drivers/serial/serial.h>
#include <drivers/keyboard/keyboard.h>
#include <tss/tss.h>
#include <gdt/gdt.h>
#include <idt/idt.h>
#include <pic/pic.h>
#include <pit/pit.h>
#include <interrupt/interrupt.h>
#include <paging/paging.h>
#include <pmm/pmm.h>
#include <kheap/kheap.h>
#include <fs/fs.h>
#include <rd/rd.h>
#include <process/process.h>
#include <syscall/syscall.h>
#include <klock/klock.h>
#include <elf/elf.h>
#include <drivers/ata/ata.h>
#include <ext2/ext2.h>
#include <common/multiboot.h>
#include <common/constants.h>
#include <debug/log.h>
#include <util/util.h>

void page_fault_handler(
  cpu_state_t cs, idt_info_t info, stack_state_t ss
  )
{
  uint32_t vaddr;
  asm("movl %%cr2, %0" : "=r"(vaddr));
  log_error(
    "kmain", "eip %x: page fault %x vaddr %x cs %x\n",
    ss.eip, info.error_code, vaddr, ss.cs
    );
}

void gp_fault_handler(
  cpu_state_t cs, idt_info_t info, stack_state_t ss
  )
{
  log_error(
    "kmain", "eip %x: gpf %x cs %x\n",
    ss.eip, info.error_code, ss.cs
    );
}

void kmain(
  uint32_t mb_info_addr,
  uint32_t mb_magic_number,
  page_directory_t kernel_pd,
  uint32_t kvirt_start,
  uint32_t kvirt_end
  )
{
  // Convert phyical addresses in the multiboot info structure
  // into virtual addresses.
  multiboot_info_t *mb_info = (multiboot_info_t *)mb_info_addr;
  mb_info->mods_addr += KERNEL_START_VADDR;
  mb_info->mmap_addr += KERNEL_START_VADDR;

  // Convert virtual addresses exported by link.ld to physical
  // addresses.
  uint32_t kphys_start = kvirt_start - KERNEL_START_VADDR;
  uint32_t kphys_end = kvirt_end - KERNEL_START_VADDR;

  if ((mb_info->flags & 8) == 0 || mb_info->mods_count != 1) {
    log_error("kmain", "Modules not loaded.");
    return;
  }

  multiboot_module_t *rd_module = (multiboot_module_t *)(mb_info->mods_addr);
  uint32_t rd_phys_start = rd_module->mod_start;
  uint32_t rd_phys_end = rd_module->mod_end;

  if (mb_magic_number != MULTIBOOT_BOOTLOADER_MAGIC) {
    log_error("kmain", "Incorrect magic number.");
    return;
  }

  interrupt_init();
  enable_interrupts();
  uint32_t eflags = interrupt_save_disable();

  fb_clear();
  serial_init(SERIAL_COM1_BASE);

  tss_init();
  uint32_t tss_vaddr = tss_get_vaddr();
  gdt_init(tss_vaddr);
  idt_init();
  pic_init();
  pit_init();
  keyboard_init();

  register_interrupt_handler(14, page_fault_handler);
  register_interrupt_handler(13, gp_fault_handler);

  uint32_t res;
  res = pmm_init(mb_info, kphys_start, kphys_end, rd_phys_start, rd_phys_end);

  res = paging_init(
    kernel_pd, (uint32_t)kernel_pd - KERNEL_START_VADDR
    );
  paging_set_kernel_pd(kernel_pd, (uint32_t)kernel_pd - KERNEL_START_VADDR);

  fs_init();
  res = rd_init(rd_phys_start, rd_phys_end);

  ata_init();

  fs_node_t test_node;
  fs_open_node(&test_node, "/rd/test", 0);
  /* uint8_t *test_text = kmalloc(test_node.length); */
  /* fs_read(&test_node, 0, test_node.length, test_text); */

  process_init();

  /* process_image_t p; */
  /* elf_load(&p, test_text); */

  /* process_t init; process_t child; */
  /* process_create_init(&init, p); */
  /* process_fork(&child, &init); */
  /* process_schedule(&init); */
  /* process_schedule(&child); */

  /* kfree(test_text); */
  /* kfree(p.text); */
  /* kfree(p.data); */

  res = ext2_init("/dev/hda");
  log_debug("kmain", "ext2 init res: %u\n", res);

  int32_t sres = fs_symlink("/ext2/d", "/ext2/lnk");
  log_debug("kmain", "sres: %u\n", sres);

  fs_node_t node;
  res = fs_open_node(&node, "/ext2/d/hello.txt", 0);
  log_debug("kmain", "res: %u\n", res);
  fs_write(&node, 0, 5, (uint8_t *)"hello");

  interrupt_restore(eflags);
}

#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  /*  
   *  +------------------------+
   *  |   ELF header           |
   *  +------------------------+
   *  |   Program Header Table |
   *  +------------------------+
   *  |   sections             |
   *  |                        |
   *  +------------------------+
   *  |   section header table |
   *  +------------------------+
   */
  int fd = fs_open(filename, 0, 0);/* ignore flags and mode in sys */

  //printf("filename is %s, fd is %d\n",filename, fd);
  Elf_Ehdr elf_head;
  // ramdisk_read(&elf_head, 0, sizeof(Elf_Ehdr));
  fs_read(fd, &elf_head, sizeof(Elf_Ehdr));
  /* magic number, little endian, LSB */
  assert(*(uint32_t *)elf_head.e_ident == 0x464c457f);
  uint32_t phum = elf_head.e_phnum;/* get the number of program header table */
  Elf_Phdr elf_phdr[phum];
  fs_read(fd, &elf_phdr, sizeof(Elf_Phdr) * phum);
  // ramdisk_read(&elf_phdr, 0 + elf_head.e_phoff, sizeof(Elf_Phdr) * phum);
  for(int i = 0; i < phum; i++){
    if(elf_phdr[i].p_type == PT_LOAD){
      /* load p_memsz size of bytes start from p_offset to p_vaddr which p_type = PT_LOAD 
       *   p_vaddr:  the virtual address the first byte of the segment should reside in memory.
       *   p_paddr:  address in physical memory where the segment will be loaded at runtime.(dynamically loaded)
       *   p_filesz: the number of bytes in the file that are allocated for the segment, including any padding. 
       *   p_memsz:  the size of the segment in memory. 
       *         q: where to set p_paddr and p_memsz dynamically?
       *         a: typically set by the linker when it creates the ELF file.
       */
      /* (void*)  is a generic pointer type that can hold the address of any data type */
      fs_lseek(fd, elf_phdr[i].p_offset, 0);
      fs_read(fd, (void*)elf_phdr[i].p_vaddr,  elf_phdr[i].p_memsz);
      /* 
       *  set block starting symbol(.bss) to zero 
       *    [(VirtAddr + FileSiz, VirtAddr + MemSiz)]
       */
      memset((void*)(elf_phdr[i].p_vaddr + elf_phdr[i].p_filesz), 0, elf_phdr[i].p_memsz - elf_phdr[i].p_filesz);
    }
  }
  fs_close(fd);
  return elf_head.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


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
  printf("file name is %s \n",filename);
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


void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  Area stack;


  stack.start = &pcb->cp;
  stack.end = (void *)&pcb->cp + STACK_SIZE;
  // stack.end = (void *)0x80070000 + STACK_SIZE;
  // printf("stack start is %p\n", stack.start);
  // printf("stack end is %p\n", stack.end);
  pcb->cp = kcontext(stack, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
   uintptr_t entry = loader(pcb, filename);
/*
|               |
+---------------+ <---- heap.end
|               |
|    context    |
|               |
+---------------+ <--+
|               |    |
                     |
                     |
|               |    |
+---------------+    |
|       cp      | ---+
+---------------+ <---- kstack.start
|               |

set the top of heap as user space stack
|               |
+---------------+ <--- heap.end(high address) 0x88000000
|               |
|      heap     |
|               |
+---------------+ <--+ head.start/stack_pointer 82b28000
|               |    |
|     stack     |    +--0x8000
|               |    |
+---------------+ <--+ stack_top(low address)
|               |
.................
|               |
+---------------+ <--- _pmem_start 0x80000000
*/
  Area user_stack;
  uint8_t *heap_end = heap.end;
  user_stack.end = heap_end;
  user_stack.start = heap_end - STACK_SIZE;
  // printf("size of uint8 and uint32 , (context *) (char*) is %d, %d, %d, %d\n",sizeof(uint8_t), sizeof(uint32_t), sizeof(Context *), sizeof(char*));

  Log("user_stack.start: %p, user_stack.end: %p", user_stack.start, user_stack.end);
  Log("entry: %p", entry);

  pcb->cp = ucontext(NULL, user_stack, (void(*)()) entry);
  uint32_t *env_base = (uint32_t *)pcb->cp - 1;
  printf("env_base is %p\n", env_base);
  /* set argv and envp */
  /*
|               |
+---------------+ <---- ustack.end(env_base) high address
|  Unspecified  |
+---------------+
|               | <----------+
|    string     | <--------+ |
|     area      | <------+ | |
|               | <----+ | | |
|               | <--+ | | | |
+---------------+    | | | | |
|  Unspecified  |    | | | | |
+---------------+    | | | | |
|     NULL      |    | | | | |
+---------------+    | | | | |
|    ......     |    | | | | |
+---------------+    | | | | |
|    envp[1]    | ---+ | | | |
+---------------+      | | | |
|    envp[0]    | -----+ | | |
+---------------+        | | |
|     NULL      |        | | |
+---------------+        | | |
| argv[argc-1]  | -------+ | |
+---------------+          | |
|    ......     |          | |
+---------------+          | |
|    argv[1]    | ---------+ |
+---------------+            |
|    argv[0]    | -----------+
+---------------+
|      argc     |
+---------------+ <---- cp->GPR0(a0)
|               |
|      stack    | <---- stack
|               |
*/
  // int argc = 0;
  int envc = 0, argc = 0;
  while(argv && argv[argc]) {printf("Argument argv[%d] is %s\n", argc, argv[argc]); argc++;}
  while(envp && envp[envc]) {printf("Argument envp[%d] is %s\n", envc, envp[envc]); envc++;}

  // char *argv_str_area[argc];
  char *argv_addr[argc], *envp_area[envc];
  char *str_area = (char *)env_base;
  for(int i = 0; i < argc; i++) {
    str_area -= strlen(argv[i]) + 1;
    argv_addr[i] = str_area;
    strcpy(str_area, argv[i]);/* should set 0 in the end of str? */
  }

  for (int i = 0; i < envc; i++){
    str_area -= strlen(envp[i]) + 1;
    envp_area[i] = str_area;
    strcpy(str_area, envp[i]);
  }

  uintptr_t *ptr = (uintptr_t *)str_area;
  ptr -= 20;  // Unspecified
  *ptr = (intptr_t)NULL; ptr--;
  for(int i = envc - 1; i >= 0; i--){
    *ptr = (intptr_t)envp_area[i];
    ptr--;
  }
  *ptr = (intptr_t)NULL; ptr--;
  for(int i = argc - 1; i >= 0; i--) {
    *ptr = (intptr_t)argv_addr[i];
    ptr--;
  }
  *ptr = argc;
  printf("addr of ptr is %p\n",ptr);
  pcb->cp->GPR0 = (uintptr_t)ptr;



  // pcb->cp->GPR0 = (uintptr_t) user_stack.start;/* set user process stack top to a0 */
}

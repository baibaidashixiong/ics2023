#include <common.h>
#include <elf.h>
#include <stdlib.h>

#ifdef CONFIG_FTRACE
uint32_t FUNC_NUM = 128;
/* Store the ELF file header. */
Elf32_Ehdr *Elf_Ehdr;
/* Store the ELF section header. */
Elf32_Shdr *Elf_Shdr, *Elf_Shdrs;
/* Store the ELF symbol table entry. */
Elf32_Sym *Elf_Sym, *Elf_Syms;

// typedef struct {
//     uint32_t addr;
//     char func_name[16];
// }FTRACE_FUNC;
// #define FUNC_NUM1 128
FTRACE_FUNC ftrace_func[FUNC_NUM1];


int i = 0, j = 0;
char *string_table;
char value_table[128];

void parse_elf(const char *elf_file)
{
    FILE *file = fopen(elf_file, "r");
    if (!file)
    {
        perror("fopen");
        exit(1);
    }

    /* Read the ELF header */
    Elf_Ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if(fread(Elf_Ehdr, sizeof(Elf32_Ehdr), 1, file));

    /* Read the section headers */
    Elf_Shdrs = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * Elf_Ehdr->e_shnum);
    /* 
     * use fseek() to move the file position indicator to the desired location before calling fread().
     * then read data from a specific location within a file without changing the file position indicator
     */
    fseek(file, Elf_Ehdr->e_shoff, SEEK_SET);
    /* To be figured: if use assert will cause core dump, why? */
    if(fread(Elf_Shdrs, sizeof(Elf32_Shdr), Elf_Ehdr->e_shnum, file));

    /* Find the symbol table section */
    for (i = 0; i < Elf_Ehdr->e_shnum; i++)
    {
        Elf_Shdr = &Elf_Shdrs[i];
        if (Elf_Shdr->sh_type == SHT_SYMTAB)
        {
            /* Read the symbol table */
            Elf_Syms = Elf_Sym = (Elf32_Sym *)malloc(Elf_Shdr->sh_size);
            fseek(file, Elf_Shdr->sh_offset, SEEK_SET);
            if(fread(Elf_Sym, Elf_Shdr->sh_size, 1, file));

            /* Find the associated string table */
            string_table = (char *)malloc(Elf_Shdrs[Elf_Shdr->sh_link].sh_size);
            fseek(file, Elf_Shdrs[Elf_Shdr->sh_link].sh_offset, SEEK_SET); /* symtab location */
            if(fread(string_table, Elf_Shdrs[Elf_Shdr->sh_link].sh_size, 1, file));
            FUNC_NUM = Elf_Shdr->sh_size / sizeof(Elf32_Sym);

            /* Print the symbols */
            for (j = 0; j < FUNC_NUM; j++)
            {
                sprintf(ftrace_func[j].func_name, "%s", string_table + Elf_Sym[j].st_name);
                ftrace_func[j].type= Elf_Sym[j].st_info;
                ftrace_func[j].addr = Elf_Sym[j].st_value;
            }
            // for (j = 0; j < FUNC_NUM; j++)
            // {
            //     printf("0x%x\n", ftrace_func[j].addr);
            //     printf("%s\n", ftrace_func[j].func_name);
            // }


            free(Elf_Syms);
            free(string_table);
        }
    }
    fclose(file);
}
#endif
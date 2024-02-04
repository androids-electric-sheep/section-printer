#include <elf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

bool file_exists(const char *filename) {
  if (access(filename, F_OK)) {
    perror("access");
    return false;
  }
  return true;
}

unsigned get_file_size(const char *filename) {
  struct stat st;
  if (stat(filename, &st)) {
    perror("stat");
    return 0;
  }
  return st.st_size;
}

void *read_file_contents(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    perror("fopen");
    return NULL;
  }
  printf("[+] Opened file for reading\n");

  const int file_size = get_file_size(filename);
  if (file_size == 0) {
    fprintf(stderr, "[-] Unable to get size of file or it's empty\n");
    return NULL;
  }
  printf("[+] File size: %d\n", file_size);

  void *file_content = malloc(file_size);
  if (file_content == NULL) {
    perror("malloc");
    return NULL;
  }

  int blocks_read = fread((void *)file_content, file_size, 1, f);
  if (blocks_read != 1) {
    fprintf(stderr, "Unable to read file contents fully\n");
    free(file_content);
    return NULL;
  }
  printf("[+] Read in file contents\n");
  fclose(f);
  return file_content;
}

char *get_shstrtab(Elf64_Ehdr *ehdr) {
  int shstrtab_index = ehdr->e_shstrndx;
  printf("[+] SHSTRTAB index: %d\n", shstrtab_index);

  Elf64_Shdr *shdr = (Elf64_Shdr *)((void *)ehdr + ehdr->e_shoff);
  shdr += shstrtab_index;
  return (char *)((void *)ehdr + shdr->sh_offset);
}

void print_section_names(Elf64_Ehdr *ehdr, char *shstrtab) {
  Elf64_Shdr *shdr = (Elf64_Shdr *)((void *)ehdr + ehdr->e_shoff);
  for (int i = 0; i < ehdr->e_shnum; ++i) {
    printf("[+] %d -> %s\n", i, &shstrtab[shdr->sh_name]);
    ++shdr;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "[-] Usage: %s <ELF_FILE>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *filename = argv[1];
  if (!file_exists(filename)) {
    fprintf(stderr, "[-] Unable to work with %s as it doesn't exist\n",
            filename);
    return EXIT_FAILURE;
  }
  printf("[+] Working on %s\n", filename);

  void *file_content = read_file_contents(filename);
  if (file_content == NULL) {
    fprintf(stderr, "Unable to read file contents\n");
    return EXIT_FAILURE;
  }

  Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_content;
  printf("[+] Entrypoint: %p\n", (void *)ehdr->e_entry);

  bool error = false;
  char *shstrtab = get_shstrtab(ehdr);
  if (shstrtab == NULL) {
    error = true;
    goto cleanup;
  }

  print_section_names(ehdr, shstrtab);

cleanup:
  printf("[+] Cleaning up\n");
  free(file_content);
  if (error) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

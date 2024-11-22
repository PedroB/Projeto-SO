#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>

#include "constants.h"
#include "parser.h"
#include "operations.h"


  void *readFilesLines(void *args){ 
  char *filename = (char *)args;
  int file_fd = open(filename, O_RDONLY); 
  if (file_fd < 0) {
        perror("Error opening file");
    }

  while (1) {
    char keys[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    char values[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    unsigned int delay;
    size_t num_pairs;

    printf("> ");
    fflush(stdout);

    switch (get_next(STDIN_FILENO)) {
      case CMD_WRITE:
        num_pairs = parse_write(STDIN_FILENO, keys, values, MAX_WRITE_SIZE, MAX_STRING_SIZE);
        if (num_pairs == 0) {
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          continue;
        }

        if (kvs_write(num_pairs, keys, values)) {
          fprintf(stderr, "Failed to write pair\n");
        }

        break;

      case CMD_READ:
        num_pairs = parse_read_delete(STDIN_FILENO, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

        if (num_pairs == 0) {
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          continue;
        }

        if (kvs_read(num_pairs, keys)) {
          fprintf(stderr, "Failed to read pair\n");
        }
        break;

      case CMD_DELETE:
        num_pairs = parse_read_delete(STDIN_FILENO, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

        if (num_pairs == 0) {
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          continue;
        }

        if (kvs_delete(num_pairs, keys)) {
          fprintf(stderr, "Failed to delete pair\n");
        }
        break;

      case CMD_SHOW:

        kvs_show();
        break;

      case CMD_WAIT:
        if (parse_wait(STDIN_FILENO, &delay, NULL) == -1) {
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          continue;
        }

        if (delay > 0) {
          printf("Waiting...\n");
          kvs_wait(delay);
        }
        break;

      case CMD_BACKUP:

        if (kvs_backup()) {
          fprintf(stderr, "Failed to perform backup.\n");
        }
        break;

      case CMD_INVALID:
        fprintf(stderr, "Invalid command. See HELP for usage\n");
        break;

      case CMD_HELP:
        printf( 
            "Available commands:\n"
            "  WRITE [(key,value)(key2,value2),...]\n"
            "  READ [key,key2,...]\n"
            "  DELETE [key,key2,...]\n"
            "  SHOW\n"
            "  WAIT <delay_ms>\n"
            "  BACKUP\n" // Not implemented
            "  HELP\n"
        );

        break;
        
      case CMD_EMPTY:
        break;

      case EOC:
        kvs_terminate();
        return 0;
    
  }
  }
  }

  int gen_path(char* dir_name,struct dirent* entry,char * in_path,char* out_path){
   (void)dir_name; 
  while(1){
  char *ptr_to_dot = strrchr(entry->d_name, '.');
    if (ptr_to_dot == NULL || strcmp(ptr_to_dot, ".job") != 0)
    {
      continue;
    }
  strcpy(out_path, in_path);
  strcpy(strrchr(out_path,'.'),".out");
  }
  return 1;
}

  int main(int argc,char *argv[]) {
  struct dirent *entry;
  char* dir_name = argv[1];
  char in_path[MAX_JOB_FILE_NAME_SIZE],out_path[MAX_JOB_FILE_NAME_SIZE];
  DIR *dir;
  dir = opendir(dir_name);
  

  if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

  if(dir == NULL){
    printf("Cannot open filename '%s'\n",dir_name);
    return 1;
  }


  if (kvs_init()) {
    fprintf(stderr, "Failed to initialize KVS\n");
    return 1;
  }


  while((entry = readdir(dir)) != NULL){
    if(gen_path(dir_name,entry,in_path,out_path)){
      continue;
    }



  }
//sdasasdadasda
  closedir(dir);
  return 0;
}

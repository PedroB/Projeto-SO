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


struct ThreadArgs {
    char file[BUFFER_SIZE];
    int thread_id;
    int BARRIER_ATIVO;
};


  void readFilesLines(void *args){ 
  struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
  int BARRIER = threadArgs->BARRIER_ATIVO;
    int fd = 0;
    if (BARRIER == 0){
      fd = open(threadArgs->file, O_RDONLY);
    }
  

  /* replaced by this*/

  // int BARRIER = threadArgs->BARRIER_ATIVO;

    /*-------------------*/




  while (1) {
    char keys[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    char values[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    unsigned int delay;
    size_t num_pairs;

    printf("> ");
    fflush(stdout);
    switch (get_next(fd)) {
      case CMD_WRITE:
        num_pairs = parse_write(fd, keys, values, MAX_WRITE_SIZE, MAX_STRING_SIZE);
        if (num_pairs == 0) {
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          continue;
        }

        if (kvs_write(num_pairs, keys, values)) {
          fprintf(stderr, "Failed to write pair\n");
        }

        break;

      case CMD_READ:

        num_pairs = parse_read_delete(fd, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

        if (num_pairs == 0) {
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          continue;
        }

        if (kvs_read(num_pairs, keys)) {
          fprintf(stderr, "Failed to read pair\n");
        }
        break;

      case CMD_DELETE:

        num_pairs = parse_read_delete(fd, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

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

        if (parse_wait(fd, &delay, NULL) == -1) {
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
      puts("entrou invalidddd");
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
      
      case EOC:
        return;
      case CMD_EMPTY:
      default:
        break;    
  }
  }
  }


/* whie (readdir)-> genpath(buscar o file da diretoria) (coloca o output do readfileslines num .out)*/

/* directory -> ver o file -> output fica num novo .out
int gen_path(char* dir_name, struct dirent* entry, char* in_path, char* out_path) {
     if (!dir_name || !entry || !in_path || !out_path) {
        return 1; 
    }


    char *ptr_to_dot = strrchr(entry->d_name, '.');
    if (strcmp(ptr_to_dot, ".job") == 0) {

        strcpy(out_path, in_path);
        
        // Find the last dot in the path
        char *last_dot = strrchr(out_path, '.');
        
        if (last_dot != NULL) {
            strcpy(last_dot, ".out");  // Replace the extension with ".out"
        } else {
            // Handle the case where no dot is found (this might need to be customized for your needs)
            strcat(out_path, ".out");  // Append ".out" at the end of the string
        }
        readFilesLines(dir_name);
    }

    return 1;
}
*/

int gen_path(char* dir_name, struct dirent* entry, char *in_path, char *out_path) {
    if (!dir_name || !entry || !in_path || !out_path) {
        return 1; 
    }
    // Check the total length of dir_name and entry->d_name
    size_t dir_len = strlen(dir_name);
    size_t file_len = strlen(entry->d_name);

    // Ensure the combined path fits within MAX_JOB_FILE_NAME_SIZE
    if (dir_len + 1 + file_len + 1 > MAX_JOB_FILE_NAME_SIZE) {
        fprintf(stderr, "Error: Combined path length exceeds MAX_JOB_FILE_NAME_SIZE.\n");
        return 1;
    }

    // Copy directory name to in_path
    strncpy(in_path, dir_name, MAX_JOB_FILE_NAME_SIZE - 1);
    in_path[MAX_JOB_FILE_NAME_SIZE - 1] = '\0';  // Ensure null termination

    // Append '/' and entry->d_name
    strncat(in_path, "/", MAX_JOB_FILE_NAME_SIZE - strlen(in_path) - 1);
    strncat(in_path, entry->d_name, MAX_JOB_FILE_NAME_SIZE - strlen(in_path) - 1);

  char *ptr_to_dot = strrchr(entry->d_name, '.');
    if (ptr_to_dot && strcmp(ptr_to_dot, ".job") == 0) {
        // Create output file path
        snprintf(out_path, MAX_JOB_FILE_NAME_SIZE, "%s/%.*s.out", 
                 dir_name, (int)(ptr_to_dot - entry->d_name), entry->d_name);

        // Call readFilesLines and redirect output to .out file
        int out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd == -1) {
            perror("Failed to open output file");
            return 1;
        }

        // Redirect stdout to the output file
        int saved_stdout = dup(STDOUT_FILENO);
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);

        // Process the file
        readFilesLines(in_path);

        // Restore stdout
        fflush(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    return 0;
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

    // readFilesLines(dir_name); 




  }
//sdasasdadasda
  closedir(dir);
  kvs_terminate();

  return 0;
}

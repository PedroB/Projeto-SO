#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "constants.h"
#include "parser.h"
#include "operations.h"

//mudança numero 2
//testar mundança numero 3
  
int main(int argc,char *argv[]) {
  struct dirent *pDirent;
  DIR *dir;
  dir = opendir(argv[1]);

  if(dir == NULL){
    printf("Cannot open filename '%s'\n",argv[1]);
    return 1;
  }

  if (kvs_init()) {
    fprintf(stderr, "Failed to initialize KVS\n");
    return 1;
  }

  while(pDirent = readdir(dir) != NULL){
    char *ptr_to_dot = strrchr(pDirent->d_name, '.');
    if (ptr_to_dot == NULL || strcmp(ptr_to_dot, ".job") != 0)
    {
      continue;
    }
  }
}

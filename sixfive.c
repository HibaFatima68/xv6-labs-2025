#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "fcntl.h"

// separators
const char *seps = " -\r\t\n./,";

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: sixfive file...\n");
    exit(1);
  }

  for(int i = 1; i < argc; i++){
    int fd = open(argv[i], O_RDONLY);
    if(fd < 0){
      fprintf(2, "sixfive: cannot open %s\n", argv[i]);
      continue;
    }

    char ch;
    char numbuf[32]; // enough for int
    int idx = 0;

    // read char by char
    while(read(fd, &ch, 1) == 1){
      if(ch >= '0' && ch <= '9'){
        // accumulate digit
        if(idx < sizeof(numbuf)-1){
          numbuf[idx++] = ch;
        }
      } else {
        // separator found
        if(idx > 0){
          numbuf[idx] = '\0';
          int val = atoi(numbuf);
          if(val % 5 == 0 || val % 6 == 0){
            printf("%d\n", val);
          }
          idx = 0; // reset buffer
        }
        // else skip
      }
    }

    // handle last number if file doesn't end with separator
    if(idx > 0){
      numbuf[idx] = '\0';
      int val = atoi(numbuf);
      if(val % 5 == 0 || val % 6 == 0){
        printf("%d\n", val);
      }
    }

    close(fd);
  }

  exit(0);
}

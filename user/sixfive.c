#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf("Usage: sixfive file...\n");
    exit(1);
  }

  char seps[] = " -\r\t\n./,";
  char buf[32];
  int len;

  for(int i = 1; i < argc; i++){
    int fd = open(argv[i], 0); // 0 = O_RDONLY
    if(fd < 0){
      printf("sixfive: cannot open %s\n", argv[i]);
      continue;
    }

    len = 0;
    char c;
    while(read(fd, &c, 1) == 1){
      if(strchr(seps, c)){
        if(len > 0){
          buf[len] = '\0';
          int num = atoi(buf);
          if(num % 5 == 0 || num % 6 == 0)
            printf("%d\n", num);
          len = 0;
        }
      } else if(c >= '0' && c <= '9'){
        if(len < sizeof(buf)-1)
          buf[len++] = c;
      }
    }
    // flush last number at EOF
    if(len > 0){
      buf[len] = '\0';
      int num = atoi(buf);
      if(num % 5 == 0 || num % 6 == 0)
        printf("%d\n", num);
    }

    close(fd);
  }
  exit(0);
}

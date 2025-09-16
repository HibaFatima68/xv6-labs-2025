#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"
#include "param.h"  // for MAXARG

void find(char *path, char *target, char **exec_argv) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        printf("find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf("find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
        case T_FILE:
            // Extract the filename from path
            p = path;
            while (*p) p++;
            while (p > path && *(p - 1) != '/') p--;

            if (strcmp(p, target) == 0) {
                if (exec_argv) {
                    int pid = fork();
                    if (pid < 0) {
                        printf("find: fork failed\n");
                    } else if (pid == 0) {
                        // child process
                        char *argv[MAXARG];
                        int i = 0;
                        while (exec_argv[i] != 0) {
                            argv[i] = exec_argv[i];
                            i++;
                        }
                        argv[i++] = path; // add filename as last argument
                        argv[i] = 0;
                        exec(argv[0], argv);
                        printf("find: exec failed\n");
                        exit(1);
                    } else {
                        int wstatus;
                        wait(&wstatus);
                    }
                } else {
                    printf("%s\n", path);
                }
            }
            break;

        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
                printf("find: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';

            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;

                int i;
                for (i = 0; i < DIRSIZ && de.name[i]; i++)
                    p[i] = de.name[i];
                p[i] = 0;

                find(buf, target, exec_argv);
            }
            break;
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        printf("Usage: find path name [-exec cmd args...]\n");
        exit(1);
    }

    char **exec_argv = 0;

    for(int i = 3; i < argc; i++){
        if(strcmp(argv[i], "-exec") == 0){
            if(i+1 >= argc){
                printf("Usage: find path name [-exec cmd args...]\n");
                exit(1);
            }
            exec_argv = &argv[i+1];
            break;
        }
    }

    find(argv[1], argv[2], exec_argv);
    exit(0);
}

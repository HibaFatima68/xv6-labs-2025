#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int simple_match(char *pattern, char *text) {
if(strcmp(pattern, ".*") == 0 || strlen(pattern) == 0) {
return 1;
}
char *pos = text;
while(*pos) {
char *p = pattern;
char *t = pos;
int match = 1;
while(*p && *t) {
if(*p != *t) {
match = 0;
break;
}
p++;
t++;
}
if(match && *p == '\0') {
return 1;
}
pos++;
}
return 0;
}

char* get_filename(char *path) {
char *filename = path;
for(char *p = path; *p; p++) {
if(*p == '/') filename = p + 1;
}
return filename;
}

void find(char *path, char *pattern) {
char buf[512], *p;
int fd;
struct dirent de;
struct stat st;

if((fd = open(path, 0)) < 0){
fprintf(2, "find: cannot open %s\n", path);
return;
}

if(fstat(fd, &st) < 0){
fprintf(2, "find: cannot stat %s\n", path);
close(fd);
return;
}

switch(st.type){
case T_FILE:
{
char *filename = get_filename(path);
if(simple_match(pattern, filename)) {
printf("%s\n", path);
}
}
break;

case T_DIR:
if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
printf("find: path too long\n");
break;
}
strcpy(buf, path);
p = buf + strlen(buf);
*p++ = '/';
while(read(fd, &de, sizeof(de)) == sizeof(de)){
if(de.inum == 0)
continue;
memmove(p, de.name, DIRSIZ);
p[DIRSIZ] = 0;
if(stat(buf, &st) < 0){
printf("find: cannot stat %s\n", buf);
continue;
}
if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
continue;
find(buf, pattern);
}
break;
}
close(fd);
}

int main(int argc, char *argv[]) {
if(argc < 3){
printf("Usage: find <directory> <pattern>\n");
printf("Note: This version uses simple substring matching\n");
printf("Use '.*' to match all files\n");
exit(1);
}
printf("Searching in '%s' for pattern '%s'\n", argv[1], argv[2]);
find(argv[1], argv[2]);
exit(0);
}

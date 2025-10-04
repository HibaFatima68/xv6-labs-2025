#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
int len = 16384;
char *buf = sbrk(len);
if (buf == (char *)-1) {
fprintf(2, "sbrk failed\n");
exit(1);
}
for (int i = 0; i < len; i += 4096) {
char tmp = buf[i];
(void)tmp;
}
int max_start = -1, max_len = 0;
for (int offset = 0; offset < len; offset++) {
int cur_start = -1, cur_len = 0;
for (int i = offset; i < len; i++) {
char c = buf[i];
if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
if (cur_start == -1) cur_start = i;
cur_len++;
} else {
if (cur_len > max_len) {
max_len = cur_len;
max_start = cur_start;
}
cur_start = -1;
cur_len = 0;
}
}
}
if (max_start != -1 && max_len > 0) {
write(1, buf + max_start, max_len);
write(1, "\n", 1);
} else {
write(1, "No secret found\n", 16);
}
exit(0);
}

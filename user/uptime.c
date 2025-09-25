
#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
int ticks = uptime();
printf("Uptime (ticks): %d\n", ticks);


exit(0);
}

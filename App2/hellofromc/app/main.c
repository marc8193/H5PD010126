#include <unistd.h>
#include <stdio.h>

void android_main(struct android_app* app) {
    printf("Hello from %s!\n", APPNAME);
    while (1) {
        sleep(1);
    }
}

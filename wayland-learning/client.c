#include <stdio.h>
#include <wayland-client.h>

/* 
cc -o client client.c -lwayland-client
cc -o client client.c xdg-shell-protocol.c -lwayland-client -lrt
 */
int
main(int argc, char *argv[])
{
    struct wl_display *display = wl_display_connect("wayland-0");
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display.\n");
        return 1;
    }
    fprintf(stderr, "Connection established!\n");

    wl_display_disconnect(display);
    return 0;
}

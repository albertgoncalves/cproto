#include <X11/Xlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;

#define EXPOSURE_MASK  ExposureMask
#define KEY_PRESS_MASK KeyPressMask

i32 main(void) {
    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        return EXIT_FAILURE;
    }
    i32    screen = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display,
                                        RootWindow(display, screen),
                                        100,
                                        100,
                                        500,
                                        300,
                                        1,
                                        WhitePixel(display, screen),
                                        BlackPixel(display, screen));
    XSelectInput(display, window, EXPOSURE_MASK | KEY_PRESS_MASK);
    XMapWindow(display, window);
    XEvent event;
    for (;;) {
        XNextEvent(display, &event);
    }
    return EXIT_SUCCESS;
}

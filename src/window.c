#include <X11/Xlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;

typedef int32_t i32;

#define EXPOSURE_MASK  ExposureMask
#define KEY_PRESS_MASK KeyPressMask

i32 main(void) {
    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        return EXIT_FAILURE;
    }
    Screen* screen = DefaultScreenOfDisplay(display);
    i32     screen_id = DefaultScreen(display);
    Window  window = XCreateSimpleWindow(display,
                                        RootWindowOfScreen(screen),
                                        100,
                                        100,
                                        500,
                                        300,
                                        1,
                                        WhitePixel(display, screen_id),
                                        BlackPixel(display, screen_id));
    XClearWindow(display, window);
    XSelectInput(display, window, EXPOSURE_MASK | KEY_PRESS_MASK);
    XMapRaised(display, window);
    XEvent event;
    for (u8 _ = 0; _ < 3; ++_) {
        XNextEvent(display, &event);
    }
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return EXIT_SUCCESS;
}

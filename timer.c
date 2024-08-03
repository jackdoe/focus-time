#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define WINDOW_WIDTH 200
#define WINDOW_HEIGHT 200

Display *display;
Window window;
Visual *visual;
Colormap colormap;
XftDraw *xft_draw;
XftFont *xft_font;
XftColor xft_color;
int screen;

long initial_seconds = 3600; // Default to 1 hour
char color_name[20] = "red";
char font_name[256] = "IBM 3270:size=24:style=Bold";
int is_game_over = 0;

void create_transparent_window() {
    XSetWindowAttributes attrs;
    XVisualInfo vinfo;
    XRenderPictFormat *fmt;

    if (!XMatchVisualInfo(display, screen, 32, TrueColor, &vinfo)) {
        fprintf(stderr, "No 32 bit visual available\n");
        exit(1);
    }

    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    visual = vinfo.visual;
    colormap = XCreateColormap(display, DefaultRootWindow(display), visual, AllocNone);

    attrs.colormap = colormap;
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;
    attrs.override_redirect = True;
    int x = screen_width - WINDOW_WIDTH;
    int y = screen_height - WINDOW_HEIGHT; 
    window = XCreateWindow(display, DefaultRootWindow(display),
                           x, y, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                           vinfo.depth, InputOutput, visual,
                           CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, &attrs);

    fmt = XRenderFindVisualFormat(display, visual);
    XRenderPictureAttributes pattr;
    pattr.poly_edge = PolyEdgeSmooth;
    pattr.poly_mode = PolyModeImprecise;

    XRenderCreatePicture(display, window, fmt, CPPolyEdge | CPPolyMode, &pattr);

    // Make the window always on top
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom wm_state_above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(display, window, wm_state, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&wm_state_above, 1);

    XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonMotionMask);
    XMapWindow(display, window);
}

void setup_font_and_color() {
    xft_font = XftFontOpenName(display, screen, font_name);
    if (!xft_font) {
        fprintf(stderr, "Failed to load font %s\n", font_name);
        exit(1);
    }

    if (!XftColorAllocName(display, visual, colormap, color_name, &xft_color)) {
        fprintf(stderr, "Failed to allocate color %s\n", color_name);
        exit(1);
    }

    xft_draw = XftDrawCreate(display, window, visual, colormap);
}

void draw_timer(long seconds_left) {
    char time_str[20];
    XGlyphInfo extents;

    if (is_game_over) {
        strcpy(time_str, "GAME OVER");
    } else {
        int hours = seconds_left / 3600;
        int minutes = (seconds_left % 3600) / 60;
        int seconds = seconds_left % 60;
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);
    }

    XftTextExtentsUtf8(display, xft_font, (XftChar8 *)time_str, strlen(time_str), &extents);

    // Clear the window with a fully transparent black
    XftDrawRect(xft_draw, &(XftColor){.color = {0, 0, 0, 0}}, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    XftDrawStringUtf8(xft_draw, &xft_color, xft_font,
                      (WINDOW_WIDTH - extents.width) / 2,
                      (WINDOW_HEIGHT + extents.height) / 2 - extents.y,
                      (XftChar8 *)time_str, strlen(time_str));
}

int main(int argc, char *argv[]) {
    XEvent event;
    struct timeval start_time, current_time;
    long elapsed_seconds, seconds_left;
    int drag_start_x, drag_start_y;

    if (argc > 1) {
        initial_seconds = atol(argv[1]) * 60;
    }
    if (argc > 2) {
        strncpy(color_name, argv[2], sizeof(color_name) - 1);
    }
    if (argc > 3) {
        strncpy(font_name, argv[3], sizeof(font_name) - 1);
    }

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen = DefaultScreen(display);

    create_transparent_window();
    setup_font_and_color();

    gettimeofday(&start_time, NULL);

    while (1) {
        while (XPending(display)) {
            XNextEvent(display, &event);
            switch (event.type) {
                case Expose:
                    draw_timer(initial_seconds);
                    break;
                case ButtonPress:
                    if (event.xbutton.button == 1) {
                        drag_start_x = event.xbutton.x;
                        drag_start_y = event.xbutton.y;
                    }
                    break;
                case MotionNotify:
                    if (event.xbutton.state & Button1Mask) {
                        XMoveWindow(display, window, event.xbutton.x_root - drag_start_x, event.xbutton.y_root - drag_start_y);
                    }
                    break;
            }
        }

        gettimeofday(&current_time, NULL);
        elapsed_seconds = current_time.tv_sec - start_time.tv_sec;
        seconds_left = initial_seconds - elapsed_seconds;

        if (seconds_left <= 0 && !is_game_over) {
            is_game_over = 1;
            seconds_left = 0;
        }

        draw_timer(seconds_left);
        XFlush(display);

        usleep(100000);  // Sleep for 100ms
    }

    XftDrawDestroy(xft_draw);
    XftFontClose(display, xft_font);
    XftColorFree(display, visual, colormap, &xft_color);
    XCloseDisplay(display);
    return 0;
}

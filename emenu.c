#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "drw.h"
#include "config.h"

#ifdef DEBUG
#define d(x) fputs(x "\n", stderr)
#else
#define d(x)
#endif
#define MIN(x, y) ((x < y) ? x : y)

static int pos = 0;
static char *text;
static int cursor = 0;
static int len = 0;
static int matched = 0;

void render_items(Drw *drw, Window win, char *items[], int count, int pos) {
    drw_rect(drw, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 0);

    drw_rect(drw, 0, 0, WINDOW_WIDTH, LINE_HEIGHT, 0, 0);
    if (len > 0) {
        char *print = malloc(len);
        if (print == NULL) {
            d("Error allocating");
            return;
        }
        strncpy(print, text, len);
        print[len] = 0;
        drw_text(drw, 0, 0, WINDOW_WIDTH, LINE_HEIGHT, 0, print, 0);

        char *upto = malloc(cursor);
        if (upto == NULL) {
            d("Error allocating");
            return;
        }
        strncpy(upto, print, cursor);
        free(print);
        upto[cursor] = 0;
        int width = drw_fontset_getwidth(drw, upto);
        drw_rect(drw, width, 0, 2, LINE_HEIGHT, 1, 0);
        free(upto);
    }

    int off = pos / LINES;
    int i;
    for (i = 0; i < MIN(count, LINES); i++) {
        if ((i + (off * LINES)) < count) {
            drw_rect(drw, 0, i * (LINE_HEIGHT) + LINE_HEIGHT, WINDOW_WIDTH, LINE_HEIGHT, 1, (i + (off * LINES)) != pos);
            drw_text(drw, 0, i * (LINE_HEIGHT) + LINE_HEIGHT, WINDOW_WIDTH, LINE_HEIGHT, 1, items[i + (off * LINES)], (i + (off * LINES)) != pos);
        }
    }
    drw_map(drw, win, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}
int main(int argc, char *argv[]) {
    int i;
    text = (char *) malloc(1);
    if (text == NULL) {
        d("Error allocating");
        return;
    }

    char **items = argv + 1;
    char **old_items = argv + 1;
    int count = argc - 1;
    matched = argc - 1;

    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        d("Could not open display");
        return -1;
    }
    d("Opened display");

    int screen_count;
    XineramaScreenInfo *screens = XineramaQueryScreens(dpy, &screen_count);
    
    int ptr_x, ptr_y;
    Window _root, _child;
    int _root_x, _root_y, _win_x, _win_y;
    unsigned int _mask;
    XQueryPointer(dpy, XDefaultRootWindow(dpy), &_root, &_child, &_root_x, &_root_y, &ptr_x, &ptr_y, &_mask);

    int base_x, win_width, win_height;
    XineramaScreenInfo screen = screens[0];
    for (i = 0; i < screen_count; screen = screens[++i]) {
        if (
                (ptr_x >= screen.x_org) && 
                (ptr_y >= screen.y_org) && 
                (ptr_x <= (screen.x_org + screen.width)) && 
                (ptr_y <= (screen.y_org + screen.height))) {
            win_width = screen.width;
            win_height = screen.height;
            base_x = screen.x_org;
            break;
        }
    }

    XWindowAttributes win_attrs;
    XGetWindowAttributes(dpy, XDefaultRootWindow(dpy), &win_attrs);
    int screen_num = XScreenNumberOfScreen(win_attrs.screen);
    d("Got window attributes");

    XSetWindowAttributes attrs;
    attrs.override_redirect = 1;
    Window win = XCreateWindow(
            dpy,
            XDefaultRootWindow(dpy),
            base_x + win_width / 2 - WINDOW_WIDTH / 2, win_height / 2 - WINDOW_HEIGHT / 2,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            0,
            win_attrs.depth,
            CopyFromParent, CopyFromParent,
            CWOverrideRedirect,
            &attrs);
    d("Created window");

    XGrabKeyboard(dpy, DefaultRootWindow(dpy), false, GrabModeAsync, GrabModeAsync, CurrentTime);
    XSelectInput(dpy, win, StructureNotifyMask | KeyPressMask);
    XMapWindow(dpy, win);
    d("Sent map request");

    XEvent ev;

    for(;;) {
        XNextEvent(dpy, &ev);

        if (ev.type == MapNotify) {
            d("Mapped window");
            break;
        }
    }

    Drw *drw = drw_create(
            dpy, 
            screen_num, 
            win,
            WINDOW_WIDTH, WINDOW_HEIGHT);
    Clr *scm = drw_scm_create(drw, colors, sizeof(colors) / sizeof(colors[0]));
    drw_setscheme(drw, scm);
    Fnt *fnt = drw_fontset_create(drw, fonts, sizeof(fonts) / sizeof(fonts[0]));
    drw_setfontset(drw, fnt);
    d("Created drawing context");

    render_items(drw, win, old_items, matched, pos);
    items = malloc(sizeof(char *) * matched);
    for (i = 0; i < matched; ++i) {
        items[i] = old_items[i];
    }

    bool running = true;
    KeySym keysym;
    char buf[25];

    while(running) {
        XNextEvent(dpy, &ev);

        if (ev.type == KeyPress) {
            memset(buf, 0, 25);
            XLookupString(&ev.xkey, buf, 25, &keysym, NULL);

            switch (keysym) {
            case XK_Up: 
                {
                    if (pos > 0) {
                        --pos;
                    }
                    break;
                }
            case XK_Down: 
                {
                    if (pos < (matched - 1)) {
                        ++pos;
                    }
                    break;
                }
            case XK_Escape:
                {
                    running = false;
                    break;
                }
            case XK_Return:
                {
                    puts(items[pos]);
                    running = false;
                    break;
                }
            case XK_BackSpace:
                {
                    if ((cursor > 0) && (len > 0)) {
                        char *old = malloc(len);
                        strcpy(old, text);

                        if (len > 1) {
                            if (realloc(text, --len) == NULL) {
                                d("Error allocating");
                            }

                            strncpy(text, old, --cursor);
                            //if (cursor > 0) {
                                strncpy(text + cursor, old + cursor + 1, strlen(old) - (cursor + 1));
                            //}
     
                            free(old);
                        } else {
                            memset(text, 0, len);
                            --cursor;
                            len = 0;
                        }
                    }

                    matched = 0;
                    int i;
                    for (i = 0; i < count; ++i) {
                        if (strlen(old_items[i]) >= len) {
                            if (memcmp(old_items[i], text, len) == 0) {
                                items[matched++] = old_items[i];
                            }
                        }
                    }
                    pos = 0;

                    break;
                }
            case XK_Left:
                {
                    if (cursor > 0) {
                        --cursor;
                    }
                    break;
                }
            case XK_Right:
                {
                    if (cursor < len) {
                        ++cursor;
                    }
                    break;
                }
            default:
                {
                    char *old = malloc(len);
                    if (old == NULL) {
                        d("Error allocating");
                        running = false;
                        break;
                    }
                    strcpy(old, text);
                    len += strlen(buf);
                    if (realloc(text, len) == NULL) {
                        d("Error allocating");
                        running = false;
                        break;
                    };
                    strncpy(text, old, cursor);
                    strncpy(text + cursor, buf, strlen(buf));
                    strncpy(text + cursor + strlen(buf), old + cursor, strlen(old) - cursor);
                    cursor += strlen(buf);
                    free(old);

                    matched = 0;
                    int i;
                    for (i = 0; i < count; ++i) {
                        if (strlen(old_items[i]) >= len) {
                            if (memcmp(old_items[i], text, len) == 0) {
                                items[matched++] = old_items[i];
                            }
                        }
                    }
                    pos = 0;

                    break;
                }
            }

            render_items(drw, win, items, matched, pos);
        }
    }
    
    XUngrabKeyboard(dpy, CurrentTime);
    XFlush(dpy);
    drw_free(drw);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    free(items);
    free(text);

    return 0;
}

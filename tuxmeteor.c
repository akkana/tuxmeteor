/*
 * Tux Meteor: a meteor counter for Unix.
 *
 * Copyright 2002 by Akkana Peck.
 * You may use, distribute, or modify this program under the terms of the GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>    /* for XComposeStatus */

static char gDir[BUFSIZ];
static char gFilename[BUFSIZ];

static FILE* logFP = 0;
static Display* dpy;
static Window win;
static int Count = 0;
static int screen;
static Atom wm_delete_atom;

static FILE* OpenLogFile()
{
    FILE* logFP;

    sprintf(gDir, "%s/.tuxmeteor", getenv("HOME"));
    if (access(gDir, W_OK) != 0)
    {
        if (mkdir(gDir, 0755) != 0)
        {
            sprintf(gFilename, "\07Can't create %s", gDir);
            perror(gFilename);
            exit(errno);
        }
    }

    sprintf(gFilename, "%s/meteors", gDir);  /* should add date/time */

    if ((logFP = fopen(gFilename, "a")) == 0)
    {
        sprintf(gDir, "\07Can't open %s", gFilename);
        perror(gDir);
        exit(errno);
    }

    return logFP;
}

void InitWindow()
{
    Atom wm_protocols[1];

    if ((dpy = XOpenDisplay(getenv("DISPLAY"))) == 0)
    {
        fprintf(stderr, "Can't open display: %s\n", getenv("DISPLAY"));
        exit(1);
    }
    screen = DefaultScreen(dpy);

    win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen),
                              0, 0,
                              DisplayWidth(dpy, screen),
                              DisplayHeight(dpy, screen),
                              3,
                              WhitePixel(dpy, screen),
                              BlackPixel(dpy, screen));
    if (!win) {
        fprintf(stderr, "Can't create window\n");
        exit(1);
    }

    XSelectInput(dpy, win, ButtonPressMask | KeyPressMask);

    wm_delete_atom = wm_protocols[0] = XInternAtom(dpy,
                                                   "WM_DELETE_WINDOW",
                                                   False);
    XSetWMProtocols(dpy, win, wm_protocols, 1);

    XMapWindow(dpy, win);
}

void LogMeteor(char ch)
{
    static GC gc = 0;
    time_t sec;
    static int x0, y0, w, h;
    static char str[]    = "Meteors counted: ____________________";

    ++Count;
    time(&sec);
    fprintf(logFP, "%c %s", ch, ctime(&sec));
    /* ctime already adds a newline, we don't need to */

    /* Flush the log file (in case we're not line buffered) every 200,
     * since we're probably running on a laptop which could lose power.
     */
    if (Count % 200 == 0)
        fflush(logFP);

    if (dpy) {
        if (!gc) {
            gc = XCreateGC(dpy, win,  0, 0);
            if (gc) {
                XGCValues values;
                Font font = XLoadFont(dpy, "-*-*-*-*-*-*-20-*-*-*-*-*-*-*");
                if (font) {
                    int direc, ascent, descent;
                    XCharStruct overall;
                    XQueryTextExtents(dpy, font, str, sizeof str - 1,
                                      &direc, &ascent, &descent, &overall);
                    w = overall.width;
                    h = ascent + descent;
                    x0 = (DisplayWidth(dpy, screen) - w) / 2;
                    y0 = (DisplayHeight(dpy, screen) - h) / 2;

                    XSetFont(dpy, gc, font);
                }
                else {
                    w = DisplayHeight(dpy, screen) / 3;
                    h = 25;
                    x0 = DisplayHeight(dpy, screen) / 3;
                    y0 = DisplayHeight(dpy, screen) / 2;
                }

                values.foreground = WhitePixel(dpy, screen);
                values.background = BlackPixel(dpy, screen);
                XChangeGC(dpy, gc, GCForeground|GCBackground, &values);
            }
        }
        
        if (gc) {
            sprintf(str+17, "%-20d", Count);
            //XClearArea(dpy, win, x0, y0, w, h, 0);
            //XDrawRectangle(dpy, win, gc, x0, y0, w, h);
            XDrawImageString(dpy, win, gc, x0, y0+h, str, (sizeof str)-1);
        }
    }
}

int HandleEvent()
{
    XEvent event;
    char buffer[20];
    KeySym keysym;
    XComposeStatus compose;

    XNextEvent(dpy, &event);
    switch (event.type)
    {
      case ButtonPress:
          LogMeteor('0' + event.xbutton.button);
          break;
      case KeyPress:
          //printf("KeyPress\n");
          XLookupString(&(event.xkey), buffer, sizeof buffer,
                        &keysym, &compose);
          switch (keysym)
          {
            case XK_q:
                return -1;
            case XK_Left:
                LogMeteor('<');
                break;
            case XK_Right:
                LogMeteor('>');
                break;
            case XK_Up:
                LogMeteor('^');
                break;
            case XK_Down:
                LogMeteor('v');
                break;
            default:
                LogMeteor(*buffer);
                break;
          }
          break;

      case ClientMessage:    /* Catch the WM_DELETE message */
          if (event.xclient.data.l[0] == wm_delete_atom)
              return -1;

      default:
          printf("Unknown event: %d\n", event.type);
          break;
    }
    return 0;
}

main()
{
    time_t sec;

    logFP = OpenLogFile();

    InitWindow();

    time(&sec);
    fprintf(logFP, "Meteor count started on %s", ctime(&sec));

    while (HandleEvent() >= 0)
        ;

    time(&sec);
    fprintf(logFP, "%d meteors counted\n", Count);
    fprintf(logFP, "Meteor count stopped on %s\n", ctime(&sec));
    fclose(logFP);
    printf("%d meteors counted, output written to %s\n", Count, gFilename);
}




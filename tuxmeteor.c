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
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#ifndef NO_X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>    /* for XComposeStatus */

static Display* dpy;
static Window win;
static Atom wm_delete_atom;
static int screen;
#endif /* NO_X11 */

static char gDir[PATH_MAX];
static char gFilename[PATH_MAX + FILENAME_MAX];
char errmsg[BUFSIZ + PATH_MAX + FILENAME_MAX];

static FILE* logFP = 0;
static int Count = 0;

static FILE* OpenLogFile()
{
    FILE* logFP;

    sprintf(gDir, "%s/.tuxmeteor", getenv("HOME"));
    if (access(gDir, W_OK) != 0)
    {
        if (mkdir(gDir, 0755) != 0)
        {
            sprintf(errmsg, "\07Can't create %s", gDir);
            perror(errmsg);
            exit(errno);
        }
    }

    sprintf(gFilename, "%s/meteors", gDir);  /* should add date/time */

    if ((logFP = fopen(gFilename, "a")) == 0)
    {
        sprintf(errmsg, "\07Can't open %s", gFilename);
        perror(errmsg);
        exit(errno);
    }

    return logFP;
}

int InitWindow()
{
#ifndef NO_X11
    Atom wm_protocols[1];

    if ((dpy = XOpenDisplay(getenv("DISPLAY"))) == 0)
        return -1;
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
    return 0;
#else /* NO_X11 */
    return -1;
#endif /* NO_X11 */
}

static struct termios termios_sav;

int InitTerminal()
{
    struct termios termios;
    if (tcgetattr(0, &termios) != 0) {
        perror("Couldn't get terminal modes!\n");
        return -1;
    }
    termios_sav = termios;
    cfmakeraw(&termios);
    if (tcsetattr(0, TCSANOW, &termios) != 0)
    {
        perror("Couldn't set raw mode!\n");
        return -1;
    }
    return 0;
}

void ResetTerminal()
{
    tcsetattr(0, TCSANOW, &termios_sav);
}

void LogMeteor(char ch)
{
#ifndef NO_X11
    static GC gc = 0;
    static int x0, y0, w, h;
    static char str[]    = "Meteors counted: ____________________";
#endif /* NO_X11 */
    time_t sec;

    ++Count;
    time(&sec);
    fprintf(logFP, "%c %s", ch, ctime(&sec));
    /* ctime already adds a newline, we don't need to */

    /* Flush the log file (in case we're not line buffered) every 200,
     * since we're probably running on a laptop which could lose power.
     */
    if (Count % 200 == 0)
        fflush(logFP);

#ifndef NO_X11
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
            XDrawImageString(dpy, win, gc, x0, y0+h, str, (sizeof str)-1);
        }
    }
#endif /* NO_X11 */
    printf("\rMeteors logged: %d ", Count);
}

int HandleEvent()
{
    static char pendingStr[15];
    static int pendingStrLen = 0;
    static const char *escSeq[] = {
        "\033[A", "\033[B", "\033[C", "\033[D"
    };
    static const char escMatches[] = {
        '^', 'v', '>', '<'
    };
#define NUM_ESC_SEQ ((sizeof escSeq / sizeof *escSeq))

    char c = getchar();

    /* If we have an escape sequence pending,
     * see if it matches any of our known escape sequences.
     */
    if (pendingStrLen > 0)
    {
        int i;
        int matching = 0;
        pendingStr[pendingStrLen++] = c;
        if (pendingStrLen > sizeof pendingStr) {  /* check for overflow */
            LogMeteor('?');
            pendingStrLen = 0;
            return 0;
        }
        for (i=0; i < NUM_ESC_SEQ; ++i) {
            if (!strncmp(escSeq[i], pendingStr, pendingStrLen)) {
                matching = 1;
                if (strlen(escSeq[i]) == pendingStrLen) {
                    LogMeteor(escMatches[i]);
                    pendingStrLen = 0;
                    return 0;
                }
            }
        }
        if (matching)
            return 0;

        /* If we don't match any known escape sequences,
         * then log entries for every keystroke we got.
         * Someone will have to sort it out later.
         */
        for (i=0; i<pendingStrLen; ++i)
            LogMeteor(pendingStr[i]);
        pendingStrLen = 0;
        return 0;
    }

    if (c == 'q')
        return -1;

    /* Try to deal with escape sequences -- start a "pending string". */
    if (c == '\033')  /* ESC */
    {
        pendingStr[pendingStrLen++] = c;
        return 0;
    }

    /* Else just log it, don't do anything special */
    LogMeteor(c);
    pendingStrLen = 0;
    return 0;
}

int XHandleEvent()
{
#ifndef NO_X11
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
#endif /* NO_X11 */
    return 0;
}

int main(int argc, char **argv)
{
    time_t sec;
    int i;
    int useX = 1;

    for (i=1; i<argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
              case 't':
                  useX = 0;
                  printf("Tux Meteor: Terminal Mode\n");
                  break;
              case 'v':
                  printf("Tux Meteor, v. %s.  ", VERSION);
                  printf("Copyright 2002, Akkana Peck\n");
                  printf("Usage: tuxmeteor [-tv]\n");
                  printf("  -t: use terminal mode (no X)\n");
                  exit(0);
            }
        }
    }

    logFP = OpenLogFile();

    if (!useX || ( InitWindow() != 0))
    {
        useX = 0;
        if (InitTerminal() != 0)
        {
            printf("Couldn't initialize either X or terminal -- exiting\n");
            exit(1);
        }
    }

    time(&sec);
    fprintf(logFP, "Meteor count started on %s", ctime(&sec));

    while ((useX ? XHandleEvent() : HandleEvent()) >= 0)
        ;

    time(&sec);
    fprintf(logFP, "%d meteors counted\n", Count);
    fprintf(logFP, "Meteor count stopped on %s\n", ctime(&sec));
    fclose(logFP);
    if (!useX)
        ResetTerminal();
    printf("%d meteors counted\nOutput written to %s\n", Count, gFilename);
    return 0;
}




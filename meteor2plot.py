#! /usr/bin/env python3

# Take a data file from tuxmeteor, and translate the date strings to unix time
# so they can be plotted by gnuplot.
#
# Usage: meteor2plot.py binsize filename
# (binsize is in seconds, default is 60)
#
# Copyright 2002 by Akkana Peck, you may reuse under the GPL, yada yada.

def main():
    import os
    import sys
    import string
    import time

    argc = 1

    if len(sys.argv) <= 1:
        binsize = 60
    else:
        try:
            binsize = int(sys.argv[1])
            argc = 2
            # you'd think ++argc would be better, but it's still 1 after that
        except ValueError:
            binsize = 60

    try:
        fp = open(sys.argv[argc], "r")
    except:
        fp = open("meteors", "r")

    curbin = 0;

    while True:
        line = fp.readline()
        if not line: break
        # Parse the time, which is in line[2:] if it's a time line
        try:
            datestr = line[2:].strip()
            # d = time.strptime(datestr, "%a %b %d %H:%M:%S %Y")
            d = time.strptime(datestr)
        except ValueError:
            continue

        curtime = time.mktime(d)
        if (curbin == 0):
            starttime = curtime
        if (int(curtime / binsize) > curbin):
            if (curbin > 0):
                print(curbin*binsize - starttime, count)
            count = 0
            curbin = int(curtime / binsize)

        count = count + 1

main()



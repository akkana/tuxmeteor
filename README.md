![TuxMeteor logo](tuxmeteor.jpg?raw=true "Tux Meteor")

Tux Meteor is a simple meteor counting program, written by Akkana Peck.

It logs the time of each meteor, and which key or button was used to
log it, so you can use different keys/buttons for different brightnesses
or parts of the sky.

Type "make" to build Tux Meteor.
By default, it will try to find your X11 libraries and build with them.
If it gets this wrong, or if you want to override the default and build
with or without X, see the comments at the beginning of the Makefile.

The binary is named tuxmeteor. You can run it from the source directory,
or run `make install` to copy it to /usr/local/bin.

When you run tuxmeteor, q will quit; any other key (or mouse button,
if you've built with X11) will count as a meteor, logged to
$HOME/.tuxmeteor/meteors.

`tuxmeteor -t` will run in terminal mode
(useful if you've built without X11 support).

The log file is $HOME/.tuxmeteor/meteors
It appends to the existing file if one is already there.

meteor2plot.py can take the output and turn it into a file suitable
for passing to a 2002-era gnuplot. (Run `meteor2plot.py binsize filename`.
binsize is in seconds, default is 60). Unfortunately gnuplot seems to have
changed since then, and neither meteor2plot.py nor plot.sh has been
updated since then. Patches appreciated!

More information on Tux Meteor:
http://shallowsky.com/software/tuxmeteor.html

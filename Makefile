tuxmeteor: tuxmeteor.o
	cc -o tuxmeteor tuxmeteor.o -L/usr/X11R6/lib -lX11

clean:
	rm *.o tuxmeteor

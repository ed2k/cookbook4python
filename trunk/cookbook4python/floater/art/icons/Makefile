.SUFFIXES: .gif .uu .xpm .png


.gif.uu:
	../../build/gif2uu < $<  > $@


all :: floater.ico floater-16.xpm floater-24.xpm floater-48.xpm floater-48.uu

floater.ico : floater-48.png floater-24.png floater-16.png
	icotool -c floater-48.png floater-24.png floater-16.png > floater.ico

floater-16.xpm : floater-16.png
	./pngtoxpm < floater-16.png > floater-16.xpm


floater-24.xpm : floater-24.png
	./pngtoxpm < floater-24.png > floater-24.xpm

floater-48.xpm : floater-48.png
	./pngtoxpm < floater-48.png > floater-48.xpm


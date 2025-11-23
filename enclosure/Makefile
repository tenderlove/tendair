OPENSCAD = /Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD

%.stl: main.scad
	$(OPENSCAD) -D'rendering="$(basename $@)"' $< -o $@

all: top.stl bottom.stl

clean:
	rm -f *.stl

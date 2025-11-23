OPENSCAD = /Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD

%.stl: main.scad
	$(OPENSCAD) -D'rendering="$(basename $@)"' $< -o $@

all: top.stl bottom.stl full.stl

clean:
	rm -f *.stl

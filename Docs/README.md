# Documentation quickstart

## Doxygen documentation
To build the doxygen documentation locally, run

```
doxygen doxyfile
```

in this folder. 
This will install the doxygen documentation in the `doxygen` subfolder. 

## Manual build
We use Sphinx with reStructured text for building the user documentation. 
To build the documentation locally, navigate to the Sphinx subfolder and run

```
cd Sphinx
make html
```

for the HTML documentation and

```
cd Sphinx
make latexpdf
```

for the LaTeX/PDF documentation.

Source files will end up in `Sphinx/build/html` and `Sphinx/build/latex`, respectively. 

## Adding Sphinx changes. 
To add changes to the Sphinx documentation, make the changes in the relevant Sphinx/source files.
To build the documentation, either build it locally as above or put Sphinx in auto-build mode:

```
cd Sphinx
sphinx-autobuild source/ build/html
```

This will start a server at http://127.0.0.1:8000 and start watching for changes in the `source` directory.
When a change is detected, the documentation is rebuilt and any open browser windows are reloaded automatically. `KeyboardInterrupt` (<kbd>ctrl</kbd>+<kbd>c</kbd>) will stop the server.


# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))
from sphinx.highlighting import PygmentsBridge
from pygments.formatters.latex import LatexFormatter

class CustomLatexFormatter(LatexFormatter):
    def __init__(self, **options):
        super(CustomLatexFormatter, self).__init__(**options)
        self.verboptions = r"formatcom=\scriptsize"

PygmentsBridge.latex_formatter = CustomLatexFormatter



# -- Project information -----------------------------------------------------

project = 'EBGeometry'
copyright = 'Robert Marskar / SINTEF Energy Research'
author = ''

html_show_copyright = False

# The full version, including alpha/beta/rc tags
#release = 'Latest'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.mathjax',
    'sphinxcontrib.bibtex'    
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# Bib file
bibtex_bibfiles = ['references.bib']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for LaTeX output ------------------------------------------------

# pdfLaTeX has no glyph for the box-drawing rule (U+2501) the performance probes print in their
# tables (PerformanceTesting.rst shows that output verbatim): the PDF shows it as '='. The HTML keeps
# the output exactly.
latex_elements = {
    'preamble': r'\DeclareUnicodeCharacter{2501}{=}',
}

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'


# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Render the TikZ figures (_static/<Name>/<Name>.tex -> _static/<Name>.png, git-ignored) when a build
# starts, so that sphinx-build, make html and sphinx-autobuild all have them. The script skips figures that
# are up to date. Without pdflatex and pdftoppm the figures are not built, and Sphinx reports each missing
# image as usual.
def build_figures(app):
    import shutil
    import subprocess
    from pathlib import Path

    script = Path(__file__).resolve().parents[3] / 'Scripts' / 'build-doc-figures.sh'

    if not (shutil.which('pdflatex') and shutil.which('pdftoppm')):
        print('build_figures: pdflatex or pdftoppm not found, figures not built')
        return

    if subprocess.run(['bash', str(script)]).returncode != 0:
        print('build_figures: ' + str(script) + ' failed; see its output above')


# Add CSS that fixes the sphinx_rtd_theme with equation number appearing above the equation. 
def setup(app):
    app.add_css_file('my_theme.css')
    app.connect('builder-inited', build_figures)

numfig = True
math_numfig = True
math_eqref_format = "Eq. {number}"

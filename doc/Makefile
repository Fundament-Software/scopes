# Makefile for MkDocs documentation
#

# You can set these variables from the command line.
BUILDDIR      = site
PYBUILDDIR    = site-packages

# Internal variables.
.PHONY: help clean html

help:
	@echo "Please use \`make <target>' where <target> is one of"
	@echo "  clean      to discard generated documentation"
	@echo "  html       to make standalone HTML files"
	@echo "  serve      to start a web server in preview mode for editing"

clean:
	rm -rf $(BUILDDIR)/* $(PYBUILDDIR)/*

html:
	pip3 install --upgrade mkdocs -t $(PYBUILDDIR)
	pip3 install --upgrade -I ./ScopesLexer -t $(PYBUILDDIR)
	PYTHONPATH="$(PYBUILDDIR);$(PYTHONPATH)" python3 -B -m mkdocs build -d "$(BUILDDIR)"
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)/."

serve:
	pip3 install --upgrade mkdocs -t $(PYBUILDDIR)
	pip3 install --upgrade -I ./ScopesLexer -t $(PYBUILDDIR)
	PYTHONPATH="$(PYBUILDDIR);$(PYTHONPATH)" python3 -B -m mkdocs serve

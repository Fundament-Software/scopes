# Makefile for MkDocs documentation
#

# You can set these variables from the command line.
MKDOCSOPTS    =
MKDOCS        = mkdocs
BUILDDIR      = site

# User-friendly check for MkDocs
ifeq ($(shell which $(MKDOCS) >/dev/null 2>&1; echo $$?), 1)
$(error The '$(MKDOCS)' command was not found. Make sure you have MkDocs installed, then set the MKDOCS environment variable to point to the full path of the '$(MKDOCS)' executable. Alternatively you can add the directory with the executable to your PATH. If you don't have MkDocs installed, grab it from https://www.mkdocs.org/)
endif

# Internal variables.
.PHONY: help clean html

help:
	@echo "Please use \`make <target>' where <target> is one of"
	@echo "  clean      to discard generated documentation"
	@echo "  html       to make standalone HTML files"

clean:
	rm -rf $(BUILDDIR)/*

html:
	$(MKDOCS) $(MKDOCSOPTS) build -d $(BUILDDIR)
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)/."

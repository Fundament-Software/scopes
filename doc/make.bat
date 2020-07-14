@ECHO OFF

REM Command file for MkDocs documentation

if "%MKDOCS%" == "" (
	set MKDOCS=mkdocs
)
set BUILDDIR=site
set MKDOCSOPTS=

if "%1" == "" goto help

if "%1" == "help" (
	:help
	echo.Please use `make ^<target^>` where ^<target^> is one of
	echo.  clean      to discard generated documentation
	echo.  html       to make standalone HTML files
	goto end
)

if "%1" == "clean" (
	for /d %%i in (%BUILDDIR%\*) do rmdir /q /s %%i
	del /q /s %BUILDDIR%\*
	goto end
)

%MKDOCS% 1> nul 2> nul
if errorlevel 9009 (
	echo.
	echo.The 'mkdocs' command was not found. Make sure you have mkdocs
	echo.installed, then set the MKDOCS environment variable to point
	echo.to the full path of the 'mkdocs' executable. Alternatively you
	echo.may add the mkdocs directory to PATH.
	echo.
	echo.If you don't have MkDocs installed, grab it from
	echo.https://www.mkdocs.org/
	exit /b 1
)

if "%1" == "html" (
	%MKDOCS% %MKDOCSOPTS% build -d %BUILDDIR%
	if errorlevel 1 exit /b 1
	echo.
	echo.Build finished. The HTML pages are in %BUILDDIR%/.
	goto end
)

:end

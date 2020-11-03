# Copyright(c)'2011 by The Givaro group
# Written by Brice Boyer (briceboyer) <boyer.brice@gmail.com>
# This file is part of Givaro.
# Givaro is governed by the CeCILL-B license under French law
# and abiding by the rules of distribution of free software.
# see the COPYRIGHT file for more details.






AC_DEFUN([GIV_DOC],
[

AC_MSG_CHECKING(whether to build documentation)


AC_ARG_WITH(docdir,
[AC_HELP_STRING([--with-docdir=<path>], [Where the Givaro documentation should be installed])],
            [
		GIVARO_DOC_PATH="$withval"
	    ],
	    [
		eval GIVARO_DOC_PATH="${prefix}/docs"
	    ])

AC_SUBST(GIVARO_DOC_PATH)

AC_ARG_WITH(doxygen,
[AC_HELP_STRING([--with-doxygen=<path>], [Give the path to Doxygen. Note: --enable-doc needed])],
            [
		DOXYGEN_PATH="$PATH:$withval"
	    ],
	    [
		DOXYGEN_PATH="$PATH"
	    ])

AC_ARG_ENABLE(doc,[AC_HELP_STRING([--enable-doc], [Enable building documentation])],
[
AC_MSG_RESULT(yes)
AC_MSG_CHECKING(whether doxygen works)
export PATH=$DOXYGEN_PATH
(doxygen --version) < /dev/null > /dev/null 2>&1 || {
	AC_MSG_RESULT(no)
	echo
	echo "You must have doxygen installed to create documentation for"
	echo "Givaro. This error only happens if you use --enable-doc."
	echo "Download the appropriate package for your distribution, or get"
	echo "the source tarball from http://www.stack.nl/~dimitri/doxygen/"
	exit -1
}
AM_CONDITIONAL(GIVARO_DOXYGEN_FOUND,     true)
AC_MSG_RESULT(yes)
AM_CONDITIONAL(GIVARO_BUILD_DOC, true)

],
[
AS_IF([(doxygen --version) < /dev/null > /dev/null 2>&1],
[AM_CONDITIONAL(GIVARO_DOXYGEN_FOUND,  true)],
[AM_CONDITIONAL(GIVARO_DOXYGEN_FOUND, false)]
)
AC_MSG_RESULT(no)
AM_CONDITIONAL(GIVARO_BUILD_DOC, false)
])

AC_MSG_CHECKING(whether dot works)
res=yes;
(dot -V) < /dev/null > /dev/null 2>&1 || res=no
AC_MSG_RESULT([$res])
AS_MKDIR_P([docs])
AS_IF([test $res = yes],
[
sed 's/^HAVE_DOT.*/HAVE_DOT = YES/' "$srcdir"/docs/Doxyfile.mod > docs/Doxyfile
sed 's/^HAVE_DOT.*/HAVE_DOT = YES/' "$srcdir"/docs/DoxyfileDev.mod > docs/DoxyfileDev
],
[ cp "$srcdir"/docs/Doxyfile.mod docs/Doxyfile ;
cp "$srcdir"/docs/DoxyfileDev.mod docs/DoxyfileDev
])


])

dnl Copyright(c)'2011 LinBox
dnl Written by Brice Boyer (briceboyer) <boyer.brice@gmail.com>
dnl This file is part of LinBox.

 dnl ========LICENCE========
 dnl This file is part of the library LinBox.
 dnl
 dnl LinBox is free software: you can redistribute it and/or modify
 dnl it under the terms of the  GNU Lesser General Public
 dnl License as published by the Free Software Foundation; either
 dnl version 2.1 of the License, or (at your option) any later version.
 dnl
 dnl This library is distributed in the hope that it will be useful,
 dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
 dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 dnl Lesser General Public License for more details.
 dnl
 dnl You should have received a copy of the GNU Lesser General Public
 dnl License along with this library; if not, write to the Free Software
 dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 dnl ========LICENCE========
 dnl.

dnl enable basic debug mode.
AC_DEFUN([AC_DEBUG],
[AC_MSG_CHECKING([whether to enable debugging options in the library])
  AC_ARG_ENABLE(debug,
[AC_HELP_STRING([--enable-debug=yes|no], [enable debugging options in library])],
      USE_DEBUG=$enableval,
      USE_DEBUG=no)
  AC_MSG_RESULT([$USE_DEBUG])
  AM_CONDITIONAL(DEBUG, [test x$USE_DEBUG = xyes])
  DBG=$USE_DEBUG
  AC_SUBST(DBG)dnl
]
)

AC_DEFUN([AC_PROFILE],
[AC_MSG_CHECKING([whether to enable profiling everything in the library])
  AC_ARG_ENABLE(profile,
[AC_HELP_STRING([--enable-profile=yes|no], [enable profiling options in library])],
      USE_PROFILE=$enableval,
      USE_PROFILE=no)
  AC_MSG_RESULT([$USE_PROFILE])
  AM_CONDITIONAL(PROFILE, [test $USE_PROFILE = yes])
  PROF=$USE_PROFILE
  AC_SUBST(PROF)dnl
]
)

dnl Enable warnings from compiler.
AC_DEFUN([AC_WARNINGS],
[AC_MSG_CHECKING([whether to enable warnings when compiling the library])
  AC_ARG_ENABLE(warnings,
[AC_HELP_STRING([--enable-warnings=yes|full|no],  [enable warnings when compiling the library.
If nothing or yes is given, more aggressive compiler warnings are passed to the compiler.
If full is given, we become paranoïd about warnings and treat them as errors.])],
      USE_WARNINGS=$enableval,
      USE_WARNINGS=no)
  AC_MSG_RESULT([$USE_WARNINGS])
  dnl  AM_CONDITIONAL(WARNINGS, [test $USE_WARNINGS = yes])
  WARN=$USE_WARNINGS
  AC_SUBST(WARN)dnl
]dnl
)dnl

CCNAM=""

AC_DEFUN([AC_COMPILER_NAME], [
		AC_MSG_CHECKING(for family name of compiler)

		dnl CHECKING for various compilers
		dnl ICC ?
		AC_TRY_RUN( [
           #ifdef __INTEL_COMPILER
				int main() { return 0 ; }
			#else
			   pas intel
		   #endif],
		[ AC_MSG_RESULT(icc)
		CCNAM=icc
		AC_SUBST(CCNAM)
		])

dnl PATHSCALE > 4 ?
		AS_IF([ test -z "${CCNAM}"], [
			AC_TRY_RUN( [
				#ifdef __PATHSCALE__
				   int main() { return !(__PATHCC__ >= 4) ; }
			   #else
				   pas ekopath non plus.
				#endif], [
		AC_MSG_RESULT(eko)
		CCNAM=eko
		AC_SUBST(CCNAM) ])
		])

dnl CLANG >= 3.9 ?
		AS_IF([ test -z "${CCNAM}"], [
			AC_TRY_RUN( [
				#ifdef __clang__
				   int main() { return !((__clang_major__ >= 4) || (__clang_major__  >=3 && __clang_minor__ >= 9)) ; }
			   #else
				   not clang3.9
				#endif], [
		AC_MSG_RESULT(clang)
		CCNAM=clang
		AC_SUBST(CCNAM) ])
		])
dnl 3.1 < CLANG <=  3.8 ?
		AS_IF([ test -z "${CCNAM}"], [
			AC_TRY_RUN( [
				#ifdef __clang__
				   int main() { return !(__clang_major__  ==3 && __clang_minor__ >=1 && __clang_minor__ <=8) ; }
			   #else
				   not clang3.8s
				#endif], [
		AC_MSG_RESULT(clang38)
		CCNAM=clang38
		AC_SUBST(CCNAM) ])
		])


dnl GCC >= 4.8 ?
		AS_IF([ test -z "${CCNAM}"], [
			AC_TRY_RUN( [
				#ifdef __GNUC__
				   int main() { return !(__GNUC__ >= 5 || (__GNUC__ == 4  && __GNUC_MINOR__ > 7 )) ; }
				#else
				   pas gcc non plus ???
				#endif], [
		CCNOM=gcc
		AS_IF([ test -n "${CC}" ], [CCNOM="`$CC --version 2>&1|  awk 'NR<2{print $1}'`"])
		CCNAM=gcc48
		AC_SUBST(CCNAM)
		AC_MSG_RESULT($CCNOM)
		])
		])
dnl GCC > 4.4 ?
		AS_IF([ test -z "${CCNAM}"], [
			AC_TRY_RUN( [
				#ifdef __GNUC__
				   int main() { return !( __GNUC__ == 4 && __GNUC_MINOR__ == 4 ) ; }
				#else
				   pas gcc non plus ???
				#endif], [
		CCNOM=gcc
		AS_IF([ test -n "${CC}" ], [CCNOM="`$CC --version 2>&1|  awk 'NR<2{print $1}'`"])
		CCNAM=gcc44
		AC_SUBST(CCNAM)
		AC_MSG_RESULT($CCNOM)
		])
		])
dnl GCC > 4.2 ?
		AS_IF([ test -z "${CCNAM}"], [
			AC_TRY_RUN( [
				#ifdef __GNUC__
				   int main() { return !(__GNUC__ == 4 && __GNUC_MINOR__ > 2) ; }
				#else
				   pas gcc non plus ???
				#endif], [
		CCNOM=gcc
		AS_IF([ test -n "${CC}" ], [CCNOM="`$CC --version 2>&1|  awk 'NR<2{print $1}'`"])
		CCNAM=gcc
		AC_SUBST(CCNAM)
		AC_MSG_RESULT($CCNOM)
		])
		])

		dnl  autre ?

		AS_IF([ test -z "${CCNAM}"],
				[ AC_MSG_RESULT(unknown)
				CCNAM=unknown
				AC_SUBST(CCNAM)
				echo
				echo " *** unknow compiler. please file a bug "
				echo
				])
])


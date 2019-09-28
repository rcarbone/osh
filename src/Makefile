#
# osh - A shell for Oracle
#
# R. Carbone (rocco@tecsiel.it)
# 2Q 2019
#
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#


# Project directories
ROOTDIR   = ..

# Level 1
PUBDIR    = ${ROOTDIR}/3rdparty
SHELLDIR  = ${ROOTDIR}/shell

# Level 2
RLIBCDIR  = ${PUBDIR}/rlibc/src

# Oracle - Basic client and SDK libraries
ORACLEDIR = ${PUBDIR}/oracle/instantclient_12_2

# OCILIB - C Driver for Oracle
OCILIBDIR = ${PUBDIR}/ocilib

#
# External software modules
#

# Oracle - Basic client and SDK libraries
INCLUDE  += -I${ORACLEDIR}/sdk/include

# OCILIB - C Driver for Oracle
INCLUDE  += -I${OCILIBDIR}/include
OCILIB    = ${OCILIBDIR}/src/.libs/libocilib.a

# rlibc
INCLUDE  += -I${RLIBCDIR}
LIBRLIBC  = ${RLIBCDIR}/librlibc.a

# shell
INCLUDE  += -I${SHELLDIR}

# C source files
MAINSRCS += try-link.c

# tcsh-specific
LIBSRCS  += tcsh-wrap.c
LIBSRCS  += init.c
LIBSRCS  += prompt.c

# Containers
LIBSRCS  += memory.c
LIBSRCS  += commands.c
LIBSRCS  += connections.c

# Ocilib Library
LIBSRCS  += ocilib.c

# Helpers
LIBSRCS  += help.c
LIBSRCS  += about.c
LIBSRCS  += version.c
LIBSRCS  += license.c
LIBSRCS  += when.c

# Connections
LIBSRCS  += connect.c
LIBSRCS  += disconnect.c
LIBSRCS  += list.c
LIBSRCS  += change.c

# User Tables
LIBSRCS  += user-tables.c
LIBSRCS  += describe.c

# Viewers
LIBSRCS  += select.c
LIBSRCS  += curses.c

# Applications
LIBSRCS  += ping.c

# The name of the games
LIBNAME   = osh
PROGRAMS += ${MAINSRCS:%.c=%}

# All C source files
SRCS      = ${LIBSRCS} ${MAINSRCS}

# All libraries
STLIB     = lib${LIBNAME}.a
# SHLIB     = ${STLIB:%.a=%.so}

# All targets
TARGETS   = ${STLIB} ${SHLIB} ${PROGRAMS}

# Object and depend files
LIBOBJS   = ${LIBSRCS:%.c=%.o}
MAINOBJS  = ${MAINSRCS:%.c=%.o}
OBJS      = ${LIBOBJS} ${MAINOBJS}
DEPS      = ${SRCS:%.c=%.M}

# C/C++ Compilers and flags
INCLUDES  = -I./
INCLUDES += ${INCLUDE}
CC        = gcc
AR        = ar
LD        = gcc
CFLAGS   += -g -Wall
CFLAGS   += -fPIC
CFLAGS   += -fno-diagnostics-color
CFLAGS   += ${INCLUDES}
ARFLAGS   = rv
LDFLAGS   =

# User and System Libraries
USRLIBS  += ${STLIB}
USRLIBS  += ${OCILIB}
USRLIBS  += ${LIBRLIBC}
SYSLIBS  += -L${ORACLEDIR} -lclntsh
SYSLIBS  += -lcurses

# The main target is responsible to make all
all: ${TARGETS}

# Static library
${STLIB}: ${LIBOBJS}
	@echo "=*= making static library $@ =*="
	@${AR} ${ARFLAGS} $@ $^ 1> /dev/null 2>& 1

# Shared library
${SHLIB}: ${LIBOBJS}
	@echo "=*= making shared library $@ =*="
	@${CC} -shared $^ ${USRLIBS} ${SYSLIBS} -o $@

# Binary programs
%: %.o ${USRLIBS}
	@echo "=*= making program $@ =*="
	@${LD} ${LDFLAGS} $^ ${SYSLIBS} -o $@

# Cleanup rules
clean:
	@rm -f ${TARGETS}
	@rm -f ${OBJS}
	@rm -f *~

distclean: clean
	@rm -f ${DEPS}

# How to make an object file from a C source
%.o: %.c
	@echo "=*= making object $@ =*="
	@${CC} -c ${CFLAGS} $<

# How to make a depend file from a C source
%.M: %.c
	@-${CC} -MM ${INCLUDES} $< -o $@

-include ${DEPS}

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
PARSER = ../etc/parse_valgrind_suppressions.awk

# Create a raw suppression file
%.X: %
	@echo -n "=*= making suppression file $@ ... "
	@rm -f $^.o
	@${CC} -c ${CFLAGS} $^.c
	@valgrind --leak-check=full --show-reachable=yes --error-limit=no --gen-suppressions=all --log-file=$^.log $^ > /dev/null
	@echo -n "parsing ... "
	@cat $^.log | ${PARSER} > $@
	@echo "done!"
	@rm -f $^.log
	@rm -f $^.o
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

# Check for errors and memory leaks
VFLAGS   += --show-leak-kinds=all
VFLAGS   += --leak-check=full
VFLAGS   += --error-exitcode=1
SUPPRESS = ../etc/oracle.X

# Run valgrind to check for errors and memory leaks
leaks: ${PROGRAMS}
	@echo "=*= ckecking programs for errors with valgrind =*="
	@for bin in ${PROGRAMS} ; do \
           echo "=*= running program [$$bin] =*=" ; \
           make -s leak-$$bin ; \
         done

vleaks: ${PROGRAMS}
	@echo "=*= ckecking programs for errors with valgrind =*="
	@for bin in ${PROGRAMS} ; do \
           make -s vleak-$$bin ; \
         done

# Run valgrind (no suppression file)
leak-%: %
	@echo -n "  $^ ... "
	@valgrind -q ${VFLAGS} $^ 1> /dev/null 2>& 1
	@if [ $? ] ; then echo -n "Ok" ; fi
	@echo

vleak-%: %
	@echo -n "  $^ "
	@valgrind -q ${VFLAGS} $^
	@if [ $? ] ; then echo -n "Ok" ; fi
	@echo

# Run valgrind (using raw suppression file)
check-%: %
	@echo "=*= ckecking $^ (suppression file $^.X) =*="
	@echo
	@valgrind ${VFLAGS} --suppressions=$^.X $^

checks: ${PROGRAMS}
	@for bin in ${PROGRAMS} ; do \
           echo "=*= checking program $$bin for errors =*=" ; \
           make check-$$bin ; \
         done

# The help target
help:
	@echo "Usage:"
	@echo "  make all       --> compile only (just to check for C program syntax)"
	@echo "  make X         --> make suppression files [*.X] (Damned Oracle!)"
	@echo "  make leaks     --> ckeck for memory leaks (no suppression files)"
	@echo "  make checks    --> ckeck for memory leaks using suppression files *.X"
	@echo
	@echo "  make leaks-v-X --> ckeck for memory leaks just in module v-X"
	@echo
	@echo "  The following modules have been currently defined:"
	@echo -n "   "
	@for p in ${PROGRAMS} ; do \
           echo -n "$$p " ; \
         done
	@echo

.DEFAULT:
	@echo "No rule has been defined to make: $@"

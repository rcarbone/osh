#
# osh - A shell for Oracle
#
# R. Carbone (rocco@tecsiel.it)
# 2Q 2019
#
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#

# The name of the game
PACKAGE  = osh

# Root installation directory
INSTDIR = /usr/local/${PACKAGE}

# The list of directories where to make something
SUBDIRS += src
SUBDIRS += shell
SUBDIRS += at-testsuite

# The main target is responsible to recursively scan subdirectories and build all the modules
all: shell/Makefile ${SUBDIRS}
	@for dir in ${SUBDIRS} ; do \
           if [ -d $$dir ] ; then \
             if [ -f $$dir/Makefile ] ; then \
               echo "Making directory $$dir ..." ; \
               make -C $$dir -s --no-print-directory $@ ; \
             fi ; \
           else \
             echo "Warning: missing directory $$dir" ; \
           fi \
         done

shell/Makefile:
	@./configure

# Cleanup rules
clean distclean:
	@rm -f *~
	@for dir in ${SUBDIRS} ; do \
           if [ -d $$dir ] ; then \
             if [ -f $$dir/Makefile ] ; then \
               make -C $$dir -s --no-print-directory $@ ; \
             fi \
           fi \
         done

# Targets that are eventually handled by recursive Makefile(s)
%:
	@for dir in ${SUBDIRS} ; do \
           if [ -d $$dir ] ; then \
             echo $$dir; \
             if [ -f $$dir/Makefile ] ; then \
               make -C $$dir -s --no-print-directory $@ ; \
               echo ; \
             fi ; \
           else \
             echo "Warning: missing directory $$dir" ; \
           fi \
         done

# Fake targets
.PHONY: all clean distclean ${SUBDIRS}

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

help:
	@echo "Usage:"
	@echo "  make [all]          --> compile all"
	@echo "  make clean          --> clean all locally generated files"
	@echo "  make distclean      --> clean all locally generated files (included all temporary)"
#
	@echo "  make fs             --> create all installation directories"
	@echo "  make tree           --> list contents of installation directories in a tree-like format"
	@echo "  make install        --> install all distribution files in ${INSTDIR}"

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

# run-time directories
BINDIR   = bin
DOCSDIR  = docs
ETCDIR   = etc
LIBDIR   = lib
MANDIR   = man

# all directories
DIRS    += ${BINDIR}
DIRS    += ${DOCSDIR}
DIRS    += ${ETCDIR}
DIRS    += ${LIBDIR}
DIRS    += ${MANDIR}

# Create run-time directories
fs:
	@if [ ! -d ${INSTDIR} ] ; then echo -n "== Creating run-time filesystem tree ... " ; fi
	@for d in ${DIRS} ; do \
           if [ ! -d ${INSTDIR}/$$d ] ; then mkdir -p ${INSTDIR}/$$d && chmod 775 ${INSTDIR}/$$d ; fi \
         done
	@if [ ! -d ${INSTDIR} ] ; then echo "done! ==" ; fi

# List run-time directories
tree:
	@tree --charset ascii ${INSTDIR}

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
copy:
	@for src in ${SRCFILES} ; do \
           dstdir=${INSTDIR}/${DSTDIR} ; \
           dst=`basename $$src` ; \
           if [ -f $$src ] ; then \
             if [ -f $$dstdir/$$dst ]; then \
               cmp -s $$src $$dstdir/$$dst ; \
               if [ $$? != 0 ]; then \
                 cp -f $$src $$dstdir/$$dst ; \
                 chmod ${MODE} $$dstdir/$$dst ; \
               fi \
             else \
               cp -f $$src $$dstdir/$$dst ; \
               chmod ${MODE} $$dstdir/$$dst ; \
             fi \
           else \
             echo "Warning: missing source file [$$src]" ; \
           fi \
         done

# Install all
install: all install-libs install-bins install-docs install-files

# Install libraries
LIBS   += 3rdparty/oracle/instantclient_12_2/libclntsh.so.12.1
LIBS   += 3rdparty/oracle/instantclient_12_2/libmql1.so
LIBS   += 3rdparty/oracle/instantclient_12_2/libipc1.so
LIBS   += 3rdparty/oracle/instantclient_12_2/libnnz12.so
LIBS   += 3rdparty/oracle/instantclient_12_2/libons.so
LIBS   += 3rdparty/oracle/instantclient_12_2/libclntshcore.so.12.1
install-libs: fs
	@echo -n "== Installing libraries ... "
	@make -s SRCFILES="${LIBS}" DSTDIR=${LIBDIR} MODE=644 copy
	@echo "done! =="

# Install binaries
BINS   += shell/osh
install-bins: fs
	@echo -n "== Installing binaries .... "
	@make -s SRCFILES="${BINS}" DSTDIR=${BINDIR} MODE=755 copy
	@echo "done! =="

# Install docs
DOCS   += docs/README
DOCS   += docs/AUTHORS
DOCS   += docs/LICENSE
DOCS   += docs/VERSION
DOCS   += docs/ChangeLog
DOCS   += docs/LICENSE-Oracle
DOCS   += docs/LICENSE-ocilib
DOCS   += docs/Copyright-tcsh
install-docs: fs
	@echo -n "== Installing docs ........ "
	@make -s SRCFILES="${DOCS}" DSTDIR=${DOCSDIR} MODE=644 copy
	@echo "done! =="

# Install files
FILES  += etc/tnsnames.ora
install-files: fs
	@echo -n "== Installing files ....... "
	@make -s SRCFILES="${FILES}" DSTDIR=${ETCDIR} MODE=644 copy
	@echo "done! =="

# Fake targets
.PHONY: help fs tree copy install install-libs install-bins install-docs install-files

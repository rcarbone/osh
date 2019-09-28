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

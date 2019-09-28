#
# osh - A shell for Oracle
#
# R. Carbone (rocco@tecsiel.it)
# 2Q 2019
#
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#

ATFILES += help.at
ATFILES += about.at
ATFILES += version.at
ATFILES += license.at
ATFILES += when.at
ATFILES += connect.at
ATFILES += disconnect.at
ATFILES += list.at
ATFILES += change.at
ATFILES += tables.at
ATFILES += describe.at
ATFILES += select.at
ATFILES += ping.at

all: testsuite

testsuite: package.m4 testsuite.at ${ATFILES}
	@echo -n "Making testsuite script ... "
	@autom4te --language=autotest -I./ testsuite.at -o $@
	@echo "done!"

run: testsuite
	@echo "Running testsuite ... "
	@sh $^

clean:
	@rm -f testsuite.log
	@rm -rf testsuite.dir
	@rm -f testsuite
	@rm -f *~

distclean: clean

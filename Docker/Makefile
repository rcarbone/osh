#
# osh - A shell for Oracle
#
# R. Carbone (rocco@tecsiel.it)
# 2Q 2019
#
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#

# The name of the game
PACKAGE    = osh
USER       = roccocarbone
REPOSITORY = ${PACKAGE}
TAG        = unstable-0.1.0
IMAGE      = ${REPOSITORY}:${TAG}

# Project directories
ROOTDIR   = ..

# Build a Docker image using the Dockerfile at current location
image: install
	@echo "Build Docker image ${IMAGE} using Dockerfile at current location ... "
	@docker build --quiet --tag ${USER}/${IMAGE} ./
#	@docker build --tag ${USER}/${IMAGE} ./
	@rm -rf ${PACKAGE}

install:
	@echo -n "Installing [${PACKAGE}] in `pwd`/${PACKAGE} ... "
	@make -s -C ${ROOTDIR} INSTDIR=`pwd`/${PACKAGE} install > /dev/null
	@echo "Done!"

run:
	@docker run -it ${USER}/${IMAGE}

tag:
	@docker tag ${IMAGE} ${USER}/${IMAGE}

push:
	@docker push ${USER}/${IMAGE}

ls:
	@docker image ls

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
VERSION    = latest
DISTRO     = debian
USER       = roccocarbone
SILENT     = --quiet

# Local image name
REPOSITORY = ${PACKAGE}
TAG        = ${VERSION}
IMAGE      = ${REPOSITORY}:${TAG}

# Dockerhub image name
SRCIMG     = ${IMAGE}
DSTIMG     = ${USER}/${IMAGE}

# Project directories
ROOTDIR   = ..

# Build a Docker image using the Dockerfile at current location
image: install
	@echo -n "Building docker container ${IMAGE} using Dockerfile at current location ... "
	@docker build ${SILENT} --tag ${IMAGE} ./
	@rm -rf ${PACKAGE}

# install hacca locally
install:
	@echo -n "Installing [${PACKAGE}] in `pwd`/${PACKAGE} ... "
	@make -s -C ${ROOTDIR} INSTDIR=`pwd`/${PACKAGE} install > /dev/null
	@echo "Done!"

run:
	@docker run -it ${IMAGE}

tag: image
	@docker tag ${SRCIMG} ${DSTIMG}

push: tag
	@docker push ${DSTIMG}
	@docker rmi ${DSTIMG}

ls:
	@docker image ls

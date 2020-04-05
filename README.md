# Docker container for the impatients

If you are impatient and not interested in the full story, here is the download button for a docker image with **osh** in an interactive container based on GNU/Debian bullseye-slim (to be minimalist!):
```
    https://hub.docker.com/r/roccocarbone/osh
```

The container is interactive, meaning that the Docker-CLI will talk directly with the osh shell included in the image.
It can be easily pulled and run using the following commands:

* docker pull roccocarbone/osh
* docker run -it roccocarbone/osh

```
   user@somehost 1> docker pull roccocarbone/osh
   Using default tag: latest
   latest: Pulling from roccocarbone/osh
   5eabfc6d6a4a: Already exists 
   a791fcf6d08d: Pull complete 
   37e0579658b0: Pull complete 
   a325dfeceb58: Pull complete 
   Digest: sha256:1713aa72b97604073f42c91df96dee0bfcb29a6a03aaeb0cc2f26d09da81167e
   Status: Downloaded newer image for roccocarbone/osh:latest
   docker.io/roccocarbone/osh:latest

   user@somehost 2> docker image ls
   REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
   roccocarbone/osh    latest              e645fd5ad43c        24 hours ago        331MB

   user@somehost 3> docker run -it roccocarbone/osh


   -- osh 0.1.0 (Apr  2 2020) -- R. Carbone (rocco@tecsiel.it)
   A hack of the popular 'tcsh' with builtin extensions for Oracle client

   Type 'help' for the list of builtin extensions implemented by this shell.

   osh 1> about 
   osh, ver. 0.1.0 built for Linux-x86_64 on Apr  2 2020 23:12:29
   Copyright(c) 2019 R. Carbone (rocco@tecsiel.it)
   License: BSD-2-Clause-FreeBSD

   osh is provided AS IS and comes with ABSOLUTELY NO WARRANTY.
   This is free software, and you are welcome to redistribute it under the terms of the license.

   Based on:
     tcsh v. 6.21.0  - C shell with file name completion and command line editing - Christos Zoulas <christos@NetBSD.org>
     OCILIB v. 4.6.6 - C Driver for Oracle - Vincent Rogier <vince.rogier@ocilib.net>
     OCI v. 12.2     - Oracle Instant Client and Call Interface - Copyright (c) 1995, 2019 Oracle
```

# What is osh?
**osh** is a Linux shell for Oracle, implemented as a clone of the **tcsh** shell with extensions to add SQL commands. **osh** may be used as both an interactive tool for interacting with Oaracle Database servers and an interpreter for the execution of testsuites.


# About
osh is a hack of the **tcsh** shell mainly implemented to include Oracle client functionalities into a shell in order to have support for:

  * Multiple Oracle connections
  * Command line TAB-completion for commands, tables and columns
  * History
  * Variables
  * I/O redirections and pipeline

# Do it by yourself

If you are impatient and not interested in the full story, here is the download button:

```
   1> git clone https://github.com/rcarbone/osh
   2> cd osh
   3> export ORACLE_HOME=`pwd`/3rdparty/oracle/instantclient_12_2
   4> export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${ORACLE_HOME}
   5> ./configure
   6> make
```

You can also want to set your own configuration file [tnsnames.ora].
Edit the file in etc/ to match your environment and set the environment variable:

```
   7> export TNS_ADMIN=`pwd`/etc/tnsnames.ora
   8> cd shell
   9> ./osh
```

# Home Page
If you are interested in a working in progress documentation please point your browser to:
  http://tecsiel.it/osh

# License
**osh** is distributed under the BSD License, meaning that you are free to redistribute, modify, or sell the software with almost no restrictions.

BSD 2-Clause FreeBSD License <http://www.freebsd.org/copyright/freebsd-license.html>

osh is based on and utilizes copyrighted software:
  tcsh v. 6.21.0  - C shell with file name completion and command line editing - Christos Zoulas <christos@NetBSD.org>
  OCILIB v. 4.6.6 - C Driver for Oracle - Vincent Rogier <vince.rogier@ocilib.net>
  OCI v. 12.2     - Oracle Instant Client and Call Interface - Copyright (c) 1995, 2019 Oracle

As with all Docker images, these likely also contain other software which may be under other licenses.
Additional license information of each additional used software can be found in the docs/ directory.

As for any pre-built image usage, it is the image user's responsibility to ensure that any use of this image complies with any relevant licenses for all software contained within.

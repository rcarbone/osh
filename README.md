# osh
osh - a Linux shell for Oracle

# About
osh is a hack of the Unix shell tcsh mainly implemented to include Oracle client
functionalities into a shell in order to have support for:

  * Multiple Oracle connections
  * Command line TAB-completion for commands, tables and columns
  * History
  * Variables
  * I/O redirections and pipeline


# Impatients

If you are impatient and not interested in the full story,
here is the download button:

   1> git clone https://github.com/rcarbone/osh

   2> cd osh

   3> export ORACLE_HOME=`pwd`/3rdparty/oracle/instantclient_12_2

   4> export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${ORACLE_HOME}

   5> ./configure

   6> make

You can also want to set your own configuration file [tnsnames.ora].
Edit the file in etc/ to match your environment and set the environment variable:

   7> export TNS_ADMIN=`pwd`/etc/tnsnames.ora

   8> cd shell

   9> ./osh


# Home Page
If you are interested in a working in progress documentation please point your browser to:
  http://tecsiel.it/osh

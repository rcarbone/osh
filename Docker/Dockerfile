FROM debian:bullseye

MAINTAINER rocco <rocco@tecsiel.it>

ENV PROJECT osh
ENV USER    osh
ENV SHELL   osh
ENV TERM    vt100

# Install the software
RUN apt-get update
RUN apt-get install -y sudo
RUN apt-get install -y apt
RUN apt-get install -y apt-utils
RUN apt-get install -y debianutils
RUN apt-get install -y perl-base
RUN apt-get install -y net-tools
RUN apt-get install -y dnsutils
RUN apt-get install -y libaio1
RUN apt-get install -y libtinfo6
RUN apt-get install -y libcrypt1
RUN apt-get install -y libncurses6
RUN apt-get install -y libgcc1
RUN apt-get install -y less
RUN apt-get install -y man-db
RUN apt-get install -y tcsh

# Install the software
COPY $PROJECT /usr/local/$PROJECT

# Add valid login shell
RUN add-shell /usr/local/$PROJECT/bin/$SHELL

# Create user and group
RUN useradd --no-create-home --user-group --home-dir /usr/local/$PROJECT --shell /usr/local/$PROJECT/bin/$SHELL --comment 'OSH user' $USER

# Change ownership
RUN chown -R $USER:$USER /usr/local/$PROJECT

# Add and configure dynamic linker run-time bindings
RUN echo "/usr/local/$PROJECT/lib" >> /etc/ld.so.conf.d/oracle.conf
RUN ldconfig

# Add user to sudoers
RUN echo "$USER   ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER    $USER
WORKDIR /usr/local/$PROJECT

# Go for fun
CMD ["/usr/local/osh/bin/osh"]
# CMD ["/bin/tcsh"]

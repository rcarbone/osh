FROM debian:bullseye-slim

MAINTAINER rocco <rocco@tecsiel.it>

ENV PROJECT osh
ENV USER    osh
ENV SHELL   osh

# Install software
COPY $PROJECT /home/$USER

# Install the software
RUN set -ex &&          \
    apt-get update &&   \
    apt-get install -qq \
    libaio1     \
    libtinfo6   \
    libcrypt1   \
    libncurses6 \
    libgcc1     \
    sudo     \
   net-tools \
   dnsutils

# 1. Add valid login shell
# 2. Create user and group
# 3. Change ownership
# 4. Add user to sudoers
# 5. Add and configure dynamic linker run-time bindings
RUN \
    add-shell /home/$USER/bin/$SHELL && \
    useradd --no-create-home --user-group --home-dir /home/$USER --shell /home/$USER/bin/$SHELL --comment 'OSH user' $USER && \
    chown -R $USER:$USER /home/$USER && \
    echo "$USER   ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers && \
    echo "/home/$USER/lib" >> /etc/ld.so.conf.d/oracle.conf && \
    ldconfig

USER    $USER
WORKDIR /home/$USER

# Go for fun
CMD ["/home/osh/bin/osh"]

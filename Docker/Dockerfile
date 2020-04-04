FROM debian:bullseye-slim

MAINTAINER rocco <rocco@tecsiel.it>

ENV PROJECT osh
ENV USER    osh
ENV SHELL   osh

# Install software
COPY $PROJECT /usr/local/$PROJECT

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
    add-shell /usr/local/$PROJECT/bin/$SHELL && \
    useradd --no-create-home --user-group --home-dir /usr/local/$PROJECT --shell /usr/local/$PROJECT/bin/$SHELL --comment 'OSH user' $USER && \
    chown -R $USER:$USER /usr/local/$PROJECT && \
    echo "$USER   ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers && \
    echo "/usr/local/$PROJECT/lib" >> /etc/ld.so.conf.d/oracle.conf && \
    ldconfig

USER    $USER
WORKDIR /usr/local/$PROJECT

# Go for fun
CMD ["/usr/local/osh/bin/osh"]

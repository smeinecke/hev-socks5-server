#!/bin/sh
# To add this repository please do:

if [ "$(whoami)" != "root" ]; then
    SUDO=sudo
fi

${SUDO} apt-get update
${SUDO} apt-get -y install lsb-release ca-certificates wget
${SUDO} wget -O /usr/share/keyrings/smeinecke.github.io-hev-socks5-server.key https://smeinecke.github.io/hev-socks5-server/public.key
echo "deb [signed-by=/usr/share/keyrings/smeinecke.github.io-hev-socks5-server.key] https://smeinecke.github.io/hev-socks5-server/repo $(lsb_release -sc) main" | ${SUDO} tee /etc/apt/sources.list.d/hev-socks5-server.list
${SUDO} apt-get update

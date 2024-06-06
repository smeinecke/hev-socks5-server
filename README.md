# HevSocks5Server

[![status](https://github.com/heiher/hev-socks5-server/actions/workflows/build.yaml/badge.svg?branch=main&event=push)](https://github.com/heiher/hev-socks5-server)

HevSocks5Server is a simple, lightweight socks5 server.

For more infos, see https://github.com/heiher/hev-socks5-server

## Debian Repository

## Currently supported debian/ubuntu versions:
 * buster
 * bullseye
 * bookworm
 * noble
 * jammy
 * focal

## How to add this repository:

### Automatically via script
```
wget -O- https://smeinecke.github.io/hev-socks5-server/add-repository.sh | bash
```

### Manually
```
apt-get install wget lsb-release ca-certificates
wget -O /usr/share/keyrings/smeinecke.github.io-hev-socks5-server.key https://smeinecke.github.io/hev-socks5-server/public.key
echo "deb [signed-by=/usr/share/keyrings/smeinecke.github.io-hev-socks5-server.key] https://smeinecke.github.io/hev-socks5-server/repo $(lsb_release -sc) main" > /etc/apt/sources.list.d/hev-socks5-server.list
```

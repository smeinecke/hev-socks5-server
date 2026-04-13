#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
    SUDO=sudo
else
    SUDO=
fi

if [ -r /etc/os-release ]; then
    # shellcheck source=/dev/null
    . /etc/os-release
    DISTRO_ID="${ID:-}"
    CODENAME="${VERSION_CODENAME:-}"
else
    DISTRO_ID=
    CODENAME=
fi

if [ -z "${DISTRO_ID}" ] || [ -z "${CODENAME}" ]; then
    echo "Unable to detect distro codename from /etc/os-release."
    echo "This repository supports Debian (buster, bullseye, bookworm, trixie) and Ubuntu (focal, jammy, noble)."
    exit 1
fi

${SUDO} apt-get update
${SUDO} apt-get -y install ca-certificates wget gpg

case "${DISTRO_ID}:${CODENAME}" in
    debian:buster|debian:bullseye|debian:bookworm|debian:trixie|ubuntu:focal|ubuntu:jammy|ubuntu:noble)
        ;;
    *)
        echo "Unsupported distribution: ${DISTRO_ID}:${CODENAME}" >&2
        echo "Supported releases: debian:{buster,bullseye,bookworm,trixie} ubuntu:{focal,jammy,noble}" >&2
        exit 1
        ;;
esac

wget -O- https://smeinecke.github.io/hev-socks5-server/public.key | ${SUDO} gpg --dearmor -o /usr/share/keyrings/smeinecke.github.io-hev-socks5-server.gpg
echo "deb [signed-by=/usr/share/keyrings/smeinecke.github.io-hev-socks5-server.gpg] https://smeinecke.github.io/hev-socks5-server/repo ${CODENAME} main" | ${SUDO} tee /etc/apt/sources.list.d/hev-socks5-server.list
${SUDO} apt-get update

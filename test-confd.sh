#!/bin/bash

set -e

BIN=./bin/hev-socks5-server
TMPDIR=$(mktemp -d)
PID=""

cleanup () {
    if [ -n "$PID" ]; then
        kill $PID 2>/dev/null || true
        wait $PID 2>/dev/null || true
    fi
    rm -rf "$TMPDIR"
}
trap cleanup EXIT

port_listening () {
    ss -ltn | awk '{ print $4 }' | grep -qE ":$1$"
}

start_server () {
    "$BIN" "$@" &
    PID=$!
    sleep 0.5
}

write_main () {
    cat >"$TMPDIR/main.yml" <<EOF
main:
  workers: 1
  port: 11080
  listen-address: '127.0.0.1'
EOF
}

run_tests () {
    if [ ! -x "$BIN" ]; then
        echo "Binary $BIN not found, run 'make' first" >&2
        return 1
    fi

    # 1. Plain main config, no conf.d
    write_main
    start_server "$TMPDIR/main.yml"
    port_listening '11080' || {
        echo 'FAIL: plain main config did not bind to port 11080' >&2
        return 1
    }
    kill $PID 2>/dev/null || true
    wait $PID 2>/dev/null || true
    PID=""

    # 2. conf.d overrides the port
    mkdir -p "$TMPDIR/main.d"
    cat >"$TMPDIR/main.d/00-port.yml" <<EOF
main:
  port: 11081
EOF
    start_server "$TMPDIR/main.yml" "$TMPDIR/main.d"
    port_listening '11081' || {
        echo 'FAIL: conf.d override did not change port to 11081' >&2
        return 1
    }
    kill $PID 2>/dev/null || true
    wait $PID 2>/dev/null || true
    PID=""

    # 3. Multiple conf.d files are applied in sorted order
    cat >"$TMPDIR/main.d/01-port.yml" <<EOF
main:
  port: 11082
EOF
    start_server "$TMPDIR/main.yml" "$TMPDIR/main.d"
    port_listening '11082' || {
        echo 'FAIL: last sorted conf.d file should win' >&2
        return 1
    }
    kill $PID 2>/dev/null || true
    wait $PID 2>/dev/null || true
    PID=""

    # 4. Auto-derived conf.d path (main.yml -> main.d)
    start_server "$TMPDIR/main.yml"
    port_listening '11082' || {
        echo 'FAIL: auto-derived conf.d path did not load' >&2
        return 1
    }
    kill $PID 2>/dev/null || true
    wait $PID 2>/dev/null || true
    PID=""

    # 5. Empty conf.d falls back to main config
    rm -f "$TMPDIR/main.d"/*.yml
    start_server "$TMPDIR/main.yml"
    port_listening '11080' || {
        echo 'FAIL: empty conf.d should fall back to main config' >&2
        return 1
    }
    kill $PID 2>/dev/null || true
    wait $PID 2>/dev/null || true
    PID=""

    echo 'All conf.d tests passed'
}

run_tests

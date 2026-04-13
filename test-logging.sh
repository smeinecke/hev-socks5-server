#!/bin/bash
# Local test script to validate improved error logging strings in binary

set -e

echo "=== Testing improved error logging ==="

# Build the server
echo "Building hev-socks5-server..."
make clean >/dev/null 2>&1 || true
make

# Check that the binary contains the improved error strings
echo "Checking binary for improved error strings..."

# The error strings are stored separately in the binary
if strings ./bin/hev-socks5-server | grep -q "host unreachable"; then
    echo "✓ Binary contains 'host unreachable'"
else
    echo "✗ Binary missing 'host unreachable'"
    exit 1
fi

if strings ./bin/hev-socks5-server | grep -q "general failure"; then
    echo "✓ Binary contains 'general failure'"
else
    echo "✗ Binary missing 'general failure'"
    exit 1
fi

if strings ./bin/hev-socks5-server | grep -q "address type not supported"; then
    echo "✓ Binary contains 'address type not supported'"
else
    echo "✗ Binary missing 'address type not supported'"
    exit 1
fi

if strings ./bin/hev-socks5-server | grep -q "command not implemented"; then
    echo "✓ Binary contains 'command not implemented'"
else
    echo "✗ Binary missing 'command not implemented'"
    exit 1
fi

# Check for the new log format string
if strings ./bin/hev-socks5-server | grep -q "%p fail: %s to %s"; then
    echo "✓ Binary contains new log format '%p fail: %s to %s'"
else
    echo "✗ Binary missing new log format"
    exit 1
fi

# Check for the helper function
if strings ./bin/hev-socks5-server | grep -q "socks5 server connect to"; then
    echo "✓ Binary contains 'socks5 server connect to' (with target address)"
else
    echo "✗ Binary missing connection logging with target address"
    exit 1
fi

if strings ./bin/hev-socks5-server | grep -q "socks5 server bind to"; then
    echo "✓ Binary contains 'socks5 server bind to'"
else
    echo "✗ Binary missing bind logging with target address"
    exit 1
fi

# Ensure old format is NOT present (this is a negative test)
if strings ./bin/hev-socks5-server | grep -q "%p fail %u"; then
    echo "✗ Binary still contains old format string '%p fail %u'"
    exit 1
else
    echo "✓ Old format string '%p fail %u' not found (replaced with new format)"
fi

echo ""
echo "=== All binary checks passed ==="
echo "The improved logging is compiled into the binary."
echo ""
echo "Expected log format examples:"
echo "  [E] 0x... fail: host unreachable to [192.0.2.1]:80"
echo "  [E] 0x... fail: address type not supported to [10.0.0.1]:443"
echo "  [I] 0x... socks5 server connect to [1.2.3.4]:8080"

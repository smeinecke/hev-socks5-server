# HevSocks5Server

[![status](https://github.com/heiher/hev-socks5-server/actions/workflows/build.yaml/badge.svg?branch=main&event=push)](https://github.com/heiher/hev-socks5-server)

HevSocks5Server is a simple, lightweight socks5 server.

For more infos, see https://github.com/heiher/hev-socks5-server

## Debian Repository

## Currently supported debian/ubuntu versions:
 * buster
 * bullseye
 * bookworm
 * trixie
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
apt-get install wget ca-certificates gpg
wget -O- https://smeinecke.github.io/hev-socks5-server/public.key | gpg --dearmor -o /usr/share/keyrings/smeinecke.github.io-hev-socks5-server.gpg
source /etc/os-release
echo "deb [signed-by=/usr/share/keyrings/smeinecke.github.io-hev-socks5-server.gpg] https://smeinecke.github.io/hev-socks5-server/repo ${VERSION_CODENAME} main" > /etc/apt/sources.list.d/hev-socks5-server.list
```

### Windows (MSYS2)

```bash
export MSYS=winsymlinks:native
git clone --recursive https://github.com/heiher/hev-socks5-server
cd hev-socks5-server
make
```

## How to Use

### Config

```yaml
main:
  # Worker threads
  workers: 4
  # Listen port
  port: 1080
  # Listen address (ipv4|ipv6)
  # Can be a single address or a YAML list for multiple addresses.
  # Maximum number of listen addresses: HEV_CONFIG_MAX_LISTEN_ADDRESSES (16)
  listen-address:
    - '::'
    - '127.0.0.1'
  # UDP listen port (0: random, a-b: range)
# udp-port: 0
  # UDP listen address (ipv4|ipv6)
# udp-listen-address: '::1'
  # UDP public address (ipv4)
# udp-public-address-v4: ''
  # UDP public address (ipv6)
# udp-public-address-v6: ''
  # Listen ipv6 only
  listen-ipv6-only: false
  # Bind source address (ipv4|ipv6)
  # It is overridden by bind-address-v{4,6} if specified
  bind-address: ''
  # Bind source address (ipv4)
  bind-address-v4: ''
  # Bind source address (ipv6)
  bind-address-v6: ''
  # Bind source network interface
  bind-interface: ''
  # Domain address type (ipv4|ipv6|unspec)
  domain-address-type: unspec
  # Socket mark (hex: 0x1, dec: 1, oct: 01)
  mark: 0
  # TCP fastopen
# tcp-fastopen: false

#auth:
# file: conf/auth.txt
# username:
# password:

#misc:
  # task stack size (bytes)
# task-stack-size: 8192
  # udp socket recv buffer (SO_RCVBUF) size (bytes)
# udp-recv-buffer-size: 524288
  # number of udp buffers in splice, 1500 bytes per buffer.
# udp-copy-buffer-nums: 10
  # TCP connect timeout (ms)
# connect-timeout: 10000
  # TCP read-write timeout (ms)
# tcp-read-write-timeout: 300000
  # UDP read-write timeout (ms)
# udp-read-write-timeout: 60000
  # null, stdout, stderr or file-path
# log-file: null
  # debug, info, warn or error
# log-level: warn
  # If present, run as a daemon with this pid file
# pid-file: /run/hev-socks5-server.pid
  # If present, set rlimit nofile; else use default value
# limit-nofile: 65535
```

`listen-address` supports up to `HEV_CONFIG_MAX_LISTEN_ADDRESSES` entries
(currently 16). If only one address is needed,
you can still use the scalar form: `listen-address: '::'`.

### Authentication file

```
<USERNAME> <SPACE> <PASSWORD> <SPACE> <MARK> <LF>
```

- USERNAME: A string of up to 255 characters
- PASSWORD: A string of up to 255 characters
- MARK: Hexadecimal

### Run

```bash
bin/hev-socks5-server conf/main.yml
```

### Live updating authentication file

Send signal `SIGUSR1` to socks5 server process after the authentication file is updated.

```bash
killall -SIGUSR1 hev-socks5-server
```

### Limit number of connections

For example, limit the number of connections for `jerry` up to `2`:

#### Config

```yaml
auth:
  file: conf/auth.txt
```

#### Auth file

```
jerry pass 1a
```

#### IPtables

```bash
iptables -A OUTPUT -p tcp --syn -m mark --mark 0x1a -m connlimit --connlimit-above 2 -j REJECT
```

#### OpenWrt 23.05+

Repo: https://github.com/openwrt/packages/tree/master/net/hev-socks5-server

```sh
# Install package
opkg install hev-socks5-server

# Edit /etc/config/hev-socks5-server

# Restart service
/etc/init.d/hev-socks5-server restart
```

## API

### C

```c
/**
 * hev_socks5_server_main_from_file:
 * @config_path: config file path
 *
 * Start and run the socks5 server, this function will blocks until the
 * hev_socks5_server_quit is called or an error occurs.
 *
 * Returns: returns zero on successful, otherwise returns -1.
 *
 * Since: 2.6.7
 */
int hev_socks5_server_main_from_file (const char *config_path);

/**
 * hev_socks5_server_main_from_str:
 * @config_str: string config
 * @config_len: the byte length of string config
 *
 * Start and run the socks5 server, this function will blocks until the
 * hev_socks5_server_quit is called or an error occurs.
 *
 * Returns: returns zero on successful, otherwise returns -1.
 *
 * Since: 2.6.7
 */
int hev_socks5_server_main_from_str (const unsigned char *config_str,
                                     unsigned int config_len);

/**
 * hev_socks5_server_quit:
 *
 * Stop the socks5 server.
 *
 * Since: 2.6.7
 */
void hev_socks5_server_quit (void);
```

### Java

```java
public class Socks5Service {
	private static native boolean Socks5StartService(String config_path);
	private static native boolean Socks5StopService();
	private static native boolean Socks5IsRunning();

	static {
		System.loadLibrary("hev-socks5-server");
	}
```

### Kotlin

```kt
object Socks5Service {
    private external fun Socks5StartService(config_path: String): Boolean
    private external fun Socks5StopService(): Boolean
    private external fun Socks5IsRunning(): Boolean

    init {
        System.loadLibrary("hev-socks5-server")
    }
}
```

Allow overriding the package and class names in `Application.mk`[^3].

```makefile
APP_CFLAGS := -DPKGNAME=hev/socks5 -DCLSNAME=Socks5Service
```

## Use Cases

### Android App

* [Socks5](https://github.com/heiher/socks5)

### iOS App

* [Socks5](https://github.com/heiher/socks5-ios)

## Contributors

* **ammar faizi** - https://github.com/ammarfaizi2
* **hev** - https://hev.cc
* **pexcn** - <i@pexcn.me>

## License

MIT

[^1]: Windows is not supported at this time.
[^2]: See [protocol specification](https://github.com/heiher/hev-socks5-core/tree/main?tab=readme-ov-file#udp-in-tcp). The [hev-socks5-tunnel](https://github.com/heiher/hev-socks5-tunnel) and [hev-socks5-tproxy](https://github.com/heiher/hev-socks5-tproxy) clients support UDP relay over TCP.
[^3]: See [Application.mk](https://github.com/heiher/socks5/blob/364084079be989233286af2e33048a442fd06f45/app/src/main/jni/Application.mk#L19)

/*
 ============================================================================
 Name        : hev-main.c
 Author      : hev <r@hev.cc>
 Copyright   : Copyright (c) 2017 - 2024 hev
 Description : Main
 ============================================================================
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>

#include <hev-task.h>
#include <hev-socks5-misc.h>
#include <hev-socks5-logger.h>

#include "hev-misc.h"
#include "hev-config.h"
#include "hev-config-const.h"
#include "hev-logger.h"
#include "hev-socks5-proxy.h"

#include "hev-main.h"

#ifdef __MSYS__
#define WEAK
#else
#define WEAK __attribute__ ((weak))
#endif

static void
show_version (void)
{
    printf ("hev-socks5-server %u.%u.%u %s\n", MAJOR_VERSION, MINOR_VERSION,
            MICRO_VERSION, COMMIT_ID);
}

static void
show_help (const char *self_path)
{
    printf ("Usage: %s [OPTIONS] CONFIG_PATH\n", self_path);
    printf ("\n");
    printf ("A high-performance SOCKS5 proxy server.\n");
    printf ("\n");
    printf ("Options:\n");
    printf ("  -h, --help       Show this help message and exit\n");
    printf ("  -v, --version    Show version information and exit\n");
    printf ("\n");
    printf ("Configuration file (YAML format):\n");
    printf ("\n");
    printf ("  main:\n");
    printf (
        "    workers               Number of worker threads (default: 4)\n");
    printf ("    port                  Listen port (default: 1080)\n");
    printf (
        "    listen-address        Listen address(es), ipv4|ipv6 (default: '::')\n");
    printf (
        "    udp-port              UDP listen port, 0 for random (default: 0)\n");
    printf ("    udp-listen-address    UDP listen address (default: '::')\n");
    printf (
        "    udp-public-address-v4 UDP public IPv4 address for UDP relay\n");
    printf (
        "    udp-public-address-v6 UDP public IPv6 address for UDP relay\n");
    printf ("    listen-ipv6-only      Listen on IPv6 only (default: false)\n");
    printf (
        "    bind-address          Bind source address (overridden by v4/v6)\n");
    printf ("    bind-address-v4       Bind source IPv4 address\n");
    printf ("    bind-address-v6       Bind source IPv6 address\n");
    printf ("    bind-interface        Bind to network interface\n");
    printf (
        "    domain-address-type   Domain address type: ipv4|ipv6|unspec (default: unspec)\n");
    printf ("    mark                  Socket mark (hex, dec, or oct)\n");
    printf ("\n");
    printf ("  auth:\n");
    printf (
        "    file                  Path to auth file (username:password format)\n");
    printf ("    username              Authentication username\n");
    printf ("    password              Authentication password\n");
    printf ("\n");
    printf ("  misc:\n");
    printf (
        "    task-stack-size       Task stack size in bytes (default: 8192)\n");
    printf (
        "    udp-recv-buffer-size  UDP socket recv buffer size (default: 524288)\n");
    printf (
        "    udp-copy-buffer-nums  Number of UDP splice buffers (default: 10)\n");
    printf (
        "    connect-timeout       TCP connect timeout in ms (default: 10000)\n");
    printf (
        "    tcp-read-write-timeout  TCP read/write timeout in ms (default: 300000)\n");
    printf (
        "    udp-read-write-timeout  UDP read/write timeout in ms (default: 60000)\n");
    printf (
        "    log-file              Log output: stdout, stderr, or file path\n");
    printf (
        "    log-level             Log level: debug, info, warn, error (default: warn)\n");
    printf (
        "    pid-file              PID file path (enables daemon mode if set)\n");
    printf (
        "    limit-nofile          Rlimit for open files (default: system default)\n");
    printf ("\n");
    printf ("Examples:\n");
    printf ("  %s conf/main.yml\n", self_path);
    printf ("  %s /etc/hev-socks5-server.yml\n", self_path);
    printf ("\n");
    printf ("Version: %u.%u.%u %s\n", MAJOR_VERSION, MINOR_VERSION,
            MICRO_VERSION, COMMIT_ID);
}

static void
sigint_handler (int signum)
{
    hev_socks5_proxy_stop ();
}

static int
hev_socks5_server_main_inner (void)
{
    const char *pid_file;
    const char *log_file;
    int log_level;
    int timeout;
    int nofile;
    int res;

    log_file = hev_config_get_misc_log_file ();
    log_level = hev_config_get_misc_log_level ();

    timeout = hev_config_get_misc_connect_timeout ();
    hev_socks5_set_connect_timeout (timeout);
    timeout = hev_config_get_misc_tcp_read_write_timeout ();
    hev_socks5_set_tcp_timeout (timeout);
    timeout = hev_config_get_misc_udp_read_write_timeout ();
    hev_socks5_set_udp_timeout (timeout);

    res = hev_config_get_misc_task_stack_size ();
    hev_socks5_set_task_stack_size (res);

    res = hev_config_get_misc_udp_recv_buffer_size ();
    hev_socks5_set_udp_recv_buffer_size (res);

    res = hev_config_get_misc_udp_copy_buffer_nums ();
    hev_socks5_set_udp_copy_buffer_nums (res);

    res = hev_logger_init (log_level, log_file);
    if (res < 0)
        goto exit1;

    res = hev_socks5_logger_init (log_level, log_file);
    if (res < 0)
        goto exit2;

    nofile = hev_config_get_misc_limit_nofile ();
    res = set_limit_nofile (nofile);
    if (res < 0)
        LOG_W ("set limit nofile");

    pid_file = hev_config_get_misc_pid_file ();
    if (pid_file)
        run_as_daemon (pid_file);

    res = hev_socks5_proxy_init ();
    if (res < 0)
        goto exit3;

    hev_socks5_proxy_run ();

    hev_socks5_proxy_fini ();
exit3:
    hev_socks5_logger_fini ();
exit2:
    hev_logger_fini ();
exit1:
    hev_config_fini ();
    return res;
}

int
hev_socks5_server_main_from_file (const char *config_path)
{
    int res = hev_config_init_from_file (config_path);
    if (res < 0)
        return -1;

    return hev_socks5_server_main_inner ();
}

int
hev_socks5_server_main_from_str (const unsigned char *config_str,
                                 unsigned int config_len)
{
    int res = hev_config_init_from_str (config_str, config_len);
    if (res < 0)
        return -1;

    return hev_socks5_server_main_inner ();
}

void
hev_socks5_server_quit (void)
{
    hev_socks5_proxy_stop ();
}

WEAK int
main (int argc, char *argv[])
{
    int res;

    if (argc < 2) {
        show_help (argv[0]);
        return -1;
    }

    if (strcmp (argv[1], "--help") == 0 || strcmp (argv[1], "-h") == 0) {
        show_help (argv[0]);
        return 0;
    }

    if (strcmp (argv[1], "--version") == 0 || strcmp (argv[1], "-v") == 0) {
        show_version ();
        return 0;
    }

    signal (SIGINT, sigint_handler);

    res = hev_socks5_server_main_from_file (argv[1]);
    if (res < 0)
        return -2;

    return 0;
}

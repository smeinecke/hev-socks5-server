/*
 ============================================================================
 Name        : hev-socket-factory.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2022 hev
 Description : Socket Factory
 ============================================================================
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <hev-task.h>
#include <hev-task-io.h>
#include <hev-task-io-socket.h>
#include <hev-memory-allocator.h>

#include "hev-misc.h"
#include "hev-logger.h"
#include "hev-config-const.h"

#include "hev-socket-factory.h"

struct _HevSocketFactory
{
    struct sockaddr_in6 addrs[HEV_CONFIG_MAX_LISTEN_ADDRESSES];
    int fds[HEV_CONFIG_MAX_LISTEN_ADDRESSES];
    int tcp_fastopen;
    int ipv6_only;
    unsigned int addr_count;
};

HevSocketFactory *
hev_socket_factory_new (const char **addrs, unsigned int addr_count,
                        const char *port, int ipv6_only, int tcp_fastopen)
{
    HevSocketFactory *self;
    unsigned int i;
    int res;

    LOG_D ("socket factory new");

    if (!addrs || addr_count == 0 ||
        addr_count > HEV_CONFIG_MAX_LISTEN_ADDRESSES)
        return NULL;

    self = hev_malloc0 (sizeof (HevSocketFactory));
    if (!self) {
        LOG_E ("socket factory alloc");
        return NULL;
    }

    for (i = 0; i < addr_count; i++) {
        res = hev_netaddr_resolve (&self->addrs[i], addrs[i], port);
        if (res < 0) {
            LOG_E ("socket factory resolve");
            hev_free (self);
            return NULL;
        }
    }

    self->tcp_fastopen = tcp_fastopen;
    self->ipv6_only = ipv6_only;
    self->addr_count = addr_count;
    for (i = 0; i < addr_count; i++)
        self->fds[i] = -1;

    return self;
}

void
hev_socket_factory_destroy (HevSocketFactory *self)
{
    unsigned int i;

    LOG_D ("socket factory destroy");

    for (i = 0; i < self->addr_count; i++) {
        if (self->fds[i] >= 0)
            close (self->fds[i]);
    }
    hev_free (self);
}

unsigned int
hev_socket_factory_get_count (HevSocketFactory *self)
{
    return self->addr_count;
}

int
hev_socket_factory_get (HevSocketFactory *self, unsigned int idx)
{
    int qlen = 100;
    int one = 1;
    int res;
    int fd;

    LOG_D ("socket factory get");

    if (idx >= self->addr_count)
        return -1;

    if (self->fds[idx] >= 0)
        return dup (self->fds[idx]);

    fd = hev_task_io_socket_socket (AF_INET6, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_E ("socket factory socket");
        goto exit;
    }

    res = setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof (one));
    if (res < 0) {
        LOG_E ("socket factory reuse");
        goto exit_close;
    }

#ifdef SO_REUSEPORT
    res = setsockopt (fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof (one));
    (void)res;
#endif

    res = setsockopt (fd, IPPROTO_IPV6, IPV6_V6ONLY, &self->ipv6_only,
                      sizeof (self->ipv6_only));
    if (res < 0) {
        LOG_E ("socket factory ipv6 only");
        goto exit_close;
    }

    res = bind (fd, (struct sockaddr *)&self->addrs[idx],
                sizeof (self->addrs[idx]));
    if (res < 0) {
        LOG_E ("socket factory bind");
        goto exit_close;
    }

    if (self->tcp_fastopen) {
        res = setsockopt (fd, IPPROTO_TCP, TCP_FASTOPEN, &qlen, sizeof (qlen));
        if (res < 0)
            LOG_W ("socket factory fastopen");
    }

    res = listen (fd, 100);
    if (res < 0) {
        LOG_E ("socket factory listen");
        goto exit_close;
    }

    self->fds[idx] = dup (fd);
    if (self->fds[idx] < 0)
        goto exit_close;

    return fd;

exit_close:
    close (fd);
exit:
    return -1;
}

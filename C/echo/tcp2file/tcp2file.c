/*=============================================================================
 * Author:         wine93 <wine93.info@gmail.com>
 * ComplieFlags:   -g
 * Description:    A simple t2f server
 * =============================================================================*/

#define _GNU_SOURCE
#include <time.h>
#include <fcntl.h>       /* O_WRONLY */
#include <stdio.h>       /* perror */
#include <stdlib.h>      /* exit */
#include <string.h>      /* memset */
#include <stdarg.h>      /* va_start va_end */
#include <stddef.h>      /* offsetof */
#include <unistd.h>      /* close */
#include <errno.h>       /* errno */
#include <signal.h>      /* signal */
#include <sys/epoll.h>   /* epoll_create epoll_ctl epoll_wait */
#include <sys/socket.h>  /* socket */
#include <netinet/in.h>  /* sockaddr_in */
#include <arpa/inet.h>   /* inet_ntoa */


// DEBUG
#define dd(...) fprintf(stderr, "[DEBUG] "); \
    fprintf(stderr, __VA_ARGS__); \

#define logerr(format) fprintf(stderr, "[ERR] (%s:%d) ", __func__, __LINE__); \
    perror(format)


// STR_T
#define string(str)     { sizeof(str) - 1, (u_char *) str }
#define null_string     { 0, NULL }
#define str_set(str, text) \
    (str)->len = sizeof(text) - 1; (str)->data = (u_char *) text
#define str_null(str)   (str)->len = 0; (str)->data = NULL


// FILE_T
#define open_file(name, mode, create, access) \
    open((const char *) name, mode|create, access)

#define INVALID_FILE         -1

#define FILE_APPEND          (O_WRONLY|O_APPEND)
#define FILE_NONBLOCK        O_NONBLOCK

#define FILE_CREATE_OR_OPEN  O_CREAT
#define FILE_OPEN            0

#define FILE_DEFAULT_ACCESS  0644
#define FILE_OWNER_ACCESS    0600


// QUEUE_T
#define queue_init(q) \
    (q)->prev = q; \
    (q)->next = q


#define queue_empty(h) \
    (h == (h)->prev)


#define queue_head(h) \
    (h)->next


#define queue_last(h) \
    (h)->prev


#define queue_sentinel(h) \
    (h)


#define queue_next(q) \
    (q)->next


#define queue_insert_tail(h, x) \
    (x)->prev = (h)->prev; \
    (x)->prev->next = x; \
    (x)->next = h; \
    (h)->prev = x


#define queue_remove(x) \
    (x)->next->prev = (x)->prev; \
    (x)->prev->next = (x)->next


#define queue_data(q, type, link) \
    (type *) ((u_char *) q - offsetof(type, link))


// EVENT_T
#define SOCKADDRLEN          sizeof(struct sockaddr)

#define MAX_BACKLOG          8192
#define MAX_BUFFER           4096
#define MAX_EVENTS           1024

#define READ_EVENT           1
#define WRITE_EVENT          2


typedef struct str_s         str_t;
typedef struct queue_s       queue_t;
typedef struct file_s        file_t;
typedef struct signal_s      signal_t;
typedef struct event_s       event_t;
typedef struct connection_s  connection_t;

typedef void (*event_handler_pt)(event_t *ev);


struct str_s {
    size_t   len;
    u_char  *data;
};


struct queue_s {
    queue_t  *prev;
    queue_t  *next;
};


struct file_s {
    int       fd;
    str_t    *name;
    queue_t   queue;
};


struct signal_s {
    int    signo;
    char  *signame;
    void  (*handler)(int signo);
};


struct event_s {
    void              *data;
    int                active;
    event_handler_pt   handler;
};


struct connection_s {
    int               fd;
    struct sockaddr  *sockaddr;
    socklen_t         socklen;
    const char       *addr_text;
    short             port;
    void             *buffer;
    u_char           *pos;
    u_char           *last;
    event_t          *rev;
    event_t          *wev;
};


static void *pcalloc(size_t size);

static const char *today();

static int create_file(file_t *file);
static int close_file(file_t *file);
static ssize_t write_file(file_t *file, void *buf, size_t n);

static void init_signal();
static void signal_handler(int signo);

static connection_t *create_connection();
static int free_connection(connection_t *c);
static int check_broken_connection(connection_t *c);

static int open_listening_socket(const char *host, short port);

static int epoll_init();
static int epoll_add_connection(connection_t *c);
static int epoll_del_connection(connection_t *c);
static int epoll_add_event(event_t *ev, int event);
static int epoll_process_events();

static void t2f_accept(event_t *ev);
static void t2f_recv(event_t *ev);
static void t2f_send(event_t *ev);
static void t2f_write_file(event_t *ev);


static queue_t              queue;
static int                  epfd = -1;
static struct epoll_event   event_list[MAX_EVENTS + 5];

static signal_t  signals[] = {
    { SIGPIPE, "SIGPIPE", signal_handler },
    { 0, NULL, NULL }
};


static void *
palloc(size_t size)
{
    void  *p;

    p = malloc(size);
    if (p == NULL) {
        return NULL;
    }

    memset(p, 0, size);

    return p;
}


static const char *
today()
{
    time_t        t;
    struct tm    *tm;
    static char   today[sizeof("yyyy-mm-dd")];

    t = time(NULL);
    tm = gmtime(&t);

    sprintf(today, "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

    return today;
}


static int
create_file(file_t *file)
{
    int  fd;

    fd = open_file(file->name->data, FILE_APPEND, FILE_CREATE_OR_OPEN, FILE_DEFAULT_ACCESS);
    if (fd == -1) {
        logerr("open() failed");
        return -1;
    }

    file->fd = fd;

    return 1;
}


static int
close_file(file_t *file)
{
    int  rc;

    free(file->name->data);
    free(file->name);

    rc = close(file->fd);
    if (rc == -1) {
        logerr("close() failed");
    }

    free(file);

    return rc;
}


static ssize_t
write_file(file_t *file, void *buf, size_t size)
{
    ssize_t  n;

    n = write(file->fd, buf, size);
    if (n == -1) {
        logerr("write() failed");
    }

    return n;
}


static connection_t *
create_connection()
{
    connection_t  *c;

    c = (connection_t *) palloc(sizeof(connection_t));
    if (c == NULL) {
        return NULL;
    }

    c->rev = (event_t *) palloc(sizeof(event_t));
    if (c->rev == NULL) {
        return NULL;
    }

    c->wev = (event_t *) palloc(sizeof(event_t));
    if (c->wev == NULL) {
        return NULL;
    }

    return c;
}


static int
free_connection(connection_t *c)
{
    int  rc;

    epoll_del_connection(c);

    rc = close(c->fd);
    if (rc == -1) {
        logerr("close() failed");
        return -1;
    }

    return 0;
}


static int
open_listening_socket(const char *host, short port)
{
    int                  s, rc, opt;
    connection_t        *lc;
    struct sockaddr_in  *sa;

    s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (s == -1) {
        logerr("socket() failed");
        return -1;
    }

    opt = 1;
    rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    if (rc == -1) {
        logerr("setsockopt() failed");
        return -1;
    }

    opt = 1;
    rc = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(int));
    if (rc == -1) {
        logerr("setsockopt() failed");
        return -1;
    }

    sa = (struct sockaddr_in *) palloc(sizeof(struct sockaddr_in));
    sa->sin_family = AF_INET;
    inet_pton(AF_INET, host, &(sa->sin_addr));
    sa->sin_port = htons(port);
    rc = bind(s, (struct sockaddr *) sa, sizeof(struct sockaddr));
    if (rc == -1) {
        logerr("bind() failed");
        return -1;
    }

    rc = listen(s, MAX_BACKLOG);
    if (rc == -1) {
        logerr("listen() failed");
        return -1;
    }

    lc = create_connection();
    if (lc == NULL) {
        return -1;
    }

    lc->fd = s;
    lc->sockaddr = (struct sockaddr *) sa;
    lc->socklen = SOCKADDRLEN;
    lc->addr_text = host;
    lc->port = port;

    lc->rev->data = (void *) lc;
    lc->rev->handler = t2f_accept;
    epoll_add_event(lc->rev, READ_EVENT);

    return 0;
}


static int
epoll_init()
{
    epfd = epoll_create(1);
    if (epfd == -1) {
        logerr("epoll_create() failed");
    }

    return epfd;
}


static int
epoll_add_connection(connection_t *c)
{
    int                 rc;
    struct epoll_event  ee;

    ee.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLRDHUP;
    ee.data.ptr = (void *) c;

    rc = epoll_ctl(epfd, EPOLL_CTL_ADD, c->fd, &ee);
    if (rc == -1) {
        logerr("epoll_ctl() failed");
        return -1;
    }

    c->rev->active = 1;
    c->wev->active = 1;

    return rc;
}


static int
epoll_del_connection(connection_t *c)
{
    int                 rc;
    struct epoll_event  ee;

    ee.events = 0;
    ee.data.ptr = NULL;

    rc = epoll_ctl(epfd, EPOLL_CTL_MOD, c->fd, &ee);
    if (rc == -1) {
        logerr("epoll_ctl() failed");
        return -1;
    }

    c->rev->active = 0;
    c->wev->active = 0;

    return rc;
}


static int
epoll_add_event(event_t *ev, int event)
{
    int                  rc;
    connection_t        *c;
    struct epoll_event   ee;

    c = (connection_t *) ev->data;

    ee.data.ptr = (void *) c;

    if (event & READ_EVENT) {
        ee.events = EPOLLIN | EPOLLET;
    } else if (event & WRITE_EVENT) {
        ee.events = EPOLLOUT | EPOLLET;
    }

    rc = epoll_ctl(epfd, EPOLL_CTL_ADD, c->fd, &ee);
    if (rc == -1) {
        logerr("epoll_ctl() failed");
        return -1;
    }

    ev->active = 1;

    return rc;

}


static int
epoll_process_events()
{
    int                  i, events, revents;
    connection_t        *c;
    event_t             *rev, *wev;
    struct epoll_event   ee;

    events = epoll_wait(epfd, event_list, MAX_EVENTS, -1);
    if (events == -1) {
        logerr("epoll_wait() failed");
        return -1;
    }

    for (i = 0; i < events; i++) {
        c = (connection_t *) event_list[i].data.ptr;

        revents = event_list[i].events;

        rev = c->rev;
        if ((revents & EPOLLIN) && rev->active) {
            rev->handler(rev);
        }

        wev = c->wev;
        if ((revents & EPOLLOUT) && wev->active) {
            wev->handler(wev);
        }
    }

    return 0;
}


static void
t2f_accept(event_t *ev)
{
    int                  s;
    struct sockaddr_in   sa;
    socklen_t            socklen;
    connection_t        *lc, *c;

    lc = (connection_t *) ev->data;

    socklen = SOCKADDRLEN;
    s = accept4(lc->fd, (struct sockaddr *) &sa, &socklen, SOCK_NONBLOCK);

    if (s == -1) {
        logerr("accept4() failed");
        return;
    }

    c = create_connection();
    if (c == NULL) {
        return;
    }

    c->fd = s;
    c->sockaddr = (struct sockaddr *) palloc(sizeof(struct sockaddr));
    memcpy((void *) c->sockaddr, (void *) &sa, SOCKADDRLEN);
    c->socklen = socklen;
    c->buffer = malloc(MAX_BUFFER + 5);
    c->pos = (u_char *) c->buffer;
    c->last = (u_char *) c->buffer;
    c->addr_text = inet_ntoa(sa.sin_addr);
    c->port = ntohs(sa.sin_port);

    c->rev->data = (void *) c;
    c->rev->handler = t2f_recv;

    c->wev->data = (void *) c;
    c->wev->handler = t2f_send;

    epoll_add_connection(c);

    dd("%s:%hu -> %s:%hu\n", c->addr_text, c->port, lc->addr_text, lc->port);
}


static void
t2f_recv(event_t *ev)
{
    size_t         n, len;
    connection_t  *c;

    c = (connection_t *) ev->data;

    for ( ;; ) {
        if (c->last == (u_char *) c->buffer + MAX_BUFFER) {
            t2f_write_file(c->wev);
        }

        len = MAX_BUFFER - (c->last - (u_char *) c->buffer);
        if (len == 0) {
            break;
        }

        n = recv(c->fd, c->last, len, 0);

        /*dd("recv %d\n", n);*/

        if (n == (size_t) -1) {
            if (errno != EAGAIN) {
                logerr("recv() failed");
            }
            t2f_write_file(c->wev);
            break;
        } else if (n == 0) {
            t2f_write_file(c->wev);
            free_connection(c);
            dd("client prematurely closed connection\n");
            break;
        }

        c->last += n;
    }
}


static void
t2f_send(event_t *ev)
{
}


static void
t2f_write_file(event_t *ev)
{
    int            rc, found;
    ssize_t        n;
    str_t         *filename;
    file_t        *file;
    queue_t       *q, *next, *last;
    connection_t  *c;

    c = (connection_t *) ev->data;

    filename = (str_t *) palloc(sizeof(str_t));
    if (filename == NULL) {
        return;
    }

    filename->len = strlen(c->addr_text) + sizeof("yyyy-mm-dd");
    filename->data = (u_char *) palloc(filename->len + 1);
    sprintf((char *) filename->data, "%s_%s", today(), c->addr_text);

    found = 0;
    for (q = queue_head(&queue); q != queue_sentinel(&queue); q = next) {
        next = queue_next(q);

        file = queue_data(q, file_t, queue);

        if (strncmp((const char *) file->name->data,
                    (const char *) filename->data, filename->len) == 0)
        {
            found = 1;
            break;
        }

        if (strncmp((const char *) file->name->data,
                    (const char *) filename->data, sizeof("yyyy-mm-dd") -1) != 0)
        {
            queue_remove(q);
            close_file(file);
        }
    }

    if (!found) {
        file = (file_t *) palloc(sizeof(file_t));
        if (file == NULL) {
            return;
        }

        file->name = filename;
        file->fd = INVALID_FILE;

        rc = create_file(file);
        if (rc == -1) {
            return;
        }

        last = queue_last(&queue);
        queue_insert_tail(&queue, &file->queue);

        dd("create file %s\n", filename->data);
    }

    while (c->pos < c->last) {
        n = write_file(file, c->pos, c->last - c->pos);
        if (n == (ssize_t) -1) {
            break;
        }

        c->pos += n;
    }

    if (c->pos == c->last) {
        c->pos = (u_char *) c->buffer;
        c->last = (u_char *) c->buffer;
    }
}


static void
init_signal()
{
    int                rc;
    signal_t          *sig;
    struct sigaction   sa;

    for (sig = signals; sig->signo != 0; sig++) {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        rc = sigaction(sig->signo, &sa, NULL);
        if (rc == -1) {
            logerr("sigaction() failed");
        }
    }
}


static void
signal_handler(int signo)
{
    switch(signo) {
        case SIGPIPE:
            dd("RECV SIGPIPE\n");
            break;

        default:
            dd("RECV UNKNOWN_SIGNAL\n");
            break;
    }
}


int
main()
{
    int  rc;

    queue_init(&queue);

    init_signal();

    epoll_init();

    rc = open_listening_socket("127.0.0.1", 3100);
    if (rc == -1) {
        return 1;
    }

    for ( ;; ) {
        epoll_process_events();
    }

    return 0;
}


/*
 * TODO: Socket Read timeout, then close the connection
 * TODO: 文件描述符与文件表(偏移量)，i-node 表之间的关系
 * TODO: 文件事件
 * TODO: 设置机器的 keepalive 参数
 * TODO: strlen vs sizeof
 */

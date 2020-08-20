/*
 * libevent echo server example using buffered events.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Required by event.h. */
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

/* Libevent. */
#include <event2/event.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>

/* Port to listen on. */
#define SERVER_PORT 5555

/**
 * Called by libevent when there is data to read.
 */
void buffered_on_read(struct bufferevent *bev, void *arg)
{
	/* Write back the read buffer. It is important to note that
	 * bufferevent_write_buffer will drain the incoming data so it
	 * is effectively gone after we call it. */
	bufferevent_write_buffer(bev, bev->input);
}

/**
 * Called by libevent when there is an error on the underlying socket
 * descriptor.
 */
void buffered_on_error(struct bufferevent *bev, short what, void *arg)
{
	if (what & BEV_EVENT_EOF) {
		/* Client disconnected, remove the read event and the
		 * free the client structure. */
		printf("Client disconnected.\n");
	} else {
		warn("Client socket error, disconnecting.\n");
	}
	bufferevent_free(bev);
	close(bev->ev_read.ev_fd);
}

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void on_accept(int fd, short ev, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct client *client;

	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) {
		warn("accept failed");
		return;
	}

	/* Set the client socket to non-blocking mode. */
	if (evutil_make_socket_nonblocking(client_fd) < 0)
		warn("failed to set client socket non-blocking");

	/* Create the buffered event.
	 *
	 * The first argument is the file descriptor that will trigger
	 * the events, in this case the clients socket.
	 *
	 * The second argument is the callback that will be called
	 * when data has been read from the socket and is available to
	 * the application.
	 *
	 * The third argument is a callback to a function that will be
	 * called when the write buffer has reached a low watermark.
	 * That usually means that when the write buffer is 0 length,
	 * this callback will be called.  It must be defined, but you
	 * don't actually have to do anything in this callback.
	 *
	 * The fourth argument is a callback that will be called when
	 * there is a socket error.  This is where you will detect
	 * that the client disconnected or other socket errors.
	 *
	 * The fifth and final argument is to store an argument in
	 * that will be passed to the callbacks.  We store the client
	 * object here.
	 */
	struct bufferevent *bev = bufferevent_socket_new(base, client_fd, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, buffered_on_read, NULL, buffered_on_error, NULL);
	/* We have to enable it before our callbacks will be
	 * called. */
	bufferevent_enable(bev, EV_READ);

	printf("Accepted connection from %s\n",
		inet_ntoa(client_addr.sin_addr));
}

int main()
{
	evutil_socket_t listen_fd;
	struct event_base *base;
	struct event *listen_event;		// listener
	struct sockaddr_in listen_addr;

	base = event_base_new();
	if (!base)
		err(1, "new base failed");

	/* Create our listening socket. */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err(1, "listen failed");
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(SERVER_PORT);
	if (bind(listen_fd, (struct sockaddr *)&listen_addr,
		sizeof(listen_addr)) < 0)
		err(1, "bind failed");
	if (listen(listen_fd, 5) < 0)
		err(1, "listen failed");

	evutil_make_listen_socket_reuseable(listen_fd);

	/* Set the socket to non-blocking, this is essential in event
	 * based programming with libevent. */
	if (evutil_make_socket_nonblocking(listen_fd) < 0)
		err(1, "failed to set server socket to non-blocking");

	/* We now have a listening socket, we create a read event to
	 * be notified when a client connects. */
	listen_event = event_new(base, listen_fd, EV_READ|EV_PERSIST, on_accept, (void *)base);
	event_add(listen_event, NULL);

	/* Start the event loop. */
	event_base_dispatch(base);

	return 0;
}

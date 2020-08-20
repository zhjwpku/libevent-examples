/*
 * libevent echo server example.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* For inet_ntoa. */
#include <arpa/inet.h>

/* Required by event.h. */
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

/* Libevent. */
#include <event2/event.h>
#include <event2/util.h>

/* Port to listen on. */
#define SERVER_PORT 5555

/**
 * This function will be called by libevent when the client socket is
 * ready for reading.
 */
void on_read(int fd, short ev, void *arg)
{
	struct event *client = (struct event *)arg;
	u_char buf[8196];
	int len, wlen;

	len = read(fd, buf, sizeof(buf));
	if (len == 0) {
		/* Client disconnected, remove the read event and the
		 * free the client structure. */
		printf("Client disconnected.\n");
                close(fd);
		event_del(client);
		event_free(client);
		return;
	} else if (len < 0) {
		/* Some other error occurred, close the socket, remove
		 * the event and free the client structure. */
		printf("Socket failure, disconnecting client: %s",
		    strerror(errno));
		close(fd);
		event_del(client);
		event_free(client);
		return;
	}

	/* XXX For the sake of simplicity we'll echo the data write
		* back to the client.  Normally we shouldn't do this in a
		* non-blocking app, we should queue the data and wait to be
		* told that we can write.
		*/
	wlen = write(fd, buf, len);
	if (wlen < len) {
		/* We didn't write all our data.  If we had proper
		* queueing/buffering setup, we'd finish off the write
		* when told we can write again.  For this simple case
		* we'll just lose the data that didn't make it in the
		* write.
		*/
		printf("Short write, not all data echoed back to client.\n");
	}
}

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void on_accept(evutil_socket_t fd, short ev, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	/* Accept the new connection. */
	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd == -1) {
		warn("accept failed");
		return;
	}

	/* Set the client socket to non-blocking mode. */
	if (evutil_make_socket_nonblocking(client_fd) < 0)
		warn("failed to set client socket non-blocking");

	/* We've accepted a new client, event_new to
	 * maintain the state of this client. */
	struct event *new_ev = event_new(base, client_fd, EV_READ|EV_PERSIST, on_read, event_self_cbarg());

	/* Setting up the event does not activate, add the event so it
	 * becomes active. */
	event_add(new_ev, NULL);

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

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err(1, "listen failed");
	/* Set the socket to non-blocking, this is essential in event
	 * based programming with libevent. */
	evutil_make_socket_nonblocking(listen_fd);
	evutil_make_listen_socket_reuseable(listen_fd);

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(SERVER_PORT);
	if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0)
		err(1, "bind failed");
	if (listen(listen_fd, 5) < 0)
		err(1, "listen failed");

	/* We now have a listening socket, we create a read event to
	 * be notified when a client connects. */
	listen_event = event_new(base, listen_fd, EV_READ|EV_PERSIST, on_accept, (void *)base);
	event_add(listen_event, NULL);

	/* Start the libevent event loop. */
	event_base_dispatch(base);

	return 0;
}

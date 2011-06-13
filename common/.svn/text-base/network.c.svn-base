/* Pioneers - Implementation of the excellent Settlers of Catan board game.
 *   Go buy a copy.
 *
 * Copyright (C) 1999 Dave Cole
 * Copyright (C) 2003, 2006 Bas Wijnen <shevek@fmf.nl>
 * Copyright (C) 2005-2009 Roland Clobus <rclobus@bigfoot.com>
 * Copyright (C) 2005 Keishi Suenaga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <ctype.h>
#include <unistd.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <errno.h>
#include <string.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifndef HAVE_GETADDRINFO_ET_AL
#include <stdlib.h>		/* For atoi */
#endif				/* ndef HAVE_GETADDRINFO_ET_AL */

#include <time.h>
#include "config.h"
#include "driver.h"
#include "game.h"
#include "map.h"
#include "network.h"
#include "log.h"

typedef union {
	struct sockaddr sa;
	struct sockaddr_in in;
#ifdef HAVE_GETADDRINFO_ET_AL
	struct sockaddr_in6 in6;
#endif				/* HAVE_GETADDRINFO_ET_AL */
} sockaddr_t;

static gboolean debug_enabled = FALSE;

static void net_attempt_to_connect(Session * ses);

/* Number of seconds between pings
 * (in the absence of other network activity).  */
static const int PING_PERIOD = 30;

void set_enable_debug(gboolean enabled)
{
	debug_enabled = enabled;
}

void debug(const gchar * fmt, ...)
{
	va_list ap;
	gchar *buff;
	gint idx;
	time_t t;
	struct tm *alpha;

	if (!debug_enabled)
		return;

	va_start(ap, fmt);
	buff = g_strdup_vprintf(fmt, ap);
	va_end(ap);

	t = time(NULL);
	alpha = localtime(&t);

	g_print("%02d:%02d:%02d ", alpha->tm_hour,
		alpha->tm_min, alpha->tm_sec);

	for (idx = 0; buff[idx] != '\0'; idx++) {
		if (isprint(buff[idx]))
			g_print("%c", buff[idx]);
		else
			switch (buff[idx]) {
			case '\n':
				g_print("\\n");
				break;
			case '\r':
				g_print("\\r");
				break;
			case '\t':
				g_print("\\t");
				break;
			default:
				g_print("\\x%02x", (buff[idx] & 0xff));
				break;
			}
	}
	g_print("\n");
}

static void read_ready(Session * ses);
static void write_ready(Session * ses);

static void listen_read(Session * ses, gboolean monitor)
{
	if (monitor && ses->read_tag == 0)
		ses->read_tag =
		    driver->input_add_read(ses->fd, (InputFunc) read_ready,
					   ses);
	if (!monitor && ses->read_tag != 0) {
		driver->input_remove(ses->read_tag);
		ses->read_tag = 0;
	}

}

static void listen_write(Session * ses, gboolean monitor)
{
	if (monitor && ses->write_tag == 0)
		ses->write_tag =
		    driver->input_add_write(ses->fd,
					    (InputFunc) write_ready, ses);
	if (!monitor && ses->write_tag != 0) {
		driver->input_remove(ses->write_tag);
		ses->write_tag = 0;
	}
}

static void notify(Session * ses, NetEvent event, gchar * line)
{
	if (ses->notify_func != NULL)
		ses->notify_func(event, ses->user_data, line);
}

static gboolean net_would_block(void)
{
#ifdef G_OS_WIN32
	return WSAGetLastError() == WSAEWOULDBLOCK;
#else				/* G_OS_WIN32 */
	return errno == EAGAIN;
#endif				/* G_OS_WIN32 */
}

static gboolean net_write_error(void)
{
#ifdef G_OS_WIN32
	int lerror = WSAGetLastError();
	return (lerror != WSAECONNRESET
		&& lerror != WSAECONNABORTED && lerror != WSAESHUTDOWN);
#else				/* G_OS_WIN32 */
	return errno != EPIPE;
#endif				/* G_OS_WIN32 */
}

/* Returns the message for error# number */
static const gchar *net_errormsg_nr(gint number)
{
	return g_strerror(number);
}

/* Returns the message for the current error */
static const gchar *net_errormsg(void)
{
	return net_errormsg_nr(errno);
}

gboolean net_close(Session * ses)
{
	if (ses->timer_id != 0) {
		g_source_remove(ses->timer_id);
		ses->timer_id = 0;
	}

	if (ses->fd >= 0) {
		listen_read(ses, FALSE);
		listen_write(ses, FALSE);
		net_closesocket(ses->fd);
		ses->fd = -1;

		while (ses->write_queue != NULL) {
			char *data = ses->write_queue->data;

			ses->write_queue
			    = g_list_remove(ses->write_queue, data);
			g_free(data);
		}
	}
#ifdef HAVE_GETADDRINFO_ET_AL
	if (ses->base_ai) {
		freeaddrinfo(ses->base_ai);
		ses->base_ai = NULL;
		ses->current_ai = NULL;
	}
#endif				/* HAVE_GETADDRINFO_ET_AL */

	return !ses->entered;
}

void net_close_when_flushed(Session * ses)
{
	ses->waiting_for_close = TRUE;
	if (ses->write_queue != NULL)
		return;

	if (net_close(ses))
		notify(ses, NET_CLOSE, NULL);
}

void net_wait_for_close(Session * ses)
{
	ses->waiting_for_close = TRUE;
}

static void close_and_callback(Session * ses)
{
	if (net_close(ses))
		notify(ses, NET_CLOSE, NULL);
}

static gboolean ping_function(gpointer s)
{
	Session *ses = (Session *) s;
	double interval = difftime(time(NULL), ses->last_response);
	/* Ask for activity every PING_PERIOD seconds, but don't ask if there
	 * was activity anyway.  */
	if (interval >= 2 * PING_PERIOD) {
		/* There was no response to the ping in time.  The connection
		 * should be considered dead.  */
		log_message(MSG_ERROR,
			    "No activity and no response to ping.  Closing connection\n");
		debug("(%d) --> %s", ses->fd, "no response");
		close_and_callback(ses);
	} else if (interval >= PING_PERIOD) {
		/* There was no activity.
		 * Send a ping (but don't update activity time).  */
		net_write(ses, "hello\n");
		ses->timer_id =
		    g_timeout_add(PING_PERIOD * 1000, ping_function, s);
	} else {
		/* Everything is fine.  Reschedule this check.  */
		ses->timer_id =
		    g_timeout_add((PING_PERIOD - interval) * 1000,
				  ping_function, s);
	}
	/* Return FALSE to not reschedule this timeout.  If it needed to be
	 * rescheduled, it has been done explicitly above (with a different
	 * timeout).  */
	return FALSE;
}

static void write_ready(Session * ses)
{
	if (!ses || ses->fd < 0)
		return;
	if (ses->connect_in_progress) {
		/* We were waiting to connect to server
		 */
		int error;
		socklen_t error_len;

		error_len = sizeof(error);
		if (getsockopt(ses->fd, SOL_SOCKET, SO_ERROR,
			       (GETSOCKOPT_ARG3) & error,
			       &error_len) < 0) {
			notify(ses, NET_CONNECT_FAIL, NULL);
			log_message(MSG_ERROR,
				    _(""
				      "Error checking connect status: %s\n"),
				    net_errormsg());
			net_close(ses);
		} else if (error != 0) {
#ifdef HAVE_GETADDRINFO_ET_AL
			if (ses->current_ai && ses->current_ai->ai_next) {
				// There are some protocols left to try
				ses->current_ai = ses->current_ai->ai_next;
				listen_read(ses, FALSE);
				listen_write(ses, FALSE);
				net_closesocket(ses->fd);
				ses->fd = -1;
				net_attempt_to_connect(ses);
				return;
			}
#endif
			log_message(MSG_ERROR,
				    _(""
				      "Error connecting to host '%s': %s\n"),
				    ses->host, net_errormsg_nr(error));
			notify(ses, NET_CONNECT_FAIL, NULL);
			net_close(ses);
		} else {
			ses->connect_in_progress = FALSE;
			notify(ses, NET_CONNECT, NULL);
			listen_write(ses, FALSE);
			listen_read(ses, TRUE);
		}
		return;
	}

	while (ses->write_queue != NULL) {
		int num;
		char *data = ses->write_queue->data;
		int len = strlen(data);

		num = send(ses->fd, data, len, 0);
		debug("write_ready: write(%d, \"%.*s\", %d) = %d",
		      ses->fd, len, data, len, num);
		if (num < 0) {
			if (net_would_block())
				break;
			if (net_write_error())
				log_message(MSG_ERROR,
					    _(""
					      "Error writing socket: %s\n"),
					    net_errormsg());
			close_and_callback(ses);
			return;
		} else if (num == len) {
			ses->write_queue
			    = g_list_remove(ses->write_queue, data);
			g_free(data);
		} else {
			memmove(data, data + num, len - num + 1);
			break;
		}
	}

	/* Stop spinning when nothing to do.
	 */
	if (ses->write_queue == NULL) {
		if (ses->waiting_for_close)
			close_and_callback(ses);
		else
			listen_write(ses, FALSE);
	}
}

void net_write(Session * ses, const gchar * data)
{
	if (!ses || ses->fd < 0)
		return;
	if (ses->write_queue != NULL || !net_connected(ses)) {
		/* reassign the pointer, because the glib docs say it may
		 * change and because if we're in the process of connecting the
		 * pointer may currently be null. */
		ses->write_queue =
		    g_list_append(ses->write_queue, g_strdup(data));
	} else {
		int len;
		int num;

		len = strlen(data);
		num = send(ses->fd, data, len, 0);
		if (num > 0) {
			if (strcmp(data, "yes\n")
			    && strcmp(data, "hello\n"))
				debug("(%d) --> %s", ses->fd, data);
		} else if (!net_would_block())
			debug("(%d) --- Error writing to socket.",
			      ses->fd);
		if (num < 0) {
			if (!net_would_block()) {
				log_message(MSG_ERROR,
					    _(""
					      "Error writing to socket: %s\n"),
					    net_errormsg());
				close_and_callback(ses);
				return;
			}
			num = 0;
		}
		if (num != len) {
			ses->write_queue
			    = g_list_append(NULL, g_strdup(data + num));
			listen_write(ses, TRUE);
		}
	}
}

void net_printf(Session * ses, const gchar * fmt, ...)
{
	char *buff;
	va_list ap;

	va_start(ap, fmt);
	buff = g_strdup_vprintf(fmt, ap);
	va_end(ap);

	net_write(ses, buff);
	g_free(buff);
}

static int find_line(char *buff, int len)
{
	int idx;

	for (idx = 0; idx < len; idx++)
		if (buff[idx] == '\n')
			return idx;
	return -1;
}

static void read_ready(Session * ses)
{
	int num;
	int offset;

	/* There is data from this connection: record the time.  */
	ses->last_response = time(NULL);

	if (ses->read_len == sizeof(ses->read_buff)) {
		/* We are in trouble now - the application has not
		 * been processing the data we have been
		 * reading. Assume something has gone wrong and
		 * disconnect
		 */
		log_message(MSG_ERROR,
			    _("Read buffer overflow - disconnecting\n"));
		close_and_callback(ses);
		return;
	}

	num = recv(ses->fd, ses->read_buff + ses->read_len,
		   sizeof(ses->read_buff) - ses->read_len, 0);
	if (num < 0) {
		if (net_would_block())
			return;
		log_message(MSG_ERROR, _("Error reading socket: %s\n"),
			    net_errormsg());
		close_and_callback(ses);
		return;
	}

	if (num == 0) {
		close_and_callback(ses);
		return;
	}

	ses->read_len += num;

	if (ses->entered)
		return;
	ses->entered = TRUE;

	offset = 0;
	while (ses->fd >= 0 && offset < ses->read_len) {
		char *line = ses->read_buff + offset;
		int len = find_line(line, ses->read_len - offset);

		if (len < 0)
			break;
		line[len] = '\0';
		offset += len + 1;

		if (!strcmp(line, "hello")) {
			net_write(ses, "yes\n");
			continue;
		}
		if (!strcmp(line, "yes")) {
			continue;	/* Don't notify the program */
		}

		debug("(%d) <-- %s", ses->fd, line);

		notify(ses, NET_READ, line);
	}

	if (offset < ses->read_len) {
		/* Did not process all data in buffer - discard
		 * processed data and copy remaining data to beginning
		 * of buffer until next time
		 */
		memmove(ses->read_buff, ses->read_buff + offset,
			ses->read_len - offset);
		ses->read_len -= offset;
	} else
		/* Processed all data in buffer, discard it
		 */
		ses->read_len = 0;

	ses->entered = FALSE;
	if (ses->fd < 0) {
		close_and_callback(ses);
	}
}

Session *net_new(NetNotifyFunc notify_func, void *user_data)
{
	Session *ses;

	ses = g_malloc0(sizeof(*ses));
	ses->notify_func = notify_func;
	ses->user_data = user_data;
	ses->fd = -1;

	return ses;
}

void net_use_fd(Session * ses, int fd, gboolean do_ping)
{
	ses->fd = fd;
	if (do_ping) {
		ses->last_response = time(NULL);
		ses->timer_id =
		    g_timeout_add(PING_PERIOD * 1000, ping_function, ses);
	}
	listen_read(ses, TRUE);
}

gboolean net_connected(Session * ses)
{
	return ses->fd >= 0 && !ses->connect_in_progress;
}

/* Set the socket to non-blocking
 * @param fd The file descriptor of the socket
 * @return TRUE if an error occurred
 */
static gboolean net_set_socket_non_blocking(int fd)
{
#ifdef HAVE_FCNTL
	return fcntl(fd, F_SETFL, O_NDELAY) < 0;
#else				/* HAVE_FCNTL */
#ifdef HAVE_WS2TCPIP_H
	unsigned long nonblocking = 1;
	return ioctlsocket(fd, FIONBIO, &nonblocking) != 0;
#else				/* HAVE_FCNTL && HAVE_WS2TCPIP_H */
#error "Don't know how to set a socket non-blocking"
#error "Please contact the mailing list,"
#error "and send the file config.h"
	return TRUE;
#endif
#endif
}

/* Is the connection in progress?
 * @return TRUE The connection is in progress
 */
static gboolean net_is_connection_in_progress(void)
{
#ifdef G_OS_WIN32
	return WSAGetLastError() == WSAEWOULDBLOCK;
#else
	return errno == EINPROGRESS;
#endif
}

/**
 * Attempt to connect the session.
 * @param ses Session
 */
static void net_attempt_to_connect(Session * ses)
{
#ifdef HAVE_GETADDRINFO_ET_AL
	struct addrinfo *aip;
#else
	struct hostent *he;
	struct sockaddr_in addr;
#endif				/* HAVE_GETADDRINFO_ET_AL */

#ifdef HAVE_GETADDRINFO_ET_AL
	aip = ses->current_ai;
	g_return_if_fail(aip != NULL);

	ses->fd = socket(aip->ai_family, SOCK_STREAM, 0);
#else				/* HAVE_GETADDRINFO_ET_AL */
	ses->fd = socket(AF_INET, SOCK_STREAM, 0);
#endif				/* HAVE_GETADDRINFO_ET_AL */
	if (ses->fd < 0) {
		log_message(MSG_ERROR,
			    _("Error creating socket: %s\n"),
			    net_errormsg());
		return;
	}
#ifdef HAVE_FCNTL
	if (fcntl(ses->fd, F_SETFD, 1) < 0) {
		log_message(MSG_ERROR,
			    _("Error setting socket close-on-exec: %s\n"),
			    net_errormsg());
		net_closesocket(ses->fd);
		ses->fd = -1;
		return;
	}
#endif
	if (net_set_socket_non_blocking(ses->fd)) {
		log_message(MSG_ERROR,
			    _("Error setting socket non-blocking: %s\n"),
			    net_errormsg());
		net_closesocket(ses->fd);
		ses->fd = -1;
		return;
	}
#ifdef HAVE_GETADDRINFO_ET_AL
	if (connect(ses->fd, aip->ai_addr, aip->ai_addrlen) < 0) {
#else				/* HAVE_GETADDRINFO_ET_AL */
	he = gethostbyname(ses->host);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(ses->port));
	addr.sin_addr = *((struct in_addr *) he->h_addr);
	memset(&addr.sin_zero, 0, 8);
	if (connect(ses->fd, (struct sockaddr *) &addr,
		    sizeof(struct sockaddr)) < 0) {
#endif				/* HAVE_GETADDRINFO_ET_AL */
		if (net_is_connection_in_progress()) {
			ses->connect_in_progress = TRUE;
			listen_write(ses, TRUE);
			return;
		} else {
			log_message(MSG_ERROR,
				    _("Error connecting to %s: %s\n"),
				    ses->host, net_errormsg());
			net_closesocket(ses->fd);
			ses->fd = -1;
			return;
		}
	} else
		listen_read(ses, TRUE);
	return;
}

gboolean net_connect(Session * ses, const gchar * host, const gchar * port)
{
#ifdef HAVE_GETADDRINFO_ET_AL
	int err;
	struct addrinfo hints;
	struct addrinfo *ai;
#else
	struct hostent *he;
	struct sockaddr_in addr;
#endif				/* HAVE_GETADDRINFO_ET_AL */

	net_close(ses);
	if (ses->host != NULL)
		g_free(ses->host);
	if (ses->port != NULL)
		g_free(ses->port);
	ses->host = g_strdup(host);
	ses->port = g_strdup(port);

#ifdef HAVE_GETADDRINFO_ET_AL
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = NETWORK_PROTOCOL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((err = getaddrinfo(host, port, &hints, &ai))) {
		log_message(MSG_ERROR,
			    _("Cannot resolve %s port %s: %s\n"), host,
			    port, gai_strerror(err));
		return FALSE;
	}
	if (!ai) {
		log_message(MSG_ERROR,
			    _(""
			      "Cannot resolve %s port %s: host not found\n"),
			    host, port);
		return FALSE;
	}
	ses->base_ai = ai;
	ses->current_ai = ai;
#endif				/* HAVE_GETADDRINFO_ET_AL */

	net_attempt_to_connect(ses);

	return (ses->fd >= 0);
}

/* Free and NULL-ify the session *ses */
void net_free(Session ** ses)
{
	/* If the sessions is still in use, do not free it */
	if (!net_close(*ses)) {
		return;
	}
	if ((*ses)->host != NULL)
		g_free((*ses)->host);
	if ((*ses)->port != NULL)
		g_free((*ses)->port);
	g_free(*ses);
	*ses = NULL;
}

gchar *get_my_hostname(void)
{
/* The following code fragment is taken from glib-2.0 v2.8 */
	gchar hostname[100];
#ifndef G_OS_WIN32
	gboolean hostname_fail =
	    (gethostname(hostname, sizeof(hostname)) == -1);
#else
	DWORD size = sizeof(hostname);
	gboolean hostname_fail = (!GetComputerName(hostname, &size));
#endif
	return g_strdup(hostname_fail ? "localhost" : hostname);
/* End of copy from glib-2.0 v2.8 */
}

gchar *get_meta_server_name(gboolean use_default)
{
	gchar *temp;

	temp = g_strdup(g_getenv("PIONEERS_META_SERVER"));
	if (!temp)
		temp = g_strdup(g_getenv("GNOCATAN_META_SERVER"));
	if (!temp) {
		if (use_default)
			temp = g_strdup(PIONEERS_DEFAULT_META_SERVER);
		else {
			temp = get_my_hostname();
		}
	}
	return temp;
}

const gchar *get_pioneers_dir(void)
{
	const gchar *pioneers_dir = g_getenv("PIONEERS_DIR");
	if (!pioneers_dir)
		pioneers_dir = g_getenv("GNOCATAN_DIR");
	if (!pioneers_dir)
		pioneers_dir = PIONEERS_DIR_DEFAULT;
	return pioneers_dir;
}

int net_open_listening_socket(const gchar * port, gchar ** error_message)
{
#ifdef HAVE_GETADDRINFO_ET_AL
	int err;
	struct addrinfo hints, *ai, *aip;
	int yes;
	gint fd = -1;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = NETWORK_PROTOCOL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if ((err = getaddrinfo(NULL, port, &hints, &ai)) || !ai) {
		*error_message =
		    g_strdup_printf(_(""
				      "Error creating struct addrinfo: %s"),
				    gai_strerror(err));
		return -1;
	}

	for (aip = ai; aip; aip = aip->ai_next) {
		fd = socket(aip->ai_family, SOCK_STREAM, 0);
		if (fd < 0) {
			continue;
		}
		yes = 1;

		/* setsockopt() before bind(); otherwise it has no effect! -- egnor */
		if (setsockopt
		    (fd, SOL_SOCKET, SO_REUSEADDR, &yes,
		     sizeof(yes)) < 0) {
			net_closesocket(fd);
			continue;
		}
		if (bind(fd, aip->ai_addr, aip->ai_addrlen) < 0) {
			net_closesocket(fd);
			continue;
		}

		break;
	}

	if (!aip) {
		*error_message =
		    g_strdup_printf(_(""
				      "Error creating listening socket: %s\n"),
				    net_errormsg());
		freeaddrinfo(ai);
		return -1;
	}

	freeaddrinfo(ai);

	if (net_set_socket_non_blocking(fd)) {
		*error_message =
		    g_strdup_printf(_(""
				      "Error setting socket non-blocking: %s\n"),
				    net_errormsg());
		net_closesocket(fd);
		return -1;
	}

	if (listen(fd, 5) < 0) {
		*error_message =
		    g_strdup_printf(_(""
				      "Error during listen on socket: %s\n"),
				    net_errormsg());
		net_closesocket(fd);
		return -1;
	}
	*error_message = NULL;
	return fd;
#else				/* HAVE_GETADDRINFO_ET_AL */
	*error_message =
	    g_strdup(_("Listening not yet supported on this platform."));
	return -1;
#endif				/* HAVE_GETADDRINFO_ET_AL */
}

void net_closesocket(int fd)
{
#ifdef G_OS_WIN32
	closesocket(fd);
#else				/* G_OS_WIN32 */
	close(fd);
#endif				/* G_OS_WIN32 */
}

gboolean net_get_peer_name(gint fd, gchar ** hostname, gchar ** servname,
			   gchar ** error_message)
{
#ifdef HAVE_GETADDRINFO_ET_AL
	sockaddr_t peer;
	socklen_t peer_len;
#endif				/* HAVE_GETADDRINFO_ET_AL */

	*hostname = g_strdup(_("unknown"));
	*servname = g_strdup(_("unknown"));

#ifdef HAVE_GETADDRINFO_ET_AL
	peer_len = sizeof(peer);
	if (getpeername(fd, &peer.sa, &peer_len) < 0) {
		*error_message =
		    g_strdup_printf(_("Error getting peer name: %s"),
				    net_errormsg());
		return FALSE;
	} else {
		int err;
		char host[NI_MAXHOST];
		char port[NI_MAXSERV];

		if ((err =
		     getnameinfo(&peer.sa, peer_len, host, NI_MAXHOST,
				 port, NI_MAXSERV, 0))) {
			*error_message =
			    g_strdup_printf(_(""
					      "Error resolving address: %s"),
					    gai_strerror(err));
			return FALSE;
		} else {
			g_free(*hostname);
			g_free(*servname);
			*hostname = g_strdup(host);
			*servname = g_strdup(port);
			return TRUE;
		}
	}
#else				/* HAVE_GETADDRINFO_ET_AL */
	*error_message =
	    g_strdup(_(""
		       "Net_get_peer_name not yet supported on this platform."));
	return FALSE;
#endif				/* HAVE_GETADDRINFO_ET_AL */
}

gint net_accept(gint accept_fd, gchar ** error_message)
{
	gint fd;
	sockaddr_t addr;
	socklen_t addr_len;

	addr_len = sizeof(addr);
	fd = accept(accept_fd, &addr.sa, &addr_len);
	if (fd < 0) {
		*error_message =
		    g_strdup_printf(_("Error accepting connection: %s"),
				    net_errormsg());
	}
	return fd;
}

void net_init(void)
{
#ifdef G_OS_WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	if (0 != WSAStartup(wVersionRequested, &wsaData)) {
		g_error("No usable version of WinSock was found.");
	}
#else				/* G_OS_WIN32 */
	/* Do nothing on unix like platforms */
#endif				/* G_OS_WIN32 */
}

void net_finish(void)
{
#ifdef G_OS_WIN32
	WSACleanup();
#else				/* G_OS_WIN32 */
	/* Do nothing on unix like platforms */
#endif				/* G_OS_WIN32 */
}

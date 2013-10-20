/*
 Part of multithreaded network servers. The servers MAIN thread is started by
 the main program. It starts in turn the ACCEPTER thread, which accepts a
 connection and passes the file descriptor to the MAIN thread. The server MAIN
 thread starts a PORT SERVER thread to handle the connection. Thus, the 
 PORT SERVER thread is NOT a child of ACCEPTER, but a brother and SIGCHILD goes
 to the server MAIN. If it would go to ACCEPTER, which is blocked in the 
 accept() system call, we would get "process interrupted in system call" and 
 that's it. Maybe its different with pthread library..?

 Part of Libnodave, a free communication libray for Siemens S7 300/400.
 
 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2001..2005.
 (C) Luis Matos (gass@otiliamatos.ath.cx) 2013.

    libads is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libads is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Lesser GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with libads.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <byteswap.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#define ThisModule "Accepter : "
#include "ads.h"
#include "accepter.h"

void *accepter(void *arg)
{
	int s;
	int newfd, pipefd;
	socklen_t addrlen;
	struct sockaddr_in addr;
	int opt;
	usleep(10000);
	pipefd = ((accepter_info *) arg)->fd;
	ads_debug(ADSDebug, ThisModule "My pipe is:%d\n", pipefd);
	;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (errno != 0) {
		ads_debug(ADSDebug, ThisModule "socket %s\n",
			  strerror(errno));
		;
	}
	ads_debug(ADSDebug, ThisModule "port %d\n",
		  ((accepter_info *) arg)->port);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(((accepter_info *) arg)->port);
	inet_aton("0.0.0.0", &addr.sin_addr);
	opt = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, 4);
	ads_debug(ADSDebug, ThisModule "setsockopt %s\n",
		  strerror(errno));
	addrlen = sizeof(addr);
	bind(s, (struct sockaddr *)&addr, addrlen);
	ads_debug(ADSDebug, ThisModule "bind %s\n", strerror(errno));
	listen(s, 1);
	ads_debug(ADSDebug, ThisModule "listen: %s\n",
		  strerror(errno));
	while (1) {
		addrlen = sizeof(addr);
		ads_debug(ADSDebug, ThisModule "before accept\n");
		newfd = (accept(s, (struct sockaddr *)&addr, &addrlen));
		ads_debug(ADSDebug,
			  ThisModule "after accept. New fd:%d\n",
			  newfd);
		;
		ads_debug(ADSDebug, ThisModule "PID(%d) client:%s\n",
			  getpid(), inet_ntoa(addr.sin_addr));
		;
		write(pipefd, &newfd, sizeof(newfd));
	}
	shutdown(s, 2);
	ads_debug(ADSDebug,
		  ThisModule "%d shutdown done. Socket is: %d\n",
		  getpid(), s);
}


/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2018 Fabian Freyer <fabian.freyer@physik.tu-berlin.de>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "monitor.h"

static int monitor_fd;

int
monitor_parse(const char *opt)
{
	char *uopt, *tmpstr, *host;
	char listen_hostout[INET_ADDRSTRLEN];
	struct sockaddr_in s_in;

	tmpstr = uopt = strdup(opt);
	host = NULL;

	if (strncmp(uopt, "unix:", 5) == 0) {
		uopt += 5;

		fprintf(stderr, "Opening unix domain socket: %s\n", uopt);
	} else if (strncmp(uopt, "tcp:", 4) == 0) {
		uopt += 4;

		s_in.sin_len = sizeof(struct sockaddr);
		s_in.sin_family = AF_INET;
		tmpstr = strsep(&uopt, ":");
		if (!uopt) {
			s_in.sin_addr.s_addr = htonl(INADDR_ANY);
			s_in.sin_port = htons(atoi(tmpstr));
		} else {
			if(inet_pton(AF_INET, tmpstr, &s_in.sin_addr) != 1)
				errx(EX_USAGE, "Invalid listen address: %s", tmpstr);

			s_in.sin_port = htons(atoi(uopt));
		}

		if (!s_in.sin_port)
			errx(EX_USAGE, "Invalid port number specified");

		fprintf(stderr, "Opening tcp socket: %s:%d\n", inet_ntop(
					AF_INET,
					&s_in.sin_addr,
					listen_hostout,
					INET_ADDRSTRLEN
					),
				ntohs(s_in.sin_port));

		if ((monitor_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}

		if (bind(monitor_fd, (struct sockaddr*)&s_in, sizeof(s_in)) < 0) {
			perror("bind");
			exit(1);
		}

		system("/bin/sh");
	} else
		errx(EX_USAGE, "Invalid monitor socket specification: %s\n", uopt);

		free(uopt);

	return 0;
}

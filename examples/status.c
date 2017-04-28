/*
 * Copyright (c) 2017, Spectra Logic Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * thislist of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <err.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libifconfig.h>


int
main(int argc, char *argv[])
{
	char *ifname;
	int fib, metric, mtu;
	ifconfig_handle_t *lifh;

	if (argc != 2)
		errx(1, "Usage: example_status <IFNAME>");

	lifh = ifconfig_open();
	if (lifh == NULL)
		errx(1, "Failed to open libifconfig handle.");
	ifname = strdup(argv[1]);

	printf("%s: ", ifname);
	
	if (ifconfig_get_metric(lifh, ifname, &metric) == 0)
		printf("metric %d ", metric);
	else
		err(1, "Failed to get interface metric");
	if (ifconfig_get_mtu(lifh, ifname, &mtu) == 0)
		printf("mtu %d\n", mtu);
	else
		err(1, "Failed to get interface MTU");

	if (ifconfig_get_fib(lifh, ifname, &fib) == 0)
		printf("\tfib: %d\n", fib);
	else
		err(1, "Failed to get interface FIB");

	ifconfig_close(lifh);
	lifh = NULL;
	free(ifname);
	return (-1);
}

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
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libifconfig.h>

static void
cb(ifconfig_handle_t *lifh, struct ifaddrs *ifa)
{
	int fib, metric, mtu;
	char *description = NULL;
	struct ifconfig_capabilities caps;
	struct ifstat ifs;

	printf("%s: flags=%x ", ifa->ifa_name, ifa->ifa_flags);

	if (ifconfig_get_metric(lifh, ifa->ifa_name, &metric) == 0)
		printf("metric %d ", metric);
	else
		err(1, "Failed to get interface metric");

	if (ifconfig_get_mtu(lifh, ifa->ifa_name, &mtu) == 0)
		printf("mtu %d\n", mtu);
	else
		err(1, "Failed to get interface MTU");

	if (ifconfig_get_description(lifh, ifa->ifa_name, &description) == 0)
		printf("\tdescription: %s\n", description);

	if (ifconfig_get_capability(lifh, ifa->ifa_name, &caps) == 0) {
		if (caps.curcap != 0)
			printf("\toptions=%x\n", caps.curcap);
		if (caps.reqcap != 0)
			printf("\tcapabilities=%x\n", caps.reqcap);
	} else
		err(1, "Failed to get interface capabilities");

	if (ifconfig_get_fib(lifh, ifa->ifa_name, &fib) == 0) {
		if (fib != 0)
			printf("\tfib: %d\n", fib);
	} else
		err(1, "Failed to get interface FIB");

	if (ifconfig_get_ifstatus(lifh, ifa->ifa_name, &ifs) == 0)
		printf("%s", ifs.ascii);

	free(description);
}

int
main(int argc, char *argv[])
{
	ifconfig_handle_t *lifh;

	if (argc != 1)
		errx(1, "Usage: example_status");

	lifh = ifconfig_open();
	if (lifh == NULL)
		errx(1, "Failed to open libifconfig handle.");

	if (ifconfig_foreach_iface(lifh, cb) != 0)
		err(1, "Failed to get interfaces");

	ifconfig_close(lifh);
	lifh = NULL;
	return (-1);
}

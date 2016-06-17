/*
 * Copyright (c) 2016, Marie Helene Kvello-Aune
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 */


#include <net/if.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>


#include "libifconfig.h" // Needed for libifc_errstate
#include "libifconfig_internal.h"

int
libifc_ioctlwrap_ret(libifc_handle_t *h, unsigned long request, int rcode)
{
	if (rcode != 0) {
		h->error.errtype = IOCTL;
		h->error.ioctl_request = request;
		h->error.errcode = errno;
	}
	return (rcode);
}


int
libifc_ioctlwrap(libifc_handle_t *h, const int addressfamily,
    unsigned long request, struct ifreq *ifr)
{
	int s;

	if (libifc_socket(h, addressfamily, &s) != 0) {
		return (-1);
	}

	int rcode = ioctl(s, request, ifr);
	return (libifc_ioctlwrap_ret(h, request, rcode));
}


int
libifc_ioctlwrap_caddr(libifc_handle_t *h, const int addressfamily,
    unsigned long request, struct ifreq *ifr)
{
	int s;

	if (libifc_socket(h, addressfamily, &s) != 0) {
		return (-1);
	}

	int rcode = ioctl(s, request, (caddr_t)ifr);
	return (libifc_ioctlwrap_ret(h, request, rcode));
}


static int
sdexpand(libifc_handle_t *h)
{
	/* Initial size of dictionary is 4. If it needs to be larger, double it. */
	int newsize;

	if (h->sockets.sdsize == 0) {
		newsize = (4 * sizeof(int));
	} else {
		newsize = (2 * h->sockets.sdsize  * sizeof(int));
	}

	/*
	 * Don't use reallocf, as we want to keep the old allocation if
	 * this one fails.
	 */
	int *nsdkeys = realloc(h->sockets.sdkeys, newsize);

	/*
	 * TODO: Decide whether type 'SOCKET' should be strictly for socket(),
	 * or also for anything involving getting a socket.
	 */
	if (nsdkeys == NULL) {
		h->error.errtype = OTHER;
		h->error.errcode = ENOMEM;
		return (-1);
	}

	int *nsdvals = realloc(h->sockets.sdvals, newsize);
	if (nsdvals == NULL) {
		free(nsdkeys);
		h->error.errtype = OTHER;
		h->error.errcode = ENOMEM;
		return (-1);
	}

	/* Keep old arrays so we can free them later */
	int *osdkeys = h->sockets.sdkeys;
	int *osdvals = h->sockets.sdvals;

	/*
	 * If libifconfig is ever going to be thread-safe and not just
	 * thread-friendly, it's important sdvals is updated first.
	 * This would prevent a potential race condition when looking up keys
	 * whose value isn't set yet.
	 */
	h->sockets.sdvals = nsdvals;
	h->sockets.sdkeys = nsdkeys;

	/* Free old memory maps. */
	free(osdkeys);
	free(osdvals);
	/* Update size */
	h->sockets.sdsize = newsize;
	return (0);
}


/*
 * Function to get socket for the specified address family.
 * If the socket doesn't already exist, attempt to create it.
 */
int libifc_socket(libifc_handle_t *h, const int addressfamily, int *s)
{
	int sock;

	for (int i = 0; i < h->sockets.sdindex; i++) {
		if (h->sockets.sdkeys[i] == addressfamily) {
			*s = h->sockets.sdvals[i];
			return (0);
		}
	}

	/* We don't have a socket of that type available. Create one. */
	if ((h->sockets.sdindex == h->sockets.sdsize) && (sdexpand(h) != 0)) {
		/* Inherit error from sdexpand() */
		return (-1);
	}

	sock = socket(addressfamily, SOCK_DGRAM, 0);
	if (sock == -1) {
		h->error.errtype = SOCKET;
		h->error.errcode = errno;
		return (-1);
	}

	h->sockets.sdkeys[h->sockets.sdindex] = addressfamily;
	h->sockets.sdvals[h->sockets.sdindex] = sock;
	h->sockets.sdindex++;
	*s = sock;
	return (0);
}

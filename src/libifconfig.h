/*
 * Copyright (c) 2016-2017, Marie Helene Kvello-Aune
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

#pragma once

#include <netinet/in.h>
#include <netinet6/in6_var.h>

#define ND6_IFF_DEFAULTIF	0x8000

typedef enum {
	OK = 0,
	OTHER,
	IOCTL,
	SOCKET
} ifconfig_errtype;

/*
 * Opaque definition so calling application can just pass a
 * pointer to it for library use.
 */
struct ifconfig_handle;
typedef struct ifconfig_handle ifconfig_handle_t;

struct ifaddrs;
struct in6_ndireq;

struct ifconfig_capabilities {
	/** Current capabilities (ifconfig prints this as 'options')*/
	int curcap;
	/** Requested capabilities (ifconfig prints this as 'capabilities')*/
	int reqcap;
};

/** Stores extra info associated with an inet address */
struct ifconfig_inet_addr {
	const struct sockaddr_in	*sin;
	const struct sockaddr_in	*netmask;
	const struct sockaddr_in	*dst;
	const struct sockaddr_in	*broadcast;
	int				prefixlen;
	uint8_t				vhid;
};

/** Stores extra info associated with an inet6 address */
struct ifconfig_inet6_addr {
	struct sockaddr_in6	*sin6;
	struct sockaddr_in6	*dstin6;
	struct in6_addrlifetime	lifetime;
	int			prefixlen;
	uint32_t		flags;
	uint8_t			vhid;
};

/** Retrieves a new state object for use in other API calls.
 * Example usage:
 *{@code
 * // Create state object
 * ifconfig_handle_t *lifh;
 * lifh = ifconfig_open();
 * if (lifh == NULL) {
 *     // Handle error
 * }
 *
 * // Do stuff with the handle
 *
 * // Dispose of the state object
 * ifconfig_close(lifh);
 * lifh = NULL;
 *}
 */
ifconfig_handle_t *ifconfig_open(void);

/** Frees resources held in the provided state object.
 * @param h The state object to close.
 * @see #ifconfig_open(void)
 */
void ifconfig_close(ifconfig_handle_t *h);

/** Identifies what kind of error occured. */
ifconfig_errtype ifconfig_err_errtype(ifconfig_handle_t *h);

/** Retrieves the errno associated with the error, if any. */
int ifconfig_err_errno(ifconfig_handle_t *h);

/* TODO: should we add an opaque user-provided data pointer ? */
typedef void (*ifconfig_foreach_func_t)(ifconfig_handle_t *h,
    struct ifaddrs *ifa);

/** Iterate over every network interface
 * @param h	An open ifconfig state object
 * @param cb	A callback function to call with a pointer to each interface
 * @return	0 on success, nonzero if the list could not be iterated
 */
int ifconfig_foreach_iface(ifconfig_handle_t *h, ifconfig_foreach_func_t cb);

/** Iterate over every address on a single network interface
 * @param h	An open ifconfig state object
 * @param ifa	A pointer that was supplied by a previous call to
 * 		ifconfig_foreach_iface
 * @param cb	A callback function to call with a pointer to each ifaddr
 */
void ifconfig_foreach_ifaddr(ifconfig_handle_t *h, struct ifaddrs *ifa,
    ifconfig_foreach_func_t cb);

/** If error type was IOCTL, this identifies which request failed. */
unsigned long ifconfig_err_ioctlreq(ifconfig_handle_t *h);
int ifconfig_get_description(ifconfig_handle_t *h, const char *name,
    char **description);
int ifconfig_set_description(ifconfig_handle_t *h, const char *name,
    const char *newdescription);
int ifconfig_unset_description(ifconfig_handle_t *h, const char *name);
int ifconfig_set_name(ifconfig_handle_t *h, const char *name,
    const char *newname);
int ifconfig_set_fib(ifconfig_handle_t *h, const char *name, int fib);
int ifconfig_get_fib(ifconfig_handle_t *h, const char *name, int *fib);
int ifconfig_set_mtu(ifconfig_handle_t *h, const char *name, const int mtu);
int ifconfig_get_mtu(ifconfig_handle_t *h, const char *name, int *mtu);
int ifconfig_get_nd6(ifconfig_handle_t *h, const char *name,
    struct in6_ndireq *nd);
int ifconfig_set_metric(ifconfig_handle_t *h, const char *name,
    const int metric);
int ifconfig_get_metric(ifconfig_handle_t *h, const char *name, int *metric);
int ifconfig_set_capability(ifconfig_handle_t *h, const char *name,
    const int capability);
int ifconfig_get_capability(ifconfig_handle_t *h, const char *name,
    struct ifconfig_capabilities *capability);
/** Retrieve the list of groups to which this interface belongs
 * @param h	An open ifconfig state object
 * @param name	The interface name
 * @param ifgr	return argument.  The caller is responsible for freeing
 * 		ifgr->ifgr_groups
 * @return	0 on success, nonzero on failure
 */
int ifconfig_get_groups(ifconfig_handle_t *h, const char *name,
    struct ifgroupreq *ifgr);
int ifconfig_get_ifstatus(ifconfig_handle_t *h, const char *name,
    struct ifstat *stat);
/** Retrieve the interface media information
 * @param h	An open ifconfig state object
 * @param name	The interface name
 * @param ifgr	return argument.  The caller is responsible for freeing
 * 		ifmr->media_list
 * @return	0 on success, nonzero on failure
 * TODO: instead of returning ifmr as a return argument and requiring the
 * caller to free a portion of it, should we return ifmr as a return value and
 * require the caller to free the whole thing?
 */
int ifconfig_get_media(ifconfig_handle_t *h, const char *name,
    struct ifmediareq *ifmr);
const char* ifconfig_get_media_type(int ifmw);
const char* ifconfig_get_media_subtype(int ifmw);
const char* ifconfig_get_media_status(const struct ifmediareq *ifmr);
void ifconfig_get_media_options_string(int ifmw, char *buf, size_t buflen);

/** Retrieve additional information about an inet address
 * @param h	An open ifconfig state object
 * @param name	The interface name
 * @param ifa	Pointer to the the address structure of interest
 * @param addr	Return argument.  It will be filled with additional information
 * 		about the address.
 * @return	0 on success, nonzero on failure.
 */
int ifconfig_inet_get_addrinfo(ifconfig_handle_t *h,
    const char *name, struct ifaddrs *ifa, struct ifconfig_inet_addr *addr);

/** Retrieve additional information about an inet6 address
 * @param h	An open ifconfig state object
 * @param name	The interface name
 * @param ifa	Pointer to the the address structure of interest
 * @param addr	Return argument.  It will be filled with additional information
 * 		about the address.
 * @return	0 on success, nonzero on failure.
 */
int ifconfig_inet6_get_addrinfo(ifconfig_handle_t *h,
    const char *name, struct ifaddrs *ifa, struct ifconfig_inet6_addr *addr);

/** Destroy a virtual interface
 * @param name Interface to destroy
 */
int ifconfig_destroy_interface(ifconfig_handle_t *h, const char *name);

/** Creates a (virtual) interface
 * @param name Name of interface to create. Example: bridge or bridge42
 * @param name ifname Is set to actual name of created interface
 */
int ifconfig_create_interface(ifconfig_handle_t *h, const char *name,
    char **ifname);

/** Creates a (virtual) interface
 * @param name Name of interface to create. Example: vlan0 or ix0.50
 * @param name ifname Is set to actual name of created interface
 * @param vlandev Name of interface to attach to
 * @param vlanid VLAN ID/Tag. Must not be 0.
 */
int ifconfig_create_interface_vlan(ifconfig_handle_t *h, const char *name,
    char **ifname, const char *vlandev, const unsigned short vlantag);

int ifconfig_set_vlantag(ifconfig_handle_t *h, const char *name,
    const char *vlandev, const unsigned short vlantag);

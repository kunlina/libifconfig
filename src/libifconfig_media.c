/*
 * Copyright (c) 1983, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
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
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_media.h>
#include <net/route.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libifconfig.h"
#include "libifconfig_internal.h"


/*static void	domediaopt(const char *, int, int);*/
/*static int	get_media_subtype(int, const char *);*/
/*static int	get_media_mode(int, const char *);*/
/*static int	get_media_options(int, const char *);*/
/*static int	lookup_media_word(struct ifmedia_description *, const char *);*/
/*static void	print_media_word(int, int);*/
/*static void	print_media_word_ifconfig(int);*/

static struct ifmedia_description *get_toptype_desc(int);
static struct ifmedia_type_to_subtype *get_toptype_ttos(int);
static struct ifmedia_description *get_subtype_desc(int,
    struct ifmedia_type_to_subtype *ttos);

#define	IFM_OPMODE(x) \
	((x) & (IFM_IEEE80211_ADHOC | IFM_IEEE80211_HOSTAP | \
	 IFM_IEEE80211_IBSS | IFM_IEEE80211_WDS | IFM_IEEE80211_MONITOR | \
	 IFM_IEEE80211_MBSS))
#define	IFM_IEEE80211_STA	0

#if 0
struct ifmediareq *
ifmedia_getstate(int s)
{
	static struct ifmediareq *ifmr = NULL;
	int *mwords;
	int xmedia = 1;

	if (ifmr == NULL) {
		ifmr = (struct ifmediareq *)malloc(sizeof(struct ifmediareq));
		if (ifmr == NULL)
			err(1, "malloc");

		(void) memset(ifmr, 0, sizeof(struct ifmediareq));
		(void) strlcpy(ifmr->ifm_name, name,
		    sizeof(ifmr->ifm_name));

		ifmr->ifm_count = 0;
		ifmr->ifm_ulist = NULL;

		/*
		 * We must go through the motions of reading all
		 * supported media because we need to know both
		 * the current media type and the top-level type.
		 */

		if (ioctl(s, SIOCGIFXMEDIA, (caddr_t)ifmr) < 0) {
			xmedia = 0;
		}
		if (xmedia == 0 && ioctl(s, SIOCGIFMEDIA, (caddr_t)ifmr) < 0) {
			err(1, "SIOCGIFMEDIA");
		}

		if (ifmr->ifm_count == 0)
			errx(1, "%s: no media types?", name);

		mwords = (int *)malloc(ifmr->ifm_count * sizeof(int));
		if (mwords == NULL)
			err(1, "malloc");
  
		ifmr->ifm_ulist = mwords;
		if (xmedia) {
			if (ioctl(s, SIOCGIFXMEDIA, (caddr_t)ifmr) < 0)
				err(1, "SIOCGIFXMEDIA");
		} else {
			if (ioctl(s, SIOCGIFMEDIA, (caddr_t)ifmr) < 0)
				err(1, "SIOCGIFMEDIA");
		}
	}

	return ifmr;
}


/**********************************************************************
 * A good chunk of this is duplicated from sys/net/if_media.c
 **********************************************************************/

#endif
static struct ifmedia_description ifm_type_descriptions[] =
    IFM_TYPE_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_ethernet_descriptions[] =
    IFM_SUBTYPE_ETHERNET_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_ethernet_aliases[] =
    IFM_SUBTYPE_ETHERNET_ALIASES;

static struct ifmedia_description ifm_subtype_ethernet_option_descriptions[] =
    IFM_SUBTYPE_ETHERNET_OPTION_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_tokenring_descriptions[] =
    IFM_SUBTYPE_TOKENRING_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_tokenring_aliases[] =
    IFM_SUBTYPE_TOKENRING_ALIASES;

static struct ifmedia_description ifm_subtype_tokenring_option_descriptions[] =
    IFM_SUBTYPE_TOKENRING_OPTION_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_fddi_descriptions[] =
    IFM_SUBTYPE_FDDI_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_fddi_aliases[] =
    IFM_SUBTYPE_FDDI_ALIASES;

static struct ifmedia_description ifm_subtype_fddi_option_descriptions[] =
    IFM_SUBTYPE_FDDI_OPTION_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_ieee80211_descriptions[] =
    IFM_SUBTYPE_IEEE80211_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_ieee80211_aliases[] =
    IFM_SUBTYPE_IEEE80211_ALIASES;

static struct ifmedia_description ifm_subtype_ieee80211_option_descriptions[] =
    IFM_SUBTYPE_IEEE80211_OPTION_DESCRIPTIONS;

struct ifmedia_description ifm_subtype_ieee80211_mode_descriptions[] =
    IFM_SUBTYPE_IEEE80211_MODE_DESCRIPTIONS;

struct ifmedia_description ifm_subtype_ieee80211_mode_aliases[] =
    IFM_SUBTYPE_IEEE80211_MODE_ALIASES;

static struct ifmedia_description ifm_subtype_atm_descriptions[] =
    IFM_SUBTYPE_ATM_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_atm_aliases[] =
    IFM_SUBTYPE_ATM_ALIASES;

static struct ifmedia_description ifm_subtype_atm_option_descriptions[] =
    IFM_SUBTYPE_ATM_OPTION_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_shared_descriptions[] =
    IFM_SUBTYPE_SHARED_DESCRIPTIONS;

static struct ifmedia_description ifm_subtype_shared_aliases[] =
    IFM_SUBTYPE_SHARED_ALIASES;

static struct ifmedia_description ifm_shared_option_descriptions[] =
    IFM_SHARED_OPTION_DESCRIPTIONS;

static struct ifmedia_description ifm_shared_option_aliases[] =
    IFM_SHARED_OPTION_ALIASES;

struct ifmedia_type_to_subtype {
	struct {
		struct ifmedia_description *desc;
		int alias;
	} subtypes[5];
	struct {
		struct ifmedia_description *desc;
		int alias;
	} options[4];
	struct {
		struct ifmedia_description *desc;
		int alias;
	} modes[3];
};

/* must be in the same order as IFM_TYPE_DESCRIPTIONS */
static struct ifmedia_type_to_subtype ifmedia_types_to_subtypes[] = {
	{
		{
			{ &ifm_subtype_shared_descriptions[0], 0 },
			{ &ifm_subtype_shared_aliases[0], 1 },
			{ &ifm_subtype_ethernet_descriptions[0], 0 },
			{ &ifm_subtype_ethernet_aliases[0], 1 },
			{ NULL, 0 },
		},
		{
			{ &ifm_shared_option_descriptions[0], 0 },
			{ &ifm_shared_option_aliases[0], 1 },
			{ &ifm_subtype_ethernet_option_descriptions[0], 0 },
			{ NULL, 0 },
		},
		{
			{ NULL, 0 },
		},
	},
	{
		{
			{ &ifm_subtype_shared_descriptions[0], 0 },
			{ &ifm_subtype_shared_aliases[0], 1 },
			{ &ifm_subtype_tokenring_descriptions[0], 0 },
			{ &ifm_subtype_tokenring_aliases[0], 1 },
			{ NULL, 0 },
		},
		{
			{ &ifm_shared_option_descriptions[0], 0 },
			{ &ifm_shared_option_aliases[0], 1 },
			{ &ifm_subtype_tokenring_option_descriptions[0], 0 },
			{ NULL, 0 },
		},
		{
			{ NULL, 0 },
		},
	},
	{
		{
			{ &ifm_subtype_shared_descriptions[0], 0 },
			{ &ifm_subtype_shared_aliases[0], 1 },
			{ &ifm_subtype_fddi_descriptions[0], 0 },
			{ &ifm_subtype_fddi_aliases[0], 1 },
			{ NULL, 0 },
		},
		{
			{ &ifm_shared_option_descriptions[0], 0 },
			{ &ifm_shared_option_aliases[0], 1 },
			{ &ifm_subtype_fddi_option_descriptions[0], 0 },
			{ NULL, 0 },
		},
		{
			{ NULL, 0 },
		},
	},
	{
		{
			{ &ifm_subtype_shared_descriptions[0], 0 },
			{ &ifm_subtype_shared_aliases[0], 1 },
			{ &ifm_subtype_ieee80211_descriptions[0], 0 },
			{ &ifm_subtype_ieee80211_aliases[0], 1 },
			{ NULL, 0 },
		},
		{
			{ &ifm_shared_option_descriptions[0], 0 },
			{ &ifm_shared_option_aliases[0], 1 },
			{ &ifm_subtype_ieee80211_option_descriptions[0], 0 },
			{ NULL, 0 },
		},
		{
			{ &ifm_subtype_ieee80211_mode_descriptions[0], 0 },
			{ &ifm_subtype_ieee80211_mode_aliases[0], 0 },
			{ NULL, 0 },
		},
	},
	{
		{
			{ &ifm_subtype_shared_descriptions[0], 0 },
			{ &ifm_subtype_shared_aliases[0], 1 },
			{ &ifm_subtype_atm_descriptions[0], 0 },
			{ &ifm_subtype_atm_aliases[0], 1 },
			{ NULL, 0 },
		},
		{
			{ &ifm_shared_option_descriptions[0], 0 },
			{ &ifm_shared_option_aliases[0], 1 },
			{ &ifm_subtype_atm_option_descriptions[0], 0 },
			{ NULL, 0 },
		},
		{
			{ NULL, 0 },
		},
	},
};

#if 0
static int
get_media_subtype(int type, const char *val)
{
	struct ifmedia_description *desc;
	struct ifmedia_type_to_subtype *ttos;
	int rval, i;

	/* Find the top-level interface type. */
	for (desc = ifm_type_descriptions, ttos = ifmedia_types_to_subtypes;
	    desc->ifmt_string != NULL; desc++, ttos++)
		if (type == desc->ifmt_word)
			break;
	if (desc->ifmt_string == NULL)
		errx(1, "unknown media type 0x%x", type);

	for (i = 0; ttos->subtypes[i].desc != NULL; i++) {
		rval = lookup_media_word(ttos->subtypes[i].desc, val);
		if (rval != -1)
			return (rval);
	}
	errx(1, "unknown media subtype: %s", val);
	/*NOTREACHED*/
}

static int
get_media_mode(int type, const char *val)
{
	struct ifmedia_description *desc;
	struct ifmedia_type_to_subtype *ttos;
	int rval, i;

	/* Find the top-level interface type. */
	for (desc = ifm_type_descriptions, ttos = ifmedia_types_to_subtypes;
	    desc->ifmt_string != NULL; desc++, ttos++)
		if (type == desc->ifmt_word)
			break;
	if (desc->ifmt_string == NULL)
		errx(1, "unknown media mode 0x%x", type);

	for (i = 0; ttos->modes[i].desc != NULL; i++) {
		rval = lookup_media_word(ttos->modes[i].desc, val);
		if (rval != -1)
			return (rval);
	}
	return -1;
}

static int
get_media_options(int type, const char *val)
{
	struct ifmedia_description *desc;
	struct ifmedia_type_to_subtype *ttos;
	char *optlist, *optptr;
	int option = 0, i, rval = 0;

	/* We muck with the string, so copy it. */
	optlist = strdup(val);
	if (optlist == NULL)
		err(1, "strdup");

	/* Find the top-level interface type. */
	for (desc = ifm_type_descriptions, ttos = ifmedia_types_to_subtypes;
	    desc->ifmt_string != NULL; desc++, ttos++)
		if (type == desc->ifmt_word)
			break;
	if (desc->ifmt_string == NULL)
		errx(1, "unknown media type 0x%x", type);

	/*
	 * Look up the options in the user-provided comma-separated
	 * list.
	 */
	optptr = optlist;
	for (; (optptr = strtok(optptr, ",")) != NULL; optptr = NULL) {
		for (i = 0; ttos->options[i].desc != NULL; i++) {
			option = lookup_media_word(ttos->options[i].desc, optptr);
			if (option != -1)
				break;
		}
		if (option == 0)
			errx(1, "unknown option: %s", optptr);
		rval |= option;
	}

	free(optlist);
	return (rval);
}

static int
lookup_media_word(struct ifmedia_description *desc, const char *val)
{

	for (; desc->ifmt_string != NULL; desc++)
		if (strcasecmp(desc->ifmt_string, val) == 0)
			return (desc->ifmt_word);

	return (-1);
}

#endif
static struct ifmedia_description *get_toptype_desc(int ifmw)
{
	struct ifmedia_description *desc;

	for (desc = ifm_type_descriptions; desc->ifmt_string != NULL; desc++)
		if (IFM_TYPE(ifmw) == desc->ifmt_word)
			break;

	return desc;
}

static struct ifmedia_type_to_subtype *get_toptype_ttos(int ifmw)
{
	struct ifmedia_description *desc;
	struct ifmedia_type_to_subtype *ttos;

	for (desc = ifm_type_descriptions, ttos = ifmedia_types_to_subtypes;
	    desc->ifmt_string != NULL; desc++, ttos++)
		if (IFM_TYPE(ifmw) == desc->ifmt_word)
			break;

	return ttos;
}

static struct ifmedia_description *get_subtype_desc(int ifmw, 
    struct ifmedia_type_to_subtype *ttos)
{
	int i;
	struct ifmedia_description *desc;

	for (i = 0; ttos->subtypes[i].desc != NULL; i++) {
		if (ttos->subtypes[i].alias)
			continue;
		for (desc = ttos->subtypes[i].desc;
		    desc->ifmt_string != NULL; desc++) {
			if (IFM_SUBTYPE(ifmw) == desc->ifmt_word)
				return desc;
		}
	}

	return NULL;
}

#if 0
static struct ifmedia_description *get_mode_desc(int ifmw, 
    struct ifmedia_type_to_subtype *ttos)
{
	int i;
	struct ifmedia_description *desc;

	for (i = 0; ttos->modes[i].desc != NULL; i++) {
		if (ttos->modes[i].alias)
			continue;
		for (desc = ttos->modes[i].desc;
		    desc->ifmt_string != NULL; desc++) {
			if (IFM_MODE(ifmw) == desc->ifmt_word)
				return desc;
		}
	}

	return NULL;
}

#endif
const char*
ifconfig_get_media_type(int ifmw)
{
	struct ifmedia_description *desc;
	/*int seen_option = 0, i;*/

	/* Find the top-level interface type. */
	desc = get_toptype_desc(ifmw);
	if (desc->ifmt_string == NULL) {
		return("<unknown type>");
	} else {
		return(desc->ifmt_string);
	}
}

const char*
ifconfig_get_media_subtype(int ifmw)
{
	struct ifmedia_description *desc;
	struct ifmedia_type_to_subtype *ttos;

	ttos = get_toptype_ttos(ifmw);
	desc = get_subtype_desc(ifmw, ttos);
	return (desc->ifmt_string);
}
#if 0

	/*
	 * Don't print the top-level type; it's not like we can
	 * change it, or anything.
	 */

	/* Find subtype. */
	desc = get_subtype_desc(ifmw, ttos);
	if (desc == NULL) {
		printf("<unknown subtype>");
		return;
	}

	if (print_toptype)
		putchar(' ');

	printf("%s", desc->ifmt_string);

	if (print_toptype) {
		desc = get_mode_desc(ifmw, ttos);
		if (desc != NULL && strcasecmp("autoselect", desc->ifmt_string))
			printf(" mode %s", desc->ifmt_string);
	}

	/* Find options. */
	for (i = 0; ttos->options[i].desc != NULL; i++) {
		if (ttos->options[i].alias)
			continue;
		for (desc = ttos->options[i].desc;
		    desc->ifmt_string != NULL; desc++) {
			if (ifmw & desc->ifmt_word) {
				if (seen_option == 0)
					printf(" <");
				printf("%s%s", seen_option++ ? "," : "",
				    desc->ifmt_string);
			}
		}
	}
	printf("%s", seen_option ? ">" : "");

	if (print_toptype && IFM_INST(ifmw) != 0)
		printf(" instance %d", IFM_INST(ifmw));
}

static void
print_media_word_ifconfig(int ifmw)
{
	struct ifmedia_description *desc;
	struct ifmedia_type_to_subtype *ttos;
	int seen_option = 0, i;

	/* Find the top-level interface type. */
	desc = get_toptype_desc(ifmw);
	ttos = get_toptype_ttos(ifmw);
	if (desc->ifmt_string == NULL) {
		printf("<unknown type>");
		return;
	}

	/*
	 * Don't print the top-level type; it's not like we can
	 * change it, or anything.
	 */

	/* Find subtype. */
	desc = get_subtype_desc(ifmw, ttos);
	if (desc == NULL) {
		printf("<unknown subtype>");
		return;
	}

	printf("media %s", desc->ifmt_string);

	desc = get_mode_desc(ifmw, ttos);
	if (desc != NULL)
		printf(" mode %s", desc->ifmt_string);

	/* Find options. */
	for (i = 0; ttos->options[i].desc != NULL; i++) {
		if (ttos->options[i].alias)
			continue;
		for (desc = ttos->options[i].desc;
		    desc->ifmt_string != NULL; desc++) {
			if (ifmw & desc->ifmt_word) {
				if (seen_option == 0)
					printf(" mediaopt ");
				printf("%s%s", seen_option++ ? "," : "",
				    desc->ifmt_string);
			}
		}
	}

	if (IFM_INST(ifmw) != 0)
		printf(" instance %d", IFM_INST(ifmw));
}
#endif

/***************************************************************************
 * Above this point, this file is mostly copied from sbin/ifconfig/ifmedia.c
 ***************************************************************************/
int
ifconfig_get_media(ifconfig_handle_t *h, const char *name,
    struct ifmediareq *ifmr)
{
	int *media_list, s;
	unsigned long cmd = SIOCGIFXMEDIA;

	(void) memset(ifmr, 0, sizeof(*ifmr));
	(void) strlcpy(ifmr->ifm_name, name, sizeof(ifmr->ifm_name));

	if (ifconfig_socket(h, AF_LOCAL, &s) != 0) {
		return (-1);
	}

	/*
	 * Check if interface supports extended media types.
	 */
	if (ioctl(s, cmd, (caddr_t)ifmr) < 0) {
		cmd = SIOCGIFMEDIA;
		if (ioctl(s, cmd, (caddr_t)ifmr) < 0) {
			/* Interface doesn't support SIOC{G,S}IFMEDIA.  */
			h->error.errtype = OK;
			return (-1);
		}
	}
	if (ifmr->ifm_count == 0)
		return (0);	/* Interface has no media types ?*/

	media_list = (int *)calloc(ifmr->ifm_count, sizeof(int));
	if (media_list == NULL) {
		h->error.errtype = OTHER;
		h->error.errcode = ENOMEM;
		return (-1);
	}
	ifmr->ifm_ulist = media_list;

	if (ioctl(s, cmd, (caddr_t)ifmr) < 0) {
		h->error.errtype = IOCTL;
		h->error.ioctl_request = cmd;
		h->error.errcode = errno;
		return (-1);
	}

	return (0);
}

const char*
ifconfig_get_media_status(const struct ifmediareq *ifmr)
{
	switch (IFM_TYPE(ifmr->ifm_active)) {
	case IFM_ETHER:
	case IFM_ATM:
		if (ifmr->ifm_status & IFM_ACTIVE)
			return ("active");
		else
			return ("no carrier");
		break;

	case IFM_FDDI:
	case IFM_TOKEN:
		if (ifmr->ifm_status & IFM_ACTIVE)
			return ("inserted");
		else
			return ("no ring");
		break;

	case IFM_IEEE80211:
		if (ifmr->ifm_status & IFM_ACTIVE) {
			/* NB: only sta mode associates */
			if (IFM_OPMODE(ifmr->ifm_active) == IFM_IEEE80211_STA)
				return ("associated");
			else
				return ("running");
		} else
			return ("no carrier");
		break;
	default:
		return ("");
	}
}

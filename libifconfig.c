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
 * 4. Neither the name of the University nor the names of its contributors
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
 */

#include <sys/ioctl.h>

#include <net/if.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libifconfig.h"
#include "libifconfig_internal.h"

libifc_handle_t* libifc_open() {
        struct libifc_handle *h = calloc(1, sizeof(struct libifc_handle));
  
        return h;
}

void libifc_close(libifc_handle_t *h) {
        if (h->sockets.sdkeys != NULL) {
            free(h->sockets.sdkeys);
            h->sockets.sdkeys = NULL;
        }
        if (h->sockets.sdvals != NULL) {
            for (int i=0; i < h->sockets.sdindex; i++){
                close(h->sockets.sdvals[i]);
            }
            free(h->sockets.sdvals);
            h->sockets.sdvals = NULL;
        }
        h->sockets.sdindex = 0;
        free(h);
}

libifc_errtype libifc_err_errtype(libifc_handle_t *h) {
        return h->error.errtype;
}

int libifc_err_errno(libifc_handle_t *h) {
        return h->error.errcode;
}

int libifc_err_ioctlreq(libifc_handle_t *h) {
    return h->error.ioctl_request;
}

int libifc_get_description(libifc_handle_t *h, const char *name, char **description) {
        struct ifreq ifr;
        char *descr = NULL;
        size_t descrlen = 64;
        int s;
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        for (;;) {
            if ((descr = reallocf(descr, descrlen)) != NULL) {
                ifr.ifr_buffer.buffer = descr;
                ifr.ifr_buffer.length = descrlen;
                if (libifc_ioctlwrap(h, s, SIOCGIFDESCR, &ifr) == 0) {
                    if (ifr.ifr_buffer.buffer == descr) {
                        if (strlen(descr) > 0) {
                            *description = strdup(descr);
                            free(descr);
                            return 0;
                        }
                    } else if (ifr.ifr_buffer.length > descrlen) {
                        descrlen = ifr.ifr_buffer.length;
                        continue;
                    }
                }
                else {
                    return -1;
                }
            } else {
                free(descr);
                h->error.errtype = OTHER;
                h->error.errcode = ENOMEM;
                return -1;
            }
            break;
        }
        free(descr);
        h->error.errtype = OTHER;
        h->error.errcode = 0;
        return -1;
}

int libifc_set_description(libifc_handle_t *h, const char *name, const char *newdescription) {
        struct ifreq ifr;
        int desclen, s;
        desclen = strlen(newdescription);
        if (libifc_socket(h, AF_LOCAL, &s) != 0)            
            return -1; // Just return, as we can inherit error from ..._socket()
        
        // Unset description if the new description is 0 characters long.
        // TODO: Decide whether this should be an error condition instead.
        if (desclen == 0)
            return libifc_unset_description(h, name);

        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        
        ifr.ifr_buffer.length = desclen + 1;
        ifr.ifr_buffer.buffer = strdup(newdescription);
        if (ifr.ifr_buffer.buffer == NULL) {
            h->error.errtype = OTHER;
            h->error.errcode = ENOMEM;
            return -1;
        }
        
        /*
         * TODO: Check whether this ioctl() call truncates or fails when new
         * description is too long. If truncates, this function should probably
         * have an error condition for this further up.
         */
        if (libifc_ioctlwrap_caddr(h, s, SIOCSIFDESCR, &ifr) != 0) {
            if (ifr.ifr_buffer.buffer != NULL)
                free(ifr.ifr_buffer.buffer);
            return -1;
        }
        if (ifr.ifr_buffer.buffer != NULL)
            free(ifr.ifr_buffer.buffer); 
        return 0;
}

int libifc_unset_description(libifc_handle_t *h, const char *name) {
        struct ifreq ifr;
        int s;
        
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        ifr.ifr_buffer.length = 0;
        ifr.ifr_buffer.buffer = NULL;
        
        if (libifc_ioctlwrap_caddr(h, s, SIOCSIFDESCR, &ifr) < 0) {
            return -1;
        }
        return 0;
}

int libifc_set_name(libifc_handle_t *h, const char *name, const char *newname) {        
        struct ifreq ifr;
        char *tmpname;
        int s;
    
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        tmpname = strdup(newname);
        if (tmpname == NULL) {
            h->error.errtype = OTHER;
            h->error.errcode = ENOMEM;
            return -1;
        }
        
        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        ifr.ifr_data = tmpname;
        
        /*
         * TODO: Check whether this ioctl() call truncates or fails when new
         * name is too long. If truncates, this function should have an error
         * condition for this further up.
         */
        if (libifc_ioctlwrap_caddr(h, s, SIOCSIFNAME, &ifr) != 0) {
            free(tmpname);
            return -1;
        }
        free(tmpname);
        return 0;
}

int libifc_set_mtu(libifc_handle_t *h, const char *name, const int mtu){
        struct ifreq ifr;
        int s;
    
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        ifr.ifr_mtu = mtu;
        if (libifc_ioctlwrap_caddr(h, s, SIOCSIFMTU, &ifr) < 0) {
            return -1;
        }
        return 0;
}

int libifc_get_mtu(libifc_handle_t *h, const char *name, int *mtu) {
        struct ifreq ifr;
        int s;
    
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        if (libifc_ioctlwrap(h, s, SIOCGIFMTU, &ifr) == -1) {
            return -1;
        }
        *mtu = ifr.ifr_mtu;
        return 0;
}

int libifc_set_metric(libifc_handle_t *h, const char *name, const int mtu){
        struct ifreq ifr;
        int s;
    
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        ifr.ifr_mtu = mtu;
        if (libifc_ioctlwrap_caddr(h, s, SIOCSIFMETRIC, &ifr) < 0) {
            return -1;
        }
        return 0;
}

int libifc_get_metric(libifc_handle_t *h, const char *name, int *metric) {
        struct ifreq ifr;
        int s;
    
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        if (libifc_ioctlwrap(h, s, SIOCGIFMETRIC, &ifr) == -1) {
            return -1;
        }
        *metric = ifr.ifr_metric;
        return 0;
}

int libifc_set_capability(libifc_handle_t *h, const char *name, const int capability) {
        struct ifreq ifr;
        struct libifc_capabilities ifcap;
        int flags;
        int value, s;
        
        /*
         * Get the socket early, as if this fails
         * there's no point to _get_capability().
         */
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;
        
        if (libifc_get_capability(h, name, &ifcap) != 0)
            return -1;
        
        value = capability;        
        flags = ifcap.curcap;
        if (value < 0) {
            value = -value;
            flags &= ~value;
        } else
            flags |= value;
        flags &= ifcap.reqcap;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        /*
         * TODO: Verify that it's safe to not have ifr.ifr_curcap
         * set for this request.
         */
        ifr.ifr_reqcap = flags;
        if (libifc_ioctlwrap_caddr(h, s, SIOCSIFCAP, &ifr) < 0) {
            return -1;
        }
        return 0;
}

// Todo: convert 'capability' to struct libifc_capabilities
int libifc_get_capability(libifc_handle_t *h, const char *name, struct libifc_capabilities *capability) {
        struct ifreq ifr;
        int s;
    
        if (libifc_socket(h, AF_LOCAL, &s) != 0)
            return -1;

        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        
        if (libifc_ioctlwrap_caddr(h, s, SIOCGIFCAP, &ifr) < 0) {
            return -1;
        }
        capability->curcap = ifr.ifr_curcap;
        capability->reqcap = ifr.ifr_reqcap;
        return 0;
}

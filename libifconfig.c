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
//#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libifconfig.h"
#include "libifconfig_socketcache.h"

struct errstate libifconfig_errstate;

void libifconfig_free_resources(void) {
        libifconfig_socketcache_free_resources();
}


/// <summary> function used by other wrapper functions to populate _errstate when appropriate. </summary>
static int libifconfig_ioctlwrap_ret(unsigned long request, int rcode) {
    if (rcode != 0) {    
        libifconfig_errstate.errtype = IOCTL;
        libifconfig_errstate.ioctl_request = request;
        libifconfig_errstate.ioctl_err = rcode;
    }
    return rcode;
}

/// <summary> function to wrap ioctl() and automatically populate libifconfig_errstate when appropriate. </summary>
static int libifconfig_ioctlwrap(int s, unsigned long request, struct ifreq *ifr) {
    int rcode = ioctl(s, request, ifr);
    return libifconfig_ioctlwrap_ret(request, rcode);    
}

/// <summary> function to wrap ioctl(), casting ifr to caddr_t, and automatically populate libifconfig_errstate when appropriate. </summary>
static int libifconfig_ioctlwrap_caddr(int s, unsigned long request, struct ifreq *ifr) {
    int rcode = ioctl(s, request, (caddr_t)ifr);
    return libifconfig_ioctlwrap_ret(request, rcode);
}



int libifconfig_get_description(const char *name, char **description) {
        struct ifreq ifr;
        char *descr = NULL;
        size_t descrlen = 64;
        int s;
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        for (;;) {
            if ((descr = reallocf(descr, descrlen)) != NULL) {
                ifr.ifr_buffer.buffer = descr;
                ifr.ifr_buffer.length = descrlen;
                if (ioctl(s, SIOCGIFDESCR, &ifr) == 0) {
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
            } else {
                if (descr != NULL)
                    free(descr);
                libifconfig_errstate.errtype = ERRNO;
                errno = ENOMEM;
                return -1;
            }
            break;
        }
        if (descr != NULL)
            free(descr);
        libifconfig_errstate.errtype = OTHER;
        return -1;
}

int libifconfig_set_description(const char *name, const char *newdescription) {
        struct ifreq ifr;
        int desclen, s;
        desclen = strlen(newdescription);
        if (libifconfig_socket(AF_LOCAL, &s) != 0)            
            return -1; // Just return, as we can inherit error from ..._socket()
        
        // Unset description if the new description is 0 characters long.
        // TODO: Decide whether this should be an error condition instead.
        if (desclen == 0)
            return libifconfig_unset_description(name);

        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        
        ifr.ifr_buffer.length = desclen + 1;
        ifr.ifr_buffer.buffer = strdup(newdescription);
        if (ifr.ifr_buffer.buffer == NULL) {
            libifconfig_errstate.errtype = ERRNO;
            errno = ENOMEM;
            return -1;
        }
        
        if (libifconfig_ioctlwrap_caddr(s, SIOCSIFDESCR, &ifr) < 0) {
            if (ifr.ifr_buffer.buffer != NULL)
                free(ifr.ifr_buffer.buffer);
            return -1;
        }
        if (ifr.ifr_buffer.buffer != NULL)
            free(ifr.ifr_buffer.buffer); 
        return 0;
}

int libifconfig_unset_description(const char *name) {
        struct ifreq ifr;
        int s;
        
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        ifr.ifr_buffer.length = 0;
        ifr.ifr_buffer.buffer = NULL;
        
        if (libifconfig_ioctlwrap_caddr(s, SIOCSIFDESCR, &ifr) < 0) {
            return -1;
        }
        return 0;
}

int libifconfig_set_name(const char *name, const char *newname) {        
        struct ifreq ifr;
        char *tmpname;
        int s;
    
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        tmpname = strdup(newname);
        if (tmpname == NULL) {
            libifconfig_errstate.errtype = ERRNO;
            errno = ENOMEM;
            return -1;
        }
        
        strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
        ifr.ifr_data = tmpname;
        
        if (libifconfig_ioctlwrap_caddr(s, SIOCSIFNAME, &ifr) < 0) {
            free(tmpname);
            return -1;
        }
        free(tmpname);
        return 0;
}

int libifconfig_set_mtu(const char *name, const int mtu){
        struct ifreq ifr;
        int s;
    
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        ifr.ifr_mtu = mtu;
        if (libifconfig_ioctlwrap_caddr(s, SIOCSIFMTU, &ifr) < 0) {
            return -1;
        }
        return 0;
}

int libifconfig_get_mtu(const char *name, int *mtu) {
        struct ifreq ifr;
        int s;
    
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        if (libifconfig_ioctlwrap(s, SIOCGIFMTU, &ifr) == -1) {
            return -1;
        }
        *mtu = ifr.ifr_mtu;
        return 0;
}

int libifconfig_set_metric(const char *name, const int mtu){
        struct ifreq ifr;
        int s;
    
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        ifr.ifr_mtu = mtu;
        if (libifconfig_ioctlwrap_caddr(s, SIOCSIFMETRIC, &ifr) < 0) {
            return -1;
        }
        return 0;
}

int libifconfig_get_metric(const char *name, int *metric) {
        struct ifreq ifr;
        int s;
    
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        if (libifconfig_ioctlwrap(s, SIOCGIFMETRIC, &ifr) == -1) {
            return -1;
        }
        *metric = ifr.ifr_metric;
        return 0;
}

/*
 * TODO: Make this use libifconfig_get_capability
 */
int libifconfig_set_capability(const char *name, const int capability) {
        struct ifreq ifr;
        int flags;
        int value, s;
        
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;
        
        value = capability;
        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        
        if (libifconfig_ioctlwrap_caddr(s, SIOCGIFCAP, &ifr) < 0) {
            return -1;
        }
        
        flags = ifr.ifr_curcap;
        if (value < 0) {
            value = -value;
            flags &= ~value;
        } else
            flags |= value;
        flags &= ifr.ifr_reqcap;
        ifr.ifr_reqcap = flags;
        if (libifconfig_ioctlwrap_caddr(s, SIOCSIFCAP, &ifr) < 0) {
            return -1;
        }
        return 0;
} 

// Todo: convert 'capability' to struct libifconfig_capabilities
int libifconfig_get_capability(const char *name, int *capability) {
        struct ifreq ifr;
        int s;
    
        if (libifconfig_socket(AF_LOCAL, &s) != 0)
            return -1;

        strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
        
        if (libifconfig_ioctlwrap_caddr(s, SIOCGIFCAP, &ifr) < 0) {
            return -1;
        }
        *capability = ifr.ifr_curcap;
        return 0;
}

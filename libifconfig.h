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

#pragma once

typedef enum {
	OTHER, IOCTL, SOCKET
} libifc_errtype;

/*
 * Opaque definition so calling application can just pass a
 * pointer to it for library use.
 */
struct libifc_handle;
typedef struct libifc_handle   libifc_handle_t;

struct libifc_capabilities {
	/// <summary>Current capabilities (ifconfig prints this as 'options')</summary>
	int curcap;
	/// <summary>Requested capabilities (ifconfig prints this as 'capabilities')</summary>
	int reqcap;
};



libifc_handle_t *libifc_open(void);
void libifc_close(libifc_handle_t *h);

libifc_errtype libifc_err_errtype(libifc_handle_t *h);
int libifc_err_errno(libifc_handle_t *h);
unsigned long libifc_err_ioctlreq(libifc_handle_t *h);

int libifc_get_description(libifc_handle_t *h, const char *name,
    char **description);
int libifc_set_description(libifc_handle_t *h, const char *name,
    const char *newdescription);
int libifc_unset_description(libifc_handle_t *h, const char *name);

int libifc_set_name(libifc_handle_t *h, const char *name, const char *newname);

int libifc_set_mtu(libifc_handle_t *h, const char *name, const int mtu);
int libifc_get_mtu(libifc_handle_t *h, const char *name, int *mtu);

int libifc_set_metric(libifc_handle_t *h, const char *name, const int metric);
int libifc_get_metric(libifc_handle_t *h, const char *name, int *metric);

int libifc_set_capability(libifc_handle_t *h, const char *name,
    const int capability);
int libifc_get_capability(libifc_handle_t *h, const char *name,
    struct libifc_capabilities *capability);
int libifc_destroy_interface(libifc_handle_t *h, const char *name);

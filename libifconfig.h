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

typedef enum {OTHER, ERRNO, IOCTL, SOCKET} libifconfig_errtype;

// Contains libifconfig error state
struct errstate {
    /// <summary>Type of error.</summary>
    libifconfig_errtype errtype;
    /// <summary>
    /// The error occured in this ioctl() request.
    /// Populated if errtype = IOCTL
    /// </summary>
    int ioctl_request;
    /// <summary>
    /// Error code returned by the ioctl() call.
    /// Populated if errtype = IOCTL
    /// </summary>
    int ioctl_err;
};

extern struct errstate libifconfig_errstate;

// TODO: Use this in get_capability?
struct libifconfig_capabilities {
    /// <summary>Current capabilities</summary>
    int curcap;
    /// <summary>Requested capabilities</summary>
    int reqcap;
};

/// <summary>Frees resources held by the internal state of the library.</summary>
void libifconfig_free_resources(void);

int libifconfig_get_description(const char *name, char **description);
int libifconfig_set_description(const char *name, const char *newdescription);
int libifconfig_unset_description(const char *name);

int libifconfig_set_name(const char *name, const char *newname);

int libifconfig_set_mtu(const char *name, const int mtu);
int libifconfig_get_mtu(const char *name, int *mtu);

int libifconfig_set_metric(const char *name, const int metric);
int libifconfig_get_metric(const char *name, int *metric);

int libifconfig_set_capability(const char *name, const int capability);
int libifconfig_get_capability(const char *name, int *capability);

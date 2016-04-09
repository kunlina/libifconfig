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
#include <stdlib.h>

#include <unistd.h>

#include "libifconfig.h" // Needed for libifconfig_errstate
#include "libifconfig_socketcache.h"

static int sdindex = 0;
static int sdsize = 0;
static int *sdkeys = NULL;
static int *sdvals = NULL;

static int sdexpand() {
         // Initial size of dictionary is 4. If it needs to be larger, double it.        
        int newsize;
        if (sdsize == 0)
            newsize = (4 * sizeof(int));
        else
            newsize = (2 * sdsize  * sizeof(int));

        /*
         * Don't use reallocf, as we want to keep the old allocation if
         * this one fails.
         */
        int *nsdkeys = realloc(sdkeys, newsize);
        if (nsdkeys == NULL) {
            libifconfig_errstate.errtype = ERRNO;
            errno = ENOMEM;
            return -1;
        }
        
        int *nsdvals = realloc(sdvals, newsize);
        if (nsdvals == NULL) {
            free(nsdkeys);
            libifconfig_errstate.errtype = ERRNO;
            errno = ENOMEM;
            return -1;                
        }
        
        // Keep old arrays so we can free them later
        int *osdkeys = sdkeys;
        int *osdvals = sdvals;
        
        /*
         * Note to future thread-safety work: sdvals should be updated first,
         * to prevent race condition where address family is found in sdkeys
         * while sdvals doesn't have a socket yet.
         */
        sdvals = nsdvals;
        sdkeys = nsdkeys;
        
        // Free old memory maps.
        if (osdkeys != NULL)
            free(osdkeys);
        if (osdvals != NULL)
            free(osdvals);
        // Update size
        sdsize = newsize;
        return 0;        
}

/*
 * Function to get socket for the specified address family.
 * If the socket doesn't already exist, attempt to create it.
 */
int libifconfig_socket(const int addressfamily, int *s) {
        int sock;

        for (int i=0; i<sdindex; i++){
            if (sdkeys[i] == addressfamily) {
                *s = sdvals[i];
                return 0;
            }                
        }
        
         // We don't have a socket of that type available. Create one.    
        if (sdindex == sdsize && sdexpand() != 0) {
            // Just return here, as we can inherit error state from sdexpand().
            return -1; 
        }
        
        
        sock = socket(addressfamily, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            libifconfig_errstate.errtype = SOCKET;
            return -1; 
        }
        
        sdkeys[sdindex] = addressfamily;
        sdvals[sdindex] = sock;
        sdindex++;
        *s = sock;
        return 0;
}

void libifconfig_socketcache_free_resources(void) {
        if (sdkeys != NULL) {
            free(sdkeys);
            sdkeys = NULL;
        }
        if (sdvals != NULL) {
            for (int i=0; i<sdindex; i++){
                close(sdvals[i]);
            }
            free(sdvals);
            sdvals = NULL;
        }
        sdindex = 0;
}

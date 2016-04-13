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

/// <summary>
/// Retrieves socket for address family <paramref name="addressfamily"> from
/// cache, or creates it if it doesn't already exist.
/// </summary>
/// <param name="addressfamily">
/// The address family of the socket to retrieve
/// </param>
/// <param name="s">
/// The retrieved socket
/// </param>
/// <returns>
/// 0 on success, -1 on error
/// </returns>
/// <example>
/// This example shows how to retrieve a socket from the cache.
/// <code>
/// static void myfunc() {
///    int s;
///    if (libifc_socket(AF_LOCAL, &s) != 0) {
///        // Handle error state here
///    }
///    // user code here
/// }
/// </code>
/// </example> 
int libifc_socket(const int addressfamily, int *s);

/// <summary>
/// Closes sockets and frees resources held by the socket cache.
/// </summary>
void libifc_socketcache_free_resources(void);
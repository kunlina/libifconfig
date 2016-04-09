# libifconfig
A C API to do most things done by the FreeBSD 'ifconfig' program today. The primary goal is to reduce code duplication and empower third-party applications. It is hoped the API will be simple enough that interacting with the network configuration becomes trivial.

## Example usage
The following examples have been written to give some pointers on how to use the API. They have not actually been tested, but they will be soon and the author will update this paragraph when that is done.

```
// Print current interface description, then remove it.
char *desc;
  if (libifconfig_get_description("em0", &desc) == 0)
    printf("old description: %s\n", desc);
  if (libifconfig_unset_description("em0") == 0)
    printf("Successfully unset description.");
  else
    printf("Couldn't unset description. Lazy example writer is lazy,
and instructs reader to imagine using libifconfig_errstate to create useful
error messages.");
```

```
// Set interface MTU
int mtu = 9000;
char *netif = "em0";
if (libifconfig_set_mtu(netif, mtu) == 0) 
  printf("Successfully changed MTU of %s to %d", netif, mtu);
else {
  switch (libifconfig_errstate.errtype)  {
  case SOCKET:
    err(1, "couldn't create socket. This shouldn't happen.\n");
    break;
  case IOCTL:
    if (libifconfig_errstate.ioctl_request == SIOCSIFMTU) 
      err(1, "Failed to set MTU (SIOCSIFMTU)\n");
    else
      err(1, "Failed to set MTU due to error in unexpected ioctl() call %lu. Error code: %i.\n", 
        libifconfig_errstate.ioctl_request, libifconfig_errstate.ioctl_err);
    break;
  default:
    err(1, "Should basically never end up here in this example.\n");
    break;
  }
}
```
## Mailing List Threads
[2016-03-04: libifconfig: A C Api for ifconfig](https://lists.freebsd.org/pipermail/freebsd-net/2016-March/044837.html)
[2016-04-09: libifconfig: Initial code available, looking for feedback](https://lists.freebsd.org/pipermail/freebsd-net/2016-April/045022.html)

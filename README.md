# libifconfig
A C API to do most things done by the FreeBSD 'ifconfig' program today. The primary goal is to reduce code duplication and empower third-party applications. It is hoped the API will be simple enough that interacting with the network configuration becomes trivial.

## Using the state object (libifc_handle_t)
You need to create and dispose of a state object (libifc_handle_t).
It is the applications discretion how to use/track state objects,
but only one thread should use a given state object at the same time.

Suggestion: Use one state object per thread, or use a locking mechanism
to prevent multiple threads from using the same state object at the same time.

```
// Create a state object
libifc_handle_t lifh = libifc_open();

// Do stuff

// Dispose of the state object
libifc_close(lifh);
lifh = NULL;
```

## Example usage
The following examples have been written to give some pointers on how to use the API. They have not actually been tested, but they will be soon and the author will update this paragraph when that is done.

```
// Print current interface description, then remove it.
libifc_handle_t *lifh = libifc_open();
// ...
char *desc;
  if (libifc_get_description(lifh, "em0", &desc) == 0)
    printf("old description: %s\n", desc);
  if (libifc_unset_description(lifh, "em0") == 0)
    printf("Successfully unset description.\n");
  else
    printf("Couldn't unset description. Lazy example writer is lazy,
and instructs reader to imagine using libifc_err_* to create useful
error messages.\n");

libifc_close(lifh);
lifh = NULL;
free(desc);
```

```
// Set interface MTU
libifc_handle_t *lifh = libifc_open();
// ...
int mtu = 9000;
char *netif = "em0";
int retcode = 0;
if (libifc_set_mtu(lifh, netif, mtu) == 0) {
    printf("Successfully changed MTU of %s to %d\n", netif, mtu);
    libifc_close(lifh);
    lifh = NULL;
    free(netif);    
    return 0;
  }
else {
  switch (libifc_err_errtype(lifh))  {
  case SOCKET:
    warnx("couldn't create socket. This shouldn't happen.\n");
    break;
  case IOCTL:
    if (libifc_err_ioctlreq(lifh) == SIOCSIFMTU) 
      warnx("Failed to set MTU (SIOCSIFMTU)\n");
    else
      warnx("Failed to set MTU due to error in unexpected ioctl() call %lu. Error code: %i.\n", 
        libif_err_ioctlreq(lifh), libif_err_errno(lifh));
    break;
  default:
    warnx("Should basically never end up here in this example.\n");
    break;
  }
  libifc_close(lifh);
  lifh = NULL;
  free(netif);
  return -1;
}
```

## Mailing List Threads
[2016-03-04: libifconfig: A C Api for ifconfig](https://lists.freebsd.org/pipermail/freebsd-net/2016-March/044837.html)  
[2016-04-09: libifconfig: Initial code available, looking for feedback](https://lists.freebsd.org/pipermail/freebsd-net/2016-April/045022.html)

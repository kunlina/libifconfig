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
libifc_handle_t *lifh = libifc_open();

// Do stuff

// Dispose of the state object
libifc_close(lifh);
lifh = NULL;
```

## Example usage
Please see the [examples directory](examples/) for some simple programs using libifconfig.

## Mailing List Threads
[2016-03-04: libifconfig: A C Api for ifconfig](https://lists.freebsd.org/pipermail/freebsd-net/2016-March/044837.html)  
[2016-04-09: libifconfig: Initial code available, looking for feedback](https://lists.freebsd.org/pipermail/freebsd-net/2016-April/045022.html)

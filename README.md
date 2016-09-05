# libifc(onfig) 
[![build status](https://gitlab.com/Savagedlight/libifconfig/badges/master/build.svg)](https://gitlab.com/Savagedlight/libifconfig/commits/master)

A C API to do most things done by the FreeBSD 'ifconfig' program today. The primary goal is to reduce code duplication and empower third-party applications. It is hoped the API will be simple enough that interacting with the network configuration becomes trivial.

## FAQ 
__Is it ready for production use?__

Absolutely not. There are currently no releases of this library, and the API may change at any time.

__I want to break stuff. How do I use libifc?__

Please see the [examples directory](examples/) for some simple programs using libifc.

__How can I contribute?__

Work still needs to be done in the following areas:
* Add more features!
* Solve tickets in the issue tracker
* Write unit tests and system integration tests. Preferably using the tools used for tests in FreeBSD base.

Feedback on API design decisions, code quality, etc are very welcome. If your feedback comes with (or in the form of) a pull request, that's great! :)

__How do I use the state object (ifconfig_handle_t)__

You need to create and dispose of a state object (ifconfig_handle_t).
It is the applications discretion how to use/track state objects,
but only one thread should use a given state object at the same time.

Suggestion: Use one state object per thread, or use a locking mechanism
to prevent multiple threads from using the same state object at the same time.

```
// Create a state object
ifconfig_handle_t *lifh = ifconfig_open();

if (lifh == NULL) {
    errx("Failed to open libifconfig handle.");
    return (-1);
}

// Do stuff

// Dispose of the state object
ifconfig_close(lifh);
lifh = NULL;
```

## Mailing List Threads
[2016-03-04: libifconfig: A C Api for ifconfig](https://lists.freebsd.org/pipermail/freebsd-net/2016-March/044837.html)  
[2016-04-09: libifconfig: Initial code available, looking for feedback](https://lists.freebsd.org/pipermail/freebsd-net/2016-April/045022.html)


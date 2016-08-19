Arbitrary notes that may or may not be useful for people working on libifc

## Check for style(9) compliance
In the root directory:
```
make -f Makefile.standalone checkstyle
```


## Enforce code style
In the root directory:
```
make -f Makefile.devel
```


## Generate documentation with Doxygen
In the root directory:
```
mkdir -p stage/doxygen
doxygen tools/Doxyfile
```

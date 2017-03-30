# $FreeBSD$
default:
	$(CC) -Wall -fPIC -lifconfig -g -o example_setdescription setdescription.c
	$(CC) -Wall -fPIC -lifconfig -g -o example_setmtu setmtu.c
	$(CC) -Wall -fPIC -lifconfig -g -o example_ifdestroy ifdestroy.c
	$(CC) -Wall -fPIC -lifconfig -g -o example_ifcreate ifcreate.c
	$(CC) -Wall -fPIC -lifconfig -g -o example_ifcreatevlan ifcreatevlan.c
	$(CC) -Wall -fPIC -lifconfig -g -o example_ifchangevlan ifchangevlan.c
clean:
	rm -f example_*

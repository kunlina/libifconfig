# $FreeBSD$
PROGS=ifchangevlan ifcreate ifcreatevlan ifdestroy setdescription setmtu

CFLAGS+=	-I../stage/libifconfig -L../stage/libifconfig
LDADD=	-lifconfig
MAN=
WARNS?=	6

.include <bsd.progs.mk>

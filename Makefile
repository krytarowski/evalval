#	$NetBSD$

PROG=	evalval

SRCS=	main.c
SRCS+=	my_getline.c
SRCS+=	parser.c
SRCS+=	evaluator.c
SRCS+=	astnode.c

LDADD+=	-lutil
DPADD+=	${LIBUTIL}

#CFLAGS+=	-Werror -Wall

# XXX
NOMAN=

DBG=	-g -O0
#DBG+=	-DDEBUG_MY_GETLINE

CLEANFILES+=	*~

.include <bsd.prog.mk>

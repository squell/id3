# Created by: squell@alumina.nl
# $FreeBSD$

PORTNAME=	id3mtag
PORTVERSION=	0.80
CATEGORIES=	audio
MASTER_SITES=	http://github.com/squell/id3/releases/download/${PORTVERSION}/
DISTNAME=	id3-${PORTVERSION}

MAINTAINER=	squell@alumina.nl
COMMENT=	Mass tagging utility for audio files

LICENSE=	BSD2CLAUSE
LICENSE_FILE=	${WRKSRC}/COPYING

MAKEFILE=	makefile
ALL_TARGET=	build
INSTALL_TARGET=	install installdoc package=id3mtag

PLIST_FILES=	bin/id3 man/man1/id3.1.gz
PORTDOCS=	README CHANGES

OPTIONS_DEFINE=	DOCS BASH
BASH_DESC=	Support for bash auto-completion
BASH_INSTALL_TARGET=	bash_completion
BASH_PLIST_FILES=	share/bash-completion/completions/id3

USES=		iconv

CFLAGS+=	-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib ${ICONV_LIB}

.include <bsd.port.mk>

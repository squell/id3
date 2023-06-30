PORTNAME=	id3mtag
PORTVERSION=	0.81
CATEGORIES=	audio
MASTER_SITES=	https://github.com/squell/id3/releases/download/${PORTVERSION}/
DISTNAME=	id3-${PORTVERSION}

MAINTAINER=	info@squell.net
COMMENT=	Mass tagging utility for audio files
WWW=		https://squell.github.io/id3

LICENSE=	BSD2CLAUSE
LICENSE_FILE=	${WRKSRC}/COPYING

USES=		iconv

MAKEFILE=	makefile
ALL_TARGET=	build

CFLAGS+=	-I${ICONV_INCLUDE_PATH}
LDFLAGS+=	-L${ICONV_PREFIX}/lib ${ICONV_LIB}

PLIST_FILES=	bin/id3 \
		man/man1/id3.1.gz
PORTDOCS=	CHANGES README

OPTIONS_DEFINE=		BASH DOCS
BASH_DESC=		Support for bash auto-completion
BASH_PLIST_FILES=	share/bash-completion/completions/id3

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/id3 ${STAGEDIR}${PREFIX}/bin
	${INSTALL_MAN} ${WRKSRC}/id3.man ${STAGEDIR}${MANPREFIX}/man/man1/id3.1

post-install-BASH-on:
	${MKDIR} ${STAGEDIR}${PREFIX}/share/bash-completion/completions
	${INSTALL_DATA} ${WRKSRC}/bash/complete ${STAGEDIR}${PREFIX}/${BASH_PLIST_FILES}

post-install-DOCS-on:
	${MKDIR} ${STAGEDIR}${DOCSDIR}
.for f in ${PORTDOCS}
	${INSTALL_DATA} ${WRKSRC}/${f} ${STAGEDIR}${DOCSDIR}
.endfor

.include <bsd.port.mk>

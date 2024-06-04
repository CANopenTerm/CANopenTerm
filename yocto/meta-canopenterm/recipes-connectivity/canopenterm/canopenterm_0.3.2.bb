SUMMARY = "CANopenTerm"
DESCRIPTION = "A versatile software tool to analyse and configure \
CANopen devices."
HOMEPAGE = "https://github.com/CANopenTerm/CANopenTerm"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=10e84ea70e8c3a1fbc462f5424806474"

python do_display_banner() {
    bb.plain("CANopenTerm 0.3.2");
    bb.plain("Copyright (c) 2024, Michael Fitzmayer");
}

addtask display_banner before do_build

SRC_URI = "https://github.com/mupfdev/CANopenTerm/archive/refs/tags/v${PV}.tar.gz \
"

S = "${WORKDIR}/CANopenTerm-${PV}"

SRC_URI[md5sum] = "bc00e65b3886794896532dab970c618a"
SRC_URI[sha256sum] = "32183d1d8ac0b50d44b121c37782562bf96850ef1f6275b64129946619c19c94"

do_compile() {
    ${CC} \
    ${CFLAGS} ${LDFLAGS} \
    ${S}/src/can.c \
    ${S}/src/command.c \
    ${S}/src/core.c \
    ${S}/src/gui.c \
    ${S}/src/main.c \
    ${S}/src/menu_bar.c \
    ${S}/src/nmt_client.c \
    ${S}/src/pdo.c \
    ${S}/src/printf.c \
    ${S}/src/scripts.c \
    ${S}/src/sdo_client.c \
    ${S}/src/table.c \
    -DUSE_LIBSOCKETCAN \
    $(pkg-config --cflags --libs --static sdl2 lua) \
    -L${D}${libdir} \
    -o ${S}/export/CANopenTerm
}

do_install () {
    install -d ${D}${bindir}
    install -d ${D}/usr/share/CANopenTerm/scripts
    install -m 0644 ${S}/export/scripts/* ${D}/usr/share/CANopenTerm/scripts
    install -m 0755 ${S}/export/CANopenTerm ${D}${bindir}
}

DEPENDS = "libsdl2 lua nuklear libsocketcan pkgconfig"
FILES:${PN} = "/usr/bin/CANopenTerm /usr/share/CANopenTerm"

inherit pkgconfig

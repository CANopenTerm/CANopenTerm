SUMMARY = "CANopenTerm"
DESCRIPTION = "A versatile software tool to analyse and configure \
CANopen devices."
HOMEPAGE = "https://github.com/mupfdev/CANopenTerm"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=f3cc781d7819d4da0c60b846649a23a7"

python do_display_banner() {
    bb.plain("CANopenTerm 0.2.7");
    bb.plain("Copyright (c) 2022, Michael Fitzmayer");
}

addtask display_banner before do_build

SRC_URI = "https://github.com/mupfdev/CANopenTerm/archive/refs/tags/v${PV}.tar.gz \
"

S = "${WORKDIR}/CANopenTerm-${PV}"

SRC_URI[md5sum] = "3751890cfde0c7ab8b7cd59df886566b"
SRC_URI[sha256sum] = "1acae11c8e223f8825f756077ba0c243dfb19aee56dc98d37d4918749683ff23"

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
    $(pkg-config --cflags --libs --static sdl2 lua) -lpcan \
    -L${D}${libdir} \
    -o ${S}/export/CANopenTerm
}

do_install () {
    install -d ${D}${bindir}
    install -m 0755 ${S}/export/CANopenTerm ${D}${bindir}
}

DEPENDS = "libsdl2 lua nuklear pcan-basic-api pkgconfig"

inherit pkgconfig

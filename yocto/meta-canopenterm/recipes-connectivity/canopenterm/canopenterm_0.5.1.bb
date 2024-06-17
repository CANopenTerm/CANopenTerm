SUMMARY = "CANopenTerm"
DESCRIPTION = "A versatile software tool to analyse and configure \
CANopen devices."
HOMEPAGE = "https://github.com/CANopenTerm/CANopenTerm"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=10e84ea70e8c3a1fbc462f5424806474"

python do_display_banner() {
    bb.plain("CANopenTerm 0.5.0");
    bb.plain("Copyright (c) 2024, Michael Fitzmayer");
}

addtask display_banner before do_build

SRC_URI = "git://github.com/CANopenTerm/CANopenTerm.git;protocol=https;branch=main"
SRCREV  = "f173512ebb7a0bf3b76bd87762179d02ae8fe58c"

S = "${WORKDIR}/git"

do_compile() {
    ${CC} \
    ${CFLAGS} ${LDFLAGS} \
    ${S}/src/can.c \
    ${S}/src/command.c \
    ${S}/src/core.c \
    ${S}/src/dict.c \
    ${S}/src/main.c \
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
    install -d ${D}/usr/local/share/lua/5.4/lua
    install -m 0644 ${S}/export/scripts/* ${D}/usr/share/CANopenTerm/scripts
    install -m 0644 ${S}/export/lua/* ${D}/usr/local/share/lua/5.4/lua
    install -m 0755 ${S}/export/CANopenTerm ${D}${bindir}
}

DEPENDS = "libsdl2 lua libsocketcan pkgconfig"

FILES:${PN} = "/usr/bin/CANopenTerm /usr/share/CANopenTerm /usr/local/share/lua/5.4/lua"

inherit pkgconfig

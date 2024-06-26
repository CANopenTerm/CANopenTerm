SUMMARY = "CANopenTerm"
DESCRIPTION = "A versatile software tool to analyse and configure \
CANopen devices."
HOMEPAGE = "https://github.com/CANopenTerm/CANopenTerm"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=10e84ea70e8c3a1fbc462f5424806474"

python do_display_banner() {
    bb.plain("CANopenTerm 1.0.2");
    bb.plain("Copyright (c) 2024, Michael Fitzmayer");
}

addtask display_banner before do_build

SRC_URI = "git://github.com/CANopenTerm/CANopenTerm.git;protocol=https;branch=main"
SRCREV  = "05009822ebfed6cab5fee250710c26eb69e5786c"

S = "${WORKDIR}/git"

do_compile() {
    ${CC} \
    ${CFLAGS} ${LDFLAGS} \
    ${S}/src/main.c \
    ${S}/src/core/buffer.c \
    ${S}/src/core/can.c \
    ${S}/src/core/can_linux.c \
    ${S}/src/core/command.c \
    ${S}/src/core/core.c \
    ${S}/src/core/dict.c \
    ${S}/src/core/nmt.c \
    ${S}/src/core/pdo.c \
    ${S}/src/core/scripts.c \
    ${S}/src/core/scripts_linux.c \
    ${S}/src/core/sdo.c \
    ${S}/src/core/table.c \
    ${S}/src/os/os.c \
    ${S}/src/os/os_linux.c \
    -DUSE_LIBSOCKETCAN \
    -I${S}/src/core \
    -I${S}/src/os \
    $(pkg-config --cflags --libs --static sdl2 lua readline) \
    -L${D}${libdir} \
    -o ${S}/export/CANopenTerm
}

do_install () {
    install -d ${D}${bindir}
    install -d ${D}/usr/share/CANopenTerm/scripts
    install -d ${D}/usr/share/lua/5.4/lua
    install -m 0644 ${S}/export/scripts/* ${D}/usr/share/CANopenTerm/scripts
    install -m 0644 ${S}/export/lua/* ${D}/usr/share/lua/5.4/lua
    install -m 0755 ${S}/export/CANopenTerm ${D}${bindir}
}

DEPENDS = "libsdl2 lua readline libsocketcan pkgconfig"

FILES:${PN} = "/usr/bin/CANopenTerm /usr/share/CANopenTerm /usr/share/lua/5.4/lua"

inherit pkgconfig

SUMMARY = "CANopenTerm"
DESCRIPTION = "A versatile software tool to analyse and configure \
CANopen devices."
HOMEPAGE = "https://github.com/CANopenTerm/CANopenTerm"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=10e84ea70e8c3a1fbc462f5424806474"

python do_display_banner() {
    bb.plain("CANopenTerm 1.0.5");
    bb.plain("Copyright (c) 2024, Michael Fitzmayer");
}

addtask display_banner before do_build

SRC_URI = "git://github.com/CANopenTerm/CANopenTerm.git;protocol=https;branch=main"
SRCREV  = "bdf39676a23875a2876650780c345b7c06d2979c"

S = "${WORKDIR}/git"

do_compile() {
    ${CC} \
    ${CFLAGS} ${LDFLAGS} \
    ${S}/src/main.c \
    ${S}/src/api/lua_can.c \
    ${S}/src/api/lua_dbc.c \
    ${S}/src/api/lua_misc.c \
    ${S}/src/api/lua_nmt.c \
    ${S}/src/api/lua_pdo.c \
    ${S}/src/api/lua_sdo.c \
    ${S}/src/api/picoc_can.c \
    ${S}/src/api/picoc_dbc.c \
    ${S}/src/api/picoc_misc.c \
    ${S}/src/api/picoc_nmt.c \
    ${S}/src/api/picoc_pdo.c \
    ${S}/src/api/picoc_sdo.c \
    ${S}/src/core/buffer.c \
    ${S}/src/core/can.c \
    ${S}/src/core/can_linux.c \
    ${S}/src/core/command.c \
    ${S}/src/core/core.c \
    ${S}/src/core/dbc.c \
    ${S}/src/core/dict.c \
    ${S}/src/core/nmt.c \
    ${S}/src/core/pdo.c \
    ${S}/src/core/scripts.c \
    ${S}/src/core/scripts_linux.c \
    ${S}/src/core/sdo.c \
    ${S}/src/core/table.c \
    ${S}/src/os/os.c \
    ${S}/src/os/os_linux.c \
    ${S}/src/picoc/clibrary.c \
    ${S}/src/picoc/debug.c \
    ${S}/src/picoc/expression.c \
    ${S}/src/picoc/heap.c \
    ${S}/src/picoc/include.c \
    ${S}/src/picoc/lex.c \
    ${S}/src/picoc/parse.c \
    ${S}/src/picoc/platform.c \
    ${S}/src/picoc/table.c \
    ${S}/src/picoc/type.c \
    ${S}/src/picoc/variable.c \
    ${S}/src/picoc/cstdlib/ctype.c \
    ${S}/src/picoc/cstdlib/errno.c \
    ${S}/src/picoc/cstdlib/math.c \
    ${S}/src/picoc/cstdlib/stdbool.c \
    ${S}/src/picoc/cstdlib/stdio.c \
    ${S}/src/picoc/cstdlib/stdlib.c \
    ${S}/src/picoc/cstdlib/string.c \
    ${S}/src/picoc/cstdlib/time.c \
    ${S}/src/picoc/cstdlib/unistd.c \
    ${S}/src/picoc/platform/library_unix.c \
    ${S}/src/picoc/platform/platform_unix.c \
    -D_CRT_SECURE_NO_WARNINGS \
    -DUSE_LIBSOCKETCAN \
    -I${S}/src/api \
    -I${S}/src/core \
    -I${S}/src/os \
    -I${S}/src/picoc \
    $(pkg-config --cflags --libs --static sdl2 lua readline) \
    -L${D}${libdir} \
    -o ${S}/export/CANopenTerm
}

do_install () {
    install -d ${D}${bindir}
    install -d ${D}/usr/share/CANopenTerm/dbc
    install -d ${D}/usr/share/CANopenTerm/scripts
    install -d ${D}/usr/share/lua/5.4/lua
    install -m 0644 ${S}/export/dbc/* ${D}/usr/share/CANopenTerm/dbc
    install -m 0644 ${S}/export/scripts/* ${D}/usr/share/CANopenTerm/scripts
    install -m 0644 ${S}/export/lua/* ${D}/usr/share/lua/5.4/lua
    install -m 0755 ${S}/export/CANopenTerm ${D}${bindir}
}

DEPENDS = "libsdl2 lua readline libsocketcan pkgconfig"

FILES:${PN} = "/usr/bin/CANopenTerm /usr/share/CANopenTerm /usr/share/lua/5.4/lua"

inherit pkgconfig

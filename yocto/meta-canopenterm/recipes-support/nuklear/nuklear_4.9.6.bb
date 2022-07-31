SUMMARY = "Nuklear"
DESCRIPTION = "A single-header ANSI C immediate mode cross-platform \
GUI library."
HOMEPAGE = "https://github.com/Immediate-Mode-UI/Nuklear"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://src/LICENSE;md5=6052431ae6cd4f0082276c54996e7770"

SRC_URI = "https://github.com/Immediate-Mode-UI/Nuklear/archive/refs/tags/${PV}.tar.gz \
"

S = "${WORKDIR}/Nuklear-${PV}"

SRC_URI[md5sum] = "1c20be6a79062be3ea8102f44dc11555"
SRC_URI[sha256sum] = "087859320af36a0409f7db48a9b15000307bde93bde3d4f6fd8ee6b5df0e2b2b"

do_install () {
    install -d ${D}${includedir}
    install -m 0644 ${S}/nuklear.h ${D}${includedir}
}

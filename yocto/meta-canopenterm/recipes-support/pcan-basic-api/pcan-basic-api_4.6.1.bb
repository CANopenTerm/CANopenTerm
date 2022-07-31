DESCRIPTION = "PCAN-Basic V4.6.1"
HOMEPAGE = "https://www.peak-system.com/PCAN-Basic.239.0.html"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/PCAN-Basic_Linux-${PV}/license.txt;md5=944a23f32fdbca70a79635370f1c2505"

SRC_URI = "https://www.peak-system.com/produktcd/Develop/PC%20interfaces/Linux/PCAN-Basic_API_for_Linux/PCAN-Basic_Linux-${PV}.tar.gz"

S = "${WORKDIR}/PCAN-Basic_Linux-${PV}"

SRC_URI[md5sum] = "5eb7484fb6e4d7dbd62f6a05723b9b47"
SRC_URI[sha256sum] = "fdbc8f9e7f0786ecb08074e097f70fd7d1f0f69ce6e30daf983cb2fb17bee4ad"

do_compile() {
    ${CC} \
    ${CFLAGS} ${LDFLAGS} \
    -DNO_RT \
    ${S}/libpcanbasic/pcanbasic/src/libpcanbasic.c \
    ${S}/libpcanbasic/pcanbasic/src/pcaninfo.c \
    ${S}/libpcanbasic/pcanbasic/src/pcanlog.c \
    ${S}/libpcanbasic/pcanbasic/src/pcbcore.c \
    ${S}/libpcanbasic/pcanbasic/src/pcblog.c \
    ${S}/libpcanbasic/pcanbasic/src/pcbtrace.c \
    ${S}/libpcanbasic/pcanbasic/src/pcan/lib/src/libpcanfd.c \
    -I${S}/libpcanbasic/pcanbasic/src/pcan/driver \
    -I${S}/libpcanbasic/pcanbasic/src/pcan/lib \
    -lm -lpthread \
    -shared \
    -o ${S}/libpcan.so.${PV}
}

do_install () {
    install -d ${D}${includedir}
    install -m 0644 ${S}/libpcanbasic/pcanbasic/PCANBasic.h ${D}${includedir}
    install -m 0644 ${S}/libpcanbasic/pcanbasic/src/pcan/driver/pcan.h ${D}${includedir}
    install -d ${D}${libdir}
    install -m 0644 ${S}/libpcan.so.${PV} ${D}${libdir}
    ln -sr ${D}${libdir}/libpcan.so.${PV} ${D}${libdir}/libpcan.so
}

pcan-basic-api_image_postprocess() {
    ln -sr ${D}${libdir}/libpcan.so.${PV} ${D}${libdir}/libpcan.so
}

ROOTFS_POSTPROCESS_COMMAND += "pcan-basic-api_image_postprocess; "

PROVIDES += " libpcan"
RPROVIDES_${PN} += " libpcan.so"

DESCRIPTION = "Motion - software motion detector and MJPEG streamer"
HOMEPAGE = "https://motion-project.github.io/"
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"
S = "${WORKDIR}/motion-release-4.7.1"

SRC_URI = "https://github.com/Motion-Project/motion/archive/refs/tags/release-4.7.1.tar.gz"
SRC_URI += " file://motion.conf file://motion.service"
SRC_URI[sha256sum] = "c11f68c7cea949227e57797b932f01432d8942ef5323c75063d19b951d0d4194"

inherit autotools pkgconfig gettext

DEPENDS = "ffmpeg jpeg libmicrohttpd"

EXTRA_OECONF = "--without-pgsql --without-mysql --with-ffmpeg --with-webp=no"

do_install:append() {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/motion.conf ${D}${sysconfdir}/motion.conf
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/motion.service ${D}${systemd_system_unitdir}/motion.service
}
SYSTEMD_SERVICE:${PN} = "motion.service"
SYSTEMD_AUTO_ENABLE = "enable"
inherit systemd

FILES:${PN} += "${sysconfdir}/motion.conf"

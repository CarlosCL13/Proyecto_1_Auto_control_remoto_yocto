FILES_SOLIBSDEV = ""
ALLOW_EMPTY:${PN}-dev = "1"
INSANE_SKIP:${PN} += "dev-so"
RDEPENDS:${PN} += "openssl libmicrohttpd libcrypto"
SUMMARY = "Servidor personalizado para Raspberry Pi 4"
DESCRIPTION = "Instala el ejecutable server y su biblioteca, y configura el servicio systemd."
LICENSE = "CLOSED"

SRC_URI += "file://server file://liblibgpio_rpi4.so file://server.service file://auth.js file://control.html file://control.js file://index.html file://styles.css file://users.json file://vehicleApi.js"

S = "${WORKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "server.service"

FILES:${PN} += "/usr/bin/server /auth.js /control.html /control.js /index.html /styles.css /users.json /vehicleApi.js /usr/lib/liblibgpio_rpi4.so"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/server ${D}${bindir}/server
    install -m 0644 ${WORKDIR}/auth.js ${D}/
    install -m 0644 ${WORKDIR}/control.html ${D}/
    install -m 0644 ${WORKDIR}/control.js ${D}/
    install -m 0644 ${WORKDIR}/index.html ${D}/
    install -m 0644 ${WORKDIR}/styles.css ${D}/
    install -m 0644 ${WORKDIR}/users.json ${D}/
    install -m 0644 ${WORKDIR}/vehicleApi.js ${D}/
    install -d ${D}${libdir}
    install -m 0755 ${WORKDIR}/liblibgpio_rpi4.so ${D}${libdir}/liblibgpio_rpi4.so
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/server.service ${D}${systemd_system_unitdir}/server.service
}

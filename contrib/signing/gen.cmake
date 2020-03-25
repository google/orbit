cmake_minimum_required(VERSION 3.13)

# Variables SRCDIR and DESTDIR have to be set by the caller!

execute_process(
  COMMAND "base64"
  INPUT_FILE "$ENV{ORBIT_SIGNING_PUBLIC_KEY_FILE}"
  OUTPUT_VARIABLE PUBLIC_KEY_BASE64)

configure_file("${SRCDIR}/etc/sudoers.d/install_signed_packages.in"
               "${DESTDIR}/etc/sudoers.d/install_signed_package")

configure_file("${SRCDIR}/DEBIAN/control.in" "${DESTDIR}/DEBIAN/control")

configure_file("${SRCDIR}/usr/sbin/install_signed_package.sh.in"
               "${DESTDIR}/usr/sbin/install_signed_package.sh")

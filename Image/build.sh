#!/bin/bash
ROOTFS_DIR=$PWD/rootfs
BOOT_DIR=$PWD/boot
TARGET_SRC=$PWD/../Server/Target
DATE=`date +%Y-%m-%d`
IMAGE=image_${DATE}-Promys.img
LOOPDEV=`losetup -f`

if [ `id -u` != 0 ]; then
    sudo $0
    exit 0
fi
function error {
    echo $1
    exit 1
}

# Get the base image already prepared from pi-gen fork
echo "Cloning base image"
[ -e promys.img ] || error "No base image"
cp promys.img ${IMAGE}

echo "Mounting rootfs and boot"
losetup -Pf ${IMAGE}
mkdir -p rootfs
mkdir -p boot
mount ${LOOPDEV}p1 boot
mount ${LOOPDEV}p2 rootfs

##### WEB PART #########

# Get client binaries from web site
echo "Retrieving client binaries from web site"
wget -O $ROOTFS_DIR/var/www/html/downloads/promys.deb http://promys.me/downloads/promys.deb
wget -O $ROOTFS_DIR/var/www/html/downloads/promys.dmg http://promys.me/downloads/promys.dmg
wget -O $ROOTFS_DIR/var/www/html/downloads/promys.exe http://promys.me/downloads/promys.exe
wget -O $ROOTFS_DIR/var/www/html/downloads/promys.rpm http://promys.me/downloads/promys.rpm

echo "Installing embedded web site"
install -m 644 $TARGET_SRC/www/html/index.html    ${ROOTFS_DIR}/var/www/html
install -m 644 $TARGET_SRC/www/html/settings.html ${ROOTFS_DIR}/var/www/html

install -m 644 $TARGET_SRC/www/html/images/Apple.png   ${ROOTFS_DIR}/var/www/html/images/
install -m 644 $TARGET_SRC/www/html/images/bg.jpg      ${ROOTFS_DIR}/var/www/html/images/
install -m 644 $TARGET_SRC/www/html/images/Tux.png     ${ROOTFS_DIR}/var/www/html/images/
install -m 644 $TARGET_SRC/www/html/images/ubuntu.png  ${ROOTFS_DIR}/var/www/html/images/
install -m 644 $TARGET_SRC/www/html/images/fedora.png  ${ROOTFS_DIR}/var/www/html/images/
install -m 644 $TARGET_SRC/www/html/images/Windows.png ${ROOTFS_DIR}/var/www/html/images/

install -m 644 $TARGET_SRC/www/html/scripts/platform.js ${ROOTFS_DIR}/var/www/html/scripts/platform.js
install -m 644 $TARGET_SRC/www/html/styles/style.css    ${ROOTFS_DIR}/var/www/html/styles/style.css

install -m 644 $TARGET_SRC/www/html/styles/fonts/Gruppo-Regular.ttf ${ROOTFS_DIR}/var/www/html/styles/fonts/Gruppo-Regular.ttf

install -m 755 $TARGET_SRC/www/html/cgi-bin/status.cgi   ${ROOTFS_DIR}/var/www/html/cgi-bin/
install -m 755 $TARGET_SRC/www/html/cgi-bin/apply.cgi    ${ROOTFS_DIR}/var/www/html/cgi-bin/

##### PROMYS BIN PART #########
echo "Installing promys application files"
install -d ${ROOTFS_DIR}/opt/promys
install -m 755 $TARGET_SRC/opt/promys/promys     ${ROOTFS_DIR}/opt/promys/
install -m 644 $TARGET_SRC/opt/promys/lucon.ttf  ${ROOTFS_DIR}/opt/promys/

##### CUSTOM PART ############
echo "Installing configuration files"
install -m 644 $TARGET_SRC/boot/splash.jpg            ${BOOT_DIR}
install -m 644 $TARGET_SRC/boot/config.txt            ${BOOT_DIR}
install -m 644 $TARGET_SRC/boot/cmdline.txt           ${BOOT_DIR}
install -m 644 $TARGET_SRC/boot/info.txt              ${BOOT_DIR}
install -m 644 $TARGET_SRC/boot/wifi.cfg              ${BOOT_DIR}

##### SYSTEM PART ############
echo "Installing system start up script"
install -m 755 $TARGET_SRC/etc/rc.local    ${ROOTFS_DIR}/etc/rc.local
echo "Promys - $DATE" > ${ROOTFS_DIR}/etc/promys-issue

##### PLYMOUTH #######
echo "Installing boot up image"
install -m 644 $TARGET_SRC/plymouth/plymouthd.defaults  ${ROOTFS_DIR}/usr/share/plymouth
install -d ${ROOTFS_DIR}/usr/share/plymouth/themes/promys
install -m 644 $TARGET_SRC/plymouth/themes/promys/promys.plymouth  ${ROOTFS_DIR}/usr/share/plymouth/themes/promys
install -m 644 $TARGET_SRC/plymouth/themes/promys/promys.script    ${ROOTFS_DIR}/usr/share/plymouth/themes/promys
install -m 644 $TARGET_SRC/plymouth/themes/promys/splash.png       ${ROOTFS_DIR}/usr/share/plymouth/themes/promys

##### Wrap it up
echo "Cleaning up"
umount rootfs
umount boot
rmdir rootfs
rmdir boot
losetup -D

echo "Zipping"
zip image_${DATE}-Promys.zip ${IMAGE}
rm ${IMAGE}

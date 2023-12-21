#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
ASSIGNMENTDIR=assignments-3-and-later-SoumyaCode74
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
	#chmod a+w ${OUTDIR}/linux-stable/scripts/dtc/dtc-lexer.l
	diff ${OUTDIR}/linux-stable/scripts/dtc/dtc-lexer.l ${HOME}/${ASSIGNMENTDIR}/working_yylloc/dtc-lexer.l | patch ${OUTDIR}/linux-stable/scripts/dtc/dtc-lexer.l
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    # TODO: Add your kernel build steps here
	make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
	make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
	make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
#	make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules	
	make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
  
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/Image

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir bin dev etc lib lib64 proc sbin sys tmp usr var
mkdir usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
cd ${OUTDIR}/rootfs

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
SRC=${ASSIGNMENTDIR}/libs
export SYSROOT=$SRC
cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 lib
cp -a $SYSROOT/lib64/ld-2.30.so lib64
cp -a $SYSROOT/lib64/libc.so.6 lib64
cp -a $SYSROOT/lib64/libm.so.6 lib64
cp -a $SYSROOT/lib64/libresolv.so.2 lib64
cp -a $SYSROOT/lib64/libc-2.30.so lib64
cp -a $SYSROOT/lib64/libm-2.30.so lib64
cp -a $SYSROOT/lib64/libresolv-2.30.so lib64

# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
cd ${FINDER_APP_DIR}
make clean
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}gcc all

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
if [ ! -d "${OUTDIR}/rootfs/home" ]; then
	mkdir ${OUTDIR}/rootfs/home
fi
if [ ! -d "${OUTDIR}/rootfs/home/conf" ]; then
	mkdir ${OUTDIR}/rootfs/home/conf
fi
cp -r ${FINDER_APP_DIR}/conf/* ${OUTDIR}/rootfs/home/conf
cp -r ${FINDER_APP_DIR}/Makefile ${OUTDIR}/rootfs/home
cp -r ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home
cp -r ${FINDER_APP_DIR}/*writer* ${OUTDIR}/rootfs/home
cp -r ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home/
cp -r ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home/


# TODO: Chown the root directory
cd ${OUTDIR}/rootfs
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
$(find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio)&
wait
cd ..
gzip -f initramfs.cpio

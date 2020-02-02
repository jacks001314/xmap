#!/bin/sh
#
#      Filename: install.sh
#
#        Author: csp001314@163.com
#   Description: ---
#        Create: 2016-11-03 20:09:47
# Last Modified: 2018-10-29 18:56:07
#

[ `id -u` -ne 0 ] && {
	echo "You must run this by root" >&2
	exit 1
}


self_path=$(cd $(dirname "$0"); pwd)

pfring_version=7.4.0
pfring_download=https://github.com/ntop/PF_RING/archive/${pfring_version}.zip
pfring_zip=${pfring_version}.zip
pfring_src=PF_RING-${pfring_version}

xmap_install_prefix=/usr/local/xmap/

if [ ! -d ${xmap_install_prefix} ]; then

   mkdir -p ${xmap_install_prefix}
fi

install_pfring(){

    cd tar
    unzip $pfring_zip
    cd $pfring_src/kernel
    ./configure
    make
    make install

    cd ../../
    cd $pfring_src/userland
    ./configure
    make
    make install
    cd ../../../
}

install_xmap(){

	echo "install xmap----------------------"

    make -C xmap/lib/common
    make -C xmap/lib/thread
    make -C xmap/lib/shm
    make -C xmap/src/probe
    make -C xmap/src/interface
    make -C xmap/src
    cp -rf xmap/src/xmap $xmap_install_prefix/bin
    cp -rf xmap/conf $xmap_install_prefix/
}

prepare_dir(){

	if [ ! -d $xmap_install_prefix/bin ];then
         mkdir -p $xmap_install_prefix/bin
    fi

}

install_all(){

    prepare_dir
	#install_pfring
	install_xmap
}

install_all


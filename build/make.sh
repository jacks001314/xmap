#!/bin/bash
# author shajf
# date 2016-03-03
##get path####

self_path=$(cd $(dirname "$0"); pwd)

pfring_version=7.4.0
pfring_download=https://github.com/ntop/PF_RING/archive/${pfring_version}.zip
pfring_zip=${pfring_version}.zip
pfring_src=PF_RING-${pfring_version}

############# product version #############
if [ -z $1 ]
then
    PRODUCT_VERSION="1.0.0"
else
    PRODUCT_VERSION=$1
fi

product_name=xmap_$PRODUCT_VERSION.bin

########## create dir ########
if [ -f $product_name ]
then
    rm -rf $product_name
fi


if [ ! -d build_dir ]; then
    mkdir build_dir
else
    rm -rf build_dir/*
fi

tar_path=./build_dir/package/tar
src_path=./build_dir/package/xmap

mkdir -p $tar_path
mkdir -p $src_path


#if [ ! -f $tar_path/$pfring_zip ] ; then

 #   wget $pfring_download -p $tar_path
#fi

#################### cp files ##########################
echo "Begin copy files......"
cd $self_path

cp -rf ./makeself ./build_dir/

cp -rf ../make.include $src_path
cp -r ../lib $src_path
cp -r ../src $src_path
cp -r ../conf $src_path

cp -f ./*.sh ./build_dir/

###### build file ##############
echo 'Begin to build installation Package......'
cd $self_path/build_dir
cp -f ./*.sh ./package/
chmod +x ./package/*.sh
chmod +x $self_path/build_dir/makeself/*.sh
./makeself/makeself.sh --gzip package $product_name 'Product Install' ./install.sh

cd $self_path
mv build_dir/$product_name .
rm -rf build_dir
echo "Build finish!!!!!"

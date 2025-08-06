#!/bin/bash

set -Eeuo pipefail

root_folder="$PWD"
dist_dir="$PWD/build/dev-linux-arm"
executable_filename="lunar-rover-nav"
version=$(git describe --always --dirty)
__dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ $# -ge 1 ]]
then
  executable_filename=$1
fi

if [[ $# -ge 2 ]]
then
  dist_dir=$1
fi

#MODE=""
PRODUCT_NAME="sli-lunar-rover"
CONTAINER_VER=$version
IMAGES="$executable_filename"
FILES="sw-description lunar-rover-shellscript.sh $IMAGES"
SWU_PACKAGE=${PRODUCT_NAME}_${CONTAINER_VER}.swu

sh_file_sha256=$(openssl dgst -sha256 < $__dir/lunar-rover-shellscript.sh | awk '{print $2}')
file_sha256=$(openssl dgst -sha256 < $dist_dir/$executable_filename | awk '{print $2}')
cp $dist_dir/$executable_filename $__dir

pushd $__dir
cp sw-description.template sw-description
sed -i "s|/\*VERSION_PLACEHOLDER\*/|$version|g" sw-description
sed -i "s|/\*SH_SHA256_PLACEHOLDER\*/|$sh_file_sha256|g" sw-description
sed -i "s|/\*SHA256_PLACEHOLDER\*/|$file_sha256|g" sw-description

for i in $FILES;do
	echo $i;done | cpio -ov -H crc >  ${SWU_PACKAGE}

# Move package to final destination
mv ${SWU_PACKAGE} $root_folder

#cleanup
rm sw-description
rm $executable_filename

popd

ln -sf $root_folder/${PRODUCT_NAME}_${CONTAINER_VER}.swu $root_folder/${PRODUCT_NAME}.swu

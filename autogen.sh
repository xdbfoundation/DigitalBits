#!/bin/sh -e

root_dir=$PWD

for i in $(cat submodules.txt);
do
    cd $root_dir'/'$i
    echo $root_dir'/'$i
    autogen=$(find . -name autogen.sh)
            if [ -x "$autogen" ]; then
                cd $(dirname "$autogen")
                ./autogen.sh
                cd $root_dir
            fi
done

./make-mks
autoreconf -i

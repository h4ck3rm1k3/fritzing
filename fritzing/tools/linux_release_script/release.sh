#!/bin/bash
svn export http://fritzing.googlecode.com/svn/trunk/fritzing compile_folder

#let's define some variables that we'll need to in the future
arch_aux=`uname -m`
if [ $arch_aux == 'x86_64' ]
	then arch='AMD64'
	else arch='i386'
fi
date=`date +%Y.%m.%d`

tarball_folder="fritzing.$date.source"
cp -rf compile_folder $tarball_folder
echo "making source tarball: $tarball_folder"
tar -cjf ./$tarball_folder.tar.bz2 $tarball_folder
rm -rf $tarball_folder

cd compile_folder
qmake CONFIG+=release -unix
make

release_folder="fritzing.$date.linux.$arch"

echo "making release folder: $release_folder"
mkdir ../$release_folder

echo "copying release files"
cp -rf bins/ parts/ examples/ Fritzing Fritzing.sh README.txt LICENSE.GPL2 LICENSE.GPL3 ../$release_folder/
cd ../$release_folder
chmod +x Fritzing.sh

echo "making library folders"
mkdir lib
mkdir lib/imageformats
mkdir lib/sqldrivers
mkdir lib/translations

cd lib
echo "copying libraries"
cp /usr/lib/libQtCore.so.4 /usr/lib/libQtGui.so.4 /usr/lib/libQtNetwork.so.4 /usr/lib/libQtSql.so.4 /usr/lib/libQtSvg.so.4 /usr/lib/libQtWebKit.so.4 /usr/lib/libQtXml.so.4 .


# if is i368 copy the libaudio
if [ $arch == 'i386' ]
	then 
	    cp /usr/lib/libaudio.so /usr/lib/libaudio.so.2 /usr/lib/libaudio.so.2.4 .
	    echo "copying libaudio files"
	else
	    echo "skipping libaudio files"
fi

echo "copying plugins"
cp /usr/lib/qt4/plugins/imageformats/libqjpeg.so imageformats
cp /usr/lib/qt4/plugins/sqldrivers/libqsqlite.so sqldrivers

echo "copying translations"
cp ../../compile_folder/translations/*.qm translations
cd ../../

echo "compressing...."
tar -cjf ./$release_folder.tar.bz2 $release_folder

echo "cleaning up"
rm -rf $release_folder
rm -rf compile_folder

echo "done!"

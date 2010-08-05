#!/bin/bash
arch_aux=`uname -m`

compile_folder="build-$arch_aux"
svn export http://fritzing.googlecode.com/svn/trunk/fritzing $compile_folder

#let's define some variables that we'll need to in the future
date=`date +%Y.%m.%d`

if [ "$arch_aux" == 'x86_64' ] ; then
	arch='AMD64'
	# only creates the source tarball, when running on the 64 platform
	tarball_folder="fritzing.$date.source"
	cp -rf $compile_folder $tarball_folder
	echo "making source tarball: $tarball_folder"
	tar -cjf ./$tarball_folder.tar.bz2 $tarball_folder
	rm -rf $tarball_folder

	else arch='i386'
fi

cd $compile_folder
QT_HOME="/home/jonathan/qtsdk-2010.04/qt"
#QT_HOME="/usr"


$QT_HOME/bin/qmake CONFIG+=release -unix
make

release_folder="fritzing.$date.linux.$arch"

echo "making release folder: $release_folder"
mkdir ../$release_folder

echo "copying release files"
cp -rf bins/ parts/ sketches/ Fritzing Fritzing.sh README.txt LICENSE.GPL2 LICENSE.GPL3 ../$release_folder/
cd ../$release_folder
chmod +x Fritzing.sh

echo "making library folders"
mkdir lib
mkdir lib/imageformats
mkdir lib/sqldrivers
mkdir translations

cd lib
echo "copying libraries"

cp $QT_HOME/lib/libQtCore.so.4 $QT_HOME/lib/libQtGui.so.4 $QT_HOME/lib/libQtNetwork.so.4 $QT_HOME/lib/libQtSql.so.4 $QT_HOME/lib/libQtSvg.so.4 $QT_HOME/lib/libQtWebKit.so.4 $QT_HOME/lib/libQtXml.so.4 $QT_HOME/lib/libQtXmlPatterns.so.4 $QT_HOME/lib/libQtDBus.so.4 $QT_HOME/lib/libphonon.so.4 .

#  

# jrc 17 july 2010, both platforms seem to need libaudio now
# seems not to be needed anymore
# if is i368 copy the libaudio
#if [ $arch == 'i386' ]
#    then
#       cp /usr/lib/libaudio.so /usr/lib/libaudio.so.2 /usr/lib/libaudio.so.2.4 .
#       echo "copying libaudio files"
#    else
#        echo "skipping libaudio files"
#fi

echo "copying plugins"
cp $QT_HOME/plugins/imageformats/libqjpeg.so imageformats
cp $QT_HOME/plugins/sqldrivers/libqsqlite.so sqldrivers

echo "copying translations"
cp ../../$compile_folder/translations/ -r ../
rm ../translations/*.ts
cd ../../

echo "compressing...."
tar -cjf ./$release_folder.tar.bz2 $release_folder

echo "cleaning up"
rm -rf $release_folder
rm -rf $compile_folder

#echo "done!"


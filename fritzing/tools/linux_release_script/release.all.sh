#!/bin/bash
schroot -c hardy_i386 -u root
cd /home/mariano/release
./release.sh
exit

schroot -c hardy_amd64 -u root
cd /home/mariano/release
./release.sh
exit

cd /home/mariano/workspace/fritzing/release
sudo chowm mariano fritzing*
sudo chgr mariano fritzing*

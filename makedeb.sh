#!/bin/sh
rm -rf em50[_-]*
(tar -C source -czvf em50-0.0.tar.gz . makefile --transform=s,,em50-0.0/, && tar -czvf em50-0.debian.tar.gz debian)
tar -xzvf em50-0.0.tar.gz
cd em50-0.0
tar -xzvf ../em50-0.debian.tar.gz
debmake
debuild

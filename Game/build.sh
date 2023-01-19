#!/bin/sh

cp -f source/LaunchMe CD
3doiso.exe -in CD -out wreckman.iso
3doEncrypt.exe genromtags wreckman.iso
#!/bin/sh

cp -f source/LaunchMe CD
3doiso.exe -in CD -out Intermission.iso
3doEncrypt.exe genromtags Intermission.iso
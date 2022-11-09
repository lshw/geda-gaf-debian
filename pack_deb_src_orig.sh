#!/bin/bash
basename=$( basename $( pwd ) )
cd ..
tar cvfJ $basename.orig.tar.xz $basename

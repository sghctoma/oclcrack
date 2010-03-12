#!/bin/sh

echo "const char * kernelSource =" > MD5_cl.h
cat MD5.cl | while read line; do
  echo "\"$line\n\"" >> MD5_cl.h
done
echo ";" >> MD5_cl.h

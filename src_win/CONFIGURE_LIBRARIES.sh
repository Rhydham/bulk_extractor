#!/bin/sh
# Run this after CONFIGURE_FC*.sh to build and install additional libraries
cat <<EOF
*******************************************************************
Building and installing additional mingw libraries for bulk_extractor.
*******************************************************************
EOF

echo "Building and installing TRE for mingw"
wget http://laurikari.net/tre/tre-0.8.0.zip
unzip tre-0.8.0.zip
cd tre-0.8.0
mingw32-configure
make
sudo make install
mingw64-configure
make
sudo make install
cd ..
rm tre-0.8.0.zip
rm -rf tre-0.8.0
echo "TRE mingw installation complete."

echo "Building and installing LIBEWF for mingw"
wget http://libewf.googlecode.com/files/libewf-experimental-20120809.tar.gz
tar -zxf libewf-experimental-20120809.tar.gz
cd libewf-20120809
mingw32-configure
make
sudo make install
mingw64-configure
make
sudo make install
cd ..
rm libewf-experimental-20120809.tar.gz
rm -rf libewf-20120809
echo "LIBEWF mingw installation complete."


# Introduction #

These instructions where created with Ubuntu 9.10, but other linux distributions should be similar.  The following steps show how to build Minerva with Qt application and gdal plugin.  If you wish to only build the sdk, skip the Qt installation step.


# Details #

Start with installing development prerequisites

```
sudo apt-get install g++ gcc subversion cmake
```

Install required libraries.

```
sudo apt-get install libboost1.40-all-dev
sudo apt-get install libxerces-c-dev
sudo apt-get install uuid-dev
sudo apt-get install libopenscenegraph-dev
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install libqt4-dev
```

gdal must be built from scratch to enable thread safety.  Get gdal from http://trac.osgeo.org/gdal/wiki/DownloadSource.  After expanding the package, run the following commands:

```
./configure --with-threads=-lpthread
make
sudo make install
```

Check out the latest Minerva:

```
svn checkout http://minervagis.googlecode.com/svn/trunk/ minerva
```

To build:

```
mkdir build
cd build
cmake ../
make
sudo make install
```

If you have dependencies in a non-standard location, or if you wish to change the install location, run ccmake instead of cmake.
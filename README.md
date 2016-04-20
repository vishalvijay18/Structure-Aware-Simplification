# Structure Aware Mesh Simplification
This is the repository for CMPT 764 course project.

###Abstract
---
This repository contains implementation of an approach to mesh simplification which uses structure information of object to preserve key features like symmetry and perpendicularity while decimation. We first gather data about symmetrical parts of object mesh, build correspondence among vertices of symmetrical parts, and feed this information to decimation framework. Near planar triangular faces are grouped together under a planar proxy which is then used in multifarious structure preserving rules. An experiment is performed comparing proposed method with that of multiple choice scheme for decimation and results show better performance.

###Steps for setup on Ubuntu (15.10)
---
You will need to install OpenGL and GLUI libraries for running codes in this repository.

- Use following command for installing freeglut package
```
sudo apt-get install freeglut3 freeglut3-dev`
sudo apt-get install binutils-gold
```
- Download packages libglui2c2 and libglui-dev from ubuntu package archive. Links below for 64bit architecture:
[libglui2c2](http://packages.ubuntu.com/precise/amd64/libglui2c2)
[libglui-dev](http://packages.ubuntu.com/precise/libglui-dev)
- Install them:
```
sudo dpkg -i libglui2c2_2.36-4ubuntu1_amd64.deb
sudo dpkg -i libglui-dev_2.36-4ubuntu1_amd64.deb
```


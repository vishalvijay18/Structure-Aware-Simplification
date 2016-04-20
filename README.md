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

###Execution steps
---
Prerequisites : OpenGl, GLUT, GLUI, GLU

- Segmentation of Symmetric parts:
Manually segment symmetric parts of given mesh, assign black (source) and red(target) color to symmetric parts respectively.
Example 1 : Suppose table with pair of symmetric legs, assign black color to first leg vertex and red color to second leg.
Example 2 : Suppose chair with four symmetric legs, assign black and red color to each pair respectively.

- Mapping :
Go to ICP directory inside project and perform below steps
- - 2.1. Run function runner("mesh_file.smf") from matlab code folder.
- - 2.2  It will generate "mesh_file_mapping.smf" in same directory where "mesh_file.smf" is placed.
	Example : 
```
runner('meshs/newbigTable-catmull.obj');
Output file : newbigTable-catmull-mapping.txt (in meshs directory)
```



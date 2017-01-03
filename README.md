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

- Mapping:
Go to ICP directory inside project and perform these steps - 1. Run function runner("mesh_file.smf") from matlab code folder. 2.  It will generate "mesh_file_mapping.smf" in same directory where "mesh_file.smf" is placed.
 Example : 
```
runner('meshs/newbigTable-catmull.obj');
Output file : newbigTable-catmull-mapping.txt (in meshs directory)
```
- Decimation:
Go to StructureAware directory and run below command to compile and run
```
make
./run_project
```
- Steps to test Decimation :
  1. Load a .smf mesh using open button of GUI.
  2. Load a mapping file corresponding to the smf file  uploaded in previous step(genertaed in step 2) from UI. Note: Mapping file is not compulsory. Code will work without it as well, ignoring symmetric decimation.
  3. Assign K = 8 for Multiple choice scheme.
  4. Give no. of collapse iteration to run in text box.
  5. For Decimation, we have two choice.  
   Click on Decimate - It will perform decimation based on normal multiple choice scheme.  
   Click on Shape Preserve - It will perform decimation based on Structure preservation algorithm.  
  6. You can explore other part of GUI, they are self explanatory and as per given assignment.
  7. Also, we provided one button namely 'GO', to perform continous simplification upto certain level after which shape may start distorting.  

###Demo Run
---
```
cd ./StructureAware directory
make
./run_project
```
- load newbigTable-catmull.smf from sampleMesh directory
- load newbigTable-catmull-mapping.txt from current directory (This is pre generated mapping for demo purpose, if you want you can generate as explained in step 2 )
- Specify K and num. of collapse iteration.
- Click on Shape Preserve for decimations.

###References
---
[Course Page](http://www.cs.sfu.ca/~haoz/teaching/cmpt464/)


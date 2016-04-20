This documents explains steps for running this project.

Prerequisites : OpenGl, GLUT, GLUI, GLU

1. Segmentation of Symmetric parts:
Manually segment symmetric parts of given mesh, assign black (source) and red(target) color to symmetric parts respectively.
Example 1 : Suppose table with pair of symmetric legs, assign black color to first leg vertex and red color to second leg.
Example 2 : Suppose chair with four symmetric legs, assign black and red color to each pair respectively.

2. Mapping :
	Go to ICP directory inside project and perform below steps 
	2.1. Run function runner("mesh_file.smf") from matlab code folder.
	2.2  It will generate "mesh_file_mapping.smf" in same directory where "mesh_file.smf" is placed.
	Example : 
		  runner('meshs/newbigTable-catmull.obj');
		  Output file : newbigTable-catmull-mapping.txt (in meshs directory)
	Some sample already added in 

3. Decimation :
Go to StructureAware directory and run below command to compile and run
compile :
 		make

run :
 		./run_project

  Steps to test Decimation :
   3.1. Load a .smf mesh using open button of GUI.
   3.2. Load a mapping file corresponding to the smf file  uploaded in previous step(genertaed in step 2) from UI.
	Note: Mapping file is not compulsory. Code will work without it as well ignoring symmetric decimation.
   3.3. Assign K = 8 for Multiple choice scheme.
   3.4. Give no. of collapse iteration to run in text box.
   3.5. For Decimation, we have two choice
   		a) Click on Decimate - It will perform decimation based on normal multiple choice scheme.
   		b) Click on Shape Preserve - It will perform decimation based on Structure preservation algorithm.
   3.6 You can explore other part of GUI, they are self explanatory and as per given assignment.

Also, we provided one button namely 'GO', to perform continous simplification upto certain level after which shape may start distorting.  

 
 Demo Run :
	Goto  cd ./StructureAware directory
	make
 	 ./run_project
 	 load newbigTable-catmull.smf from sampleMesh directory
 	 load newbigTable-catmull-mapping.txt from current directory (This is pre generated mapping for demo purpose, if you want you can generate as explained in step 2 )
 	 Specify K and num. of collapse iteration.
 	 Click on Shape Preserve for decimations.





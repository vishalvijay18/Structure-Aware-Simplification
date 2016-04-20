#include<GL/glui.h>
#include<GL/glut.h>
#include<stdlib.h>
#include "smfParser.h"
#include "structurePreserve.h"

using namespace std;

#define WIDTH 900
#define HEIGHT 700
#define FLAT_SHADED 1
#define SMOOTH_SHADED 2
#define EDGES_SHADED 3
#define WIREFRAME 4
#define ROTATION_ID 10
#define OPEN_ID 11
#define SAVE_ID 12
#define QUIT_ID 13
#define RESET_ID 14
#define DECIMATE_ID 15
#define DECIMATE_TEXT_ID 16
#define K_TEXT_ID 17
#define SHAPE_PRESERVE_DECIMATE_ID 18
#define MAPPING_OPEN_ID 19
#define SURE_SHAPE_PRESERVE_DECIMATE_ID 20

float obj_pos[] = {0.0, 0.0, 0.0};
float sphere_rotate [16] = {1, 0, 0, 0,	0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

//live variables
int window;
int color=1;
GLfloat x=200, y=200;
int display_type=FLAT_SHADED;
int segments=10;
float scale = 1.0;
float rotationX = 0.0, rotationY = 0.0;
int   lastx, lasty;
char file[1024];
int KValue = 8;           // The k value used in multiple choice scheme
int collapseCount = 1;   // The iteration of decimation

GLUI_Listbox *listbox;

vector<Vertex *> vertexList;
vector<face *> faceList;
map<pair<int, int>, w_edge *> edgeList;
map<Vertex *, int> indexMap; 

//mapping info
map<int, int> mapping;
map<int, int> revMapping;

//proxy info
map<int, glm::mat4x4 *> proxyEq;
map<int, int> proxyCount;

//normal maps
std::map<face *, GLVector*> faceNormalMap;
std::map<Vertex *, GLVector*> vertexNormalMap;

void setupViewport(int w, int h) {

  	/* This sets up the viewport so that the x- and y-scales are proportional */
  	/* to the sides of the window.  If you draw a square it will be displayed */
  	/* as a square, regardless of how the window is resized.  */

  	//glViewport(0, 0, w, h); 
	GLUI_Master.auto_set_viewport();  	
	glMatrixMode(GL_PROJECTION); // sets the matrix mode to projection -- to modify the 'view'
  	glLoadIdentity(); // intialize matrix to identity matrix
	glFrustum( -((float)w/(float)h)*0.08, ((float)w/(float)h)*0.08, -0.08, 0.08, .1, 15.0 );
}

void init() {

	glClearColor(0.7, 0.7, 1.0, 0.0); //background color
  	setupViewport(WIDTH, HEIGHT);
	file[0]='\0';
}


// function to execute a command on linux
string exec(const char* cmd) {
    boost::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "ERROR";
    char buffer[128];
    string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

//handle controls
void controlBlock( int control )
{
	if (control == ROTATION_ID) {
		glutPostRedisplay();
	}
	
	if (control == OPEN_ID) {
		string output;
		output = exec("zenity --file-selection --file-filter='SMF files (smf) | *.smf' --title=\"Select a SMF file\" 2>/dev/null");
		cout <<  output << "\n";
		obj_pos[0]=0.0; obj_pos[1]=0.0; obj_pos[2]=-2.0;
		loadMesh(output);
		glutPostRedisplay();
	}
	
	//will be saved in output.smf
	if (control == SAVE_ID) {
		saveMesh();
		exec("zenity --info --text \"Successfully saved in output.smf\" 2>/dev/null");
	}

	if (control == RESET_ID){	
		scale = 1.0;
		obj_pos[0]=0.0;	obj_pos[1]=0.0; obj_pos[2]=-2.0;
		sphere_rotate[0] = 1; sphere_rotate[1] = 0; sphere_rotate[2] = 0; sphere_rotate[3] = 0; sphere_rotate[4] = 0; 
		sphere_rotate[5] = 1; sphere_rotate[6] = 0; sphere_rotate[7] = 0; sphere_rotate[8] = 0;	sphere_rotate[9] = 0;
		sphere_rotate[10] = 1; sphere_rotate[11] = 0; sphere_rotate[12] = 0; sphere_rotate[13] = 0; sphere_rotate[14] = 0;
		sphere_rotate[15] = 1;
		
		glutPostRedisplay();
	}

	if (control == DECIMATE_ID)
	{
		if(edgeList.size() == 0)
		{
			exec("zenity --info --text \"Please open an smf file.\" 2>/dev/null");
		}
		else
		{
			multipleChoiceDecimation(KValue, collapseCount);
			glutPostRedisplay();
		}
	}

	if (control == SHAPE_PRESERVE_DECIMATE_ID)
	{
		if(edgeList.size() == 0)
		{
			exec("zenity --info --text \"Please open an smf file.\" 2>/dev/null");
		}
		else
		{
			shapePreserveDecimation(KValue, collapseCount);
		}
	}

	if (control == SURE_SHAPE_PRESERVE_DECIMATE_ID)
	{
		if(edgeList.size() == 0)
		{
			exec("zenity --info --text \"Please open an smf file.\" 2>/dev/null");
		}
		else
		{
			guaranteedShapePreserve(KValue);
		}
	}

	if (control == MAPPING_OPEN_ID) {
		string output;
		output = exec("zenity --file-selection --file-filter='Text files | *.txt' --title=\"Select a mapping file\" 2>/dev/null");
		//cout <<  output << "\n";
		obj_pos[0]=0.0; obj_pos[1]=0.0; obj_pos[2]=-2.0;
		loadMappingFile(output);
	}
}

void handleMotion(int x, int y ) {
  rotationX += (float) (y - lasty);
  rotationY += (float) (x - lastx);

  lastx = x;
  lasty = y;

  glutPostRedisplay(); 
}

void handleReshape(int w, int h) {
        setupViewport(w, h);
  	glutSetWindow(window);
	glutPostRedisplay();  
}

void handleIdle(void) {

	if (glutGetWindow() != window)
    		glutSetWindow(window);

	glutPostRedisplay();
}

void handleDisplay() {

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  	glMatrixMode( GL_MODELVIEW );
  	glLoadIdentity();
	glTranslatef(obj_pos[0], obj_pos[1], obj_pos[2]);
	glScalef( scale, scale, scale );
	glMultMatrixf(sphere_rotate);
  	glRotatef( rotationY, 0.0, 1.0, 0.0 );
  	glRotatef( rotationX, 1.0, 0.0, 0.0 );

	glColor3f(1.0, 1.0, 0.0);
	if(display_type==FLAT_SHADED || display_type==EDGES_SHADED)
	{
		glShadeModel(GL_FLAT);
		glEnable(GL_NORMALIZE);
		glBegin(GL_TRIANGLES);
		
		for(face* f : faceList)
		{	
      			face* value = f;
			
			if (f->displayFlag == 1) continue;
			
			w_edge *edge = f->edge;
    			if (edge->left != f) edge = edge->right_next->left_prev;             // Get the three points in the plane
    			Vertex *vertex1 = edge->start;
    			Vertex *vertex2 = edge->end;
    			Vertex *vertex3 = edge->left_next->end;

			if(faceNormalMap.find(f) != faceNormalMap.end())
			{
				GLVector *faceNormal = faceNormalMap[f];
				glNormal3f(faceNormal->x, faceNormal->y, faceNormal->z);
			}
			else
			{	
				cout << "error in FLAT Shading display" << endl; 
			}

			if(edge->left == value)
			{
				glVertex3f(vertex1->x, vertex1->y, vertex1->z);
				glVertex3f(vertex2->x, vertex2->y, vertex2->z);
			}
			else if(edge->right == value)
			{
				glVertex3f(vertex2->x, vertex2->y, vertex2->z);
				glVertex3f(vertex1->x, vertex1->y, vertex1->z);
			}
			
				glVertex3f(vertex3->x, vertex3->y, vertex3->z);	
		}
		glEnd();
	}
	if(display_type==SMOOTH_SHADED)
	{
		w_edge* faceEdgeArray[3];
		glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
		
		for(face* f : faceList)
		{	
      			face* value = f;
			GLVector *normal1, *normal2, *normal3;

			if (f->displayFlag == 1) continue;
	
    			w_edge *edge = f->edge;
    			if (edge->left != f) edge = edge->right_next->left_prev;             // Get the three points in the plane
    			Vertex *vertex1 = edge->start;
    			Vertex *vertex2 = edge->end;
    			Vertex *vertex3 = edge->left_next->end;
			
			normal1 = findVertexNormal(vertex1);		
			normal2 = findVertexNormal(vertex2);
			normal3 = findVertexNormal(vertex3);

			if(edge->left == f)
			{
				if(normal1 != NULL)
				glNormal3f(normal1->x, normal1->y, normal1->z);
				glVertex3f(vertex1->x, vertex1->y, vertex1->z);

				if(normal2 != NULL)
				glNormal3f(normal2->x, normal2->y, normal2->z);
				glVertex3f(vertex2->x, vertex2->y, vertex2->z); 
			}
			else if(edge->right == f)
			{
				if(normal2 != NULL)
				glNormal3f(normal2->x, normal2->y, normal2->z);
				glVertex3f(vertex2->x, vertex2->y, vertex2->z);

				if(normal1 != NULL)
				glNormal3f(normal1->x, normal1->y, normal1->z);
				glVertex3f(vertex1->x, vertex1->y, vertex1->z);
			}
				
				
				if(normal3 != NULL)
				glNormal3f(normal3->x, normal3->y, normal3->z);
				glVertex3f(vertex3->x, vertex3->y, vertex3->z);
		}
		glEnd();
	}
	if(display_type==EDGES_SHADED)
	{
		glBegin(GL_LINES);
		for(auto edg : edgeList){
			w_edge *value = edg.second;

			glVertex3f(value->start->x, value->start->y, value->start->z);
			glVertex3f(value->end->x, value->end->y, value->end->z);
    		}
    		glEnd();
	}
	if(display_type==WIREFRAME)
	{
		glBegin(GL_LINES);
		for(auto edg : edgeList){
			w_edge *value = edg.second;

			glVertex3f(value->start->x, value->start->y, value->start->z);
			glVertex3f(value->end->x, value->end->y, value->end->z);
    		}
    		glEnd();
	}
	
	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); //double buffering, RGB mode, need depth buffer
	glutInitWindowSize(WIDTH,HEIGHT); //width, height of the window
	glutInitWindowPosition(0, 0);	// location on the screen the window appears
	
	window = glutCreateWindow("Project GUI");	// creates the window
	
	init(); //things to do once, mostly in beginning	
	
	glutDisplayFunc(handleDisplay);		// tells glut which function to call to render a screen.
	glutMotionFunc(handleMotion);		// handle when motion (this generally means when mouse is moved with a button pressed)
	
 	GLUI_Master.set_glutReshapeFunc( handleReshape );

  	//Setting opengl lights

  	GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
  	GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
  	GLfloat light0_position[] = {1.0f, 1.0f, 1.0f, 0.0f};

  	glEnable(GL_LIGHTING);
  	glEnable(GL_LIGHT0);
  	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	
	glEnable(GL_DEPTH_TEST);		// enabling depth
	
	GLUI *glui = GLUI_Master.create_glui_subwindow(window,
                     GLUI_SUBWINDOW_RIGHT);
  	new GLUI_StaticText( glui, "Project GLUI" );
  	new GLUI_Separator(glui);
	
	glui->set_main_gfx_window( window );
	GLUI_Panel *obj_panel = glui->add_panel ("Display option panel");
  	
	listbox = new GLUI_Listbox(obj_panel, "Mesh display option", &display_type, 1, controlBlock);
  	listbox->add_item(FLAT_SHADED, "Flat Shaded");
	listbox->add_item(SMOOTH_SHADED, "Smooth Shaded");
	listbox->add_item(WIREFRAME, "Wireframe");
  	listbox->add_item(EDGES_SHADED, "Shaded with mesh edges");

	//scale code
	GLUI_Spinner *scale_spinner = new GLUI_Spinner( obj_panel, "Scale:", &scale);
  	scale_spinner->set_float_limits( .2f, 5.0 );
  	scale_spinner->set_alignment( GLUI_ALIGN_RIGHT );
	
	//rotation code
	GLUI_Panel * rotation_panel = glui->add_panel_to_panel(obj_panel, "", GLUI_PANEL_NONE);
	
	GLUI_Rotation *sph_rot =
		glui->add_rotation_to_panel(rotation_panel,"Rotate", sphere_rotate,
		ROTATION_ID, controlBlock); 
	sph_rot->set_spin(1.0);

	//translation code
	GLUI_Panel * translate_panel = glui->add_panel_to_panel(obj_panel, "",
		GLUI_PANEL_NONE);	

	GLUI_Translation * move_z =
		glui->add_translation_to_panel(translate_panel,"Object Z",
		GLUI_TRANSLATION_Z, &obj_pos[2]); 
	move_z->scale_factor = 0.1f;

	glui->add_column_to_panel(translate_panel, true);
	
	GLUI_Translation * move_around =
		glui->add_translation_to_panel(translate_panel,"Object XY",
		GLUI_TRANSLATION_XY, obj_pos);
	move_around->scale_factor = 0.1f;

	GLUI_Panel * decimate_panel = glui->add_panel("Decimate Panel");

	glui->add_edittext_to_panel(decimate_panel, "No. of collapse iteration: ", GLUI_EDITTEXT_INT, &collapseCount);

	glui->add_edittext_to_panel(decimate_panel, "Random count (k): ", GLUI_EDITTEXT_INT, &KValue);

	glui->add_button_to_panel(decimate_panel, "Decimate", DECIMATE_ID, controlBlock);

	glui->add_button_to_panel(decimate_panel, "Shape Preserve", SHAPE_PRESERVE_DECIMATE_ID, controlBlock);

	glui->add_separator();

	GLUI_Panel * shape_panel = glui->add_panel("Shape based Panel");
	
	glui->add_button_to_panel(shape_panel, "Go", SURE_SHAPE_PRESERVE_DECIMATE_ID, controlBlock);

	glui->add_separator();

	GLUI_Panel * mapping_panel = glui->add_panel("Mapping Panel");

	glui->add_button_to_panel(mapping_panel, "Open map file", MAPPING_OPEN_ID, controlBlock);
	//other details
	glui->add_separator();
	glui->add_button("Open", OPEN_ID, controlBlock);
	glui->add_button("Save", SAVE_ID, controlBlock);
	glui->add_button("Reset", RESET_ID, controlBlock);
  	glui->add_button("Quit", QUIT_ID, (GLUI_Update_CB)exit);

	/* register idle callback with GLUI, NOT with GLUT */
  	GLUI_Master.set_glutIdleFunc( handleIdle );	// handle when no events are occuring 

	glutMainLoop();				// once this is called, glut takes over --
						// returns only when the window is closed	
	
	return EXIT_SUCCESS;
}

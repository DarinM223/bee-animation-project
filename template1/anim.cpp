////////////////////////////////////////////////////
// anim.cpp version 4.1
// Template code for drawing an articulated figure.
// CS 174A 
////////////////////////////////////////////////////
#include <GL/glew.h>
#ifdef _WIN32
#include <windows.h>
#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifdef _WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

#include "Ball.h"
#include "FrameSaver.h"
#include "Timer.h"
#include "Shapes.h"
#include "tga.h"

#include "Angel/Angel.h"

#ifdef __APPLE__
#define glutInitContextVersion(a,b)
#define glutInitContextProfile(a)
#define glewExperimental int glewExperimentalAPPLE
#define glewInit()
#endif

FrameSaver FrSaver ;
Timer TM ;

BallData *Arcball = NULL ;
int Width = 800;
int Height = 800 ;
int Button = -1 ;
float Zoom = 1 ;
int PrevY = 0 ;

int Animate = 0 ;
int Recording = 0 ;

void resetArcball() ;
void save_image();
void instructions();
void set_colour(float r, float g, float b) ;

const int STRLEN = 100;
typedef char STR[STRLEN];

#define PI 3.1415926535897
#define X 0
#define Y 1
#define Z 2

//texture

GLuint texture_cube;
GLuint texture_earth;

// Structs that hold the Vertex Array Object index and number of vertices of each shape.
ShapeData cubeData;
ShapeData sphereData;
ShapeData coneData;
ShapeData cylData;

// Matrix stack that can be used to push and pop the modelview matrix.
class MatrixStack {
    int    _index;
    int    _size;
    mat4*  _matrices;

   public:
    MatrixStack( int numMatrices = 32 ):_index(0), _size(numMatrices)
        { _matrices = new mat4[numMatrices]; }

    ~MatrixStack()
	{ delete[]_matrices; }

    void push( const mat4& m ) {
        assert( _index + 1 < _size );
        _matrices[_index++] = m;
    }

    mat4& pop( void ) {
        assert( _index - 1 >= 0 );
        _index--;
        return _matrices[_index];
    }
};

MatrixStack  mvstack;
mat4         model_view;
GLint        uModelView, uProjection, uView;
GLint        uAmbient, uDiffuse, uSpecular, uLightPos, uShininess;
GLint        uTex, uEnableTex;

// The eye point and look-at point.
// Currently unused. Use to control a camera with LookAt().
Angel::vec4 eye(0, 0.0, 50.0,1.0);
Angel::vec4 ref(0.0, 0.0, 0.0,1.0);
Angel::vec4 up(0.0,1.0,0.0,0.0);

double TIME = 0.0 ;

/////////////////////////////////////////////////////
//    PROC: drawCylinder()
//    DOES: this function 
//          render a solid cylinder  oriented along the Z axis. Both bases are of radius 1. 
//          The bases of the cylinder are placed at Z = 0, and at Z = 1.
//
//          
// Don't change.
//////////////////////////////////////////////////////
void drawCylinder(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
}

//////////////////////////////////////////////////////
//    PROC: drawCone()
//    DOES: this function 
//          render a solid cone oriented along the Z axis with base radius 1. 
//          The base of the cone is placed at Z = 0, and the top at Z = 1. 
//         
// Don't change.
//////////////////////////////////////////////////////
void drawCone(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawCube()
//    DOES: this function draws a cube with dimensions 1,1,1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////

void drawCube(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawSphere()
//    DOES: this function draws a sphere with radius 1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////

void drawSphere(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_earth);
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );
}


void resetArcball()
{
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}


//////////////////////////////////////////////////////
//    PROC: myKey()
//    DOES: this function gets caled for any keypresses
// 
//////////////////////////////////////////////////////

void myKey(unsigned char key, int x, int y)
{
    float time ;
    switch (key) {
        case 'q':
        case 27:
            exit(0); 
        case 's':
            FrSaver.DumpPPM(Width,Height) ;
            break;
        case 'r':
            resetArcball() ;
            glutPostRedisplay() ;
            break ;
        case 'a': // togle animation
            Animate = 1 - Animate ;
            // reset the timer to point to the current time		
            time = TM.GetElapsedTime() ;
            TM.Reset() ;
            // printf("Elapsed time %f\n", time) ;
            break ;
        case '0':
            //reset your object
            break ;
        case 'm':
            if( Recording == 1 )
            {
                printf("Frame recording disabled.\n") ;
                Recording = 0 ;
            }
            else
            {
                printf("Frame recording enabled.\n") ;
                Recording = 1  ;
            }
            FrSaver.Toggle(Width);
            break ;
        case 'h':
        case '?':
            instructions();
            break;
    }
    glutPostRedisplay() ;

}

/*********************************************************
    PROC: myinit()
    DOES: performs most of the OpenGL intialization
     -- change these with care, if you must.

**********************************************************/

void myinit(void)
{
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram(program);

    // Generate vertex arrays for geometric shapes
    generateCube(program, &cubeData);
    generateSphere(program, &sphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);

    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView       = glGetUniformLocation( program, "View"       );

    glClearColor( 0.1, 0.1, 0.2, 1.0 ); // dark blue background

    uAmbient   = glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse   = glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular  = glGetUniformLocation( program, "SpecularProduct" );
    uLightPos  = glGetUniformLocation( program, "LightPosition"   );
    uShininess = glGetUniformLocation( program, "Shininess"       );
    uTex       = glGetUniformLocation( program, "Tex"             );
    uEnableTex = glGetUniformLocation( program, "EnableTex"       );

    glUniform4f(uAmbient,    0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uDiffuse,    0.6f,  0.6f,  0.6f, 1.0f);
    glUniform4f(uSpecular,   0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uLightPos,  15.0f, 15.0f, 30.0f, 0.0f);
    glUniform1f(uShininess, 100.0f);

    glEnable(GL_DEPTH_TEST);
 
    Arcball = new BallData;
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}

/*********************************************************
    PROC: set_colour();
    DOES: sets all material properties to the given colour
    -- don't change
**********************************************************/

void set_colour(float r, float g, float b)
{
    float ambient  = 0.2f;
    float diffuse  = 0.6f;
    float specular = 0.2f;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1.0f);
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1.0f);
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1.0f);
}
void drawGround();
void drawBee();
void drawBody();
void drawHead();
void drawTail();
void drawLeg(int side);
void drawLegz();
void drawWings();
void drawFlowerHead();
void drawFlowerStem();
void drawFlower();

/*********************************************************
**********************************************************
**********************************************************

    PROC: display()
    DOES: this gets called by the event handler to draw
          the scene, so this is where you need to build
          your ROBOT --  
      
        MAKE YOUR CHANGES AND ADDITIONS HERE

    Add other procedures if you like.

**********************************************************
**********************************************************
**********************************************************/
void display(void)
{
    // Clear the screen with the background colour (set in myinit)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model_view = mat4(1.0f);
    
    model_view *= Translate(0.0f, -5.0f, -15.0f);
    HMatrix r;
    Ball_Value(Arcball,r);

    mat4 mat_arcball_rot(
        r[0][0], r[0][1], r[0][2], r[0][3],
        r[1][0], r[1][1], r[1][2], r[1][3],
        r[2][0], r[2][1], r[2][2], r[2][3],
        r[3][0], r[3][1], r[3][2], r[3][3]);
    model_view *= mat_arcball_rot;
        
    glUniformMatrix4fv( uView, 1, GL_TRUE, model_view );


    // Previously glScalef(Zoom, Zoom, Zoom);
    model_view *= Scale(Zoom);

    //draw objects
    drawGround();
    drawBee();
    drawFlower();

    glutSwapBuffers();
    if(Recording == 1)
        FrSaver.DumpPPM(Width, Height);
}

/**********************************************
    PROC: myReshape()
    DOES: handles the window being resized 
    
      -- don't change
**********************************************************/
void myReshape(int w, int h)
{
    Width = w;
    Height = h;

    glViewport(0, 0, w, h);

    mat4 projection = Perspective(50.0f, (float)w/(float)h, 1.0f, 1000.0f);
    glUniformMatrix4fv( uProjection, 1, GL_TRUE, projection );
}

void instructions() 
{
    printf("Press:\n");
    printf("  s to save the image\n");
    printf("  r to restore the original view.\n") ;
    printf("  0 to set it to the zero state.\n") ;
    printf("  a to toggle the animation.\n") ;
    printf("  m to toggle frame dumping.\n") ;
    printf("  q to quit.\n");
}

// start or end interaction
void myMouseCB(int button, int state, int x, int y)
{
    Button = button ;
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width-1.0;
        arcball_coords.y = -2.0*(float)y/(float)Height+1.0;
        Ball_Mouse(Arcball, arcball_coords) ;
        Ball_Update(Arcball);
        Ball_BeginDrag(Arcball);

    }
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
        Ball_EndDrag(Arcball);
        Button = -1 ;
    }
    if( Button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        PrevY = y ;
    }


    // Tell the system to redraw the window
    glutPostRedisplay() ;
}

// interaction (mouse motion)
void myMotionCB(int x, int y)
{
    if( Button == GLUT_LEFT_BUTTON )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width - 1.0 ;
        arcball_coords.y = -2.0*(float)y/(float)Height + 1.0 ;
        Ball_Mouse(Arcball,arcball_coords);
        Ball_Update(Arcball);
        glutPostRedisplay() ;
    }
    else if( Button == GLUT_RIGHT_BUTTON )
    {
        if( y - PrevY > 0 )
            Zoom  = Zoom * 1.03 ;
        else 
            Zoom  = Zoom * 0.97 ;
        PrevY = y ;
        glutPostRedisplay() ;
    }
}


void idleCB(void)
{
    if( Animate == 1 )
    {
        // TM.Reset() ; // commenting out this will make the time run from 0
        // leaving 'Time' counts the time interval between successive calls to idleCB
        if( Recording == 0 )
            TIME = TM.GetElapsedTime() ;
        else
            TIME += 0.033 ; // save at 30 frames per second.
        
        eye.x = 20*sin(TIME);
        eye.z = 20*cos(TIME);
        
        printf("TIME %f\n", TIME) ;
        glutPostRedisplay() ; 
    }
}
/*********************************************************
     PROC: main()
     DOES: calls initialization, then hands over control
           to the event handler, which calls 
           display() whenever the screen needs to be redrawn
**********************************************************/

int main(int argc, char** argv) 
{
    glutInit(&argc, argv);
    // If your code fails to run, uncommenting these lines may help.
    //glutInitContextVersion(3, 2);
    //glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Width,Height);
    glutCreateWindow(argv[0]);
    printf("GL version %s\n", glGetString(GL_VERSION));
    glewExperimental = GL_TRUE;
    glewInit();
    
    myinit();

    glutIdleFunc(idleCB) ;
    glutReshapeFunc (myReshape);
    glutKeyboardFunc( myKey );
    glutMouseFunc(myMouseCB) ;
    glutMotionFunc(myMotionCB) ;
    instructions();

    glutDisplayFunc(display);
    glutMainLoop();

    TM.Reset() ;
    return 0;         // never reached
}
void drawGround() {
    //draw ground
    mvstack.push(model_view);
    set_colour(0.0, 0.5, 0.0);
    model_view *= Translate(0, 0, 0);
    model_view *= Scale(40, 1, 40);
    drawCube();
    model_view = mvstack.pop();
}

void drawBody() {
    mvstack.push(model_view);
    //draw body
    model_view *= Scale(2, 1, 1);
    //set color to grey
    set_colour(.658, .658, .658);
    drawCube();
    model_view = mvstack.pop();
}
void drawHead() {
    mvstack.push(model_view);
    //draw head
    model_view *= Translate(-1.5, 0, 0);
    model_view *= Scale(.5, .5, .5);
    //set color to blue
    set_colour(0.0, 0.0, 1.0);
    drawSphere();
    model_view = mvstack.pop();
}
void drawTail() {
    mvstack.push(model_view);
    //draw tail
    model_view *= Translate(2.5, 0, 0);
    model_view *= Scale(1.5, .75, .75);
    //set color to yellow
    set_colour(1.0, 1.0, 0.0);
    drawSphere();
    model_view = mvstack.pop();
}
void drawBee() {
    mvstack.push(model_view);
    //rotate bee
    model_view *= RotateY(-10*TIME);
    model_view *= Translate(0, 5, 0);
    model_view *= Translate(5, .5*sin(100+TIME), 0);
    model_view *= RotateY(90);

    //draw bee parts
    drawBody();    
    drawHead();
    drawTail();
    drawWings();
    drawLegz();

    model_view = mvstack.pop();
}
void drawWings() {
        mvstack.push(model_view);
        set_colour(.658, .658, .658);
                mvstack.push(model_view); //draw first wing
                        model_view *= Translate(0, .5, -.5);
                        model_view *= RotateX(50*sin(TIME));
                        model_view *= Translate(0, 0.025, -1);
                        model_view *= Scale(1.0, 0.05, 2);
                        drawCube();
                model_view = mvstack.pop();

                mvstack.push(model_view); //draw second wing
                        model_view *= Translate(0, .5, .5);
                        model_view *= RotateX(-50*sin(TIME));
                        model_view *= Translate(0, 0.025, 1);
                        model_view *= Scale(1.0, 0.05, 2);
                        drawCube();
                model_view = mvstack.pop();
        model_view = mvstack.pop();
}
void drawLeg(int side) {
        if (side == 0) { //if left leg
                mvstack.push(model_view); //draw thigh
                        model_view *= Translate(0, -.5, -.5);
                        model_view *= RotateX(-abs(20 * sin(.5 * TIME)));
                        mvstack.push(model_view);
                                model_view *= Translate(0, -.3, -.075);
                                model_view *= Scale(.15, .6, .15);
                                drawCube();
                        model_view = mvstack.pop();

                        model_view *= Translate(0, -.6, 0); //move down to draw foot
                        mvstack.push(model_view); //draw foot
                                model_view *= RotateX(-abs(20 * sin(.5 * TIME)));
                                model_view *= Translate(0, -.3, -.075);
                                model_view *= Scale(.15, .6, .15);
                                drawCube();
                        model_view = mvstack.pop();
                model_view = mvstack.pop();
        } else if (side == 1) { //if right leg
                mvstack.push(model_view); //draw thigh
                        model_view *= Translate(0, -.5, .5);
                        model_view *= RotateX(abs(20 * sin(.5 * TIME)));
                        mvstack.push(model_view);
                                model_view *= Translate(0, -.3, .075);
                                model_view *= Scale(.15, .6, .15);
                                drawCube();
                        model_view = mvstack.pop();

                        model_view *= Translate(0, -.6, 0); //move down to draw foot
                        mvstack.push(model_view); //draw foot
                                model_view *= RotateX(abs(20 * sin(.5 * TIME)));
                                model_view *= Translate(0, -.3, .075);
                                model_view *= Scale(.15, .6, .15);
                                drawCube();
                        model_view = mvstack.pop();
                model_view = mvstack.pop();
        }
}
void drawLegz() {
    //draw left legs
    mvstack.push(model_view);
        drawLeg(0);
        model_view *= Translate(-.5, 0, 0);
        drawLeg(0);
        model_view *= Translate(1, 0, 0);
        drawLeg(0);
    model_view = mvstack.pop();

    //draw right legs
    mvstack.push(model_view);
        drawLeg(1);
        model_view *= Translate(-.5, 0, 0);
        drawLeg(1);
        model_view *= Translate(1, 0, 0);
        drawLeg(1);
    model_view = mvstack.pop();
}
void drawFlowerHead() {
        mvstack.push(model_view);
        set_colour(1.0, 0.0, 0.0);
        model_view *= RotateZ(12*sin(TIME));
        model_view *= Translate(0, 9.9, 0);
        model_view *= Scale(1.5, 1.5, 1.5);
        drawSphere();
        model_view = mvstack.pop();
}
void drawFlowerStem() {
        mvstack.push(model_view);
                set_colour(0.5, 0.35, 0.05);
                //draw pieces of the stem
                //#1
                mvstack.push(model_view);
                        model_view *= RotateZ(1.25*sin(TIME));
                        model_view *= Translate(0, 1.0, 0);
                        model_view *= RotateZ(1.25*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
                //#2
                mvstack.push(model_view);
                        model_view *= RotateZ(2.5*sin(TIME));
                        model_view *= Translate(0, 2, 0);
                        model_view *= RotateZ(2.5*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
                //#3
                mvstack.push(model_view);
                        model_view *= RotateZ(3.75*sin(TIME));
                        model_view *= Translate(0, 3, 0);
                        model_view *= RotateZ(3.75*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
                //#4
                mvstack.push(model_view);
                        model_view *= RotateZ(5*sin(TIME));
                        model_view *= Translate(0, 4, 0);
                        model_view *= RotateZ(5*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
                //#5
                mvstack.push(model_view);
                        model_view *= RotateZ(6.25*sin(TIME));
                        model_view *= Translate(0, 5, 0);
                        model_view *= RotateZ(6.25*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
                //#6
                mvstack.push(model_view);
                        model_view *= RotateZ(7.5*sin(TIME));
                        model_view *= Translate(0, 6, 0);
                        model_view *= RotateZ(7.5*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
                //#7
                mvstack.push(model_view);
                        model_view *= RotateZ(8.75*sin(TIME));
                        model_view *= Translate(0, 7, 0);
                        model_view *= RotateZ(8.75*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
                //#8
                mvstack.push(model_view);
                        model_view *= RotateZ(10*sin(TIME));
                        model_view *= Translate(0, 8, 0);
                        model_view *= RotateZ(10*sin(TIME));
                        model_view *= Scale(.15, 1, .15);
                        drawCube();
                model_view = mvstack.pop();
        model_view = mvstack.pop();
}

void drawFlower() {
        //draw the flower parts
        mvstack.push(model_view);
        drawFlowerHead();
        drawFlowerStem();
        model_view = mvstack.pop();
}

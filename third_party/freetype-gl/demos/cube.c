/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <string.h>

#include "shader.h"
#include "mat4.h"
#include "vertex-buffer.h"
#include "screenshot-util.h"

#include <GLFW/glfw3.h>


// ------------------------------------------------------- global variables ---
GLuint shader;
vertex_buffer_t * cube;
mat4  model, view, projection;


// ------------------------------------------------------------------- init ---
void init( void )
{
    typedef struct { float x,y,z;} xyz;
    typedef struct { float r,g,b,a;} rgba;
    typedef struct { xyz position, normal; rgba color;} vertex;
    xyz v[] = { { 1, 1, 1},  {-1, 1, 1},  {-1,-1, 1}, { 1,-1, 1},
                { 1,-1,-1},  { 1, 1,-1},  {-1, 1,-1}, {-1,-1,-1} };
    xyz n[] = { { 0, 0, 1},  { 1, 0, 0},  { 0, 1, 0} ,
                {-1, 0, 1},  { 0,-1, 0},  { 0, 0,-1} };
    rgba c[] = { {1, 1, 1, 1},  {1, 1, 0, 1},  {1, 0, 1, 1},  {0, 1, 1, 1},
                 {1, 0, 0, 1},  {0, 0, 1, 1},  {0, 1, 0, 1},  {0, 0, 0, 1} };
    vertex vertices[24] =  {
      {v[0],n[0],c[0]}, {v[1],n[0],c[1]}, {v[2],n[0],c[2]}, {v[3],n[0],c[3]},
      {v[0],n[1],c[0]}, {v[3],n[1],c[3]}, {v[4],n[1],c[4]}, {v[5],n[1],c[5]},
      {v[0],n[2],c[0]}, {v[5],n[2],c[5]}, {v[6],n[2],c[6]}, {v[1],n[2],c[1]},
      {v[1],n[3],c[1]}, {v[6],n[3],c[6]}, {v[7],n[3],c[7]}, {v[2],n[3],c[2]},
      {v[7],n[4],c[7]}, {v[4],n[4],c[4]}, {v[3],n[4],c[3]}, {v[2],n[4],c[2]},
      {v[4],n[5],c[4]}, {v[7],n[5],c[7]}, {v[6],n[5],c[6]}, {v[5],n[5],c[5]} };
    GLuint indices[24] = { 0, 1, 2, 3,    4, 5, 6, 7,   8, 9,10,11,
                           12,13,14,15,  16,17,18,19,  20,21,22,23 };

    cube = vertex_buffer_new( "vertex:3f,normal:3f,color:4f" );
    vertex_buffer_push_back( cube, vertices, 24, indices, 24 );
    shader = shader_load("shaders/cube.vert","shaders/cube.frag");

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );

    glPolygonOffset( 1, 1 );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_COLOR_MATERIAL );
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_LINE_SMOOTH );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    static float theta=0, phi=0;
    static GLuint Color = 0;
    double seconds_elapsed = glfwGetTime( );

    if( !Color )
    {
        Color = glGetUniformLocation( shader, "Color" );
    }

    theta = .5f * seconds_elapsed / 0.016f;
    phi = .5f * seconds_elapsed / 0.016f;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    mat4_set_identity( &model );
    mat4_rotate( &model, theta, 0, 0, 1 );
    mat4_rotate( &model, phi, 0, 1, 0 );
    mat4_translate( &model, 0.0, 0.0, -5.0 );

    glDisable( GL_BLEND );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_POLYGON_OFFSET_FILL );

    glUseProgram( shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ),
                            1, 0, projection.data);
    }

    glUniform4f( Color, 1, 1, 1, 1 );
    vertex_buffer_render( cube, GL_QUADS );

    glDisable( GL_POLYGON_OFFSET_FILL );
    glEnable( GL_BLEND );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDepthMask( GL_FALSE );

    glUniform4f( Color, 0, 0, 0, .5 );
    vertex_buffer_render( cube, GL_QUADS );

    glUseProgram( 0 );
    glDepthMask( GL_TRUE );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glfwSwapBuffers( window );
}


// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    glViewport(0, 0, width, height);
    mat4_set_perspective( &projection, 45.0f, width/(float) height, 2.0, 10.0 );
}


// --------------------------------------------------------------- keyboard ---
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
}


// --------------------------------------------------------- error-callback ---
void error_callback( int error, const char* description )
{
    fputs( description, stderr );
}


// ------------------------------------------------------------------- main ---
int main( int argc, char **argv )
{
    GLFWwindow* window;
    char* screenshot_path = NULL;

    if (argc > 1)
    {
        if (argc == 3 && 0 == strcmp( "--screenshot", argv[1] ))
            screenshot_path = argv[2];
        else
        {
            fprintf( stderr, "Unknown or incomplete parameters given\n" );
            exit( EXIT_FAILURE );
        }
    }

    glfwSetErrorCallback( error_callback );

    if (!glfwInit( ))
    {
        exit( EXIT_FAILURE );
    }

    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    window = glfwCreateWindow( 400, 400, argv[0], NULL, NULL );

    if (!window)
    {
        glfwTerminate( );
        exit( EXIT_FAILURE );
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback( window, reshape );
    glfwSetWindowRefreshCallback( window, display );
    glfwSetKeyCallback( window, keyboard );

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
        exit( EXIT_FAILURE );
    }
    fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
#endif

    init();

    glfwShowWindow( window );
    {
        int pixWidth, pixHeight;
        glfwGetFramebufferSize( window, &pixWidth, &pixHeight );
        reshape( window, pixWidth, pixHeight );
    }

    glfwSetTime(1.0);

    while (!glfwWindowShouldClose( window ))
    {
        display( window );
        glfwPollEvents( );

        if (screenshot_path)
        {
            screenshot( window, screenshot_path );
            glfwSetWindowShouldClose( window, 1 );
        }
    }

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}

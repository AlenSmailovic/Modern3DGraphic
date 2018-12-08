// Ex1 - Variabile uniforme.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 
#include <GL/glew.h> // glew apare inainte de freeglut
#include <GL/freeglut.h> // nu trebuie uitat freeglut.h

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// include the freeglut Library file
#pragma comment (lib, "freeglut.lib")
#pragma comment (lib, "glew32.lib")

enum MoveTypes {
	ENone,
	EHorizontal,
	EVertical,
	ECircle,
	ERotation,
	EScale
};

MoveTypes movementTypes = MoveTypes::ENone;

GLuint VaoId, VboId, ColorBufferId, VertexShaderId, FragmentShaderId, ProgramId;
GLuint WorldMatrixLocation;
// Shader-ul de varfuri / Vertex shader (este privit ca un sir de caractere)
const GLchar* VertexShader =
{
   "#version 400\n"\
   "layout(location=0) in vec4 in_Position;\n"\
   "layout(location=1) in vec4 in_Color;\n"\
   "out vec4 ex_Color;\n"\
   "uniform mat4 WorldMatrix;\n"\
   "void main()\n"\
   "{\n"\
   "  gl_Position = WorldMatrix * in_Position;\n"\
   "  ex_Color = in_Color;\n"\
   "}\n"
};
// Shader-ul de fragment / Fragment shader (este privit ca un sir de caractere)
const GLchar* FragmentShader =
{
   "#version 400\n"\
   "in vec4 ex_Color;\n"\
   "out vec4 out_Color;\n"\
   "void main()\n"\
   "{\n"\
   "  out_Color = ex_Color;\n"\
   "}\n"
};
void CreateVBO()
{
   // varfurile 
	// x, y, z
	// avem trei puncte care se unesc :)
   GLfloat Vertices[] = {
      -0.8f, -0.8f, 0.0f, 1.0f,
      0.0f,  0.8f, 0.0f, 1.0f,
      0.8f, -0.8f, 0.0f, 1.0f
   };
   // culorile, ca atribute ale varfurilor
   GLfloat Colors[] = {
      1.0f, 0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f, 1.0f
   };
   // se creeaza un buffer nou
   glGenBuffers(1, &VboId);
   // este setat ca buffer curent
   glBindBuffer(GL_ARRAY_BUFFER, VboId);
   // punctele sunt "copiate" in bufferul curent
   glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

   // se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO
   glGenVertexArrays(1, &VaoId);
   glBindVertexArray(VaoId);
   // se activeaza lucrul cu atribute; atributul 0 = pozitie
   glEnableVertexAttribArray(0);
   // definește un array de atribute generice pentru coordonate la locatia 0
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

   // un nou buffer, pentru culoare
   glGenBuffers(1, &ColorBufferId);
   glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
   // atributul 1 =  culoare
   glEnableVertexAttribArray(1);
   // definește un array de atribute generice pentru culoare la locatia 1
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}
void DestroyVBO()
{
   glDisableVertexAttribArray(1);
   glDisableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glDeleteBuffers(1, &ColorBufferId);
   glDeleteBuffers(1, &VboId);
   glBindVertexArray(0);
   glDeleteVertexArrays(1, &VaoId);
}
void CreateShaders()
{
   VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
   glCompileShader(VertexShaderId);

   FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
   glCompileShader(FragmentShaderId);

   ProgramId = glCreateProgram();
   glAttachShader(ProgramId, VertexShaderId);
   glAttachShader(ProgramId, FragmentShaderId);
   glLinkProgram(ProgramId);

   GLint Success = 0;
   GLchar ErrorLog[1024] = { 0 };

   glGetProgramiv(ProgramId, GL_LINK_STATUS, &Success);
   if (Success == 0) {
      glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
      fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
      exit(1);
   }

   glValidateProgram(ProgramId);
   glGetProgramiv(ProgramId, GL_VALIDATE_STATUS, &Success);
   if (!Success) {
      glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
      fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
      exit(1);
   }

   glUseProgram(ProgramId);
   // Se face legătura între variabila uniformă din shader și locația sa în shader, după ce s-a creat programul
   WorldMatrixLocation = glGetUniformLocation(ProgramId, "WorldMatrix");
}
void DestroyShaders()
{
   glUseProgram(0);

   glDetachShader(ProgramId, VertexShaderId);
   glDetachShader(ProgramId, FragmentShaderId);

   glDeleteShader(FragmentShaderId);
   glDeleteShader(VertexShaderId);

   glDeleteProgram(ProgramId);
}
void Initialize()
{
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // culoarea de fond a ecranului

   CreateVBO();
   CreateShaders();
}

float angle = 0.0f;
float dim = 0.0f;

void RenderFunction()
{
   glClear(GL_COLOR_BUFFER_BIT);

   glm::mat4 worldTransf = glm::mat4(1.0);
   switch (movementTypes)
   {
	   case MoveTypes::ENone:
		   worldTransf = glm::mat4(1.0);
		   break;
	   case MoveTypes::EHorizontal:
		   dim += 0.001f;
		   worldTransf = glm::translate(worldTransf, glm::vec3(glm::cos(dim), 0.0f, 0.0f));
		   break;
	   case MoveTypes::EVertical:
		   worldTransf = glm::translate(worldTransf, glm::vec3(0.0f, 0.2f, 0.0f));
		   break;
	   case MoveTypes::ECircle:
		   angle += 0.001f;
		   worldTransf = glm::translate(worldTransf, glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f));
		   break;
	   case MoveTypes::ERotation:
		   angle += 0.001f;
		   worldTransf = glm::rotate(worldTransf, glm::radians(angle * 180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		   break;
	   case MoveTypes::EScale:
		   angle += 0.001f;
		   worldTransf = glm::scale(worldTransf, glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f));
		   break;
	   default:
		   glm::mat4 worldTransf = glm::mat4(1.0);
		   break;
   }
   // Se modifică valoarea variabilei uniforme din shader de la locația specificată prin Id
   glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldTransf));

   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, VboId);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

   glEnableVertexAttribArray(1);
   glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

   glDrawArrays(GL_TRIANGLES, 0, 3);

   glDisableVertexAttribArray(0);

   glutSwapBuffers();
}

static void Keyboard(unsigned char Key, int x, int y)
{
   if (tolower(Key) == 'n') {
      // stop movement
	   movementTypes = MoveTypes::ENone;
   }
   if (tolower(Key) == 'h') {
      // horizontal translation
	   movementTypes = MoveTypes::EHorizontal;
   }
   if (tolower(Key) == 'v') {
      // vertical translacction
	   movementTypes = MoveTypes::EVertical;
   }
   if (tolower(Key) == 'c') {
      // vertical and horizontal translation
	   movementTypes = MoveTypes::ECircle;
   }
   if (tolower(Key) == 'z') {
      // rotation aroun OZ
	   movementTypes = MoveTypes::ERotation;
   }
   if (tolower(Key) == 's') {
      // scaling
	   movementTypes = MoveTypes::EScale;
   }
}

void Cleanup()
{
   DestroyShaders();
   DestroyVBO();
}
int main(int argc, char* argv[])
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
   glutInitWindowPosition(100, 100); // pozitia initiala a ferestrei
   glutInitWindowSize(1000, 700); //dimensiunile ferestrei
   glutCreateWindow("Primul triunghi - OpenGL <<nou>>"); // titlul ferestrei
                                                         // nu uitati de initializare glew; 
                                                         // trebuie initializat inainte de a a initializa desenarea
   glewInit();
   Initialize();
   glutDisplayFunc(RenderFunction);
   glutIdleFunc(RenderFunction);
   glutKeyboardFunc(Keyboard);

   glutCloseFunc(Cleanup);
   glutMainLoop();
}


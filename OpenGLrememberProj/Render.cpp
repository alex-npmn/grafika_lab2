#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

double ir = 0;
double jr = 0;
double kr = 0;
void computing(double A[], double B[], double C[]) {
	double sum = 0;
	ir = (B[1] - A[1]) * (C[2] - A[2]) - (C[1] - A[1]) * (B[2] - A[2]);//координаты нормального вектора
	jr = -((B[0] - A[0]) * (C[2] - A[2]) - (B[2] - A[2]) * (C[0] - A[0]));
	kr = (B[0] - A[0]) * (C[1] - A[1]) - (B[1] - A[1]) * (C[0] - A[0]);
	sum = sqrt(ir * ir + jr * jr + kr * kr);
	ir /= sum;
	jr /= sum;
	kr /= sum;
}


void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	
	//Начало рисования квадратика станкина
	/*
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();

	*/


	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	double A[] = { 0, 0, 0 };
	double B[] = { 2, 7, 0 };
	double C[] = { 5, 3, 0 };
	
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);

	glEnd();

	glBegin(GL_TRIANGLES);
	double A1[] = { 5, 3, 0 };
	double B1[] = { 4, 10, 0 };
	double C1[] = { 9, 5, 0 };

	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);

	double A2[] = { 9, 5, 0 };
	double B2[] = { 11, 10, 0 };
	double C2[] = { 15, 5, 0 };

	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(C2);

	double A3[] = { 0, 0, 0 };
	double B3[] = { 5, 3, 0 };
	double C3[] = { 15, 5, 0 };

	glVertex3dv(A3);
	glVertex3dv(B3);
	glVertex3dv(C3);

	double A4[] = { 5, 3, 0 };
	double B4[] = { 9, 5, 0 };
	double C4[] = { 15, 5, 0 };

	glVertex3dv(A4);
	glVertex3dv(B4);
	glVertex3dv(C4);






	//bottom
	glNormal3d(0, 0, 1);
	double A01[] = { 0, 0, 2 };
	double B01[] = { 2, 7, 2 };
	double C01[] = { 5, 3, 2 };

	glVertex3dv(A01);
	glVertex3dv(B01);
	glVertex3dv(C01);

	double A11[] = { 5, 3, 2 };
	double B11[] = { 4, 10, 2 };
	double C11[] = { 9, 5, 2 };

	glVertex3dv(A11);
	glVertex3dv(B11);
	glVertex3dv(C11);

	double A21[] = { 9, 5, 2 };
	double B21[] = { 11, 10, 2 };
	double C21[] = { 15, 5, 2 };

	glVertex3dv(A21);
	glVertex3dv(B21);
	glVertex3dv(C21);

	double A31[] = { 0, 0, 2 };
	double B31[] = { 5, 3, 2 };
	double C31[] = { 15, 5, 2 };

	glVertex3dv(A31);
	glVertex3dv(B31);
	glVertex3dv(C31);

	double A41[] = { 5, 3, 2 };
	double B41[] = { 9, 5, 2 };
	double C41[] = { 15, 5, 2 };

	glVertex3dv(A41);
	glVertex3dv(B41);
	glVertex3dv(C41);

	glEnd();

	glBegin(GL_QUADS);

	glColor3d(0.2, 0.7, 0.7);

	double D[] = { 0, 0, 0 };
	double E[] = { 15, 5, 0 };
	double F[] = { 15, 5, 2 };
	double G[] = { 0, 0, 2 };

	computing(D, E, G);
	glNormal3d(ir, jr, kr);


	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(G);

	glEnd();

	glBegin(GL_QUADS);
	double D1[] = { 15, 5, 0 };
	double E1[] = { 11, 10, 0 };
	double F1[] = { 11, 10, 2 };
	double G1[] = { 15, 5, 2 };
	computing(D1, E1, G1);
	glNormal3d(ir, jr, kr);

	glVertex3dv(D1);
	glVertex3dv(E1);
	glVertex3dv(F1);
	glVertex3dv(G1);

	glEnd();

	glBegin(GL_QUADS);

	double D2[] = { 11, 10, 0 };
	double E2[] = { 9, 5, 0 };
	double F2[] = { 9, 5, 2 };
	double G2[] = { 11, 10, 2 };
	computing(D2, E2, G2);
	glNormal3d(ir, jr, kr);

	glVertex3dv(D2);
	glVertex3dv(E2);
	glVertex3dv(F2);
	glVertex3dv(G2);
	glEnd();

	glBegin(GL_QUADS);
	double D3[] = { 9, 5, 0 };
	double E3[] = { 4, 10, 0 };
	double F3[] = { 4, 10, 2 };
	double G3[] = { 9, 5, 2 };
	computing(D3, E3, G3);
	glNormal3d(ir, jr, kr);

	glVertex3dv(D3);
	glVertex3dv(E3);
	glVertex3dv(F3);
	glVertex3dv(G3);

	glEnd();

	glBegin(GL_QUADS);
	double D4[] = { 4, 10, 0 };
	double E4[] = { 5, 3, 0 };
	double F4[] = { 5, 3, 2 };
	double G4[] = { 4, 10, 2 };
	computing(D4, E4, G4);
	glNormal3d(ir, jr, kr);

	glVertex3dv(D4);
	glVertex3dv(E4);
	glVertex3dv(F4);
	glVertex3dv(G4);

	glEnd();

	glBegin(GL_QUADS);
	double D5[] = { 5, 3, 0 };
	double E5[] = { 2, 7, 0 };
	double F5[] = { 2, 7, 2 };
	double G5[] = { 5, 3, 2 };
	computing(D5, E5, G5);
	glNormal3d(ir, jr, kr);

	glVertex3dv(D5);
	glVertex3dv(E5);
	glVertex3dv(F5);
	glVertex3dv(G5);

	glEnd();

	glBegin(GL_QUADS);
	double D6[] = { 2, 7, 0 };
	double E6[] = { 0, 0, 0 };
	double F6[] = { 0, 0, 2 };
	double G6[] = { 2, 7, 2 };
	computing(D6, E6, G6);
	glNormal3d(ir, jr, kr);

	glVertex3dv(D6);
	glVertex3dv(E6);
	glVertex3dv(F6);
	glVertex3dv(G6);

	glEnd();
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}
//Barnes-hut implementation - O(n log n) nbody algorithm implementation

//Copyright 2012 Mateus Zitelli <zitellimateus@gmail.com>

//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//MA 02110-1301, USA.

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "helper.h"
#define colorMax 10E2

float angleX = 0;
float angleY = 0;
float positionZ = -10;
int paused = 0, render = 1;
int frame = 0;
struct timespec start, end;
bitmap_t image;

void initRendering()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void handleResize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, 200.0);
}

void drawScene()
{
	int i;
	float r,g,b;
	unsigned char pRGB[WIDTH * HEIGHT * 3 + 3];
	double acel;
	char buf[20];
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, positionZ);

	glRotatef(angleX, 0.0f, 1.0f, 0.0f);
	glRotatef(angleY, 1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for (i = 0; i < BODIES_QUANTITY; i++) {
	        acel = bodies[i].acel;
	        r = acel;
	        if(r > 1) r =1;
	        g = acel;
	        if(g > 1.0) g = 1 + r * 2;
	        b = g + r;
	        glColor4f(r + 0.2, g + 0.2, b + 0.2, 0.4f);
		glVertex3f(bodies[i].position.x / (50000 * LY),
			   bodies[i].position.y / (50000 * LY),
			   bodies[i].position.z / (50000 * LY));
	}
	glEnd();
	glutSwapBuffers();
	glFlush();
#if 0
	glReadPixels(0, 0, HEIGHT, WIDTH, GL_RGB, GL_UNSIGNED_BYTE, pRGB);
	free(image.pixels);
	image.pixels = malloc(sizeof(pixel_t) * WIDTH * HEIGHT);
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		image.pixels[i].red = pRGB[i * 3];
		image.pixels[i].green = pRGB[i * 3 + 1];
		image.pixels[i].blue = pRGB[i * 3 + 2];
	}
	sprintf(buf, "%i.png", frame++);
	save_png_to_file(&image, buf);
#endif
}

void update(int value)
{
	if(frame == 0) clock_gettime(CLOCK_MONOTONIC, &start);
	int i, j, k;
	//angleX += 0.5;
	if (angleX > 360)
		angleX -= 360;
	if (angleY > 360)
		angleY -= 360;
	if (angleX < 0)
		angleX += 360;
	if (angleY < 0)
		angleY += 360;
	roots_quantity = 0;
	resetNodes();
	divideNode(&nodes[0]);
#pragma omp parallel for
	for (i = 0; i < node_quantity; i++) {
		setCenterOfMass(&nodes[i]);
	}
#pragma omp parallel for
	for (i = 0; i < roots_quantity; i++) {
#pragma omp parallel for
		for (j = 0; j < roots[i]->bodies_quantity; j++) {
			forceOverNode(roots[i], NULL, roots[i]->bodies[j], 0);
		}
	}
	fprintf(positionData, "FF\n");
	if (render)
		glutPostRedisplay();
	for (i = 0; i < BODIES_QUANTITY; i++) {
		bodies[i].speed.x += bodies[i].force.x / bodies[i].mass;
		bodies[i].speed.y += bodies[i].force.y / bodies[i].mass;
		bodies[i].speed.z += bodies[i].force.z / bodies[i].mass;
		bodies[i].acel = sqrt(bodies[i].force.x * bodies[i].force.x + bodies[i].force.y * bodies[i].force.y + bodies[i].force.z * bodies[i].force.z) / bodies[i].mass / colorMax;
		fprintf(positionData, "%i,%i,%i,%i\n",
			(int)(bodies[i].position.x * 10E-16),
			(int)(bodies[i].position.y * 10E-16),
			(int)(bodies[i].position.z * 10E-16),
			(int)bodies[i].acel);
		bodies[i].position.x += bodies[i].speed.x * 5E13;
		bodies[i].position.y += bodies[i].speed.y * 5E13;
		bodies[i].position.z += bodies[i].speed.z * 5E13;
		bodies[i].force.x = 0;
		bodies[i].force.y = 0;
		bodies[i].force.z = 0;
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Time Ellapsed = %f\n", timespecDiff(&end, &start) / (float)++frame);
}

void handleKeypress(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:		//Escape key
		exit(0);
		break;
	case GLUT_KEY_RIGHT:
		angleX += 2;
		break;
	case GLUT_KEY_LEFT:
		angleX -= 2;
		break;
	case GLUT_KEY_UP:
		angleY += 2;
		break;
	case GLUT_KEY_DOWN:
		angleY -= 2;
		break;
	case 112:
		paused = !paused;
		if (paused) {
			glutIdleFunc(NULL);
			fprintf(positionData, "P1\n");
		} else {
			glutIdleFunc(update);
			fprintf(positionData, "P0\n");
		}
		break;
	case 114:
		render = !render;
		break;
	case 43:
		positionZ -= 1;
		break;
	case 95:
		positionZ += 1;
		break;
	}
	printf("%i\n", key);
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	int i, frame = 0;
	srand(time(0));
	init();
	image.width = WIDTH;
	image.height = HEIGHT;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);

	glutCreateWindow("teste");
	initRendering();

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleKeypress);
	glutReshapeFunc(handleResize);
	glutIdleFunc(update);

	glutMainLoop();
	return 0;
}

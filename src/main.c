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
#include "core.h"
#include "render.h"
#include "config.h"

bitmap_t image;

int main(int argc, char **argv){
	int i, frame = 0;
	srand(0);
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

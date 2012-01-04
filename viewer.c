#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#define WIDTH 800
#define HEIGHT 800

#define        MAXLINELENGTH    1024
#define        BUFSIZE      50000

FILE * file;
long bytesread;
char buf[BUFSIZE];
int sizeLeftover = 0;
int bLoopCompleted = 0;
long pos = 0;
float angleX = 0;
float angleY = 0;
float positionZ = -10;
int paused = 0, render = 1;
frame = 0;
struct timespec start, end;

typedef struct{
        float x;
        float y;
        float z;
        int r;
        int g;
        int b;
} point;

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

point * extractFrameData(void){
        char c, buf[2000];
        long points;
        int loop = 1, bufpos = 0, i;
        point * pointList = (point *) malloc(sizeof(point) * 3E6);
        do {
                c = fgetc(file);
                if(c == "F" && fgetc(file) == "F" && fgetc(file) == "\n"){
                        loop = 0;
                }
                if (c == '\n'){
                        for(i = 0; i < bufpos; i++){
                                printf("%c\n",buf[i]);
                        }
                        bufpos = 0;
                        ++points;
                        printf("\n");
                }
                buf[bufpos++] = c;
        } while(loop);
        return(pointList);
}

void drawScene()
{
 /*
	int i;
	unsigned char pRGB[WIDTH * HEIGHT * 3 + 3];
	char buf[20];
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, positionZ);

	glRotatef(angleX, 0.0f, 1.0f, 0.0f);
	glRotatef(angleY, 1.0f, 0.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
	glBegin(GL_POINTS);
	for (i = 0; i < BODIES_QUANTITY; i++) {
		glVertex3f(bodies[i].position.x / (50000 * LY),
			   bodies[i].position.y / (50000 * LY),
			   bodies[i].position.z / (50000 * LY));
	}
	glEnd();
	glutSwapBuffers();
	glFlush();
	*/
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
	++frame;
	glutPostRedisplay();
	extractFrameData();
	
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
		} else {
			glutIdleFunc(update);
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
	if (!(file = fopen("positionData.csv", "rb")))
		return (0);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);

	glutCreateWindow("Viewer");
	initRendering();

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleKeypress);
	glutReshapeFunc(handleResize);
	glutIdleFunc(update);

	glutMainLoop();
	return 0;
}

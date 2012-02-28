#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <png.h>

#define        MAXLINELENGTH    1024
#define        BUFSIZE      50000

typedef struct{
        float x;
        float y;
        float z;
        int r;
        int g;
        int b;
} point;

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} pixel_t;

/* A picture. */

typedef struct {
	pixel_t *pixels;
	size_t Width;
	size_t Height;
} bitmap_t;

FILE * file;
long bytesread;
char buf[BUFSIZE];
int sizeLeftover = 0;
int bLoopCompleted = 0;
long pos = 0;
float angleX = 0;
float angleY = 0;
float positionZ = -10;
int paused = 0, render = 1, frame = 0;
long points;
point * pointsData;
point * pointList;
struct timespec start, end;
bitmap_t image;
GLint m_viewport[4];
int Width = 1280, Height = 1280;

int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
	return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
	    ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
   success, non-zero on error. */
   
pixel_t * pixel_at(bitmap_t * bitmap, int x, int y){
        return(bitmap->pixels + y * bitmap->Width + x);
}

static int save_png_to_file(bitmap_t * bitmap, const char *path)
{
	FILE *fp;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	size_t x, y;
	png_byte **row_pointers = NULL;
	/* "status" contains the return value of this function. At first
	   it is set to a value which means 'failure'. When the routine
	   has finished its work, it is set to a value which means
	   'success'. */
	int status = -1;
	/* The following number is set by trial and error only. I cannot
	   see where it it is documented in the libpng manual.
	 */
	int pixel_size = 3;
	int depth = 8;

	fp = fopen(path, "wb");
	if (!fp) {
		goto fopen_failed;
	}

	png_ptr =
	    png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		goto png_create_write_struct_failed;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		goto png_create_info_struct_failed;
	}

	/* Set up error handling. */

	if (setjmp(png_jmpbuf(png_ptr))) {
		goto png_failure;
	}

	/* Set image attributes. */

	png_set_IHDR(png_ptr,
		     info_ptr,
		     bitmap->Width,
		     bitmap->Height,
		     depth,
		     PNG_COLOR_TYPE_RGB,
		     PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Initialize rows of PNG. */

	row_pointers = png_malloc(png_ptr, bitmap->Height * sizeof(png_byte *));
	for (y = 0; y < bitmap->Height; ++y) {
		png_byte *row =
		    png_malloc(png_ptr,
			       sizeof(uint8_t) * bitmap->Width * pixel_size);
		row_pointers[y] = row;
		for (x = 0; x < bitmap->Width; ++x) {
			pixel_t *pixel = pixel_at(bitmap, x, y);
			*row++ = pixel->red;
			*row++ = pixel->green;
			*row++ = pixel->blue;
		}
	}

	/* Write the image data to "fp". */

	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	/* The routine has successfully written the file, so we set
	   "status" to a value which indicates success. */

	status = 0;

	for (y = 0; y < bitmap->Height; y++) {
		png_free(png_ptr, row_pointers[y]);
	}
	png_free(png_ptr, row_pointers);

 png_failure:
 png_create_info_struct_failed:
	png_destroy_write_struct(&png_ptr, &info_ptr);
 png_create_write_struct_failed:
	fclose(fp);
 fopen_failed:
	return status;
}


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
	Width = w; Height = h;
	printf("%i, %i\n", w,h);
	image.Width = Width;
	image.Height = Height;
}

point * extractFrameData(void){
        char c, buf[2000];
        points = 0;
        int loop = 1, bufpos = 0, i, num, neg;
	double cords[4];
        do {
                c = fgetc(file);
                if(c == 'F' && fgetc(file) == 'F' && fgetc(file) == '\n'){
                        loop = 0;
                }
		if(c == EOF){
			printf("End of File\n");
			exit(0);
		}
                if (c == '\n'){
			num = 0;
			neg = 0;
			cords[0] = 0;
                        for(i = 1; i < bufpos; i++){
				if(buf[i] > '9' && buf[i] < '0' && buf[i] != ',' && buf[i] != '-') continue;
				if(buf[i] == ','){ //buf[i] == ","
					if(neg) cords[num] = -cords[num];
					neg = 0;
					++i;
					cords[++num] = 0;
				}
				if(buf[i] == '-'){
					cords[num] = buf[i + 1] - 48;//buf[i] == "-"
					neg = 1;
					i += 2;
				}
				if(buf[i] <= '9' && buf[i] >= '0')
					cords[num] = cords[num] * 10 + (buf[i] - 48);
                                //printf("%i ",cords[num]);
                        }
			if(neg) cords[num] = -cords[num];
			neg = 0;
			pointList[points].x = cords[0];
			pointList[points].y = cords[1];
			pointList[points].z = cords[2];
			pointList[points].r = cords[3];
			//printf("%f, %f, %f\n", pointList[points].x, pointList[points].y, pointList[points].z);
			++points;
                        bufpos = 0;
                        //printf("\n");
                }
                buf[bufpos++] = c;
        } while(loop);
        return(pointList);
}

void drawScene()
{

	int i, j, k;
	unsigned char pRGB[Width * Height * 3 + 3];
	char buf[20];
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, positionZ);

	glRotatef(angleX, 0.0f, 1.0f, 0.0f);
	glRotatef(angleY, 1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for (i = 0; i < points; i++) {
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
		glVertex3f(pointsData[i].x / (500000),
			   pointsData[i].y / (500000),
			   pointsData[i].z / (500000));
	}
	glEnd();
	glutSwapBuffers();
	glFlush();
#if 0
	if(frame % 3 == 0) return;
	glReadPixels(0, 0, Height, Width, GL_RGB, GL_UNSIGNED_BYTE, pRGB);
	free(image.pixels);
	image.pixels = malloc(sizeof(pixel_t) * Width * Height);
	for (i = 0; i < Width * Height; i++) {
		image.pixels[i].red = pRGB[i * 3];
		image.pixels[i].green = pRGB[i * 3 + 1];
		image.pixels[i].blue = pRGB[i * 3 + 2];
	}
	sprintf(buf, "%i.png", frame / 3);
	save_png_to_file(&image, buf);
#endif
}

void update()
{
	int i;
	++frame;
	glutPostRedisplay();
	pointsData = extractFrameData();
	drawScene();
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
	pointList = pointList = (point *) malloc(sizeof(point) * 3E6);
	if (!(file = fopen("positionData.csv", "rb")))
		return (0);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(Width, Height);
	image.Width = Width;
	image.Height = Height;
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

#include <GL/glut.h>
#include "render.h"
#include "core.h"
#include "config.h"

int paused = 0;
float angleX = 0, angleY = 0, positionZ = -10;
struct timespec start, end;
int render = 0, frame = 0;
extern int bodiesQuantity;


float Color_list[56][3] = { {204, 0, 0},
{102, 204, 0},
{0, 204, 204},
{102, 0, 204},

{117, 80, 123},
{122, 95, 80},
{85, 122, 80},
{80, 107, 122},

{52, 101, 164},
{163, 52, 156},
{163, 115, 52},
{52, 163, 60},

{115, 210, 22},
{21, 209, 209},
{115, 21, 209},
{209, 21, 21},

{193, 125, 17},
{17, 194, 38},
{17, 85, 194},
{194, 17, 173},

{245, 121, 0},
{0, 245, 0},
{0, 122, 245},
{254, 0, 245},

{237, 212, 0},
{0, 237, 95},
{0, 24, 237},
{237, 0, 142},

{239, 41, 41},
{140, 240, 41},
{41, 240, 240},
{140, 41, 240},

{173, 127, 168},
{173, 155, 127},
{127, 173, 132},
{127, 145, 173},

{114, 159, 207},
{207, 144, 205},
{207, 162, 114},
{114, 207, 115},

{138, 226, 52},
{52, 227, 227},
{140, 52, 227},
{227, 52, 52},

{233, 185, 110},
{109, 232, 123},
{109, 156, 232},
{232, 109, 218},

{252, 175, 62},
{63, 252, 82},
{63, 139, 252},
{252, 63, 234},

{252, 233, 79},
{78, 252, 145},
{78, 99, 252},
{252, 78, 186}
};

/* based on Delphi function by Witold J.Janik */
void
GiveRainbowColor (double position, unsigned char *c)
{
  /* if position > 1 then we have repetition of colors it maybe useful    */

  if (position > 1.0) {
    if (position - (int) position == 0.0)
      position = 1.0;
    else
      position = position - (int) position;
  }

  unsigned char nmax = 6;	/* number of color segments */
  double m = nmax * position;

  int n = (int) m;		// integer of m

  double f = m - n;		// fraction of m
  unsigned char t = position * 0.1;

  switch (n) {
  case 0:{
      c[0] = 255;
      c[1] = t;
      c[2] = 0;
      break;
    };
  case 1:{
      c[0] = 255 - t;
      c[1] = 255;
      c[2] = 0;
      break;
    };
  case 2:{
      c[0] = 0;
      c[1] = 255;
      c[2] = t;
      break;
    };
  case 3:{
      c[0] = 0;
      c[1] = 255 - t;
      c[2] = 255;
      break;
    };
  case 4:{
      c[0] = t;
      c[1] = 0;
      c[2] = 255;
      break;
    };
  case 5:{
      c[0] = 255;
      c[1] = 0;
      c[2] = 255 - t;
      break;
    };
  default:{
      c[0] = 255;
      c[1] = 0;
      c[2] = 0;
      break;
    };

  };				// case
}



void
initRendering ()
{
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_NORMALIZE);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
handleResize (int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (45.0, (float) w / (float) h, 1.0, 200.0);
}


void
handleKeypress (unsigned char key, int x, int y)
{
  switch (key) {
  case 27:			//Escape key
    exit (0);
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
      glutIdleFunc (NULL);
      fprintf (positionData, "P1\n");
    } else {
      glutIdleFunc (update);
      fprintf (positionData, "P0\n");
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
  printf ("%i\n", key);
  glutPostRedisplay ();
}


void
drawScene ()
{
  int i;
  float r, g, b;
  unsigned char pRGB[WIDTH * HEIGHT * 3 + 3];
  double acel;
  unsigned char color[3];
  char buf[20];
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  glTranslatef (0.0f, 0.0f, positionZ);

  glRotatef (angleX, 0.0f, 1.0f, 0.0f);
  glRotatef (angleY, 1.0f, 1.0f, 0.0f);
  glBegin (GL_POINTS);
  for (i = 0; i < bodiesQuantity; i++) {
    GiveRainbowColor (bodies[i].acel, color);
    glColor4f (255, 255, 255, 0.5f);
    glVertex3f (bodies[i].position.x / (50000 * LY),
		bodies[i].position.y / (50000 * LY),
		bodies[i].position.z / (50000 * LY));
  }
  glEnd ();
  //Tree renderization
#if 0
  for (i = 0; i < node_quantity; i++) {
    glColor4f (Color_list[i % 56][0] / 256.0, Color_list[i % 56][1] / 256.0,
	       Color_list[i % 56][2] / 256.0, 1);
    glBegin (GL_LINE_STRIP);
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].end.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glVertex3f (nodes[i].end.x / (50000 * LY), nodes[i].end.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glVertex3f (nodes[i].end.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].end.z / (50000 * LY));
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].end.y / (50000 * LY), nodes[i].end.z / (50000 * LY));
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].end.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glEnd ();
    glBegin (GL_LINE_STRIP);
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].end.y / (50000 * LY), nodes[i].end.z / (50000 * LY));
    glVertex3f (nodes[i].end.x / (50000 * LY), nodes[i].end.y / (50000 * LY),
		nodes[i].end.z / (50000 * LY));
    glVertex3f (nodes[i].end.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].end.z / (50000 * LY));
    glVertex3f (nodes[i].start.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].end.z / (50000 * LY));
    glEnd ();
    glBegin (GL_LINE_STRIP);
    glVertex3f (nodes[i].end.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].end.z / (50000 * LY));
    glVertex3f (nodes[i].end.x / (50000 * LY),
		nodes[i].start.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glEnd ();
    glBegin (GL_LINE_STRIP);
    glVertex3f (nodes[i].end.x / (50000 * LY), nodes[i].end.y / (50000 * LY),
		nodes[i].start.z / (50000 * LY));
    glVertex3f (nodes[i].end.x / (50000 * LY), nodes[i].end.y / (50000 * LY),
		nodes[i].end.z / (50000 * LY));
    glEnd ();
  }
#endif
  glutSwapBuffers ();
  glFlush ();
#if 0
  glReadPixels (0, 0, HEIGHT, WIDTH, GL_RGB, GL_UNSIGNED_BYTE, pRGB);
  free (image.pixels);
  image.pixels = malloc (sizeof (pixel_t) * WIDTH * HEIGHT);
  for (i = 0; i < WIDTH * HEIGHT; i++) {
    image.pixels[i].red = pRGB[i * 3];
    image.pixels[i].green = pRGB[i * 3 + 1];
    image.pixels[i].blue = pRGB[i * 3 + 2];
  }
  sprintf (buf, "%i.png", frame++);
  save_png_to_file (&image, buf);
#endif
}

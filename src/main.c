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
#include <time.h>
#include <string.h>
#include "core.h"
#include "render.h"
#include "config.h"

struct timespec start;
int bodiesQuantity = BODIES_QUANTITY;
int frameLimit = -1;
int visualMode = 1;

bitmap_t image;

void
readArgs (int argc, char **argv)
{
  int i, frame = 0;
  int m, n,			/* Loop counters. */
    l,				/* String length. */
    x,				/* Exit code. */
    ch;				/* Character buffer. */
  char s[256];			/* String buffer. */

  for (n = 1; n < argc; n++) {	/* Scan through args. */
    switch ((int) argv[n][0]) {	/* Check for option character. */
    case '-':
    case '/':
      x = 0;			/* Bail out if 1. */
      l = strlen (argv[n]);
      for (m = 1; m < l; ++m) {	/* Scan through options. */
	ch = (int) argv[n][m];
	switch (ch) {
	case 'n':		/* Legal options. */
	case 'N':
	case 'f':
	case 'F':
	case 'b':
	case 'B':
	  break;
	default:
	  printf ("Illegal option code = %c\n", ch);
	  x = 1;		/* Not legal option. */
	  exit (1);
	  break;
	}
	if (x == 1) {
	  break;
	}
      }
      break;
    default:
      if (ch == 'n' || ch == 'N') {
        bodiesQuantity = atoi (argv[n]);
      } else if (ch == 'f' || ch == 'F') {
        frameLimit = atoi (argv[n]);
      } else if (ch == 'b' || ch == 'B'){
       visualMode = atoi(argv[n]); 
      }
      break;
    }
  }
}

int
main (int argc, char **argv)
{
  clock_gettime (CLOCK_MONOTONIC, &start);
  readArgs (argc, argv);
  srand (0);
  init ();
  if(visualMode){
    image.width = WIDTH;
    image.height = HEIGHT;
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize (WIDTH, HEIGHT);

    glutCreateWindow ("teste");
    initRendering ();

    glutDisplayFunc (drawScene);
    glutKeyboardFunc (handleKeypress);
    glutSpecialFunc (handleKeypress);
    glutReshapeFunc (handleResize);
    glutIdleFunc (update);

    glutMainLoop ();
  }else{
    benchMode();   
  }
  return 0;
}

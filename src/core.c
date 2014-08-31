#include <png.h>
#include <stdint.h>
#include <time.h>
#include "config.h"
#include "core.h"

#define colorMax 3E4

extern int paused, render, frame, bodiesQuantity, frameLimit, visualMode;
extern float angleX, angleY, positionZ;
extern struct timespec start, end;

int64_t
timespecDiff (struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
      ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
   success, non-zero on error. */

pixel_t *
pixel_at (bitmap_t * bitmap, int x, int y)
{
  return (bitmap->pixels - y * bitmap->width - x);
}

static int
save_png_to_file (bitmap_t * bitmap, const char *path)
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

  fp = fopen (path, "wb");
  if (!fp) {
    goto fopen_failed;
  }

  png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    goto png_create_write_struct_failed;
  }

  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    goto png_create_info_struct_failed;
  }

  /* Set up error handling. */

  if (setjmp (png_jmpbuf (png_ptr))) {
    goto png_failure;
  }

  /* Set image attributes. */

  png_set_IHDR (png_ptr,
		info_ptr,
		bitmap->width,
		bitmap->height,
		depth,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  /* Initialize rows of PNG. */

  row_pointers = png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
  for (y = 0; y < bitmap->height; ++y) {
    png_byte *row = png_malloc (png_ptr,
				sizeof (uint8_t) * bitmap->width *
				pixel_size);
    row_pointers[y] = row;
    for (x = 0; x < bitmap->width; ++x) {
      pixel_t *pixel = pixel_at (bitmap, x, y);
      *row++ = pixel->red;
      *row++ = pixel->green;
      *row++ = pixel->blue;
    }
  }

  /* Write the image data to "fp". */

  png_init_io (png_ptr, fp);
  png_set_rows (png_ptr, info_ptr, row_pointers);
  png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  /* The routine has successfully written the file, so we set
     "status" to a value which indicates success. */

  status = 0;

  for (y = 0; y < bitmap->height; y++) {
    png_free (png_ptr, row_pointers[y]);
  }
  png_free (png_ptr, row_pointers);

png_failure:
png_create_info_struct_failed:
  png_destroy_write_struct (&png_ptr, &info_ptr);
png_create_write_struct_failed:
  fclose (fp);
fopen_failed:
  return status;
}

struct node *nodes;
struct node **roots;
struct body *bodies;

void
initializeNode (struct node *node, struct node *up, double sx, double sy,
		double sz, double ex, double ey, double ez,
		int bodies_quantity, int deep)
{
  struct coord coord;
  float regulator;
  coord.x = sx;
  coord.y = sy;
  coord.z = sz;
  node->start = coord;
  coord.x = ex;
  coord.y = ey;
  coord.z = ez;
  node->end = coord;
  node->deep = deep;
  node->bodies_quantity = 0;
  node->has_center_of_mass = 0;
  if (!node->initialized) {
    node->bodies =
	(struct body **) malloc (sizeof (struct body *) * bodies_quantity);
    node->initialized = 1;
  }
  node->UNE = NULL;
  node->UNW = NULL;
  node->USE = NULL;
  node->USW = NULL;
  node->DNE = NULL;
  node->DNW = NULL;
  node->DSE = NULL;
  node->DSW = NULL;
  node->UP = up;
}

void
addBodyInNode (struct body *body, struct node *node)
{
  node->bodies[node->bodies_quantity++] = body;
}

void
resetNodes (void)
{
  nodes[0].UNE = NULL;
  nodes[0].UNW = NULL;
  nodes[0].USE = NULL;
  nodes[0].USW = NULL;
  nodes[0].DNE = NULL;
  nodes[0].DNW = NULL;
  nodes[0].DSE = NULL;
  nodes[0].DSW = NULL;
  nodes[0].UP = NULL;
  nodes[0].initialized = 0;
  nodes[0].has_center_of_mass = 0;
  int i;
  printf ("Reset %i\n", node_quantity);
  for (i = 1; i < node_quantity; i++) {
    nodes[i].UNE = NULL;
    nodes[i].UNW = NULL;
    nodes[i].USE = NULL;
    nodes[i].USW = NULL;
    nodes[i].DNE = NULL;
    nodes[i].DNW = NULL;
    nodes[i].DSE = NULL;
    nodes[i].DSW = NULL;
    nodes[i].UP = NULL;
    nodes[i].has_center_of_mass = 0;
    if (nodes[i].initialized && nodes[i].bodies != NULL) {
      free (nodes[i].bodies);
    }
    nodes[i].initialized = 0;
  }
  node_quantity = 1;
}

void
divideNode (struct node *node)
{
  if (node == NULL)
    return;
  if (node->bodies_quantity == 1) {
    roots[roots_quantity++] = node;
    return;
  } else if (node->bodies_quantity == 0) {
    return;
  }
  int i, UNW = 0, UNE = 0, USW = 0, USE = 0, DNW = 0, DNE = 0, DSW =
      0, DSE = 0;
  double sx, sy, sz, ex, ey, ez, mx, my, mz;
  sx = node->start.x;
  sy = node->start.y;
  sz = node->start.z;
  ex = node->end.x;
  ey = node->end.y;
  ez = node->end.z;
  mx = (sx + ex) / 2.0;
  my = (sy + ey) / 2.0;
  mz = (sz + ez) / 2.0;
  for (i = 0; i < node->bodies_quantity; i++) {
    if (node->bodies[i]->position.x < sx
	|| node->bodies[i]->position.x >= ex
	|| node->bodies[i]->position.y < sy
	|| node->bodies[i]->position.y >= ey
	|| node->bodies[i]->position.z < sz
	|| node->bodies[i]->position.z >= ez)
      continue;
    if (node->bodies[i]->position.x < mx) {
      if (node->bodies[i]->position.y < my) {
	if (node->bodies[i]->position.z < mz) {
	  ++UNW;
	} else {		// z >= mz
	  ++DNW;
	}
      } else {			// y >= my
	if (node->bodies[i]->position.z < mz) {
	  ++USW;
	} else {		// z >= mz
	  ++DSW;
	}
      }
    } else {			// x >= mx
      if (node->bodies[i]->position.y < my) {
	if (node->bodies[i]->position.z < mz) {
	  ++UNE;
	} else {		// z >= mz
	  ++DNE;
	}
      } else {			// y >= my
	if (node->bodies[i]->position.z < mz) {
	  ++USE;
	} else {		// z >= mz
	  ++DSE;
	}
      }
    }
  }
  for (i = 0; i < node->bodies_quantity; i++) {
    if (node->bodies[i]->position.x < sx
	|| node->bodies[i]->position.x >= ex
	|| node->bodies[i]->position.y < sy
	|| node->bodies[i]->position.y >= ey
	|| node->bodies[i]->position.z < sz
	|| node->bodies[i]->position.z >= ez)
      continue;
    if (node->bodies[i]->position.x < mx) {
      if (node->bodies[i]->position.y < my) {
	if (node->bodies[i]->position.z < mz) {
	  if (node->UNW == NULL) {
	    node->UNW = &nodes[node_quantity++];
	    initializeNode (node->UNW, node,
			    sx, sy, sz, mx, my, mz, UNW, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->UNW);
	} else {		// z >= mz
	  if (node->DNW == NULL) {
	    node->DNW = &nodes[node_quantity++];
	    initializeNode (node->DNW, node,
			    sx, sy, mz, mx, my, ez, DNW, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->DNW);
	}
      } else {			// y >= my
	if (node->bodies[i]->position.z < mz) {
	  if (node->USW == NULL) {
	    node->USW = &nodes[node_quantity++];
	    initializeNode (node->USW, node,
			    sx, my, sz, mx, ey, mz, USW, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->USW);
	} else {		// z >= mz
	  if (node->DSW == NULL) {
	    node->DSW = &nodes[node_quantity++];
	    initializeNode (node->DSW, node,
			    sx, my, mz, mx, ey, ez, DSW, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->DSW);
	}
      }
    } else {			// x >= mx
      if (node->bodies[i]->position.y < my) {
	if (node->bodies[i]->position.z < mz) {
	  if (node->UNE == NULL) {
	    node->UNE = &nodes[node_quantity++];
	    initializeNode (node->UNE, node,
			    mx, sy, sz, ex, my, mz, UNE, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->UNE);
	} else {		// z >= mz
	  if (node->DNE == NULL) {
	    node->DNE = &nodes[node_quantity++];
	    initializeNode (node->DNE, node,
			    mx, sy, mz, ex, my, ez, DNE, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->DNE);
	}
      } else {			// y >= my
	if (node->bodies[i]->position.z < mz) {
	  if (node->USE == NULL) {
	    node->USE = &nodes[node_quantity++];
	    initializeNode (node->USE, node,
			    mx, my, sz, ex, ey, mz, USE, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->USE);
	} else {		// z >= mz
	  if (node->DSE == NULL) {
	    node->DSE = &nodes[node_quantity++];
	    initializeNode (node->DSE, node,
			    mx, my, mz, ex, ey, ez, DSE, node->deep + 1);
	  }
	  addBodyInNode (node->bodies[i], node->DSE);
	}
      }
    }
  }
  divideNode (node->UNW);
  divideNode (node->UNE);
  divideNode (node->USW);
  divideNode (node->USE);
  divideNode (node->DNW);
  divideNode (node->DNE);
  divideNode (node->DSW);
  divideNode (node->DSE);
}

void
setCenterOfMass (struct node *node)
{
  double px = 0, py = 0, pz = 0, mass = 0, totalSpeed, relativisticAjust;
  int i;
  struct body centerOfMass;
  if (node->bodies_quantity >= 1) {
    for (i = 0; i < node->bodies_quantity; i++) {
      totalSpeed =
	  sqrt (node->bodies[i]->speed.x *
		node->bodies[i]->speed.x +
		node->bodies[i]->speed.y *
		node->bodies[i]->speed.y +
		node->bodies[i]->speed.z * node->bodies[i]->speed.z);
      relativisticAjust = 1 / sqrt (1 - (totalSpeed * totalSpeed) / (C * C));
      px +=
	  node->bodies[i]->position.x *
	  node->bodies[i]->mass * relativisticAjust;
      py +=
	  node->bodies[i]->position.y *
	  node->bodies[i]->mass * relativisticAjust;
      pz +=
	  node->bodies[i]->position.z *
	  node->bodies[i]->mass * relativisticAjust;
      mass += node->bodies[i]->mass * relativisticAjust;
    }
    centerOfMass.position.x = px / mass;
    centerOfMass.position.y = py / mass;
    centerOfMass.position.z = pz / mass;
    centerOfMass.mass = mass;
    node->centerOfMass = centerOfMass;
  } else {
    node->centerOfMass.position.x = 0;
    node->centerOfMass.position.y = 0;
    node->centerOfMass.position.z = 0;
    node->centerOfMass.mass = 0;
  }
  node->has_center_of_mass = 1;
}

void
applyForceBetweenBodies (struct body *b1, struct body *b2)
{
  double xDistance, yDistance, zDistance, DistanceSquared, force, distance;
  xDistance = b2->position.x - b1->position.x;
  yDistance = b2->position.y - b1->position.y;
  zDistance = b2->position.z - b1->position.z;
  DistanceSquared =
      xDistance * xDistance + yDistance * yDistance +
      zDistance * zDistance + EPS2;
  force = K * b2->mass * b1->mass / DistanceSquared * 0.5;
  double dist = sqrt (DistanceSquared);
  b1->force.x += xDistance / dist * force;
  b1->force.y += yDistance / dist * force;
  b1->force.z += zDistance / dist * force;
}

int
applyForce (struct node *node, struct body *body)
{
  double xDistance, yDistance, zDistance, DistanceSquared, force, distance;
  struct body centerOfMass;
  xDistance = (body->position.x - (node->end.x + node->start.x) / 2.0);
  yDistance = (body->position.y - (node->end.y + node->start.y) / 2.0);
  zDistance = (body->position.z - (node->end.z + node->start.z) / 2.0);
  distance =
      sqrt (xDistance * xDistance + yDistance * yDistance +
	    zDistance * zDistance);
  if ((node->end.x - node->start.x) / distance < ALPHA
      || node->bodies_quantity == 1) {
    centerOfMass = node->centerOfMass;
    xDistance = centerOfMass.position.x - body->position.x;
    yDistance = centerOfMass.position.y - body->position.y;
    zDistance = centerOfMass.position.z - body->position.z;
    DistanceSquared =
	xDistance * xDistance + yDistance * yDistance +
	zDistance * zDistance + EPS2;
    force = K * centerOfMass.mass * body->mass / DistanceSquared;
    body->force.x += xDistance / sqrt (DistanceSquared) * force;
    body->force.y += yDistance / sqrt (DistanceSquared) * force;
    body->force.z += zDistance / sqrt (DistanceSquared) * force;
    return (1);
  } else {
    return (0);
  }
}

void
forceOverNode (struct node *node, struct node *down, struct body *body,
	       int inverse)
{
  int var, i;
  if (node == NULL)
    return;
  if (node->UNE != NULL && (node->UNE != down || inverse)) {
    var = 1;
    var = applyForce (node->UNE, body);
    if (!var) {
      forceOverNode (node->UNE, node, body, 1);
    }
  }
  if (node->UNW != NULL && (node->UNW != down || inverse)) {
    var = applyForce (node->UNW, body);
    if (!var) {
      forceOverNode (node->UNW, node, body, 1);
    }
  }
  if (node->USE != NULL && (node->USE != down || inverse)) {
    var = applyForce (node->USE, body);
    if (!var) {
      forceOverNode (node->USE, node, body, 1);
    }
  }
  if (node->USW != NULL && (node->USW != down || inverse)) {
    var = applyForce (node->USW, body);
    if (!var) {
      forceOverNode (node->USW, node, body, 1);
    }
  }
  if (node->DNE != NULL && (node->DNE != down || inverse)) {
    var = applyForce (node->DNE, body);
    if (!var) {
      forceOverNode (node->DNE, node, body, 1);
    }
  }
  if (node->DNW != NULL && (node->DNW != down || inverse)) {
    var = applyForce (node->DNW, body);
    if (!var) {
      forceOverNode (node->DNW, node, body, 1);
    }
  }
  if (node->DSE != NULL && (node->DSE != down || inverse)) {
    var = applyForce (node->DSE, body);
    if (!var) {
      forceOverNode (node->DSE, node, body, 1);
    }
  }
  if (node->DSW != NULL && (node->DSW != down || inverse)) {
    var = applyForce (node->DSW, body);
    if (!var) {
      forceOverNode (node->DSW, node, body, 1);
    }
  }
  if (!inverse)
    forceOverNode (node->UP, node, body, 0);
  return;
}

void
init (void)
{
  int i, j;
  double vx, vy, vz, theta, dist, dist2;
  positionData = fopen ("positionData.csv", "wb");
  nodes = (struct node *) malloc (sizeof (struct node) * MAX_NODES);
  roots = (struct node **) malloc (sizeof (struct node *) * MAX_NODES);
  bodies = (struct body *) malloc (sizeof (struct body) * bodiesQuantity);

  for (i = 0; i < MAX_NODES; i++) {
    nodes[i].initialized = 0;
    nodes[i].UNE = NULL;
    nodes[i].UNW = NULL;
    nodes[i].USE = NULL;
    nodes[i].USW = NULL;
    nodes[i].DNE = NULL;
    nodes[i].DNW = NULL;
    nodes[i].DSE = NULL;
    nodes[i].DSW = NULL;
  }

  initializeNode (&nodes[0], NULL, -SIZE_OF_SIMULATION * 30,
		  -SIZE_OF_SIMULATION * 30, -SIZE_OF_SIMULATION * 30,
		  SIZE_OF_SIMULATION * 30, SIZE_OF_SIMULATION * 30,
		  SIZE_OF_SIMULATION * 30, bodiesQuantity * 30, 0);

  for (i = 0; i < bodiesQuantity; i++) {
    vx = (rand () % 10000000 / 10000000.0) * 120E3 * LY - 60E3 * LY;
    vy = (rand () % 10000000 / 10000000.0) * 120E3 * LY - 60E3 * LY;
    vz = (rand () % 10000000 / 10000000.0) * 120E3 * LY - 60E3 * LY;
    bodies[i].position.x = vx;
    bodies[i].position.y = vy;
    bodies[i].position.z = vz;
    bodies[i].force.x = 0;
    bodies[i].force.y = 0;
    bodies[i].force.z = 0;
    bodies[i].mass = TOTAL_MASS / bodiesQuantity;
    bodies[i].speed.x = 0;
    bodies[i].speed.y = 0;
    bodies[i].speed.z = 0;
    addBodyInNode (&bodies[i], &nodes[0]);
  }
}


void
update (int value)
{
  if (frame == 0)
    clock_gettime (CLOCK_MONOTONIC, &start);
  else if(frame == frameLimit){
    exit(0);
  }
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
  resetNodes ();
  divideNode (&nodes[0]);
#pragma omp parallel for
  for (i = 0; i < node_quantity; i++) {
    setCenterOfMass (&nodes[i]);
  }
#pragma omp parallel for
  for (i = 0; i < roots_quantity; i++) {
#pragma omp parallel for
    for (j = 0; j < roots[i]->bodies_quantity; j++) {
      forceOverNode (roots[i], NULL, roots[i]->bodies[j], 0);
    }
  }
  fprintf (positionData, "FF\n");
  if (render && visualMode)
    glutPostRedisplay ();
  for (i = 0; i < bodiesQuantity; i++) {
    bodies[i].speed.x += bodies[i].force.x / bodies[i].mass;
    bodies[i].speed.y += bodies[i].force.y / bodies[i].mass;
    bodies[i].speed.z += bodies[i].force.z / bodies[i].mass;
    bodies[i].acel = sqrt (bodies[i].speed.x * bodies[i].speed.x +
			   bodies[i].speed.y * bodies[i].speed.y +
			   bodies[i].speed.z * bodies[i].speed.z) / colorMax;
    fprintf (positionData, "%i,%i,%i,%i\n",
	     (int) (bodies[i].position.x * 10E-16),
	     (int) (bodies[i].position.y * 10E-16),
	     (int) (bodies[i].position.z * 10E-16), (int) bodies[i].acel);
    bodies[i].position.x += bodies[i].speed.x * 50E12;
    bodies[i].position.y += bodies[i].speed.y * 50E12;
    bodies[i].position.z += bodies[i].speed.z * 50E12;
    bodies[i].force.x = 0;
    bodies[i].force.y = 0;
    bodies[i].force.z = 0;
  }
  clock_gettime (CLOCK_MONOTONIC, &end);
  printf ("Time Ellapsed = %f\n",
	  timespecDiff (&end, &start) / (float) ++frame);
}

void benchMode(){
  while(1){
    update(0);
  }
}

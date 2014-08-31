#include <png.h>

int node_quantity;
int roots_quantity;
FILE *positionData;

struct coord {
	double x;
	double y;
	double z;
};

struct body {
	struct coord position;
	struct coord force;
	struct coord speed;
	double acel;
	double mass;
};

struct node {
	struct coord start;
	struct coord end;
	int bodies_quantity;
	int initialized;
	int has_center_of_mass;
	int deep;
	struct body **bodies;
	struct node *UNE;
	struct node *USE;
	struct node *USW;
	struct node *UNW;
	struct node *DNE;
	struct node *DSE;
	struct node *DSW;
	struct node *DNW;
	struct node *UP;
	struct body centerOfMass;
};

typedef struct{
        float r;
        float g;
        float b;
} color;

/* A coloured pixel. */

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} pixel_t;

/* A picture. */

typedef struct {
	pixel_t *pixels;
	size_t width;
	size_t height;
} bitmap_t;

struct node *nodes;
struct node **roots;
struct body *bodies;

int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p);
pixel_t * pixel_at(bitmap_t * bitmap, int x, int y);
static int save_png_to_file(bitmap_t * bitmap, const char *path);
void initializeNode(struct node *node, struct node *up, double sx, double sy,
		    double sz, double ex, double ey, double ez,
		    int bodies_quantity, int deep);
void addBodyInNode(struct body *body, struct node *node);
void resetNodes(void);
void divideNode(struct node *node);
void setCenterOfMass(struct node *node);
void applyForceBetweenBodies(struct body *b1, struct body *b2);
int applyForce(struct node *node, struct body *body);
void forceOverNode(struct node *node, struct node *down, struct body *body,
		   int inverse);
void init(void);
void update(int value);

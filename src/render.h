#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#define colorMax 3E4

void GiveRainbowColor(double position, unsigned char * c);
void initRendering();
void handleResize(int w, int h);
void handleKeypress(unsigned char key, int x, int y);
void drawScene();
#endif

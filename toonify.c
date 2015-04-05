#define RUNME /*
# sh toonify.c to compile
gcc -Wall -lgd -lm -o `basename $0 .c` $0
exit $?
*/
// toonify.c - team Crush All Humans - CS Games 2007
// written by atn and pvk

#include <gd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define R(c) ((c&0xff0000)>>16)
#define G(c) ((c&0x00ff00)>> 8)
#define B(c) ((c&0x0000ff)    )
#define MIN(x, y) (((x) < y) ? (x) : (y))

int maxdiff = 4;

int colordiff(int,int);

int main(int argc, char **argv){
  int i, j;

  if(argc < 3){
    puts("usage: toonify <infile> <outfile> [starting threshold]");
    exit(argc != 0);
  }

  if(argc > 3)
    maxdiff = atoi(argv[3]);

  FILE *in = fopen(argv[1], "rb");
  if(!in){
    perror("fopen");
    exit(errno);
  }
  gdImagePtr im = gdImageCreateFromPng(in);
  fclose(in);

  int width = im->sx;
  int height = im->sy;

  // Convert to truecolor if it's a palette image
  if(!im->trueColor){
    gdImagePtr imtc = gdImageCreateTrueColor(im->sx, im->sy);
    gdImageCopy(imtc, im, 0, 0, 0, 0, im->sx, im->sy);
    gdImageDestroy(im);
    im = imtc;
  }

  int **pixel = im->tpixels;

  // create the image we will trace onto
  gdImagePtr im_trace = gdImageCreate(im->sx, im->sy);
  int trans = gdImageColorAllocateAlpha(im_trace,0,0,0,127);
  int black = gdImageColorAllocate(im_trace,0,0,0);

  unsigned char **trace = im_trace->pixels;

  gdImagePtr im_trace_filtered = gdImageCreate(im->sx, im->sy);

  unsigned char **trace_filtered = im_trace_filtered->pixels;

  gdImagePtr rim;

  int num_traced;
  do {
    num_traced = 0;
    // make it transparent
    for(i = 0; i < width; i++){
      for(j = 0; j < height; j++){
	trace[j][i] = trans;
      }
    }

    // trace borders with a custom method
    for(i = 2; i < width-2; i++){
      for(j = 2; j < height-2; j++){
	int tl = pixel[j-2][i-2];
	int tr = pixel[j-2][i+2];
	int bl = pixel[j+2][i-2];
	int br = pixel[j+2][i+2];
	int t = colordiff(tl,tr);
	int b = colordiff(bl,br);
	int l = colordiff(tl,bl);
	int r = colordiff(tr,br);
	switch((t<<3) | (b<<2) | (l<<1) | r){
	case 0x0: break;
	case 0x1: trace[j][i+1] = black; break;
	case 0x2: trace[j][i-1] = black; break;
	case 0x3:
	  trace[j][i-1] = black;
	  trace[j][i] = black;
	  trace[j][i+1] = black; break;
	case 0x4: trace[j+1][i] = black; break;
	case 0x5: trace[j+1][i+1] = black; break;
	case 0x6: trace[j+1][i-1] = black; break;
	case 0x7:
	  trace[j][i-1] = black;
	  trace[j][i+1] = black;
	  trace[j+1][i] = black; break;
	case 0x8: trace[j-1][i] = black; break;
	case 0x9: trace[j-1][i+1] = black; break;
	case 0xa: trace[j-1][i-1] = black; break;
	case 0xb:
	  trace[j][i-1] = black;
	  trace[j][i+1] = black;
	  trace[j-1][i] = black; break;
	case 0xc:
	  trace[j-1][i] = black;
	  trace[j][i] = black;
	  trace[j+1][i] = black; break;
	case 0xd:
	  trace[j+1][i] = black;
	  trace[j-1][i] = black;
	  trace[j][i+1] = black; break;
	case 0xe:
	  trace[j+1][i] = black;
	  trace[j-1][i] = black;
	  trace[j][i-1] = black; break;
	case 0xf:
	  trace[j][i+1] = black;
	  trace[j][i-1] = black;
	  trace[j+1][i] = black;
	  trace[j-1][i] = black;
	  trace[j][i] = black; break;
	}
      }
    }
  
    for(i = 0; i < width; i++) {
      for(j = 0; j < height; j++) {
	trace_filtered[j][i] = trans;
      }
    }
    // Smoothen traces
    for(i = 0; i < width; i++){
      for(j = 0; j < height; j++){
	int nb = 0;
	int total = 0;
	int di, dj;

	for (di = - MIN(1, i); di <= MIN(1, width - i - 1); di++) {
	  for (dj = - MIN(1, j); dj <= MIN(1, height - j - 1); dj++) {
	    if (black == trace[j+dj][i+di]) nb++;
	    total++;
	  }
	}

	if (nb >= total*2/3) {
	  trace_filtered[j][i] = black;
	  num_traced++;
	}
      }
    }
    maxdiff = (int) ceil(1.1 * maxdiff);

  //} while(num_traced > width*height/5); // Completely arbitrary
  }while(0);

  for (i = 0; i < width; i++) {
    double sumR = 0;
    double sumG = 0;
    double sumB = 0;
    int last_j = -1;
    for (j = 0; j < height; j++) {
      sumR += R(pixel[j][i]);
      sumG += G(pixel[j][i]);
      sumB += B(pixel[j][i]);

      if ((black == trace_filtered[j][i]) || (rand() < RAND_MAX/10)) {
	if (last_j != -1) {
	  int num_px = j-last_j;

	  int avgR = (int) (sumR/num_px);
	  int avgG = (int) (sumG/num_px);
	  int avgB = (int) (sumB/num_px);

	  int px = (avgR << 16) | (avgG << 8) | avgB;

	  int j2;

	  for (j2 = last_j; j2 <= j; j2++) pixel[j2][i] = px;
	}

	sumR = sumG = sumB = 0;
	last_j = j;
      }
    }
  }

  for (j = 0; j < height; j++) {
    double sumR = 0;
    double sumG = 0;
    double sumB = 0;
    int last_i = -1;
    for (i = 0; i < width; i++) {
      sumR += R(pixel[j][i]);
      sumG += G(pixel[j][i]);
      sumB += B(pixel[j][i]);

      if ((black == trace_filtered[j][i]) || (rand() < RAND_MAX/10)) {
	if (last_i != -1) {
	  int num_px = i-last_i;

	  int avgR = (int) (sumR/num_px);
	  int avgG = (int) (sumG/num_px);
	  int avgB = (int) (sumB/num_px);

	  int px = (avgR << 16) | (avgG << 8) | avgB;

	  int i2;

	  for (i2 = last_i; i2 <= i; i2++) pixel[j][i2] = px;
	}

	sumR = sumG = sumB = 0;
	last_i = i;
      }
    }
  }

#define BLURDELTA 4

  // shitty blur
  for (i = 0; i < width; i++) {
    for (j = 0; j < height; j++) {
      int sumR = 0;
      int sumG = 0;
      int sumB = 0;
      int num_px = 0;

      int di, dj;

      for (di = 0; di <= MIN(width - i - 1, BLURDELTA); di++) {
	for (dj = - 1; dj >= - MIN(j, BLURDELTA); dj--) {
	  if (black == trace_filtered[j+dj][i+di]) {
	    di = BLURDELTA + 1;
	  } else {
	    sumR += R(pixel[j+dj][i+di]);
	    sumG += G(pixel[j+dj][i+di]);
	    sumB += B(pixel[j+dj][i+di]);
	    num_px++;
	  }
	}

	for (dj = 0; dj <= MIN(height - j - 1, BLURDELTA); dj++) {
	  if (black == trace_filtered[j+dj][i+di]) {
	    di = BLURDELTA + 1;
	  } else {
	    sumR += R(pixel[j+dj][i+di]);
	    sumG += G(pixel[j+dj][i+di]);
	    sumB += B(pixel[j+dj][i+di]);
	    num_px++;
	  }
	}
      }

      for (di = - 1; di >= - MIN(i, BLURDELTA); di--) {
	for (dj = - 1; dj >= - MIN(j, BLURDELTA); dj--) {
	  if (black == trace_filtered[j+dj][i+di]) {
	    di = - BLURDELTA - 1;
	  } else {
	    sumR += R(pixel[j+dj][i+di]);
	    sumG += G(pixel[j+dj][i+di]);
	    sumB += B(pixel[j+dj][i+di]);
	    num_px++;
	  }
	}

	for (dj = 0; dj <= MIN(height - j - 1, BLURDELTA); dj++) {
	  if (black == trace_filtered[j+dj][i+di]) {
	    di = - BLURDELTA - 1;
	  } else {
	    sumR += R(pixel[j+dj][i+di]);
	    sumG += G(pixel[j+dj][i+di]);
	    sumB += B(pixel[j+dj][i+di]);
	    num_px++;
	  }
	}
      }

      if (num_px)
	pixel[j][i] = (((sumR+num_px/2)/num_px) << 16) 
	            | (((sumG+num_px/2)/num_px) << 8) 
                    | ((sumB+num_px/2)/num_px);
    }
  }
  // put borders back over the image
  for(i = 0; i < width; i++){
    for(j = 0; j < height; j++){
      if(trace_filtered[j][i] == black)
	pixel[j][i] = 0;
      else
        pixel[j][i] = 0xffffff;
    }
  }


  FILE *out = fopen(argv[2], "wb");
  gdImagePng(im, out);
  fclose(out);

  return 0;
}

int colordiff(int a, int b){
  // old1  int d = ((R(a)-R(b))*(G(a)-G(b))*(B(a)-B(b)));
  int dr = R(a)-R(b);
  int dg = G(a)-G(b);
  int db = B(a)-B(b);
  return dr * dr + dg * dg + db * db > maxdiff * maxdiff;
  // return abs(dr) + abs(dg) + abs(db) > maxdiff;
}

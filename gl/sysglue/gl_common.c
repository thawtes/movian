/*
 *  Common GL init
 *  Copyright (C) 2007 Andreas �man
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libglw/glw.h>

#include "showtime.h"



/*
 *
 */

static int
check_gl_ext(const uint8_t *s, const char *func, int fail)
{
  int l = strlen(func);
  int found;
  char *v;

  v = strstr((const char *)s, func);
  found = v != NULL && v[l] < 33;
  
  fprintf(stderr, "Checking OpenGL extension \"%s\" : %svailable", func,
	  found ? "A" : "Not a");

  if(!found && fail) {
    fprintf(stderr, ", but is required, exiting\n");
    exit(1);
  }
  fprintf(stderr, "\n");
  return found ? 0 : -1;
}





void
gl_common_init(void)
{
  const	GLubyte	*s;

  frame_duration = 16666; /* 60 Hz, we dont know anything better yet */

  fprintf(stderr, "OpenGL library: %s on %s, version %s\n",
	  glGetString(GL_VENDOR),
	  glGetString(GL_RENDERER),
	  glGetString(GL_VERSION));

  s = glGetString(GL_EXTENSIONS);

  check_gl_ext(s, "GL_ARB_pixel_buffer_object",      1);
  check_gl_ext(s, "GL_ARB_vertex_buffer_object",     1);
  check_gl_ext(s, "GL_ARB_fragment_program",         1);
  check_gl_ext(s, "GL_ARB_texture_non_power_of_two", 1);

  glDisable(GL_CULL_FACE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LINE_SMOOTH);

  glEnable(GL_POLYGON_OFFSET_FILL);
}

static int
intcmp(const void *A, const void *B)
{
  const int *a = A;
  const int *b = B;
  return *a - *b;
}


#define FRAME_DURATION_SAMPLES 31 /* should be an odd number */

void
gl_update_timings(void)
{
  struct timeval tv;
  static int64_t lastts, firstsample;
  static int deltaarray[FRAME_DURATION_SAMPLES];
  static int deltaptr;
  static int lastframedur;
  int d;

  gettimeofday(&tv, NULL);
  wallclock = (int64_t)tv.tv_sec * 1000000LL + tv.tv_usec;
  walltime = tv.tv_sec;
  
  if(lastts != 0) {
    d = wallclock - lastts;
    if(deltaptr == 0)
      firstsample = wallclock;

    deltaarray[deltaptr++] = d;

    if(deltaptr == FRAME_DURATION_SAMPLES) {
      qsort(deltaarray, deltaptr, sizeof(int), intcmp);
      d = deltaarray[FRAME_DURATION_SAMPLES / 2];

      if(lastframedur == 0) {
	lastframedur = d;
      } else {
	lastframedur = (d + lastframedur) / 2;
      }
      frame_duration = lastframedur;
      deltaptr = 0;
    }
  }
  lastts = wallclock;
}

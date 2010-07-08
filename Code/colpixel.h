#ifndef __colpixel_h_
#define __colpixel_h_

#define COLOURED_PIXELS_MAX    2048 * 2048
#include "mpiInclude.h"

// TODO: This should really be a temporary header file that grows to have more common stuff in it.
struct ColPixel
{
  float vel_r, vel_g, vel_b;
  float stress_r, stress_g, stress_b;
  float t, dt;
  float density;
  float stress;
  
  float particle_vel;
  float particle_z;
  
  int particle_inlet_id;
  int i;
};

extern ColPixel col_pixel_send[COLOURED_PIXELS_MAX];
extern ColPixel col_pixel_recv[2][COLOURED_PIXELS_MAX];

#ifndef NOMPI
  extern MPI_Datatype MPI_col_pixel_type;
#endif

#endif // __colpixel_h_
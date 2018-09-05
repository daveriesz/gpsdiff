
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gps.h>

void dsleep(double seconds)
{
  double whole   = floor(seconds);
  double decimal = seconds - whole;
  struct timespec tsp;
  
  tsp.tv_sec = (time_t)whole;
  tsp.tv_nsec = (long)(decimal * (1000000000.0));
  
  nanosleep(&tsp, 0);
}

double vertical_distance(struct gps_data_t *first, struct gps_data_t *second)
{
  return fabs(first->fix.altitude - second->fix.altitude);
}

double horizontal_distance(struct gps_data_t *first, struct gps_data_t *second)
{
  return earth_distance(
            first ->fix.latitude, first ->fix.longitude,
            second->fix.latitude, second->fix.longitude);
}

double position_distance(struct gps_data_t *first, struct gps_data_t *second)
{
  double hh = horizontal_distance(first, second);
  double vv = vertical_distance  (first, second);
  return sqrt((hh*hh) + (vv*vv));
}

int main(int argc, char **argv)
{
  char *gpsd_server_a, *gpsd_server_b;
  int ra, rb;
  struct gps_data_t gda, gdb;
  int exitearly = 0;
  int loop_finished;
  const char *jsa, *jsb;
  double dAlt[3], dPos[3], dBoth[3]; /* 0:current, 1:min, 2:max */
  int mminit = 0;

  memset(&gda, 0, sizeof(struct gps_data_t));
  memset(&gdb, 0, sizeof(struct gps_data_t));

  if(argc != 3)
  {
    fprintf(stderr, "Please supply exactly two GPSD servers.\n");
    exit(1);
  }

  gpsd_server_a = strdup(argv[1]);
  gpsd_server_b = strdup(argv[2]);
  
  ra = gps_open(gpsd_server_a, "2947", &gda);
  rb = gps_open(gpsd_server_b, "2947", &gdb);
  
  if(ra == -1) { fprintf(stderr, "Could not open gpsd server (A): >>%s<<\n", gpsd_server_a); exitearly = 1; }
  if(rb == -1) { fprintf(stderr, "Could not open gpsd server (B): >>%s<<\n", gpsd_server_b); exitearly = 1; }
  if(exitearly) { exit(1); }

  printf("open a returned %d\n", ra);
  printf("open b returned %d\n", rb);

  gps_stream(&gda, WATCH_ENABLE|WATCH_JSON, NULL);
  gps_stream(&gdb, WATCH_ENABLE|WATCH_JSON, NULL);

  for(loop_finished=0 ; loop_finished==0 ; )
  {
    if(gps_waiting(&gda, 500))
    {
      gps_read(&gda);
      jsa = gps_data(&gda);
    }
    if(gps_waiting(&gdb, 500))
    {
      gps_read(&gdb);
      jsb = gps_data(&gdb);
    }

    if(gda.status && gdb.status)
    {
      if(gda.fix.time == gdb.fix.time)
      {
        dAlt [0] = vertical_distance  (&gda, &gdb);
        dPos [0] = horizontal_distance(&gda, &gdb);
        dBoth[0] = position_distance  (&gda, &gdb);
//      dBoth[0] = sqrt((dAlt[0]*dAlt[0]) + (dPos[0]*dPos[0]));

        printf("tm:%.0f", gda.fix.time);
        printf(" dAlt: %.2f(%.2f-%.2f)", dAlt [0],dAlt [1],dAlt [2]);
        printf(" dPos: %.2f(%.2f-%.2f)", dPos [0],dPos [1],dPos [2]);
        printf(" dBoth: %2f(%.2f-%.2f)", dBoth[0],dBoth[1],dBoth[2]);
        printf("\n");
        if(mminit)
        {
          dAlt [1] = (dAlt [0]<dAlt [1])?dAlt [0]:dAlt [1];
          dAlt [2] = (dAlt [0]>dAlt [2])?dAlt [0]:dAlt [2];
          dPos [1] = (dPos [0]<dPos [1])?dPos [0]:dPos [1];
          dPos [2] = (dPos [0]>dPos [2])?dPos [0]:dPos [2];
          dBoth[1] = (dBoth[0]<dBoth[1])?dBoth[0]:dBoth[1];
          dBoth[2] = (dBoth[0]>dBoth[2])?dBoth[0]:dBoth[2];
        }
        else
        {
          dAlt [1] = dAlt [2] = dAlt [0];
          dPos [1] = dPos [2] = dPos [0];
          dBoth[1] = dBoth[2] = dBoth[0];
          mminit = 1;
        }
        
      }
      else
      {
        printf("timestamp mismatch\n");
      }
    }

    dsleep(0.1);
  }

  gps_stream(&gda, WATCH_DISABLE, NULL);
  gps_stream(&gdb, WATCH_DISABLE, NULL);

  gps_close(&gda);
  gps_close(&gdb);

  return 0;
}


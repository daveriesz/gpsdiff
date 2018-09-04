
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gps.h>

void dsleep(double seconds)
{
  double whole   = floor(seconds);
  double decimal = seconds - whole;
  struct timespec tsp;
  
  tsp.tv_sec = (time_t)(floor);
  tsp.tv_nsec = (long)(decimal * (1000000000.0));
  
  nanosleep(&tsp, 0);
}

int main(int argc, char **argv)
{
  char *gpsd_server_a, *gpsd_server_b;
  int ra, rb;
  struct gps_data_t gda, gdb;
  int exitearly = 0;
  int loop_finished;
  const char *jsa, *jsb;

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
    jsa = jsb = NULL;
    if(gps_waiting(&gda, 500))
    {
      gps_read(&gda);
      jsa = gps_data(&gda);
    }
    if(gps_waiting(&gdb, 500))
    {
      gps_read(&gda);
      jsb = gps_data(&gda);
    }

    if(jsa)
    {
      printf("jsa = %s\n", jsa);
    }

    dsleep(0.05);
  }

  gps_stream(&gda, WATCH_DISABLE, NULL);
  gps_stream(&gdb, WATCH_DISABLE, NULL);

  gps_close(&gda);
  gps_close(&gdb);

  return 0;
}


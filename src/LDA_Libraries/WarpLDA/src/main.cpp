#include "warp.hpp"

int main(int argc, char** argv)
{
	char* arg1[] = {(char*)"warplda",
                  (char*)"--prefix", (char*)"train100",
                  (char*)"--k", (char*)"20",
                  (char*)"--niter", (char*)"500",
                   NULL};

  char* arg2[] = {(char*)"warplda",
                  (char*)"--prefix", (char*)"train50",
                  (char*)"--k", (char*)"5",
                  (char*)"--niter", (char*)"300",
                  NULL};

  run_wlda(7, arg2);
  printf("\n\n\nFinished first!\n\n\n");
  run_wlda(7, arg1);

  return 0;
}

#include <graphics.h>

gint				/* changed from void for *nix */
main (void)
{
  int graphdriver = VGA, graphmode = CGAC0;
  initgraph (&graphdriver, &graphmode, NULL);
  getchar ();
  closegraph ();
  getchar ();
  return (0);
}

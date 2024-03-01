/**
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 */

/**
 * This is used to generate the simple swizzle header files.
 *
   gen r > swiz_rgba.hpp
   gen 2 > swiz2.hpp
   gen 3 > swiz3.hpp
   gen > swiz4.hpp
*/
#include <stdio.h>
#include <stdbool.h>

char pick_char(bool xyzw, int idx)
{
	if (xyzw) return idx == 3 ? 'w' : 'x' + idx;
	switch (idx) {
	case 0: return 'r';
	case 1: return 'g';
	case 2: return 'b';
	case 3: return 'a';
	}
}

void print_swiz2(bool xyzw, int x, int y) {
	printf("TRISYCL_GEN_SWIZ2(%c%c, %d, %d)\n",
			pick_char(xyzw, x),
			pick_char(xyzw, y),
			x, y);
}

void print_swiz3(bool xyzw, int x, int y, int z) {
	printf("TRISYCL_GEN_SWIZ3(%c%c%c, %d, %d, %d)\n",
			pick_char(xyzw, x),
			pick_char(xyzw, y),
			pick_char(xyzw, z),
			x, y, z);
}
void print_swiz4(bool xyzw, int x, int y, int z, int w) {
	printf("TRISYCL_GEN_SWIZ4(%c%c%c%c, %d, %d, %d, %d)\n",
			pick_char(xyzw, x),
			pick_char(xyzw, y),
			pick_char(xyzw, z),
			pick_char(xyzw, w),
			x, y, z, w);
}

void do_swiz(bool xyzw, int nswiz) {
	int x, y, z, w;
	if (xyzw) {
	  if (nswiz >= 2)
	  for (x = 0; x < 2; x++)
	    for (y = 0; y < 2; y++)
	      print_swiz2(xyzw, x, y);
	  if (nswiz >= 3)
	  for (x = 0; x < 3; x++)
	    for (y = 0; y < 3; y++)
	      for (z = 0; z < 3; z++)
		print_swiz3(xyzw, x, y, z);
	}
	if (nswiz == 4)
	for (x = 0; x < 4; x++)
	for (y = 0; y < 4; y++)
	for (z = 0; z < 4; z++)
	for (w = 0; w < 4; w++)
		print_swiz4(xyzw, x, y, z, w);
}

int main(int argc, char **argv) {
	bool use_rgba = false;
        int nswiz = 4;
	if (argc != 2)
		return -1;

	if (argv[1][0] == 'r')
		use_rgba = true;
	else if (argv[1][0] == '2')
		nswiz = 2;
	else if (argv[1][0] == '3')
		nswiz = 3;
	printf("// Generated using a simple C generator\n");
	printf("#ifdef SYCL_SIMPLE_SWIZZLES\n");
	do_swiz(!use_rgba, nswiz);
	printf("#endif\n");
}

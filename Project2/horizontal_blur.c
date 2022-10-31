#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define BLUR_AMOUNT 50

struct header {
	int height, width, maxRGB;
};
typedef struct header header_t;

struct pixel {
	double red, green, blue;
};

typedef struct pixel pixel_t;

int main(int argc, char **argv) {

	// Command line argument for filename to open and blur
	char *filename = argv[1];

	header_t head;

	FILE *fp;
	fp = fopen(filename, "r");
	char str[3];
	int	count = 3;
	
	if (strcmp(fgets(str, 3, fp), "P3") != 0)  {
		printf("File corrupted. Missing P3 in header. Exiting.\n");
		exit(EXIT_FAILURE);
	}

	fscanf(fp, "%d %d", &head.width, &head.height);
	fscanf(fp, "%d", &head.maxRGB);

	if (head.maxRGB != 255)  {
		printf("File corrupted. MAX RGB value is not 255. Exiting.\n");
		exit(EXIT_FAILURE);
	}

	printf("header file : height %d, width %d, maxrgb %d\n", head.height, head.width, head.maxRGB);

	pixel_t * pixels = malloc(head.width * head.height * sizeof(pixel_t));
	int i = 0;
	while (fscanf(fp, "%lf %lf %lf", &pixels[i].red, &pixels[i].green, &pixels[i].blue) == 3) {
		i++;
	}

	fclose(fp);

	// printing heade information to a new ppm file
	fp = fopen("_blur.ppm", "w");
	fprintf(fp, "P3\n%d %d\n%d\n", head.width, head.height, head.maxRGB);

	// From top to bottom
	for (int height = 0; height < head.height; height++) {
		// From left to right in a single row
		for (int width = 1; width <= head.width; width++) {

			// Select a pixel struct by current width, then include height to get actual position in struct array
			int pixel = width + (height * head.width) - 1;
			int z;

			// For the current pixel divide its color values by 2
			pixels[pixel].red /= 2;
			pixels[pixel].green /= 2;
			pixels[pixel].blue /= 2;

			// set the remaining pixels to include in blur to BLUR_AMOUNT, or number less than BLUR_AMOUNT
			double r = head.width - width < BLUR_AMOUNT ? head.width - width : BLUR_AMOUNT;

			// factor in the weight of the remaining pixels
			// This is when the problem occurs when I consider edge pixels rather then just full outright using BLUR_AMOUNT
			for (z = 1; z <= r; z++) {
				pixels[pixel].red += pixels[pixel+z].red * (0.5 / r);
				pixels[pixel].green += pixels[pixel+z].green * (0.5 / r);
				pixels[pixel].blue += pixels[pixel+z].blue * (0.5 / r);
			}
		}
	}

	// Print to the blur .ppm file the new pixel values
	for (i = 0; i < head.width * head.height; i++) {
		fprintf(fp, "%d %d %d ", (int) pixels[i].red, (int) pixels[i].green, (int) pixels[i].blue); 
	}


	return 0;
}

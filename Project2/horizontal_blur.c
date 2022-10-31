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

	// Check to see if user specified two command line arguments
	if(argv[1] == NULL || argv[2] == NULL) {
		fprintf(stderr, "Specify command line arguments as ./a.out [input ppm file] [output ppm file].\n");
		exit(EXIT_FAILURE);
	}

	char *input_name = argv[1];
	char *output_name = argv[2];
	FILE *fp;

	// Check to see if the input file exists in directory
	if ((fp = fopen(input_name, "r")) == NULL) {
		fprintf(stderr, "Error: %s does not exist in directory.\n", input_name);
		exit(EXIT_FAILURE);
	}

	header_t head;
	char str[3];
	int	count = 3;
	
	// Exit when there is no magic number matching P3
	if (strcmp(fgets(str, 3, fp), "P3") != 0)  {
		fprintf(stderr, "File corrupted. Missing P3 in header. Exiting.\n");
		exit(EXIT_FAILURE);
	}

	fscanf(fp, "%d %d", &head.width, &head.height);
	fscanf(fp, "%d", &head.maxRGB);

	// Exit when MAX RGB is not 255.
	if (head.maxRGB != 255)  {
		printf("File corrupted. MAX RGB value is not 255. Exiting.\n");
		exit(EXIT_FAILURE);
	}

	pixel_t * pixels = malloc(head.width * head.height * sizeof(pixel_t));
	int i = 0;
	while (fscanf(fp, "%lf %lf %lf", &pixels[i].red, &pixels[i].green, &pixels[i].blue) == 3) {
		i++;
	}

	fclose(fp);

	// printing heade information to a new ppm file
	fp = fopen(output_name, "w");
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
		fprintf(fp, "%d %d %d\t", (int) pixels[i].red, (int) pixels[i].green, (int) pixels[i].blue); 
		if (i % 10 == 0) {
			fprintf(fp, "\n");
		}
	}


	return 0;
}

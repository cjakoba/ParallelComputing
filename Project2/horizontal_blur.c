#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
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

// Divides color values by 2 and increments them by the remaining pixels color values in front of them
// until reaching the BLUR_AMOUNT, or hitting edge of the image, whichever comes first.
void Blur(pixel_t * pixels, int max_height, int max_width) {
	// From top to bottom
	for (int height = 0; height < max_height; height++) {
		// From left to right in a single row
		for (int width = 1; width <= max_width; width++) {

			// Select a pixel struct by current width, then include height to get actual position in struct array
			int pixel = width + (height * max_width) - 1;
			int z;

			// For the current pixel divide its color values by 2
			pixels[pixel].red /= 2;
			pixels[pixel].green /= 2;
			pixels[pixel].blue /= 2;

			// set the remaining pixels to include in blur to BLUR_AMOUNT, or number less than BLUR_AMOUNT
			double r = max_width - width < BLUR_AMOUNT ? max_width - width : BLUR_AMOUNT;

			// factor in the weight of the remaining pixels
			// This is when the problem occurs when I consider edge pixels rather then just full outright using BLUR_AMOUNT
			for (z = 1; z <= r; z++) {
				pixels[pixel].red += pixels[pixel+z].red * (0.5 / r);
				pixels[pixel].green += pixels[pixel+z].green * (0.5 / r);
				pixels[pixel].blue += pixels[pixel+z].blue * (0.5 / r);
			}
		}
	}
}


int main(int argc, char **argv) {
	struct timeval current;

	// Check to see if user specified two command line arguments
	if(argv[1] == NULL || argv[2] == NULL) {
		fprintf(stderr, "Specify command line arguments as ./a.out [input ppm file] [output ppm file].\n");
		exit(EXIT_FAILURE);
	}

	char *input_name = argv[1];
	char *output_name = argv[2];
	FILE *fp;

	gettimeofday(&current, NULL);
	unsigned long ms_start = (current.tv_sec * 1000) + (current.tv_usec / 1000);

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

	gettimeofday(&current, NULL);
	unsigned long ms_end = (current.tv_sec * 1000) + (current.tv_usec / 1000);
	printf("File loading took %ld ms.\n", ms_end - ms_start);


	// printing heade information to a new ppm file
	fp = fopen(output_name, "w");
	fprintf(fp, "P3\n%d %d\n%d\n", head.width, head.height, head.maxRGB);
	
	gettimeofday(&current, NULL);
	ms_start = (current.tv_sec * 1000) + (current.tv_usec / 1000);
	Blur(pixels, head.height, head.width);
	gettimeofday(&current, NULL);
	ms_end = (current.tv_sec * 1000) + (current.tv_usec / 1000);
	printf("Blur function took %ld ms.\n", ms_end - ms_start);

	gettimeofday(&current, NULL);
	ms_start = (current.tv_sec * 1000) + (current.tv_usec / 1000);
	
	// Print to the blur .ppm file the new pixel values
	for (i = 0; i < head.width * head.height; i++) {
		fprintf(fp, "%d %d %d\t", (int) pixels[i].red, (int) pixels[i].green, (int) pixels[i].blue); 
		if (i % 10 == 0) {
			fprintf(fp, "\n");
		}
	}

	gettimeofday(&current, NULL);
	ms_end = (current.tv_sec * 1000) + (current.tv_usec / 1000);
	printf("File saving took %ld ms.\n", ms_end - ms_start);
	
	return 0;
}

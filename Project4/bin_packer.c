#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define POP_SIZE 200
#define END_GEN 1000

// Applies greedy algorithm for determining number of bins for
// current array elelement arrangement sequentially
int greedy_bins(int *array, int size, int max_weight) {
	int bins = 1;
	int weight = 0;
	int j = 0;

	for (int i = 0; i < size; i++) {
		if (weight + array[i] > max_weight) {
			bins++;
			weight = 0;
			weight += array[i];
		} else {
			weight += array[i];
		}
	}
	return bins;
}

// Prints out an array of integers
void printArray(int *array, int size, int max_weight) {
	for (int i = 0; i < size; i++) {
		printf("%d, ", array[i]);
	}
	printf(" bins=%d\n", greedy_bins(array, size, max_weight));
}

// Prints out a 2D array of integers
void print2DArray(int **array, int size, int max_weight) {
	for (int i = 0; i < POP_SIZE; i++) {
		printArray(array[i], size, max_weight);
	}
	printf("\n");
}

// Employs the modern Fisher-Yates shuffle algorithm to shuffle an 
// array of integers with all permutations equally likely.
void shuffle(int *array, int size) {
	srand(time(NULL));
	int j;
	int temp;
	for (int i = size-1; i >= 0; i--) {
		j = rand() % (i + 1); //0 <= j <= i
	  	temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

int *crossover(int *chromosomeA, int *chromosomeB, int size) {
	srand(time(NULL));
	int *offspring = malloc(size * sizeof(int));
	int index = rand() % (size + 1);
	for (int i = 0; i < index; i++) {
		offspring[i] = chromosomeA[i];
	}
	for (int i = index; i < size; i++) {
		offspring[i] = chromosomeB[i];
	}
	printf("Crossover index start=%d (noninclusive)\n", index);
	return offspring;
}

int *most_fit(int **population, int size, int max_weight, int max_search) {
	int *best_indexes = malloc(max_search * sizeof(int));
	int first_best = INT_MAX;
	int first_done = 0;
	int first_index = -1;
	int second_best = INT_MAX;
	int second_index = -1;

	for (int top = 0; top < max_search; top++) {
		int best_bin = -1;
		for (int chromosome = 0; chromosome < POP_SIZE; chromosome++) {
			best_bin = greedy_bins(population[chromosome], size, max_weight);
			if (first_done == 0 && best_bin < first_best) {
				first_best = best_bin;
				first_index = chromosome;
			} else if (first_done == 1 && best_bin < second_best && chromosome != first_index) {
				second_best = best_bin;
				second_index = chromosome;
			}
		}
		first_done = 1;
	}	
	best_indexes[0] = first_index;
	best_indexes[1] = second_index;
	printf("Best fitness=%d, at index %d.\n", first_best, first_index);
	printf("Second fitness=%d, at index %d.\n", second_best, second_index);
	return best_indexes;
}

int least_fit(int **population, int size, int max_weight) {
	int worst_index = -1;
	int worst_fitness = -1;
	for (int chromosome = 0; chromosome < POP_SIZE; chromosome++) {
		int worst_bin = greedy_bins(population[chromosome], size, max_weight);
		if (worst_bin > worst_fitness) {
			worst_fitness = worst_bin;
			worst_index = chromosome;
		}
	}
	printf("The worst fitness is %d, at index %d.\n", worst_fitness, worst_index);
	return worst_index;
}

int main(int argc, char** argv) {
	char *file_name = argv[1];

	int max_weight = -1; // Max bin weight
	int size = -1; // Number of items to organize
	

	// File handling
	FILE *fp;
	if ((fp = fopen(file_name, "r")) == NULL) {
		printf("File not found.");
		exit(EXIT_FAILURE);
	}

	fscanf(fp, "%d", &max_weight); // Get the max bin weight from file
	fscanf(fp, "%d", &size); // Get the number of items to organize from file
	
	printf("Total items=%d\n", size);
	int items[size]; // All item weights to be organized
	
	// Gets all items from the file and stores into array
	for (int i = 0; i < size; i++) {
		fscanf(fp, "%d", &items[i]);
	}

	// Allocating 2D array to hold multiple chromosomes
	int **population = malloc(POP_SIZE * sizeof(int *));
	for (int i = 0; i < POP_SIZE; i++) {
		population[i] = malloc(size * sizeof(int));
	}

	// Copying over shuffled chromosomes into population
	for (int i = 0; i < POP_SIZE; i++) {
		for (int j = 0; j < size; j++) {
			population[i][j] = items[j];
		}
		shuffle(items, size);
	}

	// Finding top 2 best fit indexes
	
	int best_packer = INT_MAX;
	int end_generation = 100;
	for (int generation = 0; generation < END_GEN; generation++) {
		for (int i = 0; i < POP_SIZE; i++) {
			int fitness = greedy_bins(population[i], size, max_weight);
			if (fitness < best_packer) {
				best_packer = fitness;
			}
			int *best_indexes = most_fit(population, size, max_weight, 2);
			int worst_index = least_fit(population, size, max_weight);

			print2DArray(population, size, max_weight);
			printf("offspring generated: ");
			population[worst_index] = crossover(population[best_indexes[0]], population[best_indexes[1]], size); 

		}
	}
	printf("Best Packer was: %d\n", best_packer);


	// TODO: 
	// Generate new offspring from crossover replacing least fit chromosomes
	// add new offspring to population
	
	// Genetic algo
	// get fitness of each member of population
	// eliminate least fit members
	// new members created by modifying members of population
	// 		created by randomly mutating sequences with lowest bin count
	// up to you to determine how many seq in population
	// What percentage die each iteration
	// what percentage of the best solutions are new members based on
	// How exactly are new members mutated
	// Are entirely new random sequences introduced? and how often?
	// When does this process stop?


	return 0;
}

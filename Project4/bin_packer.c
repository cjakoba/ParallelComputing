#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#define POP_SIZE 100 // number of chromosomes in a population
#define END_GEN 1000000
#define MUTATION_PROB 0.05
#define NO_IMPROVEMENT 10000

int size; // number of genes in a chromosome
int max_weight;

// Returns a random integer from min to max-1 with uniform distribution
// Any number between 0 and max (inclusive) is likely to occur.
int random_num(int max) {
	//old implementation
	//int random = ((double) rand() / (RAND_MAX + 1.0)) * (max - min) + min; // min <= return <= max
	//return random;
	
	unsigned long num_bins = (unsigned long) max + 1;
	unsigned long num_rand = (unsigned long) RAND_MAX + 1;
	unsigned long bin_size = num_rand / num_bins;
	unsigned long defect = num_rand % num_bins;
	long x;
	do {
		x = random();
	} while (num_rand - defect <= (unsigned long)x);

	return x/bin_size;
}

// Swap two elements from an array of ints
void swap (int *array, int a, int b) {
	int temp;
	temp = array[a];
	array[a] = array[b];
	array[b] = temp;
}

// Copies over a 1D array into a 2D array
void copyInto2DArray(int *chromosome, int *gene, int size) {
	for (int i = 0; i < size; i++) {
		chromosome[i] = gene[i];
	}
}

// Prints out an array of integers
void printArray(int *array, int size) {
	for (int i = 0; i < size; i++) {
		printf("%d, ", array[i]);
	}
}

// Prints out a 2D array of integers
void print2DArray(int **array, int size) {
	for (int i = 0; i < POP_SIZE; i++) {
		printArray(array[i], size);
		printf("\n");
	}
}

// Applies greedy algorithm for determining fitness (number of bins) for
// current array elelement arrangement sequentially
int fitness(int *array, int size, int max_weight) {
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

// Employs the modern Fisher-Yates shuffle algorithm to shuffle an 
// array of integers in-place with all permutations equally likely.
void shuffle(int *array, int size) {
	// Select random index [0, size-1] to swap with i
	for (int i = size-1; i > 0; i--) {
		swap(array, i, random_num(i)); // swap i and j elements of the array in-place
	}
}

int **crossover(int *chromosomeA, int *chromosomeB, int size) {
	// Allocating 2D offspring array
	int **offspring = malloc(2 * sizeof(int *));
	for (int i = 0; i < 2; i++) {
		offspring[i] = malloc(size * sizeof(int));
	}
	

	// Generate random crossover point
	int index = 1 + random_num(size-1 - 1);
	//printf("Crossover point: %d\n", index);
	
	int seen_numbers_B[200];
	int seen_numbers_A[200];

	int head_numbers_A[200];
	int head_numbers_B[200];


	// Set heads of arrays
	for (int i = 0; i < index; i++) {
		offspring[0][i] = chromosomeA[i]; // Head from A - takes tail from B
		offspring[1][i] = chromosomeB[i]; // Head from B - takes tail from A
	}

	// Create associative arrays for numbers
	for (int i = 0; i < 200; i++) {
		seen_numbers_A[i] = 0;
		head_numbers_A[i] = 0;

		seen_numbers_B[i] = 0;
		head_numbers_B[i] = 0;
	}

	// Count all the number apperances in ChromosomeB's entire array
	for (int i = 0; i < size; i++) {
		seen_numbers_A[chromosomeA[i]]++; // Takes tail from B
		//printf("chromosomeA[i] = %d, seen_numbers_A[chromosomeA[i]] = %d\n", chromosomeA[i], seen_numbers_A[chromosomeA[i]]);
		seen_numbers_B[chromosomeB[i]]++; // Takes tail from A
		//printf("chromosomeB[i] = %d, seen_numbers_B[chromosomeB[i]] = %d\n", chromosomeB[i], seen_numbers_B[chromosomeB[i]]);
	}

	// Count all the number of appearances in Chromosome's head of array
	for (int i = 0; i < index; i++) {
		head_numbers_A[chromosomeA[i]]++;
		head_numbers_B[chromosomeB[i]]++;
	}

	// Decrement count of numbers from ChromosomeA's head of array from entirety of B's array
	for (int i = 0; i < size; i++) {
		// subtract head numbers from seen to get available numbers for tail
		//printf("chromosomeB[i] = %d seen_numbers_A[chromosomeB[i]] = %d - head_numbers_B[chromosomeB[i]] %d = %d\n", chromosomeB[i], seen_numbers_A[chromosomeB[i]], head_numbers_B[chromosomeB[i]], seen_numbers_A[chromosomeB[i]] - head_numbers_B[chromosomeB[i]]);
		if (head_numbers_B[chromosomeB[i]] > 0) {
			seen_numbers_A[chromosomeB[i]]--; // takes tail from B
			head_numbers_B[chromosomeB[i]]--;
		}
		if (head_numbers_A[chromosomeA[i]] > 0) {
			seen_numbers_B[chromosomeA[i]]--; // takes tail from B
			head_numbers_A[chromosomeA[i]]--;
		}
	}

	// Append to tail of new offspring the remaining available numbers
	int j = index;
	int k = index;

	// Increment through the entire chromosome
	for (int i = 0; i < size; i++) {

		// Checks if that number is still available for addition into offspring tail
		// Takes tail from B, Head from A
		if (seen_numbers_B[chromosomeB[i]] > 0) {
			offspring[0][j++] = chromosomeB[i];
			seen_numbers_B[chromosomeB[i]]--;
		}
	}

	for (int i = 0; i < size; i++) {

		// Takes tail from A, Head from B
		if (seen_numbers_A[chromosomeA[i]] > 0) {
			offspring[1][k++] = chromosomeA[i];
			seen_numbers_A[chromosomeA[i]]--;
		}
	}

	//printf("Crossover index ends=%d (noninclusive)\n", index);
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
			best_bin = fitness(population[chromosome], size, max_weight);
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
	//printf("Best fitness=%d, at index %d.\n", first_best, first_index);
	//printf("Second fitness=%d, at index %d.\n", second_best, second_index);
	return best_indexes;
}

int least_fit(int **population, int size, int max_weight) {
	int worst_index = -1;
	int worst_fitness = -1;
	for (int chromosome = 0; chromosome < POP_SIZE; chromosome++) {
		int worst_bin = fitness(population[chromosome], size, max_weight);
		if (worst_bin > worst_fitness) {
			worst_fitness = worst_bin;
			worst_index = chromosome;
		}
	}
	//printf("The worst fitness is %d, at index %d.\n", worst_fitness, worst_index);
	return worst_index;
}

// Compares fitness of all chromosomes in a population (for qsort).
int compare_fitness(const void *p, const void *q) {
	int **l = (int **) p;
	int **r = (int **) q; 
	int left = fitness(*l, size, max_weight);
	int right = fitness(*r, size, max_weight);
	if (left > right) {
		return 1;
	}
	if (left < right) {
		return -1;
	}
	return 0;
}

// Robert Jenkins' 96 bit Mix Function
unsigned long mix(unsigned long a, unsigned long b, unsigned long c) {
 	a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

// Want to make sure you don't choose the same individual twice
int *tournament_selection(int **population, int size, int max_weight) {
	int index_a, index_b;
	
	// Chose two random members from population of size POP_SIZE
	index_a = random_num(POP_SIZE-1);
	do {
		index_b = random_num(POP_SIZE-1);
	} while (index_b == index_a);

	int fitness_a = fitness(population[index_a], size, max_weight);
	int fitness_b = fitness(population[index_b], size, max_weight);

	// Diagnostic information
	//printf("Population: \n");
	//print2DArray(population, size);
	//printf("\n");

	//printf("Comparing index %d |", index_a);
	//printArray(population[index_a], size);
	//printf("with index %d |", index_b);
	//printArray(population[index_b], size);
	//printf("\n");

	

	// Diagnostic information for random indexes
	//printf("index_a = %d\n", index_a);
	//printf("fitness_a = %d\n", fitness_a);
	//printArray(population[index_a], size);
	//printf("------\n");
	//printf("index_b = %d\n", index_b);
	//printf("fitness_b = %d\n", fitness_b);
	//printArray(population[index_b], size);
	//printf("------\n");

	if (fitness_a <= fitness_b) {
		return population[index_a];
	}
	else {
		return population[index_b];
	}
}

// Returns two array elements (genes) unique to eachother index-wise
// Possibility that the actual values be the same
// TODO: Might need to also return new random gene if values are the same
int *get_unique_genes(int size) {
	int *genes = malloc(2 * sizeof(int));
	genes[0] = random_num(size-1);

	do {
		genes[1] = random_num(size-1);
	} while (genes[0] == genes[1]);

	return genes;
}

// Mutates a chromosome in-place, swapping two random indexes
void swap_mutate(int *chromosome, int size) {
	int *genes = get_unique_genes(size);
	swap(chromosome, genes[0], genes[1]);
}

void mutate(int *chromosome, int size) {
	swap_mutate(chromosome, size);
}

int main(int argc, char** argv) {
	unsigned long seed = mix(clock(), time(NULL), getpid());
	// Seed only once at the top of the program
	// If you reseed too quickly during the same second, you will end up getting the same numbers
	srand(seed);
	srandom(seed);
	char *file_name = argv[1];

	max_weight = -1; // Max bin weight
	size = -1; // Number of items to organize
	

	// File handling
	FILE *fp;
	if ((fp = fopen(file_name, "r")) == NULL) {
		printf("File not found.");
		exit(EXIT_FAILURE);
	}

	fscanf(fp, "%d", &max_weight); // Get the max bin weight from file
	fscanf(fp, "%d", &size); // Get the number of items to organize from file
	
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

	int **total_population = malloc((2 * POP_SIZE) * sizeof(int *));
	for (int i = 0; i < (2 * POP_SIZE); i++) {
		total_population[i] = malloc(size * sizeof(int));
	}
		
	// Generate first random population
	for (int i = 0; i < POP_SIZE; i++) {
		// Shuffle original item order in-place and copy into population array multiple times
		shuffle(items, size);
		copyInto2DArray(population[i], items, size);
	}
	
	int best = INT_MAX;

	// Convergence variables
	int no_improvement = 0;
	int previous_fitness = INT_MAX;

	// Generational loop
	for (int gen = 0; gen < END_GEN && no_improvement <= NO_IMPROVEMENT; gen++) {
		
		// Selection - Choice which chromosomes move on to the next generation
		// Can currently select the same chromosome twice
		for (int i = 0; i < POP_SIZE; i++) {
			copyInto2DArray(total_population[i], population[i], size);
		}
		for (int i = 1; i <= POP_SIZE; i+=2) {
			//printf("WINNER1:\n");
			int *winner = tournament_selection(population, size, max_weight);
			//printArray(winner, size);

			
			//printf("WINNER2:\n");
			int *winner2= tournament_selection(population, size, max_weight);
			//printArray(winner2, size);

			// Crossover - kind of buggy
			//printf("OFFSPRING:\n");
			int **offspring = crossover(winner, winner2, size);

			// Mutation of offspring - swap is working
			double mutate_A = rand() / (double) RAND_MAX;
			double mutate_B = rand() / (double) RAND_MAX;

			if (mutate_A <= MUTATION_PROB) {
				mutate(offspring[0], size);
			}
			
			if (mutate_B <= MUTATION_PROB) {
				mutate(offspring[1], size);
			}

			//for (int i = 0; i < 2; i++) { printArray(offspring[i], size); }

			copyInto2DArray(total_population[POP_SIZE * 2 - i], offspring[0], size);
			copyInto2DArray(total_population[POP_SIZE * 2 - (i + 1)], offspring[1], size);
		}
		

		// After crossover and mutation, add to population
		
		//for (int i = 0; i < 2 * POP_SIZE; i++) {
			//printArray(total_population[i], size);
		//}
		//printf("\n");
		//printf("Before sorting... ---------------------\n");
		//for (int i = 0; i < 2 * POP_SIZE; i++) {
			//printf("%d, ", fitness(total_population[i], size, max_weight));
		//}
		//printf("\n");
		
		// Sort by most fit and use those
		qsort(total_population, 2 * POP_SIZE, sizeof(total_population[0]), compare_fitness);
		//printf("After sorting... ---------------------\n");
		//for (int i = 0; i < 2 * POP_SIZE; i++) {
			//printf("%d, ", fitness(total_population[i], size, max_weight));
		//}
		//printf("\n");

		// After sort, keep the top POP_SIZE chromosomes
		for (int i = 0; i < POP_SIZE; i++) {
			copyInto2DArray(population[i], total_population[i], size);
		}

		//print2DArray(population, size);
		if (fitness(population[0], size, max_weight) <= best) {
			best = fitness(population[0], size, max_weight);
		}
		
		// Convergence detection
		if (best == previous_fitness) {
			no_improvement++;
		} else {
			no_improvement = 0;
		}
		previous_fitness = best;
	}
	
	printf("Most fit found: %d\n", fitness(population[0], size, max_weight));

	return 0;
}

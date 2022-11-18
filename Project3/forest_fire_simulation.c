#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define ANSI_COLOR_GREEN "\x1b[30;42m"
#define ANSI_COLOR_YELLOW "\x1b[33;41m"
#define ANSI_COLOR_BLACK "\x1b[100m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define ROWS 40
#define COLUMNS 80

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	srand(time(NULL) + rank);

	// Ensures the user specifies all of the arguments required to make the program functional
	for (int i = 1; i <= 4; i++) {
		if (argv[i] == NULL) {
			fprintf(stderr, "Specify command line arguments as ./a.out [input grid] [generations] [ignition probability] [growth probability]\n");
			exit(EXIT_FAILURE);
		}
	}

	char *input_file = argv[1];
	int generations = atoi(argv[2]);
	double ignition_prob = atof(argv[3]);
	double growth_prob = atof(argv[4]);

	// 2D array of characters to represent grid.
	char grid[ROWS][COLUMNS+1];
	char previous_grid[ROWS][COLUMNS+1];


	FILE *fp;
	if ((fp = fopen(input_file, "r")) == NULL) {
		fprintf(stderr, "Error: %s does not exist in directory.\n", input_file);
		exit(EXIT_FAILURE);
	}

	// Reads in rows and columns 
	for (int row = 0; row < ROWS; row++) {
		for (int column = 0; column < COLUMNS+1; column++) {
			fscanf(fp, "%c", &grid[row][column]);
			previous_grid[row][column] = grid[row][column];
		}
	}

	fclose(fp);


    int animated; // Simulation is animated by default
    if (rank == 0) {
        // Ask user for animated or static output of simulation
        printf("Animated simulation (1), or Static simulation (0): ");
        fflush(stdout);
        scanf("%d", &animated);

        // Clear any previous CLI output
        system("clear");
	}

    char row_top[COLUMNS];
    char row_bottom[COLUMNS];
    char proc_row_top[COLUMNS];
    char proc_row_bottom[COLUMNS];
    char master_grid[ROWS][COLUMNS];
    char *q = malloc(sizeof(char));
    char *c = malloc(sizeof(char));

	// Prints back rows and columns
	for (int current_gen = 0; current_gen <= generations; current_gen++) {
        MPI_Status status;
        MPI_Request request;
        
        // SEND INITAL GRID STATES TO OTHER PROCS
        // Send bottom row data to proc below when not the last process
        if (rank < size - 1) {
            // Store current proc's current bottom row information
            for (int column = 0; column < COLUMNS; column++) {
                row_bottom[column] = grid[rank * (ROWS/size) + (ROWS/size)][column];
            }
            MPI_Isend(row_bottom, COLUMNS, MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD, &request);
        }

        // Send top row data to proc above when not the only, or first process
        if (rank != 0) {
            // Store current proc's current top row information
            for (int column = 0; column < COLUMNS; column++) { row_top[column] = grid[rank * (ROWS/size)][column];
            } 
            MPI_Isend(row_top, COLUMNS, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, &request);
        }

        // RECEIVE INITAL GRID STATES TO OTHER PROCS
        // Send bottom row data to proc below when not the last process
        if (rank < size - 1) {
            // Store current proc's current bottom row information
            MPI_Recv(proc_row_bottom, COLUMNS, MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Wait(&request, MPI_STATUS_IGNORE);
            
            for (int column = 0; column < COLUMNS; column++) {
                grid[(rank + 1) * (ROWS/size)][column] = proc_row_bottom[column];
            }
        }
        // Send top row data to proc above when not the only, or first process
        if (rank != 0) {
            // Store current proc's current top row information
            MPI_Recv(proc_row_top, COLUMNS, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Wait(&request, MPI_STATUS_IGNORE);

            for (int column = 0; column < COLUMNS; column++) {
                grid[rank * (ROWS/size)][column] = proc_row_top[column];
            }
        }
        // MASTER PROCESS RECEIVING OTHER PROCESSES ENTIRE SECTIONS AFTER SIMULATION
        // Send data from processes other than master to master
        if (rank != 0) {
            // Replace whats in master proc's grid with that of other proc's info
            for (int row = rank * (ROWS/size); row < rank * (ROWS/size) + (ROWS/size); row++) {
                for (int column = 0; column < COLUMNS; column++) {
                    *q = grid[row][column];
                    MPI_Send(q, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                }
            }
        }

        // Clear CLI before outputting grid
        if (rank == 0) {
            printf("\x1b[H");
        }
        if (rank == 0) {
            printf("-------------------------------------------------------------------------------------\n");
            printf("   %s : Generation %d / %d\n", input_file, current_gen, generations);
            printf("-------------------------------------------------------------------------------------\n");
            for (int row = 0; row < rank * (ROWS/size) + (ROWS/size); row++) {
                for (int column = 0; column < COLUMNS; column++) {
                    char tile = grid[row][column];
                    if (tile == 'T') {
                        printf(ANSI_COLOR_GREEN "%c" ANSI_COLOR_RESET, tile);	
                    } 
                    else if (tile == 'X') {
                        printf(ANSI_COLOR_YELLOW "%c" ANSI_COLOR_RESET, tile);	
                    }
                    else {
                        printf(ANSI_COLOR_BLACK "%c" ANSI_COLOR_RESET, tile);	
                    }
                }
                printf("\n");
            }

            // Receive from all processes starting at 1
            for (int i = 1; i < size; i++) {
                // Its section of the grid
                for (int row = 0; row < ROWS/size; row++) {
                    for (int column = 0; column < COLUMNS; column++) {
                        MPI_Recv(c, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        if (*c == 'T') {
                            printf(ANSI_COLOR_GREEN "%c" ANSI_COLOR_RESET, *c);	
                        } 
                        else if (*c == 'X') {
                            printf(ANSI_COLOR_YELLOW "%c" ANSI_COLOR_RESET, *c);	
                        } else {
                            printf(ANSI_COLOR_BLACK "%c" ANSI_COLOR_RESET, *c);	
                        }
                    }
                    printf("\n");
                }
            }
        }
		// clear fire (X) for new generations
		for (int row = 0; row < ROWS; row++) {
            for (int column = 0; column < COLUMNS; column++) {
				previous_grid[row][column] = grid[row][column];
				// remove all X from field
				if (previous_grid[row][column] == 'X') {
					previous_grid[row][column] = ' ';
				}
			}
		}

        // MAIN LOGIC AND EDGE DECTION
        // Each process takes over separate rows according to its rank
		for (int row = 0; row < ROWS; row++) {
			double prob;
			double tree_prob;

			for (int column = 0; column < COLUMNS; column++) {
				prob = (double) rand() / RAND_MAX;
				tree_prob = (double) rand() / RAND_MAX;

				char neighbors[8]; 	
				char current = grid[row][column];
				int tree_count = 0;
				
				// Top left corner 
				if (row == 0 && column == 0) {
					neighbors[0] = grid[row][column+1]; // right
					neighbors[1] = grid[row+1][column]; // bottom
					neighbors[2] = grid[row+1][column+1]; // bottom right
				}
				// Top right corner
				else if (row == 0 && column == 80) {
					neighbors[0] = grid[row][column-1]; // left
					neighbors[1] = grid[row+1][column]; // bottom
					neighbors[2] = grid[row+1][column-1]; // bottom left
				}
				// Bottom left corner
				else if (row == 39 && column == 0) {
					neighbors[0] = grid[row-1][column]; // top
					neighbors[1] = grid[row-1][column+1]; // top right
					neighbors[2] = grid[row][column+1]; // right
				}
				// Bottom right corner
				else if (row == 39 && column == 80) {
					neighbors[0] = grid[row-1][column]; // top
					neighbors[1] = grid[row-1][column-1]; // top left
					neighbors[2] = grid[row][column-1]; // left
				}
				// Top row
				else if (row == 0) {
					neighbors[0] = grid[row][column+1]; // right
					neighbors[1] = grid[row+1][column+1]; // bottom right
					neighbors[2] = grid[row+1][column]; // bottom
					neighbors[3] = grid[row+1][column-1]; // bottom left
					neighbors[4] = grid[row][column-1]; // left
				}
				// Bottom row
				else if (row == 39) {
					neighbors[0] = grid[row][column+1]; // right
					neighbors[1] = grid[row-1][column]; // top
					neighbors[2] = grid[row-1][column+1]; // top right
					neighbors[3] = grid[row-1][column-1]; // top left
					neighbors[4] = grid[row][column-1]; // left
				}
				// Left column
				else if (column == 0) {
					neighbors[0] = grid[row-1][column]; // top
					neighbors[1] = grid[row-1][column+1]; // top right
					neighbors[2] = grid[row][column+1]; // right
					neighbors[3] = grid[row+1][column+1]; // bottom right
					neighbors[4] = grid[row+1][column]; // bottom
				}
				// Right column
				else if (column == 80) {
					neighbors[0] = grid[row-1][column]; // top
					neighbors[1] = grid[row-1][column-1]; // top left
					neighbors[2] = grid[row][column-1]; // left
					neighbors[3] = grid[row+1][column-1]; // bottom left
					neighbors[4] = grid[row+1][column]; // bottom
				}
				// Inner tiles
				else {
					neighbors[0] = grid[row-1][column]; // top
					neighbors[1] = grid[row-1][column+1]; // top right
					neighbors[2] = grid[row][column+1]; // right
					neighbors[3] = grid[row+1][column+1]; // bottom right
					neighbors[4] = grid[row+1][column]; // bottom
					neighbors[5] = grid[row+1][column-1]; // bottom left
					neighbors[6] = grid[row][column-1]; // left
					neighbors[7] = grid[row-1][column-1]; // top left
				}
				// Checks all neighbors for 
				for (int i = 0; i < 8; i++) {
					// Counts number of neighboring trees
					if (neighbors[i] == 'T') {
						tree_count++;
					}
					// Sets tree on fire if any neighboring tree is on fire
					if (current == 'T' && neighbors[i] == 'X') {
						previous_grid[row][column] = 'X';
					}
				}

				// When tile is empty, it may grow a tree based on surrounding number of trees
				if (grid[row][column] == ' ' && tree_prob <= (growth_prob * (tree_count + 1))) {
					previous_grid[row][column] = 'T';
					
				}
				// When tile is a tree, it may ignite by lightning 
				else if (grid[row][column] == 'T' && prob <= ignition_prob) {
					previous_grid[row][column] = 'X';
				}
			}
		}

        if (rank == 0) {
            // Show output for a second before clearing for animated look
            if (animated == 1) {
                sleep(1);
            } else {
                printf("-------------------------------------------------------------------------------------\n");
            }
        }

		// Copy the current grid into the previous grid
        for (int row = 0; row < ROWS; row++) {
            for (int column = 0; column < COLUMNS; column++) {
                grid[row][column] = previous_grid[row][column];
            }
        }

	} // End of generational loop

    if (rank == 0) {
        if ((fp = fopen("FOREST_FIRE_RESULTS.txt", "w+")) == NULL) {
            fprintf(stderr, "ERROR in writing to results file...");
            exit(EXIT_FAILURE);
        }
        for (int row = 0; row < ROWS; row++) {
            for (int column = 0; column < COLUMNS-1; column++) {
                fprintf(fp, "%c", grid[row][column]);
            }
            fprintf(fp, "\n");
        } 
        printf("-------------------------------------------------------------------------------------\n");
        printf("Simulation results stored in: ./FOREST_FIRE_RESULTS.txt\n");
        printf("-------------------------------------------------------------------------------------\n");
    }
    
    MPI_Finalize();
	return 0;
}

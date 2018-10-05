#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <limits.h>

typedef struct inputs{

    int id;
    int xCoor;
    int yCoor;

}strInput;

//This is needed to return multiple types from a function.
typedef struct returnStruct{

    int **adjMatrixPointer;
    strInput *inputPointer;

}strReturnStruct;

strReturnStruct *readStoreCreate(FILE *fp, char *fileName, strInput *input);
FILE *readFile(FILE *fp, char *fileName);
strInput *createStruct(FILE *fp);
int **createAdjacencyMatrix(strInput *input);
int **nearestNeighborAlgorithm(strInput *input, int **adjMatrix);
bool doesContain(int **path, int counter, int index);
int **twoOptAlgorithm(int **adjMatrix, int **nearestNeighbor, int hourLimit, int minuteLimit);
int calculateDistance(int **currentRoute);
int **twoOptSwap(int **currentRoute, int **tempPath, int **adjMatrix, int firstStop, int secondStop);
void copyContents(int **sourceMatrix, int **destMatrix);
void freeTwoDimensional(int **matrix);
void cleanUp(FILE *file, strInput *input, int **adjMatrix, strReturnStruct *structAndAdj, int **twoOpt);
void writeToFile(int **twoDimensionalMatrix, char *name);
long long getMicrotime();

//Creating a global variable to hold the size of input.
int inputSize;
int i,j;

int main(){

    //Firstly, lets get the time bounds which user wants.
    int hours = 0;
    int minutes = 0;

    printf("\nHow many hours and minutes you want the 2-opt algorithm works: ");
    scanf("%d %d", &hours, &minutes); // NOLINT

    //Also, lets let the user choose the file to use as input.
    char fileName[32];

    printf("\nWhich input file you want to process (don't write the extension ex. \"input1\"): ");
    scanf("%s", fileName);

    //Getting time.
    time_t mytime = time(NULL);
    char *time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    printf("\nStarting Time : %s\n", time_str);

    //Creating the file pointer, struct and adjacency matrix.
    FILE *input = NULL;
    strInput *strInput = NULL;
    int **adjInput = NULL;
    strReturnStruct *structAndAdj = NULL;

    //Reading input, storing it and creating the adjacency matrix of it.
    char inputFileName[32];
    strcpy(inputFileName, fileName);
    strcat(inputFileName, ".txt");

    structAndAdj = readStoreCreate(input, inputFileName, strInput);
    strInput = structAndAdj->inputPointer;
    adjInput = structAndAdj->adjMatrixPointer;

    /////////////////////////////////////////////////////////////////////////////////////////////

    //Calculating the nearest neighbor algorithm.
    int **nearestNeighbor = nearestNeighborAlgorithm(strInput, adjInput);

    //Writing to file.
    char outputNnaFileName[32];
    strcpy(outputNnaFileName, fileName);
    strcat(outputNnaFileName, "_nna_output.txt");

    writeToFile(nearestNeighbor, outputNnaFileName);

    //Same things for 2-Opt algorithm.
    int **twoOpt = twoOptAlgorithm(adjInput, nearestNeighbor, hours, minutes);

    char outputFileName[32];
    strcpy(outputFileName, fileName);
    strcat(outputFileName, "_output.txt");

    writeToFile(twoOpt, outputFileName);

    /////////////////////////////////////////////////////////////////////////////////////////////

    //Freeing the dynamic memory and closing the file pointer.
    cleanUp(input, strInput, adjInput, structAndAdj, twoOpt);

    //Getting time.
    mytime = time(NULL);
    time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    printf("\nEnding Time : %s\n", time_str);

    return 0;
}

strReturnStruct *readStoreCreate(FILE *fp, char *fileName, strInput *input){

    //Reading the file into file pointer.
    fp = readFile(fp, fileName);
    printf("\nRead input.\n");

    //Reading into the struct.
    input = createStruct(fp);

    //Adjacency matrix.
    int **adjInput = NULL;
    adjInput = createAdjacencyMatrix(input);
    printf("\nAdjacency matrix has been created.\n");

    //Now, we have created the struct and adjacency matrix. We have to return them.
    strReturnStruct *structAndAdj = malloc(sizeof(strReturnStruct));

    //Assigning these values to struct.
    structAndAdj->adjMatrixPointer = adjInput;
    structAndAdj->inputPointer = input;

    return structAndAdj;
}

FILE *readFile(FILE *fp, char *fileName){

    //If the file exists, we read it. If not, we exit the program with an error.
    if(access(fileName, F_OK) != -1){

        fp = fopen(fileName, "r");

    }else{

        printf("%s does not exist. Please make sure it exists before starting this program.\n", fileName);
        exit(1);

    }

    return fp;
}

strInput *createStruct(FILE *fp){

    //Variables.
    int id, xCoor, yCoor;
    strInput *input = malloc(sizeof(strInput));
    int index = 0;

    //While not reaching to the end of file, we read it and store the values in a dynamic array.
    while(!feof(fp)){ // NOLINT

        if(fscanf(fp, " %9d %9d %9d", &id, &xCoor, &yCoor) == 3){ // NOLINT

            //Reallocating the array every time we found a new value.
            input = realloc(input, (index + 1) * sizeof(strInput));
            input[index].id = id;
            input[index].xCoor = xCoor;
            input[index].yCoor = yCoor;
            index++;

        }
    }

    //Storing the size.
    inputSize = index;

    return input;
}

int **createAdjacencyMatrix(strInput *input){

    /////////////////IMPORTANT NOTE - IF THE SIZE IS GREATER THAN 16000, THAN THE PROGRAM CRASHES.

    //Creating the adjacency matrix dynamically and setting values to 0 by using calloc.
    int **adjInput = calloc((size_t) inputSize, sizeof(int *));

    for(i = 0; i < inputSize; i++)
        adjInput[i] = calloc((size_t) inputSize, sizeof(int));

    //Filling the matrix.
    for(i = 0; i < inputSize; i++){

        for(j = 0; j < inputSize; j++){

            //Optimizing by using if statements.
            if(i < j)
                adjInput[i][j] = (int)(round(sqrt(pow((input[i].xCoor - input[j].xCoor), 2) + pow((input[i].yCoor - input[j].yCoor), 2))));
            else if(i == j)
                adjInput[i][j] = 0;
            else
                adjInput[i][j] = adjInput[j][i];
        }
    }

    return adjInput;
}

int **nearestNeighborAlgorithm(strInput *input, int **adjMatrix){

    //Firstly, lets a create an array to hold the path we took.
    int **path = malloc(inputSize * sizeof(int *));

    for(i = 0; i < inputSize; i++)
        path[i] = malloc(2 * sizeof(int));

    //We need a variable to count the cities visited.
    int counter = 0;

    //Let's start with the city 0.
    path[counter][0] = input[0].id;
    path[counter++][1] = 0;

    //Creating necessary variables.
    //Distance[0] the id of it and distance[1] holds the distance.
    int distance[2], currentCityId = path[0][0];

    while(counter < inputSize){

        distance[1] = 999999999;

        for(i = 0; i < inputSize; i++){

            //We don't want to visit the city we visited before.
            if(doesContain(path, counter, input[i].id) == true)
                continue;

            //Compares the distances and tries to find the minimum.
            if(distance[1] > adjMatrix[currentCityId][i]){
                distance[1] = adjMatrix[currentCityId][i];
                distance[0] = input[i].id;
            }

        }

        path[counter][0] = currentCityId = distance[0];
        path[counter++][1] = distance[1];
    }

    //Lastly adding last to first distance.
    path[0][1] = adjMatrix[path[--counter][0]][path[0][0]];

    printf("\nNearest neighbor algorithm completed.\n");

    return path;
}

bool doesContain(int **path, int counter, int index){

    //We want to check if index'th number is visited or not.
    for(int i = 0; i < counter; i++){

        if(path[i][0] == index)
            return true;
    }

    return false;
}

int **twoOptAlgorithm(int **adjMatrix, int **nearestNeighbor, int hourLimit, int minuteLimit){

    //This algorithm is a very good algorithm, but it has a downside which is speed. It can take days to find output for very big cities.
    //But if we don't want to spend days, we can limit it with time threshold.

    //Calculating the starting time.
    long long startTime, currentTime, timeElapsed, timeLimit;
    startTime = getMicrotime();
    timeLimit = (hourLimit * 3600000000) + (minuteLimit * 60000000);

    //Variables and pointers.
    int **currentRoute = nearestNeighbor;
    int **newRoute = NULL;
    int newDistance, currentDistance;

    //Lets a create an array to hold the path we took.
    int **tempPath = malloc(inputSize * sizeof(int *));

    for(i = 0; i < inputSize; i++)
        tempPath[i] = malloc(2 * sizeof(int));

    //The starting point, we can return here anytime we found a better route.
    newRouteFound:
    currentDistance = calculateDistance(currentRoute);

    //Calculating the current time.
    currentTime = getMicrotime();

    //Now, we will calculate the time elapsed and continue if didn't pass the limit.
    timeElapsed = currentTime - startTime;

    if(timeElapsed < timeLimit)

        for(i = 0; i < inputSize - 1; i++){
            for(j = i + 1; j < inputSize - 1; j++){

                //Finding the new route.
                newRoute = twoOptSwap(currentRoute, tempPath, adjMatrix, i, j);
                newDistance = calculateDistance(newRoute);

                if(newDistance < currentDistance){

                    //Copy the contents of newRoute to currentRoute
                    copyContents(newRoute, currentRoute);
                    goto newRouteFound;

                }
            }
        }

    //Lastly, we need to free the temp array.
    freeTwoDimensional(tempPath);

    printf("\n2-Opt algorithm completed.\n");

    return currentRoute;
}

int calculateDistance(int **currentRoute){

    //This function calculates the total distance for the given route.
    int totalDistance = 0, count;

    for(count = 0; count < inputSize; count++)
        totalDistance += currentRoute[count][1];

    return totalDistance;
}

int **twoOptSwap(int **currentRoute, int **tempPath, int **adjMatrix, int firstStop, int secondStop){

    //This is the implementation of the 2-Opt swap. We simply hold the values till firstStop, put the items between
    //firstStop and secondStop in reverse order, then put items coming after secondStop.
    //By doing these, we generated a new route.
    int x = 0, current = 0;

    for(x = 0; x <= firstStop - 1; x++){

        tempPath[x][0] = currentRoute[x][0];
        tempPath[x][1] = currentRoute[x][1];

    }

    for(current = x, x = secondStop; x >= firstStop; x--, current++){

        tempPath[current][0] = currentRoute[x][0];
        tempPath[current][1] = currentRoute[x+1][1];

    }

    for(x = secondStop + 1; x < inputSize; x++){

        tempPath[x][0] = currentRoute[x][0];
        tempPath[x][1] = currentRoute[x][1];

    }

    //Lastly, we need to update 3 values.
    if(firstStop != 0) //If the firstStop isn't the first value, the distance to it will be false. Otherwise, this is not needed.
        tempPath[firstStop][1] = adjMatrix[tempPath[firstStop-1][0]][tempPath[firstStop][0]];
    tempPath[secondStop+1][1] = adjMatrix[tempPath[secondStop][0]][tempPath[secondStop+1][0]];
    tempPath[0][1] = adjMatrix[tempPath[0][0]][tempPath[inputSize-1][0]];

    return tempPath;
}

void copyContents(int **sourceMatrix, int **destMatrix){

    //This function simply copies all the contents of sourceMatrix to destMatrix. This function optimizes the program.
    int a,b;

    for(a = 0; a < inputSize; a++){
        for(b = 0; b < 2; b++){

            destMatrix[a][b] = sourceMatrix[a][b];

        }
    }
}

void freeTwoDimensional(int **matrix){

    //This function frees the given two dimensional array.
    int x;

    for(x = 0; x < inputSize; x++)
        free(matrix[x]);

    free(matrix);
}

void cleanUp(FILE *file, strInput *input, int **adjMatrix, strReturnStruct *structAndAdj, int **twoOpt){

    //Closing the file pointer.
    fclose(file);

    //Freeing the dynamic memory allocations.
    free(input);

    //Freeing the adjacency matrix.
    freeTwoDimensional(adjMatrix);

    //Freeing the strReturnStruct.
    free(structAndAdj);

    //Freeing the twoOpt.
    freeTwoDimensional(twoOpt);
}

void writeToFile(int **twoDimensionalMatrix, char *name){

    //This function writes the route to file like the course instructor wants.
    FILE *fp = fopen(name, "w+");

    fprintf(fp, "%d\n", calculateDistance(twoDimensionalMatrix));

    for(i = 0; i < inputSize; i++){

        fprintf(fp, "%d\n", twoDimensionalMatrix[i][0]);

    }

    fclose(fp);
}

long long getMicrotime(){
    //Time calculator.
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}
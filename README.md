The *travelling salesman problem* (TSP) asks the following question: Given a list of cities and the distances between each pair of cities, what is the **shortest possible route** that visits each city *exactly once* and returns to the origin city?

**Inputs are**: n cities, with their locations (x and y coordinates) in a 2D grid.

**Output is**: Ordering (tour) of these cities so that total distance to travel is minimized.

I firstly used the *nearest neighbor algorithm* and improved the accuracy by using *2-opt algorithm*. Since the amount of time increases as the input grows larger, I set a time limit to stop and save the work for the program which you can specify at the start of the program.

The program works without any problem but since the TSP problem requires **accuracy**, you need to give it time (I mean **days**) for it to be able to produce higher **accuracy**.

I also included the test inputs and outputs if you would like to take a look. Other than that a **verifying procedure** tsp-verifier.py is in here. You can use it to verify your solutions. Usage:

*>* python tsp-verifier.py sample-input-1.txt sample-output-1.txt
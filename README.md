# large-PMP

The following instructions should be usable for most modern Linux systems.

They are tested on Ubuntu 20.04. The project was built with Cmake version 3.16.3, using gcc 10.2.0. Older versions might work as well.

1) Downloading the project

Clone the project from Github [(instructions)](https://docs.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository-from-github/cloning-a-repository). E.g., run in a directory of your choice

```git clone https://github.com/wolledav/large-PMP.git```

to clone via https.

2) Building the project

Navigate to the project top directory, e.g.: ```cd ~/large-PMP``` 

Create a folder for the build files and build the project, using Cmake:

```
mkdir build
cd build
cmake ..
make
```

This will create an executable ```large_PMP``` in the ```build``` directory.

3) Usage

The program takes the following compulsory parameters: 

-p . . . the number of medians to select

-dm . . . path to the distance matrix file

-w . . . path to the file with customer weights (=no. of people living at a idividual customer locations)

-c . . . path to the file with location capacities

-mode . . . mode of operation (1 - TB heuristic, PMP; 2 - TB heuristic, cPMP; 3 - full RSSV heuristic, PMP; 4 - full RSSV heuristic, cPMP)

Then, there are the following optional parameters:

-o . . . path to an output file, for ewporting a solution

-th . . . no. of subproblems solved by the RSSV in parallel (default = 4)

-seed . . . seed of the random generator (default = 1)

Usage examples with the Toulon instance (to be run in the ```~/large-PMP``` directory:

```
 ./build/large_PMP -p 5 -dm ./data/toulon/dist_matrix.txt -w ./data/toulon/cust_weights.txt -c ./data/toulon/loc_capacities.txt -mode 3
```
solves the standard PMP with 5 medians for the Toulon instance. Solution is printed to the console.
```
 ./build/large_PMP -p 5 -dm ./data/toulon/dist_matrix.txt -w ./data/toulon/cust_weights.txt -c ./data/toulon/loc_capacities.txt -mode 4 -o output.txt
```
solves the capacitated PMP with 5 medians for the Toulon instance. Solution is exported to file `output.txt`.




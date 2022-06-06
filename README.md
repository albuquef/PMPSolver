# large-PMP

The following instructions should be usable for most modern Linux systems.

They are tested on Ubuntu 20.04. The project was built with Cmake version 3.16.3, using gcc 10.2.0. Older versions might work as well. The code uses the OpenMP library for parallelization, which is included in newer gcc versions.

---

## 1) Downloading the project

Clone the project from Github ([instructions](https://docs.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository-from-github/cloning-a-repository)). E.g., run in a directory of your choice

```git clone https://github.com/wolledav/large-PMP.git```

to clone via https.

## 2) Building the project

Navigate to the project top directory, e.g.: ```cd ~/large-PMP``` 

Create a folder for the build files and build the project, using Cmake:

```
mkdir build
cd build
cmake ..
make
```

This will create an executable ```large_PMP``` in the ```build``` directory.

## 3) Usage

The program takes the following compulsory parameters: 

-p . . . the number of medians to select

-dm . . . path to the distance matrix file

-w . . . path to the file with customer weights (=no. of people living at a idividual customer locations)

-c . . . path to the file with location capacities (needed even for solving standard PMP)

-mode . . . mode of operation (1 - TB heuristic, PMP; 2 - TB heuristic, cPMP; 3 - full RSSV heuristic, PMP; 4 - full RSSV heuristic, cPMP)

Then, there are the following optional parameters:

-t | -time <seconds> . . . CPU time limit, expressed in seconds
 
--help | ? . . . displays the help
 
-config <path_to_config_file.toml> . . . path to the configuration file to use
 
-v | -verbose . . . sets the verbose to true

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

You can also change the settings in the configuration file: `config.toml`.

See the following section for a full explanation.

## 4) Configuration file usage

To let you use the program without struggling with all the parameters, just configure the `config.toml` file by changing the value of each variable.

This is what the config file looks like:

```toml
verbose = false
p = 5
capacities = "./data/toulon/loc_capacities.txt"
distance_matrix = "./data/toulon/dist_matrix.txt"
output = ""
weights = "./data/toulon/cust_weights.txt"
threads = 4
mode = 3
seed = 1
time = 7200
```

As you can see in the code snippet above, all the parameters are represented. Simply change the value of a paramter, run the program and that's it.

You can now run the program like:
```
build/large_PMP
```

### 4.1) Configuration override

If you want to override the configuration for a single run by editting an option, just pass the option in the command line just like:

```
build/large_PMP -t 3600
```

In the example above, the configuration `time` parameter is ignored and replaced by the value of `-t` option you just passed. Note that all the other parameters in `config.toml` are considered.

If you want to work with multiple configuration files, just pass the config file you want to use in the command like:
```
build/large_PMP -config path_to_config_file.toml
```

This lets you change configuration on the fly.


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

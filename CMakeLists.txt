cmake_minimum_required(VERSION 3.16)
project(large_PMP)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread -fopenmp -lconcert -lilocplex -lcplex -lm -lpthread -ldl")

add_executable(${PROJECT_NAME}
                src/main.cpp 
                src/globals.hpp src/globals.cpp 
                src/instance.cpp src/instance.hpp 
                src/utils.cpp src/utils.hpp 
                src/solution_std.cpp src/solution_std.hpp 
                src/TB.cpp src/TB.hpp 
                src/RSSV.cpp src/RSSV.hpp 
                src/semaphore.hpp 
                src/solution_cap.cpp src/solution_cap.hpp 
                src/solution.cpp src/solution.hpp 
                src/config_parser.cpp 
                src/toml.hpp 
                src/TBPercentage.hpp src/TBPercentage.cpp
                src/PMP.hpp src/PMP.cpp)



CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio2211/concert
CPCPLEXDIR    = /opt/ibm/ILOG/CPLEX_Studio2211/cpoptimizer

CPLEXBINDIR = $(CPLEXDIR)/bin/$(BINDIST)

CPLEXLIBDIR = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) 
CCLNFLAGS = -lconcert -lilocplex -lcplex -lm -lpthread -ldl  #-framework CoreFoundation -framework IOKit 


CPCPLEXBINDIR = $(CPCPLEXDIR)/bin/$(BINDIST)
CPCPLEXLIBDIR = $(CPCPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CPCCLNDIRS  = -L$(CPCPLEXLIBDIR) -L$(CPCONCERTLIBDIR) 

# ------------------------------------------------------------
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR = $(CPLEXDIR)/include

CPCPLEXINCDIR = $(CPCPLEXDIR)/includerun_SPC

CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 


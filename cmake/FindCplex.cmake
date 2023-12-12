# set(CPLEX_ROOT_DIR "" CACHE PATH "CPLEX root directory.")
set(CPLEX_COMPILER_FLAGS "-m64 -g -O -fPIC -fexceptions -DNDEBUG -DIL_STD" CACHE STRING "Cplex Compiler Flags")

find_path(CPLEX_INCLUDE_DIR ilcplex/cplex.h HINTS
    ${CPLEX_ROOT_DIR}/cplex/include
)
find_path(CPLEX_CONCERT_INCLUDE_DIR ilconcert/iloenv.h HINTS
    ${CPLEX_ROOT_DIR}/concert/include
)
find_library(CPLEX_LIBRARY NAMES cplex HINTS
    ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux/static_pic
    ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_sles10_4.1/static_pic
    ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_osx/static_pic
)
find_library(CPLEX_ILOCPLEX_LIBRARY ilocplex HINTS
    ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux/static_pic
    ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_sles10_4.1/static_pic
    ${CPLEX_ROOT_DIR}/cplex/lib/x86-64_osx/static_pic
)
find_library(CPLEX_CONCERT_LIBRARY concert HINTS
    ${CPLEX_ROOT_DIR}/concert/lib/x86-64_linux/static_pic
    ${CPLEX_ROOT_DIR}/concert/lib/x86-64_sles10_4.1/static_pic
    ${CPLEX_ROOT_DIR}/concert/lib/x86-64_osx/static_pic
)
find_path(CPLEX_BIN_DIR cplex HINTS
    ${CPLEX_ROOT_DIR}/cplex/bin/x86-64_sles10_4.1
    ${CPLEX_ROOT_DIR}/cplex/bin/x86-64_linux
    ${CPLEX_ROOT_DIR}/cplex/bin/x86-64_osx
)
include(FindPackageHandleStandardArgs)

# Fix the package name to match what you use in CMakeLists.txt
find_package_handle_standard_args(Cplex DEFAULT_MSG CPLEX_LIBRARY CPLEX_INCLUDE_DIR CPLEX_ILOCPLEX_LIBRARY CPLEX_CONCERT_LIBRARY CPLEX_CONCERT_INCLUDE_DIR)

if(CPLEX_FOUND)
    set(CPLEX_INCLUDE_DIRS ${CPLEX_INCLUDE_DIR} ${CPLEX_CONCERT_INCLUDE_DIR})
    set(CPLEX_LIBRARIES ${CPLEX_CONCERT_LIBRARY} ${CPLEX_ILOCPLEX_LIBRARY} ${CPLEX_LIBRARY})
    set(CPLEX_LIBRARIES "${CPLEX_LIBRARIES};m;pthread")
endif(CPLEX_FOUND)

mark_as_advanced(CPLEX_LIBRARY CPLEX_INCLUDE_DIR CPLEX_ILOCPLEX_LIBRARY CPLEX_CONCERT_INCLUDE_DIR CPLEX_CONCERT_LIBRARY)

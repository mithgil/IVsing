%module datparser

%{
#include "utilities.h"
#include "ivs.h"
%}

// Import STL containers
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"

// Expose headers
%include "utilities.h"
%include "ivs.h"

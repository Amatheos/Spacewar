// The single translation unit that compiles miniaudio's implementation.
// It's about ~90k-line header, needs to be here to avoid triggering
// recompilation by whoever includes it
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

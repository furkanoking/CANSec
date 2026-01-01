#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

extern "C" {

int read_counter(unsigned int counter_id);

void write_counter(unsigned int counter_id);

int read_nonce_value();

}  // extern "C"

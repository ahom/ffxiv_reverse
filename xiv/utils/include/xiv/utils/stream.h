#ifndef XIV_UTILS_STREAM_H
#define XIV_UTILS_STREAM_H

#include <memory>
#include <iostream>
#include <vector>

namespace xiv
{
namespace utils
{
namespace stream
{

// Creates a istream to be able to read from the vector like a real stream
// CAUTION! If you modify the data vector while parsing the stream, shit may happen!
// This does not copy the data, it only iterates over the vector by initializing the pointers of the streambuf correctly
std::unique_ptr<std::basic_istream<char>> get_istream(const std::vector<char>& i_source);

std::unique_ptr<std::basic_ostream<char>> get_ostream(std::vector<char>& i_source);

}
}
}

#endif // XIV_UTILS_STREAM_H

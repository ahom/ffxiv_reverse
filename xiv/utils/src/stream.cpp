#include <xiv/utils/stream.h>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

namespace xiv
{
namespace utils
{
namespace stream
{

std::unique_ptr<std::basic_istream<char>> get_istream(const std::vector<char>& i_source)
{
    // basic_array_source, _data.data() start pointer, _data.size(), size of data
    // vector is required to be contiguous in memory so this works and is not undefined
    return std::unique_ptr<std::basic_istream<char>>(
               new boost::iostreams::stream<boost::iostreams::basic_array_source<char>>(i_source.data(), i_source.size()));
}

std::unique_ptr<std::basic_ostream<char>> get_ostream(std::vector<char>& i_source)
{
    return std::unique_ptr<std::basic_ostream<char>>(
               new boost::iostreams::stream<boost::iostreams::basic_array_sink<char>>(i_source.data(), i_source.size()));
}

}
}
}

#ifndef XIV_UTILS_CONV_H
#define XIV_UTILS_CONV_H

#include <cstdint>
#include <vector>
#include <ostream>

namespace xiv
{
namespace utils
{
namespace conv
{
float half2float(const uint16_t i_value);
float ubyte2float(const uint8_t i_value);

void bin2base64(const std::vector<char>& i_data, std::ostream& o_stream);
}
}
}

#endif // XIV_UTILS_CONV_H

#include <xiv/utils/conv.h>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

namespace xiv
{
namespace utils
{
namespace conv
{
float half2float(const uint16_t i_value)
{
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;

    t1 = i_value & 0x7fff;                  // Non-sign bits
    t2 = i_value & 0x8000;                  // Sign bit
    t3 = i_value & 0x7c00;                  // Exponent

    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position

    t1 += 0x38000000;                       // Adjust bias

    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

    t1 |= t2;                               // Re-insert sign bit

    return *reinterpret_cast<float*>(&t1);
}

float ubyte2float(const uint8_t i_value)
{
    return i_value / 255.0f;
}

void bin2base64(const std::vector<char>& i_data, std::ostream& o_stream)
{
    using namespace boost::archive::iterators;
    typedef
    base64_from_binary <    // convert binary values ot base64 characters
    transform_width <   // retrieve 6 bit integers from a sequence of 8 bit bytes
    const char *, 6, 8>> base64_text; // compose all the above operations in to a new iterator

    std::copy(
        base64_text(i_data.data()),
        base64_text(i_data.data() + i_data.size()),
        ostream_iterator<char>(o_stream));

    for (uint32_t i = 0; i < (i_data.size() % 3); ++i)
    {
        o_stream << "=";
    }
}
}
}
}


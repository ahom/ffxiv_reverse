#ifndef XIV_EXD_EXH_H
#define XIV_EXD_EXH_H

#include <map>

#include <xiv/utils/bparse.h>

XIV_ENUM((xiv)(exd), DataType, uint16_t,
         XIV_VALUE(string,   0)
         XIV_VALUE(boolean,  1)
         XIV_VALUE(int8,     2)
         XIV_VALUE(uint8,    3)
         XIV_VALUE(int16,    4)
         XIV_VALUE(uint16,   5)
         XIV_VALUE(int32,    6)
         XIV_VALUE(uint32,   7)
         XIV_VALUE(float32,  9)
         XIV_VALUE(uint64,   11));

XIV_STRUCT((xiv)(exd), ExhHeader,
           XIV_MEM_ARR(char, magic, 0x4)
           XIV_MEM_BE(uint16_t, unknown)
           XIV_MEM_BE(uint16_t, data_offset)
           XIV_MEM_BE(uint16_t, field_count)
           XIV_MEM_BE(uint16_t, exd_count)
           XIV_MEM_BE(uint16_t, language_count));

XIV_STRUCT((xiv)(exd), ExhMember,
           XIV_MEM_BE(DataType, type)
           XIV_MEM_BE(uint16_t, offset));

XIV_STRUCT((xiv)(exd), ExhExdDef,
           XIV_MEM_BE(uint32_t, start_id)
           XIV_MEM_BE(uint32_t, count_id));

namespace xiv
{
namespace dat
{
class File;
}
namespace exd
{

enum class Language: uint16_t;

// Header file for exd data
    class Exh
{
public:
    // The header file
    Exh(const dat::File& i_file);
    ~Exh();

    const ExhHeader& get_header() const;
    const std::vector<ExhExdDef>& get_exd_defs() const;
    const std::vector<Language>& get_languages() const;
    const std::map<uint32_t, ExhMember>& get_members() const;

protected:
    ExhHeader _header;
    // Members of the datastruct ordered(indexed) by offset
    std::map<uint32_t, ExhMember> _members;
    std::vector<ExhExdDef> _exd_defs;
    std::vector<Language> _languages;
};

}
}

#endif // XIV_EXD_EXH_H

#ifndef XIV_EXD_EXD_H
#define XIV_EXD_EXD_H

#include <memory>
#include <map>

#include <boost/variant.hpp>

#include <xiv/dat/File.h>

namespace xiv
{
namespace exd
{

class Exh;

// Field type containing all the possible types in the data files
typedef boost::variant<
std::string,
    bool,
    int8_t,
    uint8_t,
    int16_t,
    uint16_t,
    int32_t,
    uint32_t,
    float,
    uint64_t> Field;

// Data for a given language
class Exd
{
public:
    // i_exh: the header
    // i_files: the multiple exd files
    Exd(const Exh& i_exh, const std::vector<std::unique_ptr<dat::File>>& i_files);
    ~Exd();

    // Get a row by its id
    const std::vector<Field>& get_row(uint32_t id);

    // Get all rows
    const std::map<uint32_t, std::vector<Field>>& get_rows();

    // Get as csv
    void get_as_csv(std::ostream& o_stream) const;

protected:
    // Data indexed by the ID of the row, the vector is field with the same order as exh.members
    std::map<uint32_t, std::vector<Field>> _data;
};

}
}

#endif // XIV_EXD_EXD_H

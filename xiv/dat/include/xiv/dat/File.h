#ifndef XIV_DAT_FILE_H
#define XIV_DAT_FILE_H

#include <vector>

#include <boost/filesystem.hpp>

#include <xiv/utils/bparse.h>

XIV_ENUM((xiv)(dat), FileType, uint32_t,
         XIV_VALUE(empty,    1)
         XIV_VALUE(standard, 2)
         XIV_VALUE(model,    3)
         XIV_VALUE(texture,  4));

namespace xiv
{
namespace dat
{

class Dat;

// Basic file from the dats
class File
{
    friend class Dat;
public:
    File();
    ~File();

    FileType get_type() const;

    // Getters functions for the data in the file
    const std::vector<std::vector<char>>& get_data_sections() const;
    std::vector<std::vector<char>>& access_data_sections();

    void export_as_bin(const boost::filesystem::path& i_path) const;

protected:
    FileType _type;
    std::vector<std::vector<char>> _data_sections;
};

}
}

#endif // XIV_DAT_FILE_H

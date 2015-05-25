#ifndef XIV_DAT_DAT_H
#define XIV_DAT_DAT_H

#include <xiv/dat/SqPack.h>

#include <mutex>

#include <boost/filesystem.hpp>

namespace xiv
{
namespace dat
{

class File;

class Dat : public SqPack
{
public:
    // Full path to the dat file
    Dat(const boost::filesystem::path& i_path, uint32_t i_nb);
    virtual ~Dat();

    // Retrieves a file given the offset in the dat file
    std::unique_ptr<File> get_file(uint32_t i_offset);

    // Appends to the vector the data of this block, it is assumed to be preallocated
    // Is it also assumed that the _file_mutex is currently locked by this thread before the call
    void extract_block(uint32_t i_offset, std::vector<char>& o_data);

    // Returns the dat number
    uint32_t get_nb() const;

protected:
    // File reading mutex to have only one thread reading the file at a time
    std::mutex _file_mutex;

    // Dat nb
    uint32_t _nb;
};

}
}

#endif // XIV_DAT_DAT_H

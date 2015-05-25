#ifndef XIV_DAT_SQPACK_H
#define XIV_DAT_SQPACK_H

#include <fstream>

#include <boost/filesystem.hpp>

#include <xiv/utils/bparse.h>

// Common struct containing the SHA-1 hash
// Used for data integrity in all Sqpack files
XIV_STRUCT((xiv)(dat), SqPackBlockHash,
           XIV_MEM_ARR(uint8_t, hash, 0x14)
           XIV_MEM_ARR(uint32_t, padding, 0xB));

namespace xiv
{
namespace dat
{

class SqPack
{
public:
    // Full path to the sqpack file
    SqPack(const boost::filesystem::path& i_path);
    virtual ~SqPack();

protected:
    // Checks that a given block is valid iven its hash
    void is_block_valid(uint32_t i_offset, uint32_t i_size, const SqPackBlockHash& i_block_hash);

    // File handle
    std::ifstream _handle;
};

}
}

#endif // XIV_DAT_SQPACK_H

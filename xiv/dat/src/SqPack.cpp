#include <xiv/dat/SqPack.h>

#include <xiv/dat/logger.h>

XIV_STRUCT((xiv)(dat), SqPackHeader,
           XIV_MEM_ARR(char, magic, 0x8)
           XIV_MEM(uint32_t, zero)
           XIV_MEM(uint32_t, size)
           XIV_MEM(uint32_t, version)
           XIV_MEM(uint32_t, type));

XIV_STRUCT((xiv)(dat), SqPackIndexHeader,
           XIV_MEM(uint32_t, size)
           XIV_MEM(uint32_t, type));

using xiv::utils::bparse::extract;

namespace xiv
{
namespace dat
{

SqPack::SqPack(const boost::filesystem::path& i_path) :
    // Open the file
    _handle(i_path.string(), std::ios_base::in | std::ios_base::binary)
{
    XIV_DEBUG(xiv_dat_logger, "Initializing SqPack with path: " << i_path);

    // Extract the header
    extract<xiv_dat_logger, SqPackHeader>(_handle);

    // Skip until the IndexHeader the extract it
    _handle.seekg(0x400);
    extract<xiv_dat_logger, SqPackIndexHeader>(_handle);
}

SqPack::~SqPack()
{
}

void SqPack::is_block_valid(uint32_t i_offset, uint32_t i_size, const SqPackBlockHash& i_block_hash)
{
    // TODO
}

}
}

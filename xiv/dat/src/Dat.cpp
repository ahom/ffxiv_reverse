#include <xiv/dat/Dat.h>

#include <xiv/utils/zlib.h>

#include <xiv/dat/logger.h>
#include <xiv/dat/File.h>

XIV_STRUCT((xiv)(dat), DatFileHeader,
           XIV_MEM(    uint32_t,       size)
           XIV_MEM(    FileType,        entry_type)
           XIV_MEM(    uint32_t,       total_uncompressed_size)
           XIV_MEM_ARR(uint32_t,       unknown,                    0x2));

// block
XIV_STRUCT((xiv)(dat), DatBlockRecord,
           XIV_MEM(    uint32_t,           offset)
           XIV_MEM(    uint32_t,           size)
           XIV_MEM_ARR(uint32_t,           unknown,        0x4)
           XIV_MEM(    SqPackBlockHash,    block_hash));

XIV_STRUCT((xiv)(dat), DatBlockHeader,
           XIV_MEM(uint32_t, size)
           XIV_MEM(uint32_t, unknown1)
           XIV_MEM(uint32_t, compressed_size)
           XIV_MEM(uint32_t, uncompressed_size));

// standard
XIV_STRUCT((xiv)(dat), DatStdFileBlockInfos,
           XIV_MEM(uint32_t, offset)
           XIV_MEM(uint16_t, size)
           XIV_MEM(uint16_t, uncompressed_size));

// model
namespace
{
const uint32_t model_section_count = 0xB;
}

XIV_STRUCT((xiv)(dat), DatMdlFileBlockInfos,
           XIV_MEM(    uint32_t, unknown1)
           XIV_MEM_ARR(uint32_t, uncompressed_sizes,   ::model_section_count)
           XIV_MEM_ARR(uint32_t, compressed_sizes,     ::model_section_count)
           XIV_MEM_ARR(uint32_t, offsets,              ::model_section_count)
           XIV_MEM_ARR(uint16_t, block_ids,            ::model_section_count)
           XIV_MEM_ARR(uint16_t, block_counts,         ::model_section_count)
           XIV_MEM_ARR(uint32_t, unknown2,             0x2));

// texture
XIV_STRUCT((xiv)(dat), DatTexFileBlockInfos,
           XIV_MEM(uint32_t, offset)
           XIV_MEM(uint32_t, size)
           XIV_MEM(uint32_t, uncompressed_size)
           XIV_MEM(uint32_t, block_id)
           XIV_MEM(uint32_t, block_count));

using xiv::utils::bparse::extract;

namespace xiv
{
namespace dat
{

Dat::Dat(const boost::filesystem::path& i_path, uint32_t i_nb) :
    SqPack(i_path),
    _nb(i_nb)
{
    auto block_record = extract<xiv_dat_logger, DatBlockRecord>(_handle);
    block_record.offset *= 0x80;
    is_block_valid(block_record.offset, block_record.size, block_record.block_hash);
}

Dat::~Dat()
{
}

std::unique_ptr<File> Dat::get_file(uint32_t i_offset)
{
    XIV_DEBUG(xiv_dat_logger, "Get file nb: " << _nb << " - offset: " << i_offset);

    std::unique_ptr<File> output_file(new File());
    {
        // Lock in this scope
        std::lock_guard<std::mutex> lock(_file_mutex);

        // Seek to the start of the header of the file record and extract it
        _handle.seekg(i_offset);
        auto file_header = extract<xiv_dat_logger, DatFileHeader>(_handle);

        switch(file_header.entry_type)
        {
        case FileType::empty:
            XIV_WARNING(xiv_dat_logger, "File is empty");
            break;

        case FileType::standard:
        {
            output_file->_type = FileType::standard;

            uint32_t number_of_blocks = extract<xiv_dat_logger, uint32_t>(_handle, "number_of_blocks");

            // Just extract offset infos for the blocks to extract
            std::vector<DatStdFileBlockInfos> std_file_block_infos;
            extract<xiv_dat_logger, DatStdFileBlockInfos>(_handle, number_of_blocks, std_file_block_infos);

            // Pre allocate data vector for the whole file
            output_file->_data_sections.resize(1);
            auto& data_section = output_file->_data_sections.front();

            data_section.reserve(file_header.total_uncompressed_size);
            // Extract each block
            for (auto& file_block_info: std_file_block_infos)
            {
                extract_block(i_offset + file_header.size + file_block_info.offset, data_section);
            }
        }
        break;

        case FileType::model:
        {
            output_file->_type = FileType::model;

            DatMdlFileBlockInfos mdl_file_block_infos = extract<xiv_dat_logger, DatMdlFileBlockInfos>(_handle);

            // Getting the block number and read their sizes
            const uint32_t block_count = mdl_file_block_infos.block_ids[::model_section_count - 1] + mdl_file_block_infos.block_counts[::model_section_count - 1];
            std::vector<uint16_t> block_sizes;
            extract<xiv_dat_logger, uint16_t>(_handle, "block_size", block_count, block_sizes);

            // Preallocate sufficient space
            output_file->_data_sections.resize(::model_section_count);

            for (uint32_t i = 0; i < ::model_section_count; ++i)
            {
                // Preallocating for section
                auto& data_section = output_file->_data_sections[i];
                data_section.reserve(mdl_file_block_infos.uncompressed_sizes[i]);

                uint32_t current_offset = i_offset + file_header.size + mdl_file_block_infos.offsets[i];
                for (uint32_t j = 0; j < mdl_file_block_infos.block_counts[i]; ++j)
                {
                    extract_block(current_offset, data_section);
                    current_offset += block_sizes[mdl_file_block_infos.block_ids[i] + j];
                }
            }
        }
        break;

        case FileType::texture:
        {
            output_file->_type = FileType::texture;

            // Extracts mipmap entries and the block sizes
            uint32_t sections_count = extract<xiv_dat_logger, uint32_t>(_handle, "sections_count");

            std::vector<DatTexFileBlockInfos> tex_file_block_infos;
            extract<xiv_dat_logger>(_handle, sections_count, tex_file_block_infos);

            // Extracting block sizes
            uint32_t block_count = tex_file_block_infos.back().block_id + tex_file_block_infos.back().block_count;
            std::vector<uint16_t> block_sizes;
            extract<xiv_dat_logger>(_handle, "block_size", block_count, block_sizes);

            output_file->_data_sections.resize(sections_count + 1);

            // Extracting header in section 0
            const uint32_t header_size = tex_file_block_infos.front().offset;
            auto& header_section = output_file->_data_sections[0];
            header_section.resize(header_size);

            _handle.seekg(i_offset + file_header.size);
            _handle.read(header_section.data(), header_size);

            // Extracting other sections
            for (uint32_t i = 0; i < sections_count; ++i)
            {
                auto& data_section = output_file->_data_sections[i + 1];
                auto& section_infos = tex_file_block_infos[i];
                data_section.reserve(section_infos.uncompressed_size);

                uint32_t current_offset = i_offset + file_header.size + section_infos.offset;
                for (uint32_t j = 0; j < section_infos.block_count; ++j)
                {
                    extract_block(current_offset, data_section);
                    current_offset += block_sizes[section_infos.block_id + j];
                }
            }
        }
        break;

        default:
            throw std::runtime_error("Invalid entry_type: " + std::to_string(static_cast<uint32_t>(file_header.entry_type)));
            break;
        }
    }
    return output_file;
}

void Dat::extract_block(uint32_t i_offset, std::vector<char>& o_data)
{
    _handle.seekg(i_offset);

    DatBlockHeader block_header = extract<xiv_dat_logger, DatBlockHeader>(_handle);

    // Resizing the vector to write directly into it
    const uint32_t data_size = o_data.size();
    o_data.resize(data_size + block_header.uncompressed_size);

    // 32000 in compressed_size means it is not compressed so take uncompressed_size
    if (block_header.compressed_size == 32000)
    {
        _handle.read(o_data.data() + data_size, block_header.uncompressed_size);
    }
    else
    {
        // If it is compressed use zlib
        // Read the data to be decompressed
        std::vector<char> temp_buffer(block_header.compressed_size);
        _handle.read(temp_buffer.data(), block_header.compressed_size);

        utils::zlib::no_header_decompress(reinterpret_cast<uint8_t*>(temp_buffer.data()),
                                          temp_buffer.size(),
                                          reinterpret_cast<uint8_t*>(o_data.data() + data_size),
                                          block_header.uncompressed_size);
    }
}

uint32_t Dat::get_nb() const
{
    return _nb;
}

}
}

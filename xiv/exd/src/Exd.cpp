#include <xiv/exd/exd.h>

#include <boost/bind.hpp>
#include <boost/io/ios_state.hpp>

#include <xiv/utils/bparse.h>
#include <xiv/utils/stream.h>

#include <xiv/exd/logger.h>
#include <xiv/exd/Exh.h>

using xiv::utils::bparse::extract;

XIV_STRUCT((xiv)(exd), ExdHeader,
           XIV_MEM_ARR(char, magic, 0x4)
           XIV_MEM_BE(uint16_t, unknown)
           XIV_MEM_BE(uint16_t, unknown2)
           XIV_MEM_BE(uint32_t, index_size));

XIV_STRUCT((xiv)(exd), ExdRecordIndex,
           XIV_MEM_BE(uint32_t, id)
           XIV_MEM_BE(uint32_t, offset));

namespace xiv
{
namespace exd
{

Exd::Exd(const Exh& i_exh, const std::vector<std::unique_ptr<dat::File>>& i_files)
{
    // Iterates over all the files
    const uint32_t member_count = i_exh.get_members().size();
    for (auto& file_ptr: i_files)
    {
        // Get a stream
        auto stream_ptr = utils::stream::get_istream(file_ptr->get_data_sections().front());
        auto& stream = *stream_ptr;

        // Extract the header and skip to the record indices
        auto exd_header = extract<xiv_exd_logger, ExdHeader>(stream);
        stream.seekg(0x20);

        // Preallocate and extract the record_indices
        const uint32_t record_count = exd_header.index_size / sizeof(ExdRecordIndex);
        std::vector<ExdRecordIndex> record_indices;
        record_indices.reserve(record_count);
        for (uint32_t i = 0; i < record_count; ++i)
        {
            record_indices.emplace_back(extract<xiv_exd_logger, ExdRecordIndex>(stream, utils::log::Severity::trace));
        }

        for (auto& record_index: record_indices)
        {
            // Get the vector fields for the given record and preallocate it
            auto& fields = _data[record_index.id];
            fields.reserve(member_count);

            for (auto& member_entry: i_exh.get_members())
            {
                // Seek to the position of the member to extract.
                // 6 is because we have uint32_t/uint16_t at the start of each record
                stream.seekg(record_index.offset + 6 + member_entry.second.offset);

                // Switch depending on the type to extract
                switch (member_entry.second.type)
                {
                case DataType::string:
                    // Extract the offset to the actual string
                    // Seek to it then extract the actual string
                {
                    auto string_offset = extract<xiv_exd_logger, uint32_t>(stream, "string_offset", utils::log::Severity::trace, false);
                    stream.seekg(record_index.offset + 6 + i_exh.get_header().data_offset + string_offset);
                    fields.emplace_back(utils::bparse::extract_cstring<xiv_exd_logger>(stream, "string", utils::log::Severity::trace));
                }
                break;

                case DataType::boolean:
                    fields.emplace_back(extract<xiv_exd_logger, bool>(stream, "bool", utils::log::Severity::trace));
                    break;

                case DataType::int8:
                    fields.emplace_back(extract<xiv_exd_logger, int8_t>(stream, "int8_t", utils::log::Severity::trace));
                    break;

                case DataType::uint8:
                    fields.emplace_back(extract<xiv_exd_logger, uint8_t>(stream, "uint8_t", utils::log::Severity::trace));
                    break;

                case DataType::int16:
                    fields.emplace_back(extract<xiv_exd_logger, int16_t>(stream, "int16_t", utils::log::Severity::trace, false));
                    break;

                case DataType::uint16:
                    fields.emplace_back(extract<xiv_exd_logger, uint16_t>(stream, "uint16_t", utils::log::Severity::trace, false));
                    break;

                case DataType::int32:
                    fields.emplace_back(extract<xiv_exd_logger, int32_t>(stream, "int32_t", utils::log::Severity::trace, false));
                    break;

                case DataType::uint32:
                    fields.emplace_back(extract<xiv_exd_logger, uint32_t>(stream, "uint32_t", utils::log::Severity::trace, false));
                    break;

                case DataType::float32:
                    fields.emplace_back(extract<xiv_exd_logger, float>(stream, "float", utils::log::Severity::trace, false));
                    break;

                case DataType::uint64:
                    fields.emplace_back(extract<xiv_exd_logger, uint64_t>(stream, "uint64_t", utils::log::Severity::trace, false));
                    break;

                default:
                    throw std::runtime_error("Unknown DataType: " + std::to_string(static_cast<uint16_t>(member_entry.second.type)));
                    break;
                }
            }
        }
    }
}

Exd::~Exd()
{
}

const std::vector<Field>& Exd::get_row(uint32_t id)
{
    auto row_it = _data.find(id);
    if (row_it == _data.end())
    {
        throw std::runtime_error("Id not found: " + std::to_string(id));
    }

    return row_it->second;
}

// Get all rows
const std::map<uint32_t, std::vector<Field>>& Exd::get_rows()
{
    return _data;
}

class output_field : public boost::static_visitor<>
{
public:
    template <typename T>
    void operator()(T operand, std::ostream& o_stream, char i_delimiter) const
    {
        o_stream << i_delimiter << operand;
    }

    void operator()(int8_t operand, std::ostream& o_stream, char i_delimiter) const
    {
        o_stream << i_delimiter << static_cast<int16_t>(operand);
    }

    void operator()(uint8_t operand, std::ostream& o_stream, char i_delimiter) const
    {
        o_stream << i_delimiter << static_cast<uint16_t>(operand);
    }

    void operator()(const std::string& operand, std::ostream& o_stream, char i_delimiter) const
    {
        o_stream << i_delimiter;
        for (char c: operand)
        {
            if (isprint(static_cast<int>(static_cast<uint8_t>(c))))
            {
                o_stream << c;
            }
            else
            {
                // This saves the flags of the stream for a given scope, to be sure that manipulators are reset after the return
                boost::io::ios_all_saver ias(o_stream);
                o_stream << "\\x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint16_t>(static_cast<uint8_t>(c));
            }
        }
    }
};

// Get as csv
void Exd::get_as_csv(std::ostream& o_stream) const
{
    // tab delimited csv to avoid problems with commas in strings
    char delimiter = '\t';

    auto visitor = boost::bind(output_field(), _1, boost::ref(o_stream), delimiter);

    for (auto& row_entry: _data)
    {
        o_stream << row_entry.first;

        for (auto& field: row_entry.second)
        {
            boost::apply_visitor(visitor, field);
        }

        o_stream << '\n';
    }
}

}
}


#include <xiv/exd/exh.h>

#include <xiv/utils/stream.h>

#include <xiv/exd/logger.h>
#include <xiv/dat/File.h>

using xiv::utils::bparse::extract;

namespace xiv
{
namespace exd
{

Exh::Exh(const dat::File& i_file)
{
    // Get a stream from the file
    auto stream_ptr = utils::stream::get_istream(i_file.get_data_sections().front());
    auto& stream = *stream_ptr;

    // Extract header and skip to member definitions
    _header = extract<xiv_exd_logger, ExhHeader>(stream);
    stream.seekg(0x20);

    // Extract all the members and feed the _members map
    for (auto i = 0; i < _header.field_count; ++i)
    {
        auto member = extract<xiv_exd_logger, ExhMember>(stream);
        _members[member.offset] = member;
    }

    // Extract all the exd_defs
    _exd_defs.reserve(_header.exd_count);
    for (auto i = 0; i < _header.exd_count; ++i)
    {
        _exd_defs.emplace_back(extract<xiv_exd_logger, ExhExdDef>(stream));
    }

    // Extract all the languages
    _languages.reserve(_header.language_count);
    for (auto i = 0; i < _header.language_count; ++i)
    {
        _languages.emplace_back(Language(extract<xiv_exd_logger, uint16_t>(stream, "language")));
    }
}

Exh::~Exh()
{
}

const ExhHeader& Exh::get_header() const
{
    return _header;
}

const std::vector<ExhExdDef>& Exh::get_exd_defs() const
{
    return _exd_defs;
}

const std::vector<Language>& Exh::get_languages() const
{
    return _languages;
}

const std::map<uint32_t, ExhMember>& Exh::get_members() const
{
    return _members;
}

}
}

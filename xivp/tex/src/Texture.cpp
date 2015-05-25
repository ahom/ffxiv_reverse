#include <xiv/tex/Texture.h>

#include <xiv/utils/bparse.h>
#include <xiv/utils/stream.h>
#include <xiv/utils/conv.h>
#include <xiv/utils/zlib.h>

#include <xiv/dat/GameData.h>
#include <xiv/dat/File.h>

#include <xiv/tex/logger.h>

XIV_STRUCT((xiv)(tex), TexHeader,
           XIV_MEM(uint32_t, unknown1)
           XIV_MEM(TextureType,  type)
           XIV_MEM(uint16_t, unknown2)
           XIV_MEM(uint16_t, width)
           XIV_MEM(uint16_t, height)
           XIV_MEM(uint16_t, unknown3)
           XIV_MEM(uint16_t, mipmap_count));

namespace xiv
{
namespace tex
{

Texture::Texture(dat::GameData& i_game_data, const std::string& i_name) :
    _name(i_name)
{
    initialize(i_game_data.get_file(i_name));
}

Texture::Texture(std::unique_ptr<dat::File> i_file)
{
    initialize(std::move(i_file));
}

void Texture::initialize(std::unique_ptr<dat::File> i_file)
{
    if (i_file->get_data_sections().empty())
    {
        throw std::runtime_error("Trying to initialize Texture with empty file.");
    }

    auto& header_section = i_file->get_data_sections()[0];
    auto header_stream_ptr = utils::stream::get_istream(header_section);

    // Extract header
    TexHeader header = utils::bparse::extract<xiv_tex_logger, TexHeader>(*header_stream_ptr);

    _width = header.width;
    _height = header.height;
    _type = header.type;

    for (uint32_t i = 1; i < i_file->get_data_sections().size(); ++i)
    {
        auto& data_section = i_file->get_data_sections()[i];

        // Copy mipmap data
        _mipmap_data.emplace_back(data_section.size());
        std::copy(data_section.begin(), data_section.end(), _mipmap_data.back().begin());
    }
}

Texture::~Texture()
{
}

const std::string& Texture::get_name() const
{
    return _name;
}

const std::vector<std::vector<char>>& Texture::get_mipmap_data() const
{
    return _mipmap_data;
}

uint16_t Texture::get_width() const
{
    return _width;
}

uint16_t Texture::get_height() const
{
    return _height;
}

TextureType Texture::get_type() const
{
    return _type;
}

void Texture::export_as_json(const boost::filesystem::path& i_output_path) const
{
    auto json_output_path = i_output_path / "json";

    auto output_file_path = json_output_path / (_name + ".json");

    boost::filesystem::create_directories(output_file_path.parent_path());

    std::ofstream ofs(output_file_path.string());

    ofs << "{";

    ofs << "\"name\": \"" << _name << "\",";
    ofs << "\"width\": " << _width << ",";
    ofs << "\"height\": " << _height << ",";
    ofs << "\"type\": \"" << _type << "\",";

    ofs << "\"data\": \"";
    const std::vector<char>* input_buffer = &(_mipmap_data[0]);
    std::vector<char> new_input_buffer;
    if (_type == TextureType::RGBAF)
    {
        new_input_buffer.resize(input_buffer->size() * 2);
        const uint16_t* input_hfloats = reinterpret_cast<const uint16_t*>(input_buffer->data());
        float* output_floats = reinterpret_cast<float*>(new_input_buffer.data());
        for (size_t i = 0; i < input_buffer->size() / 2; ++i)
        {
            output_floats[i] = ::xiv::utils::conv::half2float(input_hfloats[i]);
        }
        input_buffer = &(new_input_buffer);
    }
    else
    {
        input_buffer = &(_mipmap_data[0]);
    }
    std::vector<char> compressed_buffer;
    utils::zlib::compress(*input_buffer, compressed_buffer);
    utils::conv::bin2base64(compressed_buffer, ofs);
    ofs << "\"";

    ofs << "}";
}

}
}

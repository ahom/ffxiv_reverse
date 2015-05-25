#include <xiv/mdl/Material.h>

#include <xiv/utils/bparse.h>
#include <xiv/utils/stream.h>

#include <xiv/dat/GameData.h>
#include <xiv/dat/File.h>

#include <xiv/tex/Texture.h>

#include <xiv/mdl/logger.h>

XIV_STRUCT((xiv)(mdl), MatHeader,
           XIV_MEM(uint32_t,	unknown1)
           XIV_MEM(uint32_t,	file_size)
           XIV_MEM(uint16_t, string_block_size)
           XIV_MEM(uint16_t, shpk_string_offset)
           XIV_MEM(uint8_t, tex_count)
           XIV_MEM(uint8_t, map_count)
           XIV_MEM(uint8_t, color_set_count)
           XIV_MEM(uint8_t, unknown2));

XIV_STRUCT((xiv) (mdl), MatStringOffset,
           XIV_MEM(uint16_t, offset)
           XIV_MEM(uint16_t, id));

using xiv::utils::bparse::extract;

namespace xiv
{
namespace mdl
{

Material::Material(dat::GameData& i_game_data, const std::string& i_name) :
    _name(i_name)
{
    auto file = i_game_data.get_file(_name);
    initialize(i_game_data, *file);
}

Material::Material(dat::GameData& i_game_data, const dat::File& i_file)
{
    initialize(i_game_data, i_file);
}

void Material::initialize(dat::GameData& i_game_data, const dat::File& i_file)
{
    auto file_stream_ptr = utils::stream::get_istream(i_file.get_data_sections()[0]);
    auto& file_stream = *file_stream_ptr;

    MatHeader header = extract<xiv_mdl_logger, MatHeader>(file_stream);

    std::vector<MatStringOffset> string_offsets;
    extract<xiv_mdl_logger>(file_stream, "string_offset", header.tex_count + header.map_count + header.color_set_count, string_offsets);

    std::streamoff strings_offset = file_stream.tellg();

    // Extracting textures
    _texs.reserve(header.tex_count);
    for (uint32_t i = 0; i < header.tex_count; ++i)
    {
        file_stream.seekg(strings_offset + string_offsets[i].offset);
        std::string tex_name = utils::bparse::extract_cstring<xiv_mdl_logger>(file_stream, "tex_name");

        if (tex_name == "dummy.tex")
        {
            tex_name = "common/graphics/texture/dummy.tex";
        }

        _texs.emplace_back(i_game_data, tex_name);
    }

    _maps.reserve(header.map_count);
    for (uint32_t i = 0; i < header.map_count; ++i)
    {
        file_stream.seekg(strings_offset + string_offsets[i + header.tex_count].offset);
        _maps.emplace_back(utils::bparse::extract_cstring<xiv_mdl_logger>(file_stream, "map_name"));
    }

    _color_sets.reserve(header.color_set_count);
    for (uint32_t i = 0; i < header.color_set_count; ++i)
    {
        file_stream.seekg(strings_offset + string_offsets[i + header.tex_count + header.map_count].offset);
        _color_sets.emplace_back(utils::bparse::extract_cstring<xiv_mdl_logger>(file_stream, "color_set"));
    }
}

Material::~Material()
{
}

const std::string& Material::get_name() const
{
    return _name;
}
const std::vector<tex::Texture>& Material::get_texs() const
{
    return _texs;
}

void Material::export_as_json(const boost::filesystem::path& i_output_path) const
{

    auto json_output_path = i_output_path / "json";

    auto output_file_path = json_output_path / (_name + ".json");

    boost::filesystem::create_directories(output_file_path.parent_path());

    std::ofstream ofs(output_file_path.string());

    ofs << "{";

    ofs << "\"name\": \"" << _name << "\",";

    ofs << "\"components\": {";
    for (uint32_t i = 0; i < _texs.size(); ++i)
    {
        if (i != 0)
        {
            ofs << ", ";
        }
        if (_texs[i].get_name().find("-v") != std::string::npos)
        {
            ofs << "\"table\": ";
        }
        else if (_texs[i].get_name().find("_d.tex") != std::string::npos)
        {
            ofs << "\"diffuse\": ";
        }
        else if (_texs[i].get_name().find("_s.tex") != std::string::npos)
        {
            ofs << "\"specular\": ";
        }
        else if (_texs[i].get_name().find("_n.tex") != std::string::npos)
        {
            ofs << "\"normal\": ";
        }
        else if (_texs[i].get_name().find("_m.tex") != std::string::npos)
        {
            ofs << "\"mask\": ";
        }
        else
        {
            ofs << "\"unknown\": ";
        }

        ofs << "\"" + _texs[i].get_name() + "\"";

        _texs[i].export_as_json(i_output_path);
    }
    ofs << "},";

    ofs << "\"maps\": [";
    for (uint32_t i = 0; i < _maps.size(); ++i)
    {
        if (i != 0)
        {
            ofs << ", ";
        }
        ofs << "\"" << _maps[i] << "\"";
    }
    ofs << "],";

    ofs << "\"color_sets\": [";
    for (uint32_t i = 0; i < _color_sets.size(); ++i)
    {
        if (i != 0)
        {
            ofs << ", ";
        }
        ofs << "\"" << _color_sets[i] << "\"";
    }
    ofs << "]";


    ofs << "}";

    ofs.close();
}

}
}


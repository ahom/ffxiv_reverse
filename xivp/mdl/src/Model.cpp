#include <xiv/mdl/Model.h>

#include <sstream>

#include <boost/format.hpp>

#include <xiv/utils/stream.h>
#include <xiv/utils/crc32.h>

#include <xiv/dat/GameData.h>
#include <xiv/dat/File.h>
#include <xiv/dat/Cat.h>
#include <xiv/dat/Index.h>

#include <xiv/tex/Texture.h>

#include <xiv/mdl/logger.h>
#include <xiv/mdl/Lod.h>
#include <xiv/mdl/Material.h>
#include <xiv/mdl/Mesh.h>

using xiv::utils::bparse::extract;

XIV_STRUCT((xiv)(mdl), MdlHeader,
           XIV_MEM(float,      unknown1)
           XIV_MEM(uint16_t,   mesh_count)
           XIV_MEM(uint16_t,   unknown_type1_count)
           XIV_MEM(uint16_t,   bones_link_count)
           XIV_MEM(uint16_t,   material_count)
           XIV_MEM(uint16_t,   bone_count)
           XIV_MEM(uint16_t,   unknown_type2_count)
           XIV_MEM(uint16_t,   unknown_type3_count)
           XIV_MEM(uint16_t,   unknown_type4_count)
           XIV_MEM(uint16_t,   unknown_type5_count)
           XIV_MEM(uint8_t,    unknown2)
           XIV_MEM(uint8_t,    unknown3)
           XIV_MEM(uint16_t,   unknown_type6_count)
           XIV_MEM(uint16_t,   unknown_type7_count)
           XIV_MEM(uint32_t,   unknown4)
           XIV_MEM(uint32_t,   unknown5)
           XIV_MEM(uint16_t,   unknown6)
           XIV_MEM(uint16_t,   unknown_type8_count)
           XIV_MEM(uint32_t,   unknown7)
           XIV_MEM(uint32_t,   unknown8)
           XIV_MEM(uint32_t,   unknown9)
           XIV_MEM(uint32_t,   unknown10));

XIV_STRUCT((xiv)(mdl), MdlUnknownType6,
           XIV_MEM(uint32_t,   unknown1)
           XIV_MEM(uint32_t,   unknown2)
           XIV_MEM(float,      unknown3)
           XIV_MEM(float,      unknown4)
           XIV_MEM(uint32_t,   unknown5)
           XIV_MEM(uint32_t,   unknown6)
           XIV_MEM(uint32_t,   unknown7)
           XIV_MEM(uint32_t,   unknown8));

XIV_STRUCT((xiv)(mdl), MdlUnknownType7,
           XIV_MEM(uint32_t,   unknown1)
           XIV_MEM(uint32_t,   unknown2)
           XIV_MEM(float,      unknown3)
           XIV_MEM(float,      unknown4)
           XIV_MEM(uint32_t,   unknown5)
           XIV_MEM(uint32_t,   unknown6)
           XIV_MEM(uint32_t,   unknown7)
           XIV_MEM(float,      unknown8));

XIV_STRUCT((xiv)(mdl), MdlUnknownType8,
           XIV_MEM(uint32_t,   indices_index)
           XIV_MEM(uint32_t,   indices_count)
           XIV_MEM(uint32_t,   index));

XIV_STRUCT((xiv)(mdl), MdlBonesLink,
           XIV_MEM(uint32_t,   indices_index)
           XIV_MEM(uint32_t,   indices_count)
           XIV_MEM(uint32_t,   index)
           XIV_MEM(int16_t,    bone_index)
           XIV_MEM(uint16_t,   bone_count));

namespace xiv
{
namespace mdl
{

Model::Model(dat::GameData& i_game_data, const std::string& i_name) :
    _name(i_name)
{
    auto file = i_game_data.get_file(i_name);
    initialize(i_game_data, *file, i_name);
}

void extract_strings(std::istream& i_stream, std::streamoff strings_offset, const std::vector<uint32_t>& i_offsets, std::vector<std::string>& o_strings)
{
    o_strings.reserve(i_offsets.size());
    for (auto offset : i_offsets)
    {
        i_stream.seekg(strings_offset + offset);
        o_strings.emplace_back(utils::bparse::extract_cstring<xiv_mdl_logger>(i_stream, "string"));
    }
}

void Model::initialize(dat::GameData& i_game_data, dat::File& i_file, const std::string& i_name)
{
    if (i_file.get_data_sections().empty())
    {
        throw std::runtime_error("Trying to initialize Model with empty file.");
    }
    else if (i_file.get_data_sections().size() < 0xB)
    {
        throw std::runtime_error("Invalid number of data_sections for Model: " + std::to_string(i_file.get_data_sections().size()));
    }

    auto& header_section = i_file.get_data_sections()[1];
    auto header_stream_ptr = utils::stream::get_istream(header_section);
    auto& header_stream = *header_stream_ptr;

    uint32_t string_count = extract<xiv_mdl_logger, uint32_t>(header_stream, "string_count");
    uint32_t string_block_size = extract<xiv_mdl_logger, uint32_t>(header_stream, "string_block_size");
    std::streamoff strings_offset = header_stream.tellg();

    // skipping string block
    header_stream.seekg(strings_offset + string_block_size);

    MdlHeader header = extract<xiv_mdl_logger, MdlHeader>(header_stream);

    // MdlUnknownType6
    std::vector<MdlUnknownType6> unknown_type6s;
    extract<xiv_mdl_logger>(header_stream, header.unknown_type6_count, unknown_type6s);

    // Lod
    std::vector<MdlLod> lods;
    extract<xiv_mdl_logger>(header_stream, 3, lods);

    // Meshes
    std::vector<MdlMesh> meshes;
    extract<xiv_mdl_logger>(header_stream, header.mesh_count, meshes);

    // UnknownType1 string offsets
    std::vector<uint32_t> unknown_type1_string_offsets;
    extract<xiv_mdl_logger>(header_stream, "unknown_type_1_string_offset", header.unknown_type1_count, unknown_type1_string_offsets);

    // MdlUnknownType7
    std::vector<MdlUnknownType7> unknown_type7s;
    extract<xiv_mdl_logger>(header_stream, header.unknown_type7_count, unknown_type7s);

    // BonesLinks
    std::vector<MdlBonesLink> bones_links;
    extract<xiv_mdl_logger>(header_stream, header.bones_link_count, bones_links);

    // MdlUnknownType8
    std::vector<MdlUnknownType8> unknown_type8s;
    extract<xiv_mdl_logger>(header_stream, header.unknown_type8_count, unknown_type8s);

    // Materials string offsets
    std::vector<uint32_t> material_string_offsets;
    extract<xiv_mdl_logger>(header_stream, "material_string_offset", header.material_count, material_string_offsets);

    // Getting materials paths and initializing them
    std::vector<std::string> material_paths;
    extract_strings(header_stream, strings_offset, material_string_offsets, material_paths);

    if (material_paths.empty())
    {
        throw std::runtime_error("No material found for model.");
    }

    _materials.reserve(material_paths.size());
    for (auto& material_path : material_paths)
    {
        _materials.emplace_back();
        auto& material = _materials.back();

        if (material_path[0] == '/')
        {
            material_path = i_name.substr(0, i_name.find("model/")) + "material" + material_path;
        }

        if (i_game_data.check_file_existence(material_path))
        {
            material.emplace(0, Material(i_game_data, material_path));
        }
        else
        {
            std::string dir_str_format;



            auto it = material_path.find("material/material/");
            if (it != std::string::npos)
            {
                dir_str_format = material_path;
                dir_str_format.replace(it, material_path.length() - it, "material/v%04d");
                XIV_INFO(xiv_mdl_logger, dir_str_format);
                if (!i_game_data.check_dir_existence(boost::str(boost::format(dir_str_format + "/") % 1)))
                {
                    throw std::runtime_error("No material found for model: " + material_path);
                }
            }
            else
            {
                it = material_path.find("material/");
                if (it != std::string::npos)
                {
                    dir_str_format = material_path;
                    dir_str_format.replace(it, material_path.length() - it, "v%04d");
                    if (!i_game_data.check_dir_existence(boost::str(boost::format(dir_str_format + "/") % 1)))
                    {
                        dir_str_format = material_path;
                        dir_str_format.replace(it, material_path.length() - it, "material/v%04d");
                        if (!i_game_data.check_dir_existence(boost::str(boost::format(dir_str_format + "/") % 1)))
                        {
                            throw std::runtime_error("No material found for model: " + material_path);
                        }
                    }
                    XIV_INFO(xiv_mdl_logger, dir_str_format);
                }
                else
                {
                    it = material_path.find_last_of("/");
                    if (it != std::string::npos)
                    {
                        dir_str_format = material_path;
                        dir_str_format.replace(it + 1, material_path.length() - it - 1, "v%04d");
                        XIV_INFO(xiv_mdl_logger, dir_str_format);
                        if (!i_game_data.check_dir_existence(boost::str(boost::format(dir_str_format + "/") % 1)))
                        {
                            throw std::runtime_error("No material found for model: " + material_path);
                        }
                    }
                    else
                    {
                        throw std::runtime_error("No material found for model: " + material_path);
                    }
                }
            }

            std::string dir_str_format_in = boost::str(boost::format(dir_str_format) % 0);

            std::vector<uint32_t> dir_crc_values;
            xiv::utils::crc32::generate_hashes_1(dir_str_format_in, dir_str_format_in.size() - 4, dir_crc_values);

            auto& chara_cat = i_game_data.get_category("chara");
            auto& cat_hash_table = chara_cat.get_index().get_hash_table();

            for (uint32_t v = 0; v < 10000; ++v)
            {
                if (cat_hash_table.find(dir_crc_values[v]) != cat_hash_table.end())
                {
                    it = material_path.find_last_of("/");
                    std::string full_path = boost::str(boost::format(dir_str_format + material_path.substr(it)) % v);
                    if (i_game_data.check_file_existence(full_path))
                    {
                        material.emplace(v, Material(i_game_data, full_path));
                    }
                }
            }
        }
    }

    auto& mesh_headers_section = i_file.get_data_sections()[0];
    std::vector<std::vector<char>> mesh_headers;

    if (mesh_headers_section.size() != 0x88 * header.mesh_count)
    {
        throw std::runtime_error("mesh_headers_section size is not valid.");
    }

    mesh_headers.resize(header.mesh_count);
    for (uint16_t i = 0; i < header.mesh_count; ++i)
    {
        mesh_headers[i].resize(0x88);
        std::copy(mesh_headers_section.data() + 0x88 * i, mesh_headers_section.data() + 0x88 * (i+1), mesh_headers[i].begin());
    }

    _lods.reserve(3);
    for (int i = 0; i < 3; ++i)
    {
        _lods.emplace_back(Lod(lods[i], meshes, mesh_headers, i_file.access_data_sections()[2 + i], i_file.access_data_sections()[8 + i]));
    }
}

Model::~Model()
{
}

void Model::export_as_json(const boost::filesystem::path& i_output_path) const
{
    auto json_output_path = i_output_path / "json";

    auto output_file_path = json_output_path / (_name + ".json");

    boost::filesystem::create_directories(output_file_path.parent_path());

    std::ofstream ofs(output_file_path.string());

    ofs << "{";
    ofs << "\"name\": \"" << _name << "\",";

    _lods[0].export_as_json(ofs);

    ofs  << ",";

    ofs << "\"materials\": [";
    for (uint32_t i = 0; i < _materials.size(); ++i)
    {
        if (i != 0)
        {
            ofs << ",";
        }
        ofs << "{";
        for (auto material_entry_it = _materials[i].begin(); material_entry_it != _materials[i].end(); ++material_entry_it)
        {
            if (material_entry_it != _materials[i].begin())
            {
                ofs << ",";
            }
            ofs << "\"" << material_entry_it->first << "\": \"" << material_entry_it->second.get_name() << "\"";
            material_entry_it->second.export_as_json(i_output_path);
        }
        ofs << "}";
    }
    ofs << "]";

    ofs << "}";
    ofs.close();
}

const std::string& Model::get_name() const
{
    return _name;
}

const std::vector<Lod>& Model::get_lods() const
{
    return _lods;
}
const std::vector<std::unordered_map<uint32_t, Material>>& Model::get_materials() const
{
    return _materials;
}

}
}

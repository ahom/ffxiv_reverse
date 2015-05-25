#ifndef XIV_MDL_MODEL_H
#define XIV_MDL_MODEL_H

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include <xiv/utils/bparse.h>

XIV_STRUCT((xiv)(mdl), MdlLod,
           XIV_MEM(uint16_t, mesh_index)
           XIV_MEM(uint16_t, mesh_count)
           XIV_MEM(float,    unknown1)
           XIV_MEM(float,    unknown2)
           XIV_MEM(uint16_t, mesh_index_2)
           XIV_MEM(uint16_t, mesh_count_2)
           XIV_MEM(uint32_t, next_mesh_index)
           XIV_MEM(uint16_t, index)
           XIV_MEM(uint16_t, unknown3)
           XIV_MEM(uint32_t, unknown4)
           XIV_MEM(uint32_t, unknown5)
           XIV_MEM(uint32_t, unknown6)
           XIV_MEM(uint32_t, unknown7)
           XIV_MEM(uint32_t, unknown8)
           XIV_MEM(uint32_t, vertex_buffer_size)
           XIV_MEM(uint32_t, index_buffer_size)
           XIV_MEM(uint32_t, vertex_buffer_offset)
           XIV_MEM(uint32_t, index_buffer_offset));

XIV_STRUCT((xiv)(mdl), MdlMesh,
           XIV_MEM(uint32_t, vert_buf_count)
           XIV_MEM(uint32_t, indices_count)
           XIV_MEM(uint16_t, material_index)
           XIV_MEM(uint16_t, bones_links_index)
           XIV_MEM(uint16_t, bones_links_count)
           XIV_MEM(uint16_t, lod_level)
           XIV_MEM(uint32_t, indices_index)
           XIV_MEM(uint32_t, vert_buf_offset_stream_0)
           XIV_MEM(uint32_t, vert_buf_offset_stream_1)
           XIV_MEM(uint32_t, unknown1)
           XIV_MEM(uint8_t,  vert_buf_size_stream_0)
           XIV_MEM(uint8_t,  vert_buf_size_stream_1)
           XIV_MEM(uint16_t, unknown2));

namespace xiv
{
namespace dat
{
class File;
class GameData;
}
namespace mdl
{

class Lod;
class Material;

class Model
{
public:
    Model(dat::GameData& i_game_data, const std::string& i_name);
    ~Model();

    const std::string& get_name() const;

    const std::vector<Lod>& get_lods() const;
    const std::vector<std::unordered_map<uint32_t, Material>>& get_materials() const;

    void export_as_json(const boost::filesystem::path& i_output_path) const;

protected:
    std::string _name;

    std::vector<Lod> _lods;
    std::vector<std::unordered_map<uint32_t, Material>> _materials;

private:
    void initialize(dat::GameData& i_game_data, dat::File& i_file, const std::string& i_name);
};

}
}

#endif // XIV_MDL_MODEL_H

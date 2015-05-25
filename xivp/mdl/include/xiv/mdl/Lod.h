#ifndef XIV_MDL_LOD_H
#define XIV_MDL_LOD_H

#include <vector>
#include <ostream>
#include <unordered_map>

#include <xiv/utils/bparse.h>

XIV_ENUM((xiv) (mdl), ElementType, uint8_t,
         XIV_VALUE(float32_3, 0x02)
         XIV_VALUE(ubyte4,    0x05)
         XIV_VALUE(ubyte4n,   0x08)
         XIV_VALUE(float16_4, 0x0E));

XIV_ENUM((xiv) (mdl), ElementUsage, uint8_t,
         XIV_VALUE(position,		 0x00)
         XIV_VALUE(blend_weight,  0x01)
         XIV_VALUE(blend_indices, 0x02)
         XIV_VALUE(normal,		 0x03)
         XIV_VALUE(tex_coord,	 0x04)
         XIV_VALUE(binormal,	     0x06)
         XIV_VALUE(color,		 0x07));

XIV_STRUCT((xiv) (mdl), MeshVertexElement,
           XIV_MEM(uint8_t, stream_id)
           XIV_MEM(uint8_t, offset)
           XIV_MEM(ElementType, type)
           XIV_MEM(ElementUsage, usage)
           XIV_MEM(uint32_t, unknown));

namespace xiv
{
namespace mdl
{

class Mesh;
struct MdlLod;
struct MdlMesh;

class Lod
{
public:
    Lod(const MdlLod& i_lod,
        const std::vector<MdlMesh>& i_meshes,
        const std::vector<std::vector<char>>& i_mesh_headers,
        std::vector<char>& i_vertex_buffer_streams,
        std::vector<char>& i_index_buffer);

    ~Lod();

    const std::vector<Mesh>& get_meshes() const;
    const std::vector<char>& get_vertex_buffer_streams() const;
    const std::vector<char>& get_index_buffer() const;

    void export_as_json(std::ostream& o_stream) const;

private:
    uint32_t export_vertex_element_as_json(std::ostream& o_stream, const MeshVertexElement& i_element, uint32_t i_vertex_buffer_stride, uint32_t i_current_offset, std::istream& i_vertex_buffer, std::ostream& io_vertex_buffer) const;
    template <typename In, typename Out> void export_vertex_data(const MeshVertexElement& i_element, uint32_t i_vertex_buffer_stride, uint32_t i_current_offset, std::istream& i_vertex_buffer, std::ostream& io_vertex_buffer) const;

protected:
    std::vector<Mesh> _meshes;
    std::vector<char> _vertex_buffer_streams;
    std::vector<uint32_t> _vertex_sizes;
    uint32_t _vertex_count;
    std::vector<char> _index_buffer;

    std::unordered_map<ElementUsage, MeshVertexElement> _vertex_element_map;
};

}
}

#endif // XIV_MDL_LOD_H

#ifndef XIV_MDL_MESH_H
#define XIV_MDL_MESH_H

#include <ostream>
#include <cstdint>

namespace xiv
{
namespace mdl
{

struct MdlMesh;

class Mesh
{
public:
    Mesh(const MdlMesh& i_mesh);
    ~Mesh();

    void export_as_json(std::ostream& o_stream) const;

    uint32_t get_vertex_count() const;
    uint32_t get_vertex_buffer_offset_0() const;
    uint32_t get_vertex_buffer_offset_1() const;

protected:
    uint32_t _material;

    uint32_t _vertex_buffer_offset_0;
    uint32_t _vertex_buffer_offset_1;

    uint32_t _vertex_index;
    uint32_t _vertex_count;

    uint32_t _index_count;
    uint32_t _index_index;
};

}
}

#endif // XIV_MDL_MESH_H

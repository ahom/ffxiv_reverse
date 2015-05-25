#include <xiv/mdl/Mesh.h>

#include <xiv/mdl/Model.h>

namespace xiv
{
namespace mdl
{

Mesh::Mesh(const MdlMesh& i_mesh) :
    _material(i_mesh.material_index),
    _vertex_buffer_offset_0(i_mesh.vert_buf_offset_stream_0),
    _vertex_buffer_offset_1(i_mesh.vert_buf_offset_stream_1),
    _vertex_index(i_mesh.vert_buf_offset_stream_0 / (i_mesh.vert_buf_size_stream_0 + i_mesh.vert_buf_size_stream_1)),
    _vertex_count(i_mesh.vert_buf_count),
    _index_count(i_mesh.indices_count),
    _index_index(i_mesh.indices_index)
{
}

Mesh::~Mesh()
{

}

uint32_t Mesh::get_vertex_count() const
{
    return _vertex_count;
}
uint32_t Mesh::get_vertex_buffer_offset_0() const
{
    return _vertex_buffer_offset_0;
}
uint32_t Mesh::get_vertex_buffer_offset_1() const
{
    return _vertex_buffer_offset_1;
}

void Mesh::export_as_json(std::ostream& o_stream) const
{
    o_stream << "{";

    o_stream << "\"index_index\": " << _index_index << ", ";
    o_stream << "\"index_count\": " << _index_count << ", ";

    o_stream << "\"vertex_index\": " << _vertex_index << ", ";
    o_stream << "\"vertex_count\": " << _vertex_count << ", ";
    o_stream << "\"material\": " << _material;

    o_stream << "}";
}

}
}

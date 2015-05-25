#include <xiv/mdl/Lod.h>

#include <xiv/utils/stream.h>
#include <xiv/utils/conv.h>
#include <xiv/utils/zlib.h>

#include <xiv/mdl/logger.h>
#include <xiv/mdl/Model.h>
#include <xiv/mdl/Mesh.h>

XIV_STRUCT((xiv)(mdl), VtxFloat16_4,
           XIV_MEM_ARR(uint16_t, values, 4));

XIV_STRUCT((xiv) (mdl), VtxFloat32_2,
           XIV_MEM_ARR(float, values, 2));
XIV_STRUCT((xiv) (mdl), VtxFloat32_3,
           XIV_MEM_ARR(float, values, 3));
XIV_STRUCT((xiv)(mdl), VtxFloat32_4,
           XIV_MEM_ARR(float, values, 4));

XIV_STRUCT((xiv) (mdl), UByte_4,
           XIV_MEM_ARR(uint8_t, values, 4));

namespace xiv
{
namespace mdl
{

Lod::Lod(const MdlLod& i_lod,
         const std::vector<MdlMesh>& i_meshes,
         const std::vector<std::vector<char>>& i_mesh_headers,
         std::vector<char>& i_vertex_buffer_streams,
         std::vector<char>& i_index_buffer) :
    _vertex_buffer_streams(std::move(i_vertex_buffer_streams)),
    _index_buffer(std::move(i_index_buffer))
{
    // compares that we have the same mesh_header for all meshes of this lod
    auto& first_mesh_header = i_mesh_headers[i_lod.mesh_index];
    for (uint16_t i = 1; i < i_lod.mesh_count; ++i)
    {
        if (!std::equal(first_mesh_header.begin(), first_mesh_header.end(), i_mesh_headers[i_lod.mesh_index + i].begin()))
        {
            throw std::runtime_error("Different mesh_headers in same lod.");
        }
    }

    // compares that we have the same vertex sizes
    for (uint16_t i = 0; i < i_lod.mesh_count; ++i)
    {
        if (!((i_meshes[0].vert_buf_size_stream_0 == i_meshes[i].vert_buf_size_stream_0)
                && (i_meshes[0].vert_buf_size_stream_1 == i_meshes[i].vert_buf_size_stream_1)))
        {
            throw std::runtime_error("Different vertex sizes in same lod.");
        }
    }

    _vertex_sizes.push_back(i_meshes[0].vert_buf_size_stream_0);
    _vertex_sizes.push_back(i_meshes[0].vert_buf_size_stream_1);

    _vertex_count = _vertex_buffer_streams.size() / (i_meshes[0].vert_buf_size_stream_0 + i_meshes[0].vert_buf_size_stream_1);

    _meshes.reserve(i_lod.mesh_count);
    for (uint16_t i = 0; i < i_lod.mesh_count; ++i)
    {
        _meshes.emplace_back(i_meshes[i_lod.mesh_index + i]);
    }

    auto header_stream = utils::stream::get_istream(first_mesh_header);
    do
    {
        MeshVertexElement mesh_vertex_element = utils::bparse::extract<xiv_mdl_logger, MeshVertexElement>(*header_stream);
        if (mesh_vertex_element.stream_id == 0xFF)
        {
            break;
        }
        _vertex_element_map[mesh_vertex_element.usage] = mesh_vertex_element;
    } while (!header_stream->eof());
}

Lod::~Lod()
{
}

const std::vector<Mesh>& Lod::get_meshes() const
{
    return _meshes;
}
const std::vector<char>& Lod::get_vertex_buffer_streams() const
{
    return _vertex_buffer_streams;
}
const std::vector<char>& Lod::get_index_buffer() const
{
    return _index_buffer;
}

template <typename In, typename Out> struct Convert {};

template <typename In>
struct Convert<In, In>
{
    void operator()(const In& i_struct, In& o_struct) const
    {
        o_struct = i_struct;
    }
};

template <>
struct Convert<VtxFloat16_4, VtxFloat32_4>
{
    void operator()(const VtxFloat16_4& i_struct, VtxFloat32_4& o_struct) const
    {
        for (uint32_t i = 0; i < 4; ++i)
        {
            o_struct.values[i] = utils::conv::half2float(i_struct.values[i]);
        }
    }
};

template <typename In, typename Out>
void Lod::export_vertex_data(const MeshVertexElement& i_element, uint32_t i_vertex_buffer_stride, uint32_t i_current_offset, std::istream& i_vertex_buffer, std::ostream& io_vertex_buffer) const
{
    In in_struct;
    Out out_struct;

    uint32_t start_vertex_index = 0;
    for (auto& mesh : _meshes)
    {
        uint32_t base_offset = (i_element.stream_id == 0) ? mesh.get_vertex_buffer_offset_0() : mesh.get_vertex_buffer_offset_1();
        for (uint32_t i = 0; i < mesh.get_vertex_count(); ++i)
        {
            i_vertex_buffer.seekg(base_offset + i * _vertex_sizes[i_element.stream_id] + i_element.offset);
            io_vertex_buffer.seekp(i_current_offset + (i + start_vertex_index) * i_vertex_buffer_stride);

            utils::bparse::extract<xiv_mdl_logger>(i_vertex_buffer, in_struct, utils::log::Severity::trace);
            Convert<In, Out>()(in_struct, out_struct);
            XIV_TRACE(xiv_mdl_logger, "in: " << in_struct << " - out: " << out_struct);

            io_vertex_buffer.write(reinterpret_cast<char*>(&out_struct), sizeof(Out));
        }
        start_vertex_index += mesh.get_vertex_count();
    }
}

uint32_t Lod::export_vertex_element_as_json(std::ostream& o_stream, const MeshVertexElement& i_element, uint32_t i_vertex_buffer_stride, uint32_t i_current_offset, std::istream& i_vertex_buffer, std::ostream& io_vertex_buffer) const
{
    uint32_t return_value = 0;

    o_stream << "\"" << i_element.usage << "\": ";
    o_stream << "{";
    o_stream << "\"offset\": " << i_current_offset << ", ";

    switch (i_element.type)
    {
    case ElementType::float16_4:
        export_vertex_data<VtxFloat16_4, VtxFloat32_4>(i_element, i_vertex_buffer_stride, i_current_offset, i_vertex_buffer, io_vertex_buffer);
        return_value = sizeof(VtxFloat32_4);
        o_stream << "\"type\": \"float\", ";
        o_stream << "\"size\": 4";
        break;

    case ElementType::float32_3:
        export_vertex_data<VtxFloat32_3, VtxFloat32_3>(i_element, i_vertex_buffer_stride, i_current_offset, i_vertex_buffer, io_vertex_buffer);
        return_value = sizeof(VtxFloat32_3);
        o_stream << "\"type\": \"float\", ";
        o_stream << "\"size\": 3";
        break;

    case ElementType::ubyte4n:
    case ElementType::ubyte4:
        export_vertex_data<UByte_4, UByte_4>(i_element, i_vertex_buffer_stride, i_current_offset, i_vertex_buffer, io_vertex_buffer);
        return_value = sizeof(UByte_4);
        o_stream << "\"type\": \"ubyte\", ";
        o_stream << "\"size\": 4";
        break;

    default:
        XIV_ERROR(xiv_mdl_logger, "Unknown ElementType: " << i_element.type);
        throw std::runtime_error("Error while converting vertex_buffer");
        break;
    }

    o_stream << "}";
    return return_value;
}

void Lod::export_as_json(std::ostream& o_stream) const
{
    auto vertex_istream_ptr = utils::stream::get_istream(_vertex_buffer_streams);
    auto& vertex_istream = *vertex_istream_ptr;

    uint32_t vertex_buffer_stride = 0;
    for (auto& vertex_element: _vertex_element_map)
    {
        switch (vertex_element.second.type)
        {
        case ElementType::float16_4:
            vertex_buffer_stride += sizeof(VtxFloat32_4);
            break;

        case ElementType::float32_3:
            vertex_buffer_stride += sizeof(VtxFloat32_3);
            break;

        case ElementType::ubyte4n:
        case ElementType::ubyte4:
            vertex_buffer_stride += sizeof(UByte_4);
            break;
        }
    }

    std::vector<char> vertex_buffer;
    vertex_buffer.resize(vertex_buffer_stride * _vertex_count);

    auto vertex_ostream_ptr = utils::stream::get_ostream(vertex_buffer);
    auto& vertex_ostream = *vertex_ostream_ptr;

    o_stream << "\"elements\": {";
    uint32_t current_offset = 0;
    for (auto& vertex_element : _vertex_element_map)
    {
        if (current_offset != 0)
        {
            o_stream << ", ";
        }
        current_offset += export_vertex_element_as_json(o_stream, vertex_element.second, vertex_buffer_stride, current_offset, vertex_istream, vertex_ostream);

    }
    o_stream << "}, ";

    o_stream << "\"vertex_buffer\": \"";
    std::vector<char> vertex_compressed_buffer;
    utils::zlib::compress(vertex_buffer, vertex_compressed_buffer);
    utils::conv::bin2base64(vertex_compressed_buffer, o_stream);
    o_stream << "\", ";

    o_stream << "\"index_buffer\": \"";
    std::vector<char> index_compressed_buffer;
    utils::zlib::compress(_index_buffer, index_compressed_buffer);
    utils::conv::bin2base64(index_compressed_buffer, o_stream);
    o_stream << "\", ";

    o_stream << "\"stride\": " << vertex_buffer_stride << ",";

    o_stream << "\"meshes\": [";
    for (uint32_t i = 0; i < _meshes.size(); ++i)
    {
        if (i != 0)
        {
            o_stream << ", ";
        }
        _meshes[i].export_as_json(o_stream);
    }
    o_stream << "]";
}

}
}

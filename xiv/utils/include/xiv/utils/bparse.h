#ifndef XIV_UTILS_BPARSE_H
#define XIV_UTILS_BPARSE_H

#include <type_traits>
#include <iomanip>
#include <sstream>
#include <vector>

#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/io/ios_state.hpp>

#include <xiv/utils/log.h>

/*
BEWARE: wall of macros.
These macros are used to automatically take care of:

- Definition of structures that are meant to be read from/written to streams
- Automatic recursive handling of byteswap (called reordering here) between be/le (assumption is host being le here... don't think this code will run elsewhere)
- Automatic creation of << operators for said structures, mainly for logging purposes

e.g.:

XIV_STRUCT((xiv)(exd), ExhHeader,
    XIV_MEM_ARR(uint8_t, magic, 4)
    XIV_MEM_BE(uint16_t, unknown)
    XIV_MEM_BE(uint16_t, data_offset)
    XIV_MEM(uint16_t, field_count)
    XIV_MEM_BE(uint16_t, exd_count)
    XIV_MEM_BE(uint16_t, language_count));
*/

// Member expansion for structure definition
#define XIV_STRUCT_DEF_MEMBER_IMPL(type, name, swap, size_existence, size) type name BOOST_PP_IF(size_existence, [size],);
#define XIV_STRUCT_DEF_MEMBER(r, data, elem) XIV_STRUCT_DEF_MEMBER_IMPL elem

// Member expansion for reordering definition
#define XIV_STRUCT_REORD_MEMBER_IMPL(type, name, swap, size_existence, size) \
    BOOST_PP_IF(swap, \
        BOOST_PP_IF(size_existence, \
            for (auto i = 0; i < size; ++i) \
            { \
                i_struct.name[i] = xiv::utils::bparse::byteswap(i_struct.name[i]); \
            }, \
            i_struct.name = xiv::utils::bparse::byteswap(i_struct.name);),) \
    BOOST_PP_IF(size_existence, \
        for (auto i = 0; i < size; ++i) \
        { \
            xiv::utils::bparse::reorder(i_struct.name[i]); \
        }, \
        xiv::utils::bparse::reorder(i_struct.name);)
#define XIV_STRUCT_REORD_MEMBER(r, data, elem) XIV_STRUCT_REORD_MEMBER_IMPL elem

// Member expansion for output definition
#define XIV_STRUCT_OUT_MEMBER_IMPL(type, name, swap, size_existence, size) \
    o_stream << "[" #name << ":"; \
    BOOST_PP_IF(size_existence, \
        o_stream << "[ "; \
        for (auto i = 0; i < size; ++i) \
        { \
            xiv::utils::bparse::output(o_stream, i_struct.name[i]) << " "; \
        } \
        o_stream << "]";, \
        xiv::utils::bparse::output(o_stream, i_struct.name);) \
    o_stream << "]";
#define XIV_STRUCT_OUT_MEMBER(r, data, elem) XIV_STRUCT_OUT_MEMBER_IMPL elem

// Helper macros for namespace definition
#define XIV_BEGIN_NAMESPACE(r, data, elem) namespace elem {
#define XIV_END_NAMESPACE(r, data, elem) }

// Helper macro for namespaces concatenation
#define XIV_NAMESPACE(r, data, elem) BOOST_PP_CAT(data, BOOST_PP_CAT(elem, ::))

// Helper macro for structure inside namespace usage
// XIV_CLASS_NAME((xiv)(dat), Struct) => xiv::dat::Struct
#define XIV_CLASS_NAME(namespaces, struct_name) BOOST_PP_CAT(BOOST_PP_SEQ_FOLD_LEFT(XIV_NAMESPACE,, namespaces), struct_name)

// Macro to be used to define a struct
#define XIV_STRUCT(namespaces, struct_name, members) \
    BOOST_PP_SEQ_FOR_EACH(XIV_BEGIN_NAMESPACE, ~, namespaces) \
    __pragma(pack(push, 1)) \
    struct struct_name \
    { \
        BOOST_PP_SEQ_FOR_EACH(XIV_STRUCT_DEF_MEMBER, ~, members) \
    }; \
    __pragma(pack(pop)) \
    inline std::ostream& operator<<(std::ostream& o_stream, const struct_name& i_struct) \
    {\
        o_stream << #struct_name "("; \
        BOOST_PP_SEQ_FOR_EACH(XIV_STRUCT_OUT_MEMBER, ~,  members) \
        o_stream << ")"; \
        return o_stream; \
    } \
    BOOST_PP_SEQ_FOR_EACH(XIV_END_NAMESPACE, ~, namespaces) \
    namespace xiv { namespace utils { namespace bparse { \
    template <> inline void reorder<XIV_CLASS_NAME(namespaces, struct_name)>(XIV_CLASS_NAME(namespaces, struct_name)& i_struct) \
    { \
        BOOST_PP_SEQ_FOR_EACH(XIV_STRUCT_REORD_MEMBER, ~, members) \
    } \
    }}}

// Various macros to define the members of a struct
// ARR = array
// LE = little endian
// BE = big endian
// type/name/endian/size_existence/size
#define XIV_MEM_ARR_LE(type, name, size) ((type, name, 0, 1, size))
#define XIV_MEM_ARR_BE(type, name, size) ((type, name, 1, 1, size))
#define XIV_MEM_ARR(type, name, size) XIV_MEM_ARR_LE(type, name, size)

#define XIV_MEM_LE(type, name) ((type, name, 0, 0, 0))
#define XIV_MEM_BE(type, name) ((type, name, 1, 0, 0))

#define XIV_MEM(type, name) XIV_MEM_LE(type, name)

// Macro expansion for enum definition
#define XIV_ENUM_DEF_VALUE_IMPL(name, value) name = value,
#define XIV_ENUM_DEF_VALUE(r, data, elem) XIV_ENUM_DEF_VALUE_IMPL elem

// Macro expansion for output definition
#define XIV_ENUM_OUT_VALUE_IMPL(name, value) name: o_stream << #name ; break;
#define XIV_ENUM_OUT_VALUE(r, data, elem) case XIV_CLASS_NAME((data), XIV_ENUM_OUT_VALUE_IMPL elem)

// Macro to be used to define a enum
#define XIV_ENUM(namespaces, enum_name, enum_type, values) \
    BOOST_PP_SEQ_FOR_EACH(XIV_BEGIN_NAMESPACE, ~, namespaces) \
    enum class enum_name: enum_type \
    { \
        BOOST_PP_SEQ_FOR_EACH(XIV_ENUM_DEF_VALUE, ~, values) \
    }; \
    inline std::ostream& operator<<(std::ostream& o_stream, enum_name i_value) \
    {\
		enum_type value = static_cast<enum_type>(i_value); \
        switch(i_value) \
        { \
            BOOST_PP_SEQ_FOR_EACH(XIV_ENUM_OUT_VALUE, enum_name, values) \
            default: \
				o_stream << "("; \
				xiv::utils::bparse::output(o_stream, value); \
				o_stream << ") "; \
                o_stream << "UNKNOWN"; \
                break; \
        } \
        return o_stream; \
    } \
    BOOST_PP_SEQ_FOR_EACH(XIV_END_NAMESPACE, ~, namespaces)

#define XIV_VALUE(name, value) ((name, value))

namespace xiv
{
namespace utils
{
namespace bparse
{

// Internal macro for byteswapping
template <int N>
void byteswap_impl(char (&bytes)[N])
{
    for (auto p = std::begin(bytes), end = std::end(bytes) - 1; p < end; ++p, --end)
    {
        std::swap(*p, *end);
    }
}

// byteswapping any type (no pointers to array)
template <typename T>
T byteswap(T value)
{
    byteswap_impl(*reinterpret_cast<char (*)[sizeof(T)]>(&value));
    return value;
}

// Read a struct from a stream
template <typename StructType>
void read(std::istream& i_stream, StructType& i_struct)
{
    static_assert(std::is_pod<StructType>::value, "StructType must be a POD to be able to use read.");
    i_stream.read(reinterpret_cast<char*>(&i_struct), sizeof(StructType));
}

// By default a type does not need reordering
template <typename StructType> void reorder(StructType& i_struct) {}

// This should not copy because of RVO
// Extract a struct from a stream and log it
template <typename LoggerType, typename StructType>
StructType extract(std::istream& i_stream, xiv::utils::log::Severity i_severity = xiv::utils::log::Severity::debug)
{
    StructType temp_struct;
    extract<LoggerType>(i_stream, temp_struct, i_severity);
    return temp_struct;
}

// "Overload" for passed struct as arg
// Usage of XIV_DEBUG_LOG macros prevent logging of this in release mode, even if severity is correct
// We do not want to compile this in release because the overhead is too big
template <typename LoggerType, typename StructType>
void extract(std::istream& i_stream, StructType& o_struct, xiv::utils::log::Severity i_severity = xiv::utils::log::Severity::debug)
{
    read(i_stream, o_struct);
    reorder(o_struct);
    XIV_DEBUG_LOG(LoggerType, i_severity, "Extracted: " << o_struct);
}

template <typename LoggerType, typename StructType>
void extract(std::istream& i_stream, uint32_t i_size, std::vector<StructType>& o_structs, xiv::utils::log::Severity i_severity = xiv::utils::log::Severity::debug)
{
    o_structs.reserve(i_size);
    for (uint32_t i = 0; i < i_size; ++i)
    {
        o_structs.emplace_back(extract<LoggerType, StructType>(i_stream, i_severity));
    }
}

// For simple (integral) types just provide name and endianness directly
template <typename LoggerType, typename StructType>
StructType extract(std::istream& i_stream, const std::string& i_name, xiv::utils::log::Severity i_severity = xiv::utils::log::Severity::debug, bool i_is_le = true)
{
    StructType temp_struct;
    read(i_stream, temp_struct);
    if (!i_is_le)
    {
        temp_struct = byteswap(temp_struct);
    }
    XIV_DEBUG_LOG(LoggerType, i_severity, "Extracted: [" << i_name << ":" << temp_struct << "]");
    return temp_struct;
}

template <typename LoggerType, typename StructType>
void extract(std::istream& i_stream, const std::string& i_name, uint32_t i_size, std::vector<StructType>& o_structs, xiv::utils::log::Severity i_severity = xiv::utils::log::Severity::debug, bool i_is_le = true)
{
    o_structs.reserve(i_size);
    for (uint32_t i = 0; i < i_size; ++i)
    {
        o_structs.emplace_back(extract<LoggerType, StructType>(i_stream, i_name, i_severity));
    }
}

// For cstrings
template <typename LoggerType>
std::string extract_cstring(std::istream& i_stream, const std::string& i_name, xiv::utils::log::Severity i_severity = xiv::utils::log::Severity::debug)
{
    std::string temp_str;
    std::getline(i_stream, temp_str, '\0');
    XIV_DEBUG_LOG(LoggerType, i_severity, "Extracted: [" << i_name << ":" << temp_str << "]");
    return temp_str;
}

// Control the output of all types, default do the default <<
template <typename Type>
inline std::ostream& output(std::ostream& o_stream, Type& io_value)
{
    return o_stream << io_value;
}

// for char output the char if printable, else just \xXX's it
inline std::ostream& output(std::ostream& o_stream, char c)
{
    if (isprint(c))
    {
        return o_stream << c;
    }
    else
    {
        // This saves the flags of the stream for a given scope, to be sure that manipulators are reset after the return
        boost::io::ios_all_saver ias(o_stream);
        return o_stream << "\\x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(c);
    }
}

inline std::ostream& output(std::ostream& o_stream, uint8_t c)
{
    return o_stream << static_cast<uint16_t>(c);
}

}
}
}

#endif // XIV_UTILS_BPARSE_H

#ifndef XIV_UTILS_LOG_H
#define XIV_UTILS_LOG_H

#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/record_ostream.hpp>

#define XIV_LOG(logger, severity, msg) BOOST_LOG_SEV(logger::get(), severity) << msg

//This prevents trace/debug msg to be compiled in release mode
#ifndef NDEBUG
#define XIV_DEBUG_LOG(logger, severity, msg) XIV_LOG(logger, severity, msg)
#else
#define XIV_DEBUG_LOG(logger, severity, msg)
#endif

// Helper macros to log on a specific logger with a severity
#define XIV_TRACE(logger, msg)   XIV_DEBUG_LOG(logger, xiv::utils::log::Severity::trace, msg)
#define XIV_DEBUG(logger, msg)   XIV_DEBUG_LOG(logger, xiv::utils::log::Severity::debug, msg)
#define XIV_INFO(logger, msg)    XIV_LOG(logger, xiv::utils::log::Severity::info, msg)
#define XIV_WARNING(logger, msg) XIV_LOG(logger, xiv::utils::log::Severity::warning, msg)
#define XIV_ERROR(logger, msg)   XIV_LOG(logger, xiv::utils::log::Severity::error, msg)
#define XIV_FATAL(logger, msg)   XIV_LOG(logger, xiv::utils::log::Severity::fatal, msg)

namespace xiv
{
namespace utils
{
namespace log
{

/*
Logging severities:
trace => full debugging at the expense of log speed, usually you find in trace the debug logs that are in tight loops
debug => normal debugging
info => informative logging, first level in release mode usually
warning => usually unexpected data is found (or deprecated), but no errors as the program can continue without issue
error => an error was found, it usually involves error handling on the program part
fatal => unrecoverable error, program must shutdown
*/
enum class Severity
{
    trace = 0,
    debug,
    info,
    warning,
    error,
    fatal
};

// Defining the attributes severity and channel
// Channel is here to be able to specify which libraries or modules are allowed to log
BOOST_LOG_ATTRIBUTE_KEYWORD(severity_level, boost::log::aux::default_attribute_names::severity(), Severity)
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, boost::log::aux::default_attribute_names::channel(), std::string)

}
}
}

#endif // XIV_UTILS_LOG_H

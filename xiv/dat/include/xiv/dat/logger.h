#ifndef XIV_DAT_LOGGER_H
#define XIV_DAT_LOGGER_H

#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

#include <xiv/utils/log.h>

// Define the multithread aware logger for the dat library with sourcename = "xiv::dat"
BOOST_LOG_INLINE_GLOBAL_LOGGER_CTOR_ARGS(
    xiv_dat_logger,
    boost::log::sources::severity_channel_logger_mt<xiv::utils::log::Severity>,
    (boost::log::keywords::channel = "xiv::dat"));

#endif // XIV_DAT_LOGGER_H

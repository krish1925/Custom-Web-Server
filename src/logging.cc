#include "logging.h"
#include <boost/filesystem.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging
{
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

void init(const std::string& log_dir)
{
    namespace fs = boost::filesystem;
    if (!fs::exists(log_dir)) fs::create_directories(log_dir);

    // rotating file sink
    boost::log::add_file_log(
        boost::log::keywords::file_name        = log_dir + "/%Y-%m-%d_%N.log",
        boost::log::keywords::rotation_size    = 10 * 1024 * 1024, // 10 MiB
        boost::log::keywords::time_based_rotation =
            boost::log::sinks::file::rotation_at_time_point(0, 0, 0), // midnight
        boost::log::keywords::format = (
            expr::stream
                << expr::format_date_time<boost::posix_time::ptime>(
                       "TimeStamp", "%Y-%m-%d %H:%M:%S")
                << " [" << expr::attr<attrs::current_thread_id::value_type>("ThreadID")
                << "] <" << boost::log::trivial::severity << "> "
                << expr::smessage));

    // console sink
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = "[%TimeStamp%] <%Severity%> %Message%");

    boost::log::add_common_attributes();
}
}

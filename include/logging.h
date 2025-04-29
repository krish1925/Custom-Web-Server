#ifndef LOGGING_H
#define LOGGING_H
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>

namespace logging
{
void init(const std::string& log_dir = "logs");
}

#endif  // LOGGING_H
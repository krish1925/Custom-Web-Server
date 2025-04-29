#include "static_file_handler.h"
#include "response.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include "logging.h"

namespace fs = std::filesystem;

namespace {

/// Minimal extension→MIME lookup.
/// Extend this map as you need (PDF, MP4, etc.).
std::string mime_from_ext(const std::string& ext) {
    static const std::unordered_map<std::string, std::string> kTable = {
        {".html", "text/html"},
        {".htm",  "text/html"},
        {".css",  "text/css"},
        {".js",   "application/javascript"},
        {".jpg",  "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png",  "image/png"},
        {".gif",  "image/gif"},
        {".txt",  "text/plain"},
        {".zip",  "application/zip"}
    };
    auto it = kTable.find(ext);
    return it == kTable.end() ? "application/octet-stream" : it->second;
}

/// Make sure the resolved file stays under <root> (path-traversal defence).
bool within_root(const fs::path& root, const fs::path& cand) {
    fs::path canon_root = fs::weakly_canonical(root);
    fs::path canon_cand = fs::weakly_canonical(cand);
    return std::mismatch(canon_root.begin(), canon_root.end(),
                         canon_cand.begin()).first == canon_root.end();
}

} // namespace

/* ------------------- StaticFileHandler impl ------------------------------ */

StaticFileHandler::StaticFileHandler(std::string url_prefix,
                                     std::string root_dir)
    : prefix_(std::move(url_prefix)),
      root_(std::move(root_dir))
{
    if (!prefix_.empty() && prefix_.back() != '/')
        prefix_ += '/';          // normalise for easy stripping
}

Response StaticFileHandler::handle(const Request& req)
{
    BOOST_LOG_TRIVIAL(info) << "[StaticFileHandler] Serving request"
                            << " uri=" << req.uri
                            << " prefix=" << prefix_
                            << " root="   << root_;
    Response res;

    /* 1. Strip the URL prefix. */
    if (req.uri.rfind(prefix_, 0) != 0) {
        res.status_code = 400;
        res.status_message = "Bad Request";
        res.headers["Content-Type"] = "text/plain";
        res.body = "URL does not match handler prefix.";
        return res;
    }
    std::string rel = req.uri.substr(prefix_.size());

    /* 2. Map to filesystem & guard against ../ etc. */
    fs::path requested = fs::path(root_) / fs::path(rel);
    std::cerr << "[StaticFileHandler] requested = " << requested << '\n';

    if (!within_root(root_, requested) ||
        !fs::exists(requested)         ||
        fs::is_directory(requested)) {
        res.status_code = 404;
        res.status_message = "Not Found";
        res.headers["Content-Type"] = "text/plain";
        res.body = "404 Not Found";
        return res;
    }

    /* 3. Read file into memory. (For big files, stream instead.) */
    std::ifstream in(requested, std::ios::binary);
    if (!in) {
        res.status_code = 500;
        res.status_message = "Internal Server Error";
        res.headers["Content-Type"] = "text/plain";
        res.body = "Could not open file.";
        return res;
    }
    std::ostringstream buf; buf << in.rdbuf();
    res.body = buf.str();

    /* 4. Fill headers. */
    res.status_code = 200;
    res.status_message = "OK";
    res.headers["Content-Type"]  = mime_from_ext(requested.extension().string());
    res.headers["Content-Length"] = std::to_string(res.body.size());

    return res;
}

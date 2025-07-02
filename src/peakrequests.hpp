#ifndef PEAKREQUESTS_HPP
#define PEAKREQUESTS_HPP
#include <string>
#include <map>
#include <vector>
#include <memory>
namespace PeakRequests {
struct Response {
    long status_code;
    std::map<std::string, std::string> headers;
    std::string text;
    std::string error;
    bool success;
    Response() : status_code(0), success(false) {}
};
class PeakRequests {
public:
    static Response get(const std::string& url,
                        const std::map<std::string, std::string>& headers = {});
    static Response post(const std::string& url,
                         const std::string& data,
                         const std::map<std::string, std::string>& headers = {},
                         const std::string& content_type = "application/x-www-form-urlencoded");
    static Response put(const std::string& url,
                        const std::string& data,
                        const std::map<std::string, std::string>& headers = {},
                        const std::string& content_type = "application/x-www-form-urlencoded");
    static Response del(const std::string& url,
                       const std::map<std::string, std::string>& headers = {});
private:
    static Response performRequest(const std::string& method,
                                   const std::string& url,
                                   const std::string& data,
                                   const std::map<std::string, std::string>& headers,
                                   const std::string& content_type);
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t headerCallback(char* buffer, size_t size, size_t nitems, void* userdata);
};
}
#endif

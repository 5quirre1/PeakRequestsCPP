#include "peakrequests.hpp"
#include <curl/curl.h>
#include <iostream>
#include <algorithm>
#include <memory>
namespace PeakRequests
{
    class CurlGlobalInit
    {
    public:
        CurlGlobalInit()
        {
            curl_global_init(CURL_GLOBAL_DEFAULT);
        }
        ~CurlGlobalInit()
        {
            curl_global_cleanup();
        }
    };
    static CurlGlobalInit global_curl_init;
    size_t PeakRequests::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }
    size_t PeakRequests::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata)
    {
        size_t len = size * nitems;
        std::string header_line(buffer, len);
        header_line.erase(std::remove_if(header_line.begin(), header_line.end(), [](char c)
                                         { return c == '\r' || c == '\n'; }),
                          header_line.end());
        if (header_line.empty())
        {
            return len;
        }
        size_t colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string key = header_line.substr(0, colon_pos);
            std::string value = header_line.substr(colon_pos + 1);
            size_t first_char = value.find_first_not_of(" \t");
            if (first_char != std::string::npos)
            {
                value = value.substr(first_char);
            }
            size_t last_char = value.find_last_not_of(" \t");
            if (last_char != std::string::npos)
            {
                value = value.substr(0, last_char + 1);
            }
            Response *response = static_cast<Response *>(userdata);
            response->headers[key] = value;
        }
        return len;
    }
    Response PeakRequests::performRequest(const std::string &method,
                                          const std::string &url,
                                          const std::string &data,
                                          const std::map<std::string, std::string> &custom_headers,
                                          const std::string &content_type)
    {
        Response response;
        CURL *curl = curl_easy_init();
        if (!curl)
        {
            response.success = false;
            response.error = "failed to initialize curl";
            return response;
        }
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_guard(curl, curl_easy_cleanup);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.text);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
        struct curl_slist *header_list = nullptr;
        for (const auto &pair : custom_headers)
        {
            std::string header_string = pair.first + ": " + pair.second;
            header_list = curl_slist_append(header_list, header_string.c_str());
        }
        if ((method == "POST" || method == "PUT") && !data.empty())
        {
            bool content_type_set = false;
            for (const auto &pair : custom_headers)
            {
                std::string key_lower = pair.first;
                std::transform(key_lower.begin(), key_lower.end(), key_lower.begin(), ::tolower);
                if (key_lower == "content-type")
                {
                    content_type_set = true;
                    break;
                }
            }
            if (!content_type_set)
            {
                std::string ct_header = "Content-Type: " + content_type;
                header_list = curl_slist_append(header_list, ct_header.c_str());
            }
        }
        if (header_list)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        }
        if (method == "POST")
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
        }
        else if (method == "PUT")
        {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
        }
        else if (method == "DELETE")
        {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        else if (method == "GET")
        {
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
            if (!data.empty())
            {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
            }
        }
        CURLcode res = curl_easy_perform(curl);
        if (header_list)
        {
            curl_slist_free_all(header_list);
        }
        if (res != CURLE_OK)
        {
            response.success = false;
            response.error = curl_easy_strerror(res);
            std::cerr << "curl_easy_perform() failed: " << response.error << std::endl;
        }
        else
        {
            response.success = true;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
        }
        return response;
    }
    Response PeakRequests::get(const std::string &url,
                               const std::map<std::string, std::string> &headers)
    {
        return performRequest("GET", url, "", headers, "");
    }
    Response PeakRequests::post(const std::string &url,
                                const std::string &data,
                                const std::map<std::string, std::string> &headers,
                                const std::string &content_type)
    {
        return performRequest("POST", url, data, headers, content_type);
    }
    Response PeakRequests::put(const std::string &url,
                              const std::string &data,
                              const std::map<std::string, std::string> &headers,
                              const std::string &content_type)
    {
        return performRequest("PUT", url, data, headers, content_type);
    }
    Response PeakRequests::del(const std::string &url,
                               const std::map<std::string, std::string> &headers)
    {
        return performRequest("DELETE", url, "", headers, "");
    }
}

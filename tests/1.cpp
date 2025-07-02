#include <iostream>
#include "peakrequests.hpp"
int main() {
    std::cout << "testing\n";
    PeakRequests::Response response = PeakRequests::PeakRequests::get("https://httpbin.org/get");
    if (response.success) {
        std::cout << "get request successful!\n";
        std::cout << "status code: " << response.status_code << "\n";
        std::cout << "response body: " << response.text.substr(0, 200) << "...\n";
    } else {
        std::cout << "get request failed: " << response.error << "\n";
    }
    return 0;
}

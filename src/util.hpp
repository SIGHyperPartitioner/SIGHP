#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

class Logger {
public:
    Logger(const std::string& message = "") {
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        timestamp_ << "[" << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S") << "] ";

        message_ << message;
    }

    template <typename T>
    Logger& operator<<(const T& value) {
        message_ << value;
        return *this;
    }

    ~Logger() {
        std::cout << timestamp_.str() <<":"<< message_.str() << std::endl;
    }

private:
    std::stringstream timestamp_;
    std::stringstream message_;
};

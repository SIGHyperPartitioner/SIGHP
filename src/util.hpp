#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

class Logger {
public:
    Logger(const std::string& message = "") {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        // 将时间戳转为字符串
        timestamp_ << "[" << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S") << "] ";

        // 存储日志信息
        message_ << message;
    }

    // 重载 << 运算符，使其支持链式操作
    template <typename T>
    Logger& operator<<(const T& value) {
        message_ << value;
        return *this;
    }

    ~Logger() {
        // 在析构函数中输出日志信息
        std::cout << timestamp_.str() <<":"<< message_.str() << std::endl;
    }

private:
    std::stringstream timestamp_;
    std::stringstream message_;
};

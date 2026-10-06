#ifndef I2CBUS_HPP
#define I2CBUS_HPP

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <string>

class I2CBus {
public:
    typedef struct
    {
        uint16_t addr;
        bool read;
        std::span<uint8_t> data;
    } i2c_msg_t;

    explicit I2CBus(const std::string& busPath);
    ~I2CBus();

    int init();
    void deinit();
    // Sends all messages as one transaction (repeated start between messages,
    // one stop at the end). Returns 0 on success or -errno on failure.
    int transfer(std::span<i2c_msg_t> messages);

private:
    int m_fd = -1;
    std::string m_busPath;
    std::mutex m_mutex;
};

#endif // I2CBUS_HPP

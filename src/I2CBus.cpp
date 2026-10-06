#include "I2CBus.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

I2CBus::I2CBus(const std::string& busPath):
    m_busPath(busPath)
{

}

I2CBus::~I2CBus()
{
    deinit();
}

int I2CBus::init()
{
    if (m_fd >= 0)
    {
        return 0;
    }

    m_fd = open(m_busPath.c_str(), O_RDWR);
    if (m_fd < 0)
    {
        return -errno;
    }

    return 0;
}

void I2CBus::deinit()
{
    if (m_fd < 0)
    {
        close(m_fd);
    }
}

int I2CBus::transfer(std::span<i2c_msg_t> messages)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_fd < 0)
    {
        return -EBADF;
    }

    if (messages.empty() || messages.size() > I2C_RDWR_IOCTL_MAX_MSGS)
    {
        return -EINVAL;
    }

    // Convert our messages to the kernel's format
    struct i2c_msg kernelMsgs[I2C_RDWR_IOCTL_MAX_MSGS] = {};

    for (size_t i = 0; i < messages.size(); ++i)
    {
        const i2c_msg_t& msg = messages[i];

        // The kernel's len field is 16 bits and zero-length messages are invalid
        if (msg.data.empty() || msg.data.size() > UINT16_MAX)
        {
            return -EINVAL;
        }

        kernelMsgs[i].addr  = msg.addr;
        kernelMsgs[i].flags = msg.read ? I2C_M_RD : 0;
        kernelMsgs[i].len   = static_cast<uint16_t>(msg.data.size());
        kernelMsgs[i].buf   = msg.data.data();
    }

    struct i2c_rdwr_ioctl_data request = {};
    request.msgs  = kernelMsgs;
    request.nmsgs = static_cast<uint32_t>(messages.size());

    if (ioctl(m_fd, I2C_RDWR, &request) < 0)
    {
        return -errno;
    }

    return 0;
}
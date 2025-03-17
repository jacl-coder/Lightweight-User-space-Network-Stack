#include "network_interface.hpp"
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/logger.hpp"
#include "../core/event_loop.hpp"

namespace lwip {

namespace {
constexpr size_t MAX_FRAME_SIZE = 1518;
}

// TunInterface实现
TunInterface::~TunInterface() {
    if (fd_ >= 0) {
        ::close(fd_);  // 使用作用域解析运算符
    }
}

bool TunInterface::init() {
    fd_ = ::open("/dev/net/tun", O_RDWR);  // 使用作用域解析运算符
    if (fd_ < 0) {
        LOG_ERROR("Failed to open tun device: " + std::string(strerror(errno)));
        return false;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "tun0", IFNAMSIZ - 1);
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;  // 正确的成员名是ifr_flags
    if (ioctl(fd_, TUNSETIFF, &ifr) < 0) {
        LOG_ERROR("Failed to set TUN interface flags: " + std::string(strerror(errno)));
        ::close(fd_);  // 使用作用域解析运算符
        return false;
    }
    
    // 设置网络参数
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG_ERROR("Failed to create socket");
        close(fd_);
        return false;
    }

    // 设置IP地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.1.100");
    memcpy(&ifr.ifr_addr, &addr, sizeof(addr));

    if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
        LOG_ERROR("Failed to set IP address: " + std::string(strerror(errno)));
        close(sock);
        close(fd_);
        return false;
    }

    // 设置网络掩码
    struct ifreq ifr_mask;
    memset(&ifr_mask, 0, sizeof(ifr_mask));
    strncpy(ifr_mask.ifr_name, "tun0", IFNAMSIZ - 1);
    struct sockaddr_in* mask = (struct sockaddr_in*)&ifr_mask.ifr_addr;
    mask->sin_family = AF_INET;
    mask->sin_addr.s_addr = inet_addr("255.255.255.0");
    if (ioctl(sock, SIOCSIFNETMASK, &ifr_mask) < 0) {
        LOG_ERROR("Failed to set netmask");
        close(sock);
        close(fd_);
        return false;
    }

    // 确保接口启用
    struct ifreq ifr_conf;  // 改名为ifr_conf避免冲突
    memcpy(&ifr_conf, &ifr, sizeof(ifr));
    
    if (ioctl(sock, SIOCGIFFLAGS, &ifr_conf) < 0) {
        LOG_ERROR("Failed to get interface flags");
        ::close(sock);
        ::close(fd_);
        return false;
    }
    
    ifr_conf.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(sock, SIOCSIFFLAGS, &ifr_conf) < 0) {
        LOG_ERROR("Failed to set interface UP");
        ::close(sock);
        ::close(fd_);
        return false;
    }

    // 添加默认路由
    system("ip link set tun0 up");
    system("ip route add 192.168.1.0/24 dev tun0");

    // 添加路由
    system("ip route add 192.168.1.0/24 dev tun0");

    close(sock);
    LOG_INFO("TUN interface initialized successfully");
    return true;
}

void TunInterface::shutdown() {
    if (fd_ >= 0) {
        ::close(fd_);  // 使用作用域解析运算符
        fd_ = -1;
    }
}

bool TunInterface::send_frame(const std::vector<uint8_t>& data) {
    if (fd_ < 0) return false;
    ssize_t written = write(fd_, data.data(), data.size());
    return written == static_cast<ssize_t>(data.size());
}

bool TunInterface::receive_frame(std::vector<uint8_t>& data) {
    if (fd_ < 0) return false;
    data.resize(MAX_FRAME_SIZE);
    ssize_t n = read(fd_, data.data(), data.size());
    if (n > 0) {
        data.resize(n);
        return true;
    }
    return false;
}

uint64_t TunInterface::get_mac_address() const { 
    return 0; 
}

void TunInterface::set_mac_address([[maybe_unused]] uint64_t mac) {}

// TapInterface实现
TapInterface::~TapInterface() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

bool TapInterface::init() {
    fd_ = open("/dev/net/tap", O_RDWR);
    if (fd_ < 0) {
        LOG_ERROR("Failed to open tap device");
        return false;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(fd_, TUNSETIFF, &ifr) < 0) {
        close(fd_);
        return false;
    }
    
    return configure_interface(ifr.ifr_name);
}

bool TapInterface::send_frame(const std::vector<uint8_t>& data) {
    if (fd_ < 0) return false;
    ssize_t written = write(fd_, data.data(), data.size());
    return written == static_cast<ssize_t>(data.size());
}

bool TapInterface::receive_frame(std::vector<uint8_t>& data) {
    if (fd_ < 0) return false;
    data.resize(MAX_FRAME_SIZE);
    ssize_t n = read(fd_, data.data(), data.size());
    if (n > 0) {
        data.resize(n);
        return true;
    }
    return false;
}

uint64_t TapInterface::get_mac_address() const {
    return 0;
}

void TapInterface::set_mac_address([[maybe_unused]] uint64_t mac) {}

bool TapInterface::configure_interface(const char* name) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock < 0) {
        return false;
    }
    
    // 初始化结构体并设置接口名称
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
    
    // 获取当前标志
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        close(sock);
        return false;
    }

    // 设置UP和RUNNING标志
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        close(sock);
        return false;
    }
    
    close(sock);
    return true;
}

class NonBlockingInterface : public NetworkInterface {
private:
    int fd_{-1};
    EventLoop event_loop_;
    
public:
    bool init() override {
        TunInterface tun;
        if (!tun.init()) {
            return false;
        }
        
        // 设置非阻塞模式
        int flags = fcntl(fd_, F_GETFL, 0);
        if (flags < 0) {
            return false;
        }
        
        if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
            return false;
        }
        
        event_loop_.run();
        return true;
    }

    bool send_frame(const std::vector<uint8_t>& data) {
        if (fd_ < 0) return false;
        ssize_t written = write(fd_, data.data(), data.size());
        return written == static_cast<ssize_t>(data.size());
    }

    bool receive_frame(std::vector<uint8_t>& data) {
        if (fd_ < 0) return false;
        data.resize(MAX_FRAME_SIZE);
        ssize_t n = read(fd_, data.data(), data.size());
        if (n > 0) {
            data.resize(n);
            return true;
        }
        return false;
    }

    uint64_t get_mac_address() const {
        return 0;
    }

    void set_mac_address([[maybe_unused]] uint64_t mac) override {}

    void process_io() {
        std::vector<uint8_t> buffer(MAX_FRAME_SIZE);
        ssize_t n;
        
        while ((n = read(fd_, buffer.data(), buffer.size())) > 0) {
            buffer.resize(n);
            on_frame_received(buffer);
        }
        
        if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("Failed to read from interface");
        }
    }
    
protected:
    virtual void on_frame_received([[maybe_unused]] const std::vector<uint8_t>& frame) {}
};

} // namespace lwip

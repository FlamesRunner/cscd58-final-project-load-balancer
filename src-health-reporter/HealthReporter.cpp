#include <iostream>
#include "hdrs/HealthReporter.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <string.h>
#include "hdrs/plusaes.hpp"
#include <signal.h>

using namespace std;

// SIGPIPE handler to prevent program from crashing on write to closed socket
void sigpipe_handler(int signum)
{
    return;
}

HealthReporter::HealthReporter(int port, string enc_key)
{
    this->port = port;
    this->enc_key = enc_key;
}

int HealthReporter::get_port()
{
    return this->port;
}

void HealthReporter::start()
{
    cout << "Starting health reporter on port " << this->port << endl;
    cout << "Encryption key: " << this->enc_key << endl;

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        cerr << "Failed to create socket" << endl;
        return;
    }

    // Bind socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cerr << "Failed to bind socket" << endl;
        return;
    }

    // Listen on socket
    if (listen(sock, 5) < 0)
    {
        cerr << "Failed to listen on socket" << endl;
        return;
    }

    // Accept connections
    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0)
        {
            cerr << "Failed to accept connection" << endl;
            continue;
        }

        HealthReporterConnection *conn = new HealthReporterConnection(client_sock, this->enc_key);
        thread t(&HealthReporterConnection::start, conn);
        t.detach();
    }
}

HealthReporterConnection::HealthReporterConnection(int socket, string enc_key)
{
    this->socket = socket;
    this->state = HR_STATE_LB_HANDSHAKE;

    // Set SIGPIPE handler
    signal(SIGPIPE, sigpipe_handler);
}

void HealthReporterConnection::start()
{
    // Start handshake process
    bool handshake_status = this->handle_lb_handshake();
    if (!handshake_status)
    {
        cerr << "Handshake failed." << endl;
        close(this->socket);
        return;
    }

    cout << "Connection established with load balancer." << endl;
    thread t(&HealthReporterConnection::handle_connected, this);
    t.join();

    // Close connection
    close(this->socket);
}

bool HealthReporterConnection::handle_lb_handshake()
{
    // Check for handshake message HR_HANDSHAKE_MSG_1
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    int bytes_read = read(this->socket, buf, sizeof(buf));
    if (bytes_read < 0)
    {
        cerr << "Failed to read from socket" << endl;
        return false;
    }

    if (strcmp(buf, HR_HANDSHAKE_MSG_1) != 0)
    {
        cerr << "Invalid handshake message (1)" << endl;
        return false;
    }

    // Send handshake message HR_HANDSHAKE_MSG_2
    if (write(this->socket, HR_HANDSHAKE_MSG_2, strlen(HR_HANDSHAKE_MSG_2)) < 0)
    {
        cerr << "Failed to write to socket" << endl;
        return false;
    }

    // Check for handshake message HR_HANDSHAKE_MSG_3
    bytes_read = read(this->socket, buf, sizeof(buf));
    if (bytes_read < 0)
    {
        cerr << "Failed to read from socket" << endl;
        return false;
    }

    if (strcmp(buf, HR_HANDSHAKE_MSG_3) != 0)
    {
        cerr << buf << endl;
        cerr << "Invalid handshake message (3)" << endl;
        return false;
    }

    // Connection is now established
    this->state = HR_STATE_CONNECTED;
    return true;
}

HealthReport_t HealthReporterConnection::generate_health_report()
{
    HealthReport_t report;
    // Get system load from /proc/loadavg
    FILE *loadavg = fopen("/proc/loadavg", "r");
    if (loadavg == NULL)
    {
        cerr << "Failed to open /proc/loadavg" << endl;
        report.cpu_load = -1;
    }
    else
    {
        // Only take minute load average
        int scan_res = fscanf(loadavg, "%f", &report.cpu_load);
        fclose(loadavg);
    }

    // Get system memory usage from /proc/meminfo
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo == NULL)
    {
        cerr << "Failed to open /proc/meminfo" << endl;
        report.mem_load = -1;
    }
    else
    {
        // Only take MemAvailable and MemTotal
        char buf[1024];
        int mem_total, mem_available = -1;
        while (fgets(buf, sizeof(buf), meminfo) != NULL)
        {
            if (strncmp(buf, "MemAvailable:", 13) == 0)
            {
                sscanf(buf, "MemAvailable: %d kB", &mem_available);
            }

            if (strncmp(buf, "MemTotal:", 9) == 0)
            {
                sscanf(buf, "MemTotal: %d kB", &mem_total);
            }
        }
        fclose(meminfo);

        // Calculate memory usage
        if (mem_total == -1 || mem_available == -1)
        {
            report.mem_load = -1;
        }
        else
        {
            report.mem_load = (int)(((float)mem_total - (float)mem_available) / (float)mem_total * 100);
        }
    }

    return report;
}

std::vector<unsigned char> HealthReporterConnection::encrypt_health_report(HealthReport_t report)
{
    unsigned char *encrypted_health_report = new unsigned char[sizeof(HealthReport_t)];
    memcpy(encrypted_health_report, &report, sizeof(HealthReport_t));
    std::string raw_data = std::string((char*)encrypted_health_report, sizeof(HealthReport_t));

    //inline std::vector<unsigned char> key_from_string(const char (*key_str)[17]) {
    char key_ptr[17];
    memcpy(key_ptr, this->enc_key.c_str(), 16);
    key_ptr[16] = '\0';
    const char (*ptr)[17] = &key_ptr;

    const std::vector<unsigned char> key = plusaes::key_from_string(&key_ptr); // 16-char = 128-bit
    const unsigned char iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    };

    // encrypt
    const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(raw_data.size());
    std::vector<unsigned char> encrypted(encrypted_size);

    plusaes::encrypt_cbc((unsigned char*)raw_data.data(), raw_data.size(), &key[0], key.size(), &iv, &encrypted[0], encrypted.size(), true);


    // Encrypt health report
    return encrypted;
}

void HealthReporterConnection::handle_connected()
{
    // Continuously transmit health reports
    while (true)
    {
        // Generate random health report
        HealthReport_t report = this->generate_health_report();

        // Encrypt health report
        std::vector<unsigned char> encrypted_health_report = encrypt_health_report(report);

        // Convert to string
        std::string encrypted_health_report_str = std::string((char*)encrypted_health_report.data(), encrypted_health_report.size());

        // Send encrypted health report
        if (write(this->socket, encrypted_health_report_str.c_str(), encrypted_health_report_str.size()) < 0) {
            cerr << "Failed to write to socket. Connection lost." << endl;
            return;
        }

        // Sleep for 1 second
        sleep(1);
    }
}
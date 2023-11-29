#include <iostream>
#include "hdrs/HealthReporter.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <random>
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

    // Set encryption key
    this->enc_key = enc_key;

    // Generate random IV
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < 16; i++)
    {
        this->iv[i] = dis(gen);
    }
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
    char buf[BUFFER_SIZE];
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

    // Send IV to load balancer
    string iv_str = string(HR_HANDSHAKE_MSG_4) + string((char *)this->iv, 16) + string("\n");
    if (write(this->socket, iv_str.c_str(), iv_str.size()) < 0)
    {
        cerr << "Failed to write to socket" << endl;
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
        char buf[BUFFER_SIZE];
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

EncryptedHealthReport *HealthReporterConnection::encrypt_health_report(HealthReport_t report)
{
    std::string raw_data = HealthReportSerialzer::serialize(report);

    char key_ptr[17];
    memcpy(key_ptr, this->enc_key.c_str(), 16);
    key_ptr[16] = '\0';
    const char (*ptr)[17] = &key_ptr;

    const std::vector<unsigned char> key = plusaes::key_from_string(&key_ptr); // 16-char = 128-bit

    // Setup encryption
    const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(raw_data.size());
    std::vector<unsigned char> encrypted(encrypted_size);

    plusaes::encrypt_cbc((unsigned char*)raw_data.data(), raw_data.size(), &key[0], key.size(), &this->iv, &encrypted[0], encrypted.size(), true);

    // Allocate memory for encrypted health report
    EncryptedHealthReport *encrypted_health_report = new EncryptedHealthReport;
    encrypted_health_report->enc_size = encrypted_size;
    encrypted_health_report->encrypted_health_report = encrypted;

    return encrypted_health_report;
}

void HealthReporterConnection::handle_connected()
{
    // Continuously transmit health reports
    while (true)
    {
        // Generate random health report
        HealthReport_t report = this->generate_health_report();

        // Encrypt health report
        EncryptedHealthReport_t *encrypted_health_report_struct = encrypt_health_report(report);
        std::vector<unsigned char> encrypted_health_report = encrypted_health_report_struct->encrypted_health_report;

        // Create base64 encoded string of encrypted health report
        char base64_encoded_buffer[encrypted_health_report.size() * 2];
        memset(base64_encoded_buffer, 0, sizeof(base64_encoded_buffer));
        int size = Base64encode(base64_encoded_buffer, (char *)encrypted_health_report.data(), encrypted_health_report.size());

        // Convert to string
        std::string encrypted_health_report_str = std::string(base64_encoded_buffer);

        // Append newline and size of encrypted health report
        encrypted_health_report_str += "\n";
        encrypted_health_report_str += std::to_string(encrypted_health_report.size());

        // Send encrypted health report
        if (write(this->socket, encrypted_health_report_str.c_str(), encrypted_health_report_str.size()) < 0) {
            cerr << "Failed to write to socket. Connection lost." << endl;
            return;
        }

        // Sleep for 1 second
        this_thread::sleep_for(chrono::seconds(1));
    }
}
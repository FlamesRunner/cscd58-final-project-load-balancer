#ifndef HR_HDRS_DECL
#include <string>
#include <iostream>
#include <vector>
#include "../global-hdrs/HealthReporterDS.hpp"

class HealthReporter
{
public:
    HealthReporter(int port, std::string enc_key);
    int get_port();
    void start();

private:
    int port;
    std::string enc_key;
};

class HealthReporterConnection
{
public:
    HealthReporterConnection(int socket, std::string enc_key);
    void start();

private:
    int socket;
    int state;
    std::string enc_key;
    bool handle_lb_handshake();
    void handle_connected();
    HealthReport_t generate_health_report();
    std::vector<unsigned char> encrypt_health_report(HealthReport_t hr);
};

#define HR_HDRS_DECL
#endif
#ifndef HR_HDRS_DECL
#include <string>
#include <iostream>
#include <vector>

#define HR_HANDSHAKE_MSG_1 "1 LoadBalancer says hello!\n"
#define HR_HANDSHAKE_MSG_2 "2 HealthReporter says hello!\n"
#define HR_HANDSHAKE_MSG_3 "3 LoadBalancer acknowledged!\n"

enum HealthReporterState
{
    HR_STATE_LB_HANDSHAKE,
    HR_STATE_CONNECTED
};

typedef struct HealthReport
{
    float cpu_load;
    float mem_load;
} __attribute__((packed)) HealthReport_t;

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
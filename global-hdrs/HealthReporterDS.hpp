#ifndef HR_DS_HDRS_DECL
#include <cstring>
#include "base64.h"

#define BUFFER_SIZE 1024
#define HR_HANDSHAKE_MSG_1 "1 LoadBalancer says hello!\n"
#define HR_HANDSHAKE_MSG_2 "2 HealthReporter says hello!\n"
#define HR_HANDSHAKE_MSG_3 "3 LoadBalancer acknowledged!\n"
#define HR_HANDSHAKE_MSG_4 "4 IV " // IV is appended to this message along with a newline

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

class HealthReportSerialzer
{
public:
    static std::string serialize(HealthReport_t report)
    {
        unsigned char *encrypted_health_report = new unsigned char[sizeof(HealthReport_t)];
        memcpy(encrypted_health_report, &report, sizeof(HealthReport_t));
        std::string raw_data = std::string((char *)encrypted_health_report, sizeof(HealthReport_t));
        return raw_data;
    };

    static HealthReport_t deserialize(std::string raw_data)
    {
        HealthReport_t report;
        memcpy(&report, raw_data.c_str(), sizeof(HealthReport_t));
        return report;
    };
};

#define HR_DS_HDRS_DECL
#endif
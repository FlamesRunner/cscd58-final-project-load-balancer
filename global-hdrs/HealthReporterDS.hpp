#ifndef HR_DS_HDRS_DECL
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

#define HR_DS_HDRS_DECL
#endif
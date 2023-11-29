#ifndef HR_HDRS_DECL
#include "../../global-hdrs/HealthReporterDS.hpp"
#include <iostream>
#include <string>
#include <vector>

class HealthReporter {
public:
  HealthReporter(int port, std::string enc_key);
  int get_port();
  void start();

private:
  int port;
  std::string enc_key;
};

typedef struct EncryptedHealthReport {
  unsigned long enc_size;
  std::vector<unsigned char> encrypted_health_report;
} EncryptedHealthReport_t;

class HealthReporterConnection {
public:
  HealthReporterConnection(int socket, std::string enc_key);
  void start();

private:
  int socket;
  int state;
  std::string enc_key;
  unsigned char iv[16];
  bool handle_lb_handshake();
  void handle_connected();
  HealthReport_t generate_health_report();
  EncryptedHealthReport *encrypt_health_report(HealthReport_t hr);
};

#define HR_HDRS_DECL
#endif
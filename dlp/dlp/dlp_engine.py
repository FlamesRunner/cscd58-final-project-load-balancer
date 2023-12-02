import logging
from scapy.all import sniff
from scapy.layers import http
from dlp.dlp_rules import DlpRules

class DlpEngine:
    def __init__(self):
        #instantiate the DlpRules class to access DLP rules
        self.dlp_rules = DlpRules()

        #set up logging
        logging.basicConfig(filename='dlp_alerts.log', level=logging.INFO,
                            format='%(asctime)s [%(levelname)s] - %(message)s')

    def packet_callback(self, packet):
        #check if the packet contains an HTTP request layer
        if packet.haslayer(http.HTTPRequest):
            #extract the payload (data) from the HTTP request
            request_data = packet[http.HTTPRequest].load.decode(errors='ignore')

            #check for potential data leakage using DLP rules
            if self.dlp_rules.detect_data_leakage(request_data):
                #log an alert
                logging.info(f"Data Leakage Detected! Packet Details: {packet.summary()}")

                #notify the user (replace with your preferred notification mechanism)
                self.notify_user()

    def notify_user(self):
        print("ALERT: Potential Data Leakage Detected! Check the log file for details.")

    def start_sniffing(self):
        try:
            print("DLP Engine is running. Press Ctrl+C to stop.")
            
            # Sniff on all interfaces and filter by load balancer port
            sniff(prn=self.packet_callback, store=0, filter="tcp port 80", iface=None)

        except KeyboardInterrupt:
            print("DLP Engine stopped.")

#instantiate the DLP engine
dlp_engine = DlpEngine()

#start sniffing immediately upon running the script
# dlp_engine.start_sniffing()


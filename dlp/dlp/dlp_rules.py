import re

class DlpRules:
    def __init__(self):
        #DLP rules
        self.patterns = [
            #patterns indicating sensitive credentials
            r'password=|apikey=|secret=|credential=|private_key=',
            
            #patterns related to IT infrastructure and server configurations
            'vpn|firewall|server_name|network_configuration',
            
            #sensitive business keywords or phrases
            'project_codename|internal_department|proprietary_product',
            
            #patterns related to performance metrics and monitoring
            'CPU_usage|memory_consumption|network_bandwidth|performance_metrics',
            
            #patterns related to web requests and traffic
            'GET|POST|HTTP_request|web_traffic|url=',
            
            #patterns indicating redundancy or resource utilization
            'redundancy|resource_utilization|load_balancer|allocation',
            
            #specific file types that may contain sensitive data
            '\.pdf$|\.xls$|\.xlsx$',
            
            #patterns indicating database connection strings
            'jdbc:|connection_string=|database_url=',
            
            #patterns indicating personally identifiable information (PII)
            'name=|address=|ssn=',
            
            #keywords related to alerting and notifications
            'alert|notification|incident|emergency'
        ]

    def detect_data_leakage(self, data):
        #check for each pattern in the input data
        for pattern in self.patterns:
            if re.search(pattern, data, flags=re.IGNORECASE):
                #return True if any pattern is found, indicating potential data leakage
                return True
        #return False if no pattern is found
        return False

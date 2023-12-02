from dlp.dlp_engine import DlpEngine

if __name__ == '__main__':
    # Instantiate the DLP engine
    dlp_engine = DlpEngine()

    # Start packet sniffing
    try:
        print("DLP Engine is running. Press Ctrl+C to stop.")
        dlp_engine.start_sniffing()
    except KeyboardInterrupt:
        print("DLP Engine stopped.")

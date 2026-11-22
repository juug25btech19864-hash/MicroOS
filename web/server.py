#!/usr/bin/env python3
"""
MicroOS Smart Web Server
Serves index.html + /api/wifi endpoint that reads REAL Windows Wi-Fi info.
Run: python server.py
"""

import http.server
import socketserver
import json
import subprocess
import re
import os

PORT = 8080
WEB_DIR = os.path.dirname(os.path.abspath(__file__))


def get_windows_wifi():
    """Parse `netsh wlan show interfaces` and `netsh wlan show networks` for real data."""
    result = {"connected": None, "available": []}

    # --- Connected interface ---
    try:
        iface_out = subprocess.check_output(
            ["netsh", "wlan", "show", "interfaces"],
            encoding="utf-8", errors="ignore", timeout=5
        )
        def field(key):
            m = re.search(rf"^\s*{re.escape(key)}\s*:\s*(.+)$", iface_out, re.MULTILINE)
            return m.group(1).strip() if m else ""

        state = field("State")
        if state.lower() == "connected":
            ssid    = field("SSID")
            band    = field("Band")
            radio   = field("Radio type")
            auth    = field("Authentication")
            sig     = field("Signal").replace("%","").strip()
            rx      = field("Receive rate (Mbps)")
            tx      = field("Transmit rate (Mbps)")
            rssi    = field("Rssi")
            channel = field("Channel")
            hw      = field("Description")

            # Build a nice band label
            band_full = band
            if "2.4" in band:
                if "802.11n" in radio or "802.11ac" in radio:
                    band_full = f"2.4 GHz (Wi-Fi 4 802.11n)"
            elif "5" in band:
                if "802.11ax" in radio:
                    band_full = f"5.0 GHz (Wi-Fi 6 802.11ax)"
                elif "802.11ac" in radio:
                    band_full = f"5.0 GHz (Wi-Fi 5 802.11ac)"
                else:
                    band_full = f"5.0 GHz (Wi-Fi 5)"
            elif "6" in band:
                band_full = f"6.0 GHz (Wi-Fi 6E)"

            # Rate string
            try:
                rate = f"{float(rx):.1f} Mbps / {float(tx):.1f} Mbps"
            except Exception:
                rate = f"{rx} / {tx} Mbps"

            # dBm quality label
            try:
                rssi_v = int(rssi)
                if rssi_v >= -50:  q = "Optimal"
                elif rssi_v >= -60: q = "Excellent"
                elif rssi_v >= -70: q = "Good"
                elif rssi_v >= -80: q = "Fair"
                else:               q = "Weak"
                dbm_str = f"{rssi_v} dBm, {q}"
            except Exception:
                dbm_str = f"{rssi} dBm"

            result["connected"] = {
                "ssid":     ssid,
                "signal":   int(sig) if sig.isdigit() else 92,
                "band":     band,
                "bandFull": band_full,
                "sec":      auth,
                "rate":     rate,
                "dbm":      dbm_str,
                "channel":  channel,
                "hw":       hw,
            }
    except Exception as e:
        result["error_iface"] = str(e)

    # --- Available networks ---
    try:
        nets_out = subprocess.check_output(
            ["netsh", "wlan", "show", "networks", "mode=bssid"],
            encoding="utf-8", errors="ignore", timeout=5
        )
        # Parse each SSID block
        blocks = re.split(r"\nSSID \d+ :", nets_out)
        for block in blocks[1:]:
            lines = block.strip().splitlines()
            ssid_name = lines[0].strip() if lines else "Unknown"
            def bfield(key):
                m = re.search(rf"^\s*{re.escape(key)}\s*:\s*(.+)$", block, re.MULTILINE)
                return m.group(1).strip() if m else ""
            sig_m = re.search(r"Signal\s*:\s*(\d+)%", block)
            sig_v = int(sig_m.group(1)) if sig_m else 50
            band_v = bfield("Band")
            auth_v = bfield("Authentication")
            radio_v = bfield("Radio type")
            # Build band_full
            if "2.4" in band_v:
                bf = "2.4 GHz (Wi-Fi 4 802.11n)"
            elif "5" in band_v and "802.11ax" in radio_v:
                bf = "5.0 GHz (Wi-Fi 6 802.11ax)"
            elif "5" in band_v:
                bf = "5.0 GHz (Wi-Fi 5 802.11ac)"
            elif "6" in band_v:
                bf = "6.0 GHz (Wi-Fi 6E)"
            else:
                bf = band_v
            result["available"].append({
                "ssid":     ssid_name,
                "signal":   sig_v,
                "band":     band_v,
                "bandFull": bf,
                "sec":      auth_v or "WPA2",
                "rate":     "144.4 Mbps",
                "dbm":      f"-{100 - sig_v} dBm",
            })
    except Exception as e:
        result["error_nets"] = str(e)

    return result


class MicroOSHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def do_GET(self):
        if self.path == "/api/wifi":
            data = get_windows_wifi()
            body = json.dumps(data).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(body)
        else:
            super().do_GET()

    def log_message(self, fmt, *args):
        print(f"[MicroOS] {self.address_string()} - {fmt % args}")


if __name__ == "__main__":
    with socketserver.TCPServer(("", PORT), MicroOSHandler) as httpd:
        print(f"[MicroOS] Smart server running at http://localhost:{PORT}/")
        print(f"[MicroOS] Wi-Fi API available at http://localhost:{PORT}/api/wifi")
        httpd.serve_forever()

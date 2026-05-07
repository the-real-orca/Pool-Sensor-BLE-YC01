import pytest
import time
import json

def test_captive_portal_discovery_and_web(workbench, slot, test_progress):
    """
    Test the Captive Portal:
    1. Force the ESP32 into Portal Mode by providing invalid WiFi credentials.
    2. Scan for the ESP32's Access Point.
    3. Connect the Workbench to the ESP32's Portal.
    4. Verify the Portal's web pages.
    """
    
    test_progress("Preparing Workbench and ESP")

    result = workbench.serial_reset(slot=slot)
    
    # wait between tests for serial communication
    time.sleep(3)

    # 0. Ensure Workbench AP is stopped so it can scan/join as STA
    print("\n--- Step 0: Preparing Workbench ---")

    # 1. Force Portal Mode via Serial API
    test_progress("Forcing Portal Mode via invalid config")
    config = {
        "wifiSSID": "NonExistentWiFi_12345",
        "wifiPassword": "wrongpassword",
        "wifiTimeout": 5, # Fast timeout for test
        "portalSSID": "ESP32-Portal-Test",
        "portalPassword": "",
        "portalTimeout": 120
    }
    
    # We use serial_write and check if it matches the success pattern
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully", timeout=10)
    assert result.get("matched"), f"ESP32 did not confirm config save. Output: {result.get('output')}"
    
    test_progress("Waiting for Captive Portal to start")
    # Wait and Monitor Serial for Portal Start
    result = workbench.serial_monitor(slot=slot, pattern="starting captive portal", timeout=15)
    assert result.get("matched"), "ESP32 did not start Captive Portal within timeout"

    # 2. Scan for the ESP32 AP
    test_progress(f"Scanning for ESP32 AP '{config['portalSSID']}'")
    time.sleep(3)
    wifi_scan = workbench.scan()
    networks = wifi_scan.get("networks", [])
    found = any(n.get("ssid") == config["portalSSID"] for n in networks)
    
    assert found, f"ESP32 Portal AP '{config['portalSSID']}' not found in WiFi scan"

    # 3. Connect Workbench to ESP32 Portal
    test_progress(f"Connecting Workbench to ESP32 Portal '{config['portalSSID']}'")
    workbench.sta_join(ssid=config["portalSSID"], timeout=30)
    esp32_ap_ip = "192.168.4.1"
    
    # 4. Verify Web Pages
    test_progress(f"Verifying Portal Web Pages at {esp32_ap_ip}")
    
    # Home page
    resp = workbench.http_get(f"http://{esp32_ap_ip}/", timeout=15)
    assert resp.status_code == 200, f"Portal home page not reachable: {resp.status_code}"
    assert "Pool Sensor" in resp.text
    
    # Status endpoint
    resp = workbench.http_get(f"http://{esp32_ap_ip}/status", timeout=15)
    assert resp.status_code == 200, "Portal status endpoint not reachable"
   
    workbench.sta_leave()

def test_portal_provisioning(workbench, slot, wifi_network, test_progress):
    """
    Test provisioning via the Portal:
    1. Ensure we are connected to the ESP32 Portal.
    2. Send new WiFi credentials via the Portal's config.json PUT API.
    3. Verify the ESP32 connects to the new WiFi.
    """

    test_progress("Preparing ESP for portal mode")
    result = workbench.serial_reset(slot=slot)

    # wait between tests for serial communication
    time.sleep(3)

    # 1. Force Portal Mode via Serial API
    test_progress("Forcing Portal Mode")
    config = {
        "wifiSSID": "NonExistentWiFi_12345",
        "wifiPassword": "wrongpassword",
        "wifiTimeout": 5, # Fast timeout for test
        "portalSSID": "ESP32-Portal-Test",
        "portalPassword": "",
        "portalTimeout": 120
    }
    
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully", timeout=10)
    assert result.get("matched"), f"ESP32 did not confirm config save."
    
    # Wait for portal to be ready
    workbench.serial_monitor(slot=slot, pattern="starting captive portal", timeout=15)

    # 2. Connect Workbench to ESP32 Portal
    test_progress(f"Connecting Workbench to ESP32 Portal '{config['portalSSID']}'")
    workbench.sta_join(ssid=config["portalSSID"], timeout=30)

    esp32_ap_ip = "192.168.4.1"
    ssid = wifi_network["ssid"]
    password = wifi_network["password"]
    
    test_progress(f"Provisioning ESP32 to connect to '{ssid}'")
    payload = {
        "wifiSSID": ssid,
        "wifiPassword": password,
        "wifiTimeout": 30,
        "portalSSID": "ESP32-Portal",
        "portalTimeout": 600
    }
    
    # Send Workbench network credentials via HTTP PUT
    resp = workbench.http_request(method="PUT", url=f"http://{esp32_ap_ip}/config.json", 
                                 body=json.dumps(payload).encode("utf-8"),
                                 headers={"Content-Type": "application/json"},
                                 timeout=15)
    assert resp.status_code == 200, f"Failed to send config to portal: {resp.status_code}"
    
    # 4. Prepare Workbench for incoming connection
    test_progress("Starting Workbench AP and waiting for ESP32 connection")
    workbench.sta_leave()
    workbench.ap_start(ssid, password)
    time.sleep(3)

    # 5. Wait for the ESP32 to connect to our Workbench AP
    event = workbench.wait_for_station(timeout=60)
    assert event["type"] == "STA_CONNECT"
    esp32_ip = event.get("ip")
    time.sleep(3)

    # 6. Verify home page on new IP
    test_progress(f"Verifying connectivity at http://{esp32_ip}/")
    resp = workbench.http_get(f"http://{esp32_ip}/", timeout=15)
    assert resp.status_code == 200, f"Home page not reachable: {resp.status_code}"
    assert "Pool Sensor" in resp.text
    

def test_captive_portal_stops_after_timeout(workbench, slot, test_progress):
    """
    Test the Captive Portal:
    1. Force the ESP32 into Portal Mode by providing invalid WiFi credentials.
    2. Scan for the ESP32's Access Point.
    3. Wait for Portal timeout
    4. Verify the Portal does not come up again
    """

    test_progress("Resetting ESP")
    result = workbench.serial_reset(slot=slot)
    
    # wait between tests for serial communication
    time.sleep(3)

    # 0. Ensure Workbench AP is stopped so it can scan/join as STA
    print("\n--- Step 0: Preparing Workbench ---")

    # 1. Force Portal Mode via Serial API
    test_progress("Forcing Portal Mode via invalid config")
    config = {
        "wifiSSID": "NonExistentWiFi_12345",
        "wifiPassword": "wrongpassword",
        "wifiTimeout": 10, # Fast timeout for test
        "portalSSID": "ESP32-Portal-Test",
        "portalPassword": "",
        "portalTimeout": 15 # Fast timeout for test
    }
    
    # We use serial_write and check if it matches the success pattern
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully", timeout=10)
    assert result.get("matched"), f"ESP32 did not confirm config save. Output: {result.get('output')}"
    
    test_progress("Waiting for Captive Portal to start")
    
    # Wait and Monitor Serial for Portal Start
    result = workbench.serial_monitor(slot=slot, pattern="starting captive portal", timeout=config["wifiTimeout"] + 5)
    assert result.get("matched"), "ESP32 did not start Captive Portal within timeout"

    # 2. Scan for the ESP32 AP
    test_progress(f"Scanning for ESP32 AP '{config['portalSSID']}'")
    found = False
    for i in range(5):
        wifi_scan = workbench.scan()
        networks = wifi_scan.get("networks", [])
        found = any(n.get("ssid") == config["portalSSID"] for n in networks)
        if found: break
        time.sleep(5)
    assert found, f"ESP32 Portal AP '{config['portalSSID']}' not found in WiFi scan"

    # 3. Wait for Portal timeout
    test_progress(f"Waiting for Portal timeout ({config['portalTimeout']}s)")
    time.sleep(config["portalTimeout"])
    result = workbench.serial_monitor(slot=slot, pattern="Standby", timeout=15)
    assert result.get("matched"), "ESP32 did not go into standby mode"

    found = True
    for i in range(5):
        time.sleep(5)
        wifi_scan = workbench.scan()
        networks = wifi_scan.get("networks", [])
        found = any(n.get("ssid") == config["portalSSID"] for n in networks)
        if not(found): break
    assert not(found), f"ESP32 Portal AP '{config['portalSSID']}' still active, after timeout"

    
    # 4. Verify Web Pages
    test_progress("Verifying the Portal does not come up again")
    time.sleep(config["wifiTimeout"])
    time.sleep(5)
    wifi_scan = workbench.scan()
    networks = wifi_scan.get("networks", [])
    found = any(n.get("ssid") == config["portalSSID"] for n in networks)
    assert not(found), f"ESP32 Portal AP '{config['portalSSID']}' reactivated"
   
    workbench.sta_leave()
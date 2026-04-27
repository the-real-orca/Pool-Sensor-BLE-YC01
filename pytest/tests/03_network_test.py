import pytest
import time
import json

def test_network_wifi_lifecycle(workbench, slot, wifi_network):
    """
    Test the full WiFi lifecycle:
    1. Connect to a new AP via Serial API configuration.
    2. Verify the ESP is reachable via HTTP.
    3. Restart the AP and verify the ESP reconnects automatically.
    """
    
    ssid = wifi_network["ssid"]
    password = wifi_network["password"]
    
    print(f"--- Step 1: Configuring ESP to connect to {ssid} ---")
    
    # 1.1 Send SET_CONFIG command with JSON
    config = {
        "wifiSSID": ssid,
        "wifiPassword": password,
        "wifiTimeout": 30 # Short timeout for faster testing
    }
    config_json = json.dumps(config)
    
    # Send SET_CONFIG command with JSON
    result = workbench.serial_write(slot=slot, data=f"SET_CONFIG {config_json}", pattern="Config saved successfully.", timeout=20)
    assert result.get("matched"), "ESP did not save config via SET_CONFIG"
    
    # 2. Wait for the ESP to connect to the AP
    print(f"Waiting for ESP to connect to {ssid}...")
    try:
        event = workbench.wait_for_station(timeout=45)
        assert event["type"] == "STA_CONNECT"
        print(f"ESP connected event received: {event}")
    except Exception as e:
        # Fallback: check ap_status directly
        status = workbench.ap_status()
        stations = status.get("stations", [])
        if any(s.get("ip") for s in stations):
            print(f"ESP is connected (found in station list): {stations}")
        else:
            pytest.fail(f"ESP failed to connect to AP within timeout: {e}")

    # 3. Verify ESP web server is reachable
    time.sleep(5) # Give web server time to start
    status = workbench.ap_status()
    stations = status.get("stations", [])
    assert len(stations) > 0, "No stations connected to AP"
    
    esp_ip = stations[0].get("ip")
    assert esp_ip, "ESP IP address not found in station list"
    
    print(f"--- Step 2: Verifying connectivity to http://{esp_ip}/ ---")
    resp = workbench.http_get(f"http://{esp_ip}/", timeout=15)
    assert resp.status_code == 200, f"ESP web server returned {resp.status_code}"
    assert "Pool Sensor" in resp.text or "Index" in resp.text, "ESP index page content mismatch"
    print("ESP web server is reachable")

    # 4. Test Reconnection
    print("--- Step 3: Testing Reconnection after AP restart ---")
    print("Stopping AP...")
    workbench.ap_stop()
    time.sleep(5)
    
    print("Starting AP again...")
    workbench.drain_events()
    workbench.ap_start(ssid, password)
    
    print(f"Waiting for ESP to reconnect to {ssid}...")
    # Reconnection might take a while depending on ESP's retry strategy
    event = workbench.wait_for_station(timeout=60)
    assert event["type"] == "STA_CONNECT"
    print("ESP reconnected successfully")
    
    # Final check
    time.sleep(2)
    status = workbench.ap_status()
    assert len(status.get("stations", [])) > 0, "ESP not found in station list after reconnect"
    print("WiFi lifecycle test PASSED")

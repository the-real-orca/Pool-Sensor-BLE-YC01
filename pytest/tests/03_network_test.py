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
    result = workbench.serial_write(slot=slot, data=f"SET_CONFIG {config_json}\n", pattern="Config saved successfully.", timeout=20)
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
    print("stations: ", stations)
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
    time.sleep(2)
    event = workbench.wait_for_station(timeout=60)
    assert event["type"] == "STA_CONNECT"
    print("ESP reconnected successfully")
    
    # Final check
    time.sleep(2)
    status = workbench.ap_status()
    print("status: ", status)
    assert len(status.get("stations", [])) > 0, "ESP not found in station list after reconnect"
    print("WiFi lifecycle test PASSED")


def test_wifi_reconnection_after_loss(workbench, slot, wifi_network):
    """
    Test if the ESP restores Wi-Fi connection after loss:
    1. Connect to AP.
    2. Stop AP.
    3. Wait for ESP to enter standby mode (after wifiTimeout).
    4. Restart AP.
    5. Wait for ESP to retry and reconnect.
    """
    
    ssid = wifi_network["ssid"]
    password = wifi_network["password"]
    wifi_timeout = 15
    
    print(f"--- Step 1: Configuring ESP with wifiTimeout={wifi_timeout} ---")
    
    config = {
        "wifiSSID": ssid,
        "wifiPassword": password,
        "wifiTimeout": wifi_timeout,
        "interval": 60
    }
    
    # Configure ESP
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully.", timeout=20)
    assert result.get("matched"), "ESP did not save config"
    
    # Wait for initial connection
    print("Waiting for initial connection...")
    result = workbench.serial_monitor(slot=slot, pattern="WiFi successfully connected", timeout=30)
    assert result.get("matched"), "Initial WiFi connection failed"
    
    # 2. Stop the AP
    print("--- Step 2: Stopping AP to simulate connection loss ---")
    workbench.ap_stop()
    
    # 3. Wait for ESP to enter standby
    print(f"Waiting for ESP to enter standby mode (should happen after {wifi_timeout}s)...")
    # It takes wifi_timeout to detect and enter standby
    result = workbench.serial_monitor(slot=slot, pattern="entering standby mode due to WiFi disconnection timeout", timeout=wifi_timeout + 15)
    assert result.get("matched"), "ESP did not enter standby mode after connection loss"
    
    # 4. Restart the AP
    print("--- Step 3: Restarting AP ---")
    workbench.ap_start(ssid, password)
    time.sleep(2)

    # 5. Wait for ESP to retry and reconnect
    print("Waiting for ESP to retry connection from standby...")
    # ESP retries every wifiTimeout seconds
    result = workbench.serial_monitor(slot=slot, pattern="Standby: WiFi reconnected!", timeout=wifi_timeout + 10)
    print("reconnected: ", result)
    assert result.get("matched"), "ESP did not reconnect"
    
    print("--- Step 4: Final verification ---")
    # Check if reachable via HTTP
    status = workbench.ap_status()
    stations = status.get("stations", [])
    assert len(stations) > 0, "ESP not connected to AP in workbench status"
    
    esp_ip = stations[0].get("ip")
    resp = workbench.http_get(f"http://{esp_ip}/status", timeout=10)
    assert resp.status_code == 200
    data = resp.json()
    assert data.get("isStandby") is False, "ESP still reports isStandby=true"
    
    print("WiFi reconnection after loss test PASSED")


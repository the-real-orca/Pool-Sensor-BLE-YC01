import pytest
import time
import json

def test_mqtt_deactivation_offline(workbench, slot, wifi_network, test_progress):
    """
    Test if MQTT is deactivated when the device enters OFFLINE (standby) mode.
    """
    test_progress("Phase 1: Setup WiFi and MQTT")
    workbench.mqtt_start()
    time.sleep(2)
    
    config = {
        "wifiSSID": wifi_network.get("ssid"),
        "wifiPassword": wifi_network.get("password"),
        "wifiTimeout": 120,
        "mqttServer": wifi_network.get("ap_ip"),
        "mqttPort": 1883,
        "mqttTopic": "/test/topic",
        "interval": 10
    }
    
    # Configure ESP
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="WiFi successfully connected", timeout=15)
    assert result.get("matched")

    test_progress("Waiting for initial connection and MQTT status")
    # Wait for ESP to reboot and connect
    time.sleep(10)
    
    status = workbench.ap_status()
    stations = status.get("stations", [])
    assert len(stations) > 0, "No stations connected to AP"
    esp_ip = stations[0].get("ip")
    
    # Wait for mqttConnected to be true in HTTP /status
    mqtt_connected = False
    for i in range(10):
        try:
            resp = workbench.http_get(f"http://{esp_ip}/status", timeout=5)
            if resp.status_code == 200 and resp.json().get("mqttConnected") is True:
                mqtt_connected = True
                break
        except Exception:
            pass
        time.sleep(3)
    
    assert mqtt_connected, "MQTT failed to connect initially"

    test_progress("Phase 2: Enter OFFLINE mode")
    # Send OFFLINE command
    result = workbench.serial_write(slot=slot, data="\nOFFLINE\n", pattern="Entering Standby", timeout=15)
    assert result.get("matched")

    time.sleep(2)

    test_progress("Phase 3: Verify MQTT is disconnected in STATUS")
    
    result = workbench.serial_write(slot=slot, data="\nSTATUS\n", pattern='"mqttConnected":false', timeout=15)
    assert result.get("matched"), "MQTT should be disconnected in OFFLINE mode"

    test_progress("Phase 4: Verify MQTT does not try to reconnect in standby")
    # Wait for a bit longer than the usual retry interval (10s)
    # If it tries to reconnect, we would see "connecting to MQTT-broker..."
    result = workbench.serial_monitor(slot=slot, pattern="connecting to MQTT-broker...", timeout=15)
    assert not result.get("matched"), "MQTT should NOT attempt to reconnect while in standby"

def test_mqtt_deactivation_captive_portal(workbench, slot, test_progress):
    """
    Test if MQTT is deactivated when the device enters Captive Portal mode.
    Force captive portal by sending a RESET without providing valid WiFi config.
    """
    test_progress("Phase 1: Setup MQTT and trigger WiFi timeout")
    workbench.mqtt_start()
    
    # wait between tests for serial communication
    time.sleep(3)

    # 1. Force Portal Mode via Serial API
    config = {
        "wifiSSID": "NonExistentWiFi_12345",
        "wifiPassword": "wrongpassword",
        "wifiTimeout": 5, # Fast timeout for test
        "portalSSID": "ESP32-Portal-Test",
        "portalPassword": "",
        "portalTimeout": 120
    }
    
    # We use serial_write and check if it matches the success pattern
    test_progress("Waiting for Captive Portal to start")
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="starting captive portal", timeout=15)
    assert result.get("matched"), "ESP32 did not start Captive Portal within timeout"


    test_progress("Phase 2: Verify MQTT deactivation")
    # Wait to see if it tries to connect to MQTT (it shouldn't)
    # The mqttLoop retry interval is 10s
     
    # Check for reconnection attempts
    result = workbench.serial_monitor(slot=slot, pattern="connecting to MQTT-broker...", timeout=5)
    assert not result.get("matched"), "MQTT should NOT attempt to connect in Captive Portal mode"

    time.sleep(2)

    # Final check via STATUS command
    result = workbench.serial_write(slot=slot, data="\nSTATUS\n", pattern='"mqttConnected":false', timeout=15)
    assert result.get("matched"), "MQTT should be disconnected in OFFLINE mode"



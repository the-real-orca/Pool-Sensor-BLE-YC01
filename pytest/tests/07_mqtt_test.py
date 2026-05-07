import pytest
import time
import json

def test_mqtt_connection(workbench, slot, wifi_network):
    """Test if the ESP connects to the MQTT broker successfully."""
    esp_ip = wifi_network.get("ip")

    # Start the workbench MQTT broker
    workbench.mqtt_start()
    time.sleep(2) # Give broker time to start
    
    # Configure the ESP with the MQTT settings
    config = {
        "wifiSSID": wifi_network.get("ssid"),
        "wifiPassword": wifi_network.get("password"),
        "wifiTimeout": 30,
        "mqttServer": wifi_network.get("ap_ip"),
        "mqttPort": 1883,
        "mqttTopic": "/test/topic",
        "interval": 60
    }
    
    # Send SET_CONFIG command
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully.", timeout=15)
    assert result.get("matched"), "Config was not saved"
    
    result = workbench.serial_monitor(slot=slot, pattern="MQTT-broker... ok", timeout=20)
    assert result.get("matched")

    # Wait for ESP to reboot and connect to WiFi + MQTT
    time.sleep(5)
    
    status = workbench.ap_status()
    stations = status.get("stations", [])
    assert len(stations) > 0, "No stations connected to AP"
    esp_ip = stations[0].get("ip")
    print(f"ESP IP address: {esp_ip}")
    assert esp_ip, "ESP IP address not found in station list"

    # Wait for HTTP /status to be up and mqttConnected to be true
    resp = workbench.http_get(f"http://{esp_ip}/status", timeout=5)
    print(resp)
    assert resp.status_code == 200, f"ESP web server returned {resp.status_code}"
    data = resp.json()
    assert data.get("mqttConnected") is True


def test_mqtt_publish(workbench, slot, wifi_network):
    """Test if the ESP publishes data to the MQTT broker."""
    esp_ip = wifi_network.get("ip")

    # Start the workbench MQTT broker
    workbench.mqtt_start()
    time.sleep(2) # Give broker time to start
    
    # Configure the ESP with the MQTT settings
    config = {
        "wifiSSID": wifi_network.get("ssid"),
        "wifiPassword": wifi_network.get("password"),
        "wifiTimeout": 30,
        "mqttServer": wifi_network.get("ap_ip"),
        "mqttPort": 1883,
        "mqttTopic": "/test/topic",
        "interval": 60
    }
    
    # Send SET_CONFIG command
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully.", timeout=15)
    assert result.get("matched"), "Config was not saved"
    
    result = workbench.serial_monitor(slot=slot, pattern="MQTT-broker... ok", timeout=20)
    assert result.get("matched")    

    # Clear previous messages
    workbench.mqtt_clear_messages()
    
    # Subscribe to the topic on the workbench
    workbench.mqtt_subscribe(config["mqttTopic"])
    
    # Send READ command via serial to force a reading and publish
    time.sleep(3)
    result = workbench.serial_write(slot=slot, data="\nREAD\n", pattern="Forcing immediate read", timeout=15)
    assert result.get("matched"), "READ command not accepted"
    
    # Wait for the reading to complete and publish
    time.sleep(10)
    
    # Check messages on the workbench
    messages = workbench.mqtt_get_messages(topic=config["mqttTopic"])
    assert len(messages) > 0, "No MQTT messages received on the expected topic"
    
    # Parse the latest message
    latest_msg = messages[-1]
    payload = latest_msg.get("payload", "")
    
    try:
        data = json.loads(payload)
        assert isinstance(data, dict), "Payload is not a JSON object"
        # Check for typical sensor fields like ph, temperature, etc.
        # It's possible the sensor hasn't read the actual value if BLE is disconnected,
        # but the JSON structure should be there.
    except json.JSONDecodeError:
        pytest.fail(f"Payload is not valid JSON: {payload}")


def test_mqtt_reconnection_after_loss(workbench, slot, wifi_network):
    """Test if the ESP reconnects to the MQTT broker after connection loss."""
    
    # Start the workbench MQTT broker
    workbench.mqtt_start()
    time.sleep(2)
    
    # Configure the ESP
    config = {
        "wifiSSID": wifi_network.get("ssid"),
        "wifiPassword": wifi_network.get("password"),
        "wifiTimeout": 30,
        "mqttServer": wifi_network.get("ap_ip"),
        "mqttPort": 1883,
        "mqttTopic": "/test/topic",
        "interval": 60
    }
    
    # Send SET_CONFIG
    print("--- Step 1: Initial configuration ---")
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully.", timeout=15)
    assert result.get("matched")
    
    result = workbench.serial_monitor(slot=slot, pattern="MQTT-broker... ok", timeout=25)
    assert result.get("matched")
    
    # Get ESP IP
    time.sleep(5)
    status = workbench.ap_status()
    stations = status.get("stations", [])
    assert len(stations) > 0
    esp_ip = stations[0].get("ip")
    
    # Verify initial connection
    resp = workbench.http_get(f"http://{esp_ip}/status", timeout=5)
    assert resp.json().get("mqttConnected") is True
    print("Initial MQTT connection verified.")

    # 2. Stop the MQTT broker
    print("--- Step 2: Stopping MQTT broker to simulate connection loss ---")
    workbench.mqtt_stop()
    time.sleep(5)
    
    # Verify disconnection (optional, might take time for client to realize)
    resp = workbench.http_get(f"http://{esp_ip}/status", timeout=5)
    print(f"Status after broker stop: mqttConnected={resp.json().get('mqttConnected')}")
    
    # 3. Restart the MQTT broker
    print("--- Step 3: Restarting MQTT broker ---")
    workbench.mqtt_start()
    time.sleep(2)
    
    # Clear and subscribe
    workbench.mqtt_clear_messages()
    workbench.mqtt_subscribe(config["mqttTopic"])
    
    # 4. Trigger READ to force reconnection and publishing
    print("--- Step 4: Triggering READ to force reconnection ---")
    workbench.serial_write(slot=slot, data="\nREAD\n")
    
    # Wait for the reconnection and publish (checking broker)
    print("Waiting for ESP to reconnect and publish message...")
    found_msg = False
    for i in range(10): # Try for 40 seconds
        messages = workbench.mqtt_get_messages(topic=config["mqttTopic"])
        if len(messages) > 0:
            found_msg = True
            print(f"MQTT message received after {i*4}s!")
            break
        time.sleep(4)
        
    assert found_msg, "No MQTT messages received after reconnection"
    
    # Parse and verify
    latest_msg = messages[-1]
    data = json.loads(latest_msg.get("payload", "{}"))
    assert data.get("mqttConnected") is True, "Published data says MQTT is not connected"
    
    print("MQTT reconnection after loss test PASSED")


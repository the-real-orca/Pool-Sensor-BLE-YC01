import json
import time

def test_http_get_password_masked(workbench, slot, wifi_connection):
    esp_ip = wifi_connection.get("ip")
    # GET /config.json should be allowed, but passwords masked
    # Note: Accessing directly via standard requests/workbench might need to be adjusted if workbench requires specific wrappers
    # Based on previous failure, we check for 200 OK and mask
    resp = workbench.http_get(f"http://{esp_ip}/config.json", timeout=10)

    assert resp.status_code == 200, "GET /config.json should be allowed"

    config = resp.json()

    # Verify passwords are masked (assuming DEBUG_SECURITY=0)
    assert config.get("wifiPassword") == "***", "wifiPassword should be masked"
    assert config.get("mqttPassword") == "***", "mqttPassword should be masked"

def test_serial_get_config_password_masked(workbench, slot):
    # Give workbench time to startup
    time.sleep(1)

    # Use serial_write with the correct pattern to capture the JSON
    # GET_CONFIG prints the configuration, ending with a newline.
    result = workbench.serial_write(slot=slot, data="\nGET_CONFIG\n", pattern="interval", timeout=10)

    # Extract the full output
    output_lines = result.get("output", [])
    full_output = "\n".join(output_lines)
    print (full_output)

    # For now, check for the mask in the string output
    assert "***" in full_output, "Passwords should be masked in Serial output"
    assert "wifiPassword" in full_output
    assert "mqttPassword" in full_output


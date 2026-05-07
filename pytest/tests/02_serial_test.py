import pytest
import time
import json

def test_serial_api_offline(workbench, slot, test_progress):
    """Test the OFFLINE command of the Serial API."""

    test_progress("Waiting for serial readiness")
    # wait between tests for serial communication
    time.sleep(3)

    test_progress("Sending OFFLINE command")
    # Send STATUS and wait for JSON line (starts with {)
    result = workbench.serial_write(slot=slot, data="\nOFFLINE\n", pattern='Standby Mode', timeout=15)
    assert result.get("matched")

    test_progress("Requesting status via serial")
    # Send STATUS and wait for JSON line (starts with {)
    result = workbench.serial_write(slot=slot, data="\nSTATUS\n", pattern='}', timeout=15)
    assert result.get("matched"), "Pattern '{' not found after STATUS command"
    
    test_progress("Validating JSON response")
    # Validate it is a valid JSON (the whole line is captured because pattern matches it)
    line = result.get("line")
    try:
        status = json.loads(line)
        assert "status" in status
    except json.JSONDecodeError:
        pytest.fail(f"Output is not valid JSON: {line}")

def test_serial_api_scan(workbench, slot, test_progress):
    """Test the SCAN commands."""
    
    test_progress("Waiting for serial readiness")
    # wait between tests for serial communication
    time.sleep(3)

    test_progress("Requesting BLE scan")
    # Test SCAN
    result = workbench.serial_write(slot=slot, data="\nSCAN\n", pattern="Forcing re-scan", timeout=15)
    assert result.get("matched")

def test_serial_api_read(workbench, slot, test_progress):
    """Test the READ commands."""

    test_progress("Waiting for serial readiness")
    # wait between tests for serial communication
    time.sleep(3)

    test_progress("Requesting immediate sensor read")
    # Test READ
    result = workbench.serial_write(slot=slot, data="\nREAD\n", pattern="Forcing immediate read", timeout=15)
    assert result.get("matched")

    test_progress("Waiting for read to complete")
    # Give esp time to read
    time.sleep(5)


def test_serial_api_get_config(workbench, slot, test_progress):
    """Test the GET_CONFIG command."""

    test_progress("Waiting for serial readiness")
    # wait between tests for serial communication
    time.sleep(3)

    test_progress("Requesting current configuration")
    # Send GET_CONFIG command and wait for a pattern that appears after the JSON output.
    # This ensures the entire config JSON is captured in the output buffer.
    result = workbench.serial_write(slot=slot, data="\nGET_CONFIG\n", pattern="}", timeout=15)
    assert result.get("matched")

    full_output = "\n".join(result.get("output", []))
    assert "Current configuration:" in full_output
    assert "wifiSSID" in full_output
    assert "mqttServer" in full_output
    assert "interval" in full_output

def test_serial_api_set_config(workbench, slot, test_progress):
    """Test the SET_CONFIG command and JSON upload."""

    test_progress("Waiting for serial readiness")
    # wait between tests for serial communication
    time.sleep(3)

    # Prepare a test config
    test_config = {
        "wifiSSID": "TestSSID",
        "wifiPassword": "TestPassword",
        "interval": 60
    }
    config_json = json.dumps(test_config)
    
    test_progress("Uploading new configuration")
    # Send SET_CONFIG command with JSON
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {config_json}\n", pattern="Config saved successfully.", timeout=15)
    assert result.get("matched")
    
    test_progress("Waiting for ESP to apply config and restart")
    # Give esp time to restart
    time.sleep(5)

    test_progress("Verifying configuration update")
    # Read result    
    result = workbench.serial_write(slot=slot, data="\nGET_CONFIG\n", pattern="}", timeout=15)
    full_output = "\n".join(result.get("output", []))
    assert "Current configuration:" in full_output
    assert "wifiSSID" in full_output
    assert "TestSSID" in full_output
    assert "mqttServer" in full_output
    assert "interval" in full_output


def test_serial_api_reset(workbench, slot, test_progress):
    """Test the RESET command."""

    test_progress("Waiting for serial readiness")
    # wait between tests for serial communication
    time.sleep(3)

    test_progress("Triggering serial RESET")
    # Send RESET command and wait for rebooting message
    result = workbench.serial_write(slot=slot, data="\nRESET\n", pattern="application starting", timeout=20)
    
    test_progress("Waiting for restart")
    # Give esp time to restart
    time.sleep(5)
    
    assert result.get("matched")


def test_serial_api_unknown(workbench, slot):
    """Test behavior for unknown commands."""

    # wait between tests for serial communication
    time.sleep(3)

    # Send unsupported command and wait for a pattern to appear.
    result = workbench.serial_write(slot=slot, data="\nUNKNOWN_CMD\n", pattern="Unknown command", timeout=15)
    assert result.get("matched")
   

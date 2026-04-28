import pytest
import time
import json

def test_serial_api_offline(workbench, slot):
    """Test the OFFLINE command of the Serial API."""

    # wait between tests for serial communication
    time.sleep(3)

    # Send STATUS and wait for JSON line (starts with {)
    result = workbench.serial_write(slot=slot, data="\nOFFLINE\n", pattern='Offline Mode', timeout=15)
    assert result.get("matched")

def test_serial_api_status(workbench, slot):
    """Test the STATUS command of the Serial API."""

    # wait between tests for serial communication
    time.sleep(3)

    # Send STATUS and wait for JSON line (starts with {)
    result = workbench.serial_write(slot=slot, data="\nSTATUS\n", pattern='}', timeout=15)
    assert result.get("matched"), "Pattern '{' not found after STATUS command"
    
    # Validate it is a valid JSON (the whole line is captured because pattern matches it)
    line = result.get("line")
    try:
        status = json.loads(line)
        assert "status" in status
    except json.JSONDecodeError:
        pytest.fail(f"Output is not valid JSON: {line}")

def test_serial_api_scan(workbench, slot):
    """Test the SCAN commands."""
    
    # wait between tests for serial communication
    time.sleep(3)

    # Test SCAN
    result = workbench.serial_write(slot=slot, data="\nSCAN\n", pattern="Forcing re-scan", timeout=15)
    assert result.get("matched")

def test_serial_api_read(workbench, slot):
    """Test the READ commands."""

    # wait between tests for serial communication
    time.sleep(3)

    # Test READ
    result = workbench.serial_write(slot=slot, data="\nREAD\n", pattern="Forcing immediate read", timeout=15)
    assert result.get("matched")

    # Give esp time to read
    time.sleep(5)


def test_serial_api_get_config(workbench, slot):
    """Test the GET_CONFIG command."""

    # wait between tests for serial communication
    time.sleep(3)

    # Send GET_CONFIG command and wait for a pattern that appears after the JSON output.
    # This ensures the entire config JSON is captured in the output buffer.
    result = workbench.serial_write(slot=slot, data="\nGET_CONFIG\n", pattern="}", timeout=15)
    assert result.get("matched")

    full_output = "\n".join(result.get("output", []))
    assert "Current configuration:" in full_output
    assert "wifiSSID" in full_output
    assert "mqttServer" in full_output
    assert "interval" in full_output

def test_serial_api_set_config(workbench, slot):
    """Test the SET_CONFIG command and JSON upload."""

    # wait between tests for serial communication
    time.sleep(3)

    # Prepare a test config
    test_config = {
        "wifiSSID": "TestSSID",
        "wifiPassword": "TestPassword",
        "interval": 60
    }
    config_json = json.dumps(test_config)
    
    # Send SET_CONFIG command with JSON
    result = workbench.serial_write(slot=slot, data=f"\nSET_CONFIG {config_json}\n", pattern="Config saved successfully.", timeout=15)
    assert result.get("matched")
    
    # Give esp time to restart
    time.sleep(5)

    # Read result    
    result = workbench.serial_write(slot=slot, data="\nGET_CONFIG\n", pattern="}", timeout=15)
    full_output = "\n".join(result.get("output", []))
    assert "Current configuration:" in full_output
    assert "wifiSSID" in full_output
    assert "TestSSID" in full_output
    assert "mqttServer" in full_output
    assert "interval" in full_output


def test_serial_api_reset(workbench, slot):
    """Test the RESET command."""

    # wait between tests for serial communication
    time.sleep(3)

    # Send RESET command and wait for rebooting message
    result = workbench.serial_write(slot=slot, data="\nRESET\n", pattern="application starting", timeout=20)
    
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
   

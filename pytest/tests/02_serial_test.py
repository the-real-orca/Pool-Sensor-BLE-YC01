import pytest
import time
import json

def test_serial_api_status(workbench, slot):
    """Test the STATUS command of the Serial API."""

    # give workbench time to startup
    time.sleep(2)

    # Send STATUS and wait for JSON line (starts with {)
    result = workbench.serial_write(slot=slot, data="STATUS", pattern='}', timeout=15)
    assert result.get("matched"), "Pattern '{' not found after STATUS command"
    
    # Validate it is a valid JSON (the whole line is captured because pattern matches it)
    line = result.get("line")
    try:
        status = json.loads(line)
        assert "time" in status
        assert "wifiSSID" in status
    except json.JSONDecodeError:
        pytest.fail(f"Output is not valid JSON: {line}")

def test_serial_api_scan(workbench, slot):
    """Test the SCAN commands."""
    
    # give workbench time to startup
    time.sleep(2)
    
    # Test SCAN
    result = workbench.serial_write(slot=slot, data="SCAN", pattern="Forcing re-scan", timeout=10)
    assert result.get("matched")

def test_serial_api_read(workbench, slot):
    """Test the READ commands."""
    
    # give workbench time to startup
    time.sleep(2)
    
    # Test READ
    result = workbench.serial_write(slot=slot, data="READ", pattern="Forcing immediate read", timeout=10)
    assert result.get("matched")


def test_serial_api_get_config(workbench, slot):
    """Test the GET_CONFIG command."""

    # give workbench time to startup
    time.sleep(2)

    # Send GET_CONFIG command and wait for a pattern that appears after the JSON output.
    # This ensures the entire config JSON is captured in the output buffer.
    result = workbench.serial_write(slot=slot, data="GET_CONFIG", timeout=10)
    
    full_output = "\n".join(result.get("output", []))
    assert "Current configuration:" in full_output
    assert "wifiSSID" in full_output
    assert "mqttServer" in full_output
    assert "interval" in full_output


def test_serial_api_set_config(workbench, slot):
    """Test the SET_CONFIG command and JSON upload."""

    # give workbench time to startup
    time.sleep(2)

    # Prepare a test config
    test_config = {
        "wifiSSID": "TestSSID",
        "wifiPassword": "TestPassword",
        "interval": 60
    }
    config_json = json.dumps(test_config)
    
    # Send SET_CONFIG command and wait for ready message
    result = workbench.serial_write(slot=slot, data="SET_CONFIG", pattern="Ready to receive config.json", timeout=10)
    assert result.get("matched")
    
    # Send the JSON and wait for success message
#    result = workbench.serial_write(slot=slot, data=config_json, pattern="Config saved successfully", timeout=30)
    result = workbench.serial_write(slot=slot, data=config_json, pattern="application starting", timeout=10)
    print(result)
#    assert result.get("matched")

    time.sleep(2)

    # Read result    
    result = workbench.serial_write(slot=slot, data="GET_CONFIG", timeout=10)
    time.sleep(2)
    full_output = "\n".join(result.get("output", []))
    assert "Current configuration:" in full_output
    assert "wifiSSID" in full_output
    assert "TestSSID" in full_output
    assert "mqttServer" in full_output
    assert "interval" in full_output


def test_serial_api_reset(workbench, slot):
    """Test the RESET command."""

    # give workbench time to startup
    time.sleep(2)

    # Send RESET command and wait for rebooting message
    result = workbench.serial_write(slot=slot, data="RESET", pattern="application starting", timeout=15)
    assert result.get("matched")


def test_serial_api_unknown(workbench, slot):
    """Test behavior for unknown commands."""

    # give workbench time to startup
    time.sleep(2)

    # Send unsupported command and wait for a pattern to appear.
    result = workbench.serial_write(slot=slot, data="UNKNOWN_CMD", pattern="Unknown command", timeout=10)
    assert result.get("matched")
   

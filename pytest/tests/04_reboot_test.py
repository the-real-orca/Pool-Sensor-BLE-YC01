import pytest
import time


def test_reboot_serial(workbench, slot):
    """Test if rebooting via Serial API shows the correct reason and identifies the reset reason."""

    # wait between tests for serial communication
    time.sleep(3)

    # Trigger Reset via Serial
    result = workbench.serial_write(slot=slot, data="\nRESET\n", pattern="Reset reason: Software reset", timeout=15)

    # Wait for device to be ready
    time.sleep(1)

    full_output = "\n".join(result.get("output", []))
    assert "Reboot requested. Reason: Serial RESET" in full_output
    assert "application starting" in full_output
    assert "Reset reason: Software reset" in full_output

    assert result.get("matched"), "Serial console did not show reboot request with reason"


def test_reboot_hard(workbench, slot):
    """Test if rebooting via workbench API shows the correct reason and identifies the reset reason."""

    # Wait for device to be ready
    time.sleep(1)

    # Trigger Reset via Workbench
    result = workbench.serial_reset(slot=slot)

    full_output = "\n".join(result.get("output", []))
    assert "application starting" in full_output
    #assert "Reset reason: Power-on" in full_output



def test_reboot_http(workbench, slot, wifi_connection):
    """Test if rebooting via HTTP command shows the correct reason in status."""
    esp_ip = wifi_connection.get("ip")
    
    # Trigger Reboot via HTTP
    print(f"Sending HTTP reboot command to http://{esp_ip}/cmd?param=reboot&value=1")
    resp = workbench.http_get(f"http://{esp_ip}/cmd?param=reboot&value=1", timeout=10)
    assert resp.status_code == 200, f"HTTP command failed with status {resp.status_code}"

    result = workbench.serial_monitor(slot=slot, pattern="Reset reason: Software reset", timeout=15)
    full_output = "\n".join(result.get("output", []))
    assert "application starting" in full_output
    assert "Reset reason: Software reset" in full_output

    assert result.get("matched"), "Serial console did not show reboot request with reason"



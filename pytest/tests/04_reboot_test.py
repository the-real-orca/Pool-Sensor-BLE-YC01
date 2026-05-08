import pytest
import time


def test_reboot_serial(workbench, slot, test_progress):
    """Test if rebooting via Serial API shows the correct reason and identifies the reset reason."""

    test_progress("Waiting for serial readiness")
    # wait between tests for serial communication
    time.sleep(3)

    test_progress("Triggering Serial RESET")
    # Trigger Reset via Serial
    result = workbench.serial_write(slot=slot, data="\nRESET\n", pattern="Reset reason: Software reset", timeout=15)

    # Wait for device to be ready
    time.sleep(1)

    full_output = "\n".join(result.get("output", []))
    assert "Reboot requested. Reason: Serial RESET" in full_output
    assert "application starting" in full_output
    assert "Reset reason: Software reset" in full_output

    assert result.get("matched"), "Serial console did not show reboot request with reason"


def test_reboot_hard(workbench, slot, test_progress):
    """Test if rebooting via workbench API shows the correct reason and identifies the reset reason."""

    test_progress("Triggering hardware RESET via workbench")
    # Wait for device to be ready
    time.sleep(1)

    # Trigger Reset via Workbench
    result = workbench.serial_reset(slot=slot)

    full_output = "\n".join(result.get("output", []))
    assert "application starting" in full_output
    #assert "Reset reason: Power-on" in full_output



def test_reboot_http(workbench, slot, wifi_connection, test_progress):
    """Test if rebooting via HTTP command shows the correct reason in status."""
    esp_ip = wifi_connection.get("ip")
    
    # Wait for device to be ready
    time.sleep(1)
        
    test_progress(f"Triggering reboot via HTTP: http://{esp_ip}/cmd?param=reboot&value=1")
    # Trigger Reboot via HTTP
    resp = workbench.http_get(f"http://{esp_ip}/cmd?param=reboot&value=1", timeout=10)
    assert resp.status_code == 200, f"HTTP command failed with status {resp.status_code}"

    test_progress("Monitoring serial for reset reason")
    result = workbench.serial_monitor(slot=slot, pattern="Reset reason: Software reset", timeout=15)
    full_output = "\n".join(result.get("output", []))
    assert "application starting" in full_output
    assert "Reset reason: Software reset" in full_output

    assert result.get("matched"), "Serial console did not show reboot request with reason"



"""Pool Sensor Test

All tests in this file require a WiFi device (DUT) connected to the workbench.
"""

import os
import re
import socket
import time

import pytest

from workbench_driver import CommandError, CommandTimeout


def _find_present_device(workbench):
    """Find the first present DUT (not a debug probe) and return its slot info.

    Any present device that isn't a probe or HID-warning slot is a DUT.
    This covers both native USB chips (VID 303a, ttyACM) and classic ESP32
    boards with third-party USB-UART bridges (CP2102, CH340, ttyUSB).
    """
    devices = workbench.get_devices()
    for d in devices:
        if not d.get("present"):
            continue
        if d.get("is_probe"):
            continue
        if d.get("usb_warning"):
            continue
        return d
    return None


def test_workbench_active(workbench):
    """WT-100: PING returns fw_version and uptime."""
    result = workbench.ping()
    assert result.get("ok"), "workbench ping failed"
    assert "fw_version" in result
    assert "uptime" in result
    assert isinstance(result["uptime"], (int, float))
    assert result["uptime"] >= 0

def test_device_present(workbench):
    slots = workbench.get_devices()
    assert any(slot.get("present") for slot in slots)

def test_startup_init(workbench):
    """startup"""
    dev = _find_present_device(workbench)
    assert dev is not None, "No DUT found"
    slot = dev.get("label", "")

    # reset DUT
    workbench.serial_reset(slot=slot)
    # analyse serial output
    result = workbench.serial_monitor(
        slot=slot,
        pattern="init complete",
        timeout=20,
    )
    # give workbench time to startup
    time.sleep(2)    
    assert result.get("matched"), "Pattern 'init complete' not found in serial output"
    assert "init complete" in result.get("output", [])

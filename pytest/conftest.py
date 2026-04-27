"""Pytest fixtures for the Embedded Workbench (HTTP-only, Pi backend).

Usage:
    pytest test_instrument.py --wt-url http://<pi-ip>:8080
"""

import os
import uuid
import json

import pytest

from workbench_driver import WorkbenchDriver


def pytest_addoption(parser):
    parser.addoption(
        "--wt-url",
        default=os.environ.get("WORKBENCH_URL", "http://esp32-workbench.home:8080/"),
        help="Portal URL for the Embedded Workbench Pi",
    )


def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "requires_dut: test needs a DUT or second WiFi device connected",
    )


@pytest.hookimpl(tryfirst=True, hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """Attach test outcome to the node so fixtures can read it."""
    outcome = yield
    rep = outcome.get_result()
    setattr(item, f"rep_{rep.when}", rep)


@pytest.fixture(scope="session")
def workbench(request):
    """Session-scoped connection to the Embedded Workbench."""
    url = request.config.getoption("--wt-url")
    driver = WorkbenchDriver(url)
    driver.open()
    driver.ping()
    yield driver
    try:
        driver.ap_stop()
    except Exception:
        pass
    driver.close()


@pytest.fixture(scope="session")
def slot(workbench):
    """Fixture to find and return the first present DUT slot label."""
    devices = workbench.get_devices()
    for d in devices:
        if not d.get("present"):
            continue
        if d.get("is_probe"):
            continue
        if d.get("usb_warning"):
            continue
        return d.get("label", "SLOT2")
    pytest.skip("No DUT found on workbench")
    return None

@pytest.fixture
def wifi_network(workbench):
    """Start a fresh AP for this test, stop on teardown."""
    ssid = f"WT-{uuid.uuid4().hex[:6].upper()}"
    password = "testpass123"
    workbench.drain_events()
    workbench.ap_start(ssid, password)
    yield {"ssid": ssid, "password": password, "ap_ip": "192.168.4.1"}
    workbench.ap_stop()

@pytest.fixture
def wifi_connection(workbench, slot, wifi_network):
    """Fixture to configure ESP and ensure it is connected to the workbench AP."""
    ssid = wifi_network["ssid"]
    password = wifi_network["password"]
    
    print(f"Configuring ESP to connect to {ssid}...")
    config = {"wifiSSID": ssid, "wifiPassword": password}
    workbench.serial_write(slot=slot, data=f"SET_CONFIG {json.dumps(config)}", pattern="Config saved successfully.", timeout=10)
    
    print("Waiting for connection...")
    station = workbench.wait_for_station(timeout=45)
    assert station.get("ip"), "Could not get ESP IP address"
    return station

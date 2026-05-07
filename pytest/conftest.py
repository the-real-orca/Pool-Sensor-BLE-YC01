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


@pytest.fixture(scope="session", autouse=True)
def workbench_test_suite(request, workbench):
    """Initializes the workbench UI for the entire test session."""
    total = len(request.session.items)
    workbench.test_start(spec="Pool Sensor Suite", phase="Automated Testing", total=total)
    yield
    workbench.test_end()


@pytest.fixture(autouse=True)
def test_progress(request, workbench):
    """Fixture to log steps and results to the workbench UI automatically."""
    # Only report for tests that actually use workbench or its derivatives
    relevant_fixtures = {"workbench", "slot", "wifi_network", "wifi_connection"}
    if not any(f in request.fixturenames for f in relevant_fixtures):
        yield None
        return

    node = request.node
    
    # Format the file name (remove path and .py)
    file_name = os.path.basename(node.nodeid.split("::")[0]).replace(".py", "")
    
    # Human friendly name transformation
    def humanize(s):
        s = s[5:] if s.startswith("test_") else s
        s = s.replace("_", " ").title()
        return s.replace("Api", "API").replace("Wifi", "WiFi").replace("Mqtt", "MQTT").replace("Ble", "BLE").replace("Http", "HTTP").replace("Rest", "REST").replace("Usb", "USB").replace("Sw", "SW").replace("Hw", "HW")

    test_name = humanize(node.name)
    # Human friendly ID (e.g. 02 Serial Test :: Serial API Status)
    test_id = f"{humanize(file_name)} :: {test_name}"
    
    # Send initial "Running" step
    workbench.test_step(test_id=test_id, name=test_name, step="Executing test...")
    
    def step(msg, manual=False):
        workbench.test_step(test_id=test_id, name=test_name, step=msg, manual=manual)
        
    yield step
    
    # Send result after test execution
    rep_call = getattr(node, "rep_call", None)
    rep_setup = getattr(node, "rep_setup", None)
    
    if rep_setup and rep_setup.failed:
        outcome = "FAIL"
        details = f"Setup failed: {rep_setup.longreprtext}"
    elif rep_call:
        if rep_call.passed:
            outcome = "PASS"
            details = ""
        elif rep_call.failed:
            outcome = "FAIL"
            details = str(rep_call.longreprtext)
        elif rep_call.skipped:
            outcome = "SKIP"
            details = str(rep_call.was_skipped)
        else:
            outcome = "SKIP"
            details = "Unknown state"
    else:
        outcome = "SKIP"
        details = "No execution report"

    workbench.test_result(test_id=test_id, name=test_name, result=outcome, details=details)


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
    
    #print(f"Configuring ESP to connect to {ssid}...")
    config = {"wifiSSID": ssid, "wifiPassword": password}
    workbench.serial_write(slot=slot, data=f"SET_CONFIG {json.dumps(config)}\n", pattern="Config saved successfully.", timeout=10)
    
    station = workbench.wait_for_station(timeout=45)
    assert station.get("ip"), "Could not get ESP IP address"
    return station

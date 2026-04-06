# Test Dokumentation

## Überblick

Die Universal-ESP32-Workbench lässt sich sehr gut mit `pytest` als Testframework für ESP-Projekte verwenden.  
Dabei ist die Workbench **nicht selbst das Testframework**, sondern die **Hardware- und Infrastruktur-Schicht**:

- Der Raspberry Pi übernimmt die Steuerung der physischen Testumgebung.
- Die Workbench stellt dafür HTTP-Endpunkte für Serial, WiFi, BLE, GPIO, Debug, Firmware und Teststatus bereit.
- `pytest` orchestriert die Testabläufe.
- Die ESP-Firmware ist das eigentliche Device Under Test (DUT).

Das ergibt eine saubere Rollenverteilung:

- **Workbench** = Hardware-Abstraktion und Steuerung
- **pytest** = Test-Orchestrierung und Assertions
- **ESP-Firmware** = zu testendes System

Diese Trennung ist besonders nützlich für:

- Smoke-Tests nach dem Flashen
- Boot- und Serial-Checks
- WiFi-/HTTP-Tests
- OTA-Tests
- BLE-Tests
- Human-in-the-loop-Tests
- Debug-/JTAG-Tests

---

## Grundprinzip

Die Workbench stellt einen HTTP-Server bereit, über den Tests hardware-nahe Aktionen auslösen können, zum Beispiel:

- Geräte erkennen
- serielle Logs lesen
- ein Board resetten
- WLAN-AP starten
- HTTP-Requests in ein isoliertes Testnetz schicken
- BLE-Geräte scannen und ansprechen
- GPIOs setzen
- OpenOCD-/GDB-Debug starten
- Firmware hochladen und verwalten

`pytest` ruft diese Funktionen über einen Driver an und prüft anschließend die erwarteten Ergebnisse.

---

## Wie die Architektur in der Praxis aussieht

### 1. Raspberry Pi mit Universal-ESP32-Workbench

Der Pi hostet die Workbench und steuert:

- USB-/Slot-Zuordnung
- RFC2217-Serial-Zugriff
- Reset- und Recovery-Mechanismen
- OpenOCD/GDB
- WiFi-Testmodi
- BLE-Zugriff
- GPIO
- Firmware-Repository
- UI und Teststatus

### 2. pytest auf deinem Entwicklungsrechner oder im gleichen Netz

`pytest` spricht die Workbench über deren Base-URL an, zum Beispiel:

```text
http://workbench.local:8080
```

oder

```text
http://<raspberry-pi-ip>:8080
```

### 3. ESP-Board als DUT

Das ESP-Programm läuft auf dem Testgerät.  
Je nach Testtiefe kann es:

- nur Boot-Logs liefern,
- einen HTTP-Server bereitstellen,
- WiFi-Provisioning unterstützen,
- OTA-Updates akzeptieren,
- BLE-Services anbieten.

---

## Was im Repository bereits vorhanden ist

Das Repository bringt bereits eine `pytest`-Struktur mit.  
Dadurch musst du nicht alles selbst neu entwerfen.

Wichtige Bausteine:

- `pytest/conftest.py`
- `pytest/workbench_driver.py`
- `pytest/workbench_test.py`

Diese Dateien zeigen bereits das vorgesehene Nutzungsmodell:

- zentrale `workbench`-Fixture
- Marker für DUT-Tests
- WiFi-Hilfs-Fixtures
- API-gekapselter Driver
- komplette Beispieltests für echte Hardware

---

## Empfohlener Einsatz mit pytest

Am sinnvollsten gehst du in drei Stufen vor.

### Stufe A: Flash + Reset + Serial prüfen

Das ist der beste Einstieg.

Ziel:

- Firmware flashen
- Gerät resetten
- per serieller Ausgabe bestätigen, dass das System korrekt startet

Vorteile:

- schnell
- robust
- unabhängig von höheren Protokollen
- ideal für CI-nahe Smoke-Tests

Typische Assertions:

- ein definierter Boot-Logmarker erscheint
- keine Panic-/Guru-Meldung
- Initialisierung wurde abgeschlossen

### Stufe B: WiFi- und HTTP-Funktion testen

Sobald dein ESP einen Netzwerk-Stack oder HTTP-Server hat, kannst du:

- AP-Modus auf der Workbench starten
- das DUT damit verbinden
- HTTP-Requests über die Workbench weiterleiten
- Status- oder API-Endpunkte des DUT prüfen

Das eignet sich für:

- Health-Endpunkte
- Konfigurations-APIs
- Provisioning-Flows
- OTA-Trigger

### Stufe C: BLE, OTA und Debug erweitern

Wenn dein Projekt diese Funktionen nutzt, kann pytest über die Workbench zusätzlich:

- BLE-Geräte scannen
- GATT-Characteristics beschreiben
- OTA-Abläufe validieren
- Debug-Sessions starten
- OpenOCD/GDB-Verfügbarkeit prüfen

---

## Schritt-für-Schritt-Anleitung

## 1. Workbench auf dem Raspberry Pi installieren

Klonen und Installation:

```bash
git clone https://github.com/SensorsIot/Universal-ESP32-Workbench.git
cd Universal-ESP32-Workbench/pi
sudo bash install.sh
```

Danach prüfen, ob der HTTP-Server erreichbar ist:

```bash
curl http://localhost:8080/api/info
curl http://localhost:8080/api/devices
```

Wenn das funktioniert, läuft die Workbench-Grundinstallation.

---

## 2. pytest-Projekt anlegen

Lege lokal eine Python-Umgebung an:

```bash
python -m venv .venv
source .venv/bin/activate
pip install pytest
```

Unter Windows:

```powershell
python -m venv .venv
.venv\Scripts\activate
pip install pytest
```

Empfohlene Projektstruktur:

```text
my-esp-tests/
├── conftest.py
├── workbench_driver.py
├── tests/
│   ├── test_boot.py
│   ├── test_wifi.py
│   ├── test_ble.py
│   └── test_ota.py
```

Du kannst `conftest.py` und `workbench_driver.py` als Ausgangspunkt direkt aus dem Repository übernehmen.

---

## 3. Grundlegende pytest-Konfiguration

Ein minimales `conftest.py`:

```python
import os
import pytest
from workbench_driver import WorkbenchDriver

def pytest_addoption(parser):
    parser.addoption(
        "--wt-url",
        default=os.environ.get("WORKBENCH_URL", "http://localhost:8080"),
    )
    parser.addoption("--run-dut", action="store_true", default=False)

def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "requires_dut: test needs a connected ESP device"
    )

def pytest_collection_modifyitems(config, items):
    if config.getoption("--run-dut"):
        return
    skip_dut = pytest.mark.skip(reason="Requires DUT (use --run-dut)")
    for item in items:
        if "requires_dut" in item.keywords:
            item.add_marker(skip_dut)

@pytest.fixture(scope="session")
def workbench(request):
    wb = WorkbenchDriver(request.config.getoption("--wt-url"))
    wb.open()
    yield wb
    wb.close()
```

### Wozu diese Struktur dient

- `--wt-url` definiert die URL der Workbench
- `--run-dut` aktiviert echte Hardwaretests
- `requires_dut` trennt Infrastrukturtests von DUT-Tests
- `workbench` stellt eine zentrale Verbindung zur Workbench her

Damit können Tests lokal oder in CI kontrolliert ausgeführt werden.

---

## 4. Der typische Testfluss

Ein typischer Hardwaretest sieht so aus:

1. Workbench-Verbindung öffnen
2. Slot/Gerät erkennen
3. optional Firmware flashen
4. Gerät resetten
5. Logs oder API-Antworten lesen
6. Assertion durchführen
7. Ressourcen sauber freigeben

---

## 5. Flash + Boot + Serial testen

Das ist der wichtigste Startpunkt.

### Beispiel-Helfer zum Flashen

```python
import subprocess

def flash_with_esptool(workbench, slot, chip, build_dir):
    slot_info = workbench.get_slot(slot)
    port = slot_info["url"]

    bootloader_offset = "0x1000" if chip == "esp32" else "0x0000"

    cmd = [
        "python3", "-m", "esptool",
        "--chip", chip,
        "--port", port,
        "--before", "default-reset",
        "--after", "no-reset",
        "write-flash",
        bootloader_offset, f"{build_dir}/bootloader.bin",
        "0x8000", f"{build_dir}/partition-table.bin",
        "0x10000", f"{build_dir}/myapp.bin",
    ]
    subprocess.run(cmd, check=True)
    workbench.serial_reset(slot)
```

### Beispieltest für Boot-Success

```python
import pytest
from helpers import flash_with_esptool

@pytest.mark.requires_dut
def test_boot_marker_after_flash(workbench):
    slot = "SLOT1"
    chip = "esp32s3"
    build_dir = "./build/export"

    flash_with_esptool(workbench, slot, chip, build_dir)

    result = workbench.serial_monitor(
        slot=slot,
        pattern="Init complete",
        timeout=20,
    )
    assert result["matched"], result.get("output", [])
```

### Warum dieser Test so wichtig ist

Dieser Test prüft sofort:

- Build-Artefakte sind korrekt
- Flashing funktioniert
- Reset funktioniert
- das Gerät bootet stabil
- die Firmware erreicht den gewünschten Initialisierungszustand

Für viele Projekte ist das bereits der wertvollste automatische Smoke-Test.

---

## 6. Gerätepräsenz und Slot-Erkennung prüfen

Ein einfacher Infrastrukturtest:

```python
def test_device_present(workbench):
    slots = workbench.get_devices()
    assert any(slot.get("present") for slot in slots)
```

Das ist nützlich, um vor DUT-Tests sicherzustellen, dass die Hardware überhaupt korrekt erkannt wurde.

---

## 7. WiFi-Tests mit pytest und Workbench

Sobald die Firmware netzwerkfähig ist, kannst du WiFi-Tests aufbauen.

Typischer Ablauf:

1. Workbench startet einen AP
2. DUT verbindet sich mit dem Testnetz
3. Workbench erkennt das DUT im Netz
4. pytest spricht das DUT per HTTP an
5. Antwort wird geprüft

### Beispieltest für einen Status-Endpunkt

```python
import pytest

@pytest.mark.requires_dut
def test_device_status_over_workbench_ap(workbench, wifi_network):
    station = workbench.wait_for_station(timeout=60)
    resp = workbench.http_get(f"http://{station['ip']}/status")

    assert resp.status_code == 200
    data = resp.json()
    assert "version" in data
```

### Gute Einsatzfälle

- `/status`
- `/health`
- Konfigurations-Endpunkte
- Provisioning-Endpunkte
- Diagnosefunktionen

---

## 8. OTA-Tests

Wenn dein DUT OTA unterstützt, kann pytest den gesamten Ablauf testen:

1. Firmware zur Workbench hochladen
2. DUT im Testnetz erreichbar machen
3. OTA-Endpunkt des DUT triggern
4. Reboot abwarten
5. neue Version verifizieren

### Empfohlenes Prüfmuster

- Upload erfolgreich
- OTA-Trigger erfolgreich
- DUT rebootet
- neue Versionsnummer ist sichtbar
- Device ist wieder funktionsfähig

---

## 9. BLE-Tests

Wenn dein Gerät BLE nutzt, kann die Workbench als BLE-Testadapter fungieren.

Mögliche Testfälle:

- Gerät im Scan sichtbar
- Geräteadresse korrekt
- Verbindung möglich
- GATT-Service vorhanden
- Characteristic kann gelesen/geschrieben werden
- Reaktion auf Schreibvorgänge korrekt

### Beispieltest

```python
import pytest

@pytest.mark.requires_dut
def test_ble_command_channel(workbench):
    devices = workbench.ble_scan(timeout=5, name_filter="MyESP")
    assert devices, "BLE device not found"

    dev = devices[0]
    workbench.ble_connect(dev["address"])
    try:
        workbench.ble_write(
            characteristic="6e400002-b5a3-f393-e0a9-e50e24dcca9e",
            data="0248454c4c4f",
            response=False,
        )
    finally:
        workbench.ble_disconnect()
```

---

## 10. Human-in-the-loop-Tests

Nicht alle Hardwaretests sind vollständig automatisierbar.  
Wenn ein Test einen manuellen Eingriff erfordert, kann die Workbench das sauber abbilden.

Typische Beispiele:

- Knopf drücken
- Jumper umsetzen
- Kabel umstecken
- Gerät bewusst in einen bestimmten Modus versetzen

### Beispiel

```python
import pytest

@pytest.mark.requires_dut
def test_press_button_to_continue(workbench):
    confirmed = workbench.human_interaction(
        "Please press BOOT on SLOT1, then click Done",
        timeout=60,
    )
    assert confirmed
```

Das ist deutlich sauberer als ein bloßes `sleep()` und erhöht die Bedienbarkeit komplexer Hardwaretests.

---

## 11. Testfortschritt in der Workbench anzeigen

Die Workbench kann Testfortschritte für UI oder Bedieneroberfläche anzeigen.

Das ist nützlich für:

- lange Testläufe
- manuell überwachte Abläufe
- Laborumgebungen
- Mehrgeräte-Setups

### Beispiel

```python
import pytest

@pytest.mark.requires_dut
def test_with_progress(workbench):
    workbench.test_start(spec="My Firmware", phase="Boot", total=1)
    try:
        workbench.test_step("TC001", "Boot smoke test", "Reset and watch serial")
        result = workbench.serial_monitor("SLOT1", pattern="Init complete", timeout=20)
        assert result["matched"]
        workbench.test_result("TC001", "Boot smoke test", "PASS")
    except Exception as e:
        workbench.test_result("TC001", "Boot smoke test", "FAIL", details=str(e))
        raise
    finally:
        workbench.test_end()
```

---

## 12. Empfohlene Teststrategie

Die beste Reihenfolge für den Einstieg ist:

### Phase 1: Infrastruktur stabilisieren

Zuerst nur prüfen:

- Workbench erreichbar
- Slots sichtbar
- DUT erkannt
- Serial-Zugriff funktioniert

### Phase 2: Flash- und Boot-Smoke-Tests

Dann:

- Firmware flashen
- Reset auslösen
- Boot-Log prüfen

### Phase 3: Funktionsnahe API-Tests

Danach:

- HTTP-Endpunkte
- WiFi-Flows
- Provisioning
- OTA

### Phase 4: Erweiterte Hardwaretests

Zum Schluss:

- BLE
- Debug/OpenOCD
- GPIO-Steuerung
- manuelle Interaktionstests

### Warum diese Reihenfolge sinnvoll ist

Weil sich Fehler so sauber eingrenzen lassen:

- Wenn Phase 1 scheitert, liegt es an der Infrastruktur
- Wenn Phase 2 scheitert, liegt es oft am Build, Flashing oder Boot
- Wenn Phase 3 scheitert, liegt es meist an der Firmware-Logik
- Wenn Phase 4 scheitert, betrifft es spezialisierte Hardwarepfade

---

## 13. Gute Designregeln für ESP-Firmware im Testbetrieb

Damit pytest mit der Workbench effektiv arbeiten kann, sollte deine Firmware einige Dinge bewusst bereitstellen.

### Empfehlenswert sind

- ein klarer Boot-Marker im Log
- ein stabiler Health- oder Status-Endpunkt
- eine klare Versionsausgabe
- reproduzierbare WiFi-Initialisierung
- definierte OTA-Aktivierung
- definierte BLE-Services/Characteristics
- möglichst deterministische Initialisierung

### Warum das wichtig ist

Tests werden deutlich stabiler, wenn sie auf klaren Beobachtungspunkten beruhen.  
Ein guter Logmarker ist oft wertvoller als schwer reproduzierbare Timing- oder Netzbedingungen.

---

## 14. Typische Startkommandos

Nur Infrastrukturtests:

```bash
pytest -q --wt-url http://workbench.local:8080
```

Mit echter Hardware:

```bash
pytest -q --wt-url http://workbench.local:8080 --run-dut
```

Gezielt nur eine Teilmenge:

```bash
pytest -q tests/test_boot.py --wt-url http://workbench.local:8080 --run-dut
```

---

## 15. Praktische Empfehlung für eigene Projekte

Für ein neues ESP-Projekt solltest du nicht sofort mit komplexen OTA- oder BLE-End-to-End-Tests anfangen.  
Robuster ist dieses Vorgehen:

1. zuerst **Boot-Test**
2. dann **Serial-Smoke-Test**
3. dann **Status-/HTTP-Test**
4. dann **WiFi-Provisioning**
5. dann **OTA**
6. dann **BLE**
7. zuletzt **Debug/JTAG**

So bekommst du schnell belastbare Ergebnisse und vermeidest instabile Testszenarien in der frühen Phase.

---

## 16. Fazit

Die Universal-ESP32-Workbench eignet sich sehr gut als Hardware-Testplattform für `pytest`.

Der sinnvollste Ansatz ist:

- **pytest** steuert die Tests,
- **Workbench** übernimmt Hardwarezugriff und Infrastruktur,
- **deine ESP-Firmware** liefert beobachtbare Zustände und Funktionen.

Mit diesem Aufbau kannst du aus einfachen Smoke-Tests schrittweise eine vollständige Hardware-Testautomatisierung entwickeln.

Besonders stark ist die Kombination bei:

- Flash-/Boot-Validierung
- Serial-Prüfungen
- WiFi-/HTTP-Tests
- OTA-Tests
- BLE-Tests
- manuell unterstützten Hardwareabläufen

Wenn du dich an die vorhandenen `pytest`-Bausteine des Repositories anlehnst, kommst du sehr schnell zu einem stabilen und wartbaren Testsystem.

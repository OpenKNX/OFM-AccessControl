### NFC Scanner

Auswahl der angeschlossenen NFC-Scanner-Hardware. Es wird die folgendes angeboten:

* **Kein NFC** (Standard): Wenn keine NFC-Scanner-Hardware angeschlossen ist
* **Schaltereinsatz mit NFC**: Die Touch-Frontplatine ist mit der NFC-Scanner-Hardware ausgerüstet, zu erkennen an der NFC-Antenne
* **Externer NFC-Leser**: Eine NFC-Scanner-Hardware ist über ein Kabel mit der Fingerprint-Platine verbunden



Die Auswahl beschreibt die Anschlussart, nicht den NFC-Chip. Die Firmware legt den Reader-Treiber und die I2C-Pins fest. In der klassischen PN7160-Firmware verwenden sowohl **Schaltereinsatz mit NFC** als auch **Externer NFC-Leser** denselben PN7160-Treiber. Die Auswahl unterscheidet die Touch-/LED-Anbindung; bei externer Anbindung werden zusätzlich Expander-Ausgänge initialisiert.

In den REG1-/XIAO-Profilen für PN7161 beziehungsweise ST25R3916 wird der passende Reader durch das Build-Profil ausgewählt. Für einen über Kabel angeschlossenen Reader ist **Externer NFC-Leser** vorgesehen. **Kein NFC** schaltet die NFC-Auswertung ab. Die Auswahl allein startet kein Apple Home Key: Dafür ist die gesonderte HomeKey-Freigabe erforderlich. Ohne HomeKey können gewöhnliche, gespeicherte UID-Tags verwendet werden.

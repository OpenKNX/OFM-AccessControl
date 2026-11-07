### NFC

Diese Seite wird angezeigt, wenn ein NFC-Scanner ausgewählt ist.

Im oberen Bereich können gespeicherte NFC-Tags nach Bezeichnung oder Tag-ID gesucht werden. In Anwendungen mit gemeinsamer HomeKey-Verwaltung werden zusätzlich die Geräte aus dem aktuellen ETS-Projekt berücksichtigt. Für Online-Abfragen muss das Gerät über die ETS-KNX-Schnittstelle erreichbar sein.

Im unteren Bereich werden Zugänge ihren Aktionen zugeordnet. Die Spalte **Tag** enthält die NFC-Tag-ID, die Spalte **Aktion** die Nummer der auszuführenden Aktion. Mehrere Aktionen für dieselbe Tag-ID sind möglich. Änderungen werden mit dem Parameterdownload wirksam.

**Info holen** liest Informationen zur ausgewählten Tag-ID ein. Bei HomeKey wird die Bezeichnung aus dem ETS-Projekt verwendet und online geprüft, ob die Geräte-ID im Leser provisioniert ist. Der Leser scannt dabei keinen Tag und erteilt keine Berechtigung.

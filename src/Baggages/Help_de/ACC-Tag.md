### Tag / NFC-Tag-ID

ID des angelernten UID-Tags oder die NFC-Tag-ID einer aktiven Home-Key-Berechtigung (0–1499). Die Spalte **Aktion** bestimmt den Aktionskanal, der nach erfolgreicher Authentifizierung ausgeführt wird.

Beispiel: Home Key mit NFC-Tag-ID **100** benötigt hier **Tag 100 → Aktion 1**. Aktion 1 muss konfiguriert und ihr Schaltobjekt mit der Gruppenadresse des gewünschten KNX-Aktors verbunden sein. Parameter anschließend herunterladen.

Home Key und normale UID-Tags benötigen getrennte IDs. Verschiedene IDs dürfen dieselbe Aktion auslösen. Eine Tag-ID darf in mehreren Zuordnungszeilen vorkommen, um mehrere Aktionen auszulösen.

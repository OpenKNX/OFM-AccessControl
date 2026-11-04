### Information

Freies Beschreibungsfeld für die Zuordnung. **Info holen** erzeugt einen Vorschlag und überschreibt dieses Feld.

Für normale NFC-Tags liest der Button den gespeicherten UID-Tag-Namen über die ETS-Onlineverbindung. Für Home Key zeigt er die Bezeichnung der aktiven Berechtigung aus dem aktuellen ETS-Projekt und prüft online, ob deren Geräte-ID im Reader provisioniert ist. Doppelte Home-Key-IDs und Kollisionen mit gespeicherten UID-Tags werden als Konflikt gemeldet.

Die Anzeige ist auf 80 Zeichen begrenzt; der vollständige Text steht im Fortschrittsdialog. Die Bezeichnung und Zuordnung stammen aus dem ETS-Projekt. Sie werden erst nach dem Download wirksam. Lokale USB-Home-Key-Zuordnungen werden hier nicht aufgelistet.

Der Button scannt keinen Tag, lernt nichts an, verändert keine Berechtigung und löst keine Aktion aus. Auch bei einem Lesefehler wird die Verbindung geschlossen und ein Fehler angezeigt, statt eine alte erfolgreiche Anzeige stehen zu lassen.

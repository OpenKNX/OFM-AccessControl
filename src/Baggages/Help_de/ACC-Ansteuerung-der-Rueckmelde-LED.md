### Rückmelde-LED

Erscheint nur bei Keypads mit Rückmelde-LED.

Die Rückmelde-LED zeigt - ähnlich wie der Fingerprint - verschiedene Zustände während der Codeeingabe an.

Die Codeerkennung beginnt mit dem ersten Tastendruck, außer man hat eingestellt, dass die erste Taste ignoriert werden soll.

Bei jedem erkannten Tastendruck blinkt die LED kurz blau.
Wurde ein korrekter Code erkannt und eine entsprechende Aktion ausgelöst, leuchtet die LED kurz grün.
Wurde ein korrekter Code erkannt und es ist keine Aktion zugeordnet, leuchtet die LED kurz gelb.
Wurde ein Code nicht erkannt, leuchtet die LED kurz rot.
Ein Code ist dann nicht erkannt, wenn

* die maximale Anzahl von 10 Zeichen eingegeben wurde und diese keinem Code entsprechen
* ein Abschlusszeichen festgelegt und eingegeben wurde und der Code falsch ist
* für 15 Sekunden kein Zeichen eingegeben wurde

Zusätzlich zu den internen Zuständen kann die Rückmelde-LED auch über ein KO vom KNX-Bus gesteuert werden, um z.B. die erfolgreiche Ausführung einer Aktion anzuzeigen.

##### Ansteuerung der Rückmelde-LED

Hier kann man einstellen, wie die Rückmelde-LED angesteuert wird.

* **immer aus** - die Rückmelde-LED wird nicht angesteuert (nicht empfohlen)
* **nur intern** - die Rückmelde-LED stellt nur die beschriebenen internen Zustände dar
* **über KO** - die Rückmelde-LED stellt das dar, was über Bustelegramme an ihr KO übermittelt wird
* **intern und KO** - Es werden interne Zustände bei der Tasteneingabe und Farben über KO dargestellt


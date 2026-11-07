### Rückmelde-Buzzer

Erscheint nur bei Keypads mit Rückmelde-Buzzer.

Der Rückmelde-Buzzer erlaubt die Rückmeldung verschiedener Zustände während der Codeeingabe mit Hilfe von Tönen.

Die Codeerkennung beginnt mit dem ersten Tastendruck, außer man hat eingestellt, dass die erste Taste ignoriert werden soll.

Bei jedem erkannten Tastendruck wird ein kurzer Piepton generiert.
Wurde ein korrekter Code erkannt und eine entsprechende Aktion ausgelöst, werden 2 kurze Pieptöne abgespielt.
Wurde ein korrekter Code erkannt und es ist keine Aktion zugeordnet, werden 2 kurze, gefolgt von einem langen Piepton abgespielt.
Wurde ein Code nicht erkannt, wird ein langer Piepton abgespielt.
Ein Code ist dann nicht erkannt, wenn

* die maximale Anzahl von 10 Zeichen eingegeben wurde und diese keinem Code entsprechen
* ein Abschlusszeichen festgelegt und eingegeben wurde und der Code falsch ist
* für die festgelegte Zeit kein Zeichen eingegeben wurde

Zusätzlich zu den internen Zuständen kann der Buzzer auch über ein KO vom KNX-Bus gesteuert werden, um z.B. die erfolgreiche Ausführung einer Aktion anzuzeigen.

##### Ansteuerung des Rückmelde-Buzzers

Hier kann man einstellen, wie der Rückmelde-Buzzer angesteuert wird.

* **immer aus** - der Rückmelde-Buzzer wird nicht angesteuert (nicht empfohlen)
* **nur intern** - der Rückmelde-Buzzer stellt nur die beschriebenen internen Zustände dar
* **über KO** - der Rückmelde-Buzzer stellt das dar, was über Bustelegramme an sein KO übermittelt wird
* **intern und KO** - Es werden interne Zustände bei der Tasteneingabe und Signale über KO abgespielt



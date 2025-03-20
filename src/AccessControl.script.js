
function ACC_clearSearchResults(input, output, context) {
    output.SearchResults = 0;
    output.SearchResultsDisplay = 0;
}

function ACC_dummy(input, output, context) { }

function ACC_sort(device, online, progress, context) {
    var parFingerActionCount = device.getParameterByName("FINACT_FingerActionCount");
    // first get rid of zero lines
    var endIndex = parFingerActionCount.value;
    var startIndex = 0;
    while (endIndex - 1 > startIndex + 1) {
        do {
            endIndex--;
            var parEndAction = device.getParameterByName("FINACT_Fa" + endIndex + "ActionId");
        } while (parEndAction.value == 0);
        do {
            startIndex++;
            var parStartAction = device.getParameterByName("FINACT_Fa" + startIndex + "ActionId");
        } while (parStartAction.value != 0);
        // now startIndex points to a zero entry and endIndex to a non-zero entry, we move end to start
        if (endIndex > startIndex) {
            FingerActionInfo
            var parStartFinger = device.getParameterByName("FINACT_Fa" + startIndex + "FingerId");
            var parEndFinger = device.getParameterByName("FINACT_Fa" + endIndex + "FingerId");
            var parStartInfo = device.getParameterByName("FINACT_Fa" + startIndex + "FingerActionInfo");
            var parEndInfo = device.getParameterByName("FINACT_Fa" + endIndex + "FingerActionInfo");
            parStartAction.value = parEndAction.value;
            parStartFinger.value = parEndFinger.value;
            parStartInfo.value = parEndInfo.value;
            parEndAction.value = 0;
            parEndFinger.value = 0;
            parEndInfo.value = "";
        }
    }
    // now do bubble sort
    do {
        var continueSort = false;
        for (var current = 1; current < parFingerActionCount.value; current++) {
            var parCurrAction = device.getParameterByName("FINACT_Fa" + current + "ActionId");
            var parNextAction = device.getParameterByName("FINACT_Fa" + (current + 1) + "ActionId");
            var parCurrFinger = device.getParameterByName("FINACT_Fa" + current + "FingerId");
            var parNextFinger = device.getParameterByName("FINACT_Fa" + (current + 1) + "FingerId");
            if (parNextAction.value == 0) { break; }
            var swap = (parCurrAction.value > parNextAction.value || (parCurrAction.value == parNextAction.value && parCurrFinger.value > parNextFinger.value));
            if (swap) {
                continueSort = true;
                var parCurrInfo = device.getParameterByName("FINACT_Fa" + current + "FingerActionInfo");
                var parNextInfo = device.getParameterByName("FINACT_Fa" + (current + 1) + "FingerActionInfo");
                var tmpAction = parCurrAction.value;
                var tmpFinger = parCurrFinger.value;
                var tmpInfo = parCurrInfo.value;
                parCurrAction.value = parNextAction.value;
                parCurrFinger.value = parNextFinger.value;
                parCurrInfo.value = parNextInfo.value;
                parNextAction.value = tmpAction;
                parNextFinger.value = tmpFinger;
                parNextInfo.value = tmpInfo;
            }
        }
    } while (continueSort);
}

function ACC_assignFingerId(device, online, progress, context) {
    var parFingerId = device.getParameterByName("FINACT_FingerId");
    var parFingerActionLine = device.getParameterByName("FINACT_FingerActionLine");
    var parTargetId = device.getParameterByName("FINACT_Fa" + parFingerActionLine.value + "FingerId");
    parTargetId.value = parFingerId.value;
}

function ACC_checkFingerIdRange(input, changed, prevValue, context) {
    var limit = [149, 199, 1499];
    if (input.FingerID > limit[input.Scanner]) {
        if (changed == "FingerID") {
            return "FingerId ist " + input.FingerID + ", aber der Fingerscanner kann nur " + limit[input.Scanner] + " Finger verwalten.";
        } else {
            return "Auf Finger-Seite gibt es FingerIds > " + limit[input.Scanner] + ". Hardware-Scanner kann erst geändert werden, wenn es keine ungültigen FingerIds für die neue Hardwareauswahl mehr gibt.";
        }
    }
    return true;
}

function ACC_checkFingerAction(device, online, progress, context) {
    var parActionId = device.getParameterByName("FINACT_Fa" + context.Channel + "ActionId");
    var parFingerId = device.getParameterByName("FINACT_Fa" + context.Channel + "FingerId");
    var parFingerActionInfo = device.getParameterByName("FINACT_Fa" + context.Channel + "FingerActionInfo");
    var parVisibleActions = device.getParameterByName("ACC_VisibleActions");

    if (parActionId.value <= parVisibleActions.value) {

        progress.setText("Fingerprint: Person zu Finger ID " + parFingerId.value + " suchen...");
        online.connect();
    
        var data = [11]; // internal function ID
        data = data.concat((parFingerId.value & 0x0000ff00) >> 8, (parFingerId.value & 0x000000ff));

        var personFinger = 0;
        var personName = "";

        var resp = online.invokeFunctionProperty(160, 3, data);
        if (resp[0] != 0) {
            progress.setText("Fingerprint: Person zu Finger ID " + parFingerId.value + " nicht gefunden.");
            online.disconnect();
            return;
        } else {
            online.disconnect();
            progress.setText("Fingerprint: Person zu Finger ID " + parFingerId.value + " gefunden.");
        
            personFinger = resp[1];
            for (var i = 2; i < resp.length; ++i) {
                if (resp[i] == 0)
                    break; // null-termination
            
                personName += String.fromCharCode(resp[i]);
            }
        }

        var personText = "";
        if (personFinger > 0) {
            var personFingerName = "";
            switch (personFinger) {
                case 1:
                    personFingerName = "Daumen rechts";
                    break;
                case 2:
                    personFingerName = "Daumen links";
                    break;
                case 3:
                    personFingerName = "Zeigefinger rechts";
                    break;
                case 4:
                    personFingerName = "Zeigefinger links";
                    break;
                case 5:
                    personFingerName = "Mittelfinger rechts";
                    break;
                case 6:
                    personFingerName = "Mittelfinger links";
                    break;
                case 7:
                    personFingerName = "Ringfinger rechts";
                    break;
                case 8:
                    personFingerName = "Ringfinger links";
                    break;
                case 9:
                    personFingerName = "Kleiner Finger rechts";
                    break;
                case 10:
                    personFingerName = "Kleiner Finger links";
                    break;
            }

            personText = personName + "; " + personFingerName;
        } else {
            personText = "Unbekannter Finger";
        }

        var parActionDescription = device.getParameterByName("ACC_Act" + parActionId.value + "Description");
        parFingerActionInfo.value = (parActionDescription.value + "; " + personText).substring(0, 80);
    } else {
        parFingerActionInfo.value = "Aktion ist nicht definiert, Finger wurde nicht ermittelt";
    }
}

function ACC_searchFingerId(device, online, progress, context) {
    var parPersonName = device.getParameterByName("FINACT_PersonName");
    var parPersonFinger = device.getParameterByName("FINACT_PersonFinger");
    var parFingerId = device.getParameterByName("FINACT_FingerId");
    var parNumberSearchResults = device.getParameterByName("FINACT_NumberSearchResults");
    var parNumberSearchResultsText = device.getParameterByName("FINACT_NumberSearchResultsText");
    var parNumberSearchResultsToDisplay = device.getParameterByName("FINACT_NumberSearchResultsToDisplay");

    parNumberSearchResults.value = 0;
    parNumberSearchResultsToDisplay.value = 0;

    progress.setText("Fingerprint: Finger ID zu Person " + parPersonName.value + " (" + parPersonFinger.value + ") suchen...");
    online.connect();

    var data = [12]; // internal function ID

    // person finger
    data = data.concat((parPersonFinger.value & 0x000000ff));

    // person name
    for (var i = 0; i < parPersonName.value.length; ++i) {
        var code = parPersonName.value.charCodeAt(i);
        data = data.concat([code]);
    }
    data = data.concat(0); // null-terminated string

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            progress.setText("Fingerprint: Finger ID zu Person " + parPersonName.value + " (" + parPersonFinger.value + ") nicht gefunden.");
            online.disconnect();
            return;
        } else {
            throw new Error("Fingerprint: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();

    var numRes = (resp.length - 3) / 31;
    var totalMatches = resp[1] << 8 | resp[2];
    // info("totalMatches " + totalMatches);
    progress.setText("Fingerprint: " + totalMatches + " Finger ID(s) zu Person " + parPersonName.value + " (" + parPersonFinger.value + ") gefunden.");

    // var fingerId = resp[1] << 8 | resp[2];
    // var personFinger = resp[3];
    // var personName = "";
    // for (var i = 4; i < 32; ++i) {
    //     if (resp[i] == 0)
    //         break; // null-termination

    //     personName += String.fromCharCode(resp[i]);
    // }

    // parPersonName.value = personName;
    // parPersonFinger.value = personFinger;
    // parFingerId.value = fingerId;


    // following up to 10 results in total
    // always 2 bytes fingerId, 1 byte personFinger and 28 bytes personName
    // info("Bevor: " + parNumberSearchResults.value);
    parNumberSearchResultsText.value = totalMatches;
    parNumberSearchResultsToDisplay.value = numRes;
    if (totalMatches > numRes) {
        parNumberSearchResults.value = 1;
    }

    // info("Danach: " + parNumberSearchResults.value);
    for (var row = 1; row <= numRes; row++) {
        // info("FINACT_Person" + row + "Name");
        parPersonName = device.getParameterByName("FINACTSER_Person" + row + "Name");
        parPersonFinger = device.getParameterByName("FINACTSER_Person" + row + "Finger");
        parFingerId = device.getParameterByName("FINACTSER_Finger" + row + "Id");

        var res = (row - 1) * 31 + 3;
        // info("res " + row + ": " + res);
        var fingerId = resp[res + 0] << 8 | resp[res + 1];
        // info("fingerid: " + fingerId);
        var personFinger = resp[res + 2];
        // info("finger: " + personFinger);
        var personName = "";
        for (var i = res + 3; i < res + 31; ++i) {
            if (resp[i] == 0)
                break; // null-termination

            personName += String.fromCharCode(resp[i]);
        }
        // info("personName: " + personName);

        parPersonName.value = personName;
        parPersonFinger.value = personFinger;
        parFingerId.value = fingerId;
    }
}    

function ACC_searchFingerName(device, online, progress, context) {
    var parPersonName = device.getParameterByName("FINACT_PersonName");
    var parPersonFinger = device.getParameterByName("FINACT_PersonFinger");
    var parFingerId = device.getParameterByName("FINACT_FingerId");
    var parNumberSearchResults = device.getParameterByName("FINACT_NumberSearchResults");

    parNumberSearchResults.value = 0;
    var fingerId = parFingerId.value;

    progress.setText("Fingerprint: Person zu Finger ID " + fingerId + " suchen...");
    online.connect();

    var data = [11]; // internal function ID
    data = data.concat((fingerId & 0x0000ff00) >> 8, (fingerId & 0x000000ff));

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            progress.setText("Fingerprint: Person zu Finger ID " + fingerId + " nicht gefunden.");
            online.disconnect();
            return;
        } else {
            throw new Error("Fingerprint: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("Fingerprint: Person zu Finger ID " + fingerId + " gefunden.");

    var personFinger = resp[1];
    var personName = "";
    for (var i = 2; i < resp.length; ++i) {
        if (resp[i] == 0)
            break; // null-termination
      
        personName += String.fromCharCode(resp[i]);
    }

    parNumberSearchResults.value = 1;

    parPersonName = device.getParameterByName("FINACTSER_Person1Name");
    parPersonFinger = device.getParameterByName("FINACTSER_Person1Finger");
    parFingerId = device.getParameterByName("FINACTSER_Finger1Id");
    parPersonName.value = personName;
    parPersonFinger.value = personFinger;
    parFingerId.value = fingerId;
}

function ACC_sleep(milliseconds) {
    var currentTime = new Date().getTime();
    while (currentTime + milliseconds >= new Date().getTime()) {
    }
}

function ACC_enrollFinger(device, online, progress, context) {
    var parFingerId = device.getParameterByName("ACC_EnrollFingerId");
    var parPersonFinger = device.getParameterByName("ACC_EnrollPersonFinger");
    var parPersonName = device.getParameterByName("ACC_EnrollPersonName");

    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " anlernen...");
    online.connect();

    var data = [1]; // internal function ID

    // finger ID
    data = data.concat((parFingerId.value & 0x0000ff00) >> 8, (parFingerId.value & 0x000000ff));

    // person finger
    data = data.concat((parPersonFinger.value & 0x000000ff));

    // person name
    for (var i = 0; i < parPersonName.value.length; ++i) {
        var code = parPersonName.value.charCodeAt(i);
        data = data.concat([code]);
    }
    data = data.concat(0); // null-terminated string

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        throw new Error("Fingerprint: Finger anlernen konnte nicht gestartet werden!");
    }


    // we wait until enrol finger is finished
    var lCancelled = false;
    if (!lCancelled) {
        var lText = "Fingerprint: Finger ID " + parFingerId.value + " anlernen gestartet.";
        progress.setText(lText);
        
        var data = [7]; // command wait for enroll request finished
        data = data.concat(0); // zero-terminated
        
        var resp = online.invokeFunctionProperty(160, 3, data);
        
        var lPercent = 0;
        if (resp[0] == 0) {
            // poll to check if enroll is finished
            while (resp[0] == 0 && !lCancelled) {
                var fingerProgress = resp[1];
                switch (fingerProgress) {
                    case 0:
                        break;
                    case 7:
                        progress.setText("Fingerprint: Finger ID " + parFingerId.value + " anlernen: Fingermodell erstellen...");
                        break;
                    case 8:
                        progress.setText("Fingerprint: Finger ID " + parFingerId.value + " anlernen: Fingermodell speichern...");
                        break;
                    default:
                        progress.setText("Fingerprint: Finger ID " + parFingerId.value + " anlernen: Muster " + fingerProgress + " von 6...");
                        break;
                }
                data = [7];
                data = data.concat(0); // zero-terminated string
                lPercent = fingerProgress / 9.0 * 100.0;
                if (lPercent <= 100) progress.setProgress(lPercent);
                ACC_sleep(1000);
                resp = online.invokeFunctionProperty(160, 3, data);
                // lCancelled = progress.isCanceled();
            }
            if (resp[0] == 1 && !lCancelled) {
                if (resp[1] == 0) {
                    throw new Error("Fingerprint: Finger ID " + parFingerId.value + " anlernen fehlgeschlagen!");
                } else {
                    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " anlernen erfolgreich.");
                }
            }
        }
    }
    if (lCancelled) {
        progress.setText("Fingerprint: Anlernen vom Benutzer abgebrochen");
    }
    progress.setProgress(100);
    ACC_sleep(100);
    online.disconnect();
}

function ACC_changeFinger(device, online, progress, context) {
    var parFingerId = device.getParameterByName("ACC_EnrollFingerId");
    var parPersonFinger = device.getParameterByName("ACC_EnrollPersonFinger");
    var parPersonName = device.getParameterByName("ACC_EnrollPersonName");

    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " ändern...");
    online.connect();

    var data = [4]; // internal function ID

    // finger ID
    data = data.concat((parFingerId.value & 0x0000ff00) >> 8, (parFingerId.value & 0x000000ff));

    // person finger
    data = data.concat((parPersonFinger.value & 0x000000ff));

    // person name
    for (var i = 0; i < parPersonName.value.length; ++i) {
        var code = parPersonName.value.charCodeAt(i);
        data = data.concat([code]);
    }
    data = data.concat(0); // null-terminated string

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            throw new Error("Fingerprint: Finger ID " + parFingerId.value + " nicht gefunden!");
        } else {
            throw new Error("Fingerprint: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " geändert.");
}

function ACC_syncFinger(device, online, progress, context) {
    var parFingerId = device.getParameterByName("ACC_SyncFingerId");

    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " synchronisieren...");
    online.connect();

    var data = [2]; // internal function ID
    data = data.concat((parFingerId.value & 0x0000ff00) >> 8, (parFingerId.value & 0x000000ff));

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            throw new Error("Fingerprint: Finger ID " + parFingerId.value + " nicht gefunden!");
        } else {
            throw new Error("Fingerprint: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " Synchronisierung gestartet.");
}

function ACC_deleteFinger(device, online, progress, context) {
    var parFingerId = device.getParameterByName("ACC_DeleteFingerId");

    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " löschen...");
    online.connect();

    var data = [3]; // internal function ID
    data = data.concat((parFingerId.value & 0x0000ff00) >> 8, (parFingerId.value & 0x000000ff));

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            throw new Error("Fingerprint: Finger ID " + parFingerId.value + " nicht gefunden!");
        } else {
            throw new Error("Fingerprint: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("Fingerprint: Finger ID " + parFingerId.value + " gelöscht.");
}

function ACC_resetFingerScanner(device, online, progress, context) {
    progress.setText("Fingerprint: Alle Finger löschen...");
    online.connect();

    var data = [6]; // internal function ID

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        throw new Error("Fingerprint: Es ist ein unbekannter Fehler aufgetreten!");
    }

    online.disconnect();
    progress.setText("Fingerprint: Alle Finger gelöscht.");
}

function ACC_setFingerPassword(device, online, progress, context) {
    var parPasswordOption = device.getParameterByName("ACC_PasswordOption");
    var parPasswordNew = device.getParameterByName("ACC_PasswordNew");
    var parPasswordOld = device.getParameterByName("ACC_PasswordOld");
    
    progress.setText("Fingerprint: " + parPasswordOption.value == 1 ? "Passwort festsetzen..." : "Passwort ändern...");
    online.connect();

    var data = [21]; // internal function ID

    // password option
    data = data.concat((parPasswordOption.value & 0x000000ff));

    // new password
    for (var i = 0; i < parPasswordNew.value.length; ++i) {
        var code = parPasswordNew.value.charCodeAt(i);
        data = data.concat([code]);
    }
    data = data.concat(0); // null-terminated string

    // change password
    if (parPasswordOption.value == 2) {
        // old password
        for (var i = 0; i < parPasswordOld.value.length; ++i) {
            var code = parPasswordOld.value.charCodeAt(i);
            data = data.concat([code]);
        }
        data = data.concat(0); // null-terminated string
    }

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            throw new Error("Fingerprint: Eingegebenes altes Passwort falsch!");
        } else {
            throw new Error("Fingerprint: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    var parPasswordAlreadySet = device.getParameterByName("ACC_PasswardAlreadySet");
    parPasswordAlreadySet.value = parPasswordNew.value == "0" ? 0 : 1;

    online.disconnect();
    progress.setText("Fingerprint: " + parPasswordOption.value == 1 ? "Passwort festgesetzt." : "Passwort geändert.");
}

function ACC_checkNfcAction(device, online, progress, context) {
    var parActionId = device.getParameterByName("NFCACT_Fa" + context.Channel + "ActionId");
    var parNfcId = device.getParameterByName("NFCACT_Fa" + context.Channel + "NfcId");
    var parNfcActionInfo = device.getParameterByName("NFCACT_Fa" + context.Channel + "NfcActionInfo");
    var parVisibleActions = device.getParameterByName("ACC_VisibleActions");

    if (parActionId.value <= parVisibleActions.value) {

        progress.setText("NFC: Tag zu NFC ID " + parNfcId.value + " suchen...");
        online.connect();
    
        var data = [111]; // internal function ID
        data = data.concat((parNfcId.value & 0x0000ff00) >> 8, (parNfcId.value & 0x000000ff));

        var nfcName = "";

        var resp = online.invokeFunctionProperty(160, 3, data);
        if (resp[0] != 0) {
            progress.setText("NFC: Tag zu NFC ID " + parNfcId.value + " nicht gefunden.");
            online.disconnect();
            return;
        } else {
            online.disconnect();
            progress.setText("NFC: Tag zu NFC ID " + parNfcId.value + " gefunden.");
        
            personNfc = resp[1];
            for (var i = 1; i < resp.length; ++i) {
                if (resp[i] == 0)
                    break; // null-termination
            
                nfcName += String.fromCharCode(resp[i]);
            }
        }

        var parActionDescription = device.getParameterByName("ACC_Act" + parActionId.value + "Description");
        parNfcActionInfo.value = (parActionDescription.value + "; " + nfcName).substring(0, 80);
    } else {
        parNfcActionInfo.value = "Aktion ist nicht definiert, NFC wurde nicht ermittelt";
    }
}

function ACC_searchNfcId(device, online, progress, context) {
    var parNfcName = device.getParameterByName("NFCACT_NfcName");
    //var parNfcTagUid = device.getParameterByName("NFCACT_NfcTagUid");
    var parNfcId = device.getParameterByName("NFCACT_NfcId");
    var parNumberSearchResults = device.getParameterByName("NFCACT_NumberSearchResults");
    var parNumberSearchResultsText = device.getParameterByName("NFCACT_NumberSearchResultsText");
    var parNumberSearchResultsToDisplay = device.getParameterByName("NFCACT_NumberSearchResultsToDisplay");

    parNumberSearchResults.value = 0;
    parNumberSearchResultsToDisplay.value = 0;

    progress.setText("NFC: Tag ID zum Tag Namen " + parNfcName.value + " suchen...");
    online.connect();

    var data = [112]; // internal function ID

    // NFC tag UID (10 bytes, currently always empty)
    for (var i = 0; i < 10; ++i) {
        data = data.concat(0);
    }

    // NFC name
    for (var i = 0; i < parNfcName.value.length; ++i) {
        var code = parNfcName.value.charCodeAt(i);
        data = data.concat([code]);
    }
    data = data.concat(0); // null-terminated string

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            progress.setText("NFC: Tag ID zum Tag Namen " + parNfcName.value + " nicht gefunden.");
            online.disconnect();
            return;
        } else {
            throw new Error("NFC: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();

    var numRes = (resp.length - 3) / 40;
    var totalMatches = resp[1] << 8 | resp[2];
    info("totalMatches " + totalMatches);
    progress.setText("NFC: " + totalMatches + " Tag ID(s) zum Tag Namen " + parNfcName.value + " gefunden.");

    // following up to 10 results in total
    // always 2 bytes nfcId, 10 byte UID and 28 bytes nfcName
    info("Bevor: " + parNumberSearchResults.value);
    parNumberSearchResultsText.value = totalMatches;
    parNumberSearchResultsToDisplay.value = numRes;
    if (totalMatches > numRes) {
        parNumberSearchResults.value = 1;
    }

    info("Danach: " + parNumberSearchResults.value);
    for (var row = 1; row <= numRes; row++) {
        // info("NFCACT_Person" + row + "Name");
        parNfcName = device.getParameterByName("NFCACTSER_Nfc" + row + "Name");
        parNfcId = device.getParameterByName("NFCACTSER_Nfc" + row + "Id");

        var res = (row - 1) * 40 + 3;
        info("res " + row + ": " + res);
        var nfcId = resp[res + 0] << 8 | resp[res + 1];
        info("nfcId: " + nfcId);
        
        // NFC tag UID, 10 bytes, currently ignored

        var nfcName = "";
        for (var i = res + 12; i < res + 40; ++i) {
            if (resp[i] == 0)
                break; // null-termination

            nfcName += String.fromCharCode(resp[i]);
        }
        info("nfcName: " + nfcName);

        parNfcName.value = nfcName;
        parNfcId.value = nfcId;
    }
}

function ACC_searchNfcName(device, online, progress, context) {
    var parNfcName = device.getParameterByName("NFCACT_NfcName");
    //var parNfcTagUid = device.getParameterByName("NFCACT_NfcTagUid");
    var parNfcId = device.getParameterByName("NFCACT_NfcId");
    var parNumberSearchResults = device.getParameterByName("NFCACT_NumberSearchResults");

    parNumberSearchResults.value = 0;
    var nfcId = parNfcId.value;

    progress.setText("NFC: Tag Namen zu Tag ID " + nfcId + " suchen...");
    online.connect();

    var data = [111]; // internal function ID
    data = data.concat((nfcId & 0x0000ff00) >> 8, (nfcId & 0x000000ff));

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            progress.setText("NFC: Tag Namen zu Tag ID " + nfcId + " nicht gefunden.");
            online.disconnect();
            return;
        } else {
            throw new Error("NFC: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("NFC: Tag Namen zu Tag ID " + nfcId + " gefunden.");

    // NFC tag UID, 10 bytes, currently ignored

    var nfcName = "";
    for (var i = 11; i < resp.length; ++i) {
        if (resp[i] == 0)
            break; // null-termination
      
        nfcName += String.fromCharCode(resp[i]);
    }

    parNumberSearchResults.value = 1;

    parNfcName = device.getParameterByName("NFCACTSER_Nfc1Name");
    parNfcId = device.getParameterByName("NFCACTSER_Nfc1Id");
    parNfcName.value = nfcName;
    parNfcId.value = nfcId;
}

var ACC_HexDigits = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'];

function ACC_enrollNfc(device, online, progress, context) {
    var parNfcId = device.getParameterByName("ACC_EnrollNfcId");
    var parTagName = device.getParameterByName("ACC_EnrollTagName");

    progress.setText("NFC: Tag ID " + parNfcId.value + " anlernen...");
    online.connect();

    var data = [101]; // internal function ID

    // NFC ID
    data = data.concat((parNfcId.value & 0x0000ff00) >> 8, (parNfcId.value & 0x000000ff));

    // tag name
    for (var i = 0; i < parTagName.value.length; ++i) {
        var code = parTagName.value.charCodeAt(i);
        data = data.concat([code]);
    }
    data = data.concat(0); // null-terminated string

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        throw new Error("NFC: Tag anlernen konnte nicht gestartet werden!");
    }

    // we wait until enrol finger is finished
    var lCancelled = false;
    if (!lCancelled) {
        var lText = "NFC: Tag ID " + parNfcId.value + " anlernen gestartet.";
        progress.setText(lText);
        
        var data = [107]; // command wait for enroll request finished
        data = data.concat(0); // zero-terminated
        
        var resp = online.invokeFunctionProperty(160, 3, data);
        
        var lPercent = 0;
        if (resp[0] == 0) {
            // poll to check if enroll is finished
            while (resp[0] == 0 && !lCancelled) {
                data = [107];
                data = data.concat(0); // zero-terminated string
                lPercent += (90.0-lPercent)/40.0;
                if (lPercent <= 100) progress.setProgress(lPercent);
                ACC_sleep(1000);
                resp = online.invokeFunctionProperty(160, 3, data);
                // lCancelled = progress.isCanceled();
            }
            if (resp[0] == 1 && !lCancelled) {
                if (resp[1] == 0) {
                    // was not able to enroll
                    if (resp[2] == 0) {
                        // timeout or other erreor
                        throw new Error("NFC: Tag ID " + parNfcId.value + " anlernen fehlgeschlagen!");
                    } else {
                        // duplicate UID
                        var UID = "";
                        for (var i = 3; i < 13; i++) {
                            if (resp[i] == 0 && (i == 7 || i == 10)) break;
                            UID += ACC_HexDigits[resp[i]>>4] + ACC_HexDigits[resp[i]&0x0F];
                        }
                        var duplicateId = resp[13] << 8 | resp[14];
                        throw new Error("NFC: Tag ID " + parNfcId.value + " anlernen: UID " + UID + " bereits an Tag ID " + duplicateId + " angelernt.");
                    }       
                } else {
                    progress.setText("NFC: Tag ID " + parNfcId.value + " anlernen erfolgreich.");
                    var UID = "";
                    for (var i = 3; i < 13; i++) {
                        if (resp[i] == 0 && (i == 7 || i == 10)) break;
                        UID += ACC_HexDigits[resp[i]>>4] + ACC_HexDigits[resp[i]&0x0F];
                    }
                    var parNfcTagUid = device.getParameterByName("ACC_EnrollNfcKey");
                    parNfcTagUid.value = UID;
                    // info("AccessControl - UID: " + UID);
                }
            }
        }
    }
    if (lCancelled) {
        progress.setText("NFC: Anlernen vom Benutzer abgebrochen");
    }
    progress.setProgress(100);
    ACC_sleep(100);
    online.disconnect();
}

function ACC_changeNfc(device, online, progress, context) {
    var parNfcId = device.getParameterByName("ACC_EnrollNfcId");
    var parTagUid = device.getParameterByName("ACC_EnrollNfcKey");
    var parTagName = device.getParameterByName("ACC_EnrollTagName");

    progress.setText("NFC: NFC ID " + parNfcId.value + " ändern...");
    online.connect();

    var data = [104]; // internal function ID

    // finger ID
    data = data.concat((parNfcId.value & 0x0000ff00) >> 8, (parNfcId.value & 0x000000ff));

    // tag UID
    var tarUidByteCount = parTagUid.value.length / 2;
    for (var i = 0; i < tarUidByteCount; ++i) {
        data = data.concat(parseInt(parTagUid.value.substr(i * 2, 2), 16));
    }
    for (var i = tarUidByteCount; i < 10; ++i) {
        data = data.concat(0); // fill up with zeros if < 10 bytes
    }

    // person name
    for (var i = 0; i < parTagName.value.length; ++i) {
        var code = parTagName.value.charCodeAt(i);
        data = data.concat([code]);
    }
    data = data.concat(0); // null-terminated string

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            throw new Error("NFC: NFC ID " + parNfcId.value + " nicht gefunden!");
        } else {
            throw new Error("NFC: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("NFC: NFC ID " + parNfcId.value + " geändert.");
}

function ACC_syncNfc(device, online, progress, context) {
    var parNfcId = device.getParameterByName("ACC_SyncNfcId");

    progress.setText("NFC: NFC ID " + parNfcId.value + " synchronisieren...");
    online.connect();

    var data = [102]; // internal function ID
    data = data.concat((parNfcId.value & 0x0000ff00) >> 8, (parNfcId.value & 0x000000ff));

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            throw new Error("NFC: NFC ID " + parNfcId.value + " nicht gefunden!");
        } else {
            throw new Error("NFC: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("NFC: NFC ID " + parNfcId.value + " Synchronisierung gestartet.");
}

function ACC_deleteNfc(device, online, progress, context) {
    var parNfcId = device.getParameterByName("ACC_DeleteNfcId");

    progress.setText("NFC: NFC ID " + parNfcId.value + " löschen...");
    online.connect();

    var data = [103]; // internal function ID
    data = data.concat((parNfcId.value & 0x0000ff00) >> 8, (parNfcId.value & 0x000000ff));

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        if (resp[0] == 1) {
            throw new Error("NFC: NFC ID " + parNfcId.value + " nicht gefunden!");
        } else {
            throw new Error("NFC: Es ist ein unbekannter Fehler aufgetreten!");
        }
    }

    online.disconnect();
    progress.setText("NFC: NFC ID " + parNfcId.value + " gelöscht.");
}

function ACC_resetNfcScanner(device, online, progress, context) {
    progress.setText("NFC: Alle NFC-Tags löschen...");
    online.connect();

    var data = [106]; // internal function ID

    var resp = online.invokeFunctionProperty(160, 3, data);
    if (resp[0] != 0) {
        throw new Error("NFC: Es ist ein unbekannter Fehler aufgetreten!");
    }

    online.disconnect();
    progress.setText("NFC: Alle NFC-Tags gelöscht.");
}
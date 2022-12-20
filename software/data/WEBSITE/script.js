const warmK = 3000;
const coldK = 9000;

const gateway = `ws://${window.location.hostname}/ws`;

let websocket;
let isWebSocketDown = true;
let lastWsIncUpdate = "";
let packetReceived = true;
let myWsInterval;
let visible = true;
let wifiToggleDisabled = true;
let ws_CloseTimeout = 0;
let breakLoopB = false;
let breakLoopC = false;
let onGoingColorLoop = false;
let onGoingBrightLoop = false;
let loadingPageOpen = false;
let openWiFiScanMenu = false;
let disconnectedWiFi = false;
let wifiStatusCaseDone = true;
let triangleClicked = false;
let brightBarClicked = false;
let loadingRunByServer = false;
let wsUpdateInterval;
let updateWsIntervalStop = false;
let lastIncomingWiFi = {};

const controlContainer = document.getElementById("controlPage");
const settingsContainer = document.getElementById("settingsPage");
const wifiContainer = document.getElementById("WiFiPage");
const loadingPage = document.getElementById("loadingPage");
const loadingTitle = document.getElementById("loadingTitle");
const root = document.querySelector(":root");
const kelvinVal = document.getElementById("kelvinVal");
const celsiusVal = document.getElementById("celsiusVal");
const brightnessVal = document.getElementById("brightnessVal");
const brightnessBar = document.querySelector(".slider");
const triangleWrap = document.getElementById("triangleSliderWrap");
const triangle = document.getElementById("triangleSlider");
const settingsButton = document.getElementById("settingsButton");
const wiFiSettingsButton = document.getElementById("WiFiSettings");
const signOutSettingsButton = document.getElementById("signOut");
const updateSettingsButton = document.getElementById("updateButton");
const backButton = document.getElementById("backButton");
const closeButton = document.getElementById("closeButton");
const topBar = document.querySelector(".topBar");

const sections = {
    controlPage: controlContainer, settingsPage: settingsContainer, WiFiPage: wifiContainer,
};

const stopUpdateInterval = () => {
    clearInterval(wsUpdateInterval);
    updateWsIntervalStop = true;
};

const startUpdateInterval = () => {
    wsUpdateInterval = setInterval(() => {
        sendWsMSG('{"EXECUTE":"UPDATE"}');
    }, 1000);
    updateWsIntervalStop = false;
    packetReceived = true;
}

const websocketSafeClose = () => {
    clearInterval(myWsInterval);
    stopUpdateInterval();

    websocket.onclose = () => {
    };
    websocket.close();
}

const sendWsMSG = (msg) => {
    if (websocket.readyState === WebSocket.OPEN) {
        stopUpdateInterval();
        websocket.send(msg);
        startUpdateInterval();
    } else console.log("MSG could not be sent\n", msg);
    console.log("WS_BFS: ", websocket.bufferedAmount);
}

const websocketDown = (isDown) => {
    if (isDown) setLoadingPage(true, "Trying to reconnect...");
    else if (isWebSocketDown) setLoadingPage(false);
    isWebSocketDown = isDown;
}


const initWebSocket = () => {
    myWsInterval = setInterval(() => {
        console.log("interval " + "rec " + packetReceived + " state " + websocket.readyState + " TOC: " + ws_CloseTimeout);
        if (updateWsIntervalStop === false && packetReceived === false) {
            console.log("closing...");
            websocketDown(true);
            if (websocket.readyState === WebSocket.CLOSING) {
                if (++ws_CloseTimeout >= 3) {
                    window.location.reload();
                }
            } else websocket.close();
        } else ws_CloseTimeout = 0;
        packetReceived = false;
    }, 4000);
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
    websocket.onerror = (event) => {
        console.error("WebSocket error observed:", event);
    };
}

const onOpen = () => {
    console.log("Connection Opened");
    websocketDown(false);
    startUpdateInterval();
}

const onClose = () => {
    websocketDown(true);
    console.log("Connection Closed, last message:");
    console.timeEnd("message");
    console.log("BFS: ", websocket.bufferedAmount);
    clearInterval(myWsInterval);
    clearInterval(wsUpdateInterval);
    setTimeout(initWebSocket, 1000);
}

const wifiConnectSelectedNetwork = (ssid, connected, secure) => {
    if (connected === true) {
        if (confirm("You are already connected to the network below\n" + ssid + "\nWould you like to disconnect?")) {
            console.log("connecting... " + ssid);
            setLoadingPage(true, "Disconnecting");
            sendWsMSG(`{"WIFI":{"DISCONNECT":true}}`);
            setTimeout(() => {
                wifiStatusCaseDone = false;
            }, 1000);
            // websocket.close();
            wifiContainer.innerHTML = "";
        } else {
            console.log("Thing was not saved to the database.");
        }
    } else if (secure === true) {
        let pass = prompt("Please enter the password to connect to the network below\n" + ssid, "Enter the password");
        if (pass != null) {
            console.log("connecting... " + ssid);
            // wifiContainer.style.display = "none";
            setLoadingPage(true, "Connecting to<br/>" + ssid);
            sendWsMSG(`{"WIFI":{"CONNECT":{"SSID":"${ssid}","PASS":"${pass}"}}}`);
            // websocket.close();
            wifiStatusCaseDone = false;
            wifiContainer.innerHTML = "";
        }
    }
};

const sendScanCommand = () => {
    sendWsMSG(`{"WIFI":{"SCAN":true}}`);
    setLoadingPage(true, "Scanning for networks");
};

const wifiScanResultPrint = (ssid, rssi, secure, connected = false) => {
    let wifiQuality = dbmToPercent(rssi);
    let wifiColors = new Array(4);

    console.log(ssid + "  " + connected);

    for (let i = 0; i < 4; i++) {
        if (wifiQuality >= i * 25) {
            wifiColors[i] = connected === true ? "#98ffc8" : "#575f7f";
        } else {
            wifiColors[i] = "hsla(228, 19%, 42%, 0.3)";
        }
    }
    wifiContainer.innerHTML += `<div class="wifiCard" onclick="wifiConnectSelectedNetwork('${ssid}',${connected},${secure})">
<div class="info">
  <div class="wifiName">${ssid}</div>
  <div class="icons">
    <div class="wifiSecure"${secure === true ? "" : ' style="display: none;"'}>
      <svg xmlns="http://www.w3.org/2000/svg" width="14" height="15.333" viewBox="0 0 14 15.333">
      <g id="Icon_feather-lock" data-name="Icon feather-lock" transform="translate(-1 -0.333)">
          <path id="Path_10" data-name="Path 10" d="M4.667,7.333V4.667a3.333,3.333,0,1,1,6.667,0V7.333" fill="none" stroke="${connected === true ? "#98ffc8" : "#575f7f"}" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" />
          <path id="Path_9" data-name="Path 9" d="M3.333,7.333h9.333A1.333,1.333,0,0,1,14,8.667v4.667a1.333,1.333,0,0,1-1.333,1.333H3.333A1.333,1.333,0,0,1,2,13.333V8.667A1.333,1.333,0,0,1,3.333,7.333Z" fill="none" stroke="${connected === true ? "#98ffc8" : "#575f7f"}" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" />
      </g>
      </svg>
    </div>
    <div class="wifiStrength">
      <svg
      xmlns="http://www.w3.org/2000/svg"
      width="32"
      height="32"
      viewBox="0 0 24 24"
      fill="none"
      stroke="hsla(228, 19%, 42%, 0.3)"
      stroke-width="2"
      stroke-linecap="round"
      stroke-linejoin="round"
      class="feather feather-wifi"
    >
      <path d="M1.42 9a16 16 0 0 1 21.16 0" stroke="${wifiColors[3]}"></path>
      <path d="M5 12.55a11 11 0 0 1 14.08 0" stroke="${wifiColors[2]}"></path>
      <path d="M8.53 16.11a6 6 0 0 1 6.95 0" stroke="${wifiColors[1]}"></path>
      <line x1="12" y1="20" x2="12.01" y2="20" stroke="${wifiColors[0]}"></line>
    </svg>
    </div>
  </div>
</div>
<div class="pass" style="display: none;">
  <input
    type="password"
    placeholder="Type the password"
    name="wifiPass"
    class="wifiPass"
  />
  <input type="button" value="CONNECT" />
</div>
</div>`;
};

const breakOngoingColorProcess = async () => {
    return new Promise(async (resolve) => {
        breakLoopC = true;
        await new Promise((resolve) => {
            let _t = setInterval(() => {
                if (onGoingColorLoop === false) {
                    clearInterval(_t);
                    resolve(true);
                }
            }, 100);
        });
        breakLoopC = false;
        resolve(true);
    });
};

const breakOngoingBrightProcess = async () => {
    return new Promise(async (resolve) => {
        breakLoopB = true;
        await new Promise((resolve) => {
            let _t = setInterval(() => {
                if (onGoingBrightLoop === false) {
                    clearInterval(_t);
                    resolve(true);
                }
            }, 100);
        });
        breakLoopB = false;
        resolve(true);
    });
};

console.time("message");

const onMessage = (event) => {
    console.timeEnd("message");
    console.time("message");

    packetReceived = true;
    let incoming = event.data;
    if (incoming === "WS_CLIENT_OVERFLOW") {
        websocketSafeClose();

        setLoadingPage(true, "MAX CLIENT OVERFLOW<br>PLEASE CLOSE ANOTHER CLIENT<br>THEN REFRESH THIS PAGE")
        loadingPage.style.color = "#ff5d5d";
        document.querySelector("#loadingPage .triangle").style.display = "none";
        return;
    }
    const incomingJSON = JSON.parse(incoming);
    console.log(JSON.parse(JSON.stringify(incomingJSON)));
    celsiusVal.innerText = Math.round(incomingJSON["NTC"]["temp"]).toString();
    wifiStrengthUpdate(incomingJSON["WIFI"]["CONNECTED"]["RSSI"], incomingJSON["WIFI"]["STATUS"] === 3);

    if (incomingJSON["WIFI"]["SCAN"]["STATUS"] === true && incomingJSON["WIFI"]["SCAN"]["DONE"] === false) {
        console.log("scan in process");
    }
    console.log(incomingJSON["WIFI"]["STATUS"]);

    if (wifiStatusCaseDone === false && incomingJSON["TIMEOUT"] === true) {
        alert("TIMEOUT");
    }

    if (wifiStatusCaseDone === false) switch (incomingJSON["WIFI"]["STATUS"]) {
        case 0:
            if (loadingPageOpen === true) {
                if (loadingTitle.innerHTML === "Disconnecting") {
                    disconnectedWiFi = true;
                    wifiStatusCaseDone = true;
                    setLoadingPage();
                    if (openWiFiScanMenu === true) sendScanCommand();
                }
            }
            break;
        case 3:
            if (loadingPageOpen === true) {
                disconnectedWiFi = false;
                wifiStatusCaseDone = true;
                setLoadingPage();
                if (openWiFiScanMenu === true) sendScanCommand();
            }
            break;
        case 4:
            if (loadingPageOpen === true) {
                disconnectedWiFi = false;
                wifiStatusCaseDone = true;
                setLoadingPage();
                if (openWiFiScanMenu === true) sendScanCommand();
                alert("Connection failed!");
            }
            break;
        case 6:
            if (loadingPageOpen === true) {
                disconnectedWiFi = false;
                wifiStatusCaseDone = true;
                setLoadingPage();
                alert("Password not correct!");
                if (openWiFiScanMenu === true) sendScanCommand();
            }
            break;
        case 7:
            if (loadingPageOpen === true) {
                disconnectedWiFi = true;
            }
            break;

        default:
            break;
    }

    if (openWiFiScanMenu && incomingJSON["WIFI"]["SCAN"]["STATUS"] === false && incomingJSON["WIFI"]["SCAN"]["DONE"] === true) {
        wifiContainer.innerHTML = "";
        incomingJSON["WIFI"]["SCAN"]["RESULT"].forEach((network) => {
            wifiScanResultPrint(network["SSID"], network["RSSI"], network["OPEN"] === false, incomingJSON["WIFI"]["STATUS"] === 3 && network["SSID"] === incomingJSON["WIFI"]["CONNECTED"]["SSID"]);
        });
        changeSection("WiFiPage");
        setLoadingPage();
    }

    if (loadingPageOpen === false || loadingRunByServer === true) {
        loadingRunByServer = incomingJSON["LOADING"]["OPEN"];
        setLoadingPage(loadingRunByServer, incomingJSON["LOADING"]["TITLE"]);
        //if (incomingJSON["LOADING"]["TITLE"].toLowerCase() === "rebooting") setTimeout(() => window.location.reload(), 3000)
    }

    lastIncomingWiFi = incomingJSON["WIFI"];
    delete incomingJSON["WIFI"];
    delete incomingJSON["NTC"];

    if (JSON.stringify(incomingJSON) === lastWsIncUpdate) return;
    console.log("JSON.stringify(event.data)");
    console.log(JSON.stringify(incomingJSON));
    console.log(lastWsIncUpdate);
    console.log("update");
    lastWsIncUpdate = JSON.stringify(incomingJSON);
    if (!triangleClicked) {
        updateDisplayColorTone(incomingJSON["LED"]["COLD"], incomingJSON["LED"]["WARM"]).then();
    }
    if (!brightBarClicked) {
        updateDisplayBrightness(incomingJSON["LED"]["BRIGHTNESS"]).then();
    }
};

const changeButton = (_settingsButton, _closeButton, _backButton) => {
    settingsButton.style.display = "none";
    closeButton.style.display = "none";
    backButton.style.display = "none";
    if (_settingsButton) settingsButton.style.display = "block";
    if (_closeButton) closeButton.style.display = "block";
    if (_backButton) backButton.style.display = "block";
};

const changeSection = (_section) => {
    for (const section in sections) sections[section].style.display = "none";
    if (sections[_section]) sections[_section].style.display = "flex";
};

const setLoadingPage = (_open = false, title = "") => {
    loadingPageOpen = _open;
    loadingTitle.innerHTML = title;
    loadingPage.style.display = _open === true ? "flex" : "none";
    topBar.style.display = _open === true ? "none" : "flex";
};

const setBrightness = (v) => {
    let _v = Math.round(v / 2.55);
    let _t = "";
    if (_v < 100) _t += "&nbsp;";
    if (_v < 10) _t += "&nbsp;";
    brightnessBar.value = v;
    brightnessVal.innerHTML = _t + _v;
};

const setKelvin = (v) => {
    let _v = round50(v);
    let _t = "";
    for (let i = _v.toString().length; i < coldK.toString().length; ++i) _t += "&nbsp;";
    kelvinVal.innerHTML = _t + _v;
};

const dbmToPercent = (dbm) => {
    if (dbm < -100) return 0;
    if (dbm >= -50) return 100;
    return 2 * (dbm + 100);
}

const wifiStrengthUpdate = (dbm, connected) => {
    wifiToggleDisabled = !connected;
    document.getElementById("wifiInfo").style.cursor = wifiToggleDisabled ? "default" : "pointer";
    let wifiQuality = dbmToPercent(dbm);
    root.style.setProperty("--wifi1", "");
    for (let i = 0; i < 4; i++) {
        if (connected === true && wifiQuality >= i * 25) {
            root.style.setProperty(`--wifi${i + 1}`, "#575F7F");
        } else {
            root.style.setProperty(`--wifi${i + 1}`, "hsla(228, 19%, 42%, 0.3)");
        }
    }
}

const getCursorPosition = (canvas, event) => {
    let _x, _y;
    const rect = canvas.getBoundingClientRect();
    if (window.TouchEvent && event instanceof TouchEvent) {
        _x = event.touches[0].pageX;
        _y = event.touches[0].pageY;
    } else {
        _x = event.clientX;
        _y = event.clientY;
    }
    const x = _x - rect.left;
    const y = _y - rect.top;
    // console.log("x: " + x + " y: " + y);
    return {x: x, y: y};
}

const calcAngleDegrees = (x, y) => {
    return (Math.atan2(y, x) * 180) / Math.PI;
}

const maxMinHeight = (mx, mn, _in) => {
    _in = Math.round(_in * 10) / 10;
    if (_in > mx) return mx;
    if (_in < mn) return mn;
    return _in;
}

const delay = (_ms) => {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve(true);
        }, _ms);
    });
};

const ParametricBlend = (t) => {
    let sqt = t * t;
    return sqt / (2.0 * (sqt - t) + 1.0);
};

const updateDisplayBrightness = async (newBright) => {
    return new Promise(async (resolve) => {
        await breakOngoingBrightProcess();
        onGoingBrightLoop = true;
        let oldBright = parseInt(brightnessBar.value);
        let amountBright = newBright - oldBright;
        let _delayOld = 0;
        let _delayMS = 0;
        if (amountBright !== 0) {
            for (let i = 0; i < 99 && visible; i++) {
                _delayMS = ParametricBlend((i + 1) / 100);
                oldBright = oldBright + amountBright * (_delayMS - _delayOld);
                setBrightness(oldBright);
                await delay((_delayMS - _delayOld) * 800);
                _delayOld = _delayMS;
                if (breakLoopB) break;
            }
            if (!breakLoopB) setBrightness(newBright);
        }
        onGoingBrightLoop = false;
        resolve(true);
    });
};

const round50 = (x) => {
    return Math.floor((x + 25) / 50) * 50;
};

const updateDisplayColorTone = async (coldPerc, warmPerc) => {
    return new Promise(async (resolve) => {
        await breakOngoingColorProcess();
        onGoingColorLoop = true;
        root.style.setProperty("--warmPercent", `${maxMinHeight(204, 0, (204 * warmPerc) / 255)}px`);
        root.style.setProperty("--coldPercent", `${maxMinHeight(204, 0, (204 * coldPerc) / 255)}px`);
        let angle = ((warmPerc - coldPerc) * 140) / 255;
        triangle.style.transform = `rotate(${angle}deg)`;

        let oldColVal = parseInt(kelvinVal.innerText);
        let percent = warmPerc / 2;
        if (warmPerc === 255) percent += (255 - coldPerc) / 2;

        console.log("PERCENT CALCULATED " + percent);
        let newColVal = Math.round((coldK * (255 - percent)) / 255) + Math.round((warmK * percent) / 255);
        let amountCol = newColVal - oldColVal;
        let _delayOld = 0;
        let _delayMS = 0;
        if (amountCol !== 0) {
            for (let i = 0; i < 99 && visible; i++) {
                _delayMS = ParametricBlend((i + 1) / 255);
                oldColVal = oldColVal + amountCol * (_delayMS - _delayOld);
                setKelvin(oldColVal);
                await delay((_delayMS - _delayOld) * 800);
                _delayOld = _delayMS;
                if (breakLoopC) break;
            }
            if (!breakLoopC) setKelvin(newColVal);
        }
        onGoingColorLoop = false;
        resolve(true);
    });
};

const handleMove = async (e) => {
    if (!triangleClicked) return;
    await breakOngoingColorProcess();
    e.preventDefault();
    let pos = getCursorPosition(triangleWrap, e);
    let angle = calcAngleDegrees(pos.x - 144.62 / 2, pos.y - 126.13 / 2) + 90;
    if (!(angle <= 140 || angle >= 220)) return;
    triangle.style.transform = `rotate(${angle}deg)`;
    if (angle > 140) angle -= 360;
    // console.log(angle);

    let warmPercent = Math.round(maxMinHeight(255, 0, (255 * (angle + 139)) / 139));
    let coldPercent = Math.round(maxMinHeight(255, 0, (255 * (-angle + 139)) / 139));

    sendWsMSG(`{"LED":{"COLD":${coldPercent},"WARM":${warmPercent}}}`);

    root.style.setProperty("--warmPercent", `${maxMinHeight(204, 0, (204 * (angle + 139)) / 139)}px`);
    root.style.setProperty("--coldPercent", `${maxMinHeight(204, 0, (204 * (-angle + 139)) / 139)}px`);

    let percent = warmPercent / 2;
    if (warmPercent === 255) percent += (255 - coldPercent) / 2;

    // let percent = maxMinHeight(100, 0, ((angle + 139) * 100) / 278);
    percent = Math.round(percent);
    console.log("PERCENT " + percent);
    setKelvin((coldK * (255 - percent)) / 255 + (warmK * percent) / 255);
};

const mouseDownEvent = () => {
    triangleClicked = true;
    root.style.setProperty("--animation", `transform 0s ease-in-out, height 0s ease-in-out`);
};

const mouseUpEvent = () => {
    triangleClicked = false;
    lastWsIncUpdate = "";
    root.style.setProperty("--animation", `transform 1s ease-in-out, height 1s ease-in-out`);
};

const mouseDownBr = () => {
    breakOngoingBrightProcess().then();
    brightBarClicked = true;
};

const mouseUpBr = () => {
    brightBarClicked = false;
    lastWsIncUpdate = "";
};

updateSettingsButton.addEventListener("click", () => {
    window.location.assign("/update");
});

signOutSettingsButton.addEventListener("click", () => {
    let request = new XMLHttpRequest();
    request.addEventListener("load", (e) => {
        if (e.target.status !== 401) return alert("Something went wrong!");
        alert("You have been signed out.");
        window.location.reload();
    });
    request.open("GET", "/logout");
    request.send();
});

wiFiSettingsButton.addEventListener("click", () => {
    console.log("clicked");
    openWiFiScanMenu = true;
    sendScanCommand();
    changeButton(false, false, true);
    changeSection("WiFiPage");
});

backButton.addEventListener("click", () => {
    if (openWiFiScanMenu === true) {
        openWiFiScanMenu = false;
        sendWsMSG(`{"WIFI":{"SCAN_DEL":true}}`);
    }
    changeButton(false, true, false);
    changeSection("settingsPage");
});

closeButton.addEventListener("click", () => {
    changeButton(true, false, false);
    changeSection("controlPage");
});

settingsButton.addEventListener("click", () => {
    changeButton(false, true, false);
    changeSection("settingsPage");
});

document.addEventListener("visibilitychange", () => {
    if (document.visibilityState === "visible") {
        visible = true;
        root.style.setProperty("--animation", `transform 1s ease-in-out, height 1s ease-in-out`);
    } else {
        visible = false;
        root.style.setProperty("--animation", `transform 0s ease-in-out, height 0s ease-in-out`);
    }
});

brightnessBar.addEventListener("input", (e) => {
    setBrightness(e.target.value);
    sendWsMSG(`{"LED":{"BRIGHTNESS":${e.target.value}}}`);
});
brightnessBar.addEventListener("mousedown", mouseDownBr);
brightnessBar.addEventListener("touchstart", mouseDownBr);
brightnessBar.addEventListener("mouseup", mouseUpBr);
brightnessBar.addEventListener("touchend", mouseUpBr);

triangleWrap.addEventListener("mousedown", mouseDownEvent);
triangleWrap.addEventListener("touchstart", mouseDownEvent);
triangleWrap.addEventListener("mouseup", mouseUpEvent);
triangleWrap.addEventListener("touchend", mouseUpEvent);
triangleWrap.addEventListener("mousemove", handleMove);
triangleWrap.addEventListener("touchmove", handleMove);
window.onload = () => {
    initWebSocket();
};
window.onbeforeunload = () => {
    websocketSafeClose();
};
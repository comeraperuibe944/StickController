#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>

//Password is recommended even if the default is null
const char* ap_ssid = "M5Controller";
const char* ap_password = NULL;

WebServer server(80);
XboxGamepadDevice* gamepad;
BleCompositeHID compositeHID("M5Controller", "M5Stack", 100);

//Web buttons
bool web_A = false, web_B = false, web_X = false, web_Y = false;
bool web_LB = false, web_RB = false;
bool web_LS_click = false, web_RS_click = false;
bool web_Menu = false, web_View = false, web_Home = false;
uint16_t web_LT = 0, web_RT = 0;
int16_t dpad_x = 0, dpad_y = 0;
int16_t ls_x = 0, ls_y = 0;
int16_t rs_x = 0, rs_y = 0;

void handleRoot();
void handlePress();
void handleStick();
void handleNotFound();
void updateAndSendReport();

void setup() {
    M5.begin();
    Serial.begin(115200);

    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    
    // instructions text
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 5);
    M5.Display.println("Instructions:");
    M5.Display.setTextSize(1);
    M5.Display.setCursor(0, 25);
    M5.Display.println("On pc, open Bluetooth > Add device >");
    M5.Display.setCursor(0, 35);
    M5.Display.println("M5Controller");
    
    M5.Display.setCursor(0, 50);
    M5.Display.println("(optional) On phone, connect to");
    M5.Display.setCursor(0, 60);
    M5.Display.println("WiFi M5Controller");
    
    M5.Display.setCursor(0, 75);
    M5.Display.println("(optional) On browser, go to");
    M5.Display.setCursor(0, 85);
    M5.Display.println("http://192.168.4.1");
   
    WiFi.softAP(ap_ssid, ap_password);
    
    server.on("/", handleRoot);
    server.on("/press", handlePress);
    server.on("/stick", handleStick);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("Web server started.");

    XboxSeriesXControllerDeviceConfiguration* config = new XboxSeriesXControllerDeviceConfiguration();
    gamepad = new XboxGamepadDevice(config);
    compositeHID.addDevice(gamepad);
    compositeHID.begin(config->getIdealHostConfiguration());
    Serial.println("Bluetooth gamepad started.");
}

void loop() {
    server.handleClient();
    updateAndSendReport();
    delay(10);
}

void updateAndSendReport() {
    M5.update();
    bool physical_A = M5.BtnA.isPressed();
    bool physical_B = M5.BtnB.isPressed();

    String pressedKeys = "";
    //updates pressed keys text on lower part of screen only since the top never changes
    M5.Display.fillRect(0, 105, 240, 30, BLACK);
    M5.Display.setCursor(0, 110);
    M5.Display.setTextSize(2);

    if (web_A || physical_A) { gamepad->press(XBOX_BUTTON_A); pressedKeys += "A "; } else { gamepad->release(XBOX_BUTTON_A); }
    if (web_B || physical_B) { gamepad->press(XBOX_BUTTON_B); pressedKeys += "B "; } else { gamepad->release(XBOX_BUTTON_B); }
    
    if (web_X) { gamepad->press(XBOX_BUTTON_X); pressedKeys += "X "; } else { gamepad->release(XBOX_BUTTON_X); }
    if (web_Y) { gamepad->press(XBOX_BUTTON_Y); pressedKeys += "Y "; } else { gamepad->release(XBOX_BUTTON_Y); }
    if (web_LB) { gamepad->press(XBOX_BUTTON_LB); pressedKeys += "LB "; } else { gamepad->release(XBOX_BUTTON_LB); }
    if (web_RB) { gamepad->press(XBOX_BUTTON_RB); pressedKeys += "RB "; } else { gamepad->release(XBOX_BUTTON_RB); }
    if (web_Menu) { gamepad->press(XBOX_BUTTON_START); pressedKeys += "Menu "; } else { gamepad->release(XBOX_BUTTON_START); }
    if (web_View) { gamepad->press(XBOX_BUTTON_SELECT); pressedKeys += "View "; } else { gamepad->release(XBOX_BUTTON_SELECT); }
    if (web_Home) { gamepad->press(XBOX_BUTTON_HOME); pressedKeys += "Home "; } else { gamepad->release(XBOX_BUTTON_HOME); }
    if(web_LS_click) { gamepad->press(XBOX_BUTTON_LS); pressedKeys += "L3 "; } else { gamepad->release(XBOX_BUTTON_LS); }
    if(web_RS_click) { gamepad->press(XBOX_BUTTON_RS); pressedKeys += "R3 "; } else { gamepad->release(XBOX_BUTTON_RS); }

    gamepad->setLeftTrigger(web_LT);
    gamepad->setRightTrigger(web_RT);
    if(web_LT > 0) pressedKeys += "LT ";
    if(web_RT > 0) pressedKeys += "RT ";
    
    uint8_t dpad_flags = 0;
    if (dpad_y > 0) { dpad_flags |= (uint8_t)XboxDpadFlags::NORTH; pressedKeys += "Up "; }
    else if (dpad_y < 0) { dpad_flags |= (uint8_t)XboxDpadFlags::SOUTH; pressedKeys += "Down "; }
    if (dpad_x < 0) { dpad_flags |= (uint8_t)XboxDpadFlags::WEST; pressedKeys += "Left "; }
    else if (dpad_x > 0) { dpad_flags |= (uint8_t)XboxDpadFlags::EAST; pressedKeys += "Right "; }

    if (dpad_flags != 0) gamepad->pressDPadDirectionFlag((XboxDpadFlags)dpad_flags);
    else gamepad->releaseDPad();

    gamepad->setLeftThumb(ls_x, ls_y);
    gamepad->setRightThumb(rs_x, rs_y);

    if (ls_x != 0 || ls_y != 0) pressedKeys += "LS ";
    if (rs_x != 0 || rs_y != 0) pressedKeys += "RS ";

    M5.Display.print(pressedKeys);

    gamepad->sendGamepadReport();
}


void handlePress() {
    String btn = server.arg("btn");
    String action = server.arg("action");
    bool is_down = (action == "down");

    if (btn == "A") web_A = is_down;
    else if (btn == "B") web_B = is_down;
    else if (btn == "X") web_X = is_down;
    else if (btn == "Y") web_Y = is_down;
    else if (btn == "Menu") web_Menu = is_down;
    else if (btn == "View") web_View = is_down;
    else if (btn == "Home") web_Home = is_down;
    else if (btn == "LB") web_LB = is_down;
    else if (btn == "RB") web_RB = is_down;
    else if (btn == "LS_click") web_LS_click = is_down;
    else if (btn == "RS_click") web_RS_click = is_down;
    else if (btn == "LT") web_LT = is_down ? XBOX_TRIGGER_MAX : 0;
    else if (btn == "RT") web_RT = is_down ? XBOX_TRIGGER_MAX : 0;
    else if (btn == "DPAD_up") { dpad_y = is_down ? 1 : 0; }
    else if (btn == "DPAD_down") { dpad_y = is_down ? -1 : 0; }
    else if (btn == "DPAD_left") { dpad_x = is_down ? -1 : 0; }
    else if (btn == "DPAD_right") { dpad_x = is_down ? 1 : 0; }

    server.send(200, "text/plain", "OK");
}

void handleStick() {
    String id = server.arg("id");
    int16_t x = server.arg("x").toInt();
    int16_t y = server.arg("y").toInt();

    if (id == "LS") { ls_x = x; ls_y = y; } 
    else if (id == "RS") { rs_x = x; rs_y = y; }
    server.send(200, "text/plain", "OK");
}

void handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html><html><head><title>Xbox Controller</title><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
<style>
    :root { --green:#107C10; --red:#E81123; --blue:#0078D7; --yellow:#FFB900; --grey:#555; --dark-grey:#333; }
    body,html { overscroll-behavior:none; margin:0; padding:0; background-color:#1e1e1e; font-family:system-ui, -apple-system, sans-serif; color:white; -webkit-user-select:none; user-select:none; width:100%; height:100%; overflow:hidden; }
    
    .joystick-zone { position:fixed; top:0; height:100%; width:50%; z-index:0; }
    #joystick-zone-left { left:0; }
    #joystick-zone-right { right:0; }

    .joystick-base { position:absolute; width:130px; height:130px; background:rgba(85,85,85,0.4); border-radius:50%; transition: opacity 0.2s; pointer-events:none; }
    .joystick-stick { position:absolute; width:65px; height:65px; background:rgba(128,128,128,0.7); border-radius:50%; pointer-events:none; }

    .button-container { position:fixed; z-index:1; }
    .button { background:var(--grey); display:flex; align-items:center; justify-content:center; font-weight:bold; border:none; box-sizing:border-box; transition:transform 0.05s ease; color:white; }
    .button.active, .button:active { transform:scale(0.9); filter:brightness(0.8); }

    /* Top Buttons */
    #top-left-buttons { top:5%; left:3%; display:flex; gap:10px; }
    #top-right-buttons { top:5%; right:3%; display:flex; gap:10px; }
    #top-center-buttons { top:3%; left:50%; transform:translateX(-50%); display:flex; gap:15px; align-items:center; }
    .bumper-btn { width:60px; height:35px; border-radius:8px; }
    .trigger-btn { width:60px; height:35px; border-radius:8px; background:var(--dark-grey); }
    .system-btn { width:45px; height:45px; border-radius:50%; font-size:24px; padding:0; }

    /* Bottom Left: D-Pad */
    #dpad-container { bottom:12%; left:10%; width:150px; height:150px; display:grid; grid-template:repeat(3, 1fr) / repeat(3, 1fr); gap:4px; }
    .dpad-btn { border-radius:10px; font-size:24px; }
    #btn-DPAD_up { grid-area: 1 / 2; } #btn-DPAD_left { grid-area: 2 / 1; } #btn-DPAD_right { grid-area: 2 / 3; } #btn-DPAD_down { grid-area: 3 / 2; }
    
    /* Bottom Right: Action Buttons */
    #action-buttons-container { bottom:12%; right:10%; width:150px; height:150px; display:grid; grid-template:repeat(3, 1fr) / repeat(3, 1fr); gap:4px; }
    .action-btn { border-radius:50%; font-size:20px; }
    #btn-Y { grid-area:1 / 2; background:var(--yellow); color:black; }
    #btn-X { grid-area:2 / 1; background:var(--blue); }
    #btn-B { grid-area:2 / 3; background:var(--red); }
    #btn-A { grid-area:3 / 2; background:var(--green); }

    /* L3/R3 in corners */
    .corner-btn { position:fixed; bottom:3%; width:45px; height:45px; border-radius:50%; background:var(--dark-grey); font-size:14px; z-index:1; }
    #btn-LS_click { left:3%; }
    #btn-RS_click { right:3%; }
</style>
</head><body>
<div id="joystick-zone-left" class="joystick-zone"></div>
<div id="joystick-zone-right" class="joystick-zone"></div>

<div id="top-left-buttons" class="button-container">
    <button id="btn-LB" class="button bumper-btn">LB</button>
    <button id="btn-LT" class="button trigger-btn">LT</button>
</div>
<div id="top-right-buttons" class="button-container">
    <button id="btn-RT" class="button trigger-btn">RT</button>
    <button id="btn-RB" class="button bumper-btn">RB</button>
</div>
<div id="top-center-buttons" class="button-container">
    <button id="btn-View" class="button system-btn">&#9664;</button>
    <button id="btn-Home" class="button system-btn">&#x2302;</button>
    <button id="btn-Menu" class="button system-btn">&#9654;</button>
</div>

<div id="dpad-container" class="button-container">
    <button id="btn-DPAD_up" class="button dpad-btn">▲</button>
    <button id="btn-DPAD_left" class="button dpad-btn">◀</button>
    <button id="btn-DPAD_right" class="button dpad-btn">▶</button>
    <button id="btn-DPAD_down" class="button dpad-btn">▼</button>
</div>

<div id="action-buttons-container" class="button-container">
    <button id="btn-Y" class="button action-btn" data-key="c">Y</button>
    <button id="btn-X" class="button action-btn" data-key="v">X</button>
    <button id="btn-B" class="button action-btn" data-key="z">B</button>
    <button id="btn-A" class="button action-btn" data-key="x">A</button>
</div>

<button id="btn-LS_click" class="button corner-btn">L3</button>
<button id="btn-RS_click" class="button corner-btn">R3</button>

<script>
class VirtualJoystick {
    constructor(zoneId, yMultiplier = 1) {
        this.zone = document.getElementById(zoneId);
        this.yMultiplier = yMultiplier;
        this.active = false; this.touchId = null;
        this.radius = 65;
        this.x = 0; this.y = 0;
        this.stickX = 0; this.stickY = 0;
        
        this.base = document.createElement('div');
        this.base.className = 'joystick-base';
        this.stick = document.createElement('div');
        this.stick.className = 'joystick-stick';

        this.zone.addEventListener('touchstart', this.handleStart.bind(this), { passive: false });
        this.zone.addEventListener('touchmove', this.handleMove.bind(this), { passive: false });
        this.zone.addEventListener('touchend', this.handleEnd.bind(this), { passive: false });
        this.zone.addEventListener('touchcancel', this.handleEnd.bind(this), { passive: false });
    }
    handleStart(e) {
        if (e.target !== this.zone) return;
        e.preventDefault();
        const touch = e.changedTouches[0];
        this.touchId = touch.identifier; this.active = true;
        document.body.appendChild(this.base); document.body.appendChild(this.stick);
        this.x = touch.pageX; this.y = touch.pageY;
        this.base.style.left = `${this.x}px`; this.base.style.top = `${this.y}px`;
        this.base.style.transform = 'translate(-50%, -50%)'; this.base.style.opacity = 1;
        this.stick.style.left = `${this.x}px`; this.stick.style.top = `${this.y}px`;
        this.stick.style.transform = 'translate(-50%, -50%)';
    }
    handleMove(e) {
        if (!this.active) return;
        const touch = Array.from(e.touches).find(t => t.identifier === this.touchId);
        if (!touch) return;
        e.preventDefault();
        let deltaX = touch.pageX - this.x; let deltaY = touch.pageY - this.y;
        const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
        if (distance > this.radius) {
            deltaX = (deltaX / distance) * this.radius;
            deltaY = (deltaY / distance) * this.radius;
        }
        this.stick.style.left = `${this.x + deltaX}px`; this.stick.style.top = `${this.y + deltaY}px`;
        this.stickX = Math.round((deltaX / this.radius) * 32767);
        this.stickY = Math.round((deltaY / this.radius) * 32767 * this.yMultiplier);
    }
    handleEnd(e) {
        const touch = Array.from(e.changedTouches).find(t => t.identifier === this.touchId);
        if (!this.active || !touch) return;
        e.preventDefault();
        this.base.style.opacity = 0; this.stickX = 0; this.stickY = 0;
        setTimeout(() => {
            if (!this.active) {
                if(this.base.parentNode) document.body.removeChild(this.base);
                if(this.stick.parentNode) document.body.removeChild(this.stick);
            }
        }, 200);
        this.active = false; this.touchId = null;
    }
}

// Analógico Esquerdo: Y INVERTIDO 
const leftStick = new VirtualJoystick('joystick-zone-left', 1); 
// Analógico Direito: Y NÃO INVERTIDO
const rightStick = new VirtualJoystick('joystick-zone-right', -1); 

function sendEvent(btnName, action) { fetch(`/press?btn=${btnName}&action=${action}`); }

const standardButtons = ["Menu", "View", "Home", "LB", "RB", "LT", "RT", "DPAD_up", "DPAD_down", "DPAD_left", "DPAD_right", "LS_click", "RS_click"];
standardButtons.forEach(id => {
    const el = document.getElementById(`btn-${id}`);
    if (!el) return;
    el.addEventListener("touchstart", e => { e.preventDefault(); e.stopPropagation(); sendEvent(id, "down"); }, { passive: false });
    el.addEventListener("touchend", e => { e.preventDefault(); e.stopPropagation(); sendEvent(id, "up"); }, { passive: false });
});

const actionContainer = document.getElementById('action-buttons-container');
let lastActionBtnId = null;
const handleActionTouch = (event) => {
    event.preventDefault();
    let touch = event.touches[0] || event.changedTouches[0];
    let element = document.elementFromPoint(touch.clientX, touch.clientY);
    let currentBtnId = null;
    if (element && element.classList.contains('action-btn')) {
        currentBtnId = element.id.replace('btn-', '');
    }
    if (currentBtnId !== lastActionBtnId) {
        if (lastActionBtnId) sendEvent(lastActionBtnId, 'up');
        if (currentBtnId) sendEvent(currentBtnId, 'down');
        lastActionBtnId = currentBtnId;
    }
};
const endActionTouch = (event) => {
    event.preventDefault();
    if (lastActionBtnId) {
        sendEvent(lastActionBtnId, 'up');
        lastActionBtnId = null;
    }
};
actionContainer.addEventListener('touchstart', handleActionTouch, { passive: false });
actionContainer.addEventListener('touchmove', handleActionTouch, { passive: false });
actionContainer.addEventListener('touchend', endActionTouch, { passive: false });
actionContainer.addEventListener('touchcancel', endActionTouch, { passive: false });

const keyMap = {
    ArrowUp: 'DPAD_up', ArrowDown: 'DPAD_down', ArrowLeft: 'DPAD_left', ArrowRight: 'DPAD_right',
    z: 'B', x: 'A', c: 'Y', v: 'X',
    q: 'LB', e: 'RB', '1': 'LT', '3': 'RT',
    Enter: 'Menu', Shift: 'View',
    " ": 'Home'
};
const keyStates = {};

document.addEventListener('keydown', (e) => {
    const key = e.key;
    const keyLower = key.toLowerCase();
    const btnId = keyMap[key] || keyMap[keyLower];
    if (!btnId || keyStates[keyLower]) return;
    
    keyStates[keyLower] = true;
    document.getElementById(`btn-${btnId}`)?.classList.add('active');
    sendEvent(btnId, 'down');
});

document.addEventListener('keyup', (e) => {
    const key = e.key;
    const keyLower = key.toLowerCase();
    const btnId = keyMap[key] || keyMap[keyLower];
    if (!btnId || !keyStates[keyLower]) return;

    keyStates[keyLower] = false;
    document.getElementById(`btn-${btnId}`)?.classList.remove('active');
    sendEvent(btnId, 'up');
});

setInterval(() => {
    fetch(`/stick?id=LS&x=${leftStick.stickX}&y=${leftStick.stickY}`);
    fetch(`/stick?id=RS&x=${rightStick.stickX}&y=${rightStick.stickY}`);
}, 50);

</script></body></html>
)rawliteral";
    server.send(200, "text/html", html);
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Not Found");
}
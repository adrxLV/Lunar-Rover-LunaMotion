// Variables for rover control
let v = 0;  // Linear velocidade
let w = 0;  // Angular velocidade  
let alpha = 110*3.14/180;  // Angulo da camera

// Gamepad variaveis
let gamepadActive = false;
let gamepadIndex = -1;
let lastUpdate = 0;
const UPDATE_INTERVAL = 60; 

// Gamepad configurações
const GAMEPAD_DEAD_ZONE = 0.15;
const TRIGGER_DEAD_ZONE = 0.05;
const TRIGGER_SENSITIVITY = 0.3;
const CAMERA_SENSITIVITY = 1; 
const MAX_LINEAR_VELOCITY = 0.44; // m/s
const MAX_ANGULAR_VELOCITY = 2.0; // rad/s

// Autonomous mode configurações
const MAX_AUTONOMOUS_LINEAR_VELOCITY = 0.3; 
const MAX_AUTONOMOUS_ANGULAR_VELOCITY = 2.0;

// estado atual
let currentLinearVelocity = 0;
let currentAngularVelocity = 0;

// nav autonoma state
let autonomousMode = false;
let flyByWireMode = false;
let flyByWireCorrecting = false; 
let lastSensorData = null;
let previousXButtonState = false;
let previousYButtonState = false;

// WebSocket conecção
let ws = new WebSocket("ws://localhost:8000/ws");

// ===========================================
// WEBSOCKET SETUP
// ===========================================
ws.onmessage = function(event) {
    let messages = document.getElementById('sensors-data')
    let data = JSON.parse(event.data);
    console.log(data);

    
    lastSensorData = data;

    let formattedJson = JSON.stringify(data, null, 2);
    messages.value = formattedJson;
};

ws.onopen = function() {
    console.log('WebSocket connected');
};

ws.onclose = function() {
    console.log('WebSocket disconnected');
    setTimeout(() => {
        ws = new WebSocket("ws://localhost:8000/ws");
        setupWebSocketHandlers();
    }, 3000);
};

function setupWebSocketHandlers() {
    ws.onmessage = function(event) {
        let messages = document.getElementById('sensors-data')
        let data = JSON.parse(event.data);
        console.log(data);

        lastSensorData = data;

        let formattedJson = JSON.stringify(data, null, 2);
        messages.value = formattedJson;
    };
}

// ===========================================
// AUTONOMOUS NAVIGATION FUNCTIONS
// ===========================================
function calculateAutonomousNavigation() {
    if (!lastSensorData || !lastSensorData.ir || !lastSensorData.gyro || !lastSensorData.acc) {
        console.log('No sensor data available for autonomous navigation');
        return { v: 0, w: 0 };
    }

    const m_ir = lastSensorData.ir;
    const m_gyro = lastSensorData.gyro;
    const m_acc = lastSensorData.acc;

    let v_auto = 0;
    let w_auto = 0;

    if (m_ir[0] < 0.2 || m_ir[1] < 0.2 || m_ir[2] < 0.2) {
        v_auto = 0;
        if (m_ir[2] > m_ir[0]) {
            w_auto = MAX_AUTONOMOUS_ANGULAR_VELOCITY;
            v_auto = -MAX_AUTONOMOUS_LINEAR_VELOCITY;
        } else {
            w_auto = -MAX_AUTONOMOUS_ANGULAR_VELOCITY;
            v_auto = -MAX_AUTONOMOUS_LINEAR_VELOCITY;
        }
        console.log(`Obstacle detected - IR: [${m_ir.join(', ')}] - Turning: v=${v_auto}, w=${w_auto}`);
    } else {
        v_auto = MAX_AUTONOMOUS_LINEAR_VELOCITY;
        w_auto = 0;
        console.log(`Clear path - IR: [${m_ir.join(', ')}] - Moving forward: v=${v_auto}, w=${w_auto}`);
    }

    return { v: v_auto, w: w_auto };
}

function toggleAutonomousMode() {
    autonomousMode = !autonomousMode;
    console.log('Autonomous mode:', autonomousMode ? 'ENABLED' : 'DISABLED');
    
    updateAutonomousModeDisplay();
    
    if (!autonomousMode) {
        currentLinearVelocity = 0;
        currentAngularVelocity = 0;
        sendGamepadCommand(0, 0);
        updateSliderDisplays(0, 0);
    } else {
        startAutonomousNavigation();
    }
}

function startAutonomousNavigation() {
    function autonomousLoop() {
        if (!autonomousMode) return;
        
        const autoNav = calculateAutonomousNavigation();
        
        currentLinearVelocity = autoNav.v;
        currentAngularVelocity = autoNav.w;
        
        v = autoNav.v;
        w = autoNav.w;
        
        sendGamepadCommand(autoNav.v, autoNav.w);
        updateSliderDisplays(autoNav.v, autoNav.w);
        
        setTimeout(autonomousLoop, 60);
    }
    
    autonomousLoop();
}

function updateAutonomousModeDisplay() {
    const statusElement = document.getElementById('autonomous-status');
    if (statusElement) {
        if (autonomousMode) {
            statusElement.textContent = 'AUTONOMOUS MODE: ON';
            statusElement.style.color = 'green';
        } else if (flyByWireMode) {
            if (flyByWireCorrecting) {
                statusElement.textContent = 'FLY-BY-WIRE: CORRECTING (AUTO)';
                statusElement.style.color = 'red';
            } else {
                statusElement.textContent = 'FLY-BY-WIRE: MANUAL CONTROL';
                statusElement.style.color = 'orange';
            }
        } else {
            statusElement.textContent = 'MANUAL MODE: ON';
            statusElement.style.color = 'blue';
        }
    }
}

function toggleFlyByWireMode() {
    flyByWireMode = !flyByWireMode;
    console.log('Fly-by-wire mode:', flyByWireMode ? 'ENABLED' : 'DISABLED');
    
    if (flyByWireMode) {
        autonomousMode = false;
        startFlyByWireLoop();
    } else {
        flyByWireCorrecting = false;
    }
    
    updateAutonomousModeDisplay();
    
    if (!flyByWireMode) {
        currentLinearVelocity = 0;
        currentAngularVelocity = 0;
        sendGamepadCommand(0, 0);
        updateSliderDisplays(0, 0);
    }
}

function startFlyByWireLoop() {
    function flyByWireLoop() {
        if (!flyByWireMode) return;
        
        if (lastSensorData && lastSensorData.ir) {
            const m_ir = lastSensorData.ir;
            const obstacleDetected = m_ir[0] < 0.2 || m_ir[1] < 0.2 || m_ir[2] < 0.2;
            
            if (obstacleDetected && !flyByWireCorrecting) {
                flyByWireCorrecting = true;
                updateAutonomousModeDisplay();
                console.log('Fly-by-wire taking control for obstacle avoidance');
            } else if (!obstacleDetected && flyByWireCorrecting) {
                flyByWireCorrecting = false;
                updateAutonomousModeDisplay();
                console.log('Fly-by-wire returning control to pilot');
            }
            
            if (flyByWireCorrecting) {
                const correction = calculateSafetyOverride(0, 0); 
                
                currentLinearVelocity = correction.v;
                currentAngularVelocity = correction.w;
                v = correction.v;
                w = correction.w;
                
                sendGamepadCommand(correction.v, correction.w);
                updateSliderDisplays(correction.v, correction.w);
            }
        }
        
        setTimeout(flyByWireLoop, 60);
    }
    
    flyByWireLoop();
}

function calculateSafetyOverride(manualV, manualW) {
    if (!lastSensorData || !lastSensorData.ir) {
        flyByWireCorrecting = false;
        return { v: manualV, w: manualW };
    }

    const m_ir = lastSensorData.ir;
    
    const obstacleDetected = m_ir[0] < 0.2 || m_ir[1] < 0.2 || m_ir[2] < 0.2;
    
    if (obstacleDetected) {
        flyByWireCorrecting = true;
        console.log(`Fly-by-wire correction active - IR: [${m_ir.join(', ')}] - Manual input ignored`);
        
        let v_correct = 0;
        let w_correct = 0;
        
        if (m_ir[2] > m_ir[0]) {
            w_correct = MAX_AUTONOMOUS_ANGULAR_VELOCITY;
            v_correct = -MAX_AUTONOMOUS_LINEAR_VELOCITY;
        } else {
            w_correct = -MAX_AUTONOMOUS_ANGULAR_VELOCITY;
            v_correct = -MAX_AUTONOMOUS_LINEAR_VELOCITY;
        }
        
        return { v: v_correct, w: w_correct };
    } else {
        if (flyByWireCorrecting) {
            console.log('Fly-by-wire correction complete - Returning control to pilot');
            flyByWireCorrecting = false;
        }
        return { v: manualV, w: manualW };
    }
}

// ===========================================
// COMMAND FUNCTIONS
// ===========================================
function emit_motor_command() {
    const cmd = {
        "v": v,
        "w": w
    };
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(cmd));
    }
}

function emit_tilt_command() {
    const cmd = { alpha };
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(cmd));
    }
}

function sendGamepadCommand(linearVel, angularVel) {
    const cmd = {
        "v": Number(linearVel.toFixed(3)),
        "w": Number(angularVel.toFixed(3))
    };
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(cmd));
    }
}

// ===========================================
// SLIDER CONTROLS SETUP
// ===========================================
function setupSliders() {
    const v_slider = document.getElementById('v_slider');
    const v_valueDisplay = document.getElementById('v_value');

    v_valueDisplay.textContent = Number(v_slider.value).toFixed(2);

    v_slider.oninput = function() {
        if (!gamepadActive) {
            v = Number(this.value);
            v_valueDisplay.textContent = v.toFixed(2);
            emit_motor_command();
        }
    }

    const w_slider = document.getElementById('w_slider');
    const w_valueDisplay = document.getElementById('w_value');

    w_valueDisplay.textContent = Number(w_slider.value).toFixed(2);

    w_slider.oninput = function() {
        if (!gamepadActive) {
            w = Number(this.value);
            w_valueDisplay.textContent = w.toFixed(2);
            emit_motor_command();
        }
    }

    const a_slider = document.getElementById('a_slider');
    const a_valueDisplay = document.getElementById('a_value');

    a_valueDisplay.textContent = Number(a_slider.value).toFixed(2);

    a_slider.oninput = function() {
        if (!gamepadActive) {
            alpha = Number(this.value)*3.14/180;
            a_valueDisplay.textContent = (alpha*180/3.14).toFixed(2);
            emit_tilt_command();
        }
    }
}

// ===========================================
// GAMEPAD SYSTEM
// ===========================================
function initializeGamepadControl() {
    if (!navigator.getGamepads) {
        console.warn('Gamepad API not supported in this browser');
        return;
    }

    window.addEventListener('gamepadconnected', function(e) {
        console.log('Gamepad connected:', e.gamepad.id);
        gamepadIndex = e.gamepad.index;
        gamepadActive = true;
        startGamepadLoop();
    });

    window.addEventListener('gamepaddisconnected', function(e) {
        console.log('Gamepad disconnected');
        gamepadActive = false;
        gamepadIndex = -1;
        
        currentLinearVelocity = 0;
        currentAngularVelocity = 0;
        updateSliderDisplays(0, 0);
        sendGamepadCommand(0, 0);
    });

    const gamepads = navigator.getGamepads();
    for (let i = 0; i < gamepads.length; i++) {
        if (gamepads[i]) {
            gamepadIndex = i;
            gamepadActive = true;
            startGamepadLoop();
            break;
        }
    }
}

function startGamepadLoop() {
    if (gamepadIndex === -1) return;
    
    function gamepadLoop(timestamp) {
        const gamepad = navigator.getGamepads()[gamepadIndex];
        if (!gamepad) {
            gamepadIndex = -1;
            gamepadActive = false;
            return;
        }
        
        if (timestamp - lastUpdate < UPDATE_INTERVAL) {
            requestAnimationFrame(gamepadLoop);
            return;
        }
        lastUpdate = timestamp;
        
        processGamepadInputs(gamepad);
        
        requestAnimationFrame(gamepadLoop);
    }
    
    requestAnimationFrame(gamepadLoop);
}

function processGamepadInputs(gamepad) {
    const xButton = gamepad.buttons[2];
    const currentXButtonState = xButton && xButton.pressed;
    
    const yButton = gamepad.buttons[3];
    const currentYButtonState = yButton && yButton.pressed;
    
    if (currentXButtonState && !previousXButtonState) {
        if (!autonomousMode) {
            flyByWireMode = false;
        }
        toggleAutonomousMode();
    }

    if (currentYButtonState && !previousYButtonState) {
        if (!flyByWireMode) {
            autonomousMode = false;
        }
        toggleFlyByWireMode();
    }
    
    previousXButtonState = currentXButtonState;
    previousYButtonState = currentYButtonState;

    if (autonomousMode) {
        return;
    }
    
    if (flyByWireMode && flyByWireCorrecting) {
        return;
    }

    const leftStickX = gamepad.axes[0] || 0;
    const rightStickY = gamepad.axes[3] || 0; 
    
    let leftTrigger = 0;
    let rightTrigger = 0;
    
    if (gamepad.axes.length > 6) leftTrigger = Math.max(0, gamepad.axes[6]);
    if (gamepad.axes.length > 7) rightTrigger = Math.max(0, gamepad.axes[7]);
    
    if (leftTrigger === 0 && gamepad.buttons[6]) leftTrigger = gamepad.buttons[6].value;
    if (rightTrigger === 0 && gamepad.buttons[7]) rightTrigger = gamepad.buttons[7].value;
    
    const processedStickX = Math.abs(leftStickX) > GAMEPAD_DEAD_ZONE ? leftStickX : 0;
    const processedRightStickY = Math.abs(rightStickY) > GAMEPAD_DEAD_ZONE ? rightStickY : 0;
    const processedLeftTrigger = leftTrigger > TRIGGER_DEAD_ZONE ? leftTrigger : 0;
    const processedRightTrigger = rightTrigger > TRIGGER_DEAD_ZONE ? rightTrigger : 0;
    
    function applySensitivityCurve(value) {
        if (value === 0) return 0;
        return Math.pow(value, 2) * TRIGGER_SENSITIVITY + value * (1 - TRIGGER_SENSITIVITY);
    }
    
    const sensitizedLeftTrigger = applySensitivityCurve(processedLeftTrigger);
    const sensitizedRightTrigger = applySensitivityCurve(processedRightTrigger);
    
    let linearVelocity = 0;
    if (sensitizedRightTrigger > 0) {
        linearVelocity = sensitizedRightTrigger * MAX_LINEAR_VELOCITY;
    } else if (sensitizedLeftTrigger > 0) {
        linearVelocity = -sensitizedLeftTrigger * MAX_LINEAR_VELOCITY;
    }
    
    const angularVelocity = -processedStickX * MAX_ANGULAR_VELOCITY;
    
    if (Math.abs(processedRightStickY) > 0) {
        const sensitizedStickY = processedRightStickY * CAMERA_SENSITIVITY;
        
        const minAngle = 15;
        const maxAngle = 110;
        const stickValue = sensitizedStickY; 
        const normalizedValue = (stickValue + CAMERA_SENSITIVITY) / (2 * CAMERA_SENSITIVITY);
        const clampedValue = Math.max(0, Math.min(1, normalizedValue)); 
        const newAngleDeg = minAngle + clampedValue * (maxAngle - minAngle);
        
        alpha = newAngleDeg * 3.14 / 180;
        
        const a_valueDisplay = document.getElementById('a_value');
        const a_slider = document.getElementById('a_slider');
        if (a_valueDisplay) a_valueDisplay.textContent = newAngleDeg.toFixed(2);
        if (a_slider) {
            a_slider.value = newAngleDeg;
           
            a_slider.dispatchEvent(new Event('input'));
        }
        
        if (typeof window.updateDialsFromExternal === 'function') {
            window.updateDialsFromExternal(undefined, undefined, newAngleDeg);
        }
        
        emit_tilt_command();
    }
    
  
    if (linearVelocity !== 0 && Math.abs(angularVelocity) > 0) {
        const angularFactor = Math.abs(angularVelocity) / MAX_ANGULAR_VELOCITY;
        const reductionFactor = 1 - (angularFactor * 0.5); 
        linearVelocity *= reductionFactor;
    }
    
    let finalVelocities = { v: linearVelocity, w: angularVelocity };
    
    const linearDiff = Math.abs(finalVelocities.v - currentLinearVelocity);
    const angularDiff = Math.abs(finalVelocities.w - currentAngularVelocity);
    
    if (linearDiff > 0.01 || angularDiff > 0.01) {
        currentLinearVelocity = finalVelocities.v;
        currentAngularVelocity = finalVelocities.w;
        
        v = finalVelocities.v;
        w = finalVelocities.w;
        
        sendGamepadCommand(finalVelocities.v, finalVelocities.w);
        updateSliderDisplays(finalVelocities.v, finalVelocities.w);
    }
}

// ===========================================
// DISPLAY UPDATE FUNCTIONS
// ===========================================
function updateSliderDisplays(linearVel, angularVel) {
    const v_valueDisplay = document.getElementById('v_value');
    const w_valueDisplay = document.getElementById('w_value');
    const v_slider = document.getElementById('v_slider');
    const w_slider = document.getElementById('w_slider');
    
    if (v_valueDisplay) v_valueDisplay.textContent = linearVel.toFixed(2);
    if (w_valueDisplay) w_valueDisplay.textContent = angularVel.toFixed(2);
    
    if (v_slider) {
        v_slider.value = linearVel;
        v_slider.dispatchEvent(new Event('input'));
    }
    if (w_slider) {
        w_slider.value = angularVel;
        w_slider.dispatchEvent(new Event('input'));
    }
    
    if (typeof window.updateDialsFromExternal === 'function') {
        window.updateDialsFromExternal(linearVel, angularVel);
    }
}

// ===========================================
// INITIALIZATION
// ===========================================
document.addEventListener('DOMContentLoaded', function() {
    console.log('Rover control system initializing...');
    
    setupSliders();
    
    initializeGamepadControl();
    
    updateAutonomousModeDisplay();
    
    console.log('Rover control system initialized');
    console.log('Press X button on gamepad to toggle autonomous navigation');
    console.log('Press Y button on gamepad to toggle fly-by-wire mode (manual + obstacle avoidance)');
});

/**
 * GAMEPAD ROVER CONTROL SYSTEM
 * Sistema de controle do rover usando Xbox gamepad
 * R/L Triggers para velocidade linear
 * Left Joystick X para velocidade angular
 */

// ===========================================
// CONFIGURAÇÃO DE VARIÁVEIS DO ROVER
// ===========================================
let gamepadActive = false;
let gamepadIndex = -1;
let ws = null;
let wsConnected = false;

// Configurações do gamepad
const GAMEPAD_DEAD_ZONE = 0.15;
const TRIGGER_DEAD_ZONE = 0.05;
const TRIGGER_SENSITIVITY = 0.3; // Fator de sensibilidade dos triggers (0.1 = baixa, 1.0 = alta)
const MAX_LINEAR_VELOCITY = 0.44; // m/s
const MAX_ANGULAR_VELOCITY = 2.0; // rad/s

// Estado atual do rover
let currentLinearVelocity = 0;
let currentAngularVelocity = 0;

// Controle de atualização
let lastUpdate = 0;
const UPDATE_INTERVAL = 50; // 20 Hz

// Log de comandos
let commandsLog = [];
const MAX_LOG_ENTRIES = 50;

// ===========================================
// INICIALIZAÇÃO
// ===========================================
document.addEventListener('DOMContentLoaded', function() {
    console.log('[Gamepad Rover] Inicializando sistema de controle...');
    
    // Inicializar WebSocket
    initializeWebSocket();
    
    // Inicializar controle do gamepad
    initializeGamepadControl();
    
    // Atualizar data/hora
    updateDateTime();
    setInterval(updateDateTime, 1000);
    
    // Configurar botões
    setupEventListeners();
    
    console.log('[Gamepad Rover] Sistema inicializado');
});

// ===========================================
// WEBSOCKET
// ===========================================
function initializeWebSocket() {
    try {
        ws = new WebSocket("ws://localhost:8000/ws");
        
        ws.onopen = function() {
            console.log('[WebSocket] Conectado ao servidor');
            wsConnected = true;
            updateWSStatus(true);
        };
        
        ws.onmessage = function(event) {
            try {
                let data = JSON.parse(event.data);
                console.log('[WebSocket] Dados recebidos:', data);
                updateSensorData(data);
            } catch (error) {
                console.error('[WebSocket] Erro ao processar dados:', error);
            }
        };
        
        ws.onclose = function() {
            console.log('[WebSocket] Conexão fechada');
            wsConnected = false;
            updateWSStatus(false);
            
            // Tentar reconectar após 3 segundos
            setTimeout(() => {
                console.log('[WebSocket] Tentando reconectar...');
                initializeWebSocket();
            }, 3000);
        };
        
        ws.onerror = function(error) {
            console.error('[WebSocket] Erro:', error);
            wsConnected = false;
            updateWSStatus(false);
        };
        
    } catch (error) {
        console.error('[WebSocket] Erro ao inicializar:', error);
        updateWSStatus(false);
    }
}

function sendCommand(v, w) {
    if (ws && wsConnected) {
        const cmd = {
            "v": Number(v.toFixed(3)),
            "w": Number(w.toFixed(3))
        };
        
        try {
            ws.send(JSON.stringify(cmd));
            logCommand(cmd);
        } catch (error) {
            console.error('[WebSocket] Erro ao enviar comando:', error);
        }
    }
}

// ===========================================
// SISTEMA DE GAMEPAD
// ===========================================
function initializeGamepadControl() {
    if (!navigator.getGamepads) {
        console.warn('[Gamepad] API não suportada neste navegador');
        updateGamepadStatus(false, 'API não suportada');
        return;
    }
    
    // Event listeners para conexão/desconexão
    window.addEventListener('gamepadconnected', function(e) {
        console.log(`[Gamepad] Conectado: ${e.gamepad.id}`);
        gamepadIndex = e.gamepad.index;
        gamepadActive = true;
        updateGamepadStatus(true, e.gamepad.id);
        startGamepadLoop();
    });
    
    window.addEventListener('gamepaddisconnected', function(e) {
        console.log(`[Gamepad] Desconectado: ${e.gamepad.id}`);
        if (e.gamepad.index === gamepadIndex) {
            gamepadIndex = -1;
            gamepadActive = false;
            updateGamepadStatus(false, 'Desconectado');
            // Parar o rover quando o gamepad é desconectado
            sendCommand(0, 0);
            updateVelocityDisplays(0, 0);
        }
    });
    
    // Verificar se já existe um gamepad conectado
    const gamepads = navigator.getGamepads();
    for (let i = 0; i < gamepads.length; i++) {
        if (gamepads[i]) {
            console.log(`[Gamepad] Detectado: ${gamepads[i].id}`);
            gamepadIndex = i;
            gamepadActive = true;
            updateGamepadStatus(true, gamepads[i].id);
            startGamepadLoop();
            break;
        }
    }
    
    if (gamepadIndex === -1) {
        updateGamepadStatus(false, 'Nenhum gamepad detectado');
    }
}

function startGamepadLoop() {
    if (gamepadIndex === -1) return;
    
    function gamepadLoop(timestamp) {
        const gamepad = navigator.getGamepads()[gamepadIndex];
        if (!gamepad) {
            gamepadIndex = -1;
            gamepadActive = false;
            updateGamepadStatus(false, 'Gamepad perdido');
            return;
        }
        
        // Controlar frequência de atualização
        if (timestamp - lastUpdate < UPDATE_INTERVAL) {
            requestAnimationFrame(gamepadLoop);
            return;
        }
        lastUpdate = timestamp;
        
        // Processar inputs do gamepad
        processGamepadInputs(gamepad);
        
        // Continuar o loop
        requestAnimationFrame(gamepadLoop);
    }
    
    requestAnimationFrame(gamepadLoop);
}

function processGamepadInputs(gamepad) {
    // Left joystick X para velocidade angular
    const leftStickX = gamepad.axes[0] || 0;
    
    // Triggers para velocidade linear
    let leftTrigger = 0;
    let rightTrigger = 0;
    
    // Tentar obter valores dos triggers (diferentes navegadores/gamepads podem usar métodos diferentes)
    if (gamepad.axes.length > 6) leftTrigger = Math.max(0, gamepad.axes[6]);
    if (gamepad.axes.length > 7) rightTrigger = Math.max(0, gamepad.axes[7]);
    
    // Fallback para botões se os eixos não estiverem disponíveis
    if (leftTrigger === 0 && gamepad.buttons[6]) leftTrigger = gamepad.buttons[6].value;
    if (rightTrigger === 0 && gamepad.buttons[7]) rightTrigger = gamepad.buttons[7].value;
    
    // Aplicar dead zones
    const processedStickX = Math.abs(leftStickX) > GAMEPAD_DEAD_ZONE ? leftStickX : 0;
    const processedLeftTrigger = leftTrigger > TRIGGER_DEAD_ZONE ? leftTrigger : 0;
    const processedRightTrigger = rightTrigger > TRIGGER_DEAD_ZONE ? rightTrigger : 0;
    
    // Aplicar curva de sensibilidade aos triggers (para controle mais suave)
    function applySensitivityCurve(value) {
        if (value === 0) return 0;
        // Curva quadrática para sensibilidade mais baixa no início
        return Math.pow(value, 2) * TRIGGER_SENSITIVITY + value * (1 - TRIGGER_SENSITIVITY);
    }
    
    const sensitizedLeftTrigger = applySensitivityCurve(processedLeftTrigger);
    const sensitizedRightTrigger = applySensitivityCurve(processedRightTrigger);
    
    // Calcular velocidades
    // Velocidade linear: R trigger = frente, L trigger = trás
    let linearVelocity = 0;
    if (sensitizedRightTrigger > 0) {
        linearVelocity = sensitizedRightTrigger * MAX_LINEAR_VELOCITY;
    } else if (sensitizedLeftTrigger > 0) {
        linearVelocity = -sensitizedLeftTrigger * MAX_LINEAR_VELOCITY;
    }
    
    // Velocidade angular: Left stick X
    const angularVelocity = -processedStickX * MAX_ANGULAR_VELOCITY; // Negativo para inversão intuitiva
    
    // Verificar se houve mudança significativa
    const linearDiff = Math.abs(linearVelocity - currentLinearVelocity);
    const angularDiff = Math.abs(angularVelocity - currentAngularVelocity);
    
    if (linearDiff > 0.01 || angularDiff > 0.01) {
        currentLinearVelocity = linearVelocity;
        currentAngularVelocity = angularVelocity;
        
        // Enviar comando para o rover
        sendCommand(linearVelocity, angularVelocity);
        
        // Atualizar displays
        updateVelocityDisplays(linearVelocity, angularVelocity);
    }
}

// ===========================================
// INTERFACE DE USUÁRIO
// ===========================================
function updateGamepadStatus(connected, info = '') {
    const indicator = document.getElementById('gamepad-indicator');
    const dot = document.getElementById('gamepad-dot');
    const text = document.getElementById('gamepad-status-text');
    
    if (connected) {
        dot.className = 'status-dot connected';
        text.textContent = `Connected: ${info}`;
        indicator.className = 'status-indicator connected';
    } else {
        dot.className = 'status-dot disconnected';
        text.textContent = info || 'Disconnected';
        indicator.className = 'status-indicator disconnected';
    }
}

function updateWSStatus(connected) {
    const dot = document.getElementById('ws-dot');
    const text = document.getElementById('ws-status-text');
    const status = document.getElementById('ws-status');
    
    if (connected) {
        dot.className = 'status-dot connected';
        text.textContent = 'Connected';
        status.className = 'connection-status connected';
    } else {
        dot.className = 'status-dot disconnected';
        text.textContent = 'Disconnected';
        status.className = 'connection-status disconnected';
    }
}

function updateVelocityDisplays(linear, angular) {
    const linearDisplay = document.getElementById('linear-velocity-display');
    const angularDisplay = document.getElementById('angular-velocity-display');
    
    linearDisplay.textContent = `${linear.toFixed(2)} m/s`;
    angularDisplay.textContent = `${angular.toFixed(2)} rad/s`;
}

function updateSensorData(data) {
    const sensorTextarea = document.getElementById('sensors-data');
    const formattedJson = JSON.stringify(data, null, 2);
    sensorTextarea.value = formattedJson;
}

function updateDateTime() {
    const now = new Date();
    const dateStr = now.toLocaleDateString('en-GB', {
        day: '2-digit',
        month: '2-digit',
        year: 'numeric'
    }).replace(/\//g, ' / ');
    
    const timeStr = now.toLocaleTimeString('en-GB', {
        hour: '2-digit',
        minute: '2-digit',
        hour12: false
    });
    
    const datetimeElement = document.getElementById('current-datetime');
    datetimeElement.textContent = `DATE: ${dateStr} | ${timeStr} GMT+0`;
}

function logCommand(cmd) {
    const timestamp = new Date().toLocaleTimeString();
    const logEntry = {
        time: timestamp,
        command: cmd
    };
    
    commandsLog.unshift(logEntry);
    if (commandsLog.length > MAX_LOG_ENTRIES) {
        commandsLog.pop();
    }
    
    updateCommandsLog();
}

function updateCommandsLog() {
    const logContainer = document.getElementById('commands-log');
    logContainer.innerHTML = '';
    
    commandsLog.forEach(entry => {
        const logItem = document.createElement('div');
        logItem.className = 'log-item';
        logItem.innerHTML = `
            <span class="log-time">[${entry.time}]</span>
            <span class="log-command">v: ${entry.command.v}, w: ${entry.command.w}</span>
        `;
        logContainer.appendChild(logItem);
    });
}

function setupEventListeners() {
    // Botão de emergência
    const emergencyBtn = document.getElementById('emergency-stop');
    emergencyBtn.addEventListener('click', function() {
        console.log('[Emergency] Parada de emergência ativada');
        sendCommand(0, 0);
        updateVelocityDisplays(0, 0);
        currentLinearVelocity = 0;
        currentAngularVelocity = 0;
        
        // Adicionar feedback visual
        emergencyBtn.classList.add('active');
        setTimeout(() => {
            emergencyBtn.classList.remove('active');
        }, 500);
    });
    
    // Botão de limpar log
    const clearLogBtn = document.getElementById('clear-log-btn');
    clearLogBtn.addEventListener('click', function() {
        commandsLog = [];
        updateCommandsLog();
    });
}

// ===========================================
// TRATAMENTO DE ERROS
// ===========================================
window.addEventListener('error', function(event) {
    console.error('[System Error]', event.error);
});

window.addEventListener('unhandledrejection', function(event) {
    console.error('[Unhandled Promise Rejection]', event.reason);
});

// ===========================================
// CLEANUP
// ===========================================
window.addEventListener('beforeunload', function() {
    if (ws && wsConnected) {
        // Parar o rover antes de fechar a página
        sendCommand(0, 0);
        ws.close();
    }
});

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

// Atualização
let lastUpdate = 0;
const UPDATE_INTERVAL = 60; 

// Log de comandos
let commandsLog = [];
const MAX_LOG_ENTRIES = 50;

// Variáveis do stream da câmera
let streamActive = false;
let roverIp = '';
let streamUrl = '';

// ===========================================
// INICIALIZAÇÃO
// ===========================================
document.addEventListener('DOMContentLoaded', function() {
    console.log('[Gamepad Rover] Inicializando sistema de controle...');
    
    // Iniciar WebSocket
    initializeWebSocket();
    
    // Iniciar controle do gamepad
    initializeGamepadControl();
    
    // Iniciar data/hora
    updateDateTime();
    setInterval(updateDateTime, 1000);
    
    // Configurar botões
    setupEventListeners();
    
    // Iniciar stream da câmera automaticamente
    setTimeout(() => {
        startCameraStream('lunar-rover-0.eec');
    }, 1000); // Aguardar 1 segundo para garantir que a página carregou completamente
    
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
            sendCommand(0, 0);
            updateVelocityDisplays(0, 0);
        }
    });
    
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
    const leftStickX = gamepad.axes[0] || 0;
    
    let leftTrigger = 0;
    let rightTrigger = 0;
    
    if (gamepad.axes.length > 6) leftTrigger = Math.max(0, gamepad.axes[6]);
    if (gamepad.axes.length > 7) rightTrigger = Math.max(0, gamepad.axes[7]);
    
    if (leftTrigger === 0 && gamepad.buttons[6]) leftTrigger = gamepad.buttons[6].value;
    if (rightTrigger === 0 && gamepad.buttons[7]) rightTrigger = gamepad.buttons[7].value;
    
    const processedStickX = Math.abs(leftStickX) > GAMEPAD_DEAD_ZONE ? leftStickX : 0;
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
    
    // Reduzir velocidade linear baseada na velocidade angular
    // Quanto maior a velocidade angular, menor a velocidade linear (mínimo 50%)
    if (linearVelocity !== 0 && Math.abs(angularVelocity) > 0) {
        const angularFactor = Math.abs(angularVelocity) / MAX_ANGULAR_VELOCITY; // 0 a 1
        const reductionFactor = 1 - (angularFactor * 0.5); // 1 a 0.5 (50% redução máxima)
        linearVelocity *= reductionFactor;
    } 
    
    const linearDiff = Math.abs(linearVelocity - currentLinearVelocity);
    const angularDiff = Math.abs(angularVelocity - currentAngularVelocity);
    
    if (linearDiff > 0.01 || angularDiff > 0.01) {
        currentLinearVelocity = linearVelocity;
        currentAngularVelocity = angularVelocity;
        
        sendCommand(linearVelocity, angularVelocity);
        
        updateVelocityDisplays(linearVelocity, angularVelocity);
    }
}

// ===========================================
// INTERFACE DO UTILIZADOR
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
    // Botão de limpar log
    const clearLogBtn = document.getElementById('clear-log-btn');
    clearLogBtn.addEventListener('click', function() {
        commandsLog = [];
        updateCommandsLog();
    });
    
    // Botão de parar stream da câmera
    const stopStreamBtn = document.getElementById('stop-stream-btn');
    stopStreamBtn.addEventListener('click', function() {
        stopCameraStream();
    });
}

// ===========================================
// CONTROLE DO STREAM DA CÂMERA
// ===========================================
function startCameraStream(ip) {
    console.log(`[Camera] Iniciando stream da câmera para IP: ${ip}`);
    
    const video = document.getElementById('rover-stream');
    const overlay = document.getElementById('stream-overlay');
    const statusText = document.getElementById('stream-status-text');
    const stopBtn = document.getElementById('stop-stream-btn');
    
    // Validar IP/domínio
    if (!ip) {
        console.error('[Camera] IP/domínio não fornecido');
        updateStreamStatus('Erro: IP/domínio inválido');
        return;
    }
    
    roverIp = ip;
    streamUrl = `rtsp://${ip}:8554/stream`;
    
    // Atualizar status
    updateStreamStatus('Conectando...');
    
    try {
        // Para streams RTSP em navegadores, precisamos usar uma solução alternativa
        // Como o navegador não suporta RTSP nativamente, vamos tentar usar HLS ou WebRTC
        
        // Primeiro, tentar conectar via WebSocket para converter RTSP em WebRTC
        const wsStreamUrl = `ws://${ip}:8080/stream`; // Assumindo que há um servidor WebSocket para conversão
        
        // Como alternativa, vamos criar um proxy HTTP para o stream
        const httpStreamUrl = `http://${ip}:8080/stream.m3u8`; // Stream HLS
        
        // Tentar configurar o stream
        video.src = httpStreamUrl;
        video.load();
        
        video.onloadstart = function() {
            console.log('[Camera] Carregando stream...');
            updateStreamStatus('Carregando stream...');
        };
        
        video.onloadeddata = function() {
            console.log('[Camera] Stream carregado com sucesso');
            streamActive = true;
            overlay.classList.add('hidden');
            stopBtn.disabled = false;
            updateStreamStatus('Stream ativo');
        };
        
        video.onerror = function(e) {
            console.error('[Camera] Erro ao carregar stream:', e);
            streamActive = false;
            overlay.classList.remove('hidden');
            stopBtn.disabled = true;
            updateStreamStatus('Erro: Não foi possível conectar ao stream. Verifique se o rover está ligado e acessível.');
            
            // Tentar método alternativo - usar mjpeg
            console.log('[Camera] Tentando MJPEG stream...');
            const mjpegUrl = `http://${ip}:8080/stream.mjpg`;
            video.src = mjpegUrl;
        };
        
        video.onplay = function() {
            console.log('[Camera] Stream reproduzindo');
        };
        
    } catch (error) {
        console.error('[Camera] Erro ao inicializar stream:', error);
        streamActive = false;
        overlay.classList.remove('hidden');
        stopBtn.disabled = true;
        updateStreamStatus('Erro: Falha ao inicializar stream');
    }
}

function stopCameraStream() {
    console.log('[Camera] Parando stream da câmera');
    
    const video = document.getElementById('rover-stream');
    const overlay = document.getElementById('stream-overlay');
    const stopBtn = document.getElementById('stop-stream-btn');
    
    // Parar o vídeo
    video.pause();
    video.src = '';
    video.load();
    
    // Resetar estado
    streamActive = false;
    overlay.classList.remove('hidden');
    stopBtn.disabled = true;
    
    updateStreamStatus('Stream parado');
    
    console.log('[Camera] Stream parado com sucesso');
}

function updateStreamStatus(message) {
    const statusText = document.getElementById('stream-status-text');
    if (statusText) {
        statusText.textContent = message;
    }
    console.log(`[Camera Status] ${message}`);
}

// ===========================================
// ERROS
// ===========================================
window.addEventListener('error', function(event) {
    console.error('[System Error]', event.error);
});

window.addEventListener('unhandledrejection', function(event) {
    console.error('[Unhandled Promise Rejection]', event.reason);
});

// ===========================================
// PARAR ANTES DE FECHAR A PAG
// ===========================================
window.addEventListener('beforeunload', function() {
    if (ws && wsConnected) {
        sendCommand(0, 0);
        ws.close();
    }
});

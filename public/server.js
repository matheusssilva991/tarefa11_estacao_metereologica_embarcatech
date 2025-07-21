const express = require('express');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = 3000;

// Middleware
app.use(express.json());
app.use(express.static(path.join(__dirname)));

// Simulação de dados dos sensores (substituir por dados reais)
let sensorData = {
    temperature: 25.5,
    humidity: 65.2,
    pressure: 1013.25,
    altitude: 850,
    lastUpdate: new Date()
};

// Configurações persistentes
let config = {
    minTemperature: 10,
    maxTemperature: 70,
    tempOffset: 0
};

// Função para simular variação dos sensores
function updateSensorData() {
    // Simula pequenas variações nos dados
    sensorData.temperature += (Math.random() - 0.5) * 2;
    sensorData.humidity += (Math.random() - 0.5) * 5;
    sensorData.pressure += (Math.random() - 0.5) * 2;
    sensorData.altitude = 44330 * (1 - Math.pow(sensorData.pressure / 1013.25, 0.1903));
    sensorData.lastUpdate = new Date();

    // Mantém valores dentro de faixas realistas
    sensorData.temperature = Math.max(15, Math.min(40, sensorData.temperature));
    sensorData.humidity = Math.max(30, Math.min(90, sensorData.humidity));
    sensorData.pressure = Math.max(950, Math.min(1050, sensorData.pressure));
}

// Atualiza dados dos sensores a cada 2 segundos
setInterval(updateSensorData, 2000);

// Rota principal - serve o index.html
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// API - Dados dos sensores
app.get('/api/weather', (req, res) => {
    try {
        const responseData = {
            temperature: parseFloat(sensorData.temperature.toFixed(1)),
            humidity: parseFloat(sensorData.humidity.toFixed(1)),
            pressure: parseFloat(sensorData.pressure.toFixed(2)),
            altitude: parseFloat(sensorData.altitude.toFixed(0)),
            minTemperature: config.minTemperature,
            maxTemperature: config.maxTemperature,
            tempOffset: config.tempOffset,
            timestamp: sensorData.lastUpdate.toISOString()
        };

        console.log(`[${new Date().toLocaleTimeString()}] Dados enviados:`, responseData);
        res.json(responseData);
    } catch (error) {
        console.error('Erro ao buscar dados dos sensores:', error);
        res.status(500).json({ error: 'Erro interno do servidor' });
    }
});

// API - Salvar configurações (limites e offset)
app.post('/api/limits', (req, res) => {
    try {
        const { min, max, offset } = req.body;

        // Validações
        if (typeof min !== 'number' || typeof max !== 'number' || typeof offset !== 'number') {
            return res.status(400).json({ error: 'Parâmetros inválidos' });
        }

        if (min >= max) {
            return res.status(400).json({ error: 'Temperatura mínima deve ser menor que máxima' });
        }

        if (offset < -10 || offset > 10) {
            return res.status(400).json({ error: 'Offset deve estar entre -10°C e +10°C' });
        }

        // Atualiza configurações
        config.minTemperature = min;
        config.maxTemperature = max;
        config.tempOffset = offset;

        // Salva configurações em arquivo (opcional)
        saveConfig();

        console.log(`[${new Date().toLocaleTimeString()}] Configurações atualizadas:`, config);
        res.json({
            success: true,
            message: 'Configurações salvas com sucesso',
            config: config
        });
    } catch (error) {
        console.error('Erro ao salvar configurações:', error);
        res.status(500).json({ error: 'Erro ao salvar configurações' });
    }
});

// API - Status do sistema
app.get('/api/status', (req, res) => {
    res.json({
        status: 'online',
        uptime: process.uptime(),
        timestamp: new Date().toISOString(),
        version: '1.0.0'
    });
});

// API - Histórico de dados (para expansão futura)
app.get('/api/history', (req, res) => {
    const hours = parseInt(req.query.hours) || 24;

    // Simula dados históricos (substituir por dados reais do banco)
    const history = [];
    const now = new Date();

    for (let i = hours; i >= 0; i--) {
        const timestamp = new Date(now.getTime() - (i * 60 * 60 * 1000));
        history.push({
            timestamp: timestamp.toISOString(),
            temperature: 25 + Math.sin(i / 4) * 5 + (Math.random() - 0.5) * 2,
            humidity: 65 + Math.cos(i / 3) * 15 + (Math.random() - 0.5) * 5,
            pressure: 1013 + Math.sin(i / 6) * 10 + (Math.random() - 0.5) * 2
        });
    }

    res.json(history);
});

// Função para salvar configurações em arquivo
function saveConfig() {
    try {
        fs.writeFileSync('config.json', JSON.stringify(config, null, 2));
    } catch (error) {
        console.error('Erro ao salvar arquivo de configuração:', error);
    }
}

// Função para carregar configurações do arquivo
function loadConfig() {
    try {
        if (fs.existsSync('config.json')) {
            const data = fs.readFileSync('config.json', 'utf8');
            config = { ...config, ...JSON.parse(data) };
            console.log('Configurações carregadas:', config);
        }
    } catch (error) {
        console.error('Erro ao carregar arquivo de configuração:', error);
    }
}

// Middleware de tratamento de erros
app.use((err, req, res, next) => {
    console.error('Erro:', err.stack);
    res.status(500).json({ error: 'Algo deu errado!' });
});

// Middleware para rotas não encontradas
app.use((req, res) => {
    res.status(404).json({ error: 'Rota não encontrada' });
});

// Carrega configurações na inicialização
loadConfig();

// Inicia o servidor
app.listen(PORT, () => {
    console.log(`🌡️  Servidor da Estação Meteorológica iniciado!`);
    console.log(`📡 Endereço: http://localhost:${PORT}`);
    console.log(`📊 API: http://localhost:${PORT}/api/weather`);
    console.log(`⚙️  Configurações: http://localhost:${PORT}/api/limits`);
    console.log(`📈 Status: http://localhost:${PORT}/api/status`);
    console.log(`📜 Logs serão exibidos em tempo real...`);
    console.log('─'.repeat(50));
});

// Graceful shutdown
process.on('SIGTERM', () => {
    console.log('\n🔌 Desligando servidor...');
    process.exit(0);
});

process.on('SIGINT', () => {
    console.log('\n🔌 Servidor interrompido pelo usuário');
    process.exit(0);
});

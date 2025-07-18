#ifndef HTML_DATA_H
#define HTML_DATA_H

const char *html_data =
"<!DOCTYPE html>"
"<html lang='pt-BR'>"
"<head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
"<title>Estação Meteorológica - Itabuna, BA</title>"
"<script src='https://cdn.tailwindcss.com'></script>"
"<script src='https://cdn.jsdelivr.net/npm/apexcharts'></script>"
"<link rel='preconnect' href='https://fonts.googleapis.com'>"
"<link rel='preconnect' href='https://fonts.gstatic.com' crossorigin>"
"<link href='https://fonts.googleapis.com/css2?family=Inter:wght@400;500;700&display=swap' rel='stylesheet'>"
"<style>"
"body{font-family:'Inter',sans-serif;background-color:#111827;color:#F9FAFB;}"
".data-card{background-color:#1F2937;border-radius:0.75rem;padding:1.5rem;border:1px solid #374151;transition:transform 0.2s ease-in-out,box-shadow 0.2s ease-in-out;}"
".data-card:hover{transform:translateY(-5px);box-shadow:0 10px 15px -3px rgba(0,0,0,0.2),0 4px 6px -2px rgba(0,0,0,0.1);}"
".chart-btn{padding:0.5rem 1rem;border-radius:0.5rem;background-color:#374151;color:#F9FAFB;font-weight:500;transition:background-color 0.2s;}"
".chart-btn.active,.chart-btn:hover{background-color:#4F46E5;}"
"</style>"
"</head>"
"<body class='p-4 md:p-8'>"
"<div class='max-w-7xl mx-auto'>"
"<header class='text-center mb-8'>"
"<h1 class='text-4xl md:text-5xl font-bold text-white'>Estação Meteorológica</h1>"
"<p class='text-lg text-gray-400 mt-2'>📍 Itabuna, Bahia</p>"
"<div class='mt-4 flex items-center justify-center space-x-2'>"
"<div class='w-3 h-3 bg-green-500 rounded-full animate-pulse'></div>"
"<span id='status-text' class='text-green-400 font-medium'>Online</span>"
"<span class='text-gray-500'>•</span>"
"<span id='last-update' class='text-gray-400'>Última atualização: --:--:--</span>"
"</div>"
"</header>"
"<section class='mb-8'>"
"<h2 class='text-2xl font-bold text-white mb-4'>Condições Atuais</h2>"
"<div class='grid grid-cols-1 md:grid-cols-3 gap-6'>"
"<div class='data-card text-center'>"
"<p class='text-lg font-medium text-gray-300'>Temperatura</p>"
"<p id='temp-value' class='text-6xl font-bold text-amber-400 my-4'>--.- °C</p>"
"<p class='text-gray-400'>🌡️</p>"
"</div>"
"<div class='data-card text-center'>"
"<p class='text-lg font-medium text-gray-300'>Umidade</p>"
"<p id='humidity-value' class='text-6xl font-bold text-sky-400 my-4'>-- %</p>"
"<p class='text-gray-400'>💧</p>"
"</div>"
"<div class='data-card text-center'>"
"<p class='text-lg font-medium text-gray-300'>Pressão Atmosférica</p>"
"<p id='pressure-value' class='text-6xl font-bold text-violet-400 my-4'>---- hPa</p>"
"<p class='text-gray-400'>💨</p>"
"</div>"
"</div>"
"</section>"
"<section class='mb-8 data-card'>"
"<div class='flex flex-col md:flex-row justify-between items-start md:items-center mb-4'>"
"<h2 class='text-2xl font-bold text-white mb-4 md:mb-0'>Histórico das Últimas Horas</h2>"
"<div id='chart-controls' class='flex space-x-2'>"
"<button class='chart-btn active' data-metric='temp'>Temperatura</button>"
"<button class='chart-btn' data-metric='humidity'>Umidade</button>"
"<button class='chart-btn' data-metric='pressure'>Pressão</button>"
"</div>"
"</div>"
"<div id='chart'></div>"
"</section>"
"<section>"
"<h2 class='text-2xl font-bold text-white mb-4'>Informações Adicionais</h2>"
"<div class='grid grid-cols-1 md:grid-cols-2 gap-6'>"
"<div class='data-card text-center'>"
"<p class='text-lg font-medium text-gray-300'>Altitude Estimada</p>"
"<p id='altitude-value' class='text-6xl font-bold text-emerald-400 my-4'>---- m</p>"
"<p class='text-gray-400'>⛰️ Baseada na pressão atual</p>"
"</div>"
"<div class='data-card'>"
"<h3 class='text-lg font-medium text-gray-300 mb-4'>Detalhes dos Sensores</h3>"
"<ul class='space-y-2 text-gray-400'>"
"<li><strong>Pressão/Temp:</strong> BMP280</li>"
"<li><strong>Umidade/Temp:</strong> AHT20</li>"
"<li><strong>Plataforma:</strong> Raspberry Pi Pico W</li>"
"</ul>"
"</div>"
"</div>"
"</section>"
"<footer class='text-center mt-12 text-gray-500'>"
"<p>&copy; 2025 - Projeto de Estação Meteorológica</p>"
"</footer>"
"</div>"
"<script>"
"let currentMetric='temp';"
"let chartData={"
"temp:Array(20).fill(0),"
"humidity:Array(20).fill(0),"
"pressure:Array(20).fill(0),"
"categories:Array(20).fill(null).map((_,i)=>{"
"const d=new Date();"
"d.setSeconds(d.getSeconds()-(20-i)*5);"
"return d.toLocaleTimeString('pt-BR',{hour:'2-digit',minute:'2-digit',second:'2-digit'});"
"})"
"};"
"let state={maxLimit:70,minLimit:10};"
"const chartOptions={"
"series:[{name:'Temperatura',data:chartData.temp}],"
"chart:{height:350,type:'area',toolbar:{show:false},zoom:{enabled:false},animations:{enabled:true,easing:'linear',dynamicAnimation:{speed:1000}}},"
"dataLabels:{enabled:false},"
"stroke:{curve:'smooth',width:3},"
"xaxis:{categories:chartData.categories,labels:{style:{colors:'#9CA3AF'}}},"
"yaxis:{labels:{style:{colors:'#9CA3AF'},formatter:(val)=>val.toFixed(1)}},"
"tooltip:{theme:'dark',x:{format:'HH:mm:ss'}},"
"grid:{borderColor:'#374151',strokeDashArray:5},"
"colors:['#FBBF24']"
"};"
"const chart=new ApexCharts(document.querySelector('#chart'),chartOptions);"
"chart.render();"
"function calculateAltitude(pressureHpa){"
"return 44330.0*(1.0-Math.pow(pressureHpa/1013.25,0.1903));"
"}"
"async function updateData(){"
"try{"
"const r=await fetch('/api/weather');"
"const d=await r.json();"
"const newTemp=d.temperature;"
"const newHumidity=d.humidity;"
"const newPressure=d.pressure;"
"const newAltitude=d.altitude;"
"if(d.maxTemperature!==undefined){state.maxLimit=d.maxTemperature;}"
"if(d.minTemperature!==undefined){state.minLimit=d.minTemperature;}"
"document.getElementById('temp-value').textContent=`${newTemp} °C`;"
"document.getElementById('humidity-value').textContent=`${Math.round(newHumidity)} %`;"
"document.getElementById('pressure-value').textContent=`${Math.round(newPressure)} hPa`;"
"document.getElementById('altitude-value').textContent=`${Math.round(newAltitude)} m`;"
"const now=new Date();"
"document.getElementById('last-update').textContent=`Última atualização: ${now.toLocaleTimeString('pt-BR')}`;"
"chartData.temp.push(newTemp);"
"chartData.humidity.push(newHumidity);"
"chartData.pressure.push(newPressure);"
"chartData.categories.push(now.toLocaleTimeString('pt-BR',{hour:'2-digit',minute:'2-digit',second:'2-digit'}));"
"if(chartData.temp.length>20){"
"chartData.temp.shift();"
"chartData.humidity.shift();"
"chartData.pressure.shift();"
"chartData.categories.shift();"
"}"
"updateChartSeries();"
"}catch(e){"
"console.error('Erro:',e);"
"}"
"}"
"function updateChartSeries(){"
"let seriesName='';"
"let data=[];"
"let color='';"
"switch(currentMetric){"
"case 'humidity':"
"seriesName='Umidade';"
"data=chartData.humidity;"
"color='#38BDF8';"
"break;"
"case 'pressure':"
"seriesName='Pressão';"
"data=chartData.pressure;"
"color='#A78BFA';"
"break;"
"default:"
"seriesName='Temperatura';"
"data=chartData.temp;"
"color='#FBBF24';"
"break;"
"}"
"chart.updateOptions({xaxis:{categories:chartData.categories},colors:[color]});"
"chart.updateSeries([{name:seriesName,data:data}]);"
"}"
"document.getElementById('chart-controls').addEventListener('click',(e)=>{"
"if(e.target.tagName==='BUTTON'){"
"document.querySelectorAll('.chart-btn').forEach(btn=>btn.classList.remove('active'));"
"e.target.classList.add('active');"
"currentMetric=e.target.dataset.metric;"
"updateChartSeries();"
"}"
"});"
"setInterval(updateData,1000);"
"updateData();"
"</script>"
"</body>"
"</html>";

#endif // HTML_DATA_H

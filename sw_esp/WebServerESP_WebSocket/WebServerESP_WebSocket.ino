#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Ticker.h>

Ticker timer;
#ifndef STASSID
#define STASSID "SSID do seu roteador"
#define STAPSK  "senha do seu roteador"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

char webpage[] PROGMEM = R"=====(
    <!DOCTYPE html>
    <html lang="pt">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.6.1/dist/css/bootstrap.min.css">
        <script src="https://cdn.jsdelivr.net/npm/jquery@3.5.1/dist/jquery.slim.min.js"></script>
        <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.1/dist/umd/popper.min.js"></script>
        <script src="https://cdn.jsdelivr.net/npm/bootstrap@4.6.1/dist/js/bootstrap.bundle.min.js"></script>
        <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>  
        <title>betelInData</title>
        <style>
            *{
                padding: 0;
                margin: 0;
                box-sizing: border-box;

            }

            h1, h2, h3, h4, h5, h6{
                color:rgb(150, 150, 150) 
            }
            
            .row {
                /*margin-left: -15px;*/
                /*margin-right: 1px;*/
                margin: auto;
            }

            .div .col{
                background-color: rgb(20, 20, 20);
            }

            body{
                color: rgb(110,110,110);
                background: rgb(40,40,40);
                
            }
        </style>
    </head>
    
    <body onload="javascript:init()">
        
        <div class="jumbotron text-center" style="background-color: rgb(20,20,20); margin-bottom:0; padding: 10px; font-family: 'Exo 2', sans-serif;">
            <h1>Projeto BetelInData</h1>
            <p>Versão <span style='color: rgb(20,20,20)'>1.0</span></p>    
        </div>
        
        <!--Área de plotagem do gráfico-->
        <div class="container-flex" >    
            <div class= "row">
                <div class="col-sm-12 col-lg-5 col-xl-3 pr-0 pl-0 " style="margin: auto; font-family: 'Exo', sans-serif; font-size: smaller;">          
                                
                    <div class="row mt-3 ml-3 mr-2" style="margin: auto; height: 35px;  border-bottom:1px solid rgba(0,0,0,0.15); background-color:rgb(20,20,20);">
                        <div class="col-xl-12 mt-1 mb-1"> 
                           <h6>Capacidade do Reservatório</h6>
                        </div>
                    </div>
                    <div class="row mt-0 ml-3 mr-2" style="margin: auto; height: 60px; background-color:rgb(20,20,20);">
                        <div class="col-xl-6 mt-1 mb-1"> 
                            <p id="sensorType" style="margin-top:0px; margin-bottom: 0px;">Tipo de Sensor</p>
                        </div>
                        <div class="col-xl-6 mt-1 mb-1"> 
                            <p id="lat" style="margin-top:0px; margin-bottom: 0px;">Lat:</p>
                            <p id="lon" style="margin-top:0px; margin-bottom: 0px;">Lon:</p>
                        </div>
                    </div>
                    <div class="row mt-0 ml-3 mr-2" style="margin: auto; height: 355px; background-color:rgb(20,20,20);">
                        <div class="col-xl-12" >
                            <div style="height: 350px;" id="chart_gauge"></div>                             
                        </div>
                    </div>                 
                </div>
                <div class="col-sm-12 col-lg-7 col-xl-6 pr-0 pl-0" style="margin: auto; font-family: 'Exo', sans-serif; font-size: smaller;">                       
                    <div class="row mt-3 ml-2 mr-2" style="margin: auto; height: 450px; background-color:rgb(20,20,20);">
                        <div class="col-xl-12" > 
                            <div style="height: 445px;" id="chart_line"></div>
                        </div>
                    </div>
                </div>
                <div class="col-sm-12 col-lg-5 col-xl-3 pl-0 pr-0" style="margin: auto; font-family: 'Exo', sans-serif; font-size: smaller;">
                    <div class="row mt-3 ml-2 mr-3" style="margin: auto; height: 35px;  border-bottom:1px solid rgba(0,0,0,0.15); background-color:rgb(20,20,20);">
                        <div class="col-xl-12 mt-1 mb-1"> 
                            <h6>Log de Eventos</h6> 
                        </div>
                    </div>
                    <div class="row mt-0 ml-2 mr-3" style="margin: auto; height: 60px; background-color:rgb(20,20,20);">
                        <div class="col-xl-12 mt-1 mb-1"> 
                            Descrição
                        </div>
                    </div>                        
                    <div class="row mt-0 ml-2 mr-3" style="margin: auto; height: 355px; background-color:rgb(20,20,20);">
                        <div class="col-xl-12 col-sm-12 col-md-12 col-lg-12" style="margin: auto; background-color:rgb(20,20,20);"> 
                            <div style="height: 350px;" id="teste2"></div>
                        </div>
                    </div>
                </div>
            </div>              
        </div>
        <!--Fim da aáea de plotagem do gráfico-->
        
        <script>
            //--------------------------- Declaração das variáveis ----------------------------------
            var webSocket

            //------------------------ Variáveis para o gráfico de linha ----------------------------
            var trace1 = {
                    x: [new Date()],
                    y: [0],
                    line: {color: 'rgb(200, 200, 200)', shape: "spline", smoothing: 0.6, width: 1},
                    fill: "tozeroy",
                    fillcolor: "rgba(230, 130, 30, 0.1)",        
                    name: 'Sinal',
                    type: 'lines', 
                    xaxis: 'x',
                };

            var data = [trace1];

            var layout = {       

                plot_bgcolor:"rgb(20,20,20)",
                paper_bgcolor:"rgb(20,20,20)",              
                font: {size: 12, color: "rgb(110,110,110)"},
                dragmode: 'zoom',
                margin: {
                    r: 30, 
                    t: 30, 
                    b: 55, 
                    l: 55
                },

                xaxis: {               
                    //range: [1,  6],
                    autorange: true,                                   
                
                    rangeslider: {
                        visible: false,
                    },
                                    
                    gridcolor: "rgb(25,25,25)"
                },

                yaxis: {
                    autorange: true,
                    gridcolor: "rgb(25,25,25)"
                },

                showlegend: true,
                legend: {"orientation": "h", x: 0},
                whiskerwidth: 2,            
            };

            var config = {
                responsive: true, 
                displayModeBar: true, 
                scrollZoom: true, 
                displaylogo: false,
                //modeBarButtonsToRemove: ['lasso2d','resetScale2d','zoomOut2d', 'zoomIn2d', 'toggleSpikelines', 'hoverClosestCartesian'],
            }
            //----------------------- Fim das variáveis do gráfico de linha -------------------------

            
            //------------------------ Variáveis para o gráfico indicador ---------------------------
            var data2 = [
            {
                domain: { x: [0, 1], y: [0, 1] },
                value: 0,
                title: { text: "Potenciômetro", font: {color: "rgb(110,110,110)"} },
                type: "indicator",
                mode: "gauge+number",
                number: {font:{color: "rgb(180,180,180)"}, suffix: "%"},
                delta: { reference: 50 },
                gauge: { axis: { range: [null, 100], tickcolor: "rgb(110,110,110)"}, bar: {color: "rgb(110,0,0)"}, bordercolor: "rgb(110,110,110)"}
            }
            ];
        
            var layout2 = {
                plot_bgcolor:"rgb(20,20,20)",
                paper_bgcolor:"rgb(20,20,20)", 
            };
            
            var config2 = {responsive:true, displaylogo: false}
            //---------------------- Fim das variáveis do gráfico Indicador -------------------------
            
           

            function init(){
                console.log("Entrou no init")                      
                Plotly.newPlot("chart_line", data, layout, config)
                Plotly.newPlot("chart_gauge", data2, layout2, config2);
                webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
                webSocket.onmessage = function(event) {
                    let data = JSON.parse(event.data);                 
                    addData(new Date(), data.value)
                    console.log(data.value)
                }
            }

            function addData(t_, value_){
                console.log(value_)
                trace1.x.push(t_)
                trace1.y.push(value_)
                if(trace1.x.length > 30){
                    trace1.x.splice(0,1)
                    trace1.y.splice(0,1)
                }
                let data_ = [trace1]
                let percent = (value_/1024)*100
                if(percent > 90 || percent < 10){
                    data2[0].gauge.bar.color = "rgb(230, 80, 80)"
                }
                else{
                    data2[0].gauge.bar.color = "rgb(230, 130, 30)"
                }
                data2[0].value = percent
                Plotly.newPlot("chart_line", data_, layout, config)
                Plotly.newPlot("chart_gauge", data2, layout2, config2)
            }
        </script>
    </body>
    </html>

)=====";

void getData();
void webSocketEv();

void setup(void) {

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 0);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", [](){
        server.send_P(200, "text/html", webpage);
    });
    server.begin();
    webSocket.begin();
    //webSocket.onEvent(webSocketEv);
    timer.attach(1, getData);
}

void loop(void) {
    webSocket.loop();
    server.handleClient();
}

void getData(){
  
    #if ARDUINOJSON_VERSION_MAJOR==6
        DynamicJsonDocument jsonBuffer(1024);
        JsonObject msg = jsonBuffer.to<JsonObject>();
    #else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& msg = jsonBuffer.createObject();
    #endif
        msg["sensor"] = "SensorX";
        msg["value"] = analogRead(A0);
    
        String str;
    #if ARDUINOJSON_VERSION_MAJOR==6
        serializeJson(msg, str);
    #else
        msg.printTo(str);
    #endif

    webSocket.broadcastTXT(str.c_str(), str.length());
}

void webSocketEv(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
    Serial.print("IP address: ");
}

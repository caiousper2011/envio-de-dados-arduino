  #include <SPI.h>
  #include <Ethernet.h>
  #include <ArduinoJson.h>
  #include <SD.h>
  File arquivo_sd;
  EthernetClient client;
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);
  void setup() {
    Serial.begin(9600);
    if (!SD.begin(4)) {
      Serial.println("Error: Cartao SD. Contate nossa central 0800 1111 2222");
      return;
    }
    byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    IPAddress ip(192, 168, 0, 177);
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Configuração usando DHCP falhou, mudando para ipFixo: " + ip);
      Ethernet.begin(mac, ip);
    }
    delay(1000);
  }
  void loop() {
    if(Serial.read()){
      bater_ponto(2, "Caio Barbosa de Almeida", "2018-09-16 15:07");
    }
  }
  void criar_batida_cartao(int batida_ponto_func, String batida_nome_func, String batida_data_func){
    if(!SD.exists(nome_arquivo_cartao(batida_ponto_func))){
      arquivo_sd = SD.open(nome_arquivo_cartao(batida_ponto_func), FILE_WRITE);
      arquivo_sd.println(gerar_objeto(batida_ponto_func, batida_nome_func, batida_data_func));
      fechar_cartao();
    }else{
      excluir_batida(batida_ponto_func);
      arquivo_sd = SD.open(nome_arquivo_cartao(batida_ponto_func), FILE_WRITE);
      arquivo_sd.println(gerar_objeto(batida_ponto_func, batida_nome_func, batida_data_func));
      fechar_cartao();
    }
  }

  String gerar_objeto(int id_batida, String nome_usuario, String data_batida){
    return "{'id':"+converterString(id_batida)+", 'nome':'"+nome_usuario+"', '':'"+data_batida+"'}";
  }
  boolean checar_cartao(int batida_ponto_cartao){
    return true;
  }
  String nome_arquivo_cartao(int texto){
    return converterString(texto) + ".txt";
  }
  void fechar_cartao(){
    arquivo_sd.close();
  }
  void bater_ponto(int id_batida_func, String nome_usuario_func, String data_batida_func){
    if (client.connect("ulissesreis.net/citrino/api", 80)) {
      Serial.println("Gravando Registro...");
      client.println("POST /registraponto?id=" + converterString(id_batida_func) + "" + nome_usuario_func + " HTTP/1.1");
      client.println("Host: ulissesreis.net/citrino/api");
      client.println();
    } else {
      Serial.println("Falha na Conexão, registro será gravado internamente.");
      delay(1000);
      criar_batida_cartao(id_batida_func, nome_usuario_func, data_batida_func);
    }
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    if(client.status()==200){
      Serial.println("Registrado com sucesso!");
      log(id_batida_func, 1, data_batida_func);
    }else{
      log(id_batida_func, 2, data_batida_func);
      criar_batida_cartao(id_batida_func, nome_usuario_func, data_batida_func);
    }
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
      while (true);
    }
  }
  String converterString(int valor){
    return "" + valor;
  }
  void excluir_batida(int nome_arquivo_excluir){
    SD.remove(nome_arquivo_cartao(nome_arquivo_excluir));
  }
  void log(int user_id, int status_log, String data_log){
    arquivo_sd = SD.open("log.txt", FILE_WRITE);
    arquivo_sd.println("{");
    arquivo_sd.println("id:" + converterString(user_id) + ",");
    arquivo_sd.println("status:" + converterString(status_log) + ",");
    arquivo_sd.println("data:" + data_log + ",");
    arquivo_sd.println("}");
    fechar_cartao();
  }
  void enviar_log(){
    int i = 0;
    arquivo_sd = SD.open("log.txt");
    JsonObject& root = jsonBuffer.parseObject(arquivo_sd);
    fechar_cartao();
    while(root[i]["id"].as<char*>()!=""){
      client.println("POST /log?id=" + root[i]["id"].as<char*>() + "&status=" + root[i]["status"].as<char*>() + "&data=" + root[i]["data"].as<char*>() + " HTTP/1.1");
    }
  }

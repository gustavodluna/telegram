#include "CTBot.h"
#include "Utilities.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Ticker.h>

CTBot miBot;
CTBotInlineKeyboard miTeclado;
boolean bienvenidaBot = false;


// Configuración para NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000); // Sincronización cada minuto


Ticker temporizadores[3];

void apagarAparato(int index) {
    Aparatos[index].Estado = false;
    Serial.println(Aparatos[index].nombre + " apagado automáticamente.");
}

// Usamos una función lambda para pasar el índice del aparato
void encenderAparato(String nombreAparato, int tiempo) {
    for (int i = 0; i < 3; i++) {
        if (nombreAparato.equalsIgnoreCase(Aparatos[i].nombre)) {
            Aparatos[i].Estado = true;
            Serial.println(Aparatos[i].nombre + " encendido por " + String(tiempo) + " segundos.");
            temporizadores[i].once(tiempo, [i]() { apagarAparato(i); });
            return;
        }
    }
}





void configurarBot() {
  miBot.setTelegramToken(token);

  for (int i = 0; i < CantidadAparatos; i++) {
    miTeclado.addButton(Aparatos[i].nombre, Aparatos[i].nombre, CTBotKeyboardButtonQuery);
    if (i % 3 == 0) {
      miTeclado.addRow();
    }
  }

  // Inicia el cliente NTP
  timeClient.begin();
}



void sincronizarHoraNTP() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
}

String obtenerHoraActual() {
  sincronizarHoraNTP();
  return timeClient.getFormattedTime();
}

void mensajeBienvenidaBot() {
  if (!bienvenidaBot) {
    bienvenidaBot = true;
    Serial << "En Línea, Sistema: " << nombre << "\n";
    for (int i = 0; i < cantidadChats; i++) {
      miBot.sendMessage(IDchat[i], "En Línea, Sistema: " + String(nombre) + "\nEscribe /ayuda para obtener los comandos disponibles.");
    }
  }

  if (estado < conectado) {
    bienvenidaBot = false;
  }
}











void actualizarBot() {
  if (estado != conectado) {
    return;
  }

  mensajeBienvenidaBot();

  TBMessage msg;

  if (miBot.getNewMessage(msg)) {
    Serial.println();
    TelnetStream.println();
    Serial << "Mensaje: " << msg.text << " de " << msg.sender.username << " ID:" << msg.sender.id << "\n";
    TelnetStream << "Mensaje: " << msg.text << " de " << msg.sender.username << " ID:" << msg.sender.id << "\n";

    for (int i = 0; i < cantidadChats; i++) {
      if (msg.sender.id == IDchat[i]) {
        if (msg.messageType == CTBotMessageQuery) {
          for (int i = 0; i < CantidadAparatos; i++) {
            if (msg.callbackQueryData.equals(Aparatos[i].nombre)) {
              Serial.println("Cambio por Teclado");
              TelnetStream.println("Cambio por Teclado");
              Aparatos[i].Estado = !Aparatos[i].Estado;
              String Mensaje = "Aparato ";
              Mensaje += Aparatos[i].nombre;
              Mensaje += ": ";
              Mensaje += (Aparatos[i].Estado ? "Encendido" : "Apagado");
              TelnetStream.println(Mensaje);
              Serial.println(Mensaje);
              miBot.endQuery(msg.callbackQueryID, Mensaje);
              return;
            }
          }
        } else if (msg.text.equalsIgnoreCase("/estado")) {
          Serial.println("Enviando estado");
          TelnetStream.println("Enviando estado");
          PedirEstado(msg.sender.id);
        } else if (msg.text.equalsIgnoreCase("/opciones")) {
          Serial.println("Enviando opciones");
          TelnetStream.println("Enviando opciones");
          PedirEstado(msg.sender.id);
          miBot.sendMessage(msg.sender.id, "Opciones disponibles:", miTeclado);
        } else if (msg.text.equalsIgnoreCase("/hora")) {
          String horaActual = obtenerHoraActual();
          Serial << "Enviando hora actual: " << horaActual << "\n";
          TelnetStream << "Enviando hora actual: " << horaActual << "\n";
          miBot.sendMessage(msg.sender.id, "La hora actual es: " + horaActual);
        } else if (msg.text.equalsIgnoreCase("/ayuda")) {
          String ayuda = "\u2139 Comandos disponibles:\n";
          ayuda += "/estado - Ver el estado de los aparatos.\n";
          ayuda += "/opciones - Mostrar el teclado interactivo.\n";
          ayuda += "/encender - Encender todos los aparatos.\n";
          ayuda += "/apagar - Apagar todos los aparatos.\n";
          ayuda += "/hora - Mostrar la hora actual sincronizada con NTP.\n";
          ayuda += "Encender - Mostrar .\n";
          miBot.sendMessage(msg.sender.id, ayuda);
        } 
        
        else if (msg.text.startsWith("Encender ")) {
        
        String comando = msg.text.substring(9);  // Extrae "Luz 10"
        int espacio = comando.indexOf(' ');
          if (espacio != -1) {
            String nombreAparato = comando.substring(0, espacio);
            int tiempo = comando.substring(espacio + 1).toInt();
            encenderAparato(nombreAparato, tiempo);
          }
        


        
        }else {
          for (int i = 0; i < CantidadAparatos; i++) {
            if (msg.text.equalsIgnoreCase(Aparatos[i].nombre)) {
              Serial.println("Cambio por Nombre");
              TelnetStream.println("Cambio por Nombre");
              Aparatos[i].Estado = !Aparatos[i].Estado;
              String Mensaje = "Cambiando ";
              Mensaje += Aparatos[i].nombre;
              Mensaje += ": ";
              Mensaje += (Aparatos[i].Estado ? "encendido" : "apagado");
              Serial.println(Mensaje);
              TelnetStream.println(Mensaje);
              miBot.sendMessage(msg.sender.id, Mensaje);
              return;
            }
          }
          Serial.println("Enviar 'ayuda'");
          TelnetStream.println("Enviar 'ayuda'");
          miBot.sendMessage(msg.sender.id, "\u2753 Comando no reconocido. Escribe /ayuda para ver los comandos disponibles.");
        }
        return;
      }
    }

    Serial << "Usuario no reconocido\n";
    Serial << "Nombre: " << msg.sender.firstName << " - " << msg.sender.lastName << "\n";
    Serial << "Usuario: " << msg.sender.username << " ID: " << int64ToAscii(msg.sender.id) << "\n";

    TelnetStream << "Usuario no reconocido\n";
    TelnetStream << "Nombre: " << msg.sender.firstName << " - " << msg.sender.lastName << "\n";
    TelnetStream << "Usuario: " << msg.sender.username << " ID: " << int64ToAscii(msg.sender.id) << "\n";

    miBot.sendMessage(msg.sender.id, "\u26D4 No te conozco, por favor contacta al administrador.");
  }
}

void PedirEstado(int64_t IDchat) {
  String Mensaje = "Estado actual:\n";

  for (int i = 0; i < CantidadAparatos; i++) {
    Mensaje += "Aparato ";
    Mensaje += Aparatos[i].nombre;
    Mensaje += ": ";
    Mensaje += (Aparatos[i].Estado ? "Encendido" : "Apagado");
    Mensaje += "\n";
  }

  TelnetStream.println(Mensaje);
  Serial.println(Mensaje);
  miBot.sendMessage(IDchat, Mensaje);
}
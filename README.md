# Heltec_Wireless_Stick_v3

# ESP32 Lora

---

## Instalación

Pagina Oficial: [Wireless Stick(V3)](https://heltec.org/project/wireless-stick-v3/)

---

### Instalación Drivers

Sigue la guía oficial de Heltec para la instalación de drivers en tu sistema operativo.

- Guía Oficial: [Driver](https://docs.heltec.org/general/establish_serial_connection.html#for-windows)

Enlace directo para el driver de Windows.

- [CP210x Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)
- Descargar la versión CP210x Universal Windows Driver

---

### Instalación de Dependencias

Pagina Oficial: [Quick Start](https://docs.heltec.org/en/node/esp32/esp32_general_docs/quick_start.html)

En el menú de Arduino IDE:

- Archivo
- Preferencias
- URLs Adicionales de gestor de placas
- Pegar el siguiente enlace

```
https://github.com/HelTec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.7/package_heltec_esp32_index.json
```

![[Pasted image 20240404175021.png]]

Posteriormente:

- Herramientas
- Placa
- Gestor de placa
- Buscar e instalar: Heltec ESP32 Series Dev-boards

![[Pasted image 20240404175319.png]]

---

### Bug de Licencia

Referencia: [Licence solved](http://community.heltec.cn/t/solved-restoring-esp32-chip-license-to-ht-m00-flash-memory/2676/3)

En caso de obtener un mensaje de error en puerto serie, reportando el ingreso de una licencia valida realizar los siguientes pasos:

- Ingresar el product id en: [查询序列号 (heltec.cn)](https://resource.heltec.cn/search)
  - El product id se reportara en el bug impreso en puerto serial la primera vez que se conecte al pc.
- La pagina devolverá una licencia valida como esta:

```
0x0AD417DA,0x3372211C,0xD201F111,0xED5F8233
```

- Debes ingresar el comando AT por puerto serie de la siguiente forma mientras te pida la licencia. (Remueve 0x y ,)

```
AT+CDKEY=0AD417DA3372211CD201F111ED5F8233
```

# nymea-mqtt
Nymea MQTT broker

The nymea MQTT broker consists of a Qt library containing a MqttClient and a MqttServer implementation.
It can be used standalone or integrated in other applications.

The currently supported MQTT protocol versions are 3.1.0 and 3.1.1.

Both, the client and the server support raw MQTT over TCP as well as MQTT over web socket. Both transports
can be used with or without SSL encryption.

Please refer to the server and client directories for minimalistic, yet fully featured examples on how to
use the library.

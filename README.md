# nymea-mqtt

Nymea MQTT broker

`nymea-mqtt` provides a Qt-based MQTT client and server implementation that can be used standalone or
embedded into other nymea components. Both sides currently support MQTT 3.1.0 and 3.1.1,
communicate over plain TCP or WebSocket transports, and allow SSL/TLS encryption to be enabled or disabled
per deployment.

Minimal but fully functional examples live in the `server/` and `client/` directories.

## Building

The project ships qmake files for Qt 5 and Qt 6. Pick one of the `nymea-mqtt.pro` files that matches your
Qt environment and run:

```bash
mkdir build
cd build
qmake ..
make
```

The shared library (`libnymea-mqtt`) can then be linked into your application. The sample client and server
targets are built from the `client/` and `server/` directories in the same way. For packaging, refer to the
Debian metadata in `debian-*`.

## License

`libnymea-mqtt` is licensed under the GNU Lesser General Public License version 3 (or, at your option,
any later version). 

The standalone client (`client/`), server (`server/`), and test suite (`tests/`) are GPL-3.0-or-later
programs. 

The full text of the GNU LGPL and GNU GPL is included in `LICENSE.LGPL3` and `LICENSE.GPL3`, respectively.

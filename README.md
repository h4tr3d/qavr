QAvr
====

Qt-based GUI for avrdude AVR programmer.

Information on Russian: http://htrd.su/wiki/proekty/qavr/start

It allows:
  - Burning, reading and verify flash to/from AVR controller
  - Burning, reading and verify eeprom to/from AVR controller
  - Visual editing of fuse bits
  - Reading fuse bits from controller
  - Reading lock bits from controller

Program distributed under GPLv2 terms.

TODO
----

  - Lock bits editing


Install
-------

Needed:
  - Qt4 at least 4.6
  - gcc compiller at least 4.0
  - cmake

To build:
  - Download sources
  - Unpack sources
  - Change current directory to souces
  - Create directory `build`
  - Run next commands: `cd build && cmake .. && make && sudo make install`

You can ask me to make builds for Windows (and pay about 10$) or download it from Russian page (old versions only).


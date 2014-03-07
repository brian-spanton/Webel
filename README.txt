Copyright © 2014 Brian Spanton

Eleweb
======

Eleweb is an independent C++ implementation of sockets, TLS, HTTP, HTML, JSON and more, in a Windows service framework suitable for web crawler, web server, web client, web browser, load simulation, http proxy, or higher level applications.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

- tls 1.0 and 1.1 implementation based on applicable RFCs
- http implementation based on applicable RFCs
- cookie implementation based on applicable RFCs
- html implementation based on http://www.whatwg.org/specs/web-apps/current-work/multipage/ and applicable RFCs
- json implementation based on applicable RFCs
- json extension for extracting json from html
- UTF-8, UTF-32, and legacy single byte (ascii, etc.) encoder/decoder based on http://encoding.spec.whatwg.org
- dynamically loads standard html named character references from http://www.whatwg.org/specs/web-apps/current-work/multipage/entities.json
- dynamically loads suite of legacy single byte encodings from http://encoding.spec.whatwg.org/encodings.json
- minimal dependencies: just stl and low level win32 pieces (sockets, crypto, threading, file io)
- 100% async and non-blocking and multi-threaded
- efficient logging subsystem
- modern suffix tree implementation
- excellent smart pointer implementation
- good protocol framework for protocol implementation and layering
- cleanly decoupled namespaces for major subsystems
- readable code with verbose but clean naming conventions
- thoughtfully factored and object oriented

Brian Spanton
brian@spanton.net

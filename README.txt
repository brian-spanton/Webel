Eleweb
======

Eleweb is an independent C++ implementation of sockets, TLS, HTTP, HTML, JSON and more, in a Windows service framework suitable for web crawler, web server, web client, web browser, load simulation, http proxy, or higher level applications.

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

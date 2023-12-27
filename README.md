# STM - Secure Terminal Messenger

## Introduction

`STM` is an application designed to be simple and realible terminal chat.

## Requirements

- C compiler
- [Termui](https://github.com/Anthhon/termui)

## Why using Termui?

`STM` was conceived with a need for a simple Text User Interface (TUI). Although `ncurses` was deemed excessive for this purpose, I created `Termui`, a lightweight statically linked library tailored for this project.

Ps: While initially designed for this project, `Termui` isn't limited to this application and can be utilized for other purposes. Explore the repository for more information.

## Building

The installation process involves a `Makefile`, making it straightforward. Upon executing the following command, you will obtain the `server` and `client` binaries.

```bash
make
# or
make build-linux
```

## Usage

### Server-side

To initiate the server-side application responsible for managing clients, use this command:

```bash
./stm-server [server_ip] [port]
```

Ensure port forwarding is configured if you intend to use it across WAN networks.

### Client-side

Subsequently, connect to the server using this command:

```bash
./stm [server_ip] [username] [port]
```

Upon successful connection, you'll see a prompt at the bottom of the terminal for sending messages. Incoming messages will appear in the terminal's message history section at the top.

## Roadmap

- [X] Create a basic UDP chat app
- [X] Implement some basic terminal interface
- [X] Implement basic message metadata
- [X] Implement [Termui](https://github.com/Anthhon/termui) TUI library
- [X] Implement multi-threading for both client and server
- [X] Make it work at WAN network
- [ ] Implement Winsock library so it work on Windows XD
- [ ] Add unit tests

## Contribute

This project operates under the [MIT License](./LICENSE), welcoming contributions from everyone. Feel free to fork the repository, make changes, and submit pull requests. Your contributions are valued and appreciated!

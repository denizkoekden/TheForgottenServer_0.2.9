# TheForgottenServer_0.2.9
Old The Forgotten Server 0.2.9 for Tibia - Fixed to compile and run with modern OS (Ubuntu 24.04)

The Forgotten Server is an open-source MMORPG server emulator that supports multiple platforms. This repository contains a customized version of the server with various bug fixes and additional features.

## Features and Fixes

### Bug Fixes
- **Database Issue with SQLite**: Resolved a bug where the `DatabaseSqLite` class could not properly inherit from the `_Database` class, which was causing compilation issues.
- **Warnings and Errors**: Addressed several compiler warnings, including issues with `snprintf`, `fgets`, and memory allocation in several parts of the code.
- **Guild Leader Minimum Level**: Implemented a configuration setting in `config.lua` to specify the minimum level required to become a guild leader.

### New Features
- **Guild Leader Minimum Level Configurable**: The minimum level required for a player to be promoted to guild leader can now be configured in `config.lua`. Simply set the desired level using the `guildLeaderMinLvl` parameter.
- **Animated Spells Above Player**: A new feature that allows animated spells to appear above the player. This can also be configured in `config.lua` using the `animatedSpellsAbovePlayer` parameter.

## Requirements

- **Ubuntu 24.04+**
- **Dependencies**:
  - `g++`
  - `liblua5.1-0-dev`
  - `libmysqlclient-dev`
  - `libsqlite3-dev`
  - `libboost-system-dev`
  - `libboost-thread-dev`
  - `libgmp-dev`
  - `libxml2-dev`
  - `luasql-mysql`
  - `luasql-sqlite3`

## Compilation Guide

Follow these steps to compile the server on Ubuntu 24.04+:

1. **Install Dependencies**:
   ~~~bash
   sudo apt-get update
   sudo apt-get install g++ liblua5.1-0-dev libmysqlclient-dev libsqlite3-dev libboost-system-dev libboost-thread-dev libgmp-dev libxml2-dev luasql-mysql luasql-sqlite3
   ~~~

2. **Clone the Repository**:
   ~~~bash
   git clone https://github.com/your-repo/forgottenserver.git
   cd forgottenserver
   ~~~

3. **Compile the Server**:
   Run the following command to compile the server:
   ~~~bash
   make
   ~~~
   The compiled binary will be generated as `TheForgottenServer`.

4. **Prepare the Server**:
   - **SQLite**: You can use SQLite as your database engine. No further setup is needed; the server will use the `forgottenserver.sql` database automatically.
   - **MySQL**: Import the `forgottenserver.sql` file into your MySQL database:
     ~~~bash
     mysql -u yourusername -p yourpassword yourdatabase < forgottenserver.sql
     ~~~
     Then, update the database credentials in `config.lua`:
     ~~~lua
     sqlType = "mysql"
     sqlHost = "127.0.0.1"
     sqlPort = 3306
     sqlUser = "yourusername"
     sqlPass = "yourpassword"
     sqlDatabase = "yourdatabase"
     ~~~

5. **Run the Server**:
   You can start the server by executing the binary:
   ~~~bash
   ./TheForgottenServer
   ~~~

   For easier use, create a `runServer.sh` script that restarts the server automatically if it crashes:
   ~~~bash
   echo '#!/bin/bash' > runServer.sh
   echo 'while true; do ./TheForgottenServer; echo "Your server crashed, restarting..."; sleep 2; done' >> runServer.sh
   chmod +x runServer.sh
   ./runServer.sh
   ~~~

## Configuration

### `config.lua`
The `config.lua` file contains various settings you can customize for your server, including:

- **Guild Leader Minimum Level**: Set the minimum level required for a player to be promoted to guild leader:
  ~~~lua
  guildLeaderMinLvl = 50
  ~~~

- **Animated Spells Above Player**: Enable or disable animated spells appearing above players:
  ~~~lua
  animatedSpellsAbovePlayer = true
  ~~~

## Using the Server

1. **SQLite**: By default, the server uses SQLite for its database. You don't need to perform any additional setup.
2. **MySQL**: If you prefer to use MySQL, import the `forgottenserver.sql` file into your database and update the database credentials in `config.lua`.

## Additional Information

For more details on configuring and running your server, refer to the official [Forgotten Server Wiki](https://github.com/otland/forgottenserver/wiki).
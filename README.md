## Dependencies

| Package           | Command                          |
|-------------------|----------------------------------|
| build-essential   | `sudo apt install build-essential`  |
| nasm              | `sudo apt install nasm`             |
| libboost-all-dev  | `sudo apt install libboost-all-dev` |
| openssl           | `sudo apt install openssl`          |
| zlib1g            | `sudo apt install zlib1g`           |
| libasio-dev       | `sudo apt install libasio-dev`      |
| pkg-config        | `sudo apt install pkg-config`       |
| libsvtav1-dev     | `sudo apt install libsvtav1-dev`    |
| libsvtav1enc-dev  | `sudo apt install libsvtav1enc-dev` |
| libx264-dev       | `sudo apt install libx264-dev`      |
| libx265-dev       | `sudo apt install libx265-dev`      |
| libvpx-dev        | `sudo apt install libvpx-dev`       |
| unixodbc-dev      | `sudo apt install unixodbc-dev`     |
| libpq-dev         | `sudo apt install libpq-dev`        |
| libpqxx           | `sudo apt install libpqxx-dev`      |
| libswscale-dev    | `sudo apt install  libswscale-dev`  |


## Build
git clone https://github.com/NextFlix-Project/VideoEncodingServer.git
cd VideoEncodingServer
mkdir build
cd build
cmake ../
make
[![CodeFactor](https://www.codefactor.io/repository/github/lucas-engen/steel-monsters/badge)](https://www.codefactor.io/repository/github/lucas-engen/steel-monsters)
[![Build](https://github.com/lucas-engen/Steel-Monsters/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/lucas-engen/Steel-Monsters/actions/workflows/build.yml)

# Steel Monsters

It's a 2D battle game between tanks

# Building

- Generate configuration
    ```sh
    autoreconf -i
    ```

- Generate makefiles
    ```sh
    ./configure
    ```
- Prepare building workspace
    ```sh
    mkdir build
    cd build
    ../configure
    ```

- Build program
    ```sh
    make
    ```

# Installing
```sh
make install
```

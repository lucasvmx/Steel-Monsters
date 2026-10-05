[![CodeFactor](https://www.codefactor.io/repository/github/lucas-engen/steel-monsters/badge)](https://www.codefactor.io/repository/github/lucas-engen/steel-monsters)
[![Build](https://github.com/lucas-engen/Steel-Monsters/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/lucas-engen/Steel-Monsters/actions/workflows/build.yml)

# Steel Monsters

It's a 2D battle game between tanks

## Gameplay

Fight the enemy tank, avoid its shells, and land three hits to win. Hold Space to fire repeatedly.

- Move with `A` / `D` or the arrow keys.
- Press `P` to pause or resume.
- Press `Esc` to return to the menu.
- After a battle, press `Enter` to play again.

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

[![CodeFactor](https://www.codefactor.io/repository/github/lucas-engen/steel-monsters/badge)](https://www.codefactor.io/repository/github/lucas-engen/steel-monsters)
[![Build](https://github.com/lucas-engen/Steel-Monsters/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/lucas-engen/Steel-Monsters/actions/workflows/build.yml)

# Steel Monsters

It's a 2D turn-based artillery game between tanks.

## Gameplay

Take turns firing ballistic shells at the enemy tank. Set your cannon angle and shot power, account for the wind, and reshape the battlefield with craters. Land three hits to win.

- Adjust the cannon angle with `Up` / `Down`.
- Adjust shot power with `W` / `S`.
- Move with `A` / `D` during your turn.
- Fire with `Space`.
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

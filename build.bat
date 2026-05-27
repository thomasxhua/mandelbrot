gcc ./src/main.c ./extern/glad/src/gl.c ^
    -Wall -Wextra -Wpedantic ^
    -I./extern/glad/include/ ^
    -I./extern/glfw/include/ ^
    -L ./ ^
    -lglfw3 ^
    -o ./dist/main


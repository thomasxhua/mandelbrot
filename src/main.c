#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define SHADER_LOG_SIZE 2048

#ifndef __FUNCTION__
    #define __FUNCTION__ "__FUNCTION__"
#endif

#define MSG_ERROR "[ERROR] "
#define MSG_ERROR_FAILED_ALLOCATION MSG_ERROR "Failed allocation in "

#define INCLUDE_STR "include"
#define INCLUDE_STR_SIZE 7

#define TITLE_COUNTDOWN_S_INITIAL 0.1

// https://fundza.com/c4serious/fileIO_reading_all/index.html
char* new_read_file(const char* path)
{
    FILE *file;
    size_t numbytes;
    file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, "Failed reading '%s'.", path);
        return NULL;
    }
    // get number of bytes
    fseek(file, 0L, SEEK_END);
    numbytes = ftell(file);
    // reset position indicator
    fseek(file, 0L, SEEK_SET);
    // string + \n + \0
    char* buffer = calloc(numbytes + 1 + 1, sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, MSG_ERROR_FAILED_ALLOCATION "%s", __FUNCTION__);
        fclose(file);
        return NULL;
    }
    fread(buffer, sizeof(char), numbytes, file);
    buffer[numbytes] = '\n';
    fclose(file);
    return buffer;
}

char* new_preprocessed_shader(const char* shader)
{
    char* new_shader_buffer = NULL;
    bool had_include = false;
    // TODO: trust im not doing *curr
    for (char* curr = (char*)shader; *curr != '\0'; ++curr)
    {
        // detect include directive
        if (*curr == '#' && strncmp(curr+1, INCLUDE_STR, INCLUDE_STR_SIZE) == 0)
        {
            // detect file name
            char* quote    = curr+1+INCLUDE_STR_SIZE;
            char* file_lhs = NULL;
            for (; *quote != '\"'; ++quote)
            {
                if (*quote == '\0' || *quote == '\n')
                {
                    fprintf(stderr, MSG_ERROR "%s: incomplete include directive.", __FUNCTION__);
                    return NULL;
                }
            }
            file_lhs = ++quote;
            for (; *quote != '\"'; ++quote)
            {
                if (*quote == '\0' || *quote == '\n')
                {
                    fprintf(stderr, MSG_ERROR "%s: incomplete file name in include directive.", __FUNCTION__);
                    return NULL;
                }
            }
            // fake mini string
            const size_t file_name_size = quote - file_lhs;
            char* file_name = calloc(file_name_size + 1, sizeof(char));
            if (!file_name)
            {
                fprintf(stderr, MSG_ERROR_FAILED_ALLOCATION "%s", __FUNCTION__);
                return NULL;
            }
            memcpy(file_name, file_lhs, file_name_size);
            char* file_buffer = new_read_file(file_name);
            free(file_name);
            if (!file_buffer)
                return NULL;
            size_t before_size = curr - shader;
            // detect next line
            char* after = ++quote;
            for (; *quote != '\n' && *quote != '\0'; ++quote);
            size_t after_size = strlen(after);
            new_shader_buffer = calloc(before_size + strlen(file_buffer) + after_size + 1, sizeof(char));
            if (!new_shader_buffer)
            {
                fprintf(stderr, MSG_ERROR_FAILED_ALLOCATION "%s", __FUNCTION__);
            }
            strncat(new_shader_buffer, shader, before_size);
            strcat(new_shader_buffer, file_buffer);
            strcat(new_shader_buffer, after);
            free(file_buffer);
            curr = after;
            had_include = true;
        }
    }
    if (!had_include)
    {
        new_shader_buffer = calloc(strlen(shader)+1, sizeof(char));
        memcpy(new_shader_buffer, shader, strlen(shader));
        return new_shader_buffer;
    }
    else
    {
        char* newer_shader_buffer = new_preprocessed_shader(new_shader_buffer);
        if (newer_shader_buffer != new_shader_buffer)
            free(new_shader_buffer);
        return newer_shader_buffer;
    }
}

void error_callback_glfw(const int error, const char* description)
{
    fprintf(stderr, MSG_ERROR "GLFW: code %i msg: %s.\n", error, description);
}

// https://antongerdelan.net/opengl/
int main()
{
    printf("Starting GLFW %s.\n", glfwGetVersionString());
    glfwSetErrorCallback(error_callback_glfw);
    //
    // init OpenGL
    //
    if (!glfwInit())
    {
        fprintf(stderr, MSG_ERROR "Couldn't start GLFW3.\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // anti aliasing
    glfwWindowHint(GLFW_SAMPLES, 8);
    // full screen
    bool full_screen = false;
    GLFWmonitor* monitor = NULL;
    int win_w = 800, win_h = 600;
    if (full_screen)
    {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        // borderless fullscreen
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        win_w = mode->width;
        win_h = mode->height;
    }
    GLFWwindow* window = glfwCreateWindow(win_w, win_h, "Mandelbrot", monitor, NULL);
    if (!window)
    {
        fprintf(stderr, MSG_ERROR "Couldn't open window with GLFW3.\n");
        return 1;
    }
    glfwMakeContextCurrent(window);
    const int version_glad = gladLoadGL(glfwGetProcAddress);
    if (version_glad == 0)
    {
        fprintf(stderr, MSG_ERROR "Failed to initialize OpenGL context.\n");
    }
    printf("Load OpenGL %i.%i\n",
        GLAD_VERSION_MAJOR(version_glad),
        GLAD_VERSION_MINOR(version_glad));
    printf("Renderer: %s.\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s.\n", glGetString(GL_VERSION));
    //
    // draw triangle
    //
    const float points[] =
    {
        // https://stackoverflow.com/questions/2588875/whats-the-best-way-to-draw-a-fullscreen-quad-in-opengl-3-2
        -1.0f, -1.0f, 0.0f, // A
         3.0f, -1.0f, 0.0f, // B
        -1.0f,  3.0f, 0.0f, // C
    };
    // copy points to graphics card in a vertex buffer object
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), points, GL_STATIC_DRAW);
    // set up VAO remembering vertex buffers
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    // read shaders
    char* vert_buffer = new_read_file("shaders/main.vert");
    if (!vert_buffer)
        return 1;
    char* frag_buffer = new_read_file("shaders/main.frag");
    if (!frag_buffer)
        return 1;
    // preprocess shaders
    char* vert_pp_buffer = new_preprocessed_shader(vert_buffer);
    char* frag_pp_buffer = new_preprocessed_shader(frag_buffer);
    GLint params;
    // vertex shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, (const char* const*)&vert_pp_buffer, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
    if (params != GL_TRUE)
    {
        GLchar log[SHADER_LOG_SIZE];
        glGetShaderInfoLog(vs, SHADER_LOG_SIZE, NULL, log);
        fprintf(stderr, MSG_ERROR "Shader index %u did not compile.\n%s\n", vs, log);
        return 1;
    }
    // fragment shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, (const char* const*)&frag_pp_buffer, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
    if (params != GL_TRUE)
    {
        GLchar log[SHADER_LOG_SIZE];
        glGetShaderInfoLog(fs, SHADER_LOG_SIZE, NULL, log);
        fprintf(stderr, MSG_ERROR "Shader index %u did not compile.\n%s\n", fs, log);
        return 1;
    }
    // link program
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, fs);
    glLinkProgram(shader_program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glGetShaderiv(shader_program, GL_LINK_STATUS, &params);
    if (params != GL_TRUE)
    {
        GLchar log[SHADER_LOG_SIZE];
        glGetShaderInfoLog(shader_program, SHADER_LOG_SIZE, NULL, log);
        fprintf(stderr, MSG_ERROR "Could not link shader program GL index %u.\n%s\n", shader_program, log);
        return 1;
    }
    double prev_s            = glfwGetTime();
    double title_countdown_s = TITLE_COUNTDOWN_S_INITIAL;
    // loop
    while (!glfwWindowShouldClose(window))
    {
        // fps
        const double curr_s    = glfwGetTime();
        const double elapsed_s = curr_s - prev_s;
        prev_s                 = curr_s;
        // print fps every title_countdown seconds
        title_countdown_s      -= elapsed_s;
        if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
        {
            const double fps = 1.0 / elapsed_s;
            char tmp[256];
            sprintf(tmp, "%.2lf FPS (%.2lfms/frame)", fps, 1000.0/fps);
            glfwSetWindowTitle(window, tmp);
            title_countdown_s = TITLE_COUNTDOWN_S_INITIAL;
        }
        // handle input
        glfwPollEvents();
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, 1);
        // check resize
        glfwGetWindowSize(window, &win_w, &win_h);
        glViewport(0, 0, win_w, win_h);
        // draw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
    }
    // clean
    free(vert_buffer);
    free(frag_buffer);
    free(vert_pp_buffer);
    free(frag_pp_buffer);
    glfwTerminate();
    return 0;
}


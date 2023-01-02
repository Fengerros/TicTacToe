#include <stdio.h>

#include <stdlib.h>

#include <Windows.h>

#include "imgui.h"

#include "imgui_impl_glfw.h"

#include "imgui_impl_opengl3.h"

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <string>

#include <fstream>

#include <sstream>

#include <iostream>

#include "shader.hpp"

#include "controls.hpp"

#include "imgui.h"

#include "imgui_impl_glfw.h"

#include "imgui_impl_opengl3.h"

#include "LoadObj.h"

GLFWwindow* window;

int*** dynamicArray3D(int x, int y, int z) {
    int*** array3D = 0;
    array3D = new int** [x];
    for (int i = 0; i < x; ++i) {
        array3D[i] = new int* [y];
        for (int j = 0; j < y; ++j)
            array3D[i][j] = new int[z];
    }
    return array3D;
}

unsigned short game_status = 0;

glm::vec3 grid_size = glm::vec3(3, 3, 3); // x y z

std::vector < glm::vec3 > block_position;
std::vector < glm::vec3 > block_color;

std::vector < glm::vec3 > grid_vertices;
std::vector < glm::vec2 > grid_uvs;
std::vector < glm::vec3 > grid_normals;

std::vector < glm::vec3 > circle_vertices;
std::vector < glm::vec2 > circle_uvs;
std::vector < glm::vec3 > circle_normals;

std::vector < glm::vec3 > cross_vertices;
std::vector < glm::vec2 > cross_uvs;
std::vector < glm::vec3 > cross_normals;

bool undo_pressed = false;
bool first_mouse = false;
bool mouse_visible = false;
bool key_pressed = false;
bool first_frame = true;

bool fullscreen = false;

float width;
float height;

void set_block_position(glm::vec3 position, glm::vec3 color) {
    block_position.push_back(position);
    block_color.push_back(color);
}

void undo_block() {
    if (block_position.size() > 0) {
        block_position.pop_back();
        block_color.pop_back();
    }
    else {
        std::cout << "No blocks to remove" << std::endl;
    }
}

bool check_winner(int*** board, int boardSize) {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (board[i][j][0] == board[i][j][1] && board[i][j][1] == board[i][j][2] && board[i][j][0] != 0) {
                return true;
            }
        }
    }
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (board[i][0][j] == board[i][1][j] && board[i][1][j] == board[i][2][j] && board[i][0][j] != 0) {
                return true;
            }
        }
    }
    for (int i = 0; i < boardSize; i++) {
        if (board[i][0][0] == board[i][1][1] && board[i][1][1] == board[i][2][2] && board[i][0][0] != 0) {
            return true;
        }
        if (board[i][0][2] == board[i][1][1] && board[i][1][1] == board[i][2][0] && board[i][0][2] != 0) {
            return true;
        }
    }
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (board[0][i][j] == board[1][i][j] && board[1][i][j] == board[2][i][j] && board[0][i][j] != 0) {
                return true;
            }
        }
    }
    for (int i = 0; i < boardSize; i++) {
        if (board[0][i][0] == board[1][i][1] && board[1][i][1] == board[2][i][2] && board[0][i][0] != 0) {
            return true;
        }
        if (board[0][i][2] == board[1][i][1] && board[1][i][1] == board[2][i][0] && board[0][i][2] != 0) {
            return true;
        }
    }
    for (int i = 0; i < boardSize; i++) {
        if (board[0][0][i] == board[1][1][i] && board[1][1][i] == board[2][2][i] && board[0][0][i] != 0) {
            return true;
        }
        if (board[0][2][i] == board[1][1][i] && board[1][1][i] == board[2][0][i] && board[0][2][i] != 0) {
            return true;
        }
    }
    if (board[0][0][0] == board[1][1][1] && board[1][1][1] == board[2][2][2] && board[0][0][0] != 0) {
        return true;
    }
    if (board[0][0][2] == board[1][1][1] && board[1][1][1] == board[2][2][0] && board[0][0][2] != 0) {
        return true;
    }
    if (board[0][2][0] == board[1][1][1] && board[1][1][1] == board[2][0][2] && board[0][2][0] != 0) {
        return true;
    }
    if (board[0][2][2] == board[1][1][1] && board[1][1][1] == board[2][0][0] && board[0][2][2] != 0) {
        return true;
    }
    return false;
}

int main() {
    if (!fullscreen) {
        width = 900;
        height = 900;
    }
    else {
        width = 1920;
        height = 1080;
    }

    glewExperimental = true; // Needed for core profile
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 8); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (fullscreen) {
        window = glfwCreateWindow(1920, 1080, "Tic Tac Toe 3D", glfwGetPrimaryMonitor(), NULL);
    }
    else {
        window = glfwCreateWindow(width, height, "Tic Tac Toe 3D", NULL, NULL);
    }
    glfwMakeContextCurrent(window);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwPollEvents();
    glfwSetCursorPos(window, width / 2, height / 2);

    GLuint programID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    GLuint ProjectionID = glGetUniformLocation(programID, "Projection");
    GLuint ViewID = glGetUniformLocation(programID, "View");
    GLuint ModelID = glGetUniformLocation(programID, "Model");

    GLuint LightDirectionID = glGetUniformLocation(programID, "LightDirection");
    GLuint LightPositionID = glGetUniformLocation(programID, "LightPosition");
    GLuint AmbientID = glGetUniformLocation(programID, "ambient_color");
    GLuint DiffuseID = glGetUniformLocation(programID, "diffuse_color");
    GLuint SpecularID = glGetUniformLocation(programID, "specular_color");

    bool grid_res = loadObj("Assets/grid_remaster.obj", grid_vertices, grid_uvs, grid_normals);
    GLuint grid_VBO, grid_VAO;

    glGenVertexArrays(1, &grid_VAO);
    glBindVertexArray(grid_VAO);

    GLfloat* grid_color_buffer_data = new GLfloat[grid_vertices.size() * 3];
    for (int v = 0; v < grid_vertices.size(); v++) {
        grid_color_buffer_data[3 * v + 0] = 1;
        grid_color_buffer_data[3 * v + 1] = 1;
        grid_color_buffer_data[3 * v + 2] = 1;
    }
    glGenBuffers(1, &grid_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(glm::vec3), &grid_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    GLuint colorbuffer;

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * 3 * sizeof(GLfloat), grid_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    GLuint grid_normal;
    glGenBuffers(1, &grid_normal);
    glBindBuffer(GL_ARRAY_BUFFER, grid_normal);
    glBufferData(GL_ARRAY_BUFFER, grid_normals.size() * sizeof(glm::vec3), &grid_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool circle_res = loadObj("Assets/circle.obj", circle_vertices, circle_uvs, circle_normals);
    GLuint circle_VBO, circle_VAO;

    glGenVertexArrays(1, &circle_VAO);
    glBindVertexArray(circle_VAO);

    GLfloat* circle_color_buffer_data = new GLfloat[circle_vertices.size() * 3];
    for (int v = 0; v < circle_vertices.size(); v++) {
        circle_color_buffer_data[3 * v + 0] = 0;
        circle_color_buffer_data[3 * v + 1] = 0;
        circle_color_buffer_data[3 * v + 2] = 0;
    }
    glGenBuffers(1, &circle_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, circle_VBO);
    glBufferData(GL_ARRAY_BUFFER, circle_vertices.size() * sizeof(glm::vec3), &circle_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, circle_vertices.size() * 3 * sizeof(GLfloat), circle_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    GLuint circle_normal;
    glGenBuffers(1, &circle_normal);
    glBindBuffer(GL_ARRAY_BUFFER, circle_normal);
    glBufferData(GL_ARRAY_BUFFER, circle_normals.size() * sizeof(glm::vec3), &circle_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool cross_res = loadObj("Assets/cross.obj", cross_vertices, cross_uvs, cross_normals);
    GLuint cross_VBO, cross_VAO;

    glGenVertexArrays(1, &cross_VAO);
    glBindVertexArray(cross_VAO);

    GLfloat* cross_color_buffer_data = new GLfloat[cross_vertices.size() * 3];
    for (int v = 0; v < cross_vertices.size(); v++) {
        cross_color_buffer_data[3 * v + 0] = 0;
        cross_color_buffer_data[3 * v + 1] = 0;
        cross_color_buffer_data[3 * v + 2] = 0;
    }

    glGenBuffers(1, &cross_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cross_VBO);
    glBufferData(GL_ARRAY_BUFFER, cross_vertices.size() * sizeof(glm::vec3), &cross_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, cross_vertices.size() * 3 * sizeof(GLfloat), cross_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    GLuint cross_normal;
    glGenBuffers(1, &cross_normal);
    glBindBuffer(GL_ARRAY_BUFFER, cross_normal);
    glBufferData(GL_ARRAY_BUFFER, cross_normals.size() * sizeof(glm::vec3), &cross_normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    int*** board = dynamicArray3D(3, 3, 3);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                board[i][j][k] = 0;
            }
        }
    }

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    do {
        if (!fullscreen) {
            if (game_status == 1) {
                MessageBox(NULL, "Player \"circle\" wins!", "Game staus", MB_OK);
                break;
            }
            else if (game_status == 2) {
                MessageBox(NULL, "Player \"cross\" wins!", "Game staus", MB_OK);
                break;
            }
            else if (game_status == 3) {
                MessageBox(NULL, "Draw!", "Game staus", MB_OK);
                break;
            }
        }
        else {
            if (game_status == 1) {
                std::cout << "Player \"circle\" wins!\n";
                break;
            }
            else if (game_status == 2) {
                std::cout << "Player \"cross\" wins!\n";
                break;
            }
            else if (game_status == 3) {
                std::cout << "Draw!", "Game staus\n";
                break;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Insert");
        static glm::vec3 block_pos = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 block_col = glm::vec3(1.0f, 1.0f, 1.0f);
        static glm::vec3 cross_col = glm::vec3(1.0f, 0.0f, 0.0f);
        static glm::vec3 circle_col = glm::vec3(0.0f, 1.0f, 0.0f);
        static glm::vec2 grid_size = glm::vec2(3.0f, 3.0f);
        static glm::vec3 grid_col = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 grid_start_col = glm::vec3(1.0f, 0.0f, 0.0f);
        static glm::vec3 app_back_col = glm::vec3(1.0f, 1.0f, 1.0f);
        static glm::vec3 light_dir = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 light_pos = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 ambient_col = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 diffuse_col = glm::vec3(1.0f, 1.0f, 1.0f);
        static glm::vec3 specular_col = glm::vec3(1.0f, 1.0f, 1.0f);

        ImGui::InputFloat3("Position", &block_pos[0]);

        if (ImGui::Button("Insert")) {
            if (board[(int)block_pos.x][(int)block_pos.y][(int)block_pos.z] == 0) {
                set_block_position(glm::vec3((int)block_pos.x, (int)block_pos.y, (int)block_pos.z), glm::vec3(0.0f, 0.0f, 0.0f));
                if (block_position.size() != 0) {
                    if (block_position.size() % 2 != 0) {
                        board[(int)block_pos.x][(int)block_pos.y][(int)block_pos.z] = 1;

                        if (check_winner(board, 3)) {
                            game_status = 1;
                        }
                    }
                    else {
                        board[(int)block_pos.x][(int)block_pos.y][(int)block_pos.z] = 2;

                        if (check_winner(board, 3)) {
                            game_status = 2;
                        }
                    }
                }

                if (block_position.size() == 27) {
                    game_status = 3;
                }
            }
            else {
                if (fullscreen) {
                    std::cout << "Wrong move! This position is already occupied!\n";
                }
                else {
                    MessageBox(NULL, "This position is already occupied!", "Wrong move!", MB_OK);
                }
            }
        }

        ImGui::End();

        ImGui::Begin("Game Settings");
        ImGui::ColorEdit3("Cross color", &cross_col[0]);
        ImGui::ColorEdit3("Circle color", &circle_col[0]);
        ImGui::ColorEdit3("Background", &app_back_col[0]);

        ImGui::Text("Grid:");
        ImGui::InputFloat2("Grid size", &grid_size[0]);
        ImGui::ColorEdit3("Grid color", &grid_col[0]);
        ImGui::ColorEdit3("Grid start color", &grid_start_col[0]);

        ImGui::End();

        ImGui::Begin("Light Settings");
        ImGui::InputFloat3("Light Position", &light_pos[0]);
        ImGui::ColorEdit3("Ambient Color", &ambient_col[0]);
        ImGui::ColorEdit3("Diffuse Color", &diffuse_col[0]);
        ImGui::ColorEdit3("Specular Color", &specular_col[0]);

        ImGui::End();

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glfwPollEvents();

        glUseProgram(programID);

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (!key_pressed) {
                mouse_visible = !mouse_visible;
                if (mouse_visible) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
                    first_mouse = true;
                }
                else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    if (first_mouse) {
                        glfwSetCursorPos(window, width / 2, height / 2);
                        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
                        first_mouse = false;
                    }
                }
                key_pressed = true;
            }
        }
        else {
            key_pressed = false;
        }

        if (!mouse_visible) {
            computeMatricesFromInputs(window, width, height);
            light_pos = glm::vec3(getPlayerPosition().x, getPlayerPosition().y, getPlayerPosition().z);
        }

        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
        glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

        glUniform3f(LightDirectionID, light_dir.x, light_dir.y, light_dir.z);
        glUniform3f(LightPositionID, light_pos.x, light_pos.y, light_pos.z);
        glUniform3f(AmbientID, ambient_col.x, ambient_col.y, ambient_col.z);
        glUniform3f(DiffuseID, diffuse_col.x, diffuse_col.y, diffuse_col.z);
        glUniform3f(SpecularID, specular_col.x, specular_col.y, specular_col.z);

        glBindVertexArray(grid_VAO);
        for (int i = 0; i < grid_size.x; i++) {
            for (int j = 0; j < grid_size.y; j++) {
                ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(i, 0, j));
                glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
                glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
                glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

                if (i == 0 && j == 0) {
                    for (int v = 0; v < grid_vertices.size(); v++) {
                        grid_color_buffer_data[3 * v + 0] = grid_start_col[0];
                        grid_color_buffer_data[3 * v + 1] = grid_start_col[1];
                        grid_color_buffer_data[3 * v + 2] = grid_start_col[2];
                    }
                }
                else {
                    for (int v = 0; v < grid_vertices.size(); v++) {
                        grid_color_buffer_data[3 * v + 0] = grid_col[0];
                        grid_color_buffer_data[3 * v + 1] = grid_col[1];
                        grid_color_buffer_data[3 * v + 2] = grid_col[2];
                    }
                }

                glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
                glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * 3 * sizeof(GLfloat), grid_color_buffer_data, GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glEnableVertexAttribArray(1);

                glDrawArrays(GL_TRIANGLES, 0, grid_vertices.size());
            }
        }
        glBindVertexArray(0);

        if (block_position.size() != 0) {
            for (int i = 0; i < block_position.size(); i++) {
                if (i % 2 == 0) {
                    glBindVertexArray(circle_VAO);

                    for (int v = 0; v < circle_vertices.size(); v++) {
                        circle_color_buffer_data[3 * v + 0] = circle_col.x;
                        circle_color_buffer_data[3 * v + 1] = circle_col.y;
                        circle_color_buffer_data[3 * v + 2] = circle_col.z;
                    }

                    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
                    glBufferData(GL_ARRAY_BUFFER, circle_vertices.size() * 3 * sizeof(GLfloat), circle_color_buffer_data, GL_STATIC_DRAW);
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                    glEnableVertexAttribArray(1);

                    ModelMatrix = glm::translate(glm::mat4(1.0), block_position[i]);
                    MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
                    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                    glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
                    glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

                    glDrawArrays(GL_TRIANGLES, 0, circle_vertices.size());
                }
                else {
                    glBindVertexArray(cross_VAO);

                    for (int v = 0; v < cross_vertices.size(); v++) {
                        cross_color_buffer_data[3 * v + 0] = cross_col.x;
                        cross_color_buffer_data[3 * v + 1] = cross_col.y;
                        cross_color_buffer_data[3 * v + 2] = cross_col.z;
                    }

                    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
                    glBufferData(GL_ARRAY_BUFFER, cross_vertices.size() * 3 * sizeof(GLfloat), cross_color_buffer_data, GL_STATIC_DRAW);
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                    glEnableVertexAttribArray(1);

                    ModelMatrix = glm::translate(glm::mat4(1.0), block_position[i]);
                    MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
                    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                    glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
                    glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

                    glDrawArrays(GL_TRIANGLES, 0, cross_vertices.size());
                }
                glBindVertexArray(0);
            }
        }
        else {
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glClearColor(app_back_col.x, app_back_col.y, app_back_col.z, 1.0f);

        glfwSwapBuffers(window);

        if (!fullscreen) {
            if (first_frame) {
                glfwSetCursorPos(window, width / 2, height / 2);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
                MessageBox(NULL, "gui - click the \"m\" key on your keyboard", "Game staus", MB_OK);
                MessageBox(NULL, "poligon mode - press the \"p\" key on your keyboard", "Game staus", MB_OK);
                MessageBox(NULL, "ESC to close app", "Game staus", MB_OK);
                glfwSetCursorPos(window, width / 2, height / 2);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
                first_frame = false;
            }
        }
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    glDeleteBuffers(1, &colorbuffer);
    glDeleteVertexArrays(1, &circle_VAO);
    glDeleteVertexArrays(1, &cross_VAO);
    glDeleteVertexArrays(1, &grid_VAO);
    glDeleteProgram(programID);


    glfwTerminate();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
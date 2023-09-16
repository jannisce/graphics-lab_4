#include <array>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>
#include <SDL.h>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <filesystem>
#include <cstdint>

struct Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    Color() : r(0), g(0), b(0), a(255) {}

    Color(int red, int green, int blue, int alpha = 255) {
        r = static_cast<Uint8>(std::min(std::max(red, 0), 255));
        g = static_cast<Uint8>(std::min(std::max(green, 0), 255));
        b = static_cast<Uint8>(std::min(std::max(blue, 0), 255));
        a = static_cast<Uint8>(std::min(std::max(alpha, 0), 255));
    }

    Color(float red, float green, float blue, float alpha = 1.0f) {
        r = std::clamp(static_cast<Uint8>(red * 255), Uint8(0), Uint8(255));
        g = std::clamp(static_cast<Uint8>(green * 255), Uint8(0), Uint8(255));
        b = std::clamp(static_cast<Uint8>(blue * 255), Uint8(0), Uint8(255));
        a = std::clamp(static_cast<Uint8>(alpha * 255), Uint8(0), Uint8(255));
    }

    // Overload the + operator to add colors
    Color operator+(const Color& other) const {
        return Color(
                std::min(255, int(r) + int(other.r)),
                std::min(255, int(g) + int(other.g)),
                std::min(255, int(b) + int(other.b)),
                std::min(255, int(a) + int(other.a))
        );
    }

    // Overload the * operator to scale colors by a factor
    Color operator*(float factor) const {
        return Color(
                std::clamp(static_cast<Uint8>(r * factor), Uint8(0), Uint8(255)),
                std::clamp(static_cast<Uint8>(g * factor), Uint8(0), Uint8(255)),
                std::clamp(static_cast<Uint8>(b * factor), Uint8(0), Uint8(255)),
                std::clamp(static_cast<Uint8>(a * factor), Uint8(0), Uint8(255))
        );
    }

    // Friend function to allow float * Color
    friend Color operator*(float factor, const Color& color);
};

struct Vertex {
    glm::vec3 position;
    Color color;
};

struct Fragment {
    glm::vec3 position;
    Color color;
};

// overload for Vertex
void print(const Vertex& v) {
    std::cout << "Vertex{" << std::endl;
    std::cout << "  glm::vec3(" << v.position.x << ", " << v.position.y << ", " << v.position.z << ")" << std::endl;
    std::cout << "  Color(" << static_cast<int>(v.color.r) << ", " << static_cast<int>(v.color.g) << ", " << static_cast<int>(v.color.b) << ")" << std::endl;
    std::cout << "}" << std::endl;
}

// overload for Color
void print(const Color& v) {
    std::cout << "Color(" << static_cast<int>(v.r) << ", " << static_cast<int>(v.g) << ", " << static_cast<int>(v.b) << ")" << std::endl;
}

// overload for glm::vec3
void print(const glm::vec3& v) {
    std::cout << "glm::vec3(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}

// overload for glm::ivec2
void print(const glm::ivec2& v) {
    std::cout << "glm::vec2(" << v.x << ", " << v.y << ")" << std::endl;
}

// overload for glm::mat4
void print(const glm::mat4& m) {
    std::cout << "glm::mat4(\n";
    for(int i = 0; i < 4; ++i) {
        std::cout << "  ";
        for(int j = 0; j < 4; ++j) {
            std::cout << m[i][j];
            if (j != 3) {
                std::cout << ", ";
            }
        }
        std::cout << (i == 3 ? "\n" : ",\n");
    }
    std::cout << ")" << std::endl;
}

// base case function to end the recursion, using abbreviated template syntax
void print(auto last) {
    std::cout << last << '\n';
}

// recursive variadic template function
template <typename T, typename... Args>
void print(T first, Args... args) {
    print(first);  // call the appropriate print function
    if(sizeof...(args) > 0)
        std::cout << ' ';  // print a space only if there are more arguments
    print(args...);  // call print with remaining arguments
}

struct Uniform {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewport;
};

Vertex vertexShader(const Vertex& vertex, const Uniform& u) {

    glm::vec4 v = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1);


    glm::vec4 r = u.viewport * u.projection * u.view * u.model * v;


    return Vertex{
            glm::vec3(r.x/r.w, r.y/r.w, r.z/r.w),
            vertex.color
    };
};

Fragment fragmentShader(Fragment fragment) {
    return fragment;
};

glm::vec3 light = normalize(glm::vec3(0.5, 2, 2));


glm::vec3 barycentricCoordinates(const glm::vec3& P, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    float w = ((B.y - C.y)*(P.x - C.x) + (C.x - B.x)*(P.y - C.y)) /
              ((B.y - C.y)*(A.x - C.x) + (C.x - B.x)*(A.y - C.y));

    float v = ((C.y - A.y)*(P.x - C.x) + (A.x - C.x)*(P.y - C.y)) /
              ((B.y - C.y)*(A.x - C.x) + (C.x - B.x)*(A.y - C.y));

    float u = 1.0f - w - v;

    return glm::vec3(w, v, u);
}


std::vector<Fragment> triangle(Vertex a, Vertex b, Vertex c) {
    glm::vec3 A = a.position;
    glm::vec3 B = b.position;
    glm::vec3 C = c.position;

    std::vector<Fragment> triangleFragments;

    float minX = std::min(std::min(A.x, B.x), C.x);
    float minY = std::min(std::min(A.y, B.y), C.y);
    float maxX = std::max(std::max(A.x, B.x), C.x);
    float maxY = std::max(std::max(A.y, B.y), C.y);
    //castear a int
    minX = std::floor(minX);
    minY = std::floor(minY);
    maxX = std::ceil(maxX);
    maxY = std::ceil(maxY);
    glm::vec3 N = glm::normalize(glm::cross(B - A, C - A));

    for (float y = minY; y <= maxY; y++) {
        for (float x = minX; x <= maxX; x++) {
            glm::vec3 P = glm::vec3(x, y, 0);

            glm::vec3 bar = barycentricCoordinates(P, A, B, C);

            if (
                    bar.x <= 1 && bar.x >= 0 &&
                    bar.y <= 1 && bar.y >= 0 &&
                    bar.z <= 1 && bar.z >= 0
                    ) {


                P.z = a.position.z * bar.x + b.position.z * bar.y + c.position.z * bar.z;
                float intensity = glm::dot(N, light) * 10;
                Color color = Color(intensity * 255, intensity * 255, intensity * 255);
                triangleFragments.push_back(
                        Fragment{P, color}
                );
            }
        }
    }

    return triangleFragments;
}

const int WINDOW_WIDTH = 840;
const int WINDOW_HEIGHT = 680;

std::array<std::array<float, WINDOW_WIDTH>, WINDOW_HEIGHT> zbuffer;


void writeBMP(const std::string& filename) {
    int width = WINDOW_WIDTH;
    int height = WINDOW_HEIGHT;

    std::string fileopen = "./" + filename;

    float zMin = std::numeric_limits<float>::max();
    float zMax = std::numeric_limits<float>::lowest();

    for (const auto& row : zbuffer) {
        for (const auto& val : row) {
            if (val != 99999.0f) { // Ignora los valores que no han sido actualizados
                zMin = std::min(zMin, val);
                zMax = std::max(zMax, val);
            }
        }
    }

// Verifica que zMin y zMax sean diferentes
    if (zMin == zMax) {
        std::cerr << "zMin y zMax son iguales. Esto producirá una imagen en blanco o negro.\n";
        return;
    }

    std::cout << "zMin: " << zMin << ", zMax: " << zMax << "\n";


    // Abre el archivo en modo binario
    std::ofstream file(fileopen, std::ios::binary);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo para escribir: " << fileopen << "\n";
        return;
    }

    // Escribe la cabecera del archivo BMP
    uint32_t fileSize = 54 + 3 * width * height;
    uint32_t dataOffset = 54;
    uint32_t imageSize = 3 * width * height;
    uint32_t biPlanes = 1;
    uint32_t biBitCount = 24;

    uint8_t header[54] = {'B', 'M',
                          static_cast<uint8_t>(fileSize & 0xFF), static_cast<uint8_t>((fileSize >> 8) & 0xFF), static_cast<uint8_t>((fileSize >> 16) & 0xFF), static_cast<uint8_t>((fileSize >> 24) & 0xFF),
                          0, 0, 0, 0,
                          static_cast<uint8_t>(dataOffset & 0xFF), static_cast<uint8_t>((dataOffset >> 8) & 0xFF), static_cast<uint8_t>((dataOffset >> 16) & 0xFF), static_cast<uint8_t>((dataOffset >> 24) & 0xFF),
                          40, 0, 0, 0,
                          static_cast<uint8_t>(width & 0xFF), static_cast<uint8_t>((width >> 8) & 0xFF), static_cast<uint8_t>((width >> 16) & 0xFF), static_cast<uint8_t>((width >> 24) & 0xFF),
                          static_cast<uint8_t>(height & 0xFF), static_cast<uint8_t>((height >> 8) & 0xFF), static_cast<uint8_t>((height >> 16) & 0xFF), static_cast<uint8_t>((height >> 24) & 0xFF),
                          static_cast<uint8_t>(biPlanes & 0xFF), static_cast<uint8_t>((biPlanes >> 8) & 0xFF),
                          static_cast<uint8_t>(biBitCount & 0xFF), static_cast<uint8_t>((biBitCount >> 8) & 0xFF)};

    file.write(reinterpret_cast<char*>(header), sizeof(header));

    // Escribe los datos de los píxeles
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float normalized = (zbuffer[y][x] - zMin) / (zMax - zMin);
            uint8_t color = static_cast<uint8_t>(normalized * 255);
            file.write(reinterpret_cast<char*>(&color), 1);
            file.write(reinterpret_cast<char*>(&color), 1);
            file.write(reinterpret_cast<char*>(&color), 1);
        }
    }

    file.close();
}

SDL_Renderer* renderer;

Uniform uniform;

struct Face {
    std::vector<std::array<int, 3>> vertexIndices;
};

std::string getCurrentPath() {
    return std::filesystem::current_path().string();
}


std::vector<glm::vec3> vertices;
std::vector<Face> faces;

// Declare a global clearColor of type Color
Color clearColor = {0, 0, 0}; // Initially set to black

// Declare a global currentColor of type Color
Color currentColor = {255, 255, 255}; // Initially set to white

// Function to clear the framebuffer with the clearColor
void clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for (auto &row : zbuffer) {
        std::fill(row.begin(), row.end(), 99999.0f);
    }
}

// Function to set a specific pixel in the framebuffer to the currentColor
void point(Fragment f) {
    if (f.position.y < WINDOW_HEIGHT && f.position.x < WINDOW_WIDTH && f.position.y > 0 && f.position.x > 0 && f.position.z < zbuffer[f.position.y][f.position.x]) {
        SDL_SetRenderDrawColor(renderer, f.color.r, f.color.g, f.color.b, f.color.a);
        SDL_RenderDrawPoint(renderer, f.position.x, f.position.y);
        zbuffer[f.position.y][f.position.x] = f.position.z;
    }
}

std::vector<std::vector<Vertex>> primitiveAssembly(
        const std::vector<Vertex>& transformedVertices
) {
    std::vector<std::vector<Vertex>> groupedVertices;

    for (int i = 0; i < transformedVertices.size(); i += 3) {
        std::vector<Vertex> vertexGroup;
        vertexGroup.push_back(transformedVertices[i]);
        vertexGroup.push_back(transformedVertices[i+1]);
        vertexGroup.push_back(transformedVertices[i+2]);

        groupedVertices.push_back(vertexGroup);
    }

    return groupedVertices;
}


void render(std::vector<glm::vec3> VBO) {
    std::vector<Vertex> transformedVertices;

    for (int i = 0; i < VBO.size(); i++) {
        glm::vec3 v = VBO[i];

        Vertex vertex = {v, Color(255, 255, 255)};
        Vertex transformedVertex = vertexShader(vertex, uniform);
        transformedVertices.push_back(transformedVertex);
    }

    std::vector<std::vector<Vertex>> triangles = primitiveAssembly(transformedVertices);

    std::vector<Fragment> fragments;
    for (const std::vector<Vertex>& triangleVertices : triangles) {
        std::vector<Fragment> rasterizedTriangle = triangle(
                triangleVertices[0],
                triangleVertices[1],
                triangleVertices[2]
        );

        fragments.insert(
                fragments.end(),
                rasterizedTriangle.begin(),
                rasterizedTriangle.end()
        );
    }

    for (Fragment fragment : fragments) {
        point(fragmentShader(fragment));
    }
}


float a = 3.14f / 3.0f;
float b = 0.81f ;
glm::mat4 createModelMatrix() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.2f, -0.09f, 0));
    glm::mat4 rotationy = glm::rotate(glm::mat4(1), glm::radians(a++), glm::vec3(0, 4, 0));
    glm::mat4 rotationx = glm::rotate(glm::mat4(1), glm::radians(b+=0.1f), glm::vec3(1, 0, 0));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.15f, 0.15f, 0.15f));
    return transtation * scale * rotationx * rotationy;
}

glm::mat4 createViewMatrix() {
    return glm::lookAt(
            // donde esta
            glm::vec3(0, 0, -5),
            // hacia adonde mira
            glm::vec3(0, 0, 0),
            // arriba
            glm::vec3(0, 1, 0)
    );
}

glm::mat4 createProjectionMatrix() {
    float fovInDegrees = 85.0f;
    float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
    float nearClip = 0.1f;
    float farClip = 100.0f;

    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

// Función para leer el archivo .obj y cargar los vértices y caras
bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<Face>& out_faces) {
    out_vertices.clear();
    out_faces.clear();

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_vertices.push_back(vertex);
        } else if (type == "f") {
            std::string lineHeader;
            Face face;
            while (iss >> lineHeader)
            {
                std::istringstream tokenstream(lineHeader);
                std::string token;
                std::array<int, 3> vertexIndices;

                // Read all three values separated by '/'
                for (int i = 0; i < 3; ++i) {
                    std::getline(tokenstream, token, '/');
                    vertexIndices[i] = std::stoi(token) - 1;
                }

                face.vertexIndices.push_back(vertexIndices);
            }
            out_faces.push_back(face);
        }
    }

    file.close();
    return true;
}

std::vector<glm::vec3> setupVertexArray(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces) {
    std::vector<glm::vec3> vertexArray;

    // For each face
    for (const auto& face : faces) {
        // For each vertex in the face
        for (const auto& vertexIndices : face.vertexIndices) {
            // Get the vertex position from the input array using the indices from the face
            glm::vec3 vertexPosition = vertices[vertexIndices[0]];

            // Add the vertex position to the vertex array
            vertexArray.push_back(vertexPosition);
        }
    }

    return vertexArray;
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("SR2", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    std::string currentPath = getCurrentPath();
    std::string fileName = "/Users/jannisce/Desktop/final.obj";
    std::string filePath = fileName;

    loadOBJ(filePath, vertices, faces);


    std::vector<glm::vec3> vertexArray = setupVertexArray(vertices, faces);

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
    );

    bool running = true;
    SDL_Event event;

    std::vector<glm::vec3> vertexBufferObject = {
            {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
            {-0.87f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f},
            {0.87f,  -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f},

            {0.0f, 1.0f,    -1.0f}, {1.0f, 1.0f, 0.0f},
            {-0.87f, -0.5f, -1.0f}, {0.0f, 1.0f, 1.0f},
            {0.87f,  -0.5f, -1.0f}, {1.0f, 0.0f, 1.0f}
    };

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event. type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        break;
                    case SDLK_DOWN:
                        break;
                    case SDLK_LEFT:
                        break;
                    case SDLK_RIGHT:
                        break;
                    case SDLK_s:
                        break;
                    case SDLK_a:
                        break;
                }
            }
        }

        uniform.model = createModelMatrix();
        uniform.view = createViewMatrix();
        uniform.projection = createProjectionMatrix();
        uniform.viewport = createViewportMatrix();


        clear();

        render(vertexArray);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
        // Llama a la función para escribir el archivo BMP
        writeBMP("draw.bmp");
    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <vector>
#include <cassert>
#include "color.h"
#include "print.h"
#include "framebuffer.h"
#include "uniforms.h"
#include "shaders.h"
#include "fragment.h"
#include "triangle.h"
#include "camera.h"
#include "ObjLoader.h"
#include "noise.h"
#include "model.h"

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
Color currentColor;

std::vector<Model> models;

bool init()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    std::cerr << "Error: Failed to initialize SDL: " << SDL_GetError() << std::endl;
    return false;
  }

  window = SDL_CreateWindow("LAB 4", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window)
  {
    std::cerr << "Error: Failed to create SDL window: " << SDL_GetError() << std::endl;
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
  {
    std::cerr << "Error: Failed to create SDL renderer: " << SDL_GetError() << std::endl;
    return false;
  }

  setupNoise();

  return true;
}

void setColor(const Color &color)
{
  currentColor = color;
}

void render()
{
  for (const auto &model : models)
  {
    // 1. Vertex Shader
    std::vector<Vertex> transformedVertices(model.vertices.size() / 3);
    for (size_t i = 0; i < model.vertices.size() / 3; ++i)
    {
      Vertex vertex = {model.vertices[3 * i], model.vertices[3 * i + 1], model.vertices[3 * i + 2]};
      transformedVertices[i] = vertexShader(vertex, model.uniforms);
    }

    // 2. Primitive Assembly
    std::vector<std::vector<Vertex>> assembledVertices(transformedVertices.size() / 3);
    for (size_t i = 0; i < transformedVertices.size() / 3; ++i)
    {
      Vertex edge1 = transformedVertices[3 * i];
      Vertex edge2 = transformedVertices[3 * i + 1];
      Vertex edge3 = transformedVertices[3 * i + 2];
      assembledVertices[i] = {edge1, edge2, edge3};
    }

    // 3. Rasterization
    std::vector<Fragment> fragments;

    for (size_t i = 0; i < assembledVertices.size(); ++i)
    {
      std::vector<Fragment> rasterizedTriangle = triangle(
          assembledVertices[i][0],
          assembledVertices[i][1],
          assembledVertices[i][2]);
      fragments.insert(fragments.end(), rasterizedTriangle.begin(), rasterizedTriangle.end());
    }

    // 4. Fragment Shader
    for (size_t i = 0; i < fragments.size(); ++i)
    {
      Fragment (*fragmentShader)(Fragment &) = nullptr;

      switch (model.currentShader)
      {
      case FOLIAGE:
        fragmentShader = foliageShader;
        break;
      case GAS_CLOUD:
        fragmentShader = gasCloudShader;
        break;
      case SOLAR:
        fragmentShader = solarShader;
        break;
      case TERRAIN:
        fragmentShader = terrainShader;
        break;
      case SPHERE:
        fragmentShader = sphereShader;
        break;
      case VIBRANT:
        fragmentShader = vibrantShader;
        break;
      case NIGHT_SKY:
        fragmentShader = nightSkyShader;
        break;
      default:
        std::cerr << "Error: Shader no reconocido." << std::endl;
        break;
      }
      const Fragment &fragment = fragmentShader(fragments[i]);

      point(fragment);
    }
  }
}

glm::mat4 createViewportMatrix(size_t screenWidth, size_t screenHeight)
{
  glm::mat4 viewport = glm::mat4(1.0f);

  // Scale
  viewport = glm::scale(viewport, glm::vec3(screenWidth / 2.0f, screenHeight / 2.0f, 0.5f));

  // Translate
  viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

  return viewport;
}

int main(int argc, char *argv[])
{

  // ShaderType currentShader = FOLIAGE;

  int shaderChoice;

  // Pide al usuario que elija un ShaderType
  std::cout << "Elige un ShaderType:" << std::endl;
  std::cout << "0 - FOLIAGE" << std::endl;
  std::cout << "1 - GAS_CLOUD" << std::endl;
  std::cout << "2 - SOLAR" << std::endl;
  std::cout << "3 - TERRAIN" << std::endl;
  std::cout << "4 - SPHERE" << std::endl;
  std::cout << "5 - VIBRANT" << std::endl;
  std::cout << "6 - NIGHT_SKY" << std::endl;

  std::cin >> shaderChoice;

  // Verifica la selección del usuario y establece el ShaderType correspondiente
  ShaderType currentShader;
  switch (shaderChoice)
  {
  case 0:
    currentShader = FOLIAGE;
    break;
  case 1:
    currentShader = GAS_CLOUD;
    break;
  case 2:
    currentShader = SOLAR;
    break;
  case 3:
    currentShader = TERRAIN;
    break;
  case 4:
    currentShader = SPHERE;
    break;
  case 5:
    currentShader = VIBRANT;
    break;
  case 6:
    currentShader = NIGHT_SKY;
    break;
  default:
    std::cout << "Selección no válida. Se establecerá FOLIAGE por defecto." << std::endl;
    currentShader = FOLIAGE;
    break;
  }

  int generateModel2;
  std::cout << "¿Deseas generar la luna? (1 para Sí, 0 para No): ";
  std::cin >> generateModel2;

  if (!init())
  {
    return 1;
  }

  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> texCoords;
  std::vector<Face> faces;
  std::vector<glm::vec3> vertexBufferObject; // This will contain both vertices and normals

  // loadOBJ("../models/sphere.obj", vertices, normals, texCoords, faces);
  loadOBJ("/Users/jannisce/Documents/GitHub/graphics-lab_4/models/sphere.obj", vertices, normals, texCoords, faces);

  for (const auto &face : faces)
  {
    for (int i = 0; i < 3; ++i)
    {
      // Get the vertex position
      glm::vec3 vertexPosition = vertices[face.vertexIndices[i]];

      // Get the normal for the current vertex
      glm::vec3 vertexNormal = normals[face.normalIndices[i]];

      // Get the texture for the current vertex
      glm::vec3 vertexTexture = texCoords[face.texIndices[i]];

      // Add the vertex position and normal to the vertex array
      vertexBufferObject.push_back(vertexPosition);
      vertexBufferObject.push_back(vertexNormal);
      vertexBufferObject.push_back(vertexTexture);
    }
  }

  Uniforms uniforms;

  glm::mat4 model = glm::mat4(1);
  glm::mat4 view = glm::mat4(1);
  glm::mat4 projection = glm::mat4(1);

  glm::vec3 translationVector(0.0f, 0.0f, 0.0f);
  float a = 45.0f;
  glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f); // Rotate around the Y-axis
  glm::vec3 scaleFactor(1.0f, 1.0f, 1.0f);

  glm::mat4 translation = glm::translate(glm::mat4(1.0f), translationVector);

  glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleFactor);

  // Initialize a Camera object
  Camera camera;
  camera.cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
  camera.targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
  camera.upVector = glm::vec3(0.0f, 1.0f, 0.0f);

  // Projection matrix
  float fovInDegrees = 45.0f;
  float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT); // Assuming a screen resolution of 800x600
  float nearClip = 0.1f;
  float farClip = 100.0f;
  uniforms.projection = glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);

  // Viewport matrix
  uniforms.viewport = createViewportMatrix(SCREEN_WIDTH, SCREEN_HEIGHT);
  Uint32 frameStart, frameTime;
  std::string title = "FPS: ";
  int speed = 10;

  bool running = true;
  while (running)
  {
    frameStart = SDL_GetTicks();

    models.clear(); // Clear models vector at the beginning of the loop

    a += 1;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(a), rotationAxis);

    // Calculate the model matrix
    uniforms.model = translation * rotation * scale;

    // Create the view matrix using the Camera object
    uniforms.view = glm::lookAt(
        camera.cameraPosition, // The position of the camera
        camera.targetPosition, // The point the camera is looking at
        camera.upVector        // The up vector defining the camera's orientation
    );

    // Model 1
    Model model1;
    model1.modelMatrix = glm::mat4(1);
    model1.vertices = vertexBufferObject;
    model1.uniforms = uniforms;
    model1.currentShader = currentShader;
    models.push_back(model1); // Add model1 to models vector

    // Model 2: smaller and placed next to the first model
    Model model2;
    model2.modelMatrix = glm::mat4(1);
    model2.vertices = vertexBufferObject;
    model2.currentShader = NIGHT_SKY;
    model2.uniforms = uniforms;
    model2.uniforms.model = glm::translate(model2.uniforms.model, glm::vec3(1.5f, 0.0f, 0.0f)) * glm::scale(model2.uniforms.model, glm::vec3(0.2f, 0.2f, 0.2f));
    if (generateModel2)
    {
      models.push_back(model2); // Add model2 to models vector only if the user chooses to generate it
    }
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        running = false;
      }

      if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        {
        case SDLK_SPACE:
          currentShader = static_cast<ShaderType>((currentShader + 1) % 7);
          std::cout << "Shader: " << currentShader << std::endl;
          break;
        case SDLK_LEFT:
          camera.cameraPosition.x += -speed;
          break;
        case SDLK_RIGHT:
          camera.cameraPosition.x += speed;
          break;
        case SDLK_UP:
          camera.cameraPosition.y += -speed;
          break;
        case SDLK_DOWN:
          camera.cameraPosition.y += speed;
          break;
        }
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    clearFramebuffer();

    render();

    renderBuffer(renderer);

    frameTime = SDL_GetTicks() - frameStart;

    // Calculate frames per second and update window title
    if (frameTime > 0)
    {
      std::ostringstream titleStream;
      titleStream << "LAB #4 - FPS: " << 1000.0 / frameTime; // Milliseconds to seconds
      SDL_SetWindowTitle(window, titleStream.str().c_str());
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

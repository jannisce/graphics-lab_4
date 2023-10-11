#pragma once
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include "uniforms.h"
#include "fragment.h"
#include "noise.h"
#include "print.h"

Vertex vertexShader(const Vertex &vertex, const Uniforms &uniforms)
{
  // Apply transformations to the input vertex using the matrices from the uniforms
  glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);

  // Perspective divide
  glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

  // Apply the viewport transform
  glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);

  // Transform the normal
  glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
  transformedNormal = glm::normalize(transformedNormal);

  glm::vec3 transformedWorldPosition = glm::vec3(uniforms.model * glm::vec4(vertex.position, 1.0f));

  // Return the transformed vertex as a vec3
  return Vertex{
      glm::vec3(screenVertex),
      transformedNormal,
      vertex.tex,
      transformedWorldPosition,
      vertex.position};
}

Fragment foliageShader(Fragment &fragment)
{

  glm::vec3 neon1 = glm::vec3(0.48f, 0.0f, 0.61f);
  glm::vec3 neon2 = glm::vec3(0.02f, 0.0f, 0.28f);
  glm::vec3 neon3 = glm::vec3(0.46f, 0.31f, 0.55f);
  glm::vec3 neon4 = glm::vec3(0.620f, 0.678f, 0.200f);
  glm::vec3 neon5 = glm::vec3(0.733f, 0.0f, 0.180f);

  // Declare the color variable
  glm::vec3 c = neon1; // Default color

  // Convert 3D position on sphere to 2D UV
  glm::vec3 pos = glm::normalize(fragment.originalPos);
  float u = 0.5f + atan2(pos.z, pos.x) / (4.0f * glm::pi<float>());
  float v = 0.5f - asin(pos.y) / glm::pi<float>();
  glm::vec2 uv = glm::vec2(u, v);

  uv = glm::clamp(uv, 0.0f, 1.0f);

  // Reflejar las coordenadas UV respecto al centro para girar 180 grados
  uv = glm::vec2(1.0f - uv.x, 1.0f - uv.y);

  // Set up the noise generator
  FastNoiseLite noiseGenerator;
  noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  float offsetX = 800.0f;
  float offsetY = 1600.0f;
  float scale = 900.0f;

  // Genera el valor de ruido
  float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
  noiseValue = (noiseValue + 1.0f) * 0.5f;

  noiseValue = glm::smoothstep(0.0f, 1.0f, noiseValue);

  c = glm::mix(c, neon2, noiseValue);

  // Blend with neon color
  c = glm::mix(c, neon2, 0.5f);

  // Apply lighting intensity
  c = c * fragment.intensity;

  // Convert glm::vec3 color to your Color class
  fragment.color = Color(c.r, c.g, c.b);

  return fragment;
}

Fragment gasCloudShader(Fragment &fragment)
{
  Color color;
  glm::vec3 mainColor = glm::vec3(0.2f, 0.8f, 1.0f);
  glm::vec3 secondColor = glm::vec3(0.0f, 0.6f, 0.6f);
  glm::vec3 thirdColor = glm::vec3(0.8f, 1.0f, 0.9f);

  glm::vec2 uv = glm::vec2(fragment.originalPos.x * 2.0 - 1.0, fragment.originalPos.y * 2.0 - 1.0);

  // Frecuencia y amplitud de las ondas en el planeta
  float frequency = 28.0; // Ajusta la frecuencia de las líneas
  float amplitude = 0.08; // Ajusta la amplitud de las líneas

  FastNoiseLite noiseGenerator;
  noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Cellular);

  float offsetX = 800.0f;
  float offsetY = 200.0f;
  float scale = 1000.0f;

  // Genera el valor de ruido
  float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
  noiseValue = (noiseValue + 1.0f) * 0.5f;

  // aplicar ruido como hacer piedras
  noiseValue = glm::smoothstep(0.0f, 1.0f, noiseValue);

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  secondColor = glm::mix(mainColor, secondColor, noiseValue);

  // Calcula el valor sinusoide para crear líneas
  float sinValue = glm::sin(uv.y * frequency) * amplitude;

  // Combina el color base con las líneas sinusoide
  secondColor = secondColor + glm::vec3(sinValue);

  color = Color(secondColor.x, secondColor.y, secondColor.z);

  fragment.color = color * fragment.intensity;

  return fragment;
}

Fragment solarShader(Fragment &fragment)
{
  Color color;

  glm::vec3 mainColor = glm::vec3(0.3f, 0.0f, 0.0f);
  glm::vec3 secondColor = glm::vec3(0.9f, 0.4f, 0.04f);
  glm::vec3 thirdColor = glm::vec3(0.5f, 0.25f, 0.0f);

  glm::vec2 uv = glm::vec2(fragment.originalPos.x * 2.0 - 1.0, fragment.originalPos.y * 2.0 - 1.0);

  FastNoiseLite noiseGenerator;
  noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Cellular);

  float offsetX = 7000.0f;
  float offsetY = 9000.0f;
  float scale = 1400.0f;

  // Genera el valor de ruido
  float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
  noiseValue = (noiseValue + 1.0f) * 0.7f;

  // aplicar un resplandor a las orillas de la esfera
  noiseValue = glm::smoothstep(0.1f, 1.0f, noiseValue);

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  secondColor = glm::mix(mainColor, secondColor, noiseValue);

  if (noiseValue > 0.2f)
  {
    // Calcula el valor sinusoide para crear líneas
    float sinValue = glm::sin(uv.y * 17.0f) * 0.1f;

    sinValue = glm::smoothstep(0.4f, 1.0f, sinValue);

    // Combina el color base con las líneas sinusoide
    secondColor = secondColor + glm::vec3(sinValue);
  }

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  thirdColor = glm::mix(secondColor, thirdColor, noiseValue);

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  mainColor = glm::mix(thirdColor, mainColor, noiseValue);

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  secondColor = glm::mix(mainColor, secondColor, noiseValue);

  color = Color(secondColor.x, secondColor.y, secondColor.z);

  fragment.color = color * fragment.intensity;

  return fragment;
}

Fragment terrainShader(Fragment &fragment)
{
  // Define the colors
  glm::vec3 spotColorGreen = glm::vec3(0.132f, 0.555f, 0.127f); // Forest green
  glm::vec3 spotColorBlue = glm::vec3(0.1f, 0.1f, 0.50f);       // Blue
  glm::vec3 earthColorLightGreen = glm::vec3(0.0f, 0.32f, 0.22f);
  glm::vec3 earthColorBrown = glm::vec3(0.74f, 0.41f, 0.22f); // Brown for earth
  glm::vec3 cloudColor = glm::vec3(1.0f, 1.0f, 1.0f);         // White

  // Convert 3D position on sphere to 2D UV
  glm::vec3 pos = glm::normalize(fragment.originalPos);
  float u = 0.5f + atan2(pos.z, pos.x) / (2.0f * glm::pi<float>());
  float v = 0.5f - asin(pos.y) / glm::pi<float>();
  glm::vec2 uv = glm::vec2(u, v);

  uv = glm::clamp(uv, 0.0f, 1.0f);

  // Set up the noise generator
  FastNoiseLite noiseGenerator;
  noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  float ox = 110.0f;
  float oy = 3500.0f;
  float z = 559.0f;

  // Generate the noise value
  float noiseValue = noiseGenerator.GetNoise((uv.x + ox) * z, (uv.y + oy) * z);

  glm::vec3 c;

  if (noiseValue < 0.4f)
  {
    // Water
    c = glm::mix(spotColorBlue, earthColorLightGreen, noiseValue);
    c = glm::smoothstep(glm::vec3(0.0f), glm::vec3(0.9f), c);
  }
  else if (noiseValue < 0.6f)
  {
    // Earth/Brown
    c = earthColorLightGreen;
  }
  else if (noiseValue < 0.8f)
  {
    // Land/Green
    c = spotColorGreen;
  }
  else if (noiseValue < 0.9f)
  {
    // Land/Green
    c = earthColorBrown;
  }
  else
  {
    // dejar azul
    c = spotColorBlue;
  }

  float cloudNoise = noiseGenerator.GetNoise((uv.x + ox) * z * 0.5f, (uv.y + oy) * z * 0.5f);
  cloudNoise = (cloudNoise + 1.0f) * 0.35f;
  cloudNoise = glm::smoothstep(0.0f, 1.0f, cloudNoise);

  // Interpolate between the cloud color and the planet color based on the cloud noise
  c = glm::mix(c, cloudColor, cloudNoise);

  // Convert glm::vec3 color to your Color class
  fragment.color = Color(c.r, c.g, c.b);

  return fragment;
}

Fragment sphereShader(Fragment &fragment)
{

  Color color;

  glm::vec3 mainColor = glm::vec3(0.7f, 0.2f, 0.6f);
  glm::vec3 secondColor = glm::vec3(0.5f, 0.2f, 0.5f);
  glm::vec3 thirdColor = glm::vec3(0.5f, 0.5f, 0.1f);

  glm::vec2 uv = glm::vec2(fragment.originalPos.x * 2.0 - 1.0, fragment.originalPos.y * 2.0 - 1.0);

  FastNoiseLite noiseGenerator;
  noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  float offsetX = 15000.0f;
  float offsetY = 1200.0f;
  float scale = 700.0f;

  // Genera el valor de ruido
  float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
  noiseValue = (noiseValue + 1.0f) * 0.5f;

  // aplicar un resplandor a las orillas de la esfera
  noiseValue = glm::smoothstep(0.0f, 1.2f, noiseValue);

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  secondColor = glm::mix(mainColor, secondColor, noiseValue);

  if (noiseValue > 0.5f)
  {
    // hacer que las orillas sean mas brillantes
    secondColor = secondColor + glm::vec3(0.1f);

    // Calcula el valor sinusoide para crear líneas
    float sinValue = glm::sin(uv.y * 25.0f) * 0.1f;

    sinValue = glm::smoothstep(0.8f, 1.0f, sinValue);
  }
  else
  {
    // Calcula el valor sinusoide para crear líneas
    float sinValue = glm::sin(uv.y * 47.0f) * 0.1f;

    sinValue = glm::smoothstep(0.1f, 1.0f, sinValue);

    // Combina el color base con las líneas sinusoide
    secondColor = secondColor + glm::vec3(sinValue);
  }

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  mainColor = glm::mix(thirdColor, mainColor, noiseValue);

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  secondColor = glm::mix(mainColor, secondColor, noiseValue);

  color = Color(secondColor.x, secondColor.y, secondColor.z);

  fragment.color = color * fragment.intensity;

  return fragment;
}

Fragment vibrantShader(Fragment &fragment)
{

  glm::vec3 cloudColor = glm::vec3(1.0f, 1.0f, 1.0f); // White
  glm::vec3 spotColorGreen = glm::vec3(0.8f, 0.0f, 0.9f);
  glm::vec3 spotColorBlue = glm::vec3(0.140f, 0.140f, 0.140f);
  glm::vec3 earthColorLightGreen = glm::vec3(0.3f, 1.0f, 0.6f);
  glm::vec3 earthColorBrown = glm::vec3(0.6f, 0.7f, 0.2f);
  glm::vec3 neon5 = glm::vec3(0.7f, 0.0f, 0.2f);

  // Convert 3D position on sphere to 2D UV
  glm::vec3 pos = glm::normalize(fragment.originalPos);
  float u = 0.5f + atan2(pos.z, pos.x) / (4.0f * glm::pi<float>());
  float v = 0.5f - asin(pos.y) / glm::pi<float>();
  glm::vec2 uv = glm::vec2(u, v);

  uv = glm::clamp(uv, 0.0f, 1.0f);

  // Set up the noise generator
  FastNoiseLite noiseGenerator;
  noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  float ox = 150000.0f;
  float oy = 1500.0f;
  float z = 3000.0f;

  // Generate the noise value
  float noiseValue = noiseGenerator.GetNoise((uv.x + ox) * z, (uv.y + oy) * z);

  glm::vec3 c;

  if (noiseValue < 0.4f)
  {
    // Water
    c = glm::mix(spotColorBlue, earthColorLightGreen, noiseValue);
    c = glm::smoothstep(glm::vec3(0.0f), glm::vec3(0.9f), c);
  }
  else if (noiseValue < 0.6f)
  {
    // Earth/Brown
    c = earthColorLightGreen;
  }
  else if (noiseValue < 0.8f)
  {
    // Land/Green
    c = spotColorGreen;
  }
  else if (noiseValue < 0.9f)
  {
    // Land/Green
    c = earthColorBrown;
  }
  else
  {
    // dejar azul
    c = spotColorBlue;
  }

  float cloudNoise = noiseGenerator.GetNoise((uv.x + ox) * z * 0.5f, (uv.y + oy) * z * 0.5f);
  cloudNoise = (cloudNoise + 0.70f) * 0.3f;
  cloudNoise = glm::smoothstep(0.0f, 1.0f, cloudNoise);

  // Interpolate between the cloud color and the planet color based on the cloud noise
  c = glm::mix(c, cloudColor, cloudNoise);

  // Convert glm::vec3 color to your Color class
  fragment.color = Color(c.r, c.g, c.b);

  return fragment;
}

Fragment nightSkyShader(Fragment &fragment)
{
  Color color;

  glm::vec3 mainColor = glm::vec3(0.130f, 0.130f, 0.130f);   // Color RGB (1, 1, 1)
  glm::vec3 secondColor = glm::vec3(0.730f, 0.730f, 0.730f); // Color RGB (0, 0, 0)

  glm::vec2 uv = glm::vec2(fragment.originalPos.x * 2.0 - 1.0, fragment.originalPos.y * 2.0 - 1.0);

  FastNoiseLite noiseGenerator;
  noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

  float offsetX = 8000.0f;
  float offsetY = 1000.0f;
  float scale = 40.0f;

  // Genera el valor de ruido
  float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
  noiseValue = (noiseValue + 1.0f) * 0.9f;

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  secondColor = glm::mix(mainColor, secondColor, noiseValue);

  if (noiseValue > 0.99f)
  {
    // Calcula el valor sinusoide para crear líneas
    float sinValue = glm::sin(uv.y * 20.0f) * 0.1f;

    sinValue = glm::smoothstep(0.8f, 1.0f, sinValue);

    // Combina el color base con las líneas sinusoide
    secondColor = secondColor + glm::vec3(sinValue);
  }

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  mainColor = glm::mix(mainColor, mainColor, noiseValue);

  // Interpola entre el color base y el color secundario basado en el valor de ruido
  secondColor = glm::mix(mainColor, secondColor, noiseValue);

  color = Color(secondColor.x, secondColor.y, secondColor.z);

  fragment.color = color;

  return fragment;
}

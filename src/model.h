#include <glm/glm.hpp>
#include <vector>
#include "uniforms.h"

enum ShaderType {
    FOLIAGE,     
    GAS_CLOUD,    
    SOLAR,       
    TERRAIN,      
    SPHERE,     
    VIBRANT,     
    NIGHT_SKY     
};

class Model {
    public:
    glm::mat4 modelMatrix;
    std::vector<glm::vec3> vertices;
    Uniforms uniforms;
    ShaderType currentShader;
};
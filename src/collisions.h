#ifndef COLLISIONS_H
#define COLLISIONS_H

// Tipos da cena e testes de intersecção (colisão) compartilhados entre
// main.cpp e collisions.cpp. Os testes de colisão em si ficam em
// collisions.cpp (exigência da especificação do trabalho).

#include <string>
#include <map>
#include <vector>
#include <cstddef>

#include <glad/glad.h>   // GLenum, GLuint
#include <glm/vec3.hpp>  // glm::vec3

// Um objeto carregado na cena virtual (malha + VAO + bounding box).
struct SceneObject
{
    std::string  name;            // Nome do objeto
    size_t       first_index;     // Índice do primeiro vértice em indices[]
    size_t       num_indices;     // Número de índices do objeto em indices[]
    GLenum       rendering_mode;  // Modo de rasterização (GL_TRIANGLES, ...)
    GLuint       vertex_array_object_id; // ID do VAO com os atributos do modelo
    glm::vec3    bbox_min;        // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Uma instância posicionada de um objeto no mundo (várias podem compartilhar a
// mesma malha — instâncias via diferentes Model matrix).
struct SceneEntity
{
    std::string mesh;

    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 localOffset;

    int  object_id;
    bool collidable;

    // Raio de aproximação para o objeto "aparecer" (0 = sempre visível).
    float appearRadius = 0.0f;

    // Índice na tabela de tipos de Pokémon (-1 = não é Pokémon capturável).
    int pokeType = -1;
};

// Variáveis globais definidas em main.cpp e usadas nos testes de colisão.
extern std::map<std::string, SceneObject> g_VirtualScene;
extern std::vector<SceneEntity>           g_Entities;

// Testes de intersecção (AABB no plano XZ) entre o jogador e os objetos da
// cena. Implementados em collisions.cpp.
//   FindCollidingEntityIndex: retorna o índice da 1ª entidade colidida ou -1.
//   CheckCollision: true se houver qualquer colisão.
int  FindCollidingEntityIndex(float playerX, float playerZ, float playerHalfSize);
bool CheckCollision(float playerX, float playerZ, float playerHalfSize);

#endif // COLLISIONS_H

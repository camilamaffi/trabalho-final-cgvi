// =============================================================================
//  collisions.cpp
//  Testes de intersecção (colisão) entre o jogador e os objetos da cena.
//  Mantido em arquivo separado conforme exigido pela especificação do trabalho.
//
//  A colisão usa Axis-Aligned Bounding Boxes (AABB) projetadas no plano XZ:
//  a partir da bounding box da malha (em g_VirtualScene) e da escala da
//  instância (SceneEntity), calculamos a meia-largura/profundidade do objeto e
//  testamos a sobreposição contra o "quadrado" do jogador. Tem propósito na
//  lógica do jogo: bloquear o jogador contra estruturas (ginásios/PokéStops) e
//  impedir que um Pokémon (re)apareça em cima de uma estrutura.
// =============================================================================

#include <cmath>     // fabs
#include <cstddef>   // size_t

#include "collisions.h"

// Colisão círculo-círculo no plano XZ (jogador vs estrutura sólida). Verdadeiro
// quando a distância entre os centros é menor que a soma dos raios.
bool CircleCollision(float playerX, float playerZ, float playerHalfSize,
                     float objX, float objZ, float objRadius)
{
    float dx  = playerX - objX;
    float dz  = playerZ - objZ;
    float sum = playerHalfSize + objRadius;
    return (dx*dx + dz*dz) < (sum*sum);
}

// Colisão esfera-esfera 3D (Pokébola vs Pokémon na captura). Verdadeiro quando a
// distância entre os centros é menor que a soma dos raios.
bool SphereCollision(const glm::vec3& a, float ra, const glm::vec3& b, float rb)
{
    glm::vec3 d = a - b;
    float sum   = ra + rb;
    return (d.x*d.x + d.y*d.y + d.z*d.z) < (sum*sum);
}

// Sobreposição de duas AABBs no plano XZ: há colisão quando as projeções em X e em
// Z se sobrepõem ao mesmo tempo (mesma lógica do teste jogador-objeto, generalizada
// para duas caixas quaisquer). Usada no (re)spawn dos Pokémon vs estruturas.
bool AABBOverlapXZ(float ax, float az, float ahx, float ahz,
                   float bx, float bz, float bhx, float bhz)
{
    bool overlapX = fabs(ax - bx) < (ahx + bhx);
    bool overlapZ = fabs(az - bz) < (ahz + bhz);
    return overlapX && overlapZ;
}

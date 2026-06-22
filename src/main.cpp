//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Computação Gráfica e Visualização I
//               Prof. Eduardo Gastal
//
//     CÓDIGO BASE PARA O TRABALHO FINAL
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Headers abaixo são específicos de C++
#include <set>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"

// Tipos da cena (SceneObject/SceneEntity), globais compartilhados e protótipos
// dos testes de colisão (implementados em collisions.cpp).
#include "collisions.h"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};


// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename, bool nearest = false, bool tiling = false, bool alpha = false); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// (A estrutura SceneObject agora é definida em "collisions.h", compartilhada
// entre main.cpp e collisions.cpp.)

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.6f;   // Ângulo em relação ao eixo Y (inclinação baixa estilo Pokémon GO)
float g_CameraDistance = 3.0f; // Distância da câmera para a origem

// Câmera livre: voa pelo cenário independente do jogador (posição com WASD na
// direção da visada, direção com o mouse). Alternada com a tecla C.
bool g_FreeCamera = false;
glm::vec3 g_FreeCamPos = glm::vec3(0.0f, 2.5f, 4.5f);

// Parâmetro de animação do balão da Equipe Rocket ao longo da curva de Bézier
// (vai de 0 a 4: um valor inteiro por segmento de um loop com 4 Béziers cúbicas).
float g_RocketT = 0.0f;

// Detecção de clique no mundo (balão e PokéStops; o teste em si é feito no loop).
bool   g_BalloonClickCheck = false;
bool   g_StopClickCheck    = false; // clique para coletar itens numa PokéStop
double g_PressPosX = 0.0, g_PressPosY = 0.0; // posição do mouse ao apertar
double g_ClickPosX = 0.0, g_ClickPosY = 0.0; // posição do clique (na soltura)

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Posição do personagem (cubo) controlado pelo teclado
float g_PlayerX = 0.0f;
float g_PlayerZ = 0.0f;

// Ângulo de orientação (yaw) do personagem: ele "olha" para onde anda,
// estilo avatar do Pokémon GO. Atualizado no loop de movimento.
float g_PlayerAngleY = 0.0f;

// Estado das teclas de movimento WASD / setas
bool g_KeyW = false;
bool g_KeyA = false;
bool g_KeyS = false;
bool g_KeyD = false;
bool g_KeyQ = false; // descer (câmera livre)
bool g_KeyE = false; // subir  (câmera livre)

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

enum class GameScene
{
    World,
    Capture
};

GameScene g_CurrentScene = GameScene::World;

// ===== Menu principal =====
// MainMenu  -> tela inicial (Iniciar / Como Jogar / Sair)
// HowToPlay -> tela de instruções (ESC volta ao menu)
// Playing   -> jogo rodando normalmente
enum class MenuState { MainMenu, HowToPlay, Playing };
MenuState g_MenuState = MenuState::MainMenu;

// Índice da opção selecionada no menu principal (0=Iniciar,1=ComoJogar,2=Sair)
int g_MenuSelection = 0;
int g_CaptureTargetIndex = -1;

// --- Mecânica de captura (mira -> arremesso -> resultado) ---
enum class CaptureState { Aiming, Throwing, Caught, Missed };
CaptureState g_CaptureState = CaptureState::Aiming;
float g_CaptureCharge  = 0.0f;  // 0..1, carrega segurando L
float g_CaptureThrowT  = 0.0f;  // 0..1, progresso do arremesso (Bézier)
float g_CaptureChance  = 0.0f;  // chance definida no momento do arremesso
float g_CaughtTimer    = 0.0f;  // tempo da animação de captura bem-sucedida
float g_MissTimer      = 0.0f;  // tempo da animação de erro (bola cai e some)
int   g_CaptureCP      = 0;     // CP do Pokémon selvagem (sorteado ao iniciar a captura)
glm::vec3 g_ThrowP0, g_ThrowP1, g_ThrowP2, g_ThrowP3; // pontos da Bézier do arremesso
bool g_KeyL = false;            // tecla L (carregar a captura)

// --- Armazenamento de Pokémons (interface 2D) ---
// Cada Pokémon capturado guarda seus atributos (HP, CP e dois ataques).
struct CapturedPokemon { int type; int hp; int cp; int atk1; int atk2; int placedGym = -1; };
std::vector<CapturedPokemon> g_Captured;        // lista de capturados (máx. 100)

// Ataques por TIPO de Pokémon (índice = pokeType: 0 = Pikachu, 1 = Charmander).
const int g_NumAttacks = 8;
const char* g_AttacksByType[3][g_NumAttacks] = {
    // Pikachu (elétrico)
    { "Choque do Trovao", "Investida", "Cauda de Ferro", "Raio Solar",
      "Ataque Rapido", "Bola Eletrica", "Trovao", "Agilidade" },
    // Charmander (fogo)
    { "Brasa", "Lanca-chamas", "Garra de Fogo", "Giro de Fogo",
      "Ataque Rapido", "Presas de Fogo", "Sopro de Fogo", "Roda de Fogo" },
    // Snorlax (normal)
    { "Investida", "Corpo Pesado", "Bocejo", "Lambida",
      "Soco", "Descanso", "Bloqueio", "Esmagar" },
};

// --- Evolução (estilo Pokémon GO: doces por espécie) ---------------------
// Tabelas paralelas indexadas pelo "tipo" do Pokémon (mesmo índice de
// g_PokeTypes). Ficam em escopo de arquivo para que o MouseButtonCallback
// também consiga consultá-las.
//   índices: 0=Pikachu 1=Charmander 2=Snorlax 3=Raichu 4=Charmeleon
const int g_EvolvesTo[5]   = {  3,  4, -1, -1, -1 }; // tipo destino (-1 = não evolui)
const int g_EvolveCost[5]  = {  3,  3,  0,  0,  0 }; // doces necessários
const int g_CandyFamily[5] = {  0,  1,  2,  0,  1 }; // família de doces (forma base)

// Doces acumulados por família (0=Pikachu, 1=Charmander, 2=Snorlax). Cada
// captura rende doces da família do Pokémon capturado.
int g_Candies[3] = { 0, 0, 0 };
const int CANDY_PER_CATCH = 3;

// Estado da animação de evolução (toca na tela de detalhe do armazenamento).
int   g_EvolvingIndex = -1;     // índice em g_Captured do Pokémon evoluindo (-1 = nenhum)
float g_EvolveTimer   = 0.0f;   // tempo decorrido da animação
int   g_EvolveToType  = -1;     // tipo destino da forma evoluída
bool  g_EvolveSwapped = false;  // já trocou a forma (no meio da animação)?
const float EVOLVE_DURATION = 2.6f; // duração total da animação (s)

// Botão "Evoluir" na tela de detalhe (coordenadas NDC). Usado tanto no desenho
// quanto no teste de clique (MouseButtonCallback), para casarem exatamente.
const float EVO_BTN_CX = 0.40f, EVO_BTN_CY = -0.58f;
const float EVO_BTN_HW = 0.22f, EVO_BTN_HH = 0.075f;

int  g_CapturedCount    = 0;     // == g_Captured.size() (atalho)
bool g_StorageOpen      = false; // janela de armazenamento aberta?
int  g_StorageScrollRow = 0;     // linha do topo (rolagem)
int  g_StorageDetail    = -1;    // índice do Pokémon em detalhe (-1 = grade)
// Ícone de acesso, no canto inferior esquerdo (coordenadas NDC).
const float UI_ICON_CX = -0.90f;
const float UI_ICON_CY = -0.84f;
const float UI_ICON_HH =  0.09f; // meia-altura do ícone em NDC

// --- Times (estilo Pokémon GO): escolhidos no modal de abertura do jogo ---
enum class Team { None, Red, Blue, Yellow };
Team g_PlayerTeam    = Team::None;
bool g_TeamModalOpen = true; // modal de escolha de time aparece ao abrir o jogo

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;
GLint g_light_tint_uniform; // tom/intensidade da luz (varia na captura)

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// estrutura para armezanar os objetos 3d do cenario
// (A estrutura SceneEntity agora é definida em "collisions.h", compartilhada
// entre main.cpp e collisions.cpp.)
std::vector<SceneEntity> g_Entities;

// ---- PokéStops e inventário -------------------------------------------------
// Um PokéStop é um ponto no mapa que, ao ser alcançado pelo jogador, fornece
// itens (Pokébola, Poção) e entra em "cooldown" antes de poder ser usado de novo.
struct PokeStop
{
    glm::vec3 position;
    float     cooldown; // tempo restante até reabilitar (0 = disponível)
};

std::vector<PokeStop> g_PokeStops;

// Tipos de item que uma PokéStop pode dar.
enum ItemType { ITEM_POKEBALL = 0, ITEM_POTION = 1, ITEM_BERRY = 2 };

// Itens "voando" da PokéStop até o jogador ao coletar. Cada um percorre uma
// curva de Bézier CÚBICA (p0..p3) do poste até o jogador — feedback visual.
struct FlyingItem { glm::vec3 p0, p1, p2, p3; float t; int type; };
std::vector<FlyingItem> g_FlyingItems;

// Ginásios: posição na base (no chão), o time dono (None = cinza/livre) e o
// tipo de Pokémon deixado lá (-1 = nenhum). Quando o jogador deixa um Pokémon,
// o ginásio passa a ter a cor do seu time.
struct Gym
{
    glm::vec3 position;
    Team      team;        // None = livre (cinza); senão, time dono
    int       pokemonType; // tipo do Pokémon deixado (-1 = nenhum)
};
std::vector<Gym> g_Gyms;

// Modal "deixar um Pokémon no ginásio?" (abre ao CLICAR num ginásio livre).
bool g_GymModalOpen    = false;
int  g_GymModalIndex   = -1;    // ginásio sendo perguntado
int  g_PlacingGymIndex = -1;    // armazenamento aberto em modo "colocar no ginásio"
bool g_GymClickCheck   = false; // pedido de teste de clique num ginásio (feito no loop)
bool g_PokeClickCheck  = false; // pedido de teste de clique num Pokémon (entra na captura)

// Inventário do jogador
int g_NumPokeballs = 0;
int g_NumPotions   = 0;
int g_NumBerries   = 0;

// Mensagem temporária mostrada na tela (ex.: "PokéStop! +2 Pokébola")
std::string g_Message;
float       g_MessageTimer = 0.0f; // segundos restantes mostrando a mensagem

// (Protótipos de CheckCollision/FindCollidingEntityIndex estão em "collisions.h".)

// Avalia um ponto de uma curva de Bézier cúbica com pontos de controle
// p0..p3 no parâmetro t em [0,1]:  B(t) = (1-t)^3 p0 + 3(1-t)^2 t p1
//                                       + 3(1-t) t^2 p2 + t^3 p3
glm::vec3 EvalCubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
    float u = 1.0f - t;
    return (u*u*u) * p0
         + (3.0f*u*u*t) * p1
         + (3.0f*u*t*t) * p2
         + (t*t*t) * p3;
}

// Desenha um quad de interface 2D centrado em (cx,cy) com meios-tamanhos (hw,hh)
// em coordenadas NDC. Requer que view e projection estejam como identidade.
void DrawUIQuad(float cx, float cy, float hw, float hh, int objid)
{
    glm::mat4 m = Matrix_Translate(cx, cy, 0.0f)
                * Matrix_Rotate_X(3.141592f / 2.0f)
                * Matrix_Scale(hw, 1.0f, hh);
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(m));
    glUniform1i(g_object_id_uniform, objid);
    DrawVirtualObject("the_plane");
}

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Semente para a aleatoriedade da chance de captura.
    srand((unsigned int) time(NULL));

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "Pokémon GO", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // FONTE: textura red_brick de Poly Haven (CC0): https://polyhaven.com/a/red_brick
    LoadTextureImage("../../data/red_brick_diff_1k.jpg"); // TextureImage0
    LoadTextureImage("../../data/map.png");               // TextureImage1
    LoadTextureImage("../../data/forest.png");            // TextureImage2 - floresta que tila fora do mapa
    // rocket_r.png: gerada pela dupla com Python/PIL (fonte Arial Black) — sem fonte externa
    LoadTextureImage("../../data/rocket_r.png");          // TextureImage3 - "R" da Equipe Rocket no balão
    // Texturas dos demais objetos (paleta do boneco via UV; resto via triplanar):
    LoadTextureImage("../../data/adventurer_palette.png", true,  false); // TextureImage4 - paleta do jogador (NEAREST)
    LoadTextureImage("../../data/tex_pikachu.png",        false, true);  // TextureImage5 - pikachu
    LoadTextureImage("../../data/tex_stone.png",          false, true);  // TextureImage6 - pedra (ginásio)
    LoadTextureImage("../../data/tex_metal.png",          false, true);  // TextureImage7 - metal (PokéStop)
    LoadTextureImage("../../data/tex_dark.png",           false, true);  // TextureImage8 - tecido escuro (balão)
    LoadTextureImage("../../data/tex_grass.png",          false, true);  // TextureImage9 - grama (borda do mapa)
    LoadTextureImage("../../data/pokebola_tex.png");      // TextureImage10 - pokébola (via UV)
    LoadTextureImage("../../data/icon_bag.png");          // TextureImage11 - ícone do armazenamento (UI)
    LoadTextureImage("../../data/pikachu_icon.png");      // TextureImage12 - miniatura do pikachu (UI)
    LoadTextureImage("../../data/tex_charmander.png",     false, true);  // TextureImage13 - charmander (via UV)
    LoadTextureImage("../../data/charmander_icon.png");   // TextureImage14 - miniatura do charmander (UI)
    // FONTE: mapa-captura.png - imagem de fundo da cena de captura, gerada pelo ChatGPT (OpenAI).
    LoadTextureImage("../../data/mapa-captura.png");      // TextureImage15 - fundo da cena de captura
    // Logos dos times (Pokémon GO), com transparência (alpha) para recortar no modal.
    LoadTextureImage("../../data/valor.png",    false, false, true); // TextureImage16 - logo Valor (vermelho)
    LoadTextureImage("../../data/mystic.png",   false, false, true); // TextureImage17 - logo Mystic (azul)
    LoadTextureImage("../../data/instinct.png", false, false, true); // TextureImage18 - logo Instinct (amarelo)
    LoadTextureImage("../../data/tex_snorlax.png", false, true);      // TextureImage19 - snorlax (via UV)
    // Atlas dos modelos de terceiros: NEAREST (sem mipmap). As texturas são
    // "folhas de expressão" (grades de olhos/bocas) e o filtro linear+mipmap
    // borraria as células vizinhas, bagunçando o rosto. NEAREST amostra a célula
    // exata (o modelo é cel-shaded/cor chapada, então fica nítido, não pixelado).
    // Charmeleon e Raichu têm partes com TRANSPARÊNCIA (ex.: bochechas) -> carrega
    // com alpha (RGBA) e o shader descarta os fragmentos transparentes.
    LoadTextureImage("../../data/tex_charmeleon.png", true, false, true); // TextureImage20 - charmeleon (atlas, via UV, com alpha)
    LoadTextureImage("../../data/tex_charmander_hd.png", true, false); // TextureImage21 - charmander HD (atlas, via UV)
    LoadTextureImage("../../data/tex_pikachu_hd.png", true, false);    // TextureImage22 - pikachu HD (atlas, via UV)
    LoadTextureImage("../../data/tex_raichu.png", true, false, true);  // TextureImage23 - raichu (atlas, via UV, com alpha)
    LoadTextureImage("../../data/tex_snorlax_hd.png", true, false);    // TextureImage24 - snorlax HD (atlas texturas+cores do glTF, via UV)

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel cubemodel("../../data/cube.obj");
    ComputeNormals(&cubemodel);
    BuildTrianglesAndAddToVirtualScene(&cubemodel);

    // pikachu_final.obj: modelo feito pela própria dupla (sem fonte externa).
    ObjModel pikachumodel("../../data/pikachu_final.obj");
    ComputeNormals(&pikachumodel);
    BuildTrianglesAndAddToVirtualScene(&pikachumodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    // Boneco do jogador (modelo low-poly "Adventurer").
    // FONTE: "Adventurer" por Quaternius, licença CC0 (domínio público), via Poly Pizza:
    //   https://poly.pizza/m/5EGWBMpuXq  -  https://quaternius.com/
    //   Modelo original em GLB; convertido para OBJ com as cores da paleta assadas
    //   como cor por vértice. Veja data/adventurer.obj e data/FONTES.txt.
    ObjModel adventurermodel("../../data/adventurer.obj");
    ComputeNormals(&adventurermodel);
    BuildTrianglesAndAddToVirtualScene(&adventurermodel);

    // Esfera (usada achatada como o disco redondo do PokéStop).
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    // Ginásios (modelo do Pokémon GO): uma variante por cor de time + uma cinza
    // para os ginásios livres. Mesma geometria, só muda a cor da torre principal;
    // as cores são por vértice (assadas a partir dos materiais do glTF original).
    // FONTE: pacote "pokemon go" (glTF) em data/pokemon-go-assets/ — modelo de
    //   terceiros; convertido pela dupla para OBJ com cor por vértice e separado
    //   em variantes graygym/redgym/blegym/yellowgym.
    ObjModel graygymmodel("../../data/graygym.obj");
    ComputeNormals(&graygymmodel);
    BuildTrianglesAndAddToVirtualScene(&graygymmodel);

    ObjModel redgymmodel("../../data/redgym.obj");
    ComputeNormals(&redgymmodel);
    BuildTrianglesAndAddToVirtualScene(&redgymmodel);

    ObjModel blegymmodel("../../data/blegym.obj");
    ComputeNormals(&blegymmodel);
    BuildTrianglesAndAddToVirtualScene(&blegymmodel);

    ObjModel yellowgymmodel("../../data/yellowgym.obj");
    ComputeNormals(&yellowgymmodel);
    BuildTrianglesAndAddToVirtualScene(&yellowgymmodel);

    // PokéStops (mesmo pacote do Pokémon GO): "openstop" (disponível, azul) e
    // "closedstop" (em cooldown, cinza). Cor por vértice, convertidos do glTF.
    ObjModel openstopmodel("../../data/openstop.obj");
    ComputeNormals(&openstopmodel);
    BuildTrianglesAndAddToVirtualScene(&openstopmodel);

    ObjModel closedstopmodel("../../data/closedstop.obj");
    ComputeNormals(&closedstopmodel);
    BuildTrianglesAndAddToVirtualScene(&closedstopmodel);

    // Itens das PokéStops (mesmo pacote do Pokémon GO): poção e fruta (berry).
    // Cor por vértice, convertidos do glTF.
    ObjModel potionmodel("../../data/potion.obj");
    ComputeNormals(&potionmodel);
    BuildTrianglesAndAddToVirtualScene(&potionmodel);

    ObjModel berrymodel("../../data/berry.obj");
    ComputeNormals(&berrymodel);
    BuildTrianglesAndAddToVirtualScene(&berrymodel);

    // Pokébola (modelo feito pela dupla; esfera UV "Esfera_UV").
    ObjModel pokebolamodel("../../data/pokebola.obj");
    ComputeNormals(&pokebolamodel);
    BuildTrianglesAndAddToVirtualScene(&pokebolamodel);

    // Charmander: segundo tipo de Pokémon (modelo "Cubo", normalizado pela dupla
    // para ficar centrado na origem e com altura 1; possui coordenadas de textura).
    ObjModel charmandermodel("../../data/charmander.obj");
    ComputeNormals(&charmandermodel);
    BuildTrianglesAndAddToVirtualScene(&charmandermodel);

    // Snorlax: 3º tipo de Pokémon (modelo feito pela dupla; shape "snorlax",
    // normalizado: centrado, base no chão, altura 1; possui UVs).
    ObjModel snorlaxmodel("../../data/snorlax.obj");
    ComputeNormals(&snorlaxmodel);
    BuildTrianglesAndAddToVirtualScene(&snorlaxmodel);

    // Charmeleon: forma EVOLUÍDA do Charmander. Convertido de charmeleon.glb
    // (pacote de terceiros) para OBJ com COR-POR-VÉRTICE assada da textura do
    // glTF; normalizado (centrado XZ, base y=0, altura 1). Shape "charmeleon".
    ObjModel charmeleonmodel("../../data/charmeleon.obj");
    ComputeNormals(&charmeleonmodel);
    BuildTrianglesAndAddToVirtualScene(&charmeleonmodel);

    // Charmander HD: modelo de EXIBIÇÃO (de terceiros, charmander.glb) usado nas
    // telas grandes (captura/detalhe/ginásio). No mapa e na miniatura segue o
    // modelo simples da dupla (charmander.obj). Textura real via atlas+UV.
    ObjModel charmanderhdmodel("../../data/charmander_hd.obj");
    ComputeNormals(&charmanderhdmodel);
    BuildTrianglesAndAddToVirtualScene(&charmanderhdmodel);

    // Pikachu HD: modelo de EXIBIÇÃO (de terceiros, 025_pikachu.glb) usado nas
    // telas grandes. No mapa/miniatura segue o modelo simples da dupla.
    ObjModel pikachuhdmodel("../../data/pikachu_hd.obj");
    ComputeNormals(&pikachuhdmodel);
    BuildTrianglesAndAddToVirtualScene(&pikachuhdmodel);

    // Raichu: forma EVOLUÍDA do Pikachu (de terceiros, raichu.glb). Modelo único
    // (sem distinção base/HD); textura real via atlas+UV.
    ObjModel raichumodel("../../data/raichu.obj");
    ComputeNormals(&raichumodel);
    BuildTrianglesAndAddToVirtualScene(&raichumodel);

    // Snorlax HD: modelo de EXIBIÇÃO (de terceiros, snorlax_pokemon.glb) usado nas
    // telas grandes. No mapa/miniatura segue o snorlax simples da dupla.
    ObjModel snorlaxhdmodel("../../data/snorlax_hd.obj");
    ComputeNormals(&snorlaxhdmodel);
    BuildTrianglesAndAddToVirtualScene(&snorlaxhdmodel);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    #define CUBE              1
    #define PLANE             2
    #define PIKACHU           3
    #define TREE              4
    #define FOREST_WALL       5
    #define PLAYER            6
    #define POKESTOP          7
    #define POKESTOP_COOLDOWN 8
    #define GYM               9
    #define GYM_TOP           10
    #define ROCKET            11
    #define ROCKET_BASKET     12
    #define ROCKET_R          13
    #define CAPTURE_BG        14
    #define POKEBALL          15
    #define UI_PANEL          16
    #define UI_ICON           17
    #define UI_THUMB          18
    #define CHARMANDER        19
    #define UI_THUMB_2        20
    #define UI_BORDER         21
    #define UI_TEAM_RED       22
    #define UI_TEAM_BLUE      23
    #define UI_TEAM_YELLOW    24
    #define GYM_RED           25
    #define GYM_BLUE          26
    #define GYM_YELLOW        27
    #define GYM_MODEL         28
    #define UI_LOGO_VALOR     29
    #define UI_LOGO_MYSTIC    30
    #define UI_LOGO_INSTINCT  31
    #define SNORLAX           32
    #define CHARMELEON        33
    #define CHARMANDER_HD     34
    #define PIKACHU_HD        35
    #define RAICHU            36
    #define SNORLAX_HD        37

    // Tabela de tipos de Pokémon. Centraliza tudo que varia entre um tipo e
    // outro (malha, textura/object_id, miniatura na UI, e os ajustes de
    // posição/escala/rotação usados na cena de captura e na tela de detalhe),
    // de modo que o resto do código trate qualquer Pokémon de forma genérica.
    struct PokemonType
    {
        const char* name;        // "Pikachu", "Charmander"
        const char* mesh;        // malha BASE (mapa + miniatura do armazenamento)
        int   modelObjId;        // object_id que texturiza a malha base
        int   thumbObjId;        // object_id que texturiza a miniatura (UI)
        glm::vec3 detailOffset;  // offset do modelo na tela de detalhe
        float detailScale;       // escala do modelo na tela de detalhe
        float facingOffset;      // ajuste de yaw para encarar a câmera na captura

        // --- Modelo de EXIBIÇÃO (HD) ---
        // Usado SÓ nas telas grandes: captura, detalhe do armazenamento e o
        // Pokémon flutuando no ginásio. No mapa e na miniatura usa-se a malha
        // base acima. dispMesh == nullptr => não há HD, cai na malha base.
        const char* dispMesh;        // malha HD (ou nullptr)
        int   dispObjId;             // object_id da malha HD
        glm::vec3 dispDetailOffset;  // offset no detalhe/ginásio (HD)
        float dispDetailScale;       // escala no detalhe/ginásio (HD)
        glm::vec3 dispCaptureOffset; // offset na captura (HD)
        float dispCaptureScale;      // escala na captura (HD)
        float dispFacing;            // yaw para encarar a câmera (HD)
    };
    const PokemonType g_PokeTypes[] = {
        // 0: Pikachu — malha BASE "Cube" (pikachu_final.obj, da dupla) no mapa e na
        // miniatura; malha HD "pikachu_hd" (025_pikachu.glb, terceiros) na
        // captura/detalhe/ginásio. HD normalizado base-0, altura 1.
        { "Pikachu",    "Cube", PIKACHU,    UI_THUMB,   glm::vec3(-0.82f, 0.0f, -0.06f), 0.105f, -1.5707963f,
          "pikachu_hd", PIKACHU_HD, glm::vec3(0.0f, -0.5f, 0.0f), 0.28f, glm::vec3(0.0f, 0.0f, 0.0f), 0.35f, 0.0f },
        // 1: Charmander — malha BASE "Cubo" (charmander.obj, da dupla) no mapa e na
        // miniatura; malha HD "charmander_hd" (charmander.glb, terceiros) na
        // captura/detalhe/ginásio. HD é normalizado base-0, altura 1.
        { "Charmander", "Cubo", CHARMANDER, UI_THUMB_2, glm::vec3( 0.0f,  0.0f,  0.0f),  0.28f, -1.5707963f,
          "charmander_hd", CHARMANDER_HD, glm::vec3(0.0f, -0.5f, 0.0f), 0.28f, glm::vec3(0.0f, 0.0f, 0.0f), 0.35f, 0.0f },
        // 2: Snorlax — malha BASE "snorlax" (da dupla) no mapa e na miniatura; malha
        // HD "snorlax_hd" (snorlax_pokemon.glb, terceiros) na captura/detalhe/ginásio.
        { "Snorlax",    "snorlax", SNORLAX,  UI_THUMB,   glm::vec3( 0.0f,  0.0f,  0.0f),  0.22f, -1.5707963f,
          "snorlax_hd", SNORLAX_HD, glm::vec3(0.0f, -0.5f, 0.0f), 0.22f, glm::vec3(0.0f, 0.0f, 0.0f), 0.45f, 0.0f },

        // --- FORMAS EVOLUÍDAS -------------------------------------------------
        // 3: Raichu (evolução do Pikachu) — modelo real (raichu.obj, textura via
        // atlas+UV; shape "raichu", altura 1). Forma única (sem base/HD). A cauda
        // é comprida, então detailScale menor para caber girando no painel.
        { "Raichu",     "raichu", RAICHU,   UI_THUMB,   glm::vec3( 0.0f, -0.5f, 0.0f), 0.18f, 0.0f,
          nullptr, 0, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 0.0f },
        // 4: Charmeleon (evolução do Charmander) — modelo real (charmeleon.obj,
        // textura real via atlas+UV). Forma única (sem distinção base/HD).
        { "Charmeleon", "charmeleon", CHARMELEON, UI_THUMB_2, glm::vec3( 0.0f, -0.5f, 0.0f), 0.30f, -1.5707963f,
          nullptr, 0, glm::vec3(0.0f), 0.0f, glm::vec3(0.0f), 0.0f, 0.0f },
    };

    // Configuração dos objetos da cena (feita uma única vez, fora do loop)
    {
        // Pokémon espalhados pelo mapa em posições ALEATÓRIAS a cada execução
        // (semente definida por srand(time) lá no início). Cada um só aparece
        // quando o jogador chega perto (appearRadius) e some ao se afastar.
        // Sempre 6 Pikachus e 6 Charmanders.
        std::vector<glm::vec3> placedSpots; // posições já usadas (para não sobrepor)
        auto randomSpot = [&placedSpots](float yBase) -> glm::vec3 {
            const float MIN_SEP = 1.2f; // distância mínima entre dois Pokémon
            float x = 0.0f, z = 0.0f;
            for (int tries = 0; tries < 100; ++tries)
            {
                x = ((float)rand() / (float)RAND_MAX) * 7.0f - 3.5f; // [-3.5, 3.5]
                z = ((float)rand() / (float)RAND_MAX) * 7.0f - 3.5f;
                if (x*x + z*z < 1.5f*1.5f) // evita nascer em cima do jogador (origem)
                    continue;
                bool tooClose = false;
                for (const glm::vec3& p : placedSpots)
                {
                    float dx = x - p.x, dz = z - p.z;
                    if (dx*dx + dz*dz < MIN_SEP*MIN_SEP) { tooClose = true; break; }
                }
                if (!tooClose)
                    break; // achou um lugar livre
            }
            glm::vec3 s(x, yBase, z);
            placedSpots.push_back(s);
            return s;
        };

        for (int i = 0; i < 6; ++i)
        {
            SceneEntity pikachu;
            pikachu.mesh = "Cube";
            pikachu.position = randomSpot(-0.96f);
            pikachu.scale = glm::vec3(0.1f);
            pikachu.localOffset = glm::vec3(-0.82f, 0.0f, -0.06f);
            pikachu.object_id = PIKACHU;
            pikachu.collidable = true;
            pikachu.appearRadius = 1.2f; // distância em que o pikachu surge
            pikachu.pokeType = 0;        // tipo 0 = Pikachu (ver g_PokeTypes)
            g_Entities.push_back(pikachu);
        }

        // Charmanders (segundo tipo de Pokémon). Mesmo comportamento de
        // aparecer/sumir e captura por colisão dos pikachus, também em posições
        // aleatórias.
        for (int i = 0; i < 6; ++i)
        {
            SceneEntity charmander;
            charmander.mesh = "Cubo";
            charmander.position = randomSpot(-1.1f);
            charmander.scale = glm::vec3(0.35f);
            // Modelo centrado na origem (altura 1): sobe meia-altura para a base
            // tocar o chão, deixando a posição.y como o ponto de apoio.
            charmander.localOffset = glm::vec3(0.0f, 0.5f, 0.0f);
            charmander.object_id = CHARMANDER;
            charmander.collidable = true;
            charmander.appearRadius = 1.2f;
            charmander.pokeType = 1;     // tipo 1 = Charmander (ver g_PokeTypes)
            g_Entities.push_back(charmander);
        }

        // Snorlax (terceiro tipo): grande e mais raro (4 no mapa). Mesmo
        // comportamento de aparecer/sumir e captura por colisão.
        for (int i = 0; i < 4; ++i)
        {
            SceneEntity snorlax;
            snorlax.mesh = "snorlax";
            snorlax.position = randomSpot(-1.1f);
            snorlax.scale = glm::vec3(0.45f);            // maior que os outros
            snorlax.localOffset = glm::vec3(0.0f, 0.5f, 0.0f); // base no chão
            snorlax.object_id = SNORLAX;
            snorlax.collidable = true;
            snorlax.appearRadius = 1.2f;
            snorlax.pokeType = 2;        // tipo 2 = Snorlax (ver g_PokeTypes)
            g_Entities.push_back(snorlax);
        }

        SceneEntity plane;
        plane.mesh = "the_plane";
        plane.position = glm::vec3(0.0f, -1.1f, 0.0f);
        plane.scale = glm::vec3(15.0f, 1.0f, 15.0f);
        plane.localOffset = glm::vec3(0.0f);
        plane.object_id = PLANE;
        plane.collidable = false;
        g_Entities.push_back(plane);

        // PokéStops espalhados pelo mapa: 13 instâncias da mesma malha base.
        const glm::vec3 pokestopSpots[] = {
            glm::vec3( 1.8f, -1.1f,  1.5f),
            glm::vec3(-2.2f, -1.1f, -0.8f),
            glm::vec3( 0.2f, -1.1f, -2.8f),
            glm::vec3(-3.0f, -1.1f,  2.4f),
            glm::vec3( 1.0f, -1.1f, -1.0f),
            glm::vec3(-1.0f, -1.1f,  1.0f),
            glm::vec3( 2.6f, -1.1f,  0.8f),
            glm::vec3(-2.8f, -1.1f, -2.6f),
            glm::vec3( 1.4f, -1.1f,  3.0f),
            glm::vec3(-1.6f, -1.1f,  3.2f),
            glm::vec3( 3.4f, -1.1f, -1.0f),
            glm::vec3(-3.6f, -1.1f,  0.4f),
            glm::vec3( 2.0f, -1.1f, -3.2f),
        };

        for (const glm::vec3& spot : pokestopSpots)
        {
            PokeStop stop;
            stop.position = spot;
            stop.cooldown = 0.0f;
            g_PokeStops.push_back(stop);
        }

        // Ginásios: 5 instâncias do mesmo modelo, espalhadas pelo mapa. Começam
        // livres (Team::None = cinza) e sem Pokémon. Posições no chão (y = -1.1).
        g_Gyms.push_back({ glm::vec3( 3.7f, -1.1f, -3.5f), Team::None, -1 });
        g_Gyms.push_back({ glm::vec3(-3.9f, -1.1f,  3.6f), Team::None, -1 });
        g_Gyms.push_back({ glm::vec3(-4.0f, -1.1f, -3.7f), Team::None, -1 });
        g_Gyms.push_back({ glm::vec3( 4.1f, -1.1f,  3.4f), Team::None, -1 });
        g_Gyms.push_back({ glm::vec3( 0.0f, -1.1f,  4.3f), Team::None, -1 });
    }

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // ===== MENU PRINCIPAL / COMO JOGAR =====
        if (g_MenuState != MenuState::Playing)
        {
            glClearColor(0.20f, 0.45f, 0.75f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(g_GpuProgramID);

            if (g_MenuState == MenuState::MainMenu)
            {
                // Título
                {
                    std::string title = "Trabalho Final - INF01047";
                    float ts = 3.0f;
                    float tw = title.size() * TextRendering_CharWidth(window) * ts;
                    TextRendering_PrintString(window, title, -tw * 0.5f, 0.65f, ts);
                }

                // Opções do menu
                const char* labels[3] = { "  Iniciar Jogo  ", "  Como Jogar  ", "     Sair     " };
                const float itemCY[3] = { 0.05f, -0.20f, -0.45f };
                for (int i = 0; i < 3; ++i)
                {
                    std::string txt = (g_MenuSelection == i ? "> " : "  ");
                    txt += labels[i];
                    if (g_MenuSelection == i) txt += " <";
                    float scale = (g_MenuSelection == i) ? 2.2f : 1.8f;
                    float tw = txt.size() * TextRendering_CharWidth(window) * scale;
                    TextRendering_PrintString(window, txt, -tw * 0.5f, itemCY[i], scale);
                }

                // Rodapé
                {
                    std::string hint = "W/S ou Setas para navegar  |  Enter ou clique para selecionar";
                    float hs = 0.9f;
                    float hw = hint.size() * TextRendering_CharWidth(window) * hs;
                    TextRendering_PrintString(window, hint, -hw * 0.5f, -0.80f, hs);
                }
            }
            else // HowToPlay
            {
                std::string title = "Como Jogar";
                float ts = 2.5f;
                float tw = title.size() * TextRendering_CharWidth(window) * ts;
                TextRendering_PrintString(window, title, -tw * 0.5f, 0.72f, ts);

                float ls = 1.15f;
                float charW = TextRendering_CharWidth(window) * ls;
                float xMid = 0.02f;
                float gap  = 0.04f;
 
                struct HTPRow { const char* key; const char* desc; };
                HTPRow rows[] = {
                    { "WASD ou Setas",   "Mover o personagem"                   },
                    { "Mouse (arrastar)","Rotacionar a camera"                   },
                    { "C",              "Alternar camera (3a pessoa / livre)"    },
                    { "L (segurar)",    "Carregar e lancar Pokebola na captura"  },
                    { "ESC",            "Voltar / Sair da cena atual"            },
                };
                float startY = 0.38f;
                float stepY  = 0.155f;
                int nRows = (int)(sizeof(rows)/sizeof(rows[0]));
                for (int i = 0; i < nRows; ++i)
                {
                    float y = startY - i * stepY;
                    float kw = strlen(rows[i].key) * charW;
                    TextRendering_PrintString(window, rows[i].key,  xMid - gap - kw, y, ls);
                    TextRendering_PrintString(window, rows[i].desc, xMid + gap,      y, ls);
                }

                std::string back = "[ ESC / Enter / Clique para voltar ]";
                float bs = 1.1f;
                float bw = back.size() * TextRendering_CharWidth(window) * bs;
                TextRendering_PrintString(window, back, -bw * 0.5f, -0.82f, bs);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // azul de céu (frestas/topo parecem céu)

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);

        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().
        // Delta time para movimento suave
        static float prev_time = (float)glfwGetTime();
        float curr_time = (float)glfwGetTime();
        float delta_t = curr_time - prev_time;
        if (delta_t > 0.1f) delta_t = 0.1f; // evita salto na primeira frame após o menu
        prev_time = curr_time;

        float speed = 2.0f;

        // Próxima posição do jogador
        float nextPlayerX = g_PlayerX;
        float nextPlayerZ = g_PlayerZ;

        // Movimento relativo à câmera
        float forwardX = -sin(g_CameraTheta);
        float forwardZ = -cos(g_CameraTheta);
        float rightX   =  cos(g_CameraTheta);
        float rightZ   = -sin(g_CameraTheta);

        if (!g_TeamModalOpen && !g_GymModalOpen && g_CurrentScene == GameScene::World && g_FreeCamera)
        {
            // Câmera livre: WASD movem na HORIZONTAL (forward/right projetados no
            // plano do chão, então não afunda no mapa). E/Q sobem/descem.
            float step = 2.5f * delta_t;
            if (g_KeyW) g_FreeCamPos += glm::vec3(forwardX, 0.0f, forwardZ) * step;
            if (g_KeyS) g_FreeCamPos -= glm::vec3(forwardX, 0.0f, forwardZ) * step;
            if (g_KeyA) g_FreeCamPos -= glm::vec3(rightX, 0.0f, rightZ) * step;
            if (g_KeyD) g_FreeCamPos += glm::vec3(rightX, 0.0f, rightZ) * step;
            if (g_KeyE) g_FreeCamPos.y += step; // sobe
            if (g_KeyQ) g_FreeCamPos.y -= step; // desce

            // Trava de altura: não afunda no mapa nem sobe acima das árvores de
            // borda (paredes de floresta vão de y=-1.1 até ~7.9).
            const float FREECAM_MIN_Y = -0.8f;
            const float FREECAM_MAX_Y =  7.5f; // ~topo das árvores de borda
            if (g_FreeCamPos.y < FREECAM_MIN_Y) g_FreeCamPos.y = FREECAM_MIN_Y;
            if (g_FreeCamPos.y > FREECAM_MAX_Y) g_FreeCamPos.y = FREECAM_MAX_Y;

            // Trava horizontal: não passa dos contornos do mapa (paredes em ±5.5).
            const float FREECAM_XZ_LIMIT = 5.2f;
            if (g_FreeCamPos.x < -FREECAM_XZ_LIMIT) g_FreeCamPos.x = -FREECAM_XZ_LIMIT;
            if (g_FreeCamPos.x >  FREECAM_XZ_LIMIT) g_FreeCamPos.x =  FREECAM_XZ_LIMIT;
            if (g_FreeCamPos.z < -FREECAM_XZ_LIMIT) g_FreeCamPos.z = -FREECAM_XZ_LIMIT;
            if (g_FreeCamPos.z >  FREECAM_XZ_LIMIT) g_FreeCamPos.z =  FREECAM_XZ_LIMIT;
        }
        else if (!g_TeamModalOpen && !g_GymModalOpen && g_CurrentScene == GameScene::World)
        {
            if (g_KeyW) { nextPlayerX += forwardX * speed * delta_t; nextPlayerZ += forwardZ * speed * delta_t; }
            if (g_KeyS) { nextPlayerX -= forwardX * speed * delta_t; nextPlayerZ -= forwardZ * speed * delta_t; }
            if (g_KeyA) { nextPlayerX -= rightX * speed * delta_t;   nextPlayerZ -= rightZ * speed * delta_t; }
            if (g_KeyD) { nextPlayerX += rightX * speed * delta_t;   nextPlayerZ += rightZ * speed * delta_t; }

            // Avatar olha para a direção em que está andando (estilo Pokémon GO).
            // Calculamos o yaw a partir do vetor de movimento desejado neste frame.
            float moveDirX = nextPlayerX - g_PlayerX;
            float moveDirZ = nextPlayerZ - g_PlayerZ;
            if (fabs(moveDirX) > 1e-5f || fabs(moveDirZ) > 1e-5f)
                g_PlayerAngleY = atan2(moveDirX, moveDirZ);

            // Os Pokémon NÃO bloqueiam o jogador e NÃO entram na captura ao
            // encostar — o jogador atravessa o corpo deles (que segue exibido no
            // mapa). A captura agora é iniciada por CLIQUE no Pokémon (testado no
            // loop de desenho, junto da projeção para a tela).
            float playerHalfSize = 0.075f;

            // Bloqueio por ESTRUTURAS sólidas (ginásios e PokéStops): a prioridade
            // é o jogador — ele não as atravessa, simplesmente não anda. Usa
            // colisão círculo-círculo (CircleCollision em collisions.cpp).
            const float GYM_COLLISION_R  = 0.45f; // raio de bloqueio do ginásio
            const float STOP_COLLISION_R = 0.30f; // raio de bloqueio da PokéStop
            bool blockedByStructure = false;
            for (const Gym& gymObj : g_Gyms)
            {
                if (CircleCollision(nextPlayerX, nextPlayerZ, playerHalfSize,
                                    gymObj.position.x, gymObj.position.z, GYM_COLLISION_R))
                { blockedByStructure = true; break; }
            }
            if (!blockedByStructure)
            {
                for (const PokeStop& stopObj : g_PokeStops)
                {
                    if (CircleCollision(nextPlayerX, nextPlayerZ, playerHalfSize,
                                        stopObj.position.x, stopObj.position.z, STOP_COLLISION_R))
                    { blockedByStructure = true; break; }
                }
            }

            if (!blockedByStructure)
            {
                g_PlayerX = nextPlayerX;
                g_PlayerZ = nextPlayerZ;
            }
            // (Se houve bloqueio por estrutura, o jogador simplesmente não anda.)
        }

        if (g_CurrentScene == GameScene::Capture
            && (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
                || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS))
        {
            g_CurrentScene = GameScene::World;
            g_CaptureTargetIndex = -1;
        }

        // ---- Máquina de estados da captura (mira / arremesso / resultado) ----
        if (g_CurrentScene == GameScene::Capture && g_CaptureTargetIndex >= 0)
        {
            const SceneEntity& tgt = g_Entities[g_CaptureTargetIndex];
            glm::vec3 tc = glm::vec3(tgt.position.x, tgt.position.y + 0.25f, tgt.position.z);
            // posição "na mão" (em frente à câmera de captura, mais embaixo)
            glm::vec3 capCam = tc + glm::vec3(1.6f, 0.5f, 0.0f);
            glm::vec3 toT = tc - capCam;
            float tl = sqrtf(toT.x*toT.x + toT.y*toT.y + toT.z*toT.z);
            glm::vec3 held = capCam + (toT / tl) * 0.45f + glm::vec3(0.0f, -0.18f, 0.0f);

            // Esfera de colisão do Pokémon para o TESTE DE INTERSEÇÃO da captura:
            // centro = tc; raio ~ metade da maior dimensão do modelo (já com escala).
            // Usa o modelo de EXIBIÇÃO (HD) quando existe — é o que está na tela.
            const PokemonType& tgtType = g_PokeTypes[tgt.pokeType >= 0 ? tgt.pokeType : 0];
            bool        tgtHD    = (tgt.pokeType >= 0 && tgtType.dispMesh != nullptr);
            const char* tMeshN   = tgtHD ? tgtType.dispMesh : tgt.mesh.c_str();
            float       tScaleU  = tgtHD ? tgtType.dispCaptureScale : tgt.scale.x;
            SceneObject& tmesh = g_VirtualScene[tMeshN];
            float tW = (tmesh.bbox_max.x - tmesh.bbox_min.x) * tScaleU;
            float tH = (tmesh.bbox_max.y - tmesh.bbox_min.y) * tScaleU;
            float tD = (tmesh.bbox_max.z - tmesh.bbox_min.z) * tScaleU;
            float pokeR = std::max(std::max(tW, tD), tH) * 0.5f;

            if (g_CaptureState == CaptureState::Aiming)
            {
                if (g_NumPokeballs <= 0)
                {
                    // Sem Pokébolas: não é possível mirar nem lançar.
                    g_CaptureCharge = 0.0f;
                }
                else if (g_KeyL)
                {
                    // segura L: carrega a barra (~1.2 s para encher)
                    g_CaptureCharge += delta_t / 1.2f;
                    if (g_CaptureCharge > 1.0f) g_CaptureCharge = 1.0f;
                }
                else if (g_CaptureCharge > 0.0f)
                {
                    // soltou L -> lança. Gasta uma Pokébola. A carga é a MIRA: quanto
                    // mais cheia, menor o erro (o destino fica mais perto do centro do
                    // Pokémon). O acerto é decidido por interseção (abaixo), não por sorte.
                    g_NumPokeballs--;
                    float missMag = (1.0f - g_CaptureCharge) * 0.55f * ((float)rand()/RAND_MAX);
                    float ang     = ((float)rand()/RAND_MAX) * 6.2831853f;
                    // Erro no plano perpendicular ao arremesso (a câmera olha em -X,
                    // então o desvio é em Y/Z: cima/baixo/lados).
                    glm::vec3 aimErr = glm::vec3(0.0f, sinf(ang), cosf(ang)) * missMag;

                    g_ThrowP0 = held;
                    g_ThrowP3 = tc + aimErr; // destino = corpo do Pokémon + erro de mira
                    glm::vec3 d = g_ThrowP3 - g_ThrowP0;
                    g_ThrowP1 = g_ThrowP0 + d * 0.33f + glm::vec3(0.0f, 0.5f, 0.0f);
                    g_ThrowP2 = g_ThrowP0 + d * 0.66f + glm::vec3(0.0f, 0.5f, 0.0f);
                    g_CaptureThrowT = 0.0f;
                    g_CaptureState  = CaptureState::Throwing;
                }
            }
            else if (g_CaptureState == CaptureState::Throwing)
            {
                g_CaptureThrowT += delta_t / 0.8f; // ~0.8 s para chegar
                if (g_CaptureThrowT >= 1.0f)
                {
                    g_CaptureThrowT = 1.0f;
                    // TESTE DE INTERSEÇÃO Pokébola x Pokémon: a bola acerta se a
                    // esfera dela (centro g_ThrowP3, raio ballR) intersecta a esfera
                    // do Pokémon (centro tc, raio pokeR). Decide a captura. O teste
                    // em si (esfera-esfera) está em collisions.cpp.
                    const float ballR = 0.04f;
                    if (SphereCollision(g_ThrowP3, ballR, tc, pokeR))
                    {
                        g_CaptureState = CaptureState::Caught; // acertou -> capturou!
                        g_CaughtTimer  = 0.0f;
                    }
                    else
                    {
                        g_CaptureState = CaptureState::Missed; // errou: bola cai e some
                        g_MissTimer    = 0.0f;
                    }
                }
            }
            else if (g_CaptureState == CaptureState::Missed)
            {
                // Bola errou: cai no chão e encolhe; depois volta para a mira.
                g_MissTimer += delta_t;
                if (g_MissTimer > 0.7f)
                {
                    g_CaptureState  = CaptureState::Aiming; // pronto p/ nova tentativa
                    g_CaptureCharge = 0.0f;
                }
            }
            else // Caught
            {
                g_CaughtTimer += delta_t;
                if (g_CaughtTimer > 1.2f)
                {
                    // Pokémon capturado: registra no armazenamento (limite 100)
                    // com atributos aleatórios, remove do mapa e volta ao mundo.
                    if (g_CapturedCount < 100)
                    {
                        int type = g_Entities[g_CaptureTargetIndex].pokeType;
                        if (type < 0) type = 0;

                        CapturedPokemon p;
                        p.type   = type;
                        p.hp   = rand() % 201; // 0..200
                        p.cp   = g_CaptureCP;  // mesmo CP mostrado na cena de captura
                        p.atk1 = rand() % g_NumAttacks;
                        p.atk2 = rand() % g_NumAttacks;
                        if (p.atk2 == p.atk1) p.atk2 = (p.atk2 + 1) % g_NumAttacks; // dois ataques diferentes
                        g_Captured.push_back(p);
                        g_CapturedCount = (int) g_Captured.size();

                        // Doces da espécie (estilo Pokémon GO): cada captura rende
                        // CANDY_PER_CATCH doces da família, usados para evoluir.
                        if (type >= 0 && type < 5)
                            g_Candies[g_CandyFamily[type]] += CANDY_PER_CATCH;
                    }
                    g_Entities[g_CaptureTargetIndex].collidable = false;
                    g_Entities[g_CaptureTargetIndex].position.y = -100.0f;
                    g_CurrentScene       = GameScene::World;
                    g_CaptureTargetIndex = -1;

                    // Vai direto para a página (detalhe) do Pokémon recém-capturado.
                    if (g_CapturedCount > 0)
                    {
                        g_StorageOpen   = true;
                        g_StorageDetail = g_CapturedCount - 1;
                    }
                }
            }
        }

        // ---- Animação de evolução (doces -> nova forma) ----------------------
        // Avança o tempo; na METADE da animação troca a forma e melhora os
        // atributos; ao terminar, encerra. O desenho (vibração + flash) é feito
        // na tela de detalhe do armazenamento.
        if (g_EvolvingIndex >= 0 && g_EvolvingIndex < (int)g_Captured.size())
        {
            g_EvolveTimer += delta_t;
            if (!g_EvolveSwapped && g_EvolveTimer >= EVOLVE_DURATION * 0.5f)
            {
                CapturedPokemon& ep = g_Captured[g_EvolvingIndex];
                ep.type = g_EvolveToType;                    // vira a forma evoluída
                ep.cp   = std::min(200, ep.cp + 60);         // mais forte
                ep.hp   = std::min(200, ep.hp + 40);
                g_EvolveSwapped = true;
            }
            if (g_EvolveTimer >= EVOLVE_DURATION)
            {
                g_EvolvingIndex = -1;
                g_EvolveToType  = -1;
                g_EvolveTimer   = 0.0f;
                g_EvolveSwapped = false;
            }
        }

        // Limite de segurança (árvores bloqueiam em ~±4.4, isso é só backstop)
        const float MAP_LIMIT = 5.0f;

        if (g_PlayerX < -MAP_LIMIT) g_PlayerX = -MAP_LIMIT;
        if (g_PlayerX >  MAP_LIMIT) g_PlayerX =  MAP_LIMIT;

        if (g_PlayerZ < -MAP_LIMIT) g_PlayerZ = -MAP_LIMIT;
        if (g_PlayerZ >  MAP_LIMIT) g_PlayerZ =  MAP_LIMIT;

        glm::vec4 camera_position_c;
        glm::vec4 camera_lookat_l;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector = glm::vec4(0.0f,1.0f,0.0f,0.0f);

        if (g_CurrentScene == GameScene::Capture && g_CaptureTargetIndex >= 0)
        {
            const SceneEntity& target = g_Entities[g_CaptureTargetIndex];
            glm::vec3 targetCenter = glm::vec3(target.position.x, target.position.y + 0.25f, target.position.z);
            glm::vec3 captureCameraPosition = targetCenter + glm::vec3(1.6f, 0.5f, 0.0f);

            camera_position_c = glm::vec4(captureCameraPosition, 1.0f);
            camera_lookat_l   = glm::vec4(targetCenter, 1.0f);
        }
        else if (g_FreeCamera)
        {
            // Câmera livre: posição própria (g_FreeCamPos) e direção dada pelos
            // ângulos esféricos controlados pelo mouse. O ponto "lookat" é a
            // posição da câmera somada ao vetor de direção (unitário).
            float dx = -cos(g_CameraPhi)*sin(g_CameraTheta);
            float dy = -sin(g_CameraPhi);
            float dz = -cos(g_CameraPhi)*cos(g_CameraTheta);
            camera_position_c = glm::vec4(g_FreeCamPos, 1.0f);
            camera_lookat_l   = glm::vec4(g_FreeCamPos + glm::vec3(dx, dy, dz), 1.0f);
        }
        else
        {
            float r = g_CameraDistance;
            float y = r*sin(g_CameraPhi);
            float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
            float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

            // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
            // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
            camera_position_c  = glm::vec4(x + g_PlayerX, y, z + g_PlayerZ, 1.0f); // Ponto "c", centro da câmera

            // Mantém a câmera DENTRO do cercado de floresta. Se ela sair além das
            // placas (±5.5), o culling esconde a placa e aparece o vazio atrás;
            // travando aqui, a câmera sempre vê a face interna das placas.
            const float CAM_XZ   = 5.2f; // limite horizontal (placas em ±5.5)
            const float CAM_YMAX = 7.0f; // abaixo do topo das placas (~7.9)
            const float CAM_YMIN = 0.3f; // não afunda no chão
            if (camera_position_c.x < -CAM_XZ) camera_position_c.x = -CAM_XZ;
            if (camera_position_c.x >  CAM_XZ) camera_position_c.x =  CAM_XZ;
            if (camera_position_c.z < -CAM_XZ) camera_position_c.z = -CAM_XZ;
            if (camera_position_c.z >  CAM_XZ) camera_position_c.z =  CAM_XZ;
            if (camera_position_c.y >  CAM_YMAX) camera_position_c.y = CAM_YMAX;
            if (camera_position_c.y <  CAM_YMIN) camera_position_c.y = CAM_YMIN;

            camera_lookat_l    = glm::vec4(g_PlayerX, 0.0f, g_PlayerZ, 1.0f); // Câmera segue o personagem
        }

        camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -20.0f; // Posição do "far plane" (aumentado para cobrir o plano escalado)

        if (g_UsePerspectiveProjection)
        {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
        else
        {
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 1.5f*g_CameraDistance/2.5f;
            float b = -t;
            float r = t*g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        // Tom da luz: neutro (branco) no mundo; na cena de captura, varia
        // suavemente entre uma luz quente (dia) e fria (entardecer) ao longo do
        // tempo, simulando o período do dia.
        glm::vec3 lightTint = glm::vec3(1.0f);
        if (g_CurrentScene == GameScene::Capture)
        {
            float dayT = sinf((float)glfwGetTime() * 0.5f) * 0.5f + 0.5f; // 0..1
            glm::vec3 warm = glm::vec3(1.15f, 1.00f, 0.80f); // luz de dia (quente)
            glm::vec3 cool = glm::vec3(0.65f, 0.75f, 1.05f); // entardecer (fria)
            lightTint = warm * (1.0f - dayT) + cool * dayT;
        }
        glUniform3f(g_light_tint_uniform, lightTint.x, lightTint.y, lightTint.z);

        // desenha o jogador
        if (g_CurrentScene != GameScene::Capture)
        {
            // Boneco do jogador. O modelo tem ~1.86 de altura e os pés na origem
            // (y=0), então o escalamos e o posicionamos sobre o chão (y = -1.1).
            // PLAYER_FACING_OFFSET: ajuste fino caso o boneco ande "de costas"
            // (some 3.1415927f para girar 180°).
            const float PLAYER_SCALE = 0.22f;
            const float PLAYER_FACING_OFFSET = 0.0f;

            model =
                Matrix_Translate(g_PlayerX, -1.1f, g_PlayerZ)
                *
                Matrix_Rotate_Y(g_PlayerAngleY + PLAYER_FACING_OFFSET)
                *
                Matrix_Scale(PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE);

            glUniformMatrix4fv(
                g_model_uniform,
                1,
                GL_FALSE,
                glm::value_ptr(model));

            glUniform1i(
                g_object_id_uniform,
                PLAYER);

            DrawVirtualObject("player_character");
        }

        // desenha todos os objetos da cena
        float captureFacingAngle = 0.0f;
        if (g_CurrentScene == GameScene::Capture && g_CaptureTargetIndex >= 0)
        {
            const SceneEntity& target = g_Entities[g_CaptureTargetIndex];
            glm::vec3 targetCenter = glm::vec3(target.position.x, target.position.y + 0.25f, target.position.z);
            glm::vec3 captureDirection = camera_position_c - glm::vec4(targetCenter, 1.0f);
            captureFacingAngle = atan2(captureDirection.x, captureDirection.z);

            // Fundo da cena de captura: um grande painel com a textura de floresta,
            // posicionado atrás do Pokémon e encarando a câmera (billboard).
            glm::vec3 toCam = glm::vec3(camera_position_c) - targetCenter;
            float lenc = sqrtf(toCam.x*toCam.x + toCam.y*toCam.y + toCam.z*toCam.z);
            glm::vec3 dirc = toCam / lenc;
            glm::vec3 bg = targetCenter - dirc * 3.0f;           // 3 unidades atrás do alvo
            float angBg = atan2(toCam.x, toCam.z);               // encara a câmera (yaw)
            model = Matrix_Translate(bg.x, bg.y, bg.z)
                  * Matrix_Rotate_Y(angBg)
                  * Matrix_Rotate_X(3.141592f / 2.0f)            // painel em pé
                  * Matrix_Scale(6.0f, 1.0f, 4.5f);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, CAPTURE_BG);
            DrawVirtualObject("the_plane");
        }

        for (size_t i = 0; i < g_Entities.size(); ++i)
        {
            const SceneEntity& obj = g_Entities[i];

            // Na cena de captura, mostramos apenas o Pokémon alvo (esconde o resto).
            if (g_CurrentScene == GameScene::Capture && g_CaptureTargetIndex != static_cast<int>(i))
                continue;
            // Visibilidade por aproximação (estilo Pokémon GO): o Pokémon só
            // aparece quando o jogador entra no raio (appearRadius), mas aparece
            // INTEIRO de uma vez — sem crescer gradualmente. Fora do raio, não
            // é desenhado.
            // appearRadius == 0 -> sempre visível.
            float appearScale = 1.0f;
            if (obj.appearRadius > 0.0f)
            {
                float dx = g_PlayerX - obj.position.x;
                float dz = g_PlayerZ - obj.position.z;
                float dist = sqrtf(dx*dx + dz*dz);

                if (dist > obj.appearRadius)
                    continue; // longe: não desenha
            }

            // Captura bem-sucedida: o Pokémon alvo "encolhe" para dentro da bola.
            if (g_CurrentScene == GameScene::Capture
                && g_CaptureState == CaptureState::Caught
                && g_CaptureTargetIndex == static_cast<int>(i))
            {
                appearScale *= std::max(0.0f, 1.0f - g_CaughtTimer / 0.8f);
            }

            // Oscilação leve dos Pokémon no mapa — animação por tempo. Só sobe a
            // partir do chão (0..amplitude) para nunca afundar no terreno. A fase
            // usa o índice para não balançarem todos em sincronia.
            float bobY = 0.0f;
            if (obj.pokeType >= 0 && g_CurrentScene != GameScene::Capture)
                bobY = (sinf((float)glfwGetTime() * 2.0f + (float)i) * 0.5f + 0.5f) * 0.06f;

            // Na CAPTURA (tela grande), o Pokémon alvo é desenhado com o modelo de
            // EXIBIÇÃO (HD) quando existe — no mapa segue o modelo base.
            bool isCaptureTarget = (g_CurrentScene == GameScene::Capture
                                    && g_CaptureTargetIndex == static_cast<int>(i)
                                    && obj.pokeType >= 0);
            bool useHDhere = isCaptureTarget && (g_PokeTypes[obj.pokeType].dispMesh != nullptr);

            const char* drawMesh  = obj.mesh.c_str();
            int         drawObj   = obj.object_id;
            glm::vec3   drawScale = obj.scale;
            glm::vec3   drawOff   = obj.localOffset;
            float       facing    = (obj.pokeType >= 0) ? g_PokeTypes[obj.pokeType].facingOffset : 0.0f;
            if (useHDhere)
            {
                const PokemonType& pt = g_PokeTypes[obj.pokeType];
                drawMesh  = pt.dispMesh;
                drawObj   = pt.dispObjId;
                drawScale = glm::vec3(pt.dispCaptureScale);
                drawOff   = pt.dispCaptureOffset;
                facing    = pt.dispFacing;
            }

            model =
                Matrix_Translate(
                    obj.position.x,
                    obj.position.y + bobY,
                    obj.position.z)
                *
                (
                    isCaptureTarget
                    ? Matrix_Rotate_Y(captureFacingAngle + facing) // gira o Pokémon para encarar a câmera
                    : Matrix_Identity()
                )
                *
                Matrix_Scale(
                    drawScale.x * appearScale,
                    drawScale.y * appearScale,
                    drawScale.z * appearScale)
                *
                Matrix_Translate(drawOff.x, drawOff.y, drawOff.z);

            glUniformMatrix4fv(
                g_model_uniform,
                1,
                GL_FALSE,
                glm::value_ptr(model));

            glUniform1i(g_object_id_uniform, drawObj);

            // Charmander/Snorlax (e os modelos HD) podem ter faces com orientação
            // invertida: desenha os dois lados para não aparecerem buracos.
            bool twoSided = (drawObj == CHARMANDER || drawObj == SNORLAX
                             || drawObj == CHARMANDER_HD || drawObj == CHARMELEON
                             || drawObj == PIKACHU_HD || drawObj == SNORLAX_HD);
            if (twoSided) glDisable(GL_CULL_FACE);
            DrawVirtualObject(drawMesh);
            if (twoSided) glEnable(GL_CULL_FACE);

            // ---- Clique no Pokémon -> abre a cena de captura --------------------
            // Só os Pokémon DESENHADOS (logo, perto o bastante para aparecer)
            // chegam aqui, então clicar neles equivale a interagir de perto.
            if (g_PokeClickCheck && g_CurrentScene == GameScene::World && obj.pokeType >= 0)
            {
                int ww = 0, wh = 0;
                glfwGetWindowSize(window, &ww, &wh);

                // Projeta o centro do corpo do Pokémon para coordenadas de tela.
                float ccy  = obj.position.y + bobY + 0.20f;
                glm::vec4 clip = projection * view * glm::vec4(obj.position.x, ccy, obj.position.z, 1.0f);
                if (clip.w > 0.0f)
                {
                    glm::vec3 ndc = glm::vec3(clip) / clip.w;
                    float sx = (ndc.x*0.5f + 0.5f) * ww;
                    float sy = (1.0f - (ndc.y*0.5f + 0.5f)) * wh;

                    // Raio de acerto = tamanho projetado (centro -> topo) do corpo.
                    glm::vec4 clipTop = projection * view * glm::vec4(obj.position.x, ccy + 0.40f, obj.position.z, 1.0f);
                    glm::vec3 ndcTop  = glm::vec3(clipTop) / clipTop.w;
                    float syTop = (1.0f - (ndcTop.y*0.5f + 0.5f)) * wh;
                    float hitR  = fabs(sy - syTop) + 16.0f;

                    float ddx = (float)g_ClickPosX - sx;
                    float ddy = (float)g_ClickPosY - sy;
                    if (ddx*ddx + ddy*ddy < hitR*hitR)
                    {
                        g_CurrentScene       = GameScene::Capture;
                        g_CaptureTargetIndex = static_cast<int>(i);
                        // reinicia a mecânica de captura
                        g_CaptureState  = CaptureState::Aiming;
                        g_CaptureCharge = 0.0f;
                        g_CaptureThrowT = 0.0f;
                        g_CaptureCP     = rand() % 201; // CP do selvagem (mostrado na cena)
                        g_PokeClickCheck = false;       // consome o clique
                    }
                }
            }
        }
        g_PokeClickCheck = false; // consumimos o clique deste frame (haja acerto ou não)

        // Painéis verticais de floresta nos 4 lados — dão sensação de 3D.
        // Todos com a MESMA orientação: o plano (deitado) é levantado por
        // Rotate_X(-90°) — deixando a textura em pé, chão embaixo — e só GIRADO em
        // Y para virar para cada lado. Assim a floresta nasce do chão igual nos 4.
        if (g_CurrentScene != GameScene::Capture)
        {
            const float WD  = 5.5f;  // distância da borda do mapa
            const float WHW = 8.0f;  // meia-largura do painel
            const float WHH = 4.5f;  // meia-altura do painel
            const float WCY = WHH - 1.1f; // centro Y: base alinhada ao chão
            const float HALF_PI = 3.141592f / 2.0f;
            const float PI      = 3.141592f;

            // Lado: posição (x,z) e yaw (giro em Y para virar ao centro).
            struct Wall { float x, z, yaw; };
            const Wall walls[4] = {
                {  0.0f, -WD,  PI       }, // Norte
                {  0.0f,  WD,  0.0f     }, // Sul
                {  WD,   0.0f, HALF_PI  }, // Leste
                { -WD,   0.0f, -HALF_PI }, // Oeste
            };

            // Culling LIGADO: cada placa só mostra a face virada para o centro;
            // quando a câmera passa por trás, a placa some (não tampa a vista).
            glUniform1i(g_object_id_uniform, FOREST_WALL);
            for (int w = 0; w < 4; ++w)
            {
                model = Matrix_Translate(walls[w].x, WCY, walls[w].z)
                      * Matrix_Rotate_Y(walls[w].yaw)
                      * Matrix_Rotate_X(-HALF_PI)        // levanta o plano (vira para o centro)
                      * Matrix_Scale(WHW, 1.0f, WHH);
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                DrawVirtualObject("the_plane");
            }
        }

        // ---- PokéStops: cooldown, coleta por proximidade e desenho ----------
        if (g_CurrentScene != GameScene::Capture)
        {
            const float STOP_NEAR_RADIUS       = 1.3f;  // distância para poder coletar
            const float POKESTOP_SIZE          = 0.8f;  // escala do modelo (altura 1)
            const float POKESTOP_COOLDOWN_TIME = 30.0f; // segundos cinza após coletar
            const int   ITEM_MAX               = 100;   // estoque máximo por item

            int swW = 0, swH = 0;
            glfwGetWindowSize(window, &swW, &swH);

            for (PokeStop& stop : g_PokeStops)
            {
                // Atualiza o cooldown com base no tempo
                if (stop.cooldown > 0.0f)
                {
                    stop.cooldown -= delta_t;
                    if (stop.cooldown < 0.0f)
                        stop.cooldown = 0.0f;
                }

                // Disponível = sem cooldown (azul). Coletar exige também estar perto.
                float dx = g_PlayerX - stop.position.x;
                float dz = g_PlayerZ - stop.position.z;
                bool  nearStop  = (dx*dx + dz*dz) < STOP_NEAR_RADIUS*STOP_NEAR_RADIUS;
                bool  available = (stop.cooldown <= 0.0f);

                // Coleta por CLIQUE: só funciona se estiver perto e disponível.
                if (g_StopClickCheck && nearStop && available)
                {
                    float cy = stop.position.y + POKESTOP_SIZE * 0.5f;
                    glm::vec4 clip = projection * view * glm::vec4(stop.position.x, cy, stop.position.z, 1.0f);
                    if (clip.w > 0.0f)
                    {
                        glm::vec3 ndc = glm::vec3(clip) / clip.w;
                        float sx = (ndc.x*0.5f + 0.5f) * swW;
                        float sy = (1.0f - (ndc.y*0.5f + 0.5f)) * swH;

                        glm::vec4 clipTop = projection * view * glm::vec4(stop.position.x, stop.position.y + POKESTOP_SIZE, stop.position.z, 1.0f);
                        glm::vec3 ndcTop  = glm::vec3(clipTop) / clipTop.w;
                        float syTop = (1.0f - (ndcTop.y*0.5f + 0.5f)) * swH;
                        float hitR  = fabs(sy - syTop) + 12.0f;

                        float ddx = (float)g_ClickPosX - sx;
                        float ddy = (float)g_ClickPosY - sy;
                        if (ddx*ddx + ddy*ddy < hitR*hitR)
                        {
                            // Coleta: sorteia de 1 a 7 itens aleatórios entre
                            // Pokébola, Poção e Fruta (respeitando o máximo) e entra
                            // em cooldown. Cada item ganho voa até o jogador.
                            int total = 1 + rand() % 7; // 1..7 itens
                            int gB = 0, gP = 0, gF = 0;
                            glm::vec3 start = glm::vec3(stop.position.x, stop.position.y + POKESTOP_SIZE, stop.position.z);
                            glm::vec3 endp  = glm::vec3(g_PlayerX, -0.7f, g_PlayerZ);

                            for (int k = 0; k < total; ++k)
                            {
                                // O 1º item é SEMPRE Pokébola (garante pelo menos uma;
                                // se vier só 1 item, é Pokébola). Os demais sorteados.
                                int type = (k == 0) ? ITEM_POKEBALL : (rand() % 3); // 0=Pokébola,1=Poção,2=Fruta
                                bool added = false;
                                if      (type == ITEM_POKEBALL && g_NumPokeballs < ITEM_MAX) { g_NumPokeballs++; gB++; added = true; }
                                else if (type == ITEM_POTION   && g_NumPotions   < ITEM_MAX) { g_NumPotions++;   gP++; added = true; }
                                else if (type == ITEM_BERRY    && g_NumBerries   < ITEM_MAX) { g_NumBerries++;   gF++; added = true; }
                                if (!added)
                                    continue; // tipo cheio: não voa

                                glm::vec3 d = endp - start;
                                float sx2 = (((float)rand()/RAND_MAX) - 0.5f) * 0.5f; // espalha um pouco
                                float sz2 = (((float)rand()/RAND_MAX) - 0.5f) * 0.5f;
                                FlyingItem fi;
                                fi.type = type;
                                fi.p0 = start;
                                fi.p1 = start + d*0.33f + glm::vec3(sx2, 0.9f, sz2);
                                fi.p2 = start + d*0.66f + glm::vec3(sx2, 0.6f, sz2);
                                fi.p3 = endp;
                                fi.t  = -0.10f * k; // atraso escalonado de largada
                                g_FlyingItems.push_back(fi);
                            }

                            stop.cooldown = POKESTOP_COOLDOWN_TIME;
                            available = false; // coletou: fica cinza (cooldown)

                            char buf[128];
                            if (gB == 0 && gP == 0 && gF == 0)
                                snprintf(buf, sizeof(buf), "PokeStop: mochila cheia!");
                            else
                                snprintf(buf, sizeof(buf), "PokeStop! +%d Pokebola, +%d Pocao, +%d Fruta", gB, gP, gF);
                            g_Message      = buf;
                            g_MessageTimer = 2.5f;
                        }
                    }
                }

                // Dois eixos independentes:
                //  - FORMA pela proximidade: perto = aberta, longe = fechada.
                //  - COR pelo cooldown: disponível = azul, em cooldown = cinza.
                const char* stopMesh = nearStop  ? "openstop" : "closedstop";
                int  stopColorId     = available ? POKESTOP   : POKESTOP_COOLDOWN;
                // Quando disponível, a PokéStop gira lentamente em Y (animação por
                // tempo, como o disco girando no jogo); em cooldown fica parada.
                float stopSpin = available ? (float)glfwGetTime() * 1.2f : 0.0f;
                model = Matrix_Translate(stop.position.x, stop.position.y, stop.position.z)
                      * Matrix_Rotate_Y(stopSpin)
                      * Matrix_Scale(POKESTOP_SIZE, POKESTOP_SIZE, POKESTOP_SIZE);
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, stopColorId);
                glDisable(GL_CULL_FACE);
                DrawVirtualObject(stopMesh);
                glEnable(GL_CULL_FACE);
            }
            g_StopClickCheck = false; // consumimos o clique deste frame

            // Atualiza o tempo restante da mensagem na tela
            if (g_MessageTimer > 0.0f)
            {
                g_MessageTimer -= delta_t;
                if (g_MessageTimer < 0.0f)
                    g_MessageTimer = 0.0f;
            }

            // ---- Itens voando (PokéStop -> jogador) por Bézier cúbica ----------
            for (size_t k = 0; k < g_FlyingItems.size(); )
            {
                FlyingItem& fi = g_FlyingItems[k];
                fi.t += delta_t / 0.6f; // ~0.6 s de voo (animação por Δt)

                if (fi.t >= 1.0f)
                {
                    g_FlyingItems.erase(g_FlyingItems.begin() + k);
                    continue; // não incrementa k
                }

                if (fi.t >= 0.0f) // t negativo = atraso de largada
                {
                    // Modelo, object_id e escala conforme o tipo do item.
                    const char* mesh   = "Esfera_UV";
                    int         objid  = POKEBALL;
                    float       isc    = 0.04f;
                    if (fi.type == ITEM_POTION) { mesh = "potion"; objid = GYM_MODEL; isc = 0.12f; }
                    else if (fi.type == ITEM_BERRY) { mesh = "berry"; objid = GYM_MODEL; isc = 0.12f; }

                    glm::vec3 pos = EvalCubicBezier(fi.p0, fi.p1, fi.p2, fi.p3, fi.t);
                    float spin = (float)glfwGetTime() * 6.0f;
                    model = Matrix_Translate(pos.x, pos.y, pos.z)
                          * Matrix_Rotate_Y(spin)
                          * Matrix_Scale(isc, isc, isc);
                    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, objid);
                    glDisable(GL_CULL_FACE);
                    DrawVirtualObject(mesh);
                    glEnable(GL_CULL_FACE);
                }
                ++k;
            }
        }

        // ---- Ginásios: desenho (cor por time) + interação por CLIQUE (de perto) -
        if (g_CurrentScene != GameScene::Capture)
        {
            const float GYM_SIZE        = 1.4f; // altura do modelo (normalizado p/ 1.0)
            const float GYM_NEAR_RADIUS = 1.6f; // só dá pra interagir se estiver perto

            int ww = 0, wh = 0;
            glfwGetWindowSize(window, &ww, &wh);

            // Desenho de cada ginásio com a cor do time dono (ou cinza, se livre).
            // Aproveitamos o laço para testar o clique (picking em espaço de tela):
            // clicar num ginásio livre abre o modal "deixar um Pokémon?".
            for (size_t gi = 0; gi < g_Gyms.size(); ++gi)
            {
                const Gym& gym = g_Gyms[gi];

                // Perto o suficiente para interagir? (usado só para o clique).
                float pdx = g_PlayerX - gym.position.x;
                float pdz = g_PlayerZ - gym.position.z;
                bool  nearGym = (pdx*pdx + pdz*pdz) < GYM_NEAR_RADIUS*GYM_NEAR_RADIUS;

                // Sempre desenha o modelo completo do ginásio. A malha já codifica
                // a cor do time (cinza = livre) por vértice.
                const char* gymMesh = "graygym";
                if      (gym.team == Team::Red)    gymMesh = "redgym";
                else if (gym.team == Team::Blue)   gymMesh = "blegym";
                else if (gym.team == Team::Yellow) gymMesh = "yellowgym";

                model = Matrix_Translate(gym.position.x, gym.position.y, gym.position.z)
                      * Matrix_Scale(GYM_SIZE, GYM_SIZE, GYM_SIZE);
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, GYM_MODEL);
                // Modelo de terceiros: desenha os dois lados para não sumirem
                // faces com orientação invertida.
                glDisable(GL_CULL_FACE);
                DrawVirtualObject(gymMesh);
                glEnable(GL_CULL_FACE);

                // Se há um Pokémon defendendo o ginásio, ele aparece FLUTUANDO
                // acima do topo, girando devagar e com um leve sobe-e-desce — assim
                // dá pra ver qual Pokémon está no ginásio.
                if (gym.pokemonType >= 0)
                {
                    const PokemonType& dt = g_PokeTypes[gym.pokemonType];
                    // Tela grande => modelo de EXIBIÇÃO (HD) quando existe.
                    bool        gUseHD = (dt.dispMesh != nullptr);
                    const char* gMesh  = gUseHD ? dt.dispMesh        : dt.mesh;
                    int         gObj   = gUseHD ? dt.dispObjId        : dt.modelObjId;
                    glm::vec3   gOff   = gUseHD ? dt.dispDetailOffset : dt.detailOffset;
                    float       gScale = gUseHD ? dt.dispDetailScale  : dt.detailScale;

                    float gspin = (float)glfwGetTime() * 1.0f;
                    float gbob  = sinf((float)glfwGetTime() * 2.0f + (float)gi) * 0.05f;
                    float gy    = gym.position.y + GYM_SIZE + 0.35f + gbob; // acima do ginásio

                    model = Matrix_Translate(gym.position.x, gy, gym.position.z)
                          * Matrix_Rotate_Y(gspin)
                          * Matrix_Scale(gScale, gScale, gScale)
                          * Matrix_Translate(gOff.x, gOff.y, gOff.z);
                    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, gObj);
                    glDisable(GL_CULL_FACE); // alguns modelos têm faces invertidas
                    DrawVirtualObject(gMesh);
                    glEnable(GL_CULL_FACE);
                }

                // Picking: interage ao clicar de perto. Ginásio livre -> modal de
                // deixar Pokémon; ginásio ocupado -> modal de dar fruta.
                if (g_GymClickCheck && nearGym)
                {
                    float cy = gym.position.y + GYM_SIZE * 0.5f;
                    glm::vec4 clip = projection * view * glm::vec4(gym.position.x, cy, gym.position.z, 1.0f);
                    if (clip.w > 0.0f)
                    {
                        glm::vec3 ndc = glm::vec3(clip) / clip.w;
                        float sx = (ndc.x*0.5f + 0.5f) * ww;
                        float sy = (1.0f - (ndc.y*0.5f + 0.5f)) * wh;

                        // Raio de acerto = tamanho projetado (centro -> topo) do ginásio.
                        glm::vec4 clipTop = projection * view * glm::vec4(gym.position.x, gym.position.y + GYM_SIZE, gym.position.z, 1.0f);
                        glm::vec3 ndcTop  = glm::vec3(clipTop) / clipTop.w;
                        float syTop = (1.0f - (ndcTop.y*0.5f + 0.5f)) * wh;
                        float hitR  = fabs(sy - syTop) + 10.0f;

                        float ddx = (float)g_ClickPosX - sx;
                        float ddy = (float)g_ClickPosY - sy;
                        if (ddx*ddx + ddy*ddy < hitR*hitR)
                        {
                            g_GymModalOpen  = true;
                            g_GymModalIndex = (int)gi;
                        }
                    }
                }
            }
            g_GymClickCheck = false; // consumimos o clique deste frame
        }

        // ---- Pokébola: aparece SÓ na captura (mira / arremesso / captura) ----
        // Na mira, só desenha se o jogador tiver Pokébolas (senão não há o que
        // segurar). Durante o arremesso/captura, a bola já foi lançada e continua
        // visível em voo.
        if (g_CurrentScene == GameScene::Capture && g_CaptureTargetIndex >= 0
            && !(g_CaptureState == CaptureState::Aiming && g_NumPokeballs <= 0))
        {
            const SceneEntity& tgt = g_Entities[g_CaptureTargetIndex];
            glm::vec3 tc = glm::vec3(tgt.position.x, tgt.position.y + 0.25f, tgt.position.z);
            glm::vec3 capCam = tc + glm::vec3(1.6f, 0.5f, 0.0f);
            glm::vec3 toT = tc - capCam;
            float tl = sqrtf(toT.x*toT.x + toT.y*toT.y + toT.z*toT.z);
            glm::vec3 held = capCam + (toT / tl) * 0.45f + glm::vec3(0.0f, -0.18f, 0.0f);

            glm::vec3 ballPos   = held; // mira: bola "na mão"
            float     ballScale = 0.03f;
            if (g_CaptureState == CaptureState::Throwing)
                ballPos = EvalCubicBezier(g_ThrowP0, g_ThrowP1, g_ThrowP2, g_ThrowP3, g_CaptureThrowT);
            else if (g_CaptureState == CaptureState::Caught)
                ballPos = tc + (toT / -tl) * 0.25f; // na frente do Pokémon (mesmo ponto do arremesso)
            else if (g_CaptureState == CaptureState::Missed)
            {
                // Errou: a bola cai (gravidade) de onde errou até o chão e encolhe
                // até sumir, antes de voltar para a mira.
                float groundY = tgt.position.y; // base do Pokémon = nível do chão
                float y = g_ThrowP3.y - 0.5f * 5.0f * g_MissTimer * g_MissTimer; // queda acelerada
                if (y < groundY) y = groundY;
                ballPos   = glm::vec3(g_ThrowP3.x, y, g_ThrowP3.z);
                ballScale = 0.03f * std::max(0.0f, 1.0f - g_MissTimer / 0.7f);
            }

            float spin = (float) glfwGetTime() * 5.0f;
            model = Matrix_Translate(ballPos.x, ballPos.y, ballPos.z)
                  * Matrix_Rotate_Y(spin)
                  * Matrix_Scale(ballScale, ballScale, ballScale); // bem menor que o Pokémon
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, POKEBALL);
            DrawVirtualObject("Esfera_UV");
        }

        // ---- Balão da Equipe Rocket: voa pelo céu por curva de Bézier cúbica --
        if (g_CurrentScene != GameScene::Capture)
        {
            // Loop fechado formado por 4 segmentos de Bézier cúbica que, juntos,
            // aproximam um círculo no plano XZ a uma certa altitude (Y).
            const float R  = 3.5f;            // raio do loop
            const float kR = 0.5523f * R;     // "handle" p/ aproximar 1/4 de círculo
            const float Y  = 1.4f;            // altitude do balão
            glm::vec3 ctrl[4][4] = {
                { glm::vec3(R,Y,0),  glm::vec3(R,Y,kR),   glm::vec3(kR,Y,R),   glm::vec3(0,Y,R)   },
                { glm::vec3(0,Y,R),  glm::vec3(-kR,Y,R),  glm::vec3(-R,Y,kR),  glm::vec3(-R,Y,0)  },
                { glm::vec3(-R,Y,0), glm::vec3(-R,Y,-kR), glm::vec3(-kR,Y,-R), glm::vec3(0,Y,-R)  },
                { glm::vec3(0,Y,-R), glm::vec3(kR,Y,-R),  glm::vec3(R,Y,-kR),  glm::vec3(R,Y,0)   },
            };

            // Avança o parâmetro (loop completo a cada ~24 s) e o mantém em [0,4).
            g_RocketT += delta_t * (4.0f / 24.0f);
            while (g_RocketT >= 4.0f) g_RocketT -= 4.0f;

            int   seg = (int)g_RocketT;          // segmento atual (0..3)
            float lt  = g_RocketT - (float)seg;  // parâmetro local na Bézier (0..1)
            glm::vec3 bp = EvalCubicBezier(ctrl[seg][0], ctrl[seg][1], ctrl[seg][2], ctrl[seg][3], lt);

            // Balanço suave (bob) só para dar vida
            float bob = sinf((float)glfwGetTime() * 1.5f) * 0.08f;

            // Envelope do balão (esfera)
            model = Matrix_Translate(bp.x, bp.y + bob, bp.z)
                  * Matrix_Scale(0.45f, 0.55f, 0.45f);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, ROCKET);
            DrawVirtualObject("the_sphere");

            // "R" da Equipe Rocket: quadradinho texturizado (rocket_r.png) colado
            // na frente do envelope, girando em Y para sempre encarar a câmera.
            {
                float angR = atan2(camera_position_c.x - bp.x, camera_position_c.z - bp.z);
                model = Matrix_Translate(bp.x, bp.y + bob, bp.z)
                      * Matrix_Rotate_Y(angR)
                      * Matrix_Translate(0.0f, 0.0f, 0.47f) // à frente da superfície
                      * Matrix_Rotate_X(3.141592f / 2.0f)   // deixa o quad em pé
                      * Matrix_Scale(0.22f, 1.0f, 0.26f);
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, ROCKET_R);
                DrawVirtualObject("the_plane");
            }

            // 4 cordas finas ligando o envelope ao cesto (cubos esticados)
            float ropeCenterY = bp.y + bob - 0.70f;
            for (int sx = -1; sx <= 1; sx += 2)
            for (int sz = -1; sz <= 1; sz += 2)
            {
                model = Matrix_Translate(bp.x + sx*0.11f, ropeCenterY, bp.z + sz*0.11f)
                      * Matrix_Scale(0.012f, 0.34f, 0.012f);
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, ROCKET_BASKET);
                DrawVirtualObject("the_cube");
            }

            // Cesto pendurado mais abaixo (cubo)
            model = Matrix_Translate(bp.x, bp.y + bob - 0.95f, bp.z)
                  * Matrix_Scale(0.16f, 0.16f, 0.16f);
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, ROCKET_BASKET);
            DrawVirtualObject("the_cube");

            // ---- Clique no balão -> mensagem do Giovanni ----
            if (g_BalloonClickCheck)
            {
                g_BalloonClickCheck = false;

                // Projeta o centro do balão para coordenadas de tela (pixels da janela)
                glm::vec4 clip = projection * view * glm::vec4(bp.x, bp.y + bob, bp.z, 1.0f);
                if (clip.w > 0.0f)
                {
                    glm::vec3 ndc = glm::vec3(clip) / clip.w;
                    int ww, wh;
                    glfwGetWindowSize(window, &ww, &wh);
                    float sx = (ndc.x*0.5f + 0.5f) * ww;
                    float sy = (1.0f - (ndc.y*0.5f + 0.5f)) * wh;

                    // Raio de acerto em pixels: tamanho projetado do balão (centro->topo)
                    glm::vec4 clipTop = projection * view * glm::vec4(bp.x, bp.y + bob + 0.55f, bp.z, 1.0f);
                    glm::vec3 ndcTop  = glm::vec3(clipTop) / clipTop.w;
                    float syTop = (1.0f - (ndcTop.y*0.5f + 0.5f)) * wh;
                    float hitR  = fabs(sy - syTop) + 8.0f;

                    float ddx = (float)g_ClickPosX - sx;
                    float ddy = (float)g_ClickPosY - sy;
                    if (ddx*ddx + ddy*ddy < hitR*hitR)
                    {
                        g_Message      = "Giovanni nao esta aqui!";
                        g_MessageTimer = 2.5f;
                    }
                }
            }
        }

        // Tela de captura (trabalho do parceiro): botão de sair
        if (g_CurrentScene == GameScene::Capture)
        {
            TextRendering_PrintString(window, "[ Sair ]", 0.58f, 0.90f, 1.0f);

            // Nome e CP do Pokémon selvagem, centralizados no topo da cena.
            if (g_CaptureTargetIndex >= 0)
            {
                int type = g_Entities[g_CaptureTargetIndex].pokeType;
                if (type < 0) type = 0;

                std::string top = std::string(g_PokeTypes[type].name)
                                + "   CP " + std::to_string(g_CaptureCP);
                float nameScale = 1.6f;
                float tw = top.size() * TextRendering_CharWidth(window) * nameScale;
                TextRendering_PrintString(window, top, -tw * 0.5f, 0.86f, nameScale);
            }

            // Contador de Pokébolas (vai diminuindo conforme você lança).
            char pb[64];
            snprintf(pb, sizeof(pb), "Pokebolas: %d", g_NumPokeballs);
            TextRendering_PrintString(window, pb, -0.98f, 0.90f, 1.2f);

            // Barra de progresso da captura (estilo Pokémon GO).
            if (g_CaptureState == CaptureState::Aiming)
            {
                if (g_NumPokeballs <= 0)
                {
                    // Sem Pokébolas: não dá pra capturar. Avisa o jogador.
                    TextRendering_PrintString(window, "Sem Pokebolas!", -0.22f, -0.78f, 1.5f);
                    TextRendering_PrintString(window, "Explore o mapa e colete Pokebolas nas PokeStops.", -0.45f, -0.88f, 1.0f);
                }
                else
                {
                    int filled = (int)(g_CaptureCharge * 20.0f);
                    if (filled > 20) filled = 20;
                    std::string bar = "[" + std::string(filled, '#') + std::string(20 - filled, '-') + "]";

                    char buf[160];
                    snprintf(buf, sizeof(buf), "Mira: %s", bar.c_str());
                    TextRendering_PrintString(window, buf, -0.45f, -0.80f, 1.2f);
                    TextRendering_PrintString(window, "Segure L para mirar (cheio = certeiro), solte para lancar", -0.50f, -0.88f, 1.0f);
                }
            }
            else if (g_CaptureState == CaptureState::Throwing)
            {
                TextRendering_PrintString(window, "Lancando...", -0.1f, -0.85f, 1.2f);
            }
            else if (g_CaptureState == CaptureState::Missed)
            {
                TextRendering_PrintString(window, "Errou! Tente de novo", -0.22f, -0.85f, 1.3f);
            }
            else // Caught
            {
                TextRendering_PrintString(window, "Pokemon capturado!", -0.25f, -0.85f, 1.4f);
            }
        }

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // HUD do mundo: inventário, modo de câmera e mensagem dos PokéStops.
        // Não aparece na cena de captura (que tem sua própria HUD) nem no modal.
        if (g_CurrentScene != GameScene::Capture && !g_TeamModalOpen)
        {
            char inv[128];
            snprintf(inv, sizeof(inv), "Pokebolas: %d   Pocoes: %d   Frutas: %d", g_NumPokeballs, g_NumPotions, g_NumBerries);
            TextRendering_PrintString(window, inv, -0.98f, 0.90f, 1.0f);

            // Indicador do modo de câmera (tecla C alterna)
            TextRendering_PrintString(
                window,
                g_FreeCamera ? "[C] Camera: Livre  (WASD move, E/Q sobe/desce)"
                             : "[C] Camera: 3a pessoa",
                -0.98f, 0.84f, 1.0f);

            if (g_MessageTimer > 0.0f)
                TextRendering_PrintString(window, g_Message, -0.45f, 0.80f, 1.5f);
        }

        // ===== Interface 2D: ícone de armazenamento + janela =====
        // Desenhamos quads diretamente em NDC (view e projection = identidade),
        // por cima da cena (sem teste de profundidade nem culling).
        // Não aparece durante a cena de captura nem com o modal de time aberto.
        if (g_CurrentScene != GameScene::Capture && !g_TeamModalOpen)
        {
            glUseProgram(g_GpuProgramID);
            glm::mat4 ui_identity = Matrix_Identity();
            glUniformMatrix4fv(g_view_uniform,       1, GL_FALSE, glm::value_ptr(ui_identity));
            glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(ui_identity));
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            float aspect = g_ScreenRatio;

            if (g_StorageOpen && g_StorageDetail < 0)
            {
                // --- GRADE de miniaturas ---
                DrawUIQuad(0.0f, 0.0f, 0.58f, 0.66f, UI_PANEL);

                const int COLS = 4, VIS_ROWS = 3;
                const float ths = 0.13f;          // meia-altura da miniatura (NDC)
                const float thw = ths / aspect;   // meia-largura (mantém quadrada)
                const float xcols[4] = { -0.42f, -0.14f, 0.14f, 0.42f };
                int start = g_StorageScrollRow * COLS;
                int end   = start + VIS_ROWS * COLS;

                // Borda + fundo de cada caixinha (ainda em 2D). Desenhamos a borda
                // dourada um pouco maior e, por cima, o fundo escuro da célula; a
                // diferença entre os dois forma a moldura. O modelo 3D vem depois.
                const float bpad = 0.012f; // espessura da borda (NDC vertical)
                for (int i = start; i < end && i < g_CapturedCount; ++i)
                {
                    int   rel = (i / COLS) - g_StorageScrollRow;
                    float x   = xcols[i % COLS];
                    float y   = 0.34f - rel * 0.32f;
                    DrawUIQuad(x, y, thw + bpad / aspect, ths + bpad, UI_BORDER);
                    DrawUIQuad(x, y, thw, ths, UI_PANEL);
                }

                // Miniaturas = o PRÓPRIO modelo 3D do Pokémon (igual ao do mapa),
                // renderizado em cada célula da grade via glViewport. Cada célula
                // é um pequeno "viewport" quadrado com uma câmera própria.
                int fbW = 0, fbH = 0;
                glfwGetFramebufferSize(window, &fbW, &fbH);

                glEnable(GL_DEPTH_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);

                glm::vec4 tcam  = glm::vec4(0.0f, 0.05f, 0.45f, 1.0f);
                glm::vec4 tlook = glm::vec4(0.0f, 0.02f, 0.0f, 1.0f);
                glm::mat4 tView = Matrix_Camera_View(tcam, tlook - tcam, glm::vec4(0.0f,1.0f,0.0f,0.0f));
                glm::mat4 tProj = Matrix_Perspective(3.141592f/3.0f, 1.0f, -0.1f, -10.0f);
                glUniformMatrix4fv(g_view_uniform,       1, GL_FALSE, glm::value_ptr(tView));
                glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(tProj));

                float tspin = (float) glfwGetTime() * 1.0f;

                for (int i = start; i < end && i < g_CapturedCount; ++i)
                {
                    int   rel = (i / COLS) - g_StorageScrollRow;
                    float x   = xcols[i % COLS];
                    float y   = 0.34f - rel * 0.32f;

                    // Célula em pixels (NDC -> tela) para o glViewport.
                    int px = (int)((x - thw + 1.0f) * 0.5f * fbW);
                    int py = (int)((y - ths + 1.0f) * 0.5f * fbH);
                    int pw = (int)(thw * 2.0f * 0.5f * fbW); // = thw * fbW
                    int ph = (int)(ths * 2.0f * 0.5f * fbH); // = ths * fbH
                    glViewport(px, py, pw, ph);

                    const PokemonType& tt = g_PokeTypes[g_Captured[i].type];
                    float ts = tt.detailScale * 0.8f; // margem para caber na célula
                    model = Matrix_Rotate_Y(tspin)
                          * Matrix_Scale(ts, ts, ts)
                          * Matrix_Translate(tt.detailOffset.x, tt.detailOffset.y, tt.detailOffset.z);
                    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, tt.modelObjId);

                    // Modelos com faces possivelmente invertidas: desenha dois lados.
                    if (tt.modelObjId == CHARMANDER || tt.modelObjId == SNORLAX || tt.modelObjId == CHARMELEON || tt.modelObjId == RAICHU) glDisable(GL_CULL_FACE);
                    else                                                          glEnable(GL_CULL_FACE);
                    DrawVirtualObject(tt.mesh);
                }

                // Restaura o viewport cheio e volta ao 2D (identidade) para
                // o ícone e os textos desenhados em seguida.
                glViewport(0, 0, fbW, fbH);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glUniformMatrix4fv(g_view_uniform,       1, GL_FALSE, glm::value_ptr(ui_identity));
                glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(ui_identity));
            }
            else if (g_StorageOpen && g_StorageDetail >= 0)
            {
                // --- DETALHE: painel + Pokémon 3D girando ---
                DrawUIQuad(0.0f, 0.0f, 0.78f, 0.92f, UI_PANEL);

                // Limpamos o depth e renderizamos o Pokémon em 3D por cima do painel,
                // com uma câmera própria. Ele gira no mesmo tamanho que aparece no mapa.
                glClear(GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);
                // Sem backface culling: alguns modelos (ex.: rabo do Charmander) têm
                // faces com orientação invertida, que sumiriam (ficariam "transparentes")
                // ao girar. Desenhar os dois lados evita esses buracos.
                glDisable(GL_CULL_FACE);

                glm::vec4 dcam  = glm::vec4(0.0f, 0.05f, 0.45f, 1.0f);
                glm::vec4 dlook = glm::vec4(0.0f, 0.02f, 0.0f, 1.0f);
                glm::mat4 dView = Matrix_Camera_View(dcam, dlook - dcam, glm::vec4(0.0f,1.0f,0.0f,0.0f));
                glm::mat4 dProj = Matrix_Perspective(3.141592f/3.0f, aspect, -0.1f, -10.0f);
                glUniformMatrix4fv(g_view_uniform,       1, GL_FALSE, glm::value_ptr(dView));
                glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(dProj));

                // Parâmetros do tipo do Pokémon em detalhe. Tela grande => usa o
                // modelo de EXIBIÇÃO (HD) quando existe; senão, a malha base.
                const PokemonType& dt = g_PokeTypes[g_Captured[g_StorageDetail].type];
                bool        useHD  = (dt.dispMesh != nullptr);
                const char* dMesh  = useHD ? dt.dispMesh        : dt.mesh;
                int         dObj   = useHD ? dt.dispObjId        : dt.modelObjId;
                glm::vec3   dOff   = useHD ? dt.dispDetailOffset : dt.detailOffset;
                float       dScale = useHD ? dt.dispDetailScale  : dt.detailScale;

                // Animação de evolução (se este Pokémon está evoluindo): gira rápido
                // e ENCOLHE até sumir (1ª metade, ainda a forma antiga); no meio troca
                // de forma; a forma nova SURGE crescendo (2ª metade). Um FLASH de luz
                // marca o instante da troca (pico em u=0.5).
                bool  evolvingThis = (g_EvolvingIndex == g_StorageDetail);
                float spinSpeed    = evolvingThis ? 9.0f : 1.5f;
                float spin  = (float) glfwGetTime() * spinSpeed;
                float grow  = 1.0f;
                if (evolvingThis)
                {
                    float u = g_EvolveTimer / EVOLVE_DURATION;          // 0..1
                    // 1ª metade: 1 -> 0 (encolhe);  2ª metade: 0 -> 1 (cresce).
                    grow = (u < 0.5f) ? (1.0f - u / 0.5f) : ((u - 0.5f) / 0.5f);
                    float flash = 1.0f + 3.5f * expf(-powf((u - 0.5f) * 5.0f, 2.0f));
                    glUniform3f(g_light_tint_uniform, flash, flash, flash); // brilho
                }

                float dscale = dScale * grow;
                model = Matrix_Rotate_Y(spin)
                      * Matrix_Scale(dscale, dscale, dscale)
                      * Matrix_Translate(dOff.x, dOff.y, dOff.z);
                glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, dObj);
                DrawVirtualObject(dMesh);

                if (evolvingThis) // restaura a luz branca neutra
                    glUniform3f(g_light_tint_uniform, 1.0f, 1.0f, 1.0f);

                // Volta para o desenho 2D (identidade) para o ícone e os textos.
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glUniformMatrix4fv(g_view_uniform,       1, GL_FALSE, glm::value_ptr(ui_identity));
                glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(ui_identity));
            }

            // Ícone (sempre visível) no canto inferior esquerdo
            DrawUIQuad(UI_ICON_CX, UI_ICON_CY, UI_ICON_HH / aspect, UI_ICON_HH, UI_ICON);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            // Textos da janela (por cima dos quads)
            if (g_StorageOpen && g_StorageDetail >= 0)
            {
                // --- Textos da tela de DETALHE ---
                const CapturedPokemon& p = g_Captured[g_StorageDetail];
                char buf[64];

                snprintf(buf, sizeof(buf), "%s", g_PokeTypes[p.type].name);
                TextRendering_PrintString(window, buf, -0.14f, 0.80f, 1.6f);

                snprintf(buf, sizeof(buf), "HP: %d / 200", p.hp);
                TextRendering_PrintString(window, buf, -0.62f, -0.42f, 1.3f);
                snprintf(buf, sizeof(buf), "CP: %d / 200", p.cp);
                TextRendering_PrintString(window, buf, -0.62f, -0.52f, 1.3f);

                // Lista de ataques pela FAMÍLIA (forma base): vale para as formas
                // evoluídas também (índice sempre 0..2).
                int at = (p.type >= 0 && p.type < 5) ? g_CandyFamily[p.type] : 0;
                TextRendering_PrintString(window, "Ataques:", -0.62f, -0.66f, 1.2f);
                snprintf(buf, sizeof(buf), "- %s", g_AttacksByType[at][p.atk1]);
                TextRendering_PrintString(window, buf, -0.58f, -0.74f, 1.1f);
                snprintf(buf, sizeof(buf), "- %s", g_AttacksByType[at][p.atk2]);
                TextRendering_PrintString(window, buf, -0.58f, -0.82f, 1.1f);

                // --- Evolução (doces por espécie) -----------------------------
                int  fam     = (p.type >= 0 && p.type < 5) ? g_CandyFamily[p.type] : 0;
                int  evoTo   = (p.type >= 0 && p.type < 5) ? g_EvolvesTo[p.type]   : -1;
                int  evoCost = (p.type >= 0 && p.type < 5) ? g_EvolveCost[p.type]  : 0;
                bool evolvingThisTxt = (g_EvolvingIndex == g_StorageDetail);

                if (evolvingThisTxt)
                {
                    // Animação rolando: aviso central, sem botão.
                    std::string ev = "Evoluindo...";
                    float es = 1.8f;
                    float ew = ev.size() * TextRendering_CharWidth(window) * es;
                    TextRendering_PrintString(window, ev, -ew * 0.5f, 0.30f, es);
                }
                else if (evoTo >= 0)
                {
                    // Mostra os doces e o botão de evoluir.
                    snprintf(buf, sizeof(buf), "Doces de %s: %d / %d",
                             g_PokeTypes[fam].name, g_Candies[fam], evoCost);
                    TextRendering_PrintString(window, buf, 0.16f, -0.44f, 1.0f);

                    bool canEvolve = (g_Candies[fam] >= evoCost) && (p.placedGym < 0);

                    // Fundo do botão (borda + painel) + rótulo centralizado.
                    // Desenha em 2D (sem teste de profundidade) para ficar por cima.
                    glDisable(GL_DEPTH_TEST);
                    DrawUIQuad(EVO_BTN_CX, EVO_BTN_CY, EVO_BTN_HW + 0.012f, EVO_BTN_HH + 0.012f, UI_BORDER);
                    DrawUIQuad(EVO_BTN_CX, EVO_BTN_CY, EVO_BTN_HW, EVO_BTN_HH, UI_PANEL);
                    glEnable(GL_DEPTH_TEST);

                    const char* label = canEvolve ? "EVOLUIR"
                                      : (p.placedGym >= 0 ? "No ginasio" : "Doces insuf.");
                    float ls = 1.1f;
                    float lw = strlen(label) * TextRendering_CharWidth(window) * ls;
                    TextRendering_PrintString(window, label, EVO_BTN_CX - lw * 0.5f, EVO_BTN_CY - 0.02f, ls);
                }

                TextRendering_PrintString(window, "(clique para voltar)", 0.34f, -0.92f, 0.8f);
            }
            else if (g_StorageOpen)
            {
                // --- Textos da GRADE ---
                if (g_PlacingGymIndex >= 0)
                {
                    // Modo "colocar no ginásio": instrução no topo.
                    std::string hint = "Escolha um Pokemon para deixar no ginasio";
                    float hs = 1.1f;
                    float hw = hint.size() * TextRendering_CharWidth(window) * hs;
                    TextRendering_PrintString(window, hint, -hw * 0.5f, 0.58f, hs);
                }
                else
                {
                    char titulo[48];
                    snprintf(titulo, sizeof(titulo), "Meus Pokemon (%d/100)", g_CapturedCount);
                    TextRendering_PrintString(window, titulo, -0.26f, 0.58f, 1.4f);
                }

                if (g_CapturedCount == 0)
                {
                    TextRendering_PrintString(window, "Vazio - capture pokemon!", -0.28f, 0.0f, 1.1f);
                }
                else
                {
                    // Rótulo "Pikachu N" centralizado embaixo de cada miniatura
                    const int COLS = 4, VIS_ROWS = 3;
                    const float ths = 0.13f;
                    const float xcols[4] = { -0.42f, -0.14f, 0.14f, 0.42f };
                    const float labelScale = 0.6f;
                    float cw = TextRendering_CharWidth(window) * labelScale;
                    int start = g_StorageScrollRow * COLS;
                    int end   = start + VIS_ROWS * COLS;
                    for (int i = start; i < end && i < g_CapturedCount; ++i)
                    {
                        int   rel = (i / COLS) - g_StorageScrollRow;
                        float x   = xcols[i % COLS];
                        float y   = 0.34f - rel * 0.32f;
                        const CapturedPokemon& cp = g_Captured[i];
                        std::string lbl = g_PokeTypes[cp.type].name;
                        if (cp.placedGym >= 0) lbl += " (gym)"; // já está num ginásio
                        float tw = lbl.size() * cw;
                        TextRendering_PrintString(window, lbl, x - tw * 0.5f, y - ths - 0.05f, labelScale);
                    }
                }
            }
        }

        // ===== Modal de escolha de time (aparece ao abrir o jogo) =====
        // Desenhado por cima de tudo, em 2D (NDC, sem profundidade/culling).
        if (g_TeamModalOpen)
        {
            glUseProgram(g_GpuProgramID);
            glm::mat4 ui_id = Matrix_Identity();
            glUniformMatrix4fv(g_view_uniform,       1, GL_FALSE, glm::value_ptr(ui_id));
            glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(ui_id));
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            float aspect = g_ScreenRatio;

            // Painel de fundo do modal
            DrawUIQuad(0.0f, 0.0f, 0.62f, 0.50f, UI_PANEL);

            // Três botões coloridos (Vermelho / Azul / Amarelo) com o logo do time.
            const float bx[3]  = { -0.34f, 0.00f, 0.34f };
            const int   bid[3] = { UI_TEAM_RED, UI_TEAM_BLUE, UI_TEAM_YELLOW };
            const int   lid[3] = { UI_LOGO_VALOR, UI_LOGO_MYSTIC, UI_LOGO_INSTINCT };
            const float bhw = 0.13f / aspect; // meia-largura (corrigida pela proporção)
            const float bhh = 0.20f;          // meia-altura
            const float bcy = -0.06f;         // centro Y dos botões
            const float lhh = 0.12f;          // meia-altura do logo (quadrado)
            const float lhw = lhh / aspect;
            for (int b = 0; b < 3; ++b)
            {
                DrawUIQuad(bx[b], bcy, bhw, bhh, bid[b]);    // botão colorido
                DrawUIQuad(bx[b], bcy, lhw, lhh, lid[b]);    // logo recortado por cima
            }

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            // Textos (título e rótulos centralizados sob cada botão)
            {
                std::string title = "Escolha seu time!";
                float ts = 2.0f;
                float ttw = title.size() * TextRendering_CharWidth(window) * ts;
                TextRendering_PrintString(window, title, -ttw * 0.5f, 0.34f, ts);

                const char* names[3] = { "Valor", "Mystic", "Instinct" };
                float ls = 1.0f;
                for (int b = 0; b < 3; ++b)
                {
                    std::string nm = names[b];
                    float ltw = nm.size() * TextRendering_CharWidth(window) * ls;
                    TextRendering_PrintString(window, nm, bx[b] - ltw * 0.5f, bcy - bhh - 0.07f, ls);
                }
            }
        }

        // ===== Modal "deixar um Pokémon no ginásio?" =====
        if (g_GymModalOpen)
        {
            glUseProgram(g_GpuProgramID);
            glm::mat4 ui_id = Matrix_Identity();
            glUniformMatrix4fv(g_view_uniform,       1, GL_FALSE, glm::value_ptr(ui_id));
            glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(ui_id));
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            float aspect = g_ScreenRatio;

            DrawUIQuad(0.0f, 0.0f, 0.60f, 0.40f, UI_PANEL);

            // Dois botões: Sim (verde) e Nao (vermelho). Bordas + fundo.
            const float gbx[2]  = { -0.26f, 0.26f };
            const float gbhw = 0.16f / aspect;
            const float gbhh = 0.12f;
            const float gbcy = -0.10f;
            DrawUIQuad(gbx[0], gbcy, gbhw + 0.012f/aspect, gbhh + 0.012f, UI_BORDER);
            DrawUIQuad(gbx[0], gbcy, gbhw, gbhh, UI_TEAM_BLUE);   // Sim
            DrawUIQuad(gbx[1], gbcy, gbhw + 0.012f/aspect, gbhh + 0.012f, UI_BORDER);
            DrawUIQuad(gbx[1], gbcy, gbhw, gbhh, UI_TEAM_RED);    // Nao

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            // Texto conforme o estado do ginásio: livre -> deixar Pokémon;
            // ocupado -> dar uma fruta ao Pokémon (mostra quantas frutas você tem).
            bool gymOccupied = (g_GymModalIndex >= 0 && g_Gyms[g_GymModalIndex].pokemonType >= 0);
            std::string t1, t2;
            if (gymOccupied)
            {
                t1 = "Dar uma fruta ao Pokemon?";
                t2 = "(Frutas: " + std::to_string(g_NumBerries) + ")";
            }
            else
            {
                t1 = "Deixar um Pokemon";
                t2 = "neste ginasio?";
            }
            float ts = 1.6f;
            float w1 = t1.size() * TextRendering_CharWidth(window) * ts;
            float w2 = t2.size() * TextRendering_CharWidth(window) * ts;
            TextRendering_PrintString(window, t1, -w1*0.5f, 0.24f, ts);
            TextRendering_PrintString(window, t2, -w2*0.5f, 0.13f, ts);

            float ls = 1.2f;
            std::string s1 = "Sim (Y)", s2 = "Nao (N)";
            float ws1 = s1.size() * TextRendering_CharWidth(window) * ls;
            float ws2 = s2.size() * TextRendering_CharWidth(window) * ls;
            TextRendering_PrintString(window, s1, gbx[0] - ws1*0.5f, gbcy - 0.03f, ls);
            TextRendering_PrintString(window, s2, gbx[1] - ws2*0.5f, gbcy - 0.03f, ls);
        }

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// (FindCollidingEntityIndex e CheckCollision foram movidas para collisions.cpp,
//  conforme a especificação do trabalho.)

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename, bool nearest, bool tiling, bool alpha)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco. alpha=true mantém o canal
    // de transparência (RGBA), usado por imagens com fundo transparente (logos).
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    int desired = alpha ? 4 : 3;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, desired);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    // tiling=true -> GL_REPEAT (texturas que se repetem, usadas no triplanar).
    GLenum wrap = tiling ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, wrap);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, wrap);

    // Parâmetros de amostragem. nearest=true (sem mipmap) evita "borrar" cores em
    // texturas tipo paleta (ex.: a paleta do boneco).
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, nearest ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, nearest ? GL_NEAREST : GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    GLint  internalFmt = alpha ? GL_SRGB8_ALPHA8 : GL_SRGB8;
    GLenum dataFmt     = alpha ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, width, height, 0, dataFmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_light_tint_uniform = glGetUniformLocation(g_GpuProgramID, "light_tint"); // Tom da luz (varia na captura)
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage3"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage4"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage5"), 5);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage6"), 6);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage7"), 7);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage8"), 8);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage9"), 9);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage10"), 10);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage11"), 11);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage12"), 12);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage13"), 13);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage14"), 14);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage15"), 15);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage16"), 16);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage17"), 17);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage18"), 18);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage19"), 19);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage20"), 20);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage21"), 21);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage22"), 22);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage23"), 23);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage24"), 24);
    glUseProgram(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice e que pertencem ao mesmo "smoothing group".

    // Obtemos a lista dos smoothing groups que existem no objeto
    std::set<unsigned int> sgroup_ids;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        assert(model->shapes[shape].mesh.smoothing_group_ids.size() == num_triangles);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);
            unsigned int sgroup = model->shapes[shape].mesh.smoothing_group_ids[triangle];
            assert(sgroup >= 0);
            sgroup_ids.insert(sgroup);
        }
    }

    size_t num_vertices = model->attrib.vertices.size() / 3;
    model->attrib.normals.reserve( 3*num_vertices );

    // Processamos um smoothing group por vez
    for (const unsigned int & sgroup : sgroup_ids)
    {
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

        // Acumulamos as normais dos vértices de todos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                glm::vec4  vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                    const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                    const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
                }

                const glm::vec4  a = vertices[0];
                const glm::vec4  b = vertices[1];
                const glm::vec4  c = vertices[2];

                const glm::vec4  n = crossproduct(b-a,c-a);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                }
            }
        }

        // Computamos a média das normais acumuladas
        std::vector<size_t> normal_indices(num_vertices, 0);

        for (size_t vertex_index = 0; vertex_index < vertex_normals.size(); ++vertex_index)
        {
            if (num_triangles_per_vertex[vertex_index] == 0)
                continue;

            glm::vec4 n = vertex_normals[vertex_index] / (float)num_triangles_per_vertex[vertex_index];
            n /= norm(n);

            model->attrib.normals.push_back( n.x );
            model->attrib.normals.push_back( n.y );
            model->attrib.normals.push_back( n.z );

            size_t normal_index = (model->attrib.normals.size() / 3) - 1;
            normal_indices[vertex_index] = normal_index;
        }

        // Escrevemos os índices das normais para os vértices dos triângulos deste smoothing group
        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];

                if (sgroup_tri != sgroup)
                    continue;

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index =
                        normal_indices[ idx.vertex_index ];
                }
            }
        }

    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;
    std::vector<float>  color_coefficients;

    // Alguns modelos (ex.: o boneco do jogador) trazem uma cor por vértice
    // embutida no .obj (linhas "v x y z r g b"). A tinyobjloader as guarda em
    // attrib.colors. Quando não existem, ela preenche com branco (1,1,1).
    bool has_vertex_colors = !model->attrib.colors.empty();

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        // Em 2026-05-18, corrigido bug encontrado pelo aluno Arthur Prediger:
        // std::numeric_limits<float>::min() retorna o menor valor positivo
        // normalizado representável, não o menor valor possível (negativo). Para
        // inicializar o limite máximo da bounding box com um valor "muito
        // pequeno", deve ser usado std::numeric_limits<float>::lowest()
        const float minval = std::numeric_limits<float>::lowest();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }

                if ( has_vertex_colors )
                {
                    color_coefficients.push_back( model->attrib.colors[3*idx.vertex_index + 0] );
                    color_coefficients.push_back( model->attrib.colors[3*idx.vertex_index + 1] );
                    color_coefficients.push_back( model->attrib.colors[3*idx.vertex_index + 2] );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !color_coefficients.empty() )
    {
        GLuint VBO_color_coefficients_id;
        glGenBuffers(1, &VBO_color_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, color_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, color_coefficients.size() * sizeof(float), color_coefficients.data());
        location = 3; // "(location = 3)" em "shader_vertex.glsl"
        number_of_dimensions = 3; // vec3 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// "Sim" no modal do ginásio. Se o ginásio está LIVRE, abre o armazenamento para
// colocar um Pokémon. Se está OCUPADO, dá uma fruta ao Pokémon (caso o jogador
// tenha): a fruta voa do jogador até o Pokémon (acima do ginásio) por Bézier.
void GymModalAccept()
{
    if (g_GymModalIndex < 0) { g_GymModalOpen = false; return; }
    Gym& gym = g_Gyms[g_GymModalIndex];

    if (gym.pokemonType < 0)
    {
        // Ginásio livre: abre o armazenamento em modo "colocar".
        g_PlacingGymIndex = g_GymModalIndex;
        g_StorageOpen     = true;
        g_StorageDetail   = -1;
    }
    else if (g_NumBerries > 0)
    {
        // Ginásio ocupado: dá uma fruta ao Pokémon (voa por Bézier cúbica).
        g_NumBerries--;
        glm::vec3 start = glm::vec3(g_PlayerX, -0.6f, g_PlayerZ);
        glm::vec3 endp  = glm::vec3(gym.position.x, gym.position.y + 1.75f, gym.position.z);
        glm::vec3 d = endp - start;
        FlyingItem fi;
        fi.type = ITEM_BERRY;
        fi.p0 = start;
        fi.p1 = start + d*0.33f + glm::vec3(0.0f, 0.9f, 0.0f);
        fi.p2 = start + d*0.66f + glm::vec3(0.0f, 0.9f, 0.0f);
        fi.p3 = endp;
        fi.t  = 0.0f;
        g_FlyingItems.push_back(fi);
        g_Message = "Voce deu uma fruta!"; g_MessageTimer = 2.5f;
    }
    else
    {
        // Ginásio ocupado mas sem frutas no inventário.
        g_Message = "Voce nao tem frutas!"; g_MessageTimer = 2.5f;
    }

    g_GymModalOpen  = false;
    g_GymModalIndex = -1;
}

// Recusa: fecha o modal e marca o ginásio para não reperguntar até o jogador
// se afastar dele.
void GymModalDecline()
{
    g_GymModalOpen  = false;
    g_GymModalIndex = -1;
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // ===== Clique no menu principal =====
        if (g_MenuState == MenuState::MainMenu)
        {
            double mx, my; glfwGetCursorPos(window, &mx, &my);
            int ww, wh; glfwGetWindowSize(window, &ww, &wh);
            float ndcX = (float)(2.0 * mx / ww - 1.0);
            float ndcY = (float)(1.0 - 2.0 * my / wh);

            const float itemCY[3] = { 0.05f, -0.20f, -0.45f };
            const float itemHW = 0.30f, itemHH = 0.09f;
            for (int i = 0; i < 3; ++i)
            {
                if (fabs(ndcX) < itemHW && fabs(ndcY - itemCY[i]) < itemHH)
                {
                    g_MenuSelection = i;
                    if (i == 0) { g_MenuState = MenuState::Playing; }
                    else if (i == 1) { g_MenuState = MenuState::HowToPlay; }
                    else if (i == 2) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
                    return;
                }
            }
            return;
        }

        // ===== Clique na tela "Como Jogar" volta ao menu =====
        if (g_MenuState == MenuState::HowToPlay)
        {
            g_MenuState = MenuState::MainMenu;
            return;
        }

        // Modal de escolha de time (abertura do jogo): clicar num dos botões
        // define o time e fecha o modal. Enquanto aberto, nada mais é clicável.
        if (g_TeamModalOpen)
        {
            double mx, my; glfwGetCursorPos(window, &mx, &my);
            int ww, wh; glfwGetWindowSize(window, &ww, &wh);
            float ndcX = (float)(2.0 * mx / ww - 1.0);
            float ndcY = (float)(1.0 - 2.0 * my / wh);

            const float bx[3]   = { -0.34f, 0.00f, 0.34f };
            const Team  team[3] = { Team::Red, Team::Blue, Team::Yellow };
            const float bhw = 0.13f / g_ScreenRatio;
            const float bhh = 0.20f;
            const float bcy = -0.06f;
            for (int b = 0; b < 3; ++b)
            {
                if (fabs(ndcX - bx[b]) < bhw && fabs(ndcY - bcy) < bhh)
                {
                    g_PlayerTeam    = team[b];
                    g_TeamModalOpen = false;
                    break;
                }
            }
            return; // modal aberto: nenhum outro clique tem efeito
        }

        // Modal do ginásio: clicar em "Sim" (deixa Pokémon) ou "Nao" (recusa).
        if (g_GymModalOpen)
        {
            double mx, my; glfwGetCursorPos(window, &mx, &my);
            int ww, wh; glfwGetWindowSize(window, &ww, &wh);
            float ndcX = (float)(2.0 * mx / ww - 1.0);
            float ndcY = (float)(1.0 - 2.0 * my / wh);

            const float gbhw = 0.16f / g_ScreenRatio;
            const float gbhh = 0.12f;
            const float gbcy = -0.10f;
            if (fabs(ndcY - gbcy) < gbhh)
            {
                if (fabs(ndcX - (-0.26f)) < gbhw)      GymModalAccept();  // Sim
                else if (fabs(ndcX - 0.26f) < gbhw)    GymModalDecline(); // Nao
            }
            return; // modal aberto: nenhum outro clique tem efeito
        }

        // Clique no ícone de armazenamento (canto inferior esquerdo) -> abre/fecha
        // a janela. Quando a janela está aberta, os cliques não afetam o mundo.
        // (Desabilitado durante a cena de captura.)
        // Durante a animação de evolução, ignoramos cliques (não dá pra navegar
        // nem cancelar até a transformação terminar).
        if (g_EvolvingIndex >= 0)
            return;

        if (g_CurrentScene != GameScene::Capture)
        {
            double mx, my; glfwGetCursorPos(window, &mx, &my);
            int ww, wh; glfwGetWindowSize(window, &ww, &wh);
            float ndcX = (float)(2.0 * mx / ww - 1.0);
            float ndcY = (float)(1.0 - 2.0 * my / wh);
            float ihw  = UI_ICON_HH / g_ScreenRatio;
            if (fabs(ndcX - UI_ICON_CX) < ihw && fabs(ndcY - UI_ICON_CY) < UI_ICON_HH)
            {
                g_StorageOpen     = !g_StorageOpen;
                g_StorageDetail   = -1;
                g_PlacingGymIndex = -1; // alternar o ícone cancela a colocação
                return;
            }
            if (g_StorageOpen)
            {
                if (g_StorageDetail >= 0)
                {
                    const CapturedPokemon& pd = g_Captured[g_StorageDetail];
                    int  fam     = (pd.type >= 0 && pd.type < 5) ? g_CandyFamily[pd.type] : 0;
                    int  evoTo   = (pd.type >= 0 && pd.type < 5) ? g_EvolvesTo[pd.type]   : -1;
                    int  evoCost = (pd.type >= 0 && pd.type < 5) ? g_EvolveCost[pd.type]  : 0;
                    bool inEvoBtn = (fabs(ndcX - EVO_BTN_CX) < EVO_BTN_HW
                                  && fabs(ndcY - EVO_BTN_CY) < EVO_BTN_HH);

                    if (inEvoBtn && evoTo >= 0 && g_Candies[fam] >= evoCost && pd.placedGym < 0)
                    {
                        // Clique no botão "Evoluir" com doces suficientes: começa a
                        // animação (a troca de forma e o gasto de doces ocorrem nela).
                        g_Candies[fam] -= evoCost;
                        g_EvolvingIndex = g_StorageDetail;
                        g_EvolveToType  = evoTo;
                        g_EvolveTimer   = 0.0f;
                        g_EvolveSwapped = false;
                    }
                    // detalhe: clicar fora do painel fecha a janela; dentro volta à grade
                    else if (fabs(ndcX) > 0.78f || fabs(ndcY) > 0.92f)
                    {
                        g_StorageOpen   = false;
                        g_StorageDetail = -1;
                    }
                    else
                    {
                        g_StorageDetail = -1;
                    }
                }
                else
                {
                    // grade: clique numa miniatura abre o detalhe daquele Pokémon
                    const int COLS = 4, VIS_ROWS = 3;
                    const float ths = 0.13f;
                    const float thw = ths / g_ScreenRatio;
                    const float xcols[4] = { -0.42f, -0.14f, 0.14f, 0.42f };
                    int start = g_StorageScrollRow * COLS;
                    int end   = start + VIS_ROWS * COLS;
                    bool hit = false;
                    for (int i = start; i < end && i < g_CapturedCount; ++i)
                    {
                        int   rel = (i / COLS) - g_StorageScrollRow;
                        float x   = xcols[i % COLS];
                        float y   = 0.34f - rel * 0.32f;
                        if (fabs(ndcX - x) < thw && fabs(ndcY - y) < ths)
                        {
                            if (g_PlacingGymIndex >= 0)
                            {
                                // Modo "colocar": só se este Pokémon NÃO estiver já
                                // em outro ginásio. Caso esteja, ignora o clique
                                // (avisa) e continua aguardando uma escolha válida.
                                if (g_Captured[i].placedGym >= 0)
                                {
                                    g_Message      = "Esse Pokemon ja esta num ginasio!";
                                    g_MessageTimer = 2.5f;
                                }
                                else
                                {
                                    g_Gyms[g_PlacingGymIndex].team        = g_PlayerTeam;
                                    g_Gyms[g_PlacingGymIndex].pokemonType = g_Captured[i].type;
                                    g_Captured[i].placedGym = g_PlacingGymIndex; // marca como ocupado
                                    g_PlacingGymIndex  = -1;
                                    g_StorageOpen      = false;
                                    g_StorageDetail    = -1;
                                }
                            }
                            else
                            {
                                g_StorageDetail = i; // abre o detalhe daquele Pokémon
                            }
                            hit = true;
                            break;
                        }
                    }
                    // Clique fora do painel (e não numa miniatura) fecha a janela
                    // (e cancela uma colocação em andamento).
                    if (!hit && (fabs(ndcX) > 0.58f || fabs(ndcY) > 0.66f))
                    {
                        g_StorageOpen     = false;
                        g_PlacingGymIndex = -1;
                    }
                }
                return; // janela aberta: cliques não afetam o mundo
            }
        }

        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_PressPosX = g_LastCursorPosX; // guardamos onde o clique começou
        g_PressPosY = g_LastCursorPosY;

        if (g_CurrentScene == GameScene::Capture)
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            double xpos = g_LastCursorPosX;
            double ypos = g_LastCursorPosY;

            float ndcX = static_cast<float>(2.0 * xpos / width - 1.0);
            float ndcY = static_cast<float>(1.0 - 2.0 * ypos / height);

            const float button_left   = 0.50f;
            const float button_right  = 0.72f;
            const float button_bottom = 0.82f;
            const float button_top    = 0.95f;

            if (ndcX >= button_left && ndcX <= button_right
                && ndcY >= button_bottom && ndcY <= button_top)
            {
                g_CurrentScene = GameScene::World;
                g_CaptureTargetIndex = -1;
            }
        }

        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;

        // Se o cursor quase não se moveu desde o pressionar, foi um "clique"
        // (não um arrasto de câmera). No mapa (sem modais/janelas abertos),
        // marcamos para testar se acertou o balão ou um ginásio (o teste em si é
        // feito no loop de desenho, onde temos as matrizes view/projection).
        if (g_CurrentScene == GameScene::World
            && !g_TeamModalOpen && !g_GymModalOpen && !g_StorageOpen)
        {
            double cx, cy;
            glfwGetCursorPos(window, &cx, &cy);
            double mdx = cx - g_PressPosX;
            double mdy = cy - g_PressPosY;
            if (mdx*mdx + mdy*mdy < 25.0) // < 5 px de movimento
            {
                g_BalloonClickCheck = true;
                g_GymClickCheck     = true;
                g_StopClickCheck    = true;
                g_PokeClickCheck    = true;
                g_ClickPosX = cx;
                g_ClickPosY = cy;
            }
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;

        // Mantemos a câmera numa faixa de inclinação "estilo Pokémon GO":
        // nunca totalmente de cima (top-down) nem tão rente ao chão a ponto de
        // ver as placas de borda "de canto" (a textura esticaria no ângulo rasante).
        float phimax = 1.1f;   // ~63°: olhar mais de cima
        float phimin = 0.45f;  // ~26°: limite inferior (evita ângulo rasante)

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_ForearmAngleZ -= 0.01f*dx;
        g_ForearmAngleX += 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_TorsoPositionX += 0.01f*dx;
        g_TorsoPositionY -= 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Se a janela de armazenamento está aberta, a rodinha rola a lista.
    if (g_StorageOpen)
    {
        g_StorageScrollRow -= (int) yoffset;
        const int COLS = 4, VIS_ROWS = 3;
        int totalRows = (g_CapturedCount + COLS - 1) / COLS;
        int maxScroll = totalRows - VIS_ROWS;
        if (maxScroll < 0) maxScroll = 0;
        if (g_StorageScrollRow < 0) g_StorageScrollRow = 0;
        if (g_StorageScrollRow > maxScroll) g_StorageScrollRow = maxScroll;
        return;
    }

    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

void Correcao_KeyCallback(int key, int action, int mod);

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // =======================
    // Não modifique esta chamada! Ela é utilizada para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    Correcao_KeyCallback(key, action, mod);
    // =======================

    // ===== Navegação no menu principal =====
    if (g_MenuState == MenuState::MainMenu && action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W)
            { g_MenuSelection = (g_MenuSelection + 2) % 3; return; }
        if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
            { g_MenuSelection = (g_MenuSelection + 1) % 3; return; }
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
        {
            if (g_MenuSelection == 0) { g_MenuState = MenuState::Playing;  return; }
            if (g_MenuSelection == 1) { g_MenuState = MenuState::HowToPlay; return; }
            if (g_MenuSelection == 2) { glfwSetWindowShouldClose(window, GLFW_TRUE); return; }
        }
        if (key == GLFW_KEY_ESCAPE)
            { glfwSetWindowShouldClose(window, GLFW_TRUE); return; }
    }

    // ===== Tela "Como Jogar" -> ESC/Enter volta ao menu =====
    if (g_MenuState == MenuState::HowToPlay && action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
            { g_MenuState = MenuState::MainMenu; return; }
    }

    // Se ainda estamos no menu, não processar as teclas do jogo
    if (g_MenuState != MenuState::Playing)
        return;

    // Modal do ginásio: Y deixa um Pokémon (abre o armazenamento), N recusa.
    if (g_GymModalOpen && action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_Y) { GymModalAccept();  return; }
        if (key == GLFW_KEY_N) { GymModalDecline(); return; }
    }

    // ESC: fecha o modal do ginásio / a janela de armazenamento, ou sai da captura.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if (g_GymModalOpen)
        {
            GymModalDecline();
        }
        else if (g_StorageOpen)
        {
            g_StorageOpen     = false;
            g_StorageDetail   = -1;
            g_PlacingGymIndex = -1; // cancela colocação em andamento
        }
        else if (g_CurrentScene == GameScene::Capture)
        {
            g_CurrentScene = GameScene::World;
            g_CaptureTargetIndex = -1;
        }
    }

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        g_ForearmAngleX = 0.0f;
        g_ForearmAngleZ = 0.0f;
        g_TorsoPositionX = 0.0f;
        g_TorsoPositionY = 0.0f;
        g_CameraTheta = 0.0f;
        g_CameraPhi = 0.6f;
        g_CameraDistance = 3.0f;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = false;
    }

    // Tecla C: alterna entre câmera em 3ª pessoa (segue o jogador) e câmera livre.
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        g_FreeCamera = !g_FreeCamera;
        if (g_FreeCamera)
        {
            // Inicia a câmera livre na posição atual da câmera orbital (sem "pulo").
            float r  = g_CameraDistance;
            float yy = r*sin(g_CameraPhi);
            float zz = r*cos(g_CameraPhi)*cos(g_CameraTheta);
            float xx = r*cos(g_CameraPhi)*sin(g_CameraTheta);
            g_FreeCamPos = glm::vec3(xx + g_PlayerX, yy, zz + g_PlayerZ);
        }
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    // Movimento do personagem com WASD ou setas
    if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
        g_KeyW = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
        g_KeyS = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)
        g_KeyA = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT)
        g_KeyD = (action != GLFW_RELEASE);

    // Q/E: descer/subir na câmera livre
    if (key == GLFW_KEY_Q)
        g_KeyQ = (action != GLFW_RELEASE);
    if (key == GLFW_KEY_E)
        g_KeyE = (action != GLFW_RELEASE);

    // L: carrega a captura (segurar) na cena de captura
    if (key == GLFW_KEY_L)
        g_KeyL = (action != GLFW_RELEASE);
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( g_UsePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
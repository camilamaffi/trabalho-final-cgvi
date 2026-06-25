#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Cor por vértice (modelos coloridos, ex.: boneco do jogador)
in vec3 vertex_color;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Tom/intensidade da luz (1,1,1 = neutro). Varia na cena de captura para simular
// o período do dia. Enviado pelo código C++ a cada frame.
uniform vec3 light_tint;

// Identificador que define qual objeto está sendo desenhado no momento
#define CUBE         1
#define PLANE        2
#define PIKACHU      3
#define TREE         4
#define FOREST_WALL  5
#define PLAYER       6
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
#define CHARMANDER        19
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
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3; // paleta do jogador (via UV)
uniform sampler2D TextureImage4; // pikachu (via UV)
uniform sampler2D TextureImage5; // pedra (ginásio / PokéStop em cooldown)
uniform sampler2D TextureImage6; // metal (PokéStop)
uniform sampler2D TextureImage7; // tecido escuro (balão)
uniform sampler2D TextureImage8; // grama (borda do mapa)
uniform sampler2D TextureImage9; // pokébola (via UV)
uniform sampler2D TextureImage10; // ícone do armazenamento (UI)
uniform sampler2D TextureImage11; // charmander (via UV)
uniform sampler2D TextureImage12; // fundo da cena de captura (mapa-captura.png)
uniform sampler2D TextureImage13; // logo do time Valor (UI, com alpha)
uniform sampler2D TextureImage14; // logo do time Mystic (UI, com alpha)
uniform sampler2D TextureImage15; // logo do time Instinct (UI, com alpha)
uniform sampler2D TextureImage16; // snorlax (via UV)
uniform sampler2D TextureImage17; // charmeleon (atlas das 5 texturas do glTF, via UV)
uniform sampler2D TextureImage18; // charmander HD (atlas das texturas do glTF, via UV)
uniform sampler2D TextureImage19; // pikachu HD (atlas das texturas do glTF, via UV)
uniform sampler2D TextureImage20; // raichu (atlas das texturas do glTF, via UV)
uniform sampler2D TextureImage21; // snorlax HD (atlas texturas+cores do glTF, via UV)

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// Mapeamento triplanar: amostra a textura por 3 projeções planares (XY, YZ, XZ)
// e mistura conforme a normal. Usado em objetos sem coordenadas de textura (UV),
// evitando que a imagem fique "esticada".
vec3 triplanar(sampler2D tex, vec3 p, vec3 nrm, float scale)
{
    vec3 b = abs(normalize(nrm));
    b /= (b.x + b.y + b.z);
    vec3 cx = texture(tex, p.yz * scale).rgb;
    vec3 cy = texture(tex, p.xz * scale).rgb;
    vec3 cz = texture(tex, p.xy * scale).rgb;
    return cx*b.x + cy*b.y + cz*b.z;
}

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

	// Coeficiente de refletância difusa
	vec3 Kd0 = vec3(0.0); // inicializado por segurança (evita lixo se cair fora dos casos)

    if ( object_id == CUBE )
    {
        Kd0 = vec3(0.2, 0.45, 0.9);
    }
    else if ( object_id == PLAYER )
    {
        // Boneco do jogador: textura-paleta do modelo, mapeada pelas UVs
        Kd0 = texture(TextureImage3, texcoords).rgb;
    }
    else if ( object_id == PIKACHU )
    {
        // Pikachu: textura amarela, mapeada pelas UVs do modelo
        Kd0 = texture(TextureImage4, texcoords).rgb;
    }
    else if ( object_id == CHARMANDER )
    {
        // Charmander: textura laranja, mapeada pelas UVs do modelo
        Kd0 = texture(TextureImage11, texcoords).rgb;
    }
    else if ( object_id == SNORLAX )
    {
        // Snorlax: textura teal, mapeada pelas UVs do modelo
        Kd0 = texture(TextureImage16, texcoords).rgb;
    }
    else if ( object_id == CHARMELEON )
    {
        // Charmeleon (evolução): textura REAL do glTF, unida num atlas e mapeada
        // pelas UVs remapeadas do modelo. Partes transparentes (alpha) são
        // descartadas para não virarem "placas pretas".
        vec4 t = texture(TextureImage17, texcoords);
        if ( t.a < 0.5 ) discard;
        Kd0 = t.rgb;
    }
    else if ( object_id == CHARMANDER_HD )
    {
        // Charmander HD (exibição): textura real do glTF (atlas), via UV.
        Kd0 = texture(TextureImage18, texcoords).rgb;
    }
    else if ( object_id == PIKACHU_HD )
    {
        // Pikachu HD (exibição): textura real do glTF (atlas), via UV.
        Kd0 = texture(TextureImage19, texcoords).rgb;
    }
    else if ( object_id == RAICHU )
    {
        // Raichu (evolução): textura real do glTF (atlas), via UV. As bochechas
        // têm fundo transparente (alpha) -> descarta para não virar placa preta.
        vec4 t = texture(TextureImage20, texcoords);
        if ( t.a < 0.5 ) discard;
        Kd0 = t.rgb;
    }
    else if ( object_id == SNORLAX_HD )
    {
        // Snorlax HD (exibição): textura real do glTF (atlas), via UV.
        Kd0 = texture(TextureImage21, texcoords).rgb;
    }
    else if ( object_id == PLANE )
    {
        float cityHalf = 5.0;

        if (abs(position_world.x) <= cityHalf && abs(position_world.z) <= cityHalf)
        {
            // Região central: map.png
            U = (position_world.x + cityHalf) / (2.0 * cityHalf);
            V = (position_world.z + cityHalf) / (2.0 * cityHalf);
            Kd0 = texture(TextureImage0, vec2(U, V)).rgb;
        }
        else
        {
            // Região externa: textura de grama (mapeamento planar por X,Z do mundo)
            Kd0 = texture(TextureImage8, position_world.xz * 0.5).rgb;
        }
    }
    else if ( object_id == TREE )
    {
        Kd0 = vec3(0.15, 0.55, 0.15);
    }
    else if ( object_id == POKESTOP )
    {
        // PokéStop disponível: textura metálica (triplanar) tingida de azul.
        Kd0 = triplanar(TextureImage6, position_world.xyz, n.xyz, 2.0) * vec3(0.20, 0.65, 1.10);
    }
    else if ( object_id == POKESTOP_COOLDOWN )
    {
        // PokéStop em cooldown: textura de pedra (triplanar) tingida de cinza.
        Kd0 = triplanar(TextureImage5, position_world.xyz, n.xyz, 2.0) * vec3(0.75, 0.75, 0.75);
    }
    else if ( object_id == GYM )
    {
        // Ginásio livre: textura de pedra (triplanar), tom cinza
        Kd0 = triplanar(TextureImage5, position_world.xyz, n.xyz, 1.2);
    }
    else if ( object_id == GYM_RED )
    {
        // Ginásio do time Vermelho: pedra tingida de vermelho. O canal R passa de
        // 1.0 para compensar a pedra escura e deixar o vermelho mais vivo/claro.
        Kd0 = triplanar(TextureImage5, position_world.xyz, n.xyz, 1.2) * vec3(1.60, 0.50, 0.48);
    }
    else if ( object_id == GYM_BLUE )
    {
        // Ginásio do time Azul: pedra tingida de azul
        Kd0 = triplanar(TextureImage5, position_world.xyz, n.xyz, 1.2) * vec3(0.35, 0.50, 0.98);
    }
    else if ( object_id == GYM_YELLOW )
    {
        // Ginásio do time Amarelo: pedra tingida de amarelo
        Kd0 = triplanar(TextureImage5, position_world.xyz, n.xyz, 1.2) * vec3(1.00, 0.85, 0.30);
    }
    else if ( object_id == GYM_MODEL )
    {
        // Modelo de ginásio do Pokémon GO: textura de pedra (triplanar) modulada
        // pela cor por vértice (que já codifica o time; cinza = livre). Assim a
        // cor vem da textura E mantém a identidade do time.
        Kd0 = triplanar(TextureImage5, position_world.xyz, n.xyz, 2.0) * vertex_color * 1.4;
    }
    else if ( object_id == ROCKET )
    {
        // Balão da Equipe Rocket: textura escura (triplanar)
        Kd0 = triplanar(TextureImage7, position_world.xyz, n.xyz, 1.5);
    }
    else if ( object_id == ROCKET_BASKET )
    {
        // Cesto/cordas do balão: textura escura (triplanar)
        Kd0 = triplanar(TextureImage7, position_world.xyz, n.xyz, 4.0);
    }
    else if ( object_id == POKEBALL )
    {
        // Pokébola: textura (vermelho/preto/branco) mapeada pelas UVs do modelo
        Kd0 = texture(TextureImage9, texcoords).rgb;
    }
    else if ( object_id == ROCKET_R )
    {
        // "R" da Equipe Rocket: textura (R vermelho em fundo preto). Descartamos
        // o fundo (fragmentos escuros) para mostrar só a letra sobre o balão.
        vec3 rtex = texture(TextureImage2, texcoords).rgb;
        if (rtex.r < 0.35)
            discard;
        Kd0 = rtex;
    }
    else if ( object_id == FOREST_WALL )
    {
        // Painéis verticais da floresta. V invertido para a imagem ficar em pé
        // (chão embaixo) com a orientação que levanta a placa virada ao centro.
        U = texcoords.x;
        V = 1.0 - texcoords.y;
        Kd0 = texture(TextureImage1, vec2(U, V)).rgb;
    }
    else if ( object_id == CAPTURE_BG )
    {
        // Fundo da cena de captura: imagem mapa-captura.png (gerada pelo ChatGPT).
        Kd0 = texture(TextureImage12, texcoords).rgb;
    }
    else if ( object_id == UI_PANEL )
    {
        // Painel da janela de armazenamento (escuro)
        Kd0 = vec3(0.11, 0.12, 0.20);
    }
    else if ( object_id == UI_BORDER )
    {
        // Borda da caixinha de cada Pokémon (dourado claro)
        Kd0 = vec3(0.95, 0.82, 0.30);
    }
    else if ( object_id == UI_TEAM_RED )
    {
        Kd0 = vec3(0.85, 0.18, 0.18); // Time Valor
    }
    else if ( object_id == UI_TEAM_BLUE )
    {
        Kd0 = vec3(0.16, 0.40, 0.85); // Time Mystic
    }
    else if ( object_id == UI_TEAM_YELLOW )
    {
        Kd0 = vec3(0.95, 0.78, 0.15); // Time Instinct
    }
    else if ( object_id == UI_ICON )
    {
        Kd0 = texture(TextureImage10, texcoords).rgb;
    }

    // Logos dos times: imagens com transparência. Amostramos RGBA e descartamos
    // os fragmentos transparentes (fundo), mostrando só o emblema sobre o botão.
    if ( object_id == UI_LOGO_VALOR || object_id == UI_LOGO_MYSTIC || object_id == UI_LOGO_INSTINCT )
    {
        vec4 logo;
        if ( object_id == UI_LOGO_VALOR )       logo = texture(TextureImage13, texcoords);
        else if ( object_id == UI_LOGO_MYSTIC ) logo = texture(TextureImage14, texcoords);
        else                                    logo = texture(TextureImage15, texcoords);
        if ( logo.a < 0.5 )
            discard;
        color = vec4(pow(logo.rgb, vec3(1.0/2.2)), 1.0);
        return;
    }

    // Objetos "chapados" (sem iluminação): fundo de captura e interface 2D.
    if ( object_id == CAPTURE_BG || object_id == UI_PANEL || object_id == UI_BORDER
         || object_id == UI_ICON
         || object_id == UI_TEAM_RED || object_id == UI_TEAM_BLUE || object_id == UI_TEAM_YELLOW )
    {
        color = vec4(pow(Kd0, vec3(1.0/2.2)), 1.0);
        return;
    }

    // ===== Modelo de iluminação de Blinn-Phong =====
    // (ambiente + difusa + especular), aplicado a todos os objetos.

    // Propriedades especulares do material. Por padrão um brilho moderado;
    // objetos foscos (chão, grama, floresta, árvores) não têm especular.
    vec3  Ks = vec3(0.30);  // refletância especular
    float q  = 32.0;        // expoente especular (quanto maior, mais concentrado)

    if ( object_id == PLANE || object_id == FOREST_WALL || object_id == TREE )
    {
        Ks = vec3(0.0);
        q  = 1.0;
    }
    else if ( object_id == PIKACHU || object_id == CHARMANDER || object_id == SNORLAX || object_id == CHARMELEON || object_id == CHARMANDER_HD || object_id == PIKACHU_HD || object_id == RAICHU || object_id == SNORLAX_HD || object_id == POKESTOP || object_id == GYM_TOP || object_id == POKEBALL )
    {
        // Pikachu, Charmander, disco do PokéStop, dourado do gym e pokébola: mais "polidos"
        Ks = vec3(0.5);
        q  = 64.0;
    }

    // Intensidades da fonte de luz e da luz ambiente. O "light_tint" varia a cor/
    // intensidade da luz (usado na cena de captura para simular o período do dia).
    vec3 I  = vec3(1.0, 1.0, 1.0) * light_tint; // espectro da fonte de luz
    vec3 Ia = vec3(0.25)          * light_tint; // espectro da luz ambiente

    // Reflectâncias ambiente e difusa derivadas da cor do objeto (Kd0)
    vec3 Kd = Kd0;
    vec3 Ka = Kd0;

    // Vetor "half" de Blinn-Phong (bissetriz entre luz e câmera)
    vec4 h = normalize(l + v);

    float lambert  = max(0.0, dot(n, l));
    float specular = pow(max(0.0, dot(n, h)), q);

    vec3 ambient_term  = Ka * Ia;                 // termo ambiente
    vec3 diffuse_term  = Kd * I * lambert;        // termo difuso (Lambert)
    vec3 specular_term = Ks * I * specular;       // termo especular (Blinn-Phong)

    color.rgb = ambient_term + diffuse_term + specular_term;

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 


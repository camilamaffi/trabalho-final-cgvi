# PROMPTS.md

Registro dos prompts usados com IA para gerar/editar código neste
trabalho, conforme exigido pelo enunciado. Em ordem cronológica.

---

**1. Câmera + avatar (estilo Pokémon GO)**
Prompt: "Ajuste a câmera look-at em terceira pessoa para um ângulo de elevação
mais baixo e faça o modelo do jogador rotacionar (yaw) na direção do vetor de
movimento. Ignore a captura por enquanto."
→ `g_CameraPhi` 1.0→0.6 + clamp do pitch; `g_PlayerAngleY` via `atan2`. (`src/main.cpp`)

**2. Malha do jogador**
Prompt: "Substitua o cubo do jogador por uma malha poligonal de um personagem
humanoide low-poly (modelo externo de licença livre), com as cores via cor por
vértice. Adicione o atributo de cor (location 3) ao pipeline."
→ Modelo Adventurer (Quaternius, CC0), GLB→OBJ com cor por vértice; atributo no
vertex/fragment shader. (`src/main.cpp`, `src/shader_vertex.glsl`,
`src/shader_fragment.glsl`, `data/adventurer.obj`)

**3. Pikachus por proximidade (teste de intersecção)**
Prompt: "Instancie vários pikachus e use teste de proximidade (distância
jogador↔objeto) para exibi-los/ocultá-los com interpolação suave de escala. Reduza
o raio de ativação."
→ `appearRadius` em `SceneEntity`; fade por distância; raio 2.2→1.2. (`src/main.cpp`)

**4. PokéStops**
Prompt: "Crie PokéStops como instâncias da mesma malha, com coleta de itens por
teste de proximidade, cooldown temporizado e HUD de inventário. Disco vertical
(esfera achatada) + anel (torus) com rotações em eixos independentes; cooldown de
60 s; estoque limitado a 100 por item."
→ PokéStops (poste+disco+anel), inventário, estados por cor, cooldown 60 s, máx 100.
(`src/main.cpp`, `src/shader_fragment.glsl`, `data/torus.obj`)

**5. Ginásio + instâncias**
Prompt: "Adicione o ginásio (apenas a estrutura) a partir das peças .stl em data/:
monte-as e converta para um único OBJ com cor por vértice. Use como instâncias —
5 ginásios e 13 PokéStops (várias Model matrix sobre a mesma malha)."
→ STLs de `data/gym/gym/` montados em `data/gym.obj`; 5 ginásios e 13 PokéStops.
(`src/main.cpp`, `src/shader_fragment.glsl`, `data/gym.obj`)

**6. Balão (curva de Bézier cúbica)**
Prompt: "Adicione um objeto (balão) cuja movimentação seja definida por uma curva
de Bézier cúbica, em loop suave pelo céu, com animação baseada em Δt. Inclua
picking por clique (mostra mensagem) e um logo 'R' via textura num quad orientado
para a câmera (billboard)."
→ Loop de 4 Béziers cúbicas (`EvalCubicBezier`), Δt; envelope+cesto+cordas; picking;
"R" texturizado (`data/rocket_r.png`). (`src/main.cpp`, `src/shader_fragment.glsl`)

**7. Iluminação especular**
Prompt: "Implemente o modelo de iluminação de Blinn-Phong (ambiente + difusa +
especular) por fragmento para todos os objetos, com parâmetros de material (Ks, q)
distintos entre superfícies foscas e polidas."
→ Blinn-Phong com half-vector; materiais por `object_id`. (`src/shader_fragment.glsl`)

**8. Câmera livre (2º tipo de câmera)**
Prompt: "Implemente uma câmera livre alternável por tecla: posição via teclado
(WASD na horizontal, E/Q em altura) e orientação via mouse. Restrinja a posição
ao volume do mapa (altura limitada às árvores de borda e XZ dentro das paredes)."
→ Câmera livre (tecla C); movimento horizontal + E/Q; clamps de altura e XZ.
(`src/main.cpp`)

**9. Comentários de FONTE**
Prompt: "Adicione comentários com a palavra FONTE no carregamento dos assets de
terceiros (ginásio, modelo do jogador, textura externa)."
→ Comentários `FONTE` nos loads correspondentes. (`src/main.cpp`)

**10. Sistema de colisão**
Prompt: "Faça um sistema de colisão para quando mover o cubo, ele tenha colisão com outros objetos"
Prompt: "Agora deixe esse codigo genérico para quando houver multiplos objetos no cenário"

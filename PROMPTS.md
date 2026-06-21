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

**11. Mapeamento de texturas em todos os objetos**
Prompt: "Faça TODOS os objetos terem a cor definida por textura (≥3 imagens
distintas). Use UV onde o modelo tem coordenadas (re-exporte o modelo do jogador
com UVs usando sua textura-paleta; pikachu pela UV) e mapeamento triplanar em
espaço de mundo onde não há UV (ginásio, PokéStop, balão, chão de grama).
Acrescente opções de filtro (NEAREST) e wrap (REPEAT) no carregamento de textura."
→ Função `triplanar` no shader; re-export do `adventurer.obj` com UVs +
`adventurer_palette.png`; texturas procedurais (PIL) para os demais; 10 unidades
de textura. (`src/main.cpp`, `src/shader_fragment.glsl`, `data/*.png`)

**12. Cena e mecânica de captura**
Prompt: "Quando o jogador se aproxima, o Pokémon aparece; ao encostar nele (teste
de colisão) entra no modo captura. Na captura, renderize apenas o Pokémon com um
plano de fundo (billboard) usando a textura de floresta. Adicione a Pokébola
(somente na captura, bem menor que o Pokémon): segure `L` para carregar uma barra
de progresso cuja chance é max(25%, carga) (vazia 25%, metade 50%, cheia 100%); ao
soltar, lance a Pokébola seguindo uma curva de Bézier cúbica até o Pokémon. Sorteie
o resultado: se capturar, o Pokémon entra na Pokébola; se falhar, a Pokébola
reaparece para nova tentativa."
→ Máquina de estados `CaptureState { Aiming, Throwing, Caught }`; gating do mundo
na captura + fundo `CAPTURE_BG`; Pokébola (`pokebola.obj`) por UV; arremesso com
`EvalCubicBezier`; barra na HUD; `srand`/`rand`. (`src/main.cpp`,
`src/shader_fragment.glsl`, `data/pokebola_tex.png`)

**13. Armazenamento e múltiplos tipos de Pokémon**
Prompt: "Generalize o jogo para suportar mais de um tipo de Pokémon (Pikachu e
Charmander), incluindo no armazenamento. Crie uma tabela de tipos com malha,
object_id/textura, miniatura, e ajustes de escala/posição/rotação por tipo. Cada
Pokémon capturado guarda seu tipo e um número por tipo (Pikachu 1, Charmander 1,
...). A grade de miniaturas e a tela de detalhe (modelo 3D girando + HP/XP/ataques
aleatórios) usam os parâmetros do tipo correspondente."
→ Tabela `PokemonType g_PokeTypes[]`; campo `pokeType` em `SceneEntity` e
`type`/`number` em `CapturedPokemon`; detecção de captura genérica (`pokeType>=0`);
miniatura/detalhe por tipo. Modelo `charmander.obj` (normalizado) + texturas
`tex_charmander.png`/`charmander_icon.png`. (`src/main.cpp`,
`src/shader_fragment.glsl`, `data/charmander.obj`, `data/tex_charmander.png`,
`data/charmander_icon.png`)

**14. Itens consumíveis na captura (CP, gasto de Pokébola, fundo)**
Prompt: "Na cena de captura mostre o nome do Pokémon e o CP (sorteado ao iniciar e
reutilizado se capturar). A captura consome Pokébolas: cada lançamento gasta uma; se
o jogador estiver sem, bloqueie a mira e mostre um aviso para coletar nas PokéStops
(e não desenhe a Pokébola na mão). Use uma imagem de fundo própria na cena."
→ `g_CaptureCP`; HUD com nome+CP e contador de Pokébolas; gating do arremesso por
`g_NumPokeballs`; campo `cp` em `CapturedPokemon` (antes `xp`). Fundo `CAPTURE_BG`
trocado para `mapa-captura.png` (TextureImage15). (`src/main.cpp`,
`src/shader_fragment.glsl`, `data/mapa-captura.png`)

**15. Spawns aleatórios dos Pokémon**
Prompt: "A cada execução, posicione os Pokémon em lugares aleatórios (6 Pikachus e 6
Charmanders), sem nascer em cima do jogador nem um sobre o outro."
→ `randomSpot()` com `rand()` (semeado por `srand(time)`), rejeição por distância
mínima da origem e dos já posicionados (`placedSpots`). (`src/main.cpp`)

**16. Armazenamento: miniaturas em 3D, borda e fechar**
Prompt: "No armazenamento, as miniaturas devem ser o próprio modelo 3D do Pokémon
(como no mapa), cada um numa caixinha com borda. Fechar a janela com ESC ou clique
fora. Mostre só o nome do tipo (sem numeração)."
→ Render por célula com `glViewport` (mini-câmera por miniatura, girando) + moldura
`UI_BORDER`; fecha por clique-fora/ESC; rótulos = nome do tipo. (`src/main.cpp`,
`src/shader_fragment.glsl`)

**17. Times e ginásios (deixar Pokémon)**
Prompt: "Ao abrir o jogo, um modal pede para escolher um time (vermelho/azul/
amarelo). Os ginásios começam cinza (livres); chegando perto e clicando num livre,
abre um modal (clique ou Y/N) perguntando se quer deixar um Pokémon. Se sim, abre o
armazenamento para escolher e o ginásio fica com a cor do time."
→ `enum class Team`, `g_PlayerTeam`, modais 2D (escolha de time / deixar Pokémon),
picking de ginásio por clique gated por proximidade, modo "colocar" no armazenamento.
(`src/main.cpp`, `src/shader_fragment.glsl`)

**18. Modelos do Pokémon GO (ginásios e PokéStops)**
Prompt: "Use o pacote glTF em data/pokemon-go-assets para trocar os ginásios e as
PokéStops. Ginásios: uma variante por cor de time (+ cinza para livre), sempre o
modelo completo. PokéStops com dois eixos independentes: a forma muda pela
distância (longe = fechada, perto = aberta) e a cor pela interação (azul =
disponível, cinza = em cooldown de 30s após coletar). Coleta por clique, só de
perto e disponível."
→ glTF convertido com trimesh (`scene.dump()` aplica transforms): cores dos
materiais assadas como cor por vértice, normalizado (centrado, base no chão,
altura 1) → `graygym/redgym/blegym/yellowgym.obj` e `openstop/closedstop.obj`.
Ginásios via `GYM_MODEL` (usa `vertex_color`). PokéStop: malha pela proximidade
(`openstop`/`closedstop`) e cor por object_id (`POKESTOP` azul / `POKESTOP_COOLDOWN`
cinza, sólidos); coleta por clique (`g_StopClickCheck`) exige perto + disponível.
Fonte do pacote em FONTES.txt (Sketchfab). (`src/main.cpp`,
`src/shader_fragment.glsl`, `data/*gym.obj`, `data/*stop.obj`)

**19. Colisão em arquivo separado + estruturas sólidas**
Prompt: "Coloque os testes de colisão num arquivo à parte chamado collisions.cpp
(exigência da disciplina). Faça os ginásios e as PokéStops bloquearem o jogador (ele
não atravessa; a prioridade é o jogador), mantendo os Pokémon: ao encostar, entra na
captura e o corpo dele é exibido."
→ `collisions.h` (tipos `SceneObject`/`SceneEntity` + `extern` dos globais +
protótipos) e `collisions.cpp` (AABB `FindCollidingEntityIndex`/`CheckCollision` +
`CircleCollision`). Bloqueio por círculo contra `g_Gyms`/`g_PokeStops` no loop de
movimento; `collisions.cpp` no CMake. (`src/collisions.h`, `src/collisions.cpp`,
`src/main.cpp`, `CMakeLists.txt`)

**20. Textura em todos os objetos + logos dos times**
Prompt: "Garanta que TODOS os objetos tenham cor vinda de textura (ginásios e
pokéstops ficaram sem). Coloque os logos dos times (com transparência) na seleção de
time, mostrando o nome do time; reduza as imagens grandes."
→ Ginásios/pokéstops passam a `triplanar(textura) * cor` (reaproveita
`tex_stone`/`tex_metal`). `LoadTextureImage` com flag `alpha` (RGBA); object_ids
`UI_LOGO_*` com `discard` do fundo transparente; logos `valor/mystic/instinct.png`
reduzidos a 512×512 (PIL). Rótulos dos botões = nome do time. (`src/main.cpp`,
`src/shader_fragment.glsl`, `data/valor.png`, `data/mystic.png`, `data/instinct.png`)

**21. Aparição instantânea do Pokémon**
Prompt: "O Pokémon não deve surgir crescendo gradualmente — aparece inteiro de uma
vez ao entrar no raio e some ao sair."
→ Removido o fade de escala por distância no loop de desenho (mantida a animação de
encolher na captura). (`src/main.cpp`)

**22. Borda de floresta + câmera sem vazios**
Prompt: "As placas de floresta da borda estão com orientações diferentes (umas de
lado, uma de cabeça pra baixo) — deixe todas em pé com o chão embaixo. E a
movimentação da câmera não pode deixar aparecer os vazios atrás das placas."
→ 4 placas `FOREST_WALL` unificadas (`Rotate_Y(yaw) * Rotate_X(-90°)`, mesma escala)
+ V invertido no shader (em pé); culling ligado (placa some atrás da câmera). Câmera
3ª pessoa travada dentro do cercado (`±5.2` em XZ, `y∈[0.3,7.0]`) e clear color de
céu. (`src/main.cpp`, `src/shader_fragment.glsl`)

**23. Animações que faltavam na SPEC (pokéstop, oscilação, luz da captura)**
Prompt: "Reveja a SPEC e implemente o que está prometido mas faltando sem precisar
de modelo novo: as PokéStops devem girar, os Pokémon devem oscilar de leve no mapa
(sem afundar no chão) e a cena de captura deve variar a iluminação (período do dia)."
→ PokéStop disponível gira em Y (`time*1.2`); Pokémon flutuam só pra cima
(`(sin(time*2+i)*0.5+0.5)*0.06`); uniform `light_tint` no shader multiplica `I`/`Ia`
(neutro no mundo, varia quente↔frio na captura). Tudo por `glfwGetTime` (Δt).
(`src/main.cpp`, `src/shader_fragment.glsl`)

**24. Itens das PokéStops (aleatórios + voando por Bézier)**
Prompt: "A PokéStop deve dar de 1 a 7 itens aleatórios entre Pokébola, Poção e
Fruta (pelo menos 1 Pokébola). Cada item ganho voa do poste até o jogador por uma
curva de Bézier cúbica, com o modelo certo de cada item. Pegue os modelos de poção e
fruta no pacote pokemon-go-assets."
→ Sorteio 1..7 (1º sempre Pokébola); inventário `g_NumBerries`; `FlyingItem`
(p0..p3, `type`) animado por `EvalCubicBezier` com modelo por tipo. Modelos
`potion.obj`/`berry.obj` convertidos do glTF (cor por vértice). (`src/main.cpp`,
`data/potion.obj`, `data/berry.obj`)

**25. Pokémon no ginásio: ver, não duplicar, dar fruta**
Prompt: "O Pokémon deixado no ginásio deve ficar visível (flutuando acima dele). Um
Pokémon que já está num ginásio não pode ser colocado em outro. E clicando num
ginásio ocupado deve aparecer a opção de dar uma fruta (checando se tenho) — a fruta
voa de mim até o Pokémon por Bézier cúbica."
→ Defensor renderizado flutuando/girando sobre o ginásio; `CapturedPokemon.placedGym`
bloqueia recolocação (rótulo "(gym)" na grade); modal do ginásio ramifica
(livre→colocar / ocupado→dar fruta), fruta voa do jogador ao Pokémon via
`EvalCubicBezier`. (`src/main.cpp`)

**26. Captura por clique (não por encostar)**
Prompt: "Para ir pra captura tem que ser por clique no Pokémon, não por encostar.
Os Pokémon não bloqueiam mais o jogador (ele atravessa o corpo, que segue exibido);
só ginásio e pokéstop bloqueiam."
→ Removido o gatilho por colisão; movimento só checa estruturas; picking em tela
`g_PokeClickCheck` no Pokémon desenhado → entra na captura. (`src/main.cpp`)

**27. Captura por interseção + ataques por tipo + Snorlax**
Prompt: "Decida a captura por interseção Pokébola↔Pokémon (a carga vira mira), não
por sorteio; a Pokébola cai no chão e some quando erra. Os ataques do Charmander têm
que ser diferentes do Pikachu. Adicione o Snorlax como 3º tipo."
→ Interseção **esfera-esfera** (`SphereCollision` em `collisions.cpp`, 3º tipo de
colisão); estado `Missed` (bola cai e encolhe); `g_AttacksByType[3][8]` por tipo;
Snorlax (`snorlax.obj` + `tex_snorlax.png`, object_id `SNORLAX`). (`src/main.cpp`,
`src/collisions.h`, `src/collisions.cpp`, `src/shader_fragment.glsl`,
`data/snorlax.obj`, `data/tex_snorlax.png`)

**28. Evolução (doces por espécie + animação)**
Prompt: "Implemente evolução estilo Pokémon GO: doces por espécie (cada captura dá
doces), botão Evoluir na tela de detalhe quando houver doces suficientes (custo 3),
e uma animação em que o Pokémon gira e encolhe, troca de forma com um flash, e a
forma evoluída surge crescendo. Pikachu→Raichu e Charmander→Charmeleon."
→ `g_Candies[]`/`g_CandyFamily[]`/`g_EvolvesTo[]`/`g_EvolveCost[]`; botão no detalhe;
animação de escala (encolhe→troca+flash→cresce) com `g_EvolvingIndex`/`g_EvolveTimer`.
(`src/main.cpp`)

**29. Modelos HD reais (modelo duplo) + conversão glTF→atlas+UV**
Prompt: "No mapa e na miniatura mantenha o modelo simples atual; nos outros lugares
de exibição (captura/detalhe/ginásio) use modelos melhores que vou subir em data.
Converta os .glb (FBX/USDZ não dá; GLB/glTF sim) igual ao que foi feito antes, com
textura real."
→ Campos de exibição (`dispMesh/dispObjId/dispDetail*/dispCapture*/dispFacing`) em
`PokemonType` (modelo duplo base/HD). Pipeline trimesh: une as texturas embutidas num
**atlas** + **remapeia UVs** (encaixe no tile por shift+clamp), carrega com **NEAREST**
(evita borrar folhas de expressão) e **alpha+discard** onde há transparência; cores
chapadas viram swatches. Charmander/Pikachu/Snorlax HD. (`src/main.cpp`,
`src/shader_fragment.glsl`, `data/*_hd.obj`, `data/tex_*_hd.png`)

**30. Evoluções com modelo real (Charmeleon, Raichu) + correções de textura**
Prompt: "Faça o Charmeleon (evolução do Charmander) e o Raichu (evolução do Pikachu)
a partir dos .glb. A textura na cara do Raichu está errada e tem placas pretas nas
bochechas — conserte."
→ `charmeleon.glb`/`raichu.glb` convertidos (atlas+UV). Rosto "errado" = mipmap
borrando as folhas de expressão → **NEAREST**; "placas pretas" = bochechas RGBA
transparentes → atlas RGBA + `discard` no shader. Forma única (sem base/HD),
object_ids `CHARMELEON`/`RAICHU`. (`src/main.cpp`, `src/shader_fragment.glsl`,
`data/charmeleon.*`, `data/raichu.*`)

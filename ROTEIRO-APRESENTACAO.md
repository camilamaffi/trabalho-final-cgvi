# Roteiro de apresentação — "Pokémon GO simplificado" (INF01047)

**Tempo alvo: ~8–10 min** · Dupla: Camila + Gilmar

> Guia para a demonstração ao vivo + pontos técnicos mapeados nos requisitos da
> disciplina. Onde estiver ⚠️, **confira antes** de apresentar.

---

## 0. Abertura (30s)
- "Fizemos um Pokémon GO simplificado em C++/OpenGL, sem usar `gluLookAt`,
  `glm::perspective`, `rotate/scale/translate` — todas as matrizes são nossas
  (`matrices.h`)."
- Tema em 1 frase: explorar o mapa, achar Pokémon, capturar, evoluir, dominar ginásios.

## 1. Movimentação + câmeras (1–2 min) → *câmeras + transformações*
- Escolha um time no **modal de abertura**.
- Ande com **WASD** — o avatar **vira na direção do movimento** (yaw por `atan2`).
- Gire/zoom com o mouse → **câmera look-at em 3ª pessoa** seguindo um ponto móvel.
- Aperte **C** → **câmera livre** (voa com WASD + E/Q).
- **Fala técnica:** "São os dois tipos de câmera exigidos: look-at que segue o
  jogador e câmera livre controlada pelo usuário."

## 2. PokéStops + itens (1 min) → *Bézier + animação por tempo + interação*
- Chegue numa **PokéStop**: ela **abre** por proximidade e **gira** (animação por tempo).
- **Clique** pra coletar → os **itens voam até você por curva de Bézier cúbica**.
- **Fala técnica:** "A trajetória dos itens é uma Bézier cúbica `EvalCubicBezier`;
  o balão da Equipe Rocket no céu também (mostre o balão)."

## 3. Captura (2 min) → *intersecção + textura + iluminação + estado*
- **Clique num Pokémon** pra entrar na captura.
- Segure **L** (mira carrega) e solte pra lançar a Pokébola.
- **Fala técnica (importante p/ banca):**
  - "O acerto é decidido por **interseção esfera-esfera** Pokébola↔Pokémon — um dos
    **3 testes de intersecção**, todos em `collisions.cpp`: **AABB**
    (jogador×Pokémon), **círculo-círculo** (jogador×ginásio/pokéstop) e
    **esfera-esfera** (captura)."
  - "No erro a bola **cai e some**; a iluminação da cena **varia com o tempo**
    (período do dia)."

## 4. Armazenamento + Evolução (2 min) → *animação + malhas + interface*
- Abra a **mochila** (ícone) → grade de miniaturas em **3D girando** (cada uma um
  `glViewport`).
- Abra um Pokémon → tela de **detalhe** (modelo HD girando, CP/HP/ataques).
- **Evolua** (botão Evoluir, custa 3 doces): mostre a animação — **gira e encolhe →
  flash → forma nova surge crescendo**.
- **Fala técnica:** "Modelos poligonais complexos de terceiros (glTF), convertidos
  por nós pra OBJ com **textura real via atlas + UV**. Usamos **modelo duplo**: um
  simples no mapa/miniatura e um HD nas telas grandes."

## 5. Ginásios (1 min) → *instâncias + interação*
- Vá num **ginásio** (cinza = livre), **clique** → deixe um Pokémon → ele fica com a
  **cor do time** e o Pokémon **flutua** girando em cima.
- Clique num ginásio ocupado → **dar fruta** (a fruta voa por Bézier).
- **Fala técnica:** "Os ginásios e PokéStops são **instâncias** da mesma malha com
  Model matrix diferentes."

## 6. Fechamento técnico (1 min) — checklist dos requisitos
- ✅ **Malhas complexas** (Pokémon HD, ginásios, jogador)
- ✅ **Transformações geométricas** controladas pelo usuário
- ✅ **2 câmeras** (look-at + livre)
- ✅ **Instâncias** (ginásios, pokéstops, Pokémon)
- ✅ **3 testes de intersecção** (AABB, círculo, esfera) em `collisions.cpp`
- ✅ **Iluminação difusa + Blinn-Phong** por fragmento
- ✅ **Texturas** (UV + triplanar; vários objetos)
- ✅ **Curva de Bézier cúbica** (itens, fruta, balão)
- ✅ **Animação por tempo** (giro, oscilação, evolução, luz)
- ⚠️ **Gouraud vs Phong** — *confirme*: temos Phong shading; se NÃO houver um objeto
  com Gouraud, ou implementem antes, ou não citem como feito.

---

## Verificar ANTES de apresentar
1. **Gouraud vs Phong** — a banca costuma cobrar os dois modelos de *interpolação*.
   Hoje o jogo é Phong (por fragmento). Confirmar se precisa de um objeto com Gouraud
   (por vértice).
2. **Atribuição dos modelos de terceiros** (os `.glb` HD/evoluções) — ter a
   fonte/licença à mão, caso perguntem.

## Divisão de trabalho (preencher)
- Camila: ...
- Gilmar: ...

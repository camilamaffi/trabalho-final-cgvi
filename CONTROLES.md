# Controles / FAQ do jogo

Guia de comandos do "Pokémon GO simplificado".

## Início do jogo

1. **Menu principal**: aparece o título e três opções — **Iniciar Jogo**,
   **Como Jogar** e **Sair**. Navegue com **W/S** ou as **setas** e confirme com
   **Enter** ou **clique**. ("Como Jogar" mostra uma tela de ajuda.)
2. **Escolha de time**: ao iniciar, abre o modal **"Escolha seu time!"** com os três
   emblemas — **Valor**, **Mystic** e **Instinct**. **Clique** num deles. Enquanto o
   modal está aberto o jogo fica travado; depois de escolher, ele fecha e o jogo começa.

## Movimentação (câmera em 3ª pessoa)

| Tecla | Ação |
|---|---|
| `W` `A` `S` `D` (ou setas) | Move o personagem pelo mapa (relativo à câmera) |
| Mouse — arrastar com botão esquerdo | Gira a câmera ao redor do personagem |
| Mouse — clique (sem arrastar) | Interage: captura Pokémon / coleta PokéStop / abre ginásio / balão |
| Mouse — rodinha (scroll) | Zoom (aproxima/afasta a câmera) |

## Câmera

| Tecla | Ação |
|---|---|
| `C` | Alterna entre **3ª pessoa** (segue o personagem) e **câmera livre** |
| `P` | Projeção **perspectiva** |
| `O` | Projeção **ortográfica** |

### No modo câmera livre (após apertar `C`)
| Tecla | Ação |
|---|---|
| `W` `A` `S` `D` | Move a câmera na horizontal (não afunda no mapa) |
| `E` | Sobe a câmera |
| `Q` | Desce a câmera |
| Mouse — botão esquerdo | Gira a direção do olhar |

A câmera livre é limitada: não afunda abaixo do mapa, não sobe acima das árvores
de borda e não passa dos contornos do mapa (paredes de floresta).

A HUD mostra o modo atual e o inventário no canto superior esquerdo.

## Armazenamento de Pokémons

- **Ícone no canto inferior esquerdo**: clique para abrir/fechar a janela de
  armazenamento. Começa vazia; cada Pokémon capturado vira uma **miniatura** lá
  (o próprio modelo 3D girando, dentro de uma caixinha com borda) com o nome embaixo.
- Clique numa miniatura para ver os **detalhes** do Pokémon (modelo girando,
  nome, CP, HP e ataques). Clique em qualquer lugar dentro do painel para voltar.
- Para **fechar** a janela: aperte **ESC** ou clique **fora** do painel.
- Com a janela aberta, use a **rodinha do mouse** para rolar (se houver muitos).
- Um Pokémon deixado num ginásio aparece com o rótulo **"(gin.)"** e não pode ser
  colocado em outro ginásio nem evoluído enquanto estiver lá.

### Evolução (doces por espécie, estilo Pokémon GO)

- Cada **captura** rende **3 doces** da espécie (família) do Pokémon capturado.
- Na tela de **detalhe**, os Pokémon que evoluem mostram quantos doces você tem e
  o **custo** (12 doces, ou seja, ~4 capturas), com um botão **EVOLUIR**.
- Com doces suficientes, clique no botão: o Pokémon **gira e encolhe** até sumir,
  dá um **flash de luz**, e a **forma evoluída surge crescendo** — ficando mais
  forte (ganha CP e HP). Evoluir **gasta** os doces.
- Um Pokémon que está **defendendo um ginásio** não pode evoluir (recolha-o antes,
  o botão fica "No ginasio"). Formas finais não mostram o botão.
- Evoluções atuais: **Pikachu → Raichu**, **Charmander → Charmeleon**
  (Snorlax não evolui).

## Interações no mapa

- **PokéStops**: a **forma** depende da distância (longe = **fechada**, perto =
  **aberta**) e a **cor** depende da interação (**azul** = disponível, **cinza** =
  em cooldown). Chegue **perto** (fica aberta) e **clique** para coletar: ela dá de
  **1 a 7 itens aleatórios** entre **Pokébola, Poção e Fruta** (pelo menos 1 é
  sempre Pokébola), que **voam até você** numa curva. Ao coletar, fica **cinza por
  30 segundos** e depois volta ao azul. Estoque máximo: **100** de cada item.
- **Pokémon (Pikachu, Charmander e Snorlax)**: há três tipos espalhados pelo
  mapa. Aparecem quando você chega perto e somem ao se afastar. Você **atravessa**
  o corpo deles ao andar (eles não bloqueiam); para capturar, **clique** no
  Pokémon — isso abre a **cena de captura**. Ao capturar um, outro do mesmo tipo
  **reaparece** em outro lugar depois de alguns segundos.
- **Ginásios**: começam **cinza** (livres). Chegue **perto** e **clique** no
  ginásio (clicar de longe não faz nada):
  - **Livre** → abre o modal "deixar um Pokémon?": responda com **clique**
    (Sim/Não) ou pelas teclas **Y** (sim) / **N** (não). Se aceitar, o
    **armazenamento abre** para você escolher; ao escolher, o ginásio fica com a
    **cor do seu time** e o Pokémon aparece **flutuando** acima dele.
  - **Ocupado** → abre o modal "dar uma fruta?": se você tiver frutas, ela **voa
    de você até o Pokémon** do ginásio.
- **Balão da Equipe Rocket**: voa sozinho pelo céu numa trajetória curva.
  **Clique nele** (botão esquerdo) para uma mensagem: "Giovanni nao esta aqui!".

## Cena de captura

Mostra só o Pokémon com um fundo de floresta.

| Tecla | Ação |
|---|---|
| `L` (segurar) | Carrega a barra de **mira** — quanto mais cheia, mais **certeira** (a Pokébola cai mais perto do centro do Pokémon) |
| `L` (soltar) | Lança a Pokébola numa trajetória curva até o Pokémon |
| `ESC` ou `ENTER` | Sair da cena de captura e voltar ao mapa |

O acerto é decidido por **interseção**: se a Pokébola encostar no corpo do Pokémon,
captura; se errar, a **bola cai no chão e some**, e você tenta de novo (gastando
outra Pokébola).

**Você precisa ter Pokébolas para capturar.** Cada lançamento gasta **1 Pokébola**
(o contador no canto vai diminuindo). Se ficar **sem Pokébolas**, aparece o aviso
para explorar o mapa e coletar mais nas **PokéStops**.

## Outros

| Tecla | Ação |
|---|---|
| `ESPAÇO` | Reseta ângulos e a câmera para a posição inicial |
| `H` | Mostra/esconde o texto informativo na tela |
| `R` | Recarrega os shaders (debug) |
| `ESC` | Fecha o jogo (no mapa) |

### Extras herdados do código base (debug)
| Comando | Ação |
|---|---|
| `X` / `Y` / `Z` (com `Shift` inverte) | Giram ângulos de Euler de teste |
| Mouse — botão direito | Ajusta ângulo do "antebraço" (exemplo do código base) |
| Mouse — botão do meio | Ajusta posição do "torso" (exemplo do código base) |

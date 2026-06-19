# Controles / FAQ do jogo

Guia de comandos do "Pokémon GO simplificado".

## Movimentação (câmera em 3ª pessoa)

| Tecla | Ação |
|---|---|
| `W` `A` `S` `D` (ou setas) | Move o personagem pelo mapa (relativo à câmera) |
| Mouse — arrastar com botão esquerdo | Gira a câmera ao redor do personagem |
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

A HUD mostra o modo atual no canto superior esquerdo.

## Armazenamento de Pokémons

- **Ícone no canto superior esquerdo**: clique para abrir/fechar a janela de
  armazenamento. Começa vazia; cada Pokémon capturado vira uma **miniatura** lá.
- Com a janela aberta, use a **rodinha do mouse** para rolar (se houver muitos).

## Interações no mapa

- **PokéStops** (poste azul com disco/anel girando): chegue perto para coletar
  itens automaticamente (**+2 Pokébolas, +1 Poção**). Depois fica em **cooldown
  de 1 minuto** (cinza). Estoque máximo: **100** de cada item.
- **Pokémon (pikachu)**: aparecem quando você chega perto e somem ao se afastar.
  Encostar em um pikachu inicia a **cena de captura**.
- **Ginásios**: estruturas (sem batalha por enquanto).
- **Balão da Equipe Rocket**: voa sozinho pelo céu numa trajetória curva.
  **Clique nele** (botão esquerdo) para uma mensagem: "Giovanni nao esta aqui!".

## Cena de captura

Mostra só o Pokémon com um fundo de floresta.

| Tecla | Ação |
|---|---|
| `L` (segurar) | Carrega a barra de captura — quanto mais cheia, maior a chance (mín. 25%, metade 50%, cheia 100%) |
| `L` (soltar) | Lança a Pokébola numa trajetória curva até o Pokémon |
| `ESC` ou `ENTER` | Sair da cena de captura e voltar ao mapa |

Se a captura falhar, a Pokébola reaparece para tentar de novo. Se acertar, o
Pokémon entra na Pokébola e é capturado.

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

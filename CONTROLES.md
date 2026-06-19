# Controles / FAQ do jogo

Guia de comandos do "Pokémon GO simplificado".

## Início do jogo

Ao abrir, aparece um **modal de escolha de time**: clique em **Vermelho**, **Azul**
ou **Amarelo** (Valor / Mystic / Instinct). Enquanto o modal está aberto o jogo
fica travado; depois de escolher, ele fecha e o jogo começa.

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

- **Ícone no canto inferior esquerdo**: clique para abrir/fechar a janela de
  armazenamento. Começa vazia; cada Pokémon capturado vira uma **miniatura** lá
  (o próprio modelo 3D girando, dentro de uma caixinha com borda).
- Clique numa miniatura para ver os **detalhes** do Pokémon (modelo girando,
  nome, CP, HP e ataques). Clique em qualquer lugar dentro do painel para voltar.
- Para **fechar** a janela: aperte **ESC** ou clique **fora** do painel.
- Com a janela aberta, use a **rodinha do mouse** para rolar (se houver muitos).

## Interações no mapa

- **PokéStops**: a **forma** depende da distância (longe = **fechada**, perto =
  **aberta**) e a **cor** depende da interação (**azul** = disponível, **cinza** =
  em cooldown). Chegue **perto** (fica aberta) e **clique** para coletar itens
  (**+2 Pokébolas, +1 Poção**); ao coletar ela fica **cinza por 30 segundos** e
  depois volta ao azul. Estoque máximo: **100** de cada item.
- **Pokémon (Pikachu e Charmander)**: há dois tipos espalhados pelo mapa.
  Aparecem quando você chega perto e somem ao se afastar. Encostar em um deles
  inicia a **cena de captura**. No armazenamento cada um aparece com o nome do
  seu tipo (Pikachu / Charmander).
- **Ginásios**: começam **cinza** (livres). Chegue **perto** e **clique** no
  ginásio para abrir um modal perguntando se quer deixar um Pokémon (clicar de
  longe não faz nada): responda com **clique**
  (Sim/Não) ou pelas teclas **Y** (sim) / **N** (não). Se aceitar, o
  **armazenamento abre** para você escolher qual Pokémon deixar; ao escolher, o
  ginásio fica com a **cor do seu time**. (ESC ou "Não" recusa.)
- **Balão da Equipe Rocket**: voa sozinho pelo céu numa trajetória curva.
  **Clique nele** (botão esquerdo) para uma mensagem: "Giovanni nao esta aqui!".

## Cena de captura

Mostra só o Pokémon com um fundo de floresta.

| Tecla | Ação |
|---|---|
| `L` (segurar) | Carrega a barra de captura — quanto mais cheia, maior a chance (mín. 25%, metade 50%, cheia 100%) |
| `L` (soltar) | Lança a Pokébola numa trajetória curva até o Pokémon |
| `ESC` ou `ENTER` | Sair da cena de captura e voltar ao mapa |

**Você precisa ter Pokébolas para capturar.** Cada lançamento gasta **1 Pokébola**
(o contador no canto aparece diminuindo). Se ficar **sem Pokébolas**, aparece o
aviso para explorar o mapa e coletar mais nas **PokéStops** (+2 por visita).

Se a captura falhar, a Pokébola reaparece para tentar de novo (gastando outra). Se
acertar, o Pokémon entra na Pokébola e é capturado.

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

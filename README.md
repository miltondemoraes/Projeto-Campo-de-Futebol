# Campo de Futebol - Computação Gráfica

## Descrição
Implementação em **C++ com OpenGL** de um campo de futebol com todos os elementos geométricos padrão (retas e circunferências).

## Estrutura

O código segue padrões profissionais de programação:

- **Classe `Point2D`**: Representação de pontos 2D
- **Classe `SoccerField`**: Encapsulamento de toda a lógica de desenho do campo
  - Métodos privados para desenho de primitivas: `drawLine()`, `drawCircle()`, `drawArc()`
  - Métodos privados para cada elemento: `drawFieldBorder()`, `drawCenterLine()`, `drawPenaltyArea()`, etc.
  - Método público `draw()` que coordena todo o desenho

## Elementos do Campo

- Contorno principal (retângulo)
- Linha do meio
- Círculo central
- Ponto central
- Áreas de penalti (2)
- Áreas de meta/baliza (2)
- Pontos de penalti (2)
- Arcos de canto (4)
- Arcos das áreas de penalti (2)

**Dimensões conforme padrão oficial FIFA:**
- Largura: 105m
- Altura: 68m
- Raio do círculo central: 9.15m
- Raio do arco de penalti: 9.15m

## Compilação

### Com CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Executar:
```bash
soccer_field
```

## Controles
- **ESC**: Fechar aplicação
- A janela é responsiva e mantém proporções corretas

## Requisitos
- OpenGL
- FreeGLUT
- Compilador C++11 ou superior

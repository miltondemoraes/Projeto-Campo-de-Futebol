#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>

#include "audio.h"

#define PI 3.14159265358979323846f

/* Estrutura para representar um jogador dinâmico */
typedef struct {
    float baseX, baseY;    /* Posição base (inicial) */
    float x, y;            /* Posição atual dinâmica */
    int team;              /* 0 = esquerda, 1 = direita */
    int position;          /* 0=goleiro, 1-4=defesa, 5-8=meio, 9-10=ataque */
} Player;

/* Estrutura para o jogador especial do 3x3 */
typedef struct {
    float x, y;
    bool active;
} SpecialPlayer;

static SpecialPlayer specialPlayer = { -70.0f, 0.0f, false };
static bool ballMovementLocked = false;

static Player players[22];  /* 11 por time */

static float ballX = 0.0f;
static float ballY = 0.0f;
static const float ballR = 1.2f;
static const float ballStep = 1.2f;
static int scoreLeft = 0;
static int scoreRight = 0;
static float crowdPhase = 0.0f;
static int g_winW = 1200;
static int g_winH = 780;

typedef struct {
    int x, y, w, h;
    bool hovered;
    const char* label;
} Button;

static Button invaderButton = { 0, 0, 150, 40, false, "Activate Invader" };

static void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (x >= invaderButton.x && x <= invaderButton.x + invaderButton.w &&
            y >= invaderButton.y && y <= invaderButton.y + invaderButton.h) {
            
            if (!specialPlayer.active) {
                specialPlayer.active = true;
                specialPlayer.x = -70.0f;
                specialPlayer.y = 0.0f;
                ballMovementLocked = true;
            }
        }
    }
}

static void passiveMotion(int x, int y) {
    invaderButton.hovered = (x >= invaderButton.x && x <= invaderButton.x + invaderButton.w &&
                             y >= invaderButton.y && y <= invaderButton.y + invaderButton.h);
    glutPostRedisplay();
}

/* Inicializa os jogadores com suas posições base */
static void initializePlayers(void) {
    int idx = 0;
    
    /* Time da esquerda (ataca para a direita) */
    const float L[][2] = {
        { -46.5f, 0.0f },   /* 0: goleiro */
        { -38.0f, -16.0f }, { -38.0f, -5.5f }, { -38.0f, 5.5f }, { -38.0f, 16.0f }, /* 1-4: defesa */
        { -35.0f, -14.0f }, { -35.0f, -4.0f }, { -35.0f, 4.0f }, { -35.0f, 14.0f }, /* 5-8: meio */
        { -14.0f, -8.0f }, { -14.0f, 8.0f }  /* 9-10: ataque */
    };
    
    for (int i = 0; i < 11; i++) {
        players[idx].baseX = L[i][0];
        players[idx].baseY = L[i][1];
        players[idx].x = L[i][0];
        players[idx].y = L[i][1];
        players[idx].team = 0;
        players[idx].position = i;
        idx++;
    }
    
    /* Time da direita (ataca para a esquerda) */
    const float R[][2] = {
        { 46.5f, 0.0f },
        { 38.0f, -16.0f }, { 38.0f, -5.5f }, { 38.0f, 5.5f }, { 38.0f, 16.0f },
        { 35.0f, -30.0f }, { 35.0f, -15.0f }, { 35.0f, 15.0f }, { 35.0f, 30.0f },
        { 14.0f, -8.0f }, { 14.0f, 8.0f }
    };
    
    for (int i = 0; i < 11; i++) {
        players[idx].baseX = R[i][0];
        players[idx].baseY = R[i][1];
        players[idx].x = R[i][0];
        players[idx].y = R[i][1];
        players[idx].team = 1;
        players[idx].position = i;
        idx++;
    }
}

static float clampf(float value, float minV, float maxV) {
    if (value < minV) return minV;
    if (value > maxV) return maxV;
    return value;
}

/* Atualiza as posições dos jogadores baseado na bola */
static void updatePlayerPositions(void) {
    const float playerSpeed = 0.08f;
    const float fieldHalfH = 34.0f;
    const float smallAreaHalfH = 5.5f;
    static float gameTime = 0.0f;
    gameTime += 0.016f;
    
    for (int i = 0; i < 22; i++) {
        Player *p = &players[i];
        float dx = ballX - p->baseX;
        float dy = ballY - p->baseY;
        float dist = sqrtf(dx * dx + dy * dy);
        float targetX, targetY;
        
        if (p->team == 0) {
            /* Time esquerda (ataca para direita) */
            if (p->position == 0) {
                /* Goleiro se move só em Y, fica em X = -47 */
                p->x = -47.0f;
                p->y = clampf(ballY, -smallAreaHalfH, smallAreaHalfH);
            }
            else if (p->position >= 1 && p->position <= 4) {
                /* DEFESA: segue a bola mas fica recuado */
                targetX = p->baseX + (ballX - 0.0f) * 0.15f;
                targetY = p->baseY + (ballY - p->baseY) * 0.5f + sinf(gameTime * 1.5f + p->position) * 2.0f;
                
                targetX = clampf(targetX, -52.5f, 5.0f);
                targetY = clampf(targetY, -fieldHalfH, fieldHalfH);
                
                p->x += (targetX - p->x) * playerSpeed * 1.2f;
                p->y += (targetY - p->y) * playerSpeed * 1.2f;
            }
            else if (p->position >= 5 && p->position <= 8) {
                /* MEIO-CAMPO: avança quando bola está no meio ou ataque */
                if (ballX > -5.0f) {
                    /* Bola saiu do fundo: avança MAIS */
                    targetX = p->baseX + 18.0f;
                } else {
                    /* Bola em defesa: fica mais recuado */
                    targetX = p->baseX + 5.0f;
                }
                
                targetY = p->baseY + (ballY - p->baseY) * 0.8f + cosf(gameTime * 2.2f + p->position) * 7.0f;
                targetX = clampf(targetX, -40.0f, 38.0f);
                targetY = clampf(targetY, -fieldHalfH, fieldHalfH);
                
                p->x += (targetX - p->x) * playerSpeed * 1.6f;
                p->y += (targetY - p->y) * playerSpeed * 1.6f;
            }
            else if (p->position >= 9 && p->position <= 10) {
                /* ATACANTES: AVANÇAM MUITO quando bola está em zona de ataque */
                if (ballX > 5.0f) {
                    /* Bola está no ataque: AVANÇA MUITO para dentro da área */
                    targetX = p->baseX + 30.0f;  /* Avanço agressivo! */
                    targetY = p->baseY + (ballY - p->baseY) * 0.7f + sinf(gameTime * 3.5f) * 8.0f;
                } else if (ballX > -15.0f) {
                    /* Bola no meio: avança moderadamente */
                    targetX = p->baseX + 18.0f;
                    targetY = p->baseY + (ballY - p->baseY) * 0.7f + cosf(gameTime * 2.5f) * 5.0f;
                } else {
                    /* Bola em defesa: volta para base */
                    targetX = p->baseX + 5.0f;
                    targetY = p->baseY + sinf(gameTime * 2.0f) * 4.0f;
                }
                
                targetX = clampf(targetX, -18.0f, 52.0f);
                targetY = clampf(targetY, -fieldHalfH, fieldHalfH);
                
                p->x += (targetX - p->x) * playerSpeed * 2.0f;  /* Mais rápido! */
                p->y += (targetY - p->y) * playerSpeed * 2.0f;
            }
        }
        else {
            /* Time direita (ataca para esquerda) */
            if (p->position == 0) {
                /* Goleiro */
                p->x = 47.0f;
                p->y = clampf(ballY, -smallAreaHalfH, smallAreaHalfH);
            }
            else if (p->position >= 1 && p->position <= 4) {
                /* DEFESA */
                targetX = p->baseX + (ballX - 0.0f) * 0.15f;
                targetY = p->baseY + (ballY - p->baseY) * 0.5f + sinf(gameTime * 1.5f + p->position) * 2.0f;
                
                targetX = clampf(targetX, -5.0f, 52.5f);
                targetY = clampf(targetY, -fieldHalfH, fieldHalfH);
                
                p->x += (targetX - p->x) * playerSpeed * 1.2f;
                p->y += (targetY - p->y) * playerSpeed * 1.2f;
            }
            else if (p->position >= 5 && p->position <= 8) {
                /* MEIO-CAMPO */
                if (ballX < 5.0f) {
                    /* Bola saiu do fundo: avança MAIS */
                    targetX = p->baseX - 18.0f;
                } else {
                    /* Bola em defesa: fica mais recuado */
                    targetX = p->baseX - 5.0f;
                }
                
                targetY = p->baseY + (ballY - p->baseY) * 0.8f + cosf(gameTime * 2.2f + p->position) * 7.0f;
                targetX = clampf(targetX, -38.0f, 40.0f);
                targetY = clampf(targetY, -fieldHalfH, fieldHalfH);
                
                p->x += (targetX - p->x) * playerSpeed * 1.6f;
                p->y += (targetY - p->y) * playerSpeed * 1.6f;
            }
            else if (p->position >= 9 && p->position <= 10) {
                /* ATACANTES */
                if (ballX < -5.0f) {
                    /* Bola está no ataque: AVANÇA MUITO */
                    targetX = p->baseX - 30.0f;  /* Avanço agressivo! */
                    targetY = p->baseY + (ballY - p->baseY) * 0.7f + sinf(gameTime * 3.5f) * 8.0f;
                } else if (ballX < 15.0f) {
                    /* Bola no meio: avança moderadamente */
                    targetX = p->baseX - 18.0f;
                    targetY = p->baseY + (ballY - p->baseY) * 0.7f + cosf(gameTime * 2.5f) * 5.0f;
                } else {
                    /* Bola em defesa: volta para base */
                    targetX = p->baseX - 5.0f;
                    targetY = p->baseY + sinf(gameTime * 2.0f) * 4.0f;
                }
                
                targetX = clampf(targetX, -52.0f, 18.0f);
                targetY = clampf(targetY, -fieldHalfH, fieldHalfH);
                
                p->x += (targetX - p->x) * playerSpeed * 2.0f;  /* Mais rápido! */
                p->y += (targetY - p->y) * playerSpeed * 2.0f;
            }
        }
    }
}

static void drawFilledRect(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void drawGradientRect(float x1, float y1, float x2, float y2,
                             float rTop, float gTop, float bTop,
                             float rBot, float gBot, float bBot) {
    glBegin(GL_QUADS);
    glColor3f(rBot, gBot, bBot);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glColor3f(rTop, gTop, bTop);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void drawRectOutline(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void drawCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float a = 2.0f * PI * (float)i / (float)segments;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawFilledCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float a = 2.0f * PI * (float)i / (float)segments;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawArc(float cx, float cy, float r, float a0, float a1, int segments) {
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float a = a0 + t * (a1 - a0);
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    while (*text != '\0') {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

static void drawTextPixels(int x, int y, const char *text) {
    glRasterPos2i(x, y);
    while (*text != '\0') {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

/* team: 0 = esquerda (vermelho), 1 = direita (azul). (x,y) = posição dos pés. */
static void drawPlayer(float x, float y, int team) {
    float jr, jg, jb;
    if (team == 0) {
        jr = 0.88f;
        jg = 0.16f;
        jb = 0.18f;
    } else {
        jr = 0.14f;
        jg = 0.32f;
        jb = 0.92f;
    }

    glColor3f(0.05f, 0.09f, 0.06f);
    drawFilledCircle(x + 0.12f, y - 0.18f, 1.05f, 22);

    glColor3f(jr, jg, jb);
    drawFilledRect(x - 0.75f, y + 0.4f, x + 0.75f, y + 2.45f);

    glLineWidth(2.4f);
    glColor3f(0.12f, 0.12f, 0.14f);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x, y + 2.5f);
    glVertex2f(x, y + 1.1f);
    glVertex2f(x - 1.15f, y + 0.15f);
    glVertex2f(x, y + 1.1f);
    glVertex2f(x + 1.15f, y + 0.15f);
    glEnd();

    glColor3f(0.94f, 0.80f, 0.68f);
    drawFilledCircle(x, y + 3.15f, 0.85f, 22);
    glColor3f(0.72f, 0.58f, 0.48f);
    drawCircle(x, y + 3.15f, 0.85f, 22);
}

static void drawPlayers(void) {
    int i;
    for (i = 0; i < 22; i++) {
        drawPlayer(players[i].x, players[i].y, players[i].team);
    }
}

static void drawCrowdBand(float yMin, float yMax, float seedOffset) {
    const int cols = 90;
    const int rows = 6;
    const float xStart = -58.0f;
    const float xEnd = 58.0f;
    const float dx = (xEnd - xStart) / (float)(cols - 1);
    const float dy = (yMax - yMin) / (float)(rows + 1);

    glPointSize(2.7f);
    glBegin(GL_POINTS);
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            float x = xStart + dx * (float)col;
            float yBase = yMin + dy * (float)(row + 1);
            float wave = sinf(crowdPhase + seedOffset + (float)col * 0.25f + (float)row * 0.7f);
            float y = yBase + wave * 0.25f;

            float pulse = 0.62f + 0.38f * sinf(crowdPhase * 1.3f + (float)col * 0.19f);
            if (((col + row) % 4) == 0) {
                glColor3f(0.92f * pulse, 0.18f * pulse, 0.18f * pulse);
            } else if (((col + row) % 4) == 1) {
                glColor3f(0.20f * pulse, 0.42f * pulse, 0.96f * pulse);
            } else if (((col + row) % 4) == 2) {
                glColor3f(0.96f * pulse, 0.92f * pulse, 0.24f * pulse);
            } else {
                glColor3f(0.92f * pulse, 0.92f * pulse, 0.92f * pulse);
            }

            glVertex2f(x, y);
        }
    }
    glEnd();
}

static void drawCrowdFlags(float yBase, float phaseOffset) {
    const int flags = 9;
    for (int i = 0; i < flags; i++) {
        float x = -52.0f + i * 13.0f;
        float wave = sinf(crowdPhase * 1.6f + phaseOffset + (float)i * 0.9f);
        float top = yBase + 2.2f;

        glColor3f(0.75f, 0.75f, 0.75f);
        glBegin(GL_LINES);
        glVertex2f(x, yBase);
        glVertex2f(x, top);
        glEnd();

        glBegin(GL_TRIANGLES);
        if ((i % 2) == 0) glColor3f(0.84f, 0.17f, 0.17f);
        else              glColor3f(0.16f, 0.35f, 0.86f);
        glVertex2f(x, top);
        glVertex2f(x + 2.2f + wave * 0.5f, top - 0.5f);
        glVertex2f(x, top - 1.0f);
        glEnd();
    }
}

static void drawCrowd(void) {
    // Arquibancadas em camadas para dar profundidade.
    drawGradientRect(-60.0f, 34.0f, 60.0f, 40.0f, 0.28f, 0.31f, 0.36f, 0.18f, 0.20f, 0.24f);
    drawGradientRect(-60.0f, -40.0f, 60.0f, -34.0f, 0.24f, 0.27f, 0.31f, 0.14f, 0.16f, 0.20f);

    glColor3f(0.36f, 0.39f, 0.44f);
    drawFilledRect(-60.0f, 36.8f, 60.0f, 37.2f);
    drawFilledRect(-60.0f, -37.2f, 60.0f, -36.8f);

    drawCrowdBand(34.0f, 40.0f, 0.0f);
    drawCrowdBand(-40.0f, -34.0f, 2.4f);
    drawCrowdFlags(37.0f, 0.0f);
    drawCrowdFlags(-37.0f, 1.7f);
}

static void drawFieldStripes(void) {
    // Gramado em listras verticais com variação sutil.
    const int stripes = 18;
    for (int i = 0; i < stripes; i++) {
        float x0 = -52.5f + (105.0f / (float)stripes) * (float)i;
        float x1 = -52.5f + (105.0f / (float)stripes) * (float)(i + 1);
        float tone = 0.015f * sinf((float)i * 0.9f + crowdPhase * 0.3f);
        if (i % 2 == 0) glColor3f(0.14f + tone, 0.49f + tone, 0.17f + tone);
        else            glColor3f(0.16f + tone, 0.53f + tone, 0.19f + tone);
        drawFilledRect(x0, -34.0f, x1, 34.0f);
    }

    // Vinheta suave nas bordas do gramado.
    glColor4f(0.02f, 0.12f, 0.03f, 0.20f);
    drawFilledRect(-52.5f, -34.0f, -50.8f, 34.0f);
    drawFilledRect(50.8f, -34.0f, 52.5f, 34.0f);
}

static void drawGoalNet(float xFront, float xBack) {
    const int netCols = 6;
    const int netRows = 5;
    const float yMin = -3.66f;
    const float yMax = 3.66f;

    glColor3f(0.88f, 0.88f, 0.88f);
    glLineWidth(1.0f);

    for (int i = 0; i <= netCols; i++) {
        float t = (float)i / (float)netCols;
        float x = xFront + t * (xBack - xFront);
        glBegin(GL_LINES);
        glVertex2f(x, yMin);
        glVertex2f(x, yMax);
        glEnd();
    }

    for (int j = 0; j <= netRows; j++) {
        float y = yMin + ((yMax - yMin) / (float)netRows) * (float)j;
        glBegin(GL_LINES);
        glVertex2f(xFront, y);
        glVertex2f(xBack, y);
        glEnd();
    }
}

static void drawSpecialPlayer() {
    if (specialPlayer.active) {
        drawPlayer(specialPlayer.x, specialPlayer.y, 0); // Usa o mesmo template do jogador do time 0
    }
}

static void updateSpecialPlayer() {
    if (specialPlayer.active) {
        specialPlayer.x += 1.5f; // Velocidade do jogador especial
        if (specialPlayer.x > 70.0f) { // Se ele cruzou o campo
            specialPlayer.active = false;
            ballMovementLocked = false; // Destrava a bola
        }
    }
}

static void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    // Fundo (grama ao redor)
    glColor3f(0.07f, 0.35f, 0.10f);
    drawFilledRect(-60.0f, -40.0f, 60.0f, 40.0f);
    drawCrowd();

    // Gramado interno com listras
    drawFieldStripes();

    // Linhas do campo
    glColor3f(0.96f, 0.96f, 0.96f);
    glLineWidth(2.2f);

    // Retângulo principal (105 x 68)
    drawRectOutline(-52.5f, -34.0f, 52.5f, 34.0f);

    // Linha do meio
    glBegin(GL_LINES);
    glVertex2f(0.0f, -34.0f);
    glVertex2f(0.0f, 34.0f);
    glEnd();

    // Círculo central (raio 9.15m)
    drawCircle(0.0f, 0.0f, 9.15f, 140);

    // Ponto central
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glVertex2f(0.0f, 0.0f);
    glEnd();

    // Grandes áreas (16.5m de profundidade, 40.32m de largura)
    drawRectOutline(-52.5f, -20.16f, -36.0f, 20.16f);  // esquerda
    drawRectOutline(36.0f, -20.16f, 52.5f, 20.16f);    // direita

    // Pequenas áreas (5.5m de profundidade, 18.32m de largura)
    drawRectOutline(-52.5f, -9.16f, -47.0f, 9.16f);    // esquerda
    drawRectOutline(47.0f, -9.16f, 52.5f, 9.16f);      // direita

    // Pontos de pênalti (11m da linha de gol)
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glVertex2f(-41.5f, 0.0f); // -52.5 + 11
    glVertex2f(41.5f, 0.0f);  //  52.5 - 11
    glEnd();

    // "D" da área (arco de raio 9.15m em torno do ponto penal)
    drawArc(-41.5f, 0.0f, 9.15f, -0.95f, 0.95f, 90);         // esquerda (aberto para dentro)
    drawArc(41.5f, 0.0f, 9.15f, PI - 0.95f, PI + 0.95f, 90); // direita

    // Arcos de escanteio (raio ~1m)
    drawArc(-52.5f, -34.0f, 1.0f, 0.0f, PI * 0.5f, 32);
    drawArc(-52.5f, 34.0f, 1.0f, -PI * 0.5f, 0.0f, 32);
    drawArc(52.5f, -34.0f, 1.0f, PI * 0.5f, PI, 32);
    drawArc(52.5f, 34.0f, 1.0f, PI, PI * 1.5f, 32);

    // Gols simples (fora do campo)
    drawRectOutline(-54.5f, -3.66f, -52.5f, 3.66f); // esquerda
    drawRectOutline(52.5f, -3.66f, 54.5f, 3.66f);   // direita
    drawGoalNet(-52.5f, -54.5f);
    drawGoalNet(52.5f, 54.5f);

    drawPlayers();
    drawSpecialPlayer();

    // Sombra da bola no gramado.
    glColor3f(0.05f, 0.08f, 0.05f);
    drawFilledCircle(ballX + 0.15f, ballY - 0.20f, ballR * 0.95f, 30);

    // Bola com sombreamento e "gomos".
    glColor3f(0.94f, 0.94f, 0.94f);
    drawFilledCircle(ballX, ballY, ballR, 42);
    glColor3f(0.78f, 0.78f, 0.78f);
    drawFilledCircle(ballX - 0.28f, ballY + 0.30f, ballR * 0.45f, 24);
    glColor3f(0.18f, 0.18f, 0.18f);
    drawFilledCircle(ballX, ballY, ballR * 0.22f, 16);
    drawArc(ballX, ballY, ballR * 0.55f, 0.2f, 2.7f, 24);
    drawArc(ballX, ballY, ballR * 0.55f, 3.4f, 5.9f, 24);
    glColor3f(0.10f, 0.10f, 0.10f);
    glLineWidth(1.0f);
    drawCircle(ballX, ballY, ballR, 42);

    /* Placar centralizado no topo da janela (coordenadas em pixels). */
    {
        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText), "%d  x  %d", scoreLeft, scoreRight);
        int w = g_winW;
        int h = g_winH;
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, (double)w, 0.0, (double)h, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        const int panelW = 240;
        const int panelH = 46;
        const int marginTop = 14;
        int px = w / 2 - panelW / 2;
        int py = h - marginTop - panelH;

        glColor3f(0.02f, 0.02f, 0.02f);
        drawFilledRect((float)px, (float)py, (float)(px + panelW), (float)(py + panelH));

        int textW = glutBitmapLength(GLUT_BITMAP_HELVETICA_18,
                                     reinterpret_cast<const unsigned char *>(scoreText));
        int tx = px + (panelW - textW) / 2;
        int ty = py + 15;
        glColor3f(1.0f, 1.0f, 1.0f);
        drawTextPixels(tx, ty, scoreText);

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    // Desenha o botão
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)g_winW, (double)g_winH, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    invaderButton.x = g_winW / 2 - invaderButton.w / 2;
    invaderButton.y = g_winH - 60;

    if (invaderButton.hovered) {
        glColor3f(0.2f, 0.6f, 0.2f);
    } else {
        glColor3f(0.1f, 0.4f, 0.1f);
    }
    drawFilledRect(invaderButton.x, invaderButton.y, invaderButton.x + invaderButton.w, invaderButton.y + invaderButton.h);

    glColor3f(1.0f, 1.0f, 1.0f);
    int textW = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)invaderButton.label);
    drawTextPixels(invaderButton.x + (invaderButton.w - textW) / 2, invaderButton.y + 25, invaderButton.label);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

static void reshape(int w, int h) {
    g_winW = w;
    g_winH = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (h == 0) ? 1.0f : (float)w / (float)h;
    float worldW = 120.0f, worldH = 80.0f;
    if (aspect >= worldW / worldH) {
        float halfW = (worldH * aspect) * 0.5f;
        glOrtho(-halfW, halfW, -worldH * 0.5f, worldH * 0.5f, -1.0f, 1.0f);
    } else {
        float halfH = (worldW / aspect) * 0.5f;
        glOrtho(-worldW * 0.5f, worldW * 0.5f, -halfH, halfH, -1.0f, 1.0f);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void moveBall(float dx, float dy) {
    if (ballMovementLocked) return; // Se o movimento da bola estiver travado, não faz nada

    const float fieldHalfW = 52.5f;
    const float fieldHalfH = 34.0f;
    const float goalHalfH = 3.66f;
    const float goalBackX = 54.5f;
    const float prevX = ballX;
    const float prevY = ballY;

    float nextY = clampf(ballY + dy, -fieldHalfH + ballR, fieldHalfH - ballR);
    float xLimit = (fabsf(nextY) <= goalHalfH) ? (goalBackX - ballR) : (fieldHalfW - ballR);
    float nextX = clampf(ballX + dx, -xLimit, xLimit);

    ballX = nextX;
    ballY = nextY;

    int scored = 0;

    // Gol na esquerda: ponto para o time da direita.
    if (ballX < -fieldHalfW && fabsf(ballY) <= goalHalfH) {
        scoreRight++;
        ballX = 0.0f;
        ballY = 0.0f;
        scored = 1;
    }

    // Gol na direita: ponto para o time da esquerda.
    if (ballX > fieldHalfW && fabsf(ballY) <= goalHalfH) {
        scoreLeft++;
        ballX = 0.0f;
        ballY = 0.0f;
        scored = 1;
    }

    if (scored) {
        audioPlayGoal();
    } else if (ballX != prevX || ballY != prevY) {
        audioPlayKick();
    }

    glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y) {
    (void)x;
    (void)y;

    switch (key) {
        case 'w':
        case 'W':
            moveBall(0.0f, ballStep);
            break;
        case 's':
        case 'S':
            moveBall(0.0f, -ballStep);
            break;
        case 'a':
        case 'A':
            moveBall(-ballStep, 0.0f);
            break;
        case 'd':
        case 'D':
            moveBall(ballStep, 0.0f);
            break;
        case 27: // ESC
            glutLeaveMainLoop();
            break;
        default:
            break;
    }
}

static void special(int key, int x, int y) {
    (void)x;
    (void)y;

    switch (key) {
        case GLUT_KEY_UP:
            moveBall(0.0f, ballStep);
            break;
        case GLUT_KEY_DOWN:
            moveBall(0.0f, -ballStep);
            break;
        case GLUT_KEY_LEFT:
            moveBall(-ballStep, 0.0f);
            break;
        case GLUT_KEY_RIGHT:
            moveBall(ballStep, 0.0f);
            break;
        default:
            break;
    }
}

static void timer(int value) {
    (void)value;
    crowdPhase += 0.08f;
    if (crowdPhase > 2.0f * PI) {
        crowdPhase -= 2.0f * PI;
    }
    updatePlayerPositions();
    updateSpecialPlayer(); // Atualiza o jogador especial
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    const int winW = 1200;
    const int winH = 780;
    glutInitWindowSize(winW, winH);
    {
        int sw = glutGet(GLUT_SCREEN_WIDTH);
        int sh = glutGet(GLUT_SCREEN_HEIGHT);
        if (sw > 0 && sh > 0) {
            glutInitWindowPosition((sw - winW) / 2, (sh - winH) / 2);
        } else {
            glutInitWindowPosition(80, 60);
        }
    }
    glutCreateWindow("Campo de Futebol - OpenGL + FreeGLUT");

    audioInit();
    initializePlayers();
    audioPlayCrowd();
    atexit(audioShutdown);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouseClick);
    glutPassiveMotionFunc(passiveMotion);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
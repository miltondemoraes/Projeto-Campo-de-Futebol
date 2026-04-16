#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>

#include "audio.h"

#define PI 3.14159265358979323846f

static float ballX = 0.0f;
static float ballY = 0.0f;
static const float ballR = 1.2f;
static const float ballStep = 1.2f;
static int scoreLeft = 0;
static int scoreRight = 0;
static float crowdPhase = 0.0f;
static int g_winW = 1200;
static int g_winH = 780;

static float clampf(float value, float minV, float maxV) {
    if (value < minV) return minV;
    if (value > maxV) return maxV;
    return value;
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
    /* Time da esquerda (ataca para a direita) */
    const float L[][2] = {
        { -46.5f, 0.0f },   /* goleiro */
        { -38.0f, -16.0f }, { -38.0f, -5.5f }, { -38.0f, 5.5f }, { -38.0f, 16.0f }, /* defesa */
        { -26.0f, -14.0f }, { -26.0f, -4.0f }, { -26.0f, 4.0f }, { -26.0f, 14.0f }, /* meio */
        { -14.0f, -8.0f }, { -14.0f, 8.0f }  /* ataque */
    };
    /* Time da direita */
    const float R[][2] = {
        { 46.5f, 0.0f },
        { 38.0f, -16.0f }, { 38.0f, -5.5f }, { 38.0f, 5.5f }, { 38.0f, 16.0f },
        { 26.0f, -14.0f }, { 26.0f, -4.0f }, { 26.0f, 4.0f }, { 26.0f, 14.0f },
        { 14.0f, -8.0f }, { 14.0f, 8.0f }
    };

    const int n = (int)(sizeof(L) / sizeof(L[0]));
    for (int i = 0; i < n; i++) {
        drawPlayer(L[i][0], L[i][1], 0);
        drawPlayer(R[i][0], R[i][1], 1);
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
    atexit(audioShutdown);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
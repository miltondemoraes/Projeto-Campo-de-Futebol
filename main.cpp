#include <math.h>
#include <GL/freeglut.h>

#define PI 3.14159265358979323846f

static void drawFilledRect(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
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

static void drawArc(float cx, float cy, float r, float a0, float a1, int segments) {
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float a = a0 + t * (a1 - a0);
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawFieldStripes(void) {
    // Listras horizontais suaves para dar realismo
    const int stripes = 12;
    for (int i = 0; i < stripes; i++) {
        float y0 = -34.0f + (68.0f / stripes) * i;
        float y1 = -34.0f + (68.0f / stripes) * (i + 1);
        if (i % 2 == 0) glColor3f(0.16f, 0.55f, 0.18f);
        else            glColor3f(0.13f, 0.50f, 0.16f);
        drawFilledRect(-52.5f, y0, 52.5f, y1);
    }
}

static void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    // Fundo (grama ao redor)
    glColor3f(0.07f, 0.35f, 0.10f);
    drawFilledRect(-60.0f, -40.0f, 60.0f, 40.0f);

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

    glutSwapBuffers();
}

static void reshape(int w, int h) {
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

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 780);
    glutCreateWindow("Campo de Futebol - OpenGL + FreeGLUT");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
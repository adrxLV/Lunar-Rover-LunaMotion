#include <stdio.h>
#include <math.h>

const float M_PI = 3.1415926535897;

#define DT 0.1         // Passo de tempo
#define K_ATT 1.0      // Ganho de atra��o
#define K_REP 100.0    // Ganho de repuls�o
#define RHO_0 2.0      // Raio de influ�ncia do obst�culo
#define K_THETA 2.0    // Ganho de controle angular
#define V_MAX 1.0      // Velocidade m�xima

// Estrutura do carrinho
typedef struct {
    double x;
    double y;
    double theta;
} Carrinho;

// Fun��o para calcular dist�ncia euclidiana
double dist(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

// Fun��o principal
int main() {
    // Inicializa��o do carrinho
    Carrinho carrinho = { 0.0, 0.0, 0.0 };

    // Objetivo (atrator)
    double goal_x = 5.0;
    double goal_y = 5.0;

    // Obst�culo (repulsor)
    double obs_x = 2.5;
    double obs_y = 2.5;

    // Simula��o
    for (int step = 0; step < 200; step++) {
        double dx_att = goal_x - carrinho.x;
        double dy_att = goal_y - carrinho.y;

        // For�a atrativa
        double fx_att = K_ATT * dx_att;
        double fy_att = K_ATT * dy_att;

        // Dist�ncia ao obst�culo
        double rho = dist(carrinho.x, carrinho.y, obs_x, obs_y);
        double fx_rep = 0.0, fy_rep = 0.0;

        if (rho < RHO_0) {
            double factor = K_REP * (1.0 / rho - 1.0 / RHO_0) / (rho * rho);
            fx_rep = factor * (carrinho.x - obs_x);
            fy_rep = factor * (carrinho.y - obs_y);
        }

        // For�a total
        double fx = fx_att + fx_rep;
        double fy = fy_att + fy_rep;

        // �ngulo desejado
        double theta_d = atan2(fy, fx);
        double e_theta = theta_d - carrinho.theta;

        // Normaliza o erro angular entre -pi e pi
        while (e_theta > M_PI) e_theta -= 2 * M_PI;
        while (e_theta < -M_PI) e_theta += 2 * M_PI;

        // Controle angular
        double omega = K_THETA * e_theta;

        // Controle de velocidade (reduz se desalinhado)
        double v = V_MAX * cos(e_theta);
        if (v < 0) v = 0;

        // Atualiza��o de estado
        carrinho.x += v * cos(carrinho.theta) * DT;
        carrinho.y += v * sin(carrinho.theta) * DT;
        carrinho.theta += omega * DT;

        // Sa�da para visualiza��o
        printf("Step %3d: x = %.2f, y = %.2f, theta = %.2f\n", step, carrinho.x, carrinho.y, carrinho.theta*360.0/M_PI);

        // Verifica chegada ao objetivo
        if (dist(carrinho.x, carrinho.y, goal_x, goal_y) < 0.1) {
            printf("Objetivo alcan�ado!\n");
            break;
        }
    }

    return 0;
}
#include <stdio.h>
#include <math.h>

const float M_PI = 3.1415926535897;

#define DT 0.1         // Passo de tempo
#define K_ATT 1.0      // Ganho de atração
#define K_REP 100.0    // Ganho de repulsão
#define RHO_0 2.0      // Raio de influência do obstáculo
#define K_THETA 2.0    // Ganho de controle angular
#define V_MAX 1.0      // Velocidade máxima

// Estrutura do carrinho
typedef struct {
    double x;
    double y;
    double theta;
} Carrinho;

// Função para calcular distância euclidiana
double dist(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

// Função principal
int main() {
    // Inicialização do carrinho
    Carrinho carrinho = { 0.0, 0.0, 0.0 };

    // Objetivo (atrator)
    double goal_x = 5.0;
    double goal_y = 5.0;

    // Obstáculo (repulsor)
    double obs_x = 2.5;
    double obs_y = 2.5;

    // Simulação
    for (int step = 0; step < 200; step++) {
        double dx_att = goal_x - carrinho.x;
        double dy_att = goal_y - carrinho.y;

        // Força atrativa
        double fx_att = K_ATT * dx_att;
        double fy_att = K_ATT * dy_att;

        // Distância ao obstáculo
        double rho = dist(carrinho.x, carrinho.y, obs_x, obs_y);
        double fx_rep = 0.0, fy_rep = 0.0;

        if (rho < RHO_0) {
            double factor = K_REP * (1.0 / rho - 1.0 / RHO_0) / (rho * rho);
            fx_rep = factor * (carrinho.x - obs_x);
            fy_rep = factor * (carrinho.y - obs_y);
        }

        // Força total
        double fx = fx_att + fx_rep;
        double fy = fy_att + fy_rep;

        // Ângulo desejado
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

        // Atualização de estado
        carrinho.x += v * cos(carrinho.theta) * DT;
        carrinho.y += v * sin(carrinho.theta) * DT;
        carrinho.theta += omega * DT;

        // Saída para visualização
        printf("Step %3d: x = %.2f, y = %.2f, theta = %.2f\n", step, carrinho.x, carrinho.y, carrinho.theta*360.0/M_PI);

        // Verifica chegada ao objetivo
        if (dist(carrinho.x, carrinho.y, goal_x, goal_y) < 0.1) {
            printf("Objetivo alcançado!\n");
            break;
        }
    }

    return 0;
}
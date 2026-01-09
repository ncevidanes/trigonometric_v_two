#include <math.h>
#include <emscripten.h> // Biblioteca necessária para o KEEPALIVE

// Buffer (memória) para guardar a fórmula
char formula[256];
int pos = 0; // Posição atual de leitura

// ------------------------------------------------------------
// FUNÇÕES EXPORTADAS (O JavaScript chama estas)
// ------------------------------------------------------------

// Recebe a string do JS, caractere por caractere
EMSCRIPTEN_KEEPALIVE
void set_formula_char(int index, int c) {
    if (index < 255) {
        formula[index] = (char)c;
        formula[index + 1] = '\0'; // Fecha a string com nulo
    }
}

// ------------------------------------------------------------
// FUNÇÕES INTERNAS (O JavaScript NÃO vê estas, só o C usa)
// ------------------------------------------------------------

char peek() {
    return formula[pos];
}

char get() {
    return formula[pos++];
}

// Declaração antecipada para permitir recursão
float parse_expression(float x_val); 

// Analisa números, 'x', parênteses e funções (s, c, t, q)
float parse_factor(float x_val) {
    char c = peek();
    
    // 1. Números
    if (c >= '0' && c <= '9') {
        float num = 0;
        while (peek() >= '0' && peek() <= '9') {
            num = num * 10 + (get() - '0');
        }
        // Suporte a decimais (ex: 10.5)
        if (peek() == '.') {
            get();
            float decimal = 0.1;
            while (peek() >= '0' && peek() <= '9') {
                num += (get() - '0') * decimal;
                decimal /= 10;
            }
        }
        return num;
    }
    // 2. Variável X
    else if (c == 'x') {
        get();
        return x_val;
    }
    // 3. Parênteses
    else if (c == '(') {
        get();
        float result = parse_expression(x_val);
        get(); // Consome o ')'
        return result;
    }
    // 4. Funções Matemáticas
    else if (c == 's') { get(); return sin(parse_factor(x_val)); } // Seno
    else if (c == 'c') { get(); return cos(parse_factor(x_val)); } // Cosseno
    else if (c == 't') { get(); return tan(parse_factor(x_val)); } // Tangente
    else if (c == 'q') { get(); return sqrt(parse_factor(x_val)); } // Raiz Quadrada (sqrt)
    
    return 0; // Se não entender, retorna 0
}

// Analisa Multiplicação (*), Divisão (/) e Potência (^)
float parse_term(float x_val) {
    float left = parse_factor(x_val);
    while (peek() == '*' || peek() == '/' || peek() == '^') {
        char op = get();
        float right = parse_factor(x_val);
        if (op == '*') left *= right;
        else if (op == '/') left /= right;
        else if (op == '^') left = pow(left, right);
    }
    return left;
}

// Analisa Soma (+) e Subtração (-)
float parse_expression(float x_val) {
    float left = parse_term(x_val);
    while (peek() == '+' || peek() == '-') {
        char op = get();
        float right = parse_term(x_val);
        if (op == '+') left += right;
        else if (op == '-') left -= right;
    }
    return left;
}

// ------------------------------------------------------------
// FUNÇÃO EXPORTADA PRINCIPAL
// ------------------------------------------------------------

// O JS chama isso 800 vezes por frame (uma vez para cada pixel X)
EMSCRIPTEN_KEEPALIVE
float calcular_formula(float x_val) {
    pos = 0; // Reseta o leitor para o início da frase
    return parse_expression(x_val);
}

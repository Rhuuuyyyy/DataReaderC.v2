#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <json-c/json.h>
#include "dados.bin"

#define MAX_DESCRICAO 100
#define MAX_CATEGORIA 50

typedef struct {
    int numero;
    char* descricao; // <-- ISSO É UM PONTEIRO
    double umidade;
    int energia_kcal;
    double proteina;
    double carboidrato;
    char* categoria; // <-- ISSO É UM PONTEIRO
} Alimento;

typedef struct {
    int numero;
    char descricao[MAX_DESCRICAO]; // <-- Array de tamanho fixo
    double umidade;
    int energia_kcal;
    double proteina;
    double carboidrato;
    char categoria[MAX_CATEGORIA]; // <-- Array de tamanho fixo
} AlimentoArquivo;

int main() {

}

// ===================================================================================
// FUNÇÃO PARA LER OS DADOS DO ARQUIVO .JSON ORIGINAL
// ===================================================================================
Alimento** ler_alimentos_do_json(const char* nome_arquivo, int* total_alimentos) {
    // Esta função é uma especialista em ler o arquivo .json, que tem uma estrutura mais complexa.
    FILE* arquivo = fopen(nome_arquivo, "r"); // O "r" serve para indicar que o arquivo vai ser apenas lido...
    if (arquivo == NULL) { perror("Nao foi possivel abrir o arquivo dados.json"); return NULL; }
    
    // Lê o conteúdo inteiro do arquivo para a memória de uma só vez.
    fseek(arquivo, 0, SEEK_END); long tamanho = ftell(arquivo); fseek(arquivo, 0, SEEK_SET); 
    /* "fseek" é um ponteiro de posição, ou seja, se o ponteiro está no final do arquivo, o elemento lido
    vai ser o mesmo que está no final do arquivo.
    O "fseek(arquivo, 0, SEEK_END): Joga o ponteiro de posição para o final do arquivo...
    O long tamanho = ftell(arquivo): Diz quantos bytes foram percorridos, fazendo com que a variável
    guarde o tamanho do arquivo...
    O "fseek(arquivo, 0, SEEK_SET): Faz o ponteiro de posição voltar para o início do arquivo..." */
    char* buffer_json = (char*)malloc(tamanho + 1);
    // Essa linha faz com que o "malloc" aloque a memória necessária (quantidade exata de bytes do arquivo lido + 1 para o \0)...
    if (buffer_json == NULL) { fprintf(stderr, "Falha ao alocar memoria para o buffer do JSON...\n"); fclose(arquivo); return NULL; }
    // Se nõa tiver memória o suficiente para alocar, o sistema retorna um erro..
    fread(buffer_json, 1, tamanho, arquivo); fclose(arquivo); buffer_json[tamanho] = '\0';
    // fread = file read: Lê as informações do arquivo e aloca efetivamente na memória...
    // o buffer_json[tamanho] = '\0' adiciona o \0 no final da memória para indicar o fim de uma string...
    
    // Usa a biblioteca 'json-c' para interpretar o texto do arquivo e transformá-lo em objetos que o C entende.
    json_object* json_completo = json_tokener_parse(buffer_json); /* O "tokener" seleciona o "buffer_json" que é um texto simples, e quebra em tokens lógicos,
     e o "parse" interpreta e analisa para entender a composição hierárquica do texto...*/
    if (json_completo == NULL) { fprintf(stderr, "Erro ao interpretar o conteudo JSON.\n"); free(buffer_json); return NULL; }
    
    size_t n_alimentos = json_object_array_length(json_completo); *total_alimentos = n_alimentos;
    /* O "n_alimentos" é um size_t, que é um tipo de variável que guarda o tamanho de algo na memória .O "json_completo" é o arquivo "dados.json" 
    já lido e interpretado, e sabemos que o objeto é um array.
    O "json_object_array_lenght()" serve para contar quantos
    elementos o array, que é um json, possui.*/
    Alimento** array_alimentos = (Alimento**)malloc(n_alimentos * sizeof(Alimento*)); /* Aqui a função pega o tamanho de um alimento,
    e aloca uma memória do tamanho da quantidade de alimentos x tamanho de cada alimento...*/
    if (array_alimentos == NULL) { fprintf(stderr, "Falha ao alocar memoria para o array de alimentos.\n"); free(buffer_json); json_object_put(json_completo); return NULL; }
    
    // Percorre cada objeto de alimento no JSON...
    for (int i = 0; i < n_alimentos; i++) {
        json_object* item = json_object_array_get_idx(json_completo, i); /* "json_completo" é o vetor onde está os alimentos. o "json_object_array_get_idx" seleciona
        o alimento que está na posição "i" */
        array_alimentos[i] = (Alimento*)malloc(sizeof(Alimento)); // Aloca uma memória do tamanho do item para o alimento.
        
        // Pega cada dado do objeto JSON (Numero, Descricao, etc.)...
        json_object *j_numero, *j_descricao, *j_umidade, *j_energia, *j_proteina, *j_carboidrato, *j_categoria;
        json_object_object_get_ex(item, "Numero", &j_numero); 
        json_object_object_get_ex(item, "Descricao", &j_descricao); 
        json_object_object_get_ex(item, "Umidade", &j_umidade); 
        json_object_object_get_ex(item, "Energia_kcal", &j_energia); 
        json_object_object_get_ex(item, "Proteina", &j_proteina); 
        json_object_object_get_ex(item, "Carboidrato", &j_carboidrato); 
        json_object_object_get_ex(item, "Categoria", &j_categoria);
        
        // ...e preenche a "ficha de alimento" com esses dados.
        array_alimentos[i]->numero = json_object_get_int(j_numero); 
        array_alimentos[i]->descricao = strdup(json_object_get_string(j_descricao)); 
        array_alimentos[i]->umidade = json_object_get_double(j_umidade); 
        array_alimentos[i]->energia_kcal = json_object_get_int(j_energia); 
        array_alimentos[i]->proteina = json_object_get_double(j_proteina); 
        array_alimentos[i]->carboidrato = json_object_get_double(j_carboidrato); 
        array_alimentos[i]->categoria = strdup(json_object_get_string(j_categoria));
    }
    
    json_object_put(json_completo); free(buffer_json); // Libera a memória usada pela leitura do JSON.
    return array_alimentos; // Retorna a lista completa com os 597 alimentos.
}

// ===================================================================================
// FUNÇÃO PARA SALVAR OS DADOS EM BINÁRIO
// ===================================================================================

bool salvar_em_binario(Alimento** array_memoria, int total_alimentos, const char* dados.bin) {
    FILE* f_binario = fopen(dados.bin, "wb"); // Essa linha serva para gerar um arquivo binário para apenas escrita -> "wb" = Wite Binary
    if (f_binario == NULL) {
        perror("Não foi possível criar o arquivo em Binário...");
        return false;
    }
    printf("Escrevendo dados em '%s'... \n", dados.bin)

    for (int i = 0; i < total_alimentos; i++) {
        AlimentoArquivo temp_bin; // Essa linha é responsável por criar uma struct temporária
        memset(&temp_bin, 0, sizeof(AlimentoArquivo));

        temp_bin.numero = array_memoria[i]->numero;
        temp_bin.numero = array_memoria[i]->umidade;
        temp_bin.numero = array_memoria[i]->proteina;
        temp_bin.numero = array_memoria[i]->energia_kcal;
        temp_bin.numero = array_memoria[i]->carboidrato;
        
        strncpy(temp_bin.descricao, array_memoria[i]->descricao, MAX_DESCICAO - 1);
        strncpy(temp_bin.descricao, array_memoria[i]->categoria, MAX_DESCICAO - 1);

        fwrite(&temp_bin, sizeof(AlimentoArquivo), 1, f_binario);
    }

    fclose(f_binario);
    printf("Arquivo '%s' criado com sucesso... \n", dados.bin)

    return true;
}

// ===================================================================================
// FUNÇÃO MAIN 
// ===================================================================================
int main() {
    int total_alimentos = 0;
    const char* json_filename = "dados.json"; // Variável que se trata do ponteiro do nome do arquivo json dos dados
    const char* bin_filename = "dados.bin"; // Variável que se trata do ponteiro do nome do arquivo binário dos dados

    // 1° PARTE: LER OS DADOS DO ARQUIVO JSON
    printf("Lendo dados de '%s'...\n", json_filename);
    Alimento** array_memoria = ler_alimentos_do_json(json_filename, &total_alimentos);
    
    if (array_memoria == NULL) {
        fprintf(stderr, "Falha ao ler o arquivo JSON. Encerrando.\n");
        return 1;
    }
    printf("%d alimentos lidos com sucesso da memória.\n", total_alimentos);

    // 2° PARTE: SALVAR OS DADOS LIDOS NO ARQUIVO BINÁRIO 
    if (!salvar_em_binario(array_memoria, total_alimentos, bin_filename)) {
        fprintf(stderr, "Falha ao salvar o arquivo binário.\n");
        // Aqui não retornamos 1, pois o código deve continuar, para que a memória seja liberada, e não cause um memory leak
    }

    // --- 3. LIBERAR MEMÓRIA ---
    printf("Limpando memória...\n");
    for (int i = 0; i < total_alimentos; i++) {
        free(array_memoria[i]->descricao); // Libera a string alocada com strdup
        free(array_memoria[i]->categoria); // Libera a string alocada com strdup
        free(array_memoria[i]);           // Libera a struct Alimento
        // O 
    }
    free(array_memoria); // Libera o array principal de ponteiros

    printf("Processo concluído.\n");
    return 0;
}

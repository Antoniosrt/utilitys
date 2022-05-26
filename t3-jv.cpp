#include <caca.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

struct Editor
{
    std::vector<char *> linhas; // linhas do editor
    bool sair;                  // sinal para sair
    unsigned int cursor_x;      // coluna do cursor
    unsigned int cursor_y;      // linha do cursor
    unsigned int nlinhas;       // maximo de linhas
    unsigned int ncolunas;      // maximo de colunas
    caca_canvas_t *cv;
    caca_display_t *dp;
    caca_event_t ev;

    void inicia(void)
    {
        // cria  atela
        dp = caca_create_display(NULL);
        if (!dp)
            throw std::runtime_error{"Nao foi possivel abrir janela!"};
        cv = caca_get_canvas(dp);
        caca_set_cursor(dp, 1);
        caca_set_mouse(dp, 1);
        caca_set_display_title(dp, "Editor Caca");
        caca_clear_canvas(cv);
        caca_set_color_ansi(cv, CACA_WHITE, CACA_BLACK);

        // inicia as variaveis principais
        sair = false;
        cursor_x = 0;
        cursor_y = 0;
        ncolunas = caca_get_canvas_width(cv);
        nlinhas = caca_get_canvas_height(cv);
        // std::cout << "Tamanho da tela: " << caca_get_canvas_width(cv)
        //    << " " << caca_get_canvas_height(cv) << std::endl;
    }

    // fecha o aplicativo e libera tudo
    void finaliza(void)
    {
        caca_free_display(dp);
        destroi();
    }

    // insere uma linha no editor, com alocacao dinamica
    void insere(std::string &linha)
    {
        char *vetor;
        if (linha.size() == 0)
            linhas.push_back(nullptr);
        else
        {
            vetor = new char[linha.size() + 1];
            strncpy(vetor, linha.c_str(), linha.size());
            vetor[linha.size()] = '\0';
            linhas.push_back(vetor);
        }
    }

    // remove a ultima linha sem liberar memoria
    // retorna a linha como char*
    char *remove(void)
    {
        char *l = linhas.back(); // pega ultima linha
        linhas.pop_back();       // remove ultima linha
        return l;
    }

    int tamanho(void) const
    {
        return linhas.size();
    }

    // precisa liberar toda memoria alocada
    void destroi(void)
    {
        // libera toda a memória
        for (auto it = linhas.begin(); it != linhas.end(); it++)
        {
            if (*it != nullptr)
                delete[](*it);
        }
        linhas.clear();
    }

    // le um arquivo inteiro nessa struct editor
    void carrega(std::string arquivo)
    {
        std::ifstream entrada{arquivo};
        std::string linha;
        while (std::getline(entrada, linha))
        {
            insere(linha);
        }
    }

    // salva todas as linhas do editor em um arquivo texto
    void salva(std::string s)
    {
        std::ofstream saida{s};
        for (auto it = linhas.begin(); it != linhas.end(); it++)
        {
            if (*it != nullptr)
                saida << *it << std::endl;
            else
                saida << std::endl;
        }
    }

    // move cursor
    void move_esq(void)
    {
        if (cursor_x > 0)
            cursor_x--;
    }

    // move cursor
    void move_dir(void)
    {
        if (cursor_x < (ncolunas - 1))
            cursor_x++;
    }

    // move cursor
    void move_cima(void)
    {
        if (cursor_y > 0)
            cursor_y--;
    }

    // move cursor
    void move_baixo(void)
    {
        if (cursor_y < (nlinhas - 1))
            cursor_y++;
    }

    // move cursor
    void move_end(void)
    {
        if (cursor_y < linhas.size() && linhas[cursor_y] != nullptr)
            cursor_x = strlen(linhas[cursor_y]);
        else
            cursor_x = 0;
    }

    // move cursor
    void move_home(void)
    {
        cursor_x = 0;
    }

    // insere um caractere na posicao atual do cursor.
    // - depois move cursor para direita
    // - se estiver em uma linha vazia (nullptr) cria linha
    // - se estiver apos o fim da linha, cria espacos
    // - se estiver apos a ultima linha, cria linhas nullptr vazias
    void insere_char(const char c)
    {
        if(linhas.size() == 0)
            linhas.push_back(nullptr);

        
        // - se estiver em uma linha vazia (nullptr) cria linha
        if (linhas[cursor_y] == nullptr)
        {
            linhas[cursor_y] = new char[2];
            linhas[cursor_y][0] = c;
            linhas[cursor_y][1] = '\0';
        }
        // - se estiver apos o fim da linha, cria espaços
        else if (cursor_x > strlen(linhas[cursor_y]))
        {
            char *aux = linhas[cursor_y];
            linhas[cursor_y] = new char[cursor_x - strlen(aux)];
            strncpy(linhas[cursor_y], aux, cursor_x);
            linhas[cursor_y][cursor_x] = c;
            linhas[cursor_y][cursor_x + 1] = '\0';
            delete[](aux);
        }
        else
        {
            char *aux = linhas[cursor_y];
            linhas[cursor_y] = new char[strlen(aux) + 2];
            //adiciona o caracter no lugar do cursor x e empurra a frase pra frente
            strncpy(linhas[cursor_y], aux, cursor_x);
            linhas[cursor_y][cursor_x] = c;
            strncpy(linhas[cursor_y] + cursor_x + 1, aux + cursor_x, strlen(aux) - cursor_x);
            linhas[cursor_y][strlen(aux) + 1] = '\0';
            delete[](aux);
            
        }
        move_dir();
    }

    // remove um caracter (BACKSPACE) antes da posicao do cursor
    // - move cursor para esquerda
    // - se esta no comeco da linha, gruda a linha com a de cima
    // - se a linha ficar vazia, libera memoria e deixa nullptr
    // - cursor em espaco vazio, faz nada
    void remove_char(void)
    {
        if (cursor_y >= linhas.size())
        {
            return;
        }
        if (cursor_x == 0)
        {
            if (cursor_y > 0)
            {
                gruda_linha();
            }
            else{
                return ;
            }
        }
        else
        {
            if (cursor_x > 0)
            {
                char *aux = linhas[cursor_y];
                linhas[cursor_y] = new char[strlen(aux) - 1];
                strncpy(linhas[cursor_y], aux, cursor_x - 1);
                 strncpy(linhas[cursor_y] + cursor_x - 1,
                     aux + cursor_x, strlen(aux) - cursor_x);
                linhas[cursor_y][strlen(aux) - 1] = '\0';
                delete[] aux;
                // if (strlen(linhas[cursor_y]) == 0)
                // {
                //     //delete[] linhas[cursor_y];
                //     linhas[cursor_y] = nullptr;
                // }
            }
        }
        move_esq();
    }

    // gruda linha atual com a de cima.
    // - move cursos para cima
    // - se a linha atual, ou a de cima, esta vazia, apenas remove a linha
    // - se esta em espaco vazio, nao tem efeito
    void gruda_linha(void)
    {
        // TODO
        if (cursor_y > 0)
        {
            if (linhas[cursor_y] == nullptr && linhas[cursor_y - 1] == nullptr)
            {
                remove();
                move_cima();
            }
            else if (linhas[cursor_y] == nullptr)
            {
                // char *aux = linhas[cursor_y - 1];
                // linhas[cursor_y - 1] = new char[strlen(aux) + 1];
                // strncpy(linhas[cursor_y - 1], aux, strlen(aux));
                // linhas[cursor_y - 1][strlen(aux)] = '\0';
                // delete[] aux;
                remove();
                move_cima();
            }
            else if (linhas[cursor_y - 1] == nullptr)
            {
                char *aux = linhas[cursor_y];
                linhas[cursor_y - 1] = new char[strlen(aux) + 1];
                strncpy(linhas[cursor_y - 1], aux, strlen(aux));
                linhas[cursor_y - 1][strlen(aux)] = '\0';
                delete[] aux;
                remove();
                move_cima();
            }
            else
            {
                char *aux = linhas[cursor_y];
                linhas[cursor_y - 1] = new char[strlen(aux) + strlen(linhas[cursor_y - 1]) + 1];
                strncpy(linhas[cursor_y - 1], linhas[cursor_y - 1], strlen(linhas[cursor_y - 1]));
                strncpy(linhas[cursor_y - 1] + strlen(linhas[cursor_y - 1]), aux, strlen(aux));
                linhas[cursor_y - 1][strlen(linhas[cursor_y - 1])] = '\0';
                delete[] aux;
                remove();
                move_cima();
            }
        }
        else{
            return ;
        }
    }

    // quebra a linha no cursor, onde a nova linha comeca com o caractere do cursor
    // - o cursor move para a proxima linha, comeco da linha
    // - se esta em espaco vazio, faz nada
    void quebra_linha(void)
    {
        // TODO
        if (cursor_y < linhas.size() && cursor_x+1 < strlen(linhas[cursor_y]))
        {
            if(linhas[cursor_y] == nullptr){
                return ;
            }
            //TODO:ARRUMAR PARA NAO EXCLUIR A LINHA ATUAL
            //pega toda linha atual
            char *aux = linhas[cursor_y];
            //cria nova linha
            linhas[cursor_y+1] = new char[strlen(aux) - cursor_x + 1];
            //copia para nova linha
            strncpy(linhas[cursor_y+1], aux, strlen(aux) - cursor_x);
            //mantem a linha antiga
            linhas[cursor_y] = strncpy(linhas[cursor_y], aux, cursor_x);
            //strncpy(linhas[cursor_y+1], aux, strlen(aux) - cursor_x);
            linhas[cursor_y][strlen(aux) - cursor_x] = '\0';
            delete[] aux;
            move_baixo();
            cursor_x = 0;
        }
         if(cursor_y <= linhas.size() && cursor_x+1 == strlen(linhas[cursor_y]))
        {   
            move_baixo();
            cursor_x = 0;
        }

        
    }

    void legenda(void)
    {
        // TODO
    }

    bool verifica_fim(void)
    {
        if (caca_get_event(dp, CACA_EVENT_ANY, &ev, 0))
        {
            if (caca_get_event_type(&ev) & CACA_EVENT_KEY_PRESS)
            {
                int tecla = caca_get_event_key_ch(&ev);
                switch (tecla)
                {
                case CACA_KEY_ESCAPE:
                    sair = true;
                    break;
                case CACA_KEY_END:
                    move_end();
                    break;
                case CACA_KEY_HOME:
                    move_home();
                    break;
                case CACA_KEY_UP:
                    // cima
                    move_cima();
                    break;
                case CACA_KEY_DOWN:
                    // baixo
                    move_baixo();
                    break;
                case CACA_KEY_LEFT:
                    // esquerda
                    move_esq();
                    break;
                case CACA_KEY_RIGHT:
                    // direita
                    move_dir();
                    break;
                case CACA_KEY_BACKSPACE:
                    remove_char();
                    break;
                case CACA_KEY_DELETE:
                    break;
                case CACA_KEY_RETURN:
                    quebra_linha();
                    break;
                case CACA_KEY_CTRL_D:
                    salva("saida.txt");
                    break;
                default:
                    insere_char(static_cast<char>(tecla));
                    break;
                } // switch
            }     // if key
        }         // if event

        return sair;
    }

    void atualiza(void)
    {
        unsigned int n;
        auto it = linhas.begin();
        // limpa a tela
        caca_clear_canvas(cv);
        // desenha o texto
        for (n = 0, it = linhas.begin();
             it != linhas.end() && n < nlinhas; it++, n++)
        {
            if ((*it) != nullptr)
            {
                // std::cout << *it << std::endl;
                caca_put_str(cv, 0, n, *it);
            }
        }
        // posiciona cursor
        caca_gotoxy(cv, cursor_x, cursor_y);
        // manda mostrar tudo na tela
        caca_refresh_display(dp);
        // dorme um pouco
        usleep(100);
    }
    // pode adicionar mais funcoes!
}; // fim do editor

int main(int argc, char **argv)
{
    Editor editor;
    // inicia o editor
    editor.inicia();
    editor.carrega("texto.txt");
    // editor.legenda();

    while (editor.verifica_fim() == false)
    {
        editor.atualiza();
    }
    editor.finaliza();

    return 0;
}

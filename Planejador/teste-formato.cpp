#include <fstream>
#include <iostream>
#include <stdexcept>
#include "planejador.h"

using namespace std;

void verificar(bool condicao, const char* mensagem)
{
  if (!condicao)
    throw runtime_error(mensagem);
}

// Grava os arquivos usados exclusivamente neste teste.
void gravar(const char* nome, const char* conteudo)
{
  ofstream arquivo(nome);

  if (!arquivo.is_open())
    throw runtime_error("Nao foi possivel criar o arquivo de teste.");

  arquivo << conteudo;
  arquivo.close();

  if (arquivo.fail())
    throw runtime_error("Falha ao gravar o arquivo de teste.");
}

int main()
{
  try
  {
    const char* arq_pontos = "build/formato-pontos.txt";
    const char* arq_rotas = "build/formato-rotas.txt";

    Planejador P;

    // Caso 1: registros separados por espaco, sem ENTER final.
    gravar(arq_pontos,
           "ID;Nome;Latitude;Longitude\n"
           "#A;Ponto A;0;0 #B;Ponto B;0;0.01");

    gravar(arq_rotas,
           "ID;Nome;Extremidade 1;Extremidade 2;Comprimento\n"
           "&AB;Rota AB;#A;#B;3");

    P.ler(arq_pontos, arq_rotas);

    verificar(P.getNumPontos() == 2 && P.getNumRotas() == 1,
              "Arquivo valido sem ENTER final foi lido incorretamente.");

    cout << "OK: registros com espaco e sem ENTER final aceitos.\n";

    // Caso 2: dois pontos grudados, sem separador.
    gravar(arq_pontos,
           "ID;Nome;Latitude;Longitude\n"
           "#A;Ponto A;0;0#B;Ponto B;0;0.01");

    bool rejeitou = false;

    try
    {
      P.ler(arq_pontos, arq_rotas);
    }
    catch (const ios_base::failure&)
    {
      rejeitou = true;
    }

    verificar(rejeitou,
              "Pontos sem separador deveriam ser rejeitados.");

    cout << "OK: pontos sem separador rejeitados.\n";

    // Restaura o arquivo valido de pontos.
    gravar(arq_pontos,
           "ID;Nome;Latitude;Longitude\n"
           "#A;Ponto A;0;0 #B;Ponto B;0;0.01");

    // Caso 3: duas rotas grudadas, sem separador.
    gravar(arq_rotas,
           "ID;Nome;Extremidade 1;Extremidade 2;Comprimento\n"
           "&AB;Rota AB;#A;#B;3&BA;Rota BA;#B;#A;4");

    rejeitou = false;

    try
    {
      P.ler(arq_pontos, arq_rotas);
    }
    catch (const ios_base::failure&)
    {
      rejeitou = true;
    }

    verificar(rejeitou,
              "Rotas sem separador deveriam ser rejeitadas.");

    cout << "OK: rotas sem separador rejeitadas.\n";
  }
  catch (const exception& erro)
  {
    cerr << "ERRO: " << erro.what() << '\n';
    return 1;
  }

  return 0;
}
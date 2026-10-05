#include <iostream>
#include <chrono>
#include "planejador.h"

using namespace std;

/// NAO DEVE SER MODIFICADA
int main()
{
  // O planejador de caminhos
  Planejador G;
  // O caminho a ser calculado:
  // Os pontos do caminho
  Caminho C;
  // O numero de nohs gerados no calculo do caminho
  int NA(-1),NF(-1);
  // O comprimento do caminho calculado
  double compr(-1.0);
  // O tempo de calculo do caminho
  double deltaT;

  try
  {
    G.ler("pontos.txt", "rotas.txt");
  }
  catch(exception& E)
  {
    cerr << "Erro na leitura do mapa: " << E.what() << endl;
    return -1;
  }

  // Variaveis auxiliares para leitura de dados
  string S;
  int opcao;
  IDPonto id_origem, id_destino;
  Ponto P;

  do
  {
    cout << endl;
    cout << "1 - Imprimir pontos\n";
    cout << "2 - Imprimir rotas\n";
    cout << "3 - Calcular e imprimir caminho\n";
    cout << "0 - Sair\n";
    do
    {
      cout << "OPCAO: ";
      cin >> S;
      try
      {
        opcao = stoi(S);
      }
      catch(...)
      {
        opcao = -1;
      }
    }
    while (opcao<0 || opcao>3);
    switch(opcao)
    {
    case 1:
      cout << "PONTOS:\n";
      for (size_t i=0; i<G.getNumPontos(); ++i)
      {
        cout << G.getPonto(i) << endl;
      }
      break;
    case 2:
      cout << "ROTAS\n";
      for (size_t i=0; i<G.getNumRotas(); ++i)
      {
        cout << G.getRota(i) << endl;
      }
      break;
    case 3:
      do
      {
        cout << "ID do ponto de origem: ";
        cin >> S;
        id_origem.set(move(S));
        try
        {
          P = G.getPonto(id_origem);
        }
        catch(...)
        {
          id_origem = IDPonto();
        }
      } while (!id_origem.valid());
      do
      {
        cout << "ID do ponto de destino: ";
        cin >> S;
        id_destino.set(move(S));
        try
        {
          P = G.getPonto(id_destino);
        }
        catch(...)
        {
          id_destino = IDPonto();
        }
      } while (!id_destino.valid());

      // Calcula o tempo de execucao do calculo do caminho
      {
        using namespace chrono;

        // Relogio antes da execucao
        steady_clock::time_point t1 = steady_clock::now();
        // Calcula o caminho
        compr = G.calculaCaminho(id_origem,id_destino,C,NA,NF);
        // Relogio depois da execucao
        steady_clock::time_point t2 = steady_clock::now();
        // Diferenca entre os dois instantes de tempo
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        // Converte para milessegundos
        deltaT = 1000*time_span.count();
      }

      // Imprime os dados sobre o calculo do caminho
      cout << "Tempo: " << deltaT << "ms\t"
           << "Nohs em aberto: " << NA << " fechado: " << NF << endl;

      // Imprime o comprimento total (-1 se erro ou se nao existe caminho)
      cout << "TOTAL: " << compr << "km\n";

      // Imprime o resultado do calculo
      if (NA<0 || NF<0)
      {
        // Nunca deve entrar aqui...
        cout << "Erro no algoritmo de calculo do caminho. Reveja seu codigo!\n";
      }
      else if (compr<0.0)
      {
        cout << "Algoritmo concluido. Nenhum caminho foi encontrado\n";
      }
      else
      {
        Trecho T;
        Rota R;
        Ponto P;

        // Imprime as etapas do caminho
        cout << "==========\n";
        for (const auto& T : C)
        {
          if (!T.first.valid())
          {
            cout << "De ";
          }
          else
          {
            R = G.getRota(T.first);
            cout << "Por " << R.nome << " ateh ";
          }
          P = G.getPonto(T.second);
          cout << P.nome;
          if (T.first.valid()) cout << " (" << R.comprimento << "km)";
          cout << endl;
        }
      }
      break;
    case 0:
    default:
      break;
    }
  }
  while (opcao!=0);

  return 0;
}

#include <fstream>
#include <iostream>
#include <stdexcept>
#include "planejador.h"

using namespace std;

// Interrompe o teste se uma verificacao falhar.
void verificar(bool condicao, const char* mensagem)
{
  if (!condicao)
    throw runtime_error(mensagem);
}

int main()
{
  try
  {
    // Cria arquivos exclusivos deste teste na pasta build.
    {
      ofstream pontos("build/pontos_astar.txt");
      ofstream rotas("build/rotas_astar.txt");

      verificar(pontos.is_open() && rotas.is_open(),
                "Nao foi possivel criar os arquivos de teste.");

      pontos << "ID;Nome;Latitude;Longitude\n"
             << "#A;Ponto A;0;0\n"
             << "#B;Ponto B;0;0.01\n"
             << "#C;Ponto C;0;0.02\n"
             << "#D;Ponto D;0;0.03\n";

      rotas << "ID;Nome;Extremidade 1;Extremidade 2;Comprimento\n"
            << "&AC;Rota AC;#A;#C;10\n"
            << "&AB;Rota AB;#A;#B;3\n"
            << "&BC;Rota BC;#B;#C;4\n";
    }

    Planejador P;
    P.ler("build/pontos_astar.txt", "build/rotas_astar.txt");

    IDPonto A, B, C, D;
    A.set("#A");
    B.set("#B");
    C.set("#C");
    D.set("#D");

    IDRota AB, BC;
    AB.set("&AB");
    BC.set("&BC");

    Caminho caminho;
    int numAberto, numFechado;
    double comprimento;

    // Caso 1: o caminho indireto eh mais curto.
    comprimento = P.calculaCaminho(
        A, C, caminho, numAberto, numFechado);

    verificar(comprimento == 7.0,
              "Ate C: o comprimento deveria ser 7 km.");

    verificar(caminho.size() == 3,
              "Ate C: o caminho deveria conter tres pontos.");

    verificar(caminho[0].second == A &&
              caminho[1].second == B &&
              caminho[2].second == C,
              "Ate C: a sequencia deveria ser A, B, C.");

    verificar(!caminho[0].first.valid() &&
              caminho[1].first == AB &&
              caminho[2].first == BC,
              "Ate C: as rotas do caminho estao incorretas.");

    verificar(numAberto == 0 && numFechado == 3,
              "Ate C: contagens de Aberto ou Fechado incorretas.");

    cout << "OK: caminho minimo A -> B -> C, com 7 km.\n";

    // Caso 2: origem e destino iguais.
    comprimento = P.calculaCaminho(
        A, A, caminho, numAberto, numFechado);

    verificar(comprimento == 0.0 && caminho.size() == 1,
              "Origem igual ao destino: resultado incorreto.");

    verificar(caminho[0].second == A &&
              !caminho[0].first.valid(),
              "Origem igual ao destino: trecho incorreto.");

    verificar(numAberto == 0 && numFechado == 1,
              "Origem igual ao destino: contagens incorretas.");

    cout << "OK: origem igual ao destino, com 0 km.\n";

    // Caso 3: o destino existe, mas esta isolado.
    comprimento = P.calculaCaminho(
        A, D, caminho, numAberto, numFechado);

    verificar(comprimento == -1.0 && caminho.empty(),
              "Destino isolado: deveria retornar -1 e caminho vazio.");

    verificar(numAberto == 0 && numFechado == 3,
              "Destino isolado: contagens incorretas.");

    cout << "OK: destino isolado identificado.\n";
  }
  catch (const exception& erro)
  {
    cerr << "ERRO: " << erro.what() << '\n';
    return 1;
  }

  return 0;
}
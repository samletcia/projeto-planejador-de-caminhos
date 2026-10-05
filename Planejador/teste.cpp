#include <iostream>
#include <string>
#include "planejador.h"

using namespace std;

int main()
{
  Planejador Base,X;
  string arq_pontos, arq_rotas;
  const string DIR="arq_teste/";

  // Leitura dos arquivos base
  arq_pontos = "pontos.txt";
  arq_rotas = "rotas.txt";
  try
  {
    Base.ler(arq_pontos,arq_rotas);
    cout << "OK! Arquivos base lidos "
         << " NumPontos=" << Base.getNumPontos()
         << " NumRotas=" << Base.getNumRotas() << endl;
  }
  catch(exception& E)
  {
    cerr << "ERRO na leitura dos arquivos base - " << E.what() << endl;
  }

  Ponto PB, PX;
  Rota RB, RX;
  size_t I,K;

  // Leitura de arquivos validos
  for (I=1; I<=2; ++I)
  {
    arq_pontos = DIR+"pontos_ok"+to_string(I)+".txt";
    arq_rotas = DIR+"rotas_ok"+to_string(I)+".txt";
    cout << "Lendo arquivos " << I << ": " << arq_pontos << " e " << arq_rotas << endl;
    try
    {
      X.ler(arq_pontos,arq_rotas); // Gera excecao se houver erro na leitura
      if (X.getNumPontos() != Base.getNumPontos())
      {
        cerr << "ERRO: NumPontos base=" << Base.getNumPontos()
             << " NumPontos " << arq_pontos << "=" << X.getNumPontos() << endl;
        throw 0;
      }
      for (K=0; K<Base.getNumPontos(); ++K)
      {
        PB = Base.getPonto(K);
        PX = X.getPonto(K);
        if (PB.id != PX.id ||
            PB.nome != PX.nome ||
            PB.latitude != PX.latitude ||
            PB.longitude != PX.longitude)
        {
           cerr << "ERRO: Ponto " << K << " da base:\n" << PB << endl
                << "diferente do Ponto " << K << " lido:\n" << PX << endl;
           throw 0;
        }
      }
      if (X.getNumRotas() != Base.getNumRotas())
      {
        cerr << "ERRO: NumRotas base=" << Base.getNumRotas()
             << " NumRotas " << arq_pontos << "=" << X.getNumRotas() << endl;
      }
      for (K=0; K<Base.getNumRotas(); ++K)
      {
        RB = Base.getRota(K);
        RX = X.getRota(K);
        if (RB.id != RX.id ||
            RB.nome != RX.nome ||
            RB.extremidade[0] != RX.extremidade[0] ||
            RB.extremidade[1] != RX.extremidade[1] ||
            RB.comprimento != RX.comprimento)
        {
           cerr << "ERRO: Rota " << K << " da base:\n" << RB << endl
                << "diferente da Rota " << K << " lida:\n" << RX << endl;
           throw 0;
        }
      }
      cout << "OK! Mesmos pontos e rotas" << endl;
    }
    catch(int Z)
    {
      // Nao faz nada: msg de erro jah foi impressa
    }
    catch(exception& E)
    {
      cerr << "ERRO na leitura do arquivo " << I << " - " << E.what() << endl;
    }
  }

  // Tentativa de leitura de arquivos de pontos invalidos
  cout << "\nVerifique se foi detectado erro nos arquivos incorretos de pontos e nao alterou conteudo anterior\n";

  // Arquivo de rotas reduzido correto
  arq_rotas = DIR+"rotas_ok0.txt";

  for (I=1; I<=24; ++I)
  {
    // Guarda as caracteristicas do arquivo correto
    X = Base;

    arq_pontos = DIR+"pontos_err"+to_string(I)+".txt";
    try
    {
      X.ler(arq_pontos,arq_rotas);
      cerr << I << " - ERRO (nao detectou erro)" << endl;
    }
    catch(exception& E)
    {
      if (X.getNumPontos()!= Base.getNumPontos() ||
          X.getNumRotas() != Base.getNumRotas())
      {
        cerr << I << " - ERRO (detectou erro mas alterou conteudo): " << E.what() << endl;
      }
      else
      {
        cout << I << " - OK (detectou erro e nao alterou conteudo): " << E.what() << endl;
      }
    }
  }

  // Tentativa de leitura de arquivos de rotas invalidos
  cout << "\nVerifique se foi detectado erro nos arquivos incorretos de rotas e nao alterou conteudo anterior\n";

  // Arquivo de pontos reduzido correto
  arq_pontos = DIR+"pontos_ok0.txt";

  for (I=1; I<=24; ++I)
  {
    // Guarda as caracteristicas do arquivo correto
    X = Base;

    arq_rotas = DIR+"rotas_err"+to_string(I)+".txt";
    try
    {
      X.ler(arq_pontos,arq_rotas);
      cerr << I << " - ERRO (nao detectou erro)" << endl;
    }
    catch(exception& E)
    {
      if (X.getNumPontos()!= Base.getNumPontos() ||
          X.getNumRotas() != Base.getNumRotas())
      {
        cerr << I << " - ERRO (detectou erro mas alterou conteudo): " << E.what() << endl;
      }
      else
      {
        cout << I << " - OK (detectou erro e nao alterou conteudo): " << E.what() << endl;
      }
    }
  }


  return 0;
}

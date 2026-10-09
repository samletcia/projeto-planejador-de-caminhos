#include "planejador.h"
#include <stdexcept>
#include <cmath>  /* sin, cos, etc */
#include <cctype> /* isspace */
#include <fstream>
#include <algorithm>
#include <list>

using namespace std;

/* *************************
 * CLASSE IDPONTO        *
 ************************* */

/// Atribuicao de string
/// NAO DEVE SER MODIFICADA
void IDPonto::set(string &&S)
{
  t = move(S);
  if (!valid())
    t.clear();
}

/* *************************
 * CLASSE IDROTA         *
 ************************* */

/// Atribuicao de string
/// NAO DEVE SER MODIFICADA
void IDRota::set(string &&S)
{
  t = move(S);
  if (!valid())
    t.clear();
}

/* *************************
 * CLASSE PONTO          *
 ************************* */

/// Impressao em console
/// NAO DEVE SER MODIFICADA
ostream &operator<<(ostream &X, const Ponto &P)
{
  X << P.id << '\t' << P.nome << " (" << P.latitude << ',' << P.longitude << ')';
  return X;
}

/// Distancia entre 2 pontos (formula de haversine)
/// NAO DEVE SER MODIFICADA
double Ponto::distancia(const Ponto &P) const
{
  // Gera excecao se pontos invalidos
  if (!valid() || !P.valid())
    throw invalid_argument("distancia: ponto(s) invalido(s)");

  // Tratar logo pontos identicos
  if (id == P.id)
    return 0.0;
  // Constantes
  static const double MY_PI = 3.14159265358979323846;
  static const double R_EARTH = 6371.0;
  // Conversao para radianos
  double lat1 = MY_PI * this->latitude / 180.0;
  double lat2 = MY_PI * P.latitude / 180.0;
  double lon1 = MY_PI * this->longitude / 180.0;
  double lon2 = MY_PI * P.longitude / 180.0;
  // Seno das diferencas
  double sin_dlat2 = sin((lat2 - lat1) / 2.0);
  double sin_dlon2 = sin((lon2 - lon1) / 2.0);
  // Quadrado do seno do angulo entre os pontos
  double sin2_ang = sin_dlat2 * sin_dlat2 + cos(lat1) * cos(lat2) * sin_dlon2 * sin_dlon2;
  // Em vez de utilizar a funcao arcosseno, asin(sqrt(sin2_ang)),
  // vou utilizar a funcao arcotangente, menos sensivel a erros numericos.
  // Distancia entre os pontos
  return 2.0 * R_EARTH * atan2(sqrt(sin2_ang), sqrt(1 - sin2_ang));
}

/* *************************
 * CLASSE ROTA           *
 ************************* */

/// Impressao em console
/// NAO DEVE SER MODIFICADA
ostream &operator<<(ostream &X, const Rota &R)
{
  X << R.id << '\t' << R.nome << '\t' << R.comprimento << "km"
    << " [" << R.extremidade[0] << ',' << R.extremidade[1] << ']';
  return X;
}

/// Retorna a outra extremidade da rota, a que nao eh o parametro.
/// Gera excecao se o parametro nao for uma das extremidades da rota.
/// NAO DEVE SER MODIFICADA
IDPonto Rota::outraExtremidade(const IDPonto &ID) const
{
  if (extremidade[0] == ID)
    return extremidade[1];
  if (extremidade[1] == ID)
    return extremidade[0];
  throw invalid_argument("outraExtremidade: invalid IDPonto parameter");
}

/* *************************
 * CLASSE PLANEJADOR     *
 ************************* */

/// Torna o mapa vazio
/// NAO DEVE SER MODIFICADA
void Planejador::clear()
{
  pontos.clear();
  rotas.clear();
}

/// Funcao auxiliar para eliminar eventuais separadores do final de uma string.
/// NAO DEVE SER MODIFICADA
void trim(string &S)
{
  while (!S.empty() && isspace(S.back()))
    S.pop_back();
}

/// Leh um mapa dos arquivos arq_pontos e arq_rotas.
/// Caso nao consiga ler dos arquivos, deixa o mapa inalterado e
/// gera excecao ios_base::failure.
/// Deve receber ACRESCIMOS
void Planejador::ler(const std::string &arq_pontos,
                     const std::string &arq_rotas)
{
  // Vetores temporarios para armazenamento dos Pontos e Rotas lidos.
  vector<Ponto> pontos_lidos;
  vector<Rota> rotas_lidas;

  // Leh os Pontos do arquivo e armazena no vetor temporario de Pontos.
  // Em caso de qualquer erro, gera excecao ios_base::failure com mensagem:
  //   "Erro <CODIGO> na leitura do arquivo de pontos <ARQ_PONTOS>"
  try
  {
    // 1) Abre uma stream associada ao arquivo de Pontos
    //    (Em caso de erro, codigo 1)
    // 2) Consome eventuais separadores, leh o cabecalho do arquivo, elimina eventuais
    //    separadores no final da string e testa o cabecalho:
    //    "ID;Nome;Latitude;Longitude"
    //    (Em caso de erro ou valor lido diferente, codigo 2)
    //    Consome os separadores apos o cabecalho
    // 3) Enquanto o arquivo nao acabar (eof), repita a leitura de cada um dos Pontos:
    //    | 3.1) Leh a ID e elimina eventuais separadores no final da string
    //    |      (Em caso de erro ou conteudo lido vazio, codigo 3)
    //    |      O teste se a ID eh valida serah feito ao testar o Ponto
    //    | 3.2) Consome os separadores, leh o nome e elimina eventuais separadores no final
    //    |      da string
    //    |      (Em caso de erro ou conteudo lido vazio, codigo 4)
    //    |      O teste se o nome eh valido serah feito ao testar o Ponto
    //    | 3.3) Leh a latitude
    //    |      (Em caso de erro, codigo 5)
    //    |      O teste se a latitude eh valida serah feito ao testar o Ponto
    //    | 3.4) Leh o caractere ';'
    //    |      (Em caso de erro ou valor lido diferente, codigo 6)
    //    | 3.5) Leh a longitude
    //    |      (Em caso de erro, codigo 7)
    //    |      O teste se a longitude eh valida serah feito ao testar o Ponto
    //    | 3.6) Consome os separadores apos o Ponto
    //    | 3.7) Testa se o Ponto com os parametros lidos eh valido
    //    |      (Em caso de erro, codigo 8)
    //    | 3.8) Testa que nao existe Ponto com a mesma ID no vetor temporario
    //    |      de Pontos lidos ateh agora
    //    |      (Em caso de erro, codigo 9)
    //    | 3.9) Insere o Ponto lido no vetor temporario de Pontos
    // 4) Se nao foi lido nenhum Ponto, gera erro (codigo 10)
    // 5) Fecha o arquivo de Pontos
    ifstream arquivo(arq_pontos);

    if (!arquivo.is_open())
      throw 1;

    string S;

    if (!getline(arquivo >> ws, S))
      throw 2;

    trim(S);

    if (S != "ID;Nome;Latitude;Longitude")
      throw 2;

    arquivo >> ws;

    while (!arquivo.eof())
    {
      Ponto P;

      // Le a identificacao ate o ponto e virgula.
      if (!getline(arquivo >> ws, S, ';'))
        throw 3;

      trim(S);

      if (S.empty())
        throw 3;

      P.id.set(move(S));

      // Le o nome ate o ponto e virgula.
      if (!getline(arquivo >> ws, P.nome, ';'))
        throw 4;

      trim(P.nome);

      if (P.nome.empty())
        throw 4;

      // Le a latitude.
      if (!(arquivo >> P.latitude))
        throw 5;

      // Confere o separador entre latitude e longitude.
      char separador;

      if (!(arquivo >> separador) || separador != ';')
        throw 6;

      // Le a longitude.
      if (!(arquivo >> P.longitude))
        throw 7;

      // Exige um separador entre registros, ou o fim do arquivo.
      int proximo = arquivo.peek();

      if (proximo != char_traits<char>::eof() && !isspace(proximo))
        throw 7;

      // Prepara a stream para o proximo registro.
      arquivo >> ws;

      // Verifica as regras de validade do ponto.
      if (!P.valid())
        throw 8;

      // Rejeita uma identificacao que ja foi lida.
      if (find(pontos_lidos.begin(), pontos_lidos.end(), P.id) != pontos_lidos.end())
        throw 9;

      // Guarda o ponto validado no vetor temporario.
      pontos_lidos.push_back(move(P));
    }

    if (pontos_lidos.empty())
      throw 10;

    arquivo.close();
  }
  catch (int i)
  {
    // Chama o destrutor de todas as variaveis criadas dentro do try, inclusive da
    // stream associada ao arquivo. Portanto, nao precisa fechar a stream.
    string msg = "Erro " + to_string(i) + " na leitura do arquivo de pontos " + arq_pontos;
    throw ios_base::failure(msg);
  }

  // Leh as Rotas do arquivo e armazena no vetor temporario de Rotas.
  // Em caso de qualquer erro, gera excecao ios_base::failure com mensagem:
  //   "Erro <CODIGO> na leitura do arquivo de rotas <ARQ_ROTAS>"
  try
  {
    // 1) Abre uma stream associada ao arquivo de Rotas
    //    (Em caso de erro, codigo 1)
    // 2) Consome eventuais separadores, leh o cabecalho do arquivo, elimina eventuais
    //    separadores no final da string e testa o cabecalho:
    //    "ID;Nome;Extremidade 1;Extremidade 2;Comprimento"
    //    (Em caso de erro ou valor lido diferente, codigo 2)
    //    Consome os separadores apos o cabecalho
    // 3) Enquanto o arquivo nao acabar (eof), repita a leitura de cada uma das Rotas:
    //    | 3.1) Leh a ID e elimina eventuais separadores no final da string
    //    |      (Em caso de erro ou conteudo lido vazio, codigo 3)
    //    |      O teste se a ID eh valida serah feito ao testar a Rota
    //    | 3.2) Consome os separadores, leh o nome e elimina eventuais separadores no final
    //    |      da string
    //    |      (Em caso de erro ou conteudo lido vazio, codigo 4)
    //    |      O teste se o nome eh valido serah feito ao testar a Rota
    //    | 3.3) Consome os separadores, leh a ID da extremidade[0] e elimina eventuais
    //    |      separadores no final da string
    //    |      (Em caso de erro ou conteudo lido vazio, codigo 5)
    //    |      O teste se a ID eh valida serah feito ao testar a Rota
    //    | 3.4) Consome os separadores, leh a ID da extremidade[1] e elimina eventuais
    //    |      separadores no final da string
    //    |      (Em caso de erro ou conteudo lido vazio, codigo 6)
    //    |      O teste se a ID eh valida serah feito ao testar a Rota
    //    | 3.5) Leh o comprimento
    //    |      (Em caso de erro na leitura, codigo 7)
    //    |      O teste se o comprimento eh valido serah feito ao testar o Ponto
    //    | 3.6) Consome os separadores apos a Rota
    //    | 3.7) Testa se a Rota com esses parametros lidos eh valida
    //    |      (Em caso de erro, codigo 8)
    //    | 3.8) Testa que a Id da extremidade[0] corresponde a um ponto lido
    //    |      no vetor temporario de Pontos
    //    |      (Em caso de erro, codigo 9)
    //    | 3.9) Testa que a Id da extremidade[1] corresponde a um ponto lido
    //    |      no vetor temporario de Pontos
    //    |      (Em caso de erro, codigo 10)
    //    | 3.10)Testa que nao existe Rota com a mesma ID no vetor temporario
    //    |      de Rotas lidas ateh agora
    //    |      (Em caso de erro, codigo 11)
    //    | 3.11)Insere a Rota lida no vetor temporario de Rotas
    // 4) Se nao foi lido nenhuma Rota, gera erro (codigo 12)
    // 5) Fecha o arquivo de Rotas
    ifstream arquivo(arq_rotas);

    if (!arquivo.is_open())
      throw 1;

    // Le e confere o cabecalho.
    string S;

    if (!getline(arquivo >> ws, S))
      throw 2;

    trim(S);

    if (S != "ID;Nome;Extremidade 1;Extremidade 2;Comprimento")
      throw 2;

    arquivo >> ws;

    while (!arquivo.eof())
    {
      Rota R;

      // Le a identificacao da rota.
      if (!getline(arquivo >> ws, S, ';'))
        throw 3;

      trim(S);

      if (S.empty())
        throw 3;

      R.id.set(move(S));

      // Le o nome da rota.
      if (!getline(arquivo >> ws, R.nome, ';'))
        throw 4;

      trim(R.nome);

      if (R.nome.empty())
        throw 4;

      // Le a identificacao da primeira extremidade.
      if (!getline(arquivo >> ws, S, ';'))
        throw 5;

      trim(S);

      if (S.empty())
        throw 5;

      R.extremidade[0].set(move(S));

      // Le a identificacao da segunda extremidade.
      if (!getline(arquivo >> ws, S, ';'))
        throw 6;

      trim(S);

      if (S.empty())
        throw 6;

      R.extremidade[1].set(move(S));

      // Le o comprimento da rota.
      if (!(arquivo >> R.comprimento))
        throw 7;
        
      // Exige um separador entre registros, ou o fim do arquivo.
      int proximo = arquivo.peek();

      if (proximo != char_traits<char>::eof() && !isspace(proximo))
        throw 7;

      arquivo >> ws;

      // Confere a validade dos dados da rota.
      if (!R.valid())
        throw 8;

      // Confere se a primeira extremidade existe no mapa.
      if (find(pontos_lidos.begin(), pontos_lidos.end(),
               R.extremidade[0]) == pontos_lidos.end())
        throw 9;

      // Confere se a segunda extremidade existe no mapa.
      if (find(pontos_lidos.begin(), pontos_lidos.end(),
               R.extremidade[1]) == pontos_lidos.end())
        throw 10;

      // Rejeita uma identificacao de rota repetida.
      if (find(rotas_lidas.begin(), rotas_lidas.end(), R.id) != rotas_lidas.end())
        throw 11;

      rotas_lidas.push_back(move(R));
    }

    if (rotas_lidas.empty())
      throw 12;

    arquivo.close();
  }
  catch (int i)
  {
    // Chama o destrutor de todas as variaveis criadas dentro do try, inclusive da
    // stream associada ao arquivo. Portanto, nao precisa fechar a stream.
    string msg = "Erro " + to_string(i) + " na leitura do arquivo de rotas " + arq_rotas;
    throw ios_base::failure(msg);
  }

  // Faz os vetores de Pontos e Rotas do planejador assumirem o conteudo dos
  // vetores temporarios de Pontos e Rotas
  pontos.swap(pontos_lidos);
  rotas.swap(rotas_lidas);
}

/// Retorna um Ponto do mapa, passando a id como parametro.
/// Se a id for inexistente, gera excecao.
/// Deve receber ACRESCIMOS
Ponto Planejador::getPonto(const IDPonto &Id) const
{
  // Procura um ponto que corresponde aa Id do parametro
  auto itr = find(pontos.begin(), pontos.end(), Id);
  // Em caso de sucesso, retorna o ponto encontrado
  if (itr != pontos.end())
    return *itr;

  // Se nao encontrou, gera excecao
  throw invalid_argument("getPonto: invalid IDPonto parameter");
}

/// Retorna um Rota do mapa, passando a id como parametro.
/// Se a id for inexistente, gera excecao.
/// Deve receber ACRESCIMOS
Rota Planejador::getRota(const IDRota &Id) const
{
  // Procura uma rota que corresponde aa Id do parametro
  auto itr = find(rotas.begin(), rotas.end(), Id);

  // Em caso de sucesso, retorna a rota encontrada
  if (itr != rotas.end())
    return *itr;

  // Se nao encontrou, gera excecao
  throw invalid_argument("getRota: invalid IDRota parameter");
}

/// *******************************************************************************
/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// *******************************************************************************

/// Noh: struct/classe dos elementos dos conjuntos de busca do algoritmo A*.
/// Deve ser DECLARADA E IMPLEMENTADA inteiramente.
struct Noh
{
  IDPonto id_pt;
  IDRota id_rt;
  double g;
  double h;

  Noh() : id_pt(), id_rt(), g(0.0), h(0.0) {}

  double f() const
  {
    return g + h;
  }

  bool operator==(const IDPonto &Id) const
  {
    return id_pt == Id;
  }

  bool operator<(const Noh &outro) const
  {
    return f() < outro.f();
  }
};

/// Calcula o caminho mais curto no mapa entre origem e destino, usando o algoritmo A*
/// Retorna o comprimento do caminho encontrado (<0 se nao existe caminho).
/// O parametro C retorna o caminho encontrado (vazio se nao existe caminho).
/// O parametro NumAberto retorna o numero de nos (>=0) em Aberto ao termino do algoritmo A*,
/// mesmo quando nao existe caminho.
/// O parametro NumFechado retorna o numero de nos (>=0) em Fechado ao termino do algoritmo A*,
/// mesmo quando nao existe caminho.
/// Em caso de parametros de entrada invalidos ou de erro no algoritmo, gera excecao.
/// Deve receber ACRESCIMOS.
double Planejador::calculaCaminho(const IDPonto &id_origem,
                                  const IDPonto &id_destino,
                                  Caminho &C, int &NumAberto, int &NumFechado)
{
  // Comprimento total do caminho encontrado, a ser retornado pela funcao calculaCaminho.
  // Inicializado com valor -1, que significa caminho nao encontrado.
  // Ao termino do algoritmo, deve passar a conter o valor correto.
  double Compr = -1.0;
  // Zera o caminho resultado.
  // Ao termino do algoritmo, deve passar a conter o valor correto.
  C.clear();
  // Atribui valores invalidos no numero de nohs calculados.
  // Ao termino do algoritmo, deve passar a conter o valor correto.
  NumAberto = NumFechado = -1;

  try
  {
    // Mapa vazio
    if (empty())
      throw 1;

    Ponto pt_origem, pt_destino;
    // Calcula os pontos que correspondem a id_origem e id_destino.
    // Se algum nao existir, throw 2
    try
    {
      pt_origem = getPonto(id_origem);
      pt_destino = getPonto(id_destino);
    }
    catch (...)
    {
      throw 2;
    }

    /* *****************************  /
    /  IMPLEMENTACAO DO ALGORITMO A*  /
    /  ***************************** */

    // Conjuntos de busca.
    list<Noh> Aberto;
    vector<Noh> Fechado;

    // Prepara o noh correspondente ao ponto de origem.
    Noh atual;

    atual.id_pt = id_origem;
    atual.id_rt = IDRota();
    atual.g = 0.0;
    atual.h = pt_origem.distancia(pt_destino);

    // A busca comeca apenas com a origem em Aberto.
    Aberto.push_back(atual);

    // CONTINUAR AQUI: laco principal do A*
    while (!Aberto.empty())
    {
      // Retira o primeiro noh, que possui o menor custo total.
      atual = Aberto.front();
      Aberto.pop_front();

      // Registra o noh analisado.
      Fechado.push_back(atual);

      // Encerra a busca quando o destino eh retirado de Aberto.
      if (atual.id_pt == id_destino)
        break;

      // Localiza a primeira rota conectada ao ponto atual.
      auto itr_rota = find(rotas.begin(), rotas.end(), atual.id_pt);

      while (itr_rota != rotas.end())
      {
        // CONTINUAR AQUI: gerar e avaliar o sucessor desta rota

        // Cria o sucessor na outra extremidade da rota.
        Noh suc;

        suc.id_pt = itr_rota->outraExtremidade(atual.id_pt);
        suc.id_rt = itr_rota->id;

        // Soma o comprimento desta rota ao custo ja percorrido.
        suc.g = atual.g + itr_rota->comprimento;

        // Calcula a estimativa da distancia restante.
        Ponto pt_suc = getPonto(suc.id_pt);
        suc.h = pt_suc.distancia(pt_destino);

        // CONTINUAR AQUI: verificar o sucessor em Fechado e Aberto

        bool inserir = true;

        // Verifica se o ponto ja foi analisado.
        auto itr_fechado = find(Fechado.begin(), Fechado.end(), suc.id_pt);

        if (itr_fechado != Fechado.end())
        {
          inserir = false;
        }
        else
        {
          // Verifica se ja existe uma opcao para esse ponto em Aberto.
          auto itr_aberto = find(Aberto.begin(), Aberto.end(), suc.id_pt);

          if (itr_aberto != Aberto.end())
          {
            if (suc.f() < itr_aberto->f())
            {
              // Remove a opcao antiga, pois encontramos uma melhor.
              Aberto.erase(itr_aberto);
            }
            else
            {
              // A opcao existente tem custo menor ou igual.
              inserir = false;
            }
          }
        }

        if (inserir)
        {
          // Encontra o primeiro noh com custo maior que o do sucessor.
          auto posicao = upper_bound(Aberto.begin(), Aberto.end(), suc);

          // Insere antes dessa posicao, mantendo a ordem.
          Aberto.insert(posicao, suc);
        }

        // Continua a busca a partir da rota seguinte.
        ++itr_rota;
        itr_rota = find(itr_rota, rotas.end(), atual.id_pt);
      }
    }

    // CONTINUAR AQUI: registrar resultados e reconstruir o caminho

    // Registra as quantidades finais dos conjuntos de busca.
    NumAberto = static_cast<int>(Aberto.size());
    NumFechado = static_cast<int>(Fechado.size());

    if (atual.id_pt == id_destino)
    {
      // O custo passado do destino eh o comprimento total.
      Compr = atual.g;

      // Percorre o caminho de tras para frente.
      while (atual.id_rt.valid())
      {
        C.push_front(Trecho(atual.id_rt, atual.id_pt));

        // Recupera a rota usada para chegar ao ponto atual.
        Rota rota_ant = getRota(atual.id_rt);

        // A outra extremidade identifica o ponto antecessor.
        IDPonto id_pt_ant = rota_ant.outraExtremidade(atual.id_pt);

        // Recupera o noh do antecessor em Fechado.
        auto itr_ant = find(Fechado.begin(), Fechado.end(), id_pt_ant);

        if (itr_ant == Fechado.end())
          throw 3;

        atual = *itr_ant;
      }

      // Inclui a origem, que nao possui rota de chegada.
      C.push_front(Trecho(IDRota(), atual.id_pt));
    }
  }
  catch (int i)
  {
    string msg_err = "Erro " + to_string(i) + " no calculo do caminho\n";
    throw invalid_argument(msg_err);
  }

  // Retorna o comprimento calculado para o caminho
  return Compr;
}

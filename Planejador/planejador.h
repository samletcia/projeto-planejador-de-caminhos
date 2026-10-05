#ifndef _PLANEJADOR_H_
#define _PLANEJADOR_H_

#include <string>
#include <iostream> /* istream, ostream */
#include <utility>  /* pair */
#include <deque>
#include <vector>

/* *************************
 * CLASSE IDPONTO        *
 ************************* */

/// Identificador de um Ponto
/// NAO DEVE SER MODIFICADA
class IDPonto
{
private:
  std::string t;

public:
  // Construtor
  IDPonto() : t("") {}
  // Atribuicao de string
  void set(std::string &&S);
  // Teste de validade
  bool valid() const
  {
    return (t.size() >= 2 && t[0] == '#');
  }
  // Comparacao
  friend inline bool operator==(const IDPonto &ID1, const IDPonto &ID2)
  {
    return ID1.t == ID2.t;
  }
  friend inline bool operator!=(const IDPonto &ID1, const IDPonto &ID2)
  {
    return ID1.t != ID2.t;
  }
  // Impressao
  friend std::ostream &operator<<(std::ostream &X, const IDPonto &ID)
  {
    return X << ID.t;
  }
};

/* *************************
 * CLASSE IDROTA         *
 ************************* */

/// Identificador de uma Rota
/// NAO DEVE SER MODIFICADA
class IDRota
{
private:
  std::string t;

public:
  // Construtor
  IDRota() : t("") {}
  // Atribuicao de string temporaria
  void set(std::string &&S);
  // Teste de validade
  bool valid() const
  {
    return (t.size() >= 2 && t[0] == '&');
  }
  // Comparacao
  friend inline bool operator==(const IDRota &ID1, const IDRota &ID2)
  {
    return ID1.t == ID2.t;
  }
  friend inline bool operator!=(const IDRota &ID1, const IDRota &ID2)
  {
    return ID1.t != ID2.t;
  }
  // Impressao
  friend std::ostream &operator<<(std::ostream &X, const IDRota &ID)
  {
    return X << ID.t;
  }
};

/* *************************
 * CLASSE PONTO          *
 ************************* */

/// Um ponto no mapa
/// Pode receber ACRESCIMOS (nao modificacoes, supressoes)
struct Ponto
{
  IDPonto id;       // Identificador do ponto
  std::string nome; // Denominacao usual do ponto
  double latitude;  // Em graus: -90 polo sul, +90 polo norte
  double longitude; // Em graus: de -180 a +180 (positivos a leste de Greenwich,
                    //                           negativos a oeste de Greenwich)
  // Construtor default
  Ponto() : id(), nome(""), latitude(0.0), longitude(0.0) {}
  // Teste de validade
  bool valid() const
  {
    return id.valid() && nome.size() >= 2 &&
           latitude >= -90.0 && latitude <= 90.00 &&
           longitude >= -180.0 && longitude <= 180.0;
  }

  // Impressao em console
  friend std::ostream &operator<<(std::ostream &X, const Ponto &P);

  // Sobrecarga de operadores
  // Utilizados pelos algoritmos STL
  bool operator==(const IDPonto &Id) const
  {
    return id == Id;
  }

  // Distancia entre 2 pontos (formula de haversine)
  double distancia(const Ponto &P) const;
};

/* *************************
 * CLASSE ROTA           *
 ************************* */

/// Uma rota no mapa
/// Pode receber ACRESCIMOS (nao modificacoes, supressoes)
struct Rota
{
  IDRota id;              // Identificador da rota
  std::string nome;       // Denominacao usual da rota
  IDPonto extremidade[2]; // Ids dos pontos extremos da rota
  double comprimento;     // Comprimento da rota (em km)

  // Construtor default
  Rota() : id(), nome(""), extremidade(), comprimento(0.0) {}
  // Teste de validade
  bool valid() const
  {
    return id.valid() && nome.size() >= 2 && comprimento > 0.0 &&
           extremidade[0].valid() && extremidade[1].valid();
  }

  // Impressao em console
  friend std::ostream &operator<<(std::ostream &X, const Rota &R);

  // Sobrecarga de operadores
  // Utilizados pelos algoritmos STL
  bool operator==(const IDRota &Id) const
  {
    return id == Id;
  }

  bool operator==(const IDPonto &Id) const
  {
    return extremidade[0] == Id || extremidade[1] == Id;
  }

  // Retorna a outra extremidade da rota, a que nao eh o parametro.
  // Gera excecao se o parametro nao for uma das extremidades da rota.
  IDPonto outraExtremidade(const IDPonto &ID) const;
};

/* *************************
 * CLASSE CAMINHO        *
 ************************* */

/// Um caminho encontrado entre dois pontos: uma pilha de Trechos.
/// Cada Trecho eh um par <IDRota,IDPonto>.
/// No 1o Trecho, a IDRota eh vazia == IDRota() e a IDPonto deve ser do ponto origem.
/// No ultimo Trecho, a IDPonto deve ser do ponto destino.
/// Cada Trecho, exceto o primeiro, eh composto pela rota que trouxe do
/// elemento anterior ateh ele e pelo ponto que faz parte do caminho.
/// NAO DEVEM SER MODIFICADAS
using Trecho = std::pair<IDRota, IDPonto>;
using Caminho = std::deque<Trecho>;

/* *************************
 * CLASSE PLANEJADOR     *
 ************************* */

/// A classe que armazena os pontos e as rotas do mapa do Planejador
/// e calcula caminho mais curto entre pontos.
/// NAO DEVE SER MODIFICADA
class Planejador
{
private:
  std::vector<Ponto> pontos;
  std::vector<Rota> rotas;

public:
  // Cria um mapa vazio
  Planejador() : pontos(), rotas() {}

  // Cria um mapa com o conteudo dos arquivos arq_pontos e arq_rotas.
  // Gera excecao em caso de arquivos invalidos.
  Planejador(const std::string &arq_pontos,
             const std::string &arq_rotas) : Planejador()
  {
    ler(arq_pontos, arq_rotas);
  }

  // Destrutor (nao eh obrigatorio...)
  ~Planejador()
  {
    clear();
  }

  // Torna o mapa vazio
  void clear();

  // Testa se um mapa estah vazio
  bool empty() const
  {
    return pontos.empty();
  }

  // Leh um mapa dos arquivos arq_pontos e arq_rotas.
  // Caso nao consiga ler dos arquivos, deixa o mapa inalterado e gera excecao ios_base::failure.
  void ler(const std::string &arq_pontos,
           const std::string &arq_rotas);

  // Consulta dimensoes do mapa
  size_t getNumPontos() const { return pontos.size(); }
  size_t getNumRotas() const { return rotas.size(); }

  // Retorna um Ponto do mapa, passando o indice como parametro.
  // Se o indice for invalido, a chamada ao "at" gera excecao.
  Ponto getPonto(int I) const { return pontos.at(I); }

  // Retorna uma Rota do mapa, passando o indice como parametro.
  // Se o indice for invalido, a chamada ao "at" gera excecao.
  Rota getRota(int I) const { return rotas.at(I); }

  // Retorna um Ponto do mapa, passando a id como parametro.
  // Se a id for inexistente, gera excecao.
  Ponto getPonto(const IDPonto &Id) const;

  // Retorna um Rota do mapa, passando a id como parametro.
  // Se a id for inexistente, gera excecao.
  Rota getRota(const IDRota &Id) const;

  // Calcula o caminho mais curto no mapa entre origem e destino, usando o algoritmo A*
  // Retorna o comprimento do caminho encontrado (<0 se nao existe caminho).
  // O parametro C retorna o caminho encontrado (vazio se nao existe caminho).
  // O parametro NumAberto retorna o numero de nos (>=0) em Aberto ao termino do algoritmo A*,
  // mesmo quando nao existe caminho.
  // O parametro NumFechado retorna o numero de nos (>=0) em Fechado ao termino do algoritmo A*,
  // mesmo quando nao existe caminho.
  // Em caso de parametros de entrada invalidos ou de erro no algoritmo, gera excecao.
  double calculaCaminho(const IDPonto &id_origem,
                        const IDPonto &id_destino,
                        Caminho &C, int &NumAberto, int &NumFechado);
};

#endif // _PLANEJADOR_H_

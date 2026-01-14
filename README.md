# Sistema de Leitura de Células de Carga

Projeto eletrônico desenvolvido para leitura de **empuxo de motores foguete**, produzido pela equipe de foguetemodelismo **Cactus Rockets Design**.

O sistema realiza a **aquisição, calibração, armazenamento e transmissão em tempo real da força (empuxo)** medida por células de carga, utilizando microcontroladores ESP32.

Para usar conseguir usar estes códigos propriamente, basta ter o Arduino IDE instalado em sua máquina e configurado para ESP32.

---

## Funcionamento Geral

O sistema é baseado em um ESP32 responsável por realizar a leitura das células de carga por meio do módulo HX711, converter os valores digitais obtidos em força (N) através de um processo de calibração e, em seguida, armazenar e transmitir esses dados.

O HX711 atua como um conversor analógico-digital de alta resolução, transformando as variações de resistência das células de carga em valores digitais. Esses valores não possuem unidade física direta e, por isso, passam por um processo de calibração que os relaciona a uma força conhecida, permitindo a medição do empuxo.

Após a conversão, os dados são gravados em um cartão microSD e transmitidos em tempo real para outro ESP32 utilizando o protocolo ESP-NOW, que permite comunicação sem fio de baixa latência sem a necessidade de um roteador Wi-Fi.

Foi adotada uma conexão USB macho/fêmea entre as células de carga e o HX711 com o objetivo de facilitar a modularidade do sistema. Dessa forma, é possível adicionar ou remover células de carga para alterar o empuxo máximo suportado sem modificar o restante do circuito. Sempre que essa configuração for alterada, a calibração deve ser refeita via software.

---

## Estrutura do Sistema

O sistema foi projetado de forma modular e é dividido em duas partes principais. O transmissor é responsável pela leitura das células de carga, pela aplicação da calibração, pelo armazenamento dos dados e pela transmissão das informações. O receptor, por sua vez, atua apenas na recepção dos dados enviados e na sua exibição em tempo real.

Para que o sistema funcione corretamente, ambos os módulos devem estar configurados de maneira compatível, especialmente no que diz respeito à comunicação via ESP-NOW.

---

## Requisitos de Software

Para utilizar o sistema, é necessário ter a Arduino IDE instalada e configurada com suporte às placas ESP32. Além disso, algumas bibliotecas específicas devem ser instaladas por meio do Gerenciador de Bibliotecas da própria Arduino IDE.

As bibliotecas utilizadas são a HX711 (versão 0.7.3), responsável pela comunicação com o conversor HX711, e a biblioteca SD (versão 1.2.4), utilizada para gravação dos dados no cartão microSD.

---

## Calibração da Célula de Carga

Antes de realizar qualquer medição de empuxo, é obrigatória a calibração da célula de carga para garantir que os valores lidos representem corretamente a força aplicada.

A calibração é realizada utilizando o código `Calibration_PID.ino`, que executa um procedimento de calibração em dois pontos. Inicialmente, o sistema mede o valor correspondente ao zero de carga, ou seja, sem nenhuma força aplicada. Em seguida, é aplicada uma carga conhecida à célula de carga para que o sistema possa relacionar a variação dos valores digitais à força real.

Durante o procedimento, o usuário deve acompanhar o Monitor Serial. O sistema solicitará a remoção de qualquer carga da célula e, posteriormente, a aplicação do peso conhecido. A partir de múltiplas leituras médias, o código calcula automaticamente o fator de calibração e o offset do HX711.

A conversão de massa para força é feita utilizando a relação física:

Força (N) = massa (kg) × gravidade (9,8 m/s²)

Ao final do processo, o Monitor Serial exibirá os valores de fator de calibração e offset prontos para serem utilizados no código do transmissor. Sempre que a célula de carga, a configuração mecânica ou o número de células for alterado, a calibração deve ser refeita.

---

## Configuração do Transmissor

Após a obtenção dos parâmetros de calibração, o arquivo `Transmitter.ino` deve ser configurado com os valores calculados. Esses parâmetros garantem que a função de leitura retorne diretamente a força em Newtons.

Além disso, o código permite habilitar ou desabilitar a gravação dos dados no cartão microSD e a transmissão via ESP-NOW, de acordo com a necessidade do ensaio. Essa flexibilidade permite que o sistema seja utilizado tanto em testes simples quanto em campanhas completas de aquisição de dados.

---

## Configuração do Endereço MAC (ESP-NOW)

Para que a transmissão de dados funcione corretamente, é necessário configurar o endereço MAC do ESP32 utilizado como receptor. Esse endereço identifica de forma única o dispositivo que irá receber os dados enviados pelo transmissor.

O endereço MAC pode ser obtido executando o código `Get_MAC_address.ino` no ESP32 receptor e observando o valor exibido no Monitor Serial. Esse valor deve ser convertido para o formato hexadecimal utilizado no código e inserido na variável `broadcastAddress` do arquivo `Transmitter.ino`.

Após essa configuração, o transmissor estará apto a enviar os dados diretamente para o receptor.

---

## Configuração do Receptor

A configuração do receptor é simples e consiste apenas em realizar o upload do código `Receiver.ino` no ESP32 responsável pela recepção. Uma vez configurado, o receptor passará a exibir em tempo real o tempo da medição e a força medida pelo sistema transmissor.

---

## Aquisição e Armazenamento de Dados

Cada amostra adquirida pelo sistema contém o instante de tempo em milissegundos, a força medida em Newtons, além dos valores de fator de calibração e offset utilizados. Essas informações são exibidas no Monitor Serial, gravadas no arquivo `data.txt` no cartão microSD e transmitidas em tempo real para o receptor por meio do ESP-NOW.

---

## Observações Importantes

A calibração deve ser refeita sempre que houver troca da célula de carga, alteração da configuração mecânica, mudança no número de células utilizadas e quanto algum teste de empuxo for ser realizado. O sistema mede diretamente força, expressa em Newtons, e não massa. Para garantir que as leituras estão de fato corretas, é aconselhado que dois pesos diferentes sejam ultilizados na calibração. Em leituras com alta taxa de amostragem, é esperado que haja algum nível de ruído nos dados.

---

## Links

* [Relatório do sistema antigo](https://drive.google.com/file/d/1Hbn8Wrecm2mFYuo-eOkDm_AL81IvbxrG/view?usp=sharing)

---

Projeto desenvolvido para fins acadêmicos e experimentais na área de instrumentação, aquisição de dados e sistemas embarcados.

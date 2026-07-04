# Conversor Buck Microcontrolado

Projeto de um conversor CC-CC abaixador (Buck Converter) com controle digital implementado em um microcontrolador **CH32V003**, desenvolvido com foco em aplicações de eletrônica de potência, sistemas embarcados e controle digital.

O projeto contempla o desenvolvimento completo do hardware e do firmware, incluindo o dimensionamento da etapa de potência, condicionamento dos sinais analógicos, controle por PWM, algoritmo de controle PI, proteção contra sobrecorrente e interface de comunicação serial.

---

## Principais características

* Conversor Buck microcontrolado.
* Controle digital de tensão utilizando controlador **PI**.
* Geração de PWM por hardware utilizando o temporizador **TIM1**.
* Aquisição de tensão e corrente através do ADC.
* Conversão automática utilizando **ADC + DMA**.
* Comunicação serial UART para monitoramento e configuração.
* Limitação de corrente por software.
* Controle de rampa (*Rate Limiter*) para partida suave e alterações da referência.
* Proteção contra *Integral Windup*.
* Firmware desenvolvido em linguagem C.
* Hardware desenvolvido no KiCad.

---

## Hardware

O conversor é composto pelos seguintes blocos principais:

* Microcontrolador **CH32V003**;
* Conversor Buck de potência;
* Driver para acionamento do MOSFET;
* Fonte auxiliar isolada para alimentação do gate driver;
* Reguladores de tensão para alimentação dos circuitos de controle;
* Divisor resistivo para medição da tensão;
* Resistor shunt para medição da corrente;
* Amplificador operacional interno do CH32V003 para condicionamento do sinal de corrente.

---

## Estratégia de controle

O controle da tensão de saída é realizado por um controlador **Proporcional–Integral (PI)**.

A implementação inclui:

* cálculo periódico do erro;
* atualização do termo integral;
* limitação do integrador (*Anti-Windup*);
* saturação da saída do controlador;
* atualização do ciclo de trabalho do PWM.

Para reduzir transitórios durante alterações da referência, foi implementado um algoritmo de **Rate Limiter**, permitindo variações graduais da tensão de saída.

Como mecanismo adicional de proteção, quando a corrente ultrapassa o limite configurado, a referência de tensão é reduzida gradualmente e o integrador do controlador é parcialmente descarregado, proporcionando recuperação rápida após a remoção da condição de sobrecorrente.

---

## Comunicação Serial

O firmware disponibiliza uma interface UART para:

* monitoramento da tensão;
* monitoramento da corrente;
* ajuste de parâmetros;
* depuração do sistema.

A transmissão utiliza **DMA**, reduzindo significativamente a utilização da CPU.

---

## Estrutura do repositório

```text
.
├── Firmware/
├── Hardware/
│   ├── Esquemáticos
│   ├── PCB
│   └── Bibliotecas
├── Documentação/
├── Imagens/
└── README.md
```

---

## Ferramentas utilizadas

* KiCad
* GCC para RISC-V
* WCH MounRiver Studio
* Git
* GitHub

---

## Aplicações

Este projeto pode ser utilizado como base para:

* estudos de Eletrônica de Potência;
* Sistemas Embarcados;
* Controle Digital;
* Fontes chaveadas;
* Conversores CC-CC;
* projetos acadêmicos;
* trabalhos de conclusão de curso.

---

## Licença

Este projeto está disponibilizado para fins acadêmicos e de pesquisa.

---

Caso este projeto seja utilizado em trabalhos acadêmicos, solicita-se a devida citação do repositório.

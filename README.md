# Conversor Buck Microcontrolado

Projeto de desenvolvimento de um **Conversor CC-CC Buck Microcontrolado**, utilizando o microcontrolador **CH32V003F4P6** para geração do sinal PWM e controle da tensão de saída.

## 📖 Descrição

Este projeto tem como objetivo implementar um conversor Buck com controle digital, permitindo o ajuste da razão cíclica (Duty Cycle) por meio de um microcontrolador.

O sistema foi desenvolvido para aplicações didáticas e de pesquisa em Eletrônica de Potência, podendo ser utilizado para estudos sobre:

- Conversores CC-CC;
- Geração de PWM;
- Controle digital;
- Aquisição de tensão via ADC;
- Controle em malha aberta e futura implementação em malha fechada (PI/PID).

---

## ⚙️ Características

- Microcontrolador CH32V003F4P6
- Conversor Buck
- Controle digital por PWM
- Ajuste do Duty Cycle por botões
- Monitoramento da tensão de saída
- Interface simples de operação
- Código em linguagem C

---

## 🖥 Hardware

### Microcontrolador

- CH32V003F4P6

### Entrada

- Fonte CC

### Saída

- Conversor Buck

### Controle

- PWM por Timer

### Sensores

- Leitura da tensão de saída via ADC

---

## 🎮 Funcionamento

Os botões disponíveis permitem:

| Botão | Função |
|--------|--------|
| BOT+ | Incrementa o Duty Cycle |
| BOT− | Decrementa o Duty Cycle |
| Liga/Desliga | Habilita ou desabilita o PWM |

As saídas digitais disponibilizam o valor do Duty Cycle em formato binário para depuração.

---

## 📂 Estrutura do Projeto

```
ConversorBuckMicrocontrolado/
│
├── Firmware/
├── Hardware/
├── Documentação/
├── PCB/
├── Simulações/
└── README.md
```

---

## 🔧 Ferramentas Utilizadas

- MounRiver Studio
- WCH-Link
- EasyEDA / KiCad (PCB)
- Git
- GitHub

---

## 🚀 Objetivos

- Desenvolver um conversor Buck de baixo custo.
- Implementar controle digital utilizando microcontrolador RISC-V.
- Disponibilizar um projeto open source para estudos em Eletrônica de Potência.

---

## 📈 Melhorias Futuras

- Controle PI
- Controle PID
- Display OLED
- Comunicação Serial
- Interface USB
- Telemetria
- Registro de dados
- Proteção contra sobrecorrente
- Proteção contra sobretensão

---

## 📷 Imagens

Em breve serão adicionadas imagens do circuito, PCB e funcionamento.

---

## 📄 Licença

Este projeto é disponibilizado para fins acadêmicos e educacionais.

---

## 👨‍💻 Autor

**Luís Alberto Martignago**

GitHub:
https://github.com/LuisMartignago

---

## ⭐ Contribuições

Contribuições são bem-vindas.

Caso encontre algum problema ou tenha sugestões de melhorias, fique à vontade para abrir uma *Issue* ou enviar um *Pull Request*.
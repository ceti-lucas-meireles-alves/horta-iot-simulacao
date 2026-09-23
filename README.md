# Horta IoT Escolar — Simulação Wokwi

Protótipo virtual para monitorar uma horta escolar usando ESP32, RFID, umidade simulada, servo e página web.

## Interações

- Gire o potenciômetro para representar a umidade do solo.
- Aproxime um cartão do leitor RFID para autorizar o usuário.
- Abra a página web exibida no monitor serial do Wokwi.
- Use os botões `Iniciar irrigação` e `Parar`.
- Pressione o botão físico para alternar a irrigação.
- Observe o servo e os LEDs de status.

## Limites da simulação

O potenciômetro representa um sensor de umidade; o servo representa uma válvula ou bomba. A simulação não valida dimensionamento elétrico, vazão, isolamento, potência do motor ou segurança de uma instalação real.

## Próxima etapa física

Antes de conectar o motor de 12 V, usar um driver apropriado, transistor/MOSFET ou ponte H, diodo de proteção e fonte separada conforme o componente escolhido. Nunca ligar o motor diretamente a um pino do Arduino ou ESP32.

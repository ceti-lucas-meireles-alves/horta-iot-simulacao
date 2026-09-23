# Horta IoT Escolar — Simulação Wokwi

Protótipo educacional de baixo custo para controle de acesso e rastreabilidade de recursos da horta escolar, usando ESP32, MFRC522, umidade simulada, servo e página web local.

## Objetivo científico

Projetar e avaliar um sistema IoT que identifique usuários por RFID, registre eventos e controle um atuador em uma maquete de infraestrutura rural, funcionando localmente quando a internet estiver indisponível.

O projeto relaciona programação, eletrônica, redes, segurança e sustentabilidade a problemas reais da comunidade: controle de ferramentas, acesso à horta, autorização de equipamentos e uso consciente de água e energia.

### Problema de pesquisa

Como um sistema baseado em ESP32 e MFRC522 pode contribuir para o controle e a rastreabilidade de espaços e recursos em uma escola rural com conectividade limitada?

### Hipótese

Um protótipo RFID+ESP32 pode identificar usuários, bloquear ações não autorizadas e manter o registro local dos eventos, apoiando a aprendizagem prática e a gestão responsável dos recursos escolares.

## Componentes e responsabilidades

| Componente | Função no protótipo | Evolução para a escola rural |
| --- | --- | --- |
| `board-esp32-devkit-c-v4` | Processamento, Wi-Fi, servidor local e controle dos atuadores | Monitorar horta, reservatório, ferramentas e energia |
| `board-mfrc522` | Leitura de cartões e tags RFID | Identificar alunos e autorizar acesso a espaços ou equipamentos |
| Potenciômetro | Simulação da umidade do solo | Substituição por sensor capacitivo em etapa física |
| Servo | Representação de fechadura ou válvula | Acionamento de mecanismo de maquete; bomba real exige driver apropriado |

## Aplicações possíveis

- controle de entrada na horta;
- retirada e devolução de ferramentas;
- autorização de acesso a reservatórios;
- registro de equipes responsáveis pela irrigação;
- controle de uma maquete de porta rural;
- futura integração com sensores de nível, umidade e consumo de energia.

## Interações

- Gire o potenciômetro para representar a umidade do solo.
- Aproxime um cartão do leitor RFID para autorizar o usuário.
- Abra a página web exibida no monitor serial do Wokwi.
- Use os botões `Iniciar irrigação` e `Parar`.
- Pressione o botão físico para alternar a irrigação.
- Observe o servo e os LEDs de status.

Cartões autorizados para o protótipo:

- `01020304`
- `11223344`

Cartões não cadastrados devem gerar acesso negado e não podem iniciar a irrigação.

## Funcionamento em conectividade limitada

O sistema foi planejado para operar localmente. A página web é servida pelo ESP32 e o registro principal aparece no monitor serial. Em uma montagem física, a próxima evolução é ativar um ponto de acesso local do ESP32 e armazenar os eventos em memória ou cartão SD, evitando dependência de internet.

## Limites da simulação

O potenciômetro representa um sensor de umidade; o servo representa uma válvula ou bomba. A simulação não valida dimensionamento elétrico, vazão, isolamento, potência do motor ou segurança de uma instalação real.

O MFRC522 identifica o UID do cartão, não uma pessoa por biometria. Portanto, a associação entre UID e aluno é apenas didática e deve ser protegida em uma instalação real. RFID também não deve ser tratado como autenticação inviolável.

## Referências científicas

- MANDASARI, R. Deasy et al. *AIoT-Based Soil Moisture Monitoring System for Precision Agriculture and Energy Efficiency in Rural Smart Villages*. REKA ELKOMIKA, 2025. Disponível em: <https://ejurnal.itenas.ac.id/index.php/rekaelkomika_pkm/article/view/14375>.
- WIJANARKO, Yudi; ALFARIZAL, Niksen; PRATAMA, Muhammad Regi. *Implementation of an RFID RC522 and IoT-Based Automatic Door Security System in an Electrical Engineering Laboratory*. 2025. DOI: <https://doi.org/10.24014/ijaidm.v8i2.37007>.
- VARSHA, Sree; GNANAMALAR, R. Hepziba. *Design and Implementation of an IoT-Based Smart Irrigation System for Sustainable Agriculture*. 2025. DOI: <https://doi.org/10.26438/ijsrcse.v14i1.782>.
- SUCIPTO, Rifa’i Adi et al. *Optimalisasi Aturan Akses Asrama Berbasis Data-Driven Menggunakan RFID Access Monitoring System*. 2026. Disponível em: <https://prosiding.pnj.ac.id/index.php/SNTE/article/view/5949>.

## Próxima etapa física

Antes de conectar o motor de 12 V, usar um driver apropriado, transistor/MOSFET ou ponte H, diodo de proteção e fonte separada conforme o componente escolhido. Nunca ligar o motor diretamente a um pino do Arduino ou ESP32.
